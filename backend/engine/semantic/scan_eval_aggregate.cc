#include <map>
#include <string>
#include <utility>
#include <vector>

#include "absl/status/statusor.h"
#include "absl/strings/ascii.h"
#include "absl/strings/str_cat.h"
#include "backend/engine/semantic/error.h"
#include "backend/engine/semantic/eval_udaf.h"
#include "backend/engine/semantic/functions/specialized_funcs.h"
#include "backend/engine/semantic/scan_eval_internal.h"
#include "backend/engine/semantic/value.h"
#include "googlesql/public/function.h"
#include "googlesql/public/sql_function.h"
#include "googlesql/public/type.h"
#include "googlesql/resolved_ast/resolved_ast.h"
#include "googlesql/resolved_ast/resolved_node_kind.pb.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace semantic {
namespace scan_eval_internal {

using ::bigquery_emulator::backend::engine::semantic::EvalContext;
using ::bigquery_emulator::backend::engine::semantic::EvalExpr;

const ::googlesql::ResolvedScan* StripBarrierScans(
    const ::googlesql::ResolvedScan* scan) {
  while (scan != nullptr &&
         scan->node_kind() == ::googlesql::RESOLVED_BARRIER_SCAN) {
    scan = scan->GetAs<::googlesql::ResolvedBarrierScan>()->input_scan();
  }
  return scan;
}

std::string GroupKeyFingerprint(const std::vector<Value>& keys) {
  std::string fp;
  for (const Value& key : keys) {
    absl::StrAppend(&fp, key.DebugString(), "\x1e");
  }
  return fp;
}

absl::StatusOr<Value> EvalAggregateForRows(
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
  std::vector<std::vector<Value>> arg_columns(
      static_cast<size_t>(agg.argument_list_size()));
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
    for (int a = 0; a < agg.argument_list_size(); ++a) {
      auto v = EvalExpr(*agg.argument_list(a), row_ctx);
      if (!v.ok()) return v.status();
      arg_columns[static_cast<size_t>(a)].push_back(*std::move(v));
    }
  }
  std::string agg_name;
  if (agg.function() != nullptr) {
    agg_name = absl::AsciiStrToLower(
        agg.function()->FullName(/*include_group=*/false));
    if (agg_name.empty()) {
      agg_name = absl::AsciiStrToLower(agg.function()->Name());
    }
  }
  if (agg_name == "$count_star") {
    return Value::Int64(static_cast<int64_t>(effective_rows.size()));
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
        static_cast<const ::googlesql::SQLFunction*>(agg.function());
    return EvalSqlUdafBody(agg, *sql_fn, arg_columns, row_indices, ctx);
  }
  return functions::EvalAggregateCall(agg, arg_columns);
}

absl::StatusOr<ColumnBindings> MaterializeAggregateGroup(
    const ::googlesql::ResolvedAggregateScan& aggregate,
    const std::vector<ColumnBindings>& input_rows,
    const std::vector<size_t>& row_indices,
    const std::vector<Value>* group_keys,
    EvalContext& ctx) {
  ColumnBindings out_row;
  if (group_keys != nullptr) {
    if (static_cast<int>(group_keys->size()) !=
        aggregate.group_by_list_size()) {
      return absl::InternalError(
          "semantic: aggregate group key arity mismatch");
    }
    for (int g = 0; g < aggregate.group_by_list_size(); ++g) {
      out_row.emplace(aggregate.group_by_list(g)->column().column_id(),
                      (*group_keys)[static_cast<size_t>(g)]);
    }
  }
  for (int i = 0; i < aggregate.aggregate_list_size(); ++i) {
    const ::googlesql::ResolvedComputedColumnBase* cc =
        aggregate.aggregate_list(i);
    if (cc == nullptr || cc->expr() == nullptr) {
      return absl::InternalError("semantic: aggregate column has null expr");
    }
    const auto* agg =
        cc->expr()->GetAs<::googlesql::ResolvedAggregateFunctionCall>();
    if (agg == nullptr) {
      return MakeSemanticError(
          SemanticErrorReason::kNotImplemented,
          "semantic: aggregate expression is not a function call");
    }
    auto result = EvalAggregateForRows(
        *agg, aggregate.input_scan(), input_rows, row_indices, ctx);
    if (!result.ok()) return result.status();
    out_row.emplace(cc->column().column_id(), *std::move(result));
  }
  return out_row;
}

absl::StatusOr<std::vector<ColumnBindings>> MaterializeAggregateScan(
    const ::googlesql::ResolvedAggregateScan& aggregate, EvalContext& ctx) {
  auto input_or = MaterializeScanImpl(aggregate.input_scan(), ctx);
  if (!input_or.ok()) return input_or.status();
  const std::vector<ColumnBindings>& input_rows = *input_or;

  if (aggregate.grouping_set_list_size() > 0) {
    return MakeSemanticError(
        SemanticErrorReason::kNotImplemented,
        "semantic: GROUPING SETS aggregate scans are not implemented");
  }

  if (aggregate.group_by_list_size() == 0) {
    std::vector<size_t> all_rows;
    all_rows.reserve(input_rows.size());
    for (size_t r = 0; r < input_rows.size(); ++r) {
      all_rows.push_back(r);
    }
    auto row_or = MaterializeAggregateGroup(aggregate,
                                            input_rows,
                                            all_rows,
                                            /*group_keys=*/nullptr,
                                            ctx);
    if (!row_or.ok()) return row_or.status();
    return std::vector<ColumnBindings>{std::move(*row_or)};
  }

  std::map<std::string, std::pair<std::vector<Value>, std::vector<size_t>>>
      groups;
  for (size_t r = 0; r < input_rows.size(); ++r) {
    EvalContext row_ctx = ctx;
    row_ctx.columns = &input_rows[r];
    std::vector<Value> keys;
    keys.reserve(aggregate.group_by_list_size());
    for (int g = 0; g < aggregate.group_by_list_size(); ++g) {
      const ::googlesql::ResolvedComputedColumn* gc =
          aggregate.group_by_list(g);
      if (gc == nullptr || gc->expr() == nullptr) {
        return absl::InternalError("semantic: group by column has null expr");
      }
      auto key = EvalExpr(*gc->expr(), row_ctx);
      if (!key.ok()) return key.status();
      keys.push_back(*std::move(key));
    }
    const std::string fp = GroupKeyFingerprint(keys);
    auto it = groups.find(fp);
    if (it == groups.end()) {
      groups.emplace(fp,
                     std::make_pair(std::move(keys), std::vector<size_t>{r}));
    } else {
      it->second.second.push_back(r);
    }
  }

  std::vector<ColumnBindings> out;
  out.reserve(groups.size());
  for (const auto& kv : groups) {
    auto row_or = MaterializeAggregateGroup(
        aggregate, input_rows, kv.second.second, &kv.second.first, ctx);
    if (!row_or.ok()) return row_or.status();
    out.push_back(std::move(*row_or));
  }
  return out;
}

}  // namespace scan_eval_internal
}  // namespace semantic
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
