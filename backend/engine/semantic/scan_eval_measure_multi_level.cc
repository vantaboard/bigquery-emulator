#include <cstring>

#include "googlesql/public/function.h"
#include "googlesql/public/sql_function.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace semantic {
namespace scan_eval_internal {

using ::bigquery_emulator::backend::engine::semantic::EvalContext;
using ::bigquery_emulator::backend::engine::semantic::EvalExpr;

std::string PlainAggregateNameForPrivacyStub(absl::string_view agg_name) {
  std::string stripped(agg_name);
  if (absl::StartsWith(stripped, "$differential_privacy_")) {
    stripped.erase(0, strlen("$differential_privacy_"));
  } else if (absl::StartsWith(stripped, "$anon_")) {
    stripped.erase(0, strlen("$anon_"));
  } else {
    return std::string(agg_name);
  }
  static constexpr absl::string_view kReportSuffixes[] = {
      "_with_report_json",
      "_with_report_proto",
      "_report_json",
      "_report_proto",
  };
  for (absl::string_view suffix : kReportSuffixes) {
    if (absl::EndsWith(stripped, suffix)) {
      stripped.erase(stripped.size() - suffix.size());
      break;
    }
  }
  if (stripped == "count_star" || absl::StartsWith(stripped, "count_star")) {
    return "$count_star";
  }
  static constexpr absl::string_view kTypeSuffixes[] = {
      "_double_array",
      "_int64",
      "_uint64",
      "_double",
      "_numeric",
  };
  for (absl::string_view suffix : kTypeSuffixes) {
    if (absl::EndsWith(stripped, suffix)) {
      stripped.erase(stripped.size() - suffix.size());
      break;
    }
  }
  return stripped;
}

bool IsGrainLockAnyValue(
    const ::googlesql::ResolvedAggregateFunctionCall& inner_agg,
    const ::googlesql::ResolvedComputedColumnBase& cc) {
  if (inner_agg.function() == nullptr ||
      absl::AsciiStrToLower(inner_agg.function()->Name()) != "any_value") {
    return false;
  }
  return absl::StartsWith(cc.column().name(), "$any_value_grain_lock");
}

absl::StatusOr<Value> FinishAggregateFromArgColumns(
    const ::googlesql::ResolvedAggregateFunctionCall& agg,
    const std::vector<std::vector<Value>>& arg_columns,
    const std::vector<ColumnBindings>& input_rows,
    EvalContext& ctx);

// Measure rewrite grain-locks with inner `any_value` nodes. At evaluation
// time the inner group should apply the outer constituent aggregate (e.g.
// SUM(amount) per store_id grain), not a literal ANY_VALUE pick.
absl::StatusOr<Value> EvalGrainLockInnerWithOuterAggregate(
    const ::googlesql::ResolvedAggregateFunctionCall& outer_agg,
    const ::googlesql::ResolvedAggregateFunctionCall& inner_any_value,
    const ::googlesql::ResolvedScan* input_scan,
    const std::vector<ColumnBindings>& input_rows,
    const std::vector<size_t>& row_indices,
    EvalContext& ctx) {
  std::vector<std::vector<Value>> arg_columns(
      static_cast<size_t>(inner_any_value.argument_list_size()));
  absl::flat_hash_map<std::string, Value> row_columns_by_name;
  for (size_t r : row_indices) {
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
    for (int a = 0; a < inner_any_value.argument_list_size(); ++a) {
      auto v = EvalExpr(*inner_any_value.argument_list(a), row_ctx);
      if (!v.ok()) return v.status();
      arg_columns[static_cast<size_t>(a)].push_back(*std::move(v));
    }
  }
  std::vector<ColumnBindings> dummy_rows(row_indices.size());
  return FinishAggregateFromArgColumns(outer_agg, arg_columns, dummy_rows, ctx);
}

struct MultiLevelInnerRow {
  ColumnBindings bindings{};
  absl::flat_hash_map<std::string, Value> by_name{};
};

absl::StatusOr<Value> FinishAggregateFromArgColumns(
    const ::googlesql::ResolvedAggregateFunctionCall& agg,
    const std::vector<std::vector<Value>>& arg_columns,
    const std::vector<ColumnBindings>& input_rows,
    EvalContext& ctx) {
  std::string agg_name;
  if (agg.function() != nullptr) {
    agg_name = absl::AsciiStrToLower(
        agg.function()->FullName(/*include_group=*/false));
    if (agg_name.empty()) {
      agg_name = absl::AsciiStrToLower(agg.function()->Name());
    }
  }
  agg_name = PlainAggregateNameForPrivacyStub(agg_name);
  if (agg_name == "$count_star") {
    return Value::Int64(
        static_cast<int64_t>(arg_columns.empty() ? 0 : arg_columns[0].size()));
  }
  if (agg.function() != nullptr &&
      absl::AsciiStrToLower(agg.function()->Name()) == "array_agg") {
    return EvalArrayAgg(agg, arg_columns, input_rows, ctx);
  }
  if (agg.function() != nullptr &&
      absl::AsciiStrToLower(agg.function()->Name()) == "string_agg") {
    return EvalStringAgg(agg, arg_columns, input_rows, ctx);
  }
  if (agg.function() != nullptr &&
      agg.function()->GetGroup() ==
          ::googlesql::SQLFunction::kSQLFunctionGroup &&
      agg.function()->IsAggregate()) {
    const auto* sql_fn =
        dynamic_cast<const ::googlesql::SQLFunction*>(agg.function());
    if (sql_fn == nullptr) {
      return absl::InvalidArgumentError(
          "semantic: aggregate function is not an SQL function");
    }
    std::vector<size_t> all_rows;
    all_rows.reserve(input_rows.size());
    for (size_t i = 0; i < input_rows.size(); ++i) {
      all_rows.push_back(i);
    }
    return EvalSqlUdafBody(agg, *sql_fn, arg_columns, all_rows, ctx);
  }
  return functions::EvalAggregateCall(agg, arg_columns);
}

absl::StatusOr<Value> EvalMultiLevelAggregateForRows(
    const ::googlesql::ResolvedAggregateFunctionCall& agg,
    const ::googlesql::ResolvedScan* input_scan,
    const std::vector<ColumnBindings>& input_rows,
    const std::vector<size_t>& row_indices,
    EvalContext& ctx) {
  std::vector<size_t> effective_rows = row_indices;
  if (agg.having_modifier() != nullptr) {
    auto filtered_or =
        FilterRowsByHavingModifier(agg, input_rows, row_indices, ctx);
    if (!filtered_or.ok()) return filtered_or.status();
    effective_rows = std::move(*filtered_or);
  }

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

  std::vector<MultiLevelInnerRow> inner_results;
  inner_results.reserve(inner_groups.size());
  for (auto& [fp, group] : inner_groups) {
    MultiLevelInnerRow inner;
    for (int g = 0; g < agg.group_by_list_size(); ++g) {
      const ::googlesql::ResolvedComputedColumn* gc = agg.group_by_list(g);
      inner.bindings.emplace(gc->column().column_id(),
                             group.first[static_cast<size_t>(g)]);
      inner.by_name[std::string(gc->column().name())] =
          group.first[static_cast<size_t>(g)];
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
                    agg, *inner_agg, input_scan, input_rows, group.second, ctx)
              : EvalAggregateForRows(
                    *inner_agg, input_scan, input_rows, group.second, ctx);
      if (!val.ok()) return val.status();
      inner.bindings.emplace(cc->column().column_id(), *val);
      inner.by_name[std::string(cc->column().name())] = *val;
    }

    if (agg.having_expr() != nullptr) {
      EvalContext having_ctx = ctx;
      having_ctx.columns = &inner.bindings;
      having_ctx.columns_by_name = &inner.by_name;
      auto having_or = EvalExpr(*agg.having_expr(), having_ctx);
      if (!having_or.ok()) return having_or.status();
      if (having_or->is_null() || !having_or->bool_value()) {
        continue;
      }
    }
    inner_results.push_back(std::move(inner));
  }

  std::vector<std::vector<Value>> arg_columns(
      static_cast<size_t>(agg.argument_list_size()));
  std::vector<ColumnBindings> outer_input_rows;
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

  return FinishAggregateFromArgColumns(agg, arg_columns, outer_input_rows, ctx);
}

}  // namespace scan_eval_internal
}  // namespace semantic
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
