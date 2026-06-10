#include <optional>
#include <vector>

#include "absl/status/statusor.h"
#include "backend/engine/semantic/error.h"
#include "backend/engine/semantic/scan_eval_internal.h"
#include "backend/engine/semantic/value.h"
#include "googlesql/resolved_ast/resolved_ast.h"
#include "googlesql/resolved_ast/resolved_node_kind.pb.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace semantic {
namespace scan_eval_internal {

namespace {

int CompareHavingValues(const Value& a, const Value& b) {
  if (a.is_null() && b.is_null()) return 0;
  if (a.is_null()) return -1;
  if (b.is_null()) return 1;
  if (ValueEqual(a, b)) return 0;
  return ValueLess(a, b) ? -1 : 1;
}

}  // namespace

absl::StatusOr<std::vector<size_t>> FilterRowsByHavingModifier(
    const ::googlesql::ResolvedAggregateFunctionCall& call,
    const std::vector<ColumnBindings>& input_rows,
    const std::vector<size_t>& row_indices,
    EvalContext& ctx) {
  const auto* mod = call.having_modifier();
  if (mod == nullptr) return row_indices;
  if (mod->having_expr() == nullptr) {
    return MakeSemanticError(SemanticErrorReason::kInvalidArgument,
                             "semantic: HAVING modifier missing expression");
  }

  struct RowHaving {
    size_t index = 0;
    Value having_val;
  };
  std::vector<RowHaving> rows;
  rows.reserve(row_indices.size());
  for (size_t r : row_indices) {
    EvalContext row_ctx = ctx;
    row_ctx.columns = &input_rows[r];
    auto having_or = EvalExpr(*mod->having_expr(), row_ctx);
    if (!having_or.ok()) return having_or.status();
    rows.push_back(RowHaving{r, *std::move(having_or)});
  }

  std::optional<Value> best;
  for (const RowHaving& row : rows) {
    if (row.having_val.is_null()) continue;
    if (!best.has_value()) {
      best = row.having_val;
      continue;
    }
    const int cmp = CompareHavingValues(row.having_val, *best);
    const bool is_better =
        mod->kind() == ::googlesql::ResolvedAggregateHavingModifier::MAX
            ? cmp > 0
            : cmp < 0;
    if (is_better) {
      best = row.having_val;
    }
  }

  if (!best.has_value()) {
    return std::vector<size_t>{};
  }

  std::vector<size_t> filtered;
  filtered.reserve(rows.size());
  for (const RowHaving& row : rows) {
    if (!row.having_val.is_null() && ValueEqual(row.having_val, *best)) {
      filtered.push_back(row.index);
    }
  }
  return filtered;
}

}  // namespace scan_eval_internal
}  // namespace semantic
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
