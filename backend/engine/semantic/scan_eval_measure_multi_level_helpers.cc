#include <map>
#include <string>
#include <utility>
#include <vector>

#include "absl/container/flat_hash_map.h"
#include "absl/status/statusor.h"
#include "backend/engine/semantic/error.h"
#include "backend/engine/semantic/eval_udaf.h"
#include "backend/engine/semantic/scan_eval_internal.h"
#include "backend/engine/semantic/value.h"
#include "googlesql/resolved_ast/resolved_ast.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace semantic {
namespace scan_eval_internal {

using ::bigquery_emulator::backend::engine::semantic::EvalContext;
using ::bigquery_emulator::backend::engine::semantic::EvalExpr;

absl::StatusOr<
    std::map<std::string, std::pair<std::vector<Value>, std::vector<size_t>>>>
BuildMultiLevelInnerGroups(
    const ::googlesql::ResolvedAggregateFunctionCall& agg,
    const ::googlesql::ResolvedScan* input_scan,
    const std::vector<ColumnBindings>& input_rows,
    const std::vector<size_t>& effective_rows,
    const EvalContext& ctx) {
  std::map<std::string, std::pair<std::vector<Value>, std::vector<size_t>>>
      inner_groups;
  absl::flat_hash_map<std::string, Value> row_columns_by_name;
  for (size_t r : effective_rows) {
    EvalContext row_ctx = ctx;
    row_ctx.columns = &input_rows[r];
    row_columns_by_name.clear();
    PopulateColumnNameBindingsDeep(
        input_scan, input_rows[r], row_columns_by_name);
    if (ctx.columns_by_name != nullptr) {
      for (const auto& [name, val] : *ctx.columns_by_name) {
        row_columns_by_name[name] = val;
      }
    }
    row_ctx.columns_by_name = &row_columns_by_name;

    std::vector<Value> keys;
    keys.reserve(agg.group_by_list_size());
    for (int g = 0; g < agg.group_by_list_size(); ++g) {
      const ::googlesql::ResolvedComputedColumn* gc = agg.group_by_list(g);
      if (gc == nullptr || gc->expr() == nullptr) {
        return absl::InternalError(
            "semantic: multi-level aggregate group_by column has null expr");
      }
      auto key = EvalExpr(*gc->expr(), row_ctx);
      if (!key.ok()) return key.status();
      keys.push_back(*std::move(key));
    }
    const std::string fp = GroupKeyFingerprint(keys);
    auto it = inner_groups.find(fp);
    if (it == inner_groups.end()) {
      inner_groups.emplace(
          fp, std::make_pair(std::move(keys), std::vector<size_t>{r}));
    } else {
      it->second.second.push_back(r);
    }
  }
  return inner_groups;
}

absl::StatusOr<MultiLevelInnerRow> EvalMultiLevelInnerGroup(
    const ::googlesql::ResolvedAggregateFunctionCall& agg,
    const ::googlesql::ResolvedScan* input_scan,
    const std::vector<ColumnBindings>& input_rows,
    const std::vector<Value>& group_keys,
    const std::vector<size_t>& row_indices,
    const EvalContext& ctx) {
  MultiLevelInnerRow inner;
  for (int g = 0; g < agg.group_by_list_size(); ++g) {
    const ::googlesql::ResolvedComputedColumn* gc = agg.group_by_list(g);
    inner.bindings.emplace(gc->column().column_id(),
                           group_keys[static_cast<size_t>(g)]);
    inner.by_name[std::string(gc->column().name())] =
        group_keys[static_cast<size_t>(g)];
  }
  for (int i = 0; i < agg.group_by_aggregate_list_size(); ++i) {
    const ::googlesql::ResolvedComputedColumnBase* cc =
        agg.group_by_aggregate_list(i);
    if (cc == nullptr || cc->expr() == nullptr) {
      return absl::InternalError(
          "semantic: multi-level aggregate inner column has null expr");
    }
    const auto* inner_agg =
        cc->expr()->GetAs<::googlesql::ResolvedAggregateFunctionCall>();
    if (inner_agg == nullptr) {
      return MakeSemanticError(
          SemanticErrorReason::kNotImplemented,
          "semantic: multi-level aggregate inner expression is not an "
          "aggregate call");
    }
    absl::StatusOr<Value> val =
        IsGrainLockAnyValue(*inner_agg, *cc)
            ? EvalGrainLockInnerWithOuterAggregate(
                  agg, *inner_agg, input_scan, input_rows, row_indices, ctx)
            : EvalAggregateForRows(
                  *inner_agg, input_scan, input_rows, row_indices, ctx);
    if (!val.ok()) return val.status();
    inner.bindings.emplace(cc->column().column_id(), *val);
    inner.by_name[std::string(cc->column().name())] = *val;
  }
  return inner;
}

absl::StatusOr<bool> EvalMultiLevelHaving(
    const ::googlesql::ResolvedAggregateFunctionCall& agg,
    const MultiLevelInnerRow& inner,
    const EvalContext& ctx) {
  if (agg.having_expr() == nullptr) return true;
  EvalContext having_ctx = ctx;
  having_ctx.columns = &inner.bindings;
  having_ctx.columns_by_name = &inner.by_name;
  auto having_or = EvalExpr(*agg.having_expr(), having_ctx);
  if (!having_or.ok()) return having_or.status();
  return !having_or->is_null() && having_or->bool_value();
}

absl::StatusOr<std::vector<std::vector<Value>>> BuildMultiLevelOuterArgColumns(
    const ::googlesql::ResolvedAggregateFunctionCall& agg,
    std::vector<MultiLevelInnerRow>& inner_results,
    std::vector<ColumnBindings>& outer_input_rows,
    const EvalContext& ctx) {
  std::vector<std::vector<Value>> arg_columns(
      static_cast<size_t>(agg.argument_list_size()));
  outer_input_rows.reserve(inner_results.size());
  for (MultiLevelInnerRow& inner : inner_results) {
    outer_input_rows.push_back(inner.bindings);
    const size_t row_index = outer_input_rows.size() - 1;
    EvalContext row_ctx = ctx;
    row_ctx.columns = &outer_input_rows[row_index];
    row_ctx.columns_by_name = &inner.by_name;
    for (int a = 0; a < agg.argument_list_size(); ++a) {
      auto v = EvalExpr(*agg.argument_list(a), row_ctx);
      if (!v.ok()) return v.status();
      arg_columns[static_cast<size_t>(a)].push_back(*std::move(v));
    }
  }
  return arg_columns;
}

}  // namespace scan_eval_internal
}  // namespace semantic
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
