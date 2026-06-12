#include <utility>
#include <vector>

#include "absl/status/statusor.h"
#include "backend/engine/semantic/eval_expr.h"
#include "backend/engine/semantic/value.h"
#include "googlesql/public/type.h"
#include "googlesql/public/value.h"
#include "googlesql/resolved_ast/resolved_ast.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace semantic {
namespace eval_expr_internal {

namespace {

using ::bigquery_emulator::backend::engine::semantic::EvalContext;
using ::bigquery_emulator::backend::engine::semantic::Value;

void AppendFlattenValues(const Value& input, std::vector<Value>& out) {
  if (input.type()->IsArray()) {
    if (input.is_null()) return;
    for (const Value& elem : input.elements()) {
      out.push_back(elem);
    }
  } else {
    out.push_back(input);
  }
}

}  // namespace

absl::StatusOr<Value> EvalFlattenedArg(const EvalContext& ctx) {
  if (ctx.flatten == nullptr || ctx.flatten->current == nullptr) {
    return absl::InternalError(
        "semantic: ResolvedFlattenedArg outside active FLATTEN evaluation");
  }
  return *ctx.flatten->current;
}

absl::StatusOr<Value> EvalFlatten(
    const ::googlesql::ResolvedFlatten& flatten, const EvalContext& ctx) {
  if (flatten.expr() == nullptr) {
    return absl::InvalidArgumentError("semantic: FLATTEN missing input expr");
  }
  auto initial_or = EvalExpr(*flatten.expr(), ctx);
  if (!initial_or.ok()) return initial_or.status();

  std::vector<Value> values;
  AppendFlattenValues(*initial_or, values);

  for (int i = 0; i < flatten.get_field_list_size(); ++i) {
    const ::googlesql::ResolvedExpr* get_field = flatten.get_field_list(i);
    if (get_field == nullptr) {
      return absl::InternalError("semantic: FLATTEN get_field_list has null");
    }
    if (values.empty()) break;

    std::vector<Value> next_values;
    FlattenEvalScope frame;
    EvalContext step_ctx = ctx;
    step_ctx.flatten = &frame;

    for (const Value& v : values) {
      if (v.is_null()) {
        const ::googlesql::Type* t = get_field->type();
        if (t != nullptr && !t->IsArray()) {
          next_values.push_back(Value::Null(t));
        }
        continue;
      }
      frame.current = &v;
      auto step_or = EvalExpr(*get_field, step_ctx);
      if (!step_or.ok()) return step_or.status();
      AppendFlattenValues(*step_or, next_values);
    }
    values = std::move(next_values);
  }

  if (flatten.type() == nullptr || !flatten.type()->IsArray()) {
    return absl::InternalError("semantic: FLATTEN missing ARRAY output type");
  }
  return Value::Array(flatten.type()->AsArray(), std::move(values));
}

}  // namespace eval_expr_internal
}  // namespace semantic
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
