#include "backend/engine/semantic/eval_expr.h"

#include <cmath>
#include <cstdint>
#include <string>
#include <utility>
#include <vector>

#include "absl/container/flat_hash_map.h"
#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "absl/strings/ascii.h"
#include "absl/strings/match.h"
#include "absl/strings/numbers.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/string_view.h"
#include "backend/engine/semantic/error.h"
#include "backend/engine/semantic/eval_expr_internal.h"
#include "backend/engine/semantic/eval_udaf.h"
#include "backend/engine/semantic/frame_stack.h"
#include "backend/engine/semantic/functions/datetime_funcs_internal.h"
#include "backend/engine/semantic/functions/json_funcs.h"
#include "backend/engine/semantic/system_variables.h"
#include "backend/engine/semantic/value.h"
#include "googlesql/public/constant.h"
#include "googlesql/public/functions/date_time_util.h"
#include "googlesql/public/type.h"
#include "googlesql/public/types/struct_type.h"
#include "googlesql/public/value.h"
#include "googlesql/resolved_ast/resolved_ast.h"
#include "googlesql/resolved_ast/resolved_node_kind.pb.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace semantic {

using eval_expr_internal::EvalResolvedCast;
using eval_expr_internal::NullOfType;
using eval_expr_internal::ToDouble;

// Implemented in `scan_eval.cc`; linked via the `scan_eval` target.
absl::StatusOr<Value> EvalSubqueryExpr(
    const ::googlesql::ResolvedSubqueryExpr& node, const EvalContext& ctx);

