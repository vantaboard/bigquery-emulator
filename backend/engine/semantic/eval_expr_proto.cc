// Value-table / range-variable row field access (`ResolvedGetRowField`).
//
// This used to also host the PROTO construction / field-access /
// REPLACE_FIELDS / FILTER_FIELDS handlers, but BigQuery
// (PRODUCT_EXTERNAL) does not expose user PROTO types or those
// functions (verified via `bq query --dry_run`: "Type not found" for
// `NEW <proto>`, "REPLACE_FIELDS() is not supported", "Function not
// found: FILTER_FIELDS"), so those shapes are now `unsupported`.
//
// `ResolvedGetRowField` IS reachable in BigQuery (e.g. value-table
// field access `t.f`), so its evaluator stays here.

#include <string>

#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "absl/strings/str_cat.h"
#include "backend/engine/semantic/eval_context.h"
#include "backend/engine/semantic/eval_expr.h"
#include "backend/engine/semantic/eval_expr_internal.h"
#include "backend/engine/semantic/value.h"
#include "googlesql/public/type.h"
#include "googlesql/public/types/struct_type.h"
#include "googlesql/resolved_ast/resolved_ast.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace semantic {
namespace eval_expr_internal {

absl::StatusOr<Value> EvalGetRowField(
    const ::googlesql::ResolvedGetRowField& node, const EvalContext& ctx) {
  if (node.expr() == nullptr) {
    return absl::InvalidArgumentError(
        "semantic: ResolvedGetRowField has null expr");
  }
  if (node.column() == nullptr) {
    return absl::InvalidArgumentError(
        "semantic: ResolvedGetRowField has null column");
  }
  auto base_or = EvalExpr(*node.expr(), ctx);
  if (!base_or.ok()) return base_or.status();
  if (base_or->is_null()) {
    return NullOfType(node.type());
  }
  if (!base_or->type()->IsStruct()) {
    return absl::InvalidArgumentError(
        "semantic: GetRowField base is not STRUCT");
  }
  const ::googlesql::StructType* st = base_or->type()->AsStruct();
  if (st == nullptr) {
    return absl::InvalidArgumentError(
        "semantic: GetRowField STRUCT type cast failed");
  }
  const std::string& name = node.column()->Name();
  for (int i = 0; i < st->num_fields(); ++i) {
    if (st->field(i).name == name) {
      return base_or->field(i);
    }
  }
  return absl::InvalidArgumentError(absl::StrCat(
      "semantic: GetRowField column '", name, "' not found in row STRUCT"));
}

}  // namespace eval_expr_internal
}  // namespace semantic
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
