#include "backend/engine/semantic/eval_expr.h"

#include <cmath>
#include <cstdint>
#include <string>
#include <utility>
#include <vector>

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

absl::StatusOr<Value> EvalExpr(const ::googlesql::ResolvedExpr& expr,
                               const EvalContext& ctx) {
  switch (expr.node_kind()) {
    case ::googlesql::RESOLVED_LITERAL: {
      const auto& lit = *expr.GetAs<::googlesql::ResolvedLiteral>();
      // ResolvedLiteral carries the analyzer-validated `Value`
      // directly; copying it is cheap (refcounted backing for
      // STRING / ARRAY / STRUCT).
      // cpp-lint:allow(statusor-unchecked-value) -- `lit.value()`
      // is `ResolvedLiteral::value()` returning `googlesql::Value`,
      // not a `StatusOr<T>::value()` unwrap.
      return lit.value();
    }
    case ::googlesql::RESOLVED_PARAMETER: {
      const auto& param = *expr.GetAs<::googlesql::ResolvedParameter>();
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
              absl::StrCat(
                  "semantic: no value bound for positional parameter #",
                  param.position()));
        }
        return ctx.parameters->by_position[idx];
      }
      return absl::InvalidArgumentError(
          "semantic: ResolvedParameter has neither name nor position");
    }
    case ::googlesql::RESOLVED_FUNCTION_CALL:
      return EvalFunctionCall(*expr.GetAs<::googlesql::ResolvedFunctionCall>(),
                              ctx);
    case ::googlesql::RESOLVED_CAST: {
      const auto& cast = *expr.GetAs<::googlesql::ResolvedCast>();
      if (cast.expr() == nullptr) {
        return absl::InvalidArgumentError(
            "semantic: ResolvedCast has null expr");
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
    case ::googlesql::RESOLVED_ARGUMENT_REF: {
      // `ResolvedArgumentRef` reads an argument of the enclosing
      // SQL UDF / TVF invocation. The caller (UDF / TVF executor
      // body) pushes a `FrameStack` frame at invocation, declares
      // each argument by name, and points `ctx.arguments` at the
      // frame stack. The analyzer canonicalizes argument names to
      // lower-case at registration, so the frame stack's case-
      // insensitive `Lookup` returns the right binding even if a
      // body case-shifts the reference (e.g. `RETURN X` vs. the
      // signature's `x`).
      //
      // Argument references arriving here with no frame stack
      // (i.e. `ctx.arguments == nullptr`) mean either: (a) the
      // analyzer emitted a `ResolvedArgumentRef` outside a UDF /
      // TVF body (engine wiring bug), or (b) the body is being
      // evaluated without the invocation frame plumbed through
      // (caller bug). Either way we surface a structured
      // `kInvalidArgument` so the gateway envelope names the
      // missing argument rather than silently substituting NULL.
      const auto& ref = *expr.GetAs<::googlesql::ResolvedArgumentRef>();
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
    case ::googlesql::RESOLVED_CONSTANT: {
      const auto& node = *expr.GetAs<::googlesql::ResolvedConstant>();
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
      // `ResolvedConstant` carries a non-owning pointer to a
      // `googlesql::Constant` registered on the catalog. The
      // BigQuery emulator's catalog adapter (today only the
      // analyzer's built-in constants, future module-defined
      // constants once `CREATE CONSTANT` lands) stores
      // `SimpleConstant` instances whose `HasValue()` is true and
      // `GetValue()` returns the bound `Value` verbatim. Constants
      // whose value is not available yet (e.g. an unresolved
      // `SQLConstant`) surface as a structured `kInvalidArgument`
      // so the gateway envelope names the constant rather than
      // silently substituting NULL.
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
    case ::googlesql::RESOLVED_COLUMN_REF: {
      // A `ResolvedColumnRef` reads a column the surrounding scan
      // emits row-at-a-time. The FROM-clause executor binds the
      // current row's `ColumnBindings` onto `ctx.columns` before
      // calling `EvalExpr`; a missing binding is an analyzer /
      // executor mismatch and surfaces a structured INVALID_ARGUMENT.
      // Scalar-only SELECT keeps `ctx.columns == nullptr` and
      // every column reference there is a bug -- the scalar path
      // resolves columns through `ResolvedProjectScan::expr_list`,
      // not through column refs.
      const auto& ref = *expr.GetAs<::googlesql::ResolvedColumnRef>();
      if (ctx.columns == nullptr) {
        if (ctx.columns_by_name != nullptr) {
          auto name_it =
              ctx.columns_by_name->find(std::string(ref.column().name()));
          if (name_it != ctx.columns_by_name->end()) {
            return name_it->second;
          }
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
    case ::googlesql::RESOLVED_MAKE_STRUCT: {
      const auto& node = *expr.GetAs<::googlesql::ResolvedMakeStruct>();
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
    case ::googlesql::RESOLVED_GET_STRUCT_FIELD: {
      const auto& node = *expr.GetAs<::googlesql::ResolvedGetStructField>();
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
      const int idx = node.field_idx();
      if (idx < 0 || idx >= base->num_fields()) {
        return absl::InvalidArgumentError(
            "semantic: GetStructField index out of range");
      }
      return base->field(idx);
    }
    case ::googlesql::RESOLVED_GET_JSON_FIELD: {
      const auto& node = *expr.GetAs<::googlesql::ResolvedGetJsonField>();
      if (node.expr() == nullptr) {
        return absl::InvalidArgumentError(
            "semantic: ResolvedGetJsonField has null expr");
      }
      auto base = EvalExpr(*node.expr(), ctx);
      if (!base.ok()) return base.status();
      return functions::JsonGetField(*base, node.field_name(), expr.type());
    }
    case ::googlesql::RESOLVED_WITH_EXPR: {
      const auto& node = *expr.GetAs<::googlesql::ResolvedWithExpr>();
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
    case ::googlesql::RESOLVED_SUBQUERY_EXPR:
      return EvalSubqueryExpr(*expr.GetAs<::googlesql::ResolvedSubqueryExpr>(),
                              ctx);
    case ::googlesql::RESOLVED_SYSTEM_VARIABLE: {
      const auto& node = *expr.GetAs<::googlesql::ResolvedSystemVariable>();
      return GetSystemVariable(ctx.project_id, node.name_path());
    }
    case ::googlesql::RESOLVED_AGGREGATE_FUNCTION_CALL:
      if (ctx.udaf != nullptr) {
        return EvalUdafInnerAggregate(
            *expr.GetAs<::googlesql::ResolvedAggregateFunctionCall>(),
            *ctx.udaf,
            ctx);
      }
      return MakeSemanticError(
          SemanticErrorReason::kNotImplemented,
          "semantic: aggregate function call outside SQL UDAF body evaluation "
          "is not yet implemented");
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