namespace {

absl::StatusOr<Value> DefaultInternalAnalyzerColumn(
    const ::googlesql::ResolvedColumn& col) {
  const absl::string_view name = col.name();
  if (name.empty() || name[0] != '$') {
    return absl::NotFoundError("not an internal analyzer column");
  }
  if (name == "$side_effects") {
    const ::googlesql::Type* type = col.type();
    if (type != nullptr && type->kind() == ::googlesql::TYPE_BYTES) {
      return Value::NullBytes();
    }
    return Value::Null(type);
  }
  const ::googlesql::Type* type = col.type();
  if (type != nullptr && type->kind() == ::googlesql::TYPE_BOOL) {
    return Value::Bool(false);
  }
  return Value::Null(type);
}

absl::StatusOr<Value> EvalResolvedLiteral(
    const ::googlesql::ResolvedLiteral& lit) {
  // cpp-lint:allow(statusor-unchecked-value) -- `lit.value()` is
  // `ResolvedLiteral::value()` returning `googlesql::Value`, not a
  // `StatusOr<T>::value()` unwrap.
  return lit.value();
}

absl::StatusOr<Value> EvalResolvedParameter(
    const ::googlesql::ResolvedParameter& param, const EvalContext& ctx) {
  if (ctx.parameters == nullptr) {
    return absl::InvalidArgumentError(
        "semantic: ResolvedParameter referenced but no parameter "
        "bindings supplied");
  }
  if (param.is_untyped()) {
    return MakeSemanticError(
        SemanticErrorReason::kInvalidArgument,
        "semantic: untyped parameter has no value to bind to");
  }
  if (!param.name().empty()) {
    const std::string key = absl::AsciiStrToLower(param.name());
    auto it = ctx.parameters->by_name.find(key);
    if (it == ctx.parameters->by_name.end()) {
      return MakeSemanticError(
          SemanticErrorReason::kInvalidArgument,
          absl::StrCat("semantic: no value bound for parameter @",
                       param.name()));
    }
    return it->second;
  }
  if (param.position() > 0) {
    size_t idx = static_cast<size_t>(param.position()) - 1;
    if (idx >= ctx.parameters->by_position.size()) {
      return MakeSemanticError(
          SemanticErrorReason::kInvalidArgument,
          absl::StrCat("semantic: no value bound for positional parameter #",
                       param.position()));
    }
    return ctx.parameters->by_position[idx];
  }
  return absl::InvalidArgumentError(
      "semantic: ResolvedParameter has neither name nor position");
}

absl::StatusOr<Value> LookupColumnRef(const ::googlesql::ResolvedColumnRef& ref,
                                      const EvalContext& ctx) {
  if (ctx.columns == nullptr) {
    if (ctx.columns_by_name != nullptr) {
      auto name_it =
          ctx.columns_by_name->find(std::string(ref.column().name()));
      if (name_it != ctx.columns_by_name->end()) {
        return name_it->second;
      }
    }
    if (auto internal = DefaultInternalAnalyzerColumn(ref.column());
        internal.ok()) {
      return *std::move(internal);
    }
    return MakeSemanticError(
        SemanticErrorReason::kNotImplemented,
        absl::StrCat("semantic: ResolvedColumnRef '",
                     ref.column().name(),
                     "' referenced without a row binding; correlated scans "
                     "require correlated scan support; see "
                     "docs/ENGINE_POLICY.md"));
  }
  auto it = ctx.columns->find(ref.column().column_id());
  if (it == ctx.columns->end()) {
    if (ctx.columns_by_name != nullptr) {
      auto name_it =
          ctx.columns_by_name->find(std::string(ref.column().name()));
      if (name_it != ctx.columns_by_name->end()) {
        return name_it->second;
      }
    }
    if (auto internal = DefaultInternalAnalyzerColumn(ref.column());
        internal.ok()) {
      return *std::move(internal);
    }
    return MakeSemanticError(
        SemanticErrorReason::kInvalidArgument,
        absl::StrCat("semantic: no row binding for column '",
                     ref.column().name(),
                     "' (column_id=",
                     ref.column().column_id(),
                     ")"));
  }
  return it->second;
}

absl::StatusOr<Value> EvalResolvedConstant(
    const ::googlesql::ResolvedConstant& node, const EvalContext& ctx) {
  const ::googlesql::Constant* constant = node.constant();
  if (constant == nullptr) {
    return absl::InternalError(
        "semantic: ResolvedConstant has null constant pointer");
  }
  if (ctx.script_variables != nullptr &&
      ctx.script_variables->Has(constant->Name())) {
    absl::StatusOr<Value> bound =
        ctx.script_variables->Lookup(constant->Name());
    if (!bound.ok()) return bound.status();
    return *std::move(bound);
  }
  if (!constant->HasValue()) {
    return MakeSemanticError(
        SemanticErrorReason::kInvalidArgument,
        absl::StrCat("semantic: constant '",
                     constant->FullName(),
                     "' has no bound value (catalog returned "
                     "HasValue=false)"));
  }
  absl::StatusOr<Value> value = constant->GetValue();
  if (!value.ok()) {
    return MakeSemanticError(SemanticErrorReason::kInvalidArgument,
                             absl::StrCat("semantic: constant '",
                                          constant->FullName(),
                                          "' failed to provide its value: ",
                                          value.status().message()));
  }
  return *std::move(value);
}

absl::StatusOr<Value> EvalResolvedMakeStruct(
    const ::googlesql::ResolvedMakeStruct& node, const EvalContext& ctx) {
  const ::googlesql::Type* type = node.type();
  if (type == nullptr || !type->IsStruct()) {
    return absl::InvalidArgumentError(
        "semantic: ResolvedMakeStruct has non-STRUCT type");
  }
  const ::googlesql::StructType* st = type->AsStruct();
  if (st == nullptr || st->num_fields() != node.field_list_size()) {
    return absl::InvalidArgumentError(
        "semantic: ResolvedMakeStruct field count mismatch");
  }
  std::vector<Value> fields;
  fields.reserve(node.field_list_size());
  for (int i = 0; i < node.field_list_size(); ++i) {
    const ::googlesql::ResolvedExpr* field_expr = node.field_list(i);
    if (field_expr == nullptr) {
      return absl::InvalidArgumentError(
          "semantic: ResolvedMakeStruct field is null");
    }
    auto v = EvalExpr(*field_expr, ctx);
    if (!v.ok()) return v.status();
    fields.push_back(*std::move(v));
  }
  return Value::Struct(st, std::move(fields));
}

absl::StatusOr<Value> EvalResolvedWithExpr(
    const ::googlesql::ResolvedWithExpr& node, const EvalContext& ctx) {
  if (node.expr() == nullptr) {
    return absl::InvalidArgumentError(
        "semantic: ResolvedWithExpr has null body expr");
  }
  ColumnBindings bindings;
  if (ctx.columns != nullptr) {
    bindings = *ctx.columns;
  }
  absl::flat_hash_map<std::string, Value> by_name;
  EvalContext inner_ctx = ctx;
  for (int i = 0; i < node.assignment_list_size(); ++i) {
    const ::googlesql::ResolvedComputedColumn* cc = node.assignment_list(i);
    if (cc == nullptr || cc->expr() == nullptr) {
      return absl::InternalError(
          "semantic: ResolvedWithExpr assignment has null expr");
    }
    auto bound = EvalExpr(*cc->expr(), inner_ctx);
    if (!bound.ok()) return bound.status();
    bindings.emplace(cc->column().column_id(), *bound);
    by_name[std::string(cc->column().name())] = *bound;
    inner_ctx.columns = &bindings;
    inner_ctx.columns_by_name = &by_name;
  }
  inner_ctx.columns = &bindings;
  inner_ctx.columns_by_name = &by_name;
  return EvalExpr(*node.expr(), inner_ctx);
}

absl::StatusOr<Value> EvalResolvedCastExpr(
    const ::googlesql::ResolvedCast& cast, const EvalContext& ctx) {
  if (cast.expr() == nullptr) {
    return absl::InvalidArgumentError("semantic: ResolvedCast has null expr");
  }
  auto inner = EvalExpr(*cast.expr(), ctx);
  if (!inner.ok()) {
    if (cast.return_null_on_error() && cast.type() != nullptr) {
      return NullOfType(cast.type());
    }
    return inner;
  }
  const ::googlesql::Type* source = cast.expr()->type();
  return EvalResolvedCast(cast, *std::move(inner), source);
}

absl::StatusOr<Value> EvalResolvedArgumentRefExpr(
    const ::googlesql::ResolvedArgumentRef& ref, const EvalContext& ctx) {
  if (ctx.arguments == nullptr) {
    return MakeSemanticError(
        SemanticErrorReason::kInvalidArgument,
        absl::StrCat("semantic: ResolvedArgumentRef '",
                     ref.name(),
                     "' evaluated without an invocation frame; UDF / "
                     "TVF body executors must populate "
                     "EvalContext::arguments before calling EvalExpr"));
  }
  absl::StatusOr<Value> bound = ctx.arguments->Lookup(ref.name());
  if (!bound.ok()) {
    return MakeSemanticError(
        SemanticErrorReason::kInvalidArgument,
        absl::StrCat("semantic: ResolvedArgumentRef '",
                     ref.name(),
                     "' has no binding on the invocation frame: ",
                     bound.status().message()));
  }
  return *std::move(bound);
}

absl::StatusOr<Value> EvalResolvedGetStructFieldExpr(
    const ::googlesql::ResolvedGetStructField& node,
    const ::googlesql::ResolvedExpr& expr,
    const EvalContext& ctx) {
  if (node.expr() == nullptr) {
    return absl::InvalidArgumentError(
        "semantic: ResolvedGetStructField has null expr");
  }
  auto base = EvalExpr(*node.expr(), ctx);
  if (!base.ok()) return base.status();
  if (base->is_null()) return NullOfType(expr.type());
  if (!base->type()->IsStruct()) {
    return absl::InvalidArgumentError(
        "semantic: GetStructField base is not STRUCT");
  }
  const int field_idx = node.field_idx();
  if (field_idx < 0 || field_idx >= base->num_fields()) {
    return absl::InvalidArgumentError(
        "semantic: GetStructField index out of range");
  }
  return base->field(field_idx);
}

absl::StatusOr<Value> EvalResolvedGetJsonFieldExpr(
    const ::googlesql::ResolvedGetJsonField& node,
    const ::googlesql::ResolvedExpr& expr,
    const EvalContext& ctx) {
  if (node.expr() == nullptr) {
    return absl::InvalidArgumentError(
        "semantic: ResolvedGetJsonField has null expr");
  }
  auto base = EvalExpr(*node.expr(), ctx);
  if (!base.ok()) return base.status();
  return functions::JsonGetField(*base, node.field_name(), expr.type());
}

absl::StatusOr<Value> EvalResolvedSystemVariableExpr(
    const ::googlesql::ResolvedSystemVariable& node, const EvalContext& ctx) {
  if (ctx.script_system_variables != nullptr) {
    auto it = ctx.script_system_variables->find(node.name_path());
    if (it != ctx.script_system_variables->end()) {
      return it->second;
    }
  }
  return GetSystemVariable(ctx.project_id, node.name_path());
}

absl::StatusOr<Value> EvalResolvedAggregateFunctionCallExpr(
    const ::googlesql::ResolvedAggregateFunctionCall& agg,
    const EvalContext& ctx) {
  if (ctx.udaf != nullptr) {
    return EvalUdafInnerAggregate(agg, *ctx.udaf, ctx);
  }
  return MakeSemanticError(
      SemanticErrorReason::kNotImplemented,
      "semantic: aggregate function call outside SQL UDAF body evaluation "
      "is not yet implemented");
}

absl::StatusOr<Value> EvalResolvedExpressionColumnExpr(
    const ::googlesql::ResolvedExpressionColumn& col, const EvalContext& ctx) {
  if (ctx.columns_by_name != nullptr) {
    auto it = ctx.columns_by_name->find(col.name());
    if (it != ctx.columns_by_name->end()) {
      return it->second;
    }
  }
  if (ctx.script_variables != nullptr) {
    absl::StatusOr<Value> bound = ctx.script_variables->Lookup(col.name());
    if (bound.ok()) return *std::move(bound);
  }
  return MakeSemanticError(
      SemanticErrorReason::kInvalidArgument,
      absl::StrCat("semantic: no binding for expression column '",
                   col.name(),
                   "'; populate EvalContext::columns_by_name or "
                   "script_variables before evaluating AnalyzeExpression "
                   "output"));
}

absl::StatusOr<Value> EvalResolvedCatalogColumnRefExpr(
    const ::googlesql::ResolvedCatalogColumnRef& ref) {
  if (!ref.name().empty()) {
    return MakeSemanticError(
        SemanticErrorReason::kNotImplemented,
        "semantic: ResolvedCatalogColumnRef graph-property references "
        "are out of scope locally (Graph / GQL is unsupported and not "
        "planned; see docs/ENGINE_POLICY.md)");
  }
  if (ref.column() == nullptr) {
    return MakeSemanticError(
        SemanticErrorReason::kInvalidArgument,
        "semantic: ResolvedCatalogColumnRef has neither catalog column "
        "nor graph property name");
  }
  return MakeSemanticError(
      SemanticErrorReason::kNotImplemented,
      absl::StrCat("semantic: ResolvedCatalogColumnRef for catalog column '",
                   ref.column()->Name(),
                   "' is not reachable from PRODUCT_EXTERNAL query SQL "
                   "today; DDL expression contexts remain unsupported "
                   "(see ROADMAP §Catalog / sequence helpers)"));
}

}  // namespace

absl::StatusOr<Value> EvalExpr(const ::googlesql::ResolvedExpr& expr,
                               const EvalContext& ctx) {
  switch (expr.node_kind()) {
    case ::googlesql::RESOLVED_LITERAL:
      return EvalResolvedLiteral(*expr.GetAs<::googlesql::ResolvedLiteral>());
    case ::googlesql::RESOLVED_PARAMETER:
      return EvalResolvedParameter(
          *expr.GetAs<::googlesql::ResolvedParameter>(), ctx);
    case ::googlesql::RESOLVED_FUNCTION_CALL:
      return EvalFunctionCall(*expr.GetAs<::googlesql::ResolvedFunctionCall>(),
                              ctx);
    case ::googlesql::RESOLVED_CAST:
      return EvalResolvedCastExpr(*expr.GetAs<::googlesql::ResolvedCast>(),
                                  ctx);
    case ::googlesql::RESOLVED_ARGUMENT_REF:
      return EvalResolvedArgumentRefExpr(
          *expr.GetAs<::googlesql::ResolvedArgumentRef>(), ctx);
    case ::googlesql::RESOLVED_COLUMN_REF:
      return LookupColumnRef(*expr.GetAs<::googlesql::ResolvedColumnRef>(),
                             ctx);
    case ::googlesql::RESOLVED_CONSTANT:
      return EvalResolvedConstant(*expr.GetAs<::googlesql::ResolvedConstant>(),
                                  ctx);
    case ::googlesql::RESOLVED_MAKE_STRUCT:
      return EvalResolvedMakeStruct(
          *expr.GetAs<::googlesql::ResolvedMakeStruct>(), ctx);
    case ::googlesql::RESOLVED_GET_STRUCT_FIELD:
      return EvalResolvedGetStructFieldExpr(
          *expr.GetAs<::googlesql::ResolvedGetStructField>(), expr, ctx);
    case ::googlesql::RESOLVED_GET_JSON_FIELD:
      return EvalResolvedGetJsonFieldExpr(
          *expr.GetAs<::googlesql::ResolvedGetJsonField>(), expr, ctx);
    case ::googlesql::RESOLVED_WITH_EXPR:
      return EvalResolvedWithExpr(*expr.GetAs<::googlesql::ResolvedWithExpr>(),
                                  ctx);
    case ::googlesql::RESOLVED_SUBQUERY_EXPR:
      return EvalSubqueryExpr(*expr.GetAs<::googlesql::ResolvedSubqueryExpr>(),
                              ctx);
    case ::googlesql::RESOLVED_SYSTEM_VARIABLE:
      return EvalResolvedSystemVariableExpr(
          *expr.GetAs<::googlesql::ResolvedSystemVariable>(), ctx);
    case ::googlesql::RESOLVED_AGGREGATE_FUNCTION_CALL:
      return EvalResolvedAggregateFunctionCallExpr(
          *expr.GetAs<::googlesql::ResolvedAggregateFunctionCall>(), ctx);
    case ::googlesql::RESOLVED_GET_ROW_FIELD:
      return eval_expr_internal::EvalGetRowField(
          *expr.GetAs<::googlesql::ResolvedGetRowField>(), ctx);
    case ::googlesql::RESOLVED_EXPRESSION_COLUMN:
      return EvalResolvedExpressionColumnExpr(
          *expr.GetAs<::googlesql::ResolvedExpressionColumn>(), ctx);
    case ::googlesql::RESOLVED_CATALOG_COLUMN_REF:
      return EvalResolvedCatalogColumnRefExpr(
          *expr.GetAs<::googlesql::ResolvedCatalogColumnRef>());
    default:
      return MakeSemanticError(SemanticErrorReason::kNotImplemented,
                               absl::StrCat("semantic: ResolvedExpr kind ",
                                            expr.node_kind_string(),
                                            " is not yet implemented"));
  }
}

}  // namespace semantic
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
