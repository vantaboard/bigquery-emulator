#include <map>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "absl/container/flat_hash_map.h"
#include "absl/status/statusor.h"
#include "absl/strings/ascii.h"
#include "absl/strings/str_cat.h"
#include "backend/engine/semantic/error.h"
#include "backend/engine/semantic/eval_expr.h"
#include "backend/engine/semantic/scan_eval_internal.h"
#include "backend/engine/semantic/value.h"
#include "googlesql/public/functions/match_recognize/compiled_pattern.h"
#include "googlesql/public/functions/match_recognize/match_partition.h"
#include "googlesql/resolved_ast/resolved_ast.h"
#include "googlesql/resolved_ast/resolved_ast_enums.pb.h"
#include "googlesql/resolved_ast/resolved_node_kind.pb.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace semantic {
namespace scan_eval_internal {

namespace {

using ::bigquery_emulator::backend::engine::semantic::EvalContext;
using ::bigquery_emulator::backend::engine::semantic::EvalExpr;
using ::googlesql::functions::match_recognize::CompiledPattern;
using ::googlesql::functions::match_recognize::Match;
using ::googlesql::functions::match_recognize::MatchOptions;
using ::googlesql::functions::match_recognize::MatchPartition;
using ::googlesql::functions::match_recognize::MatchResult;
using ::googlesql::functions::match_recognize::PatternOptions;

const ::googlesql::ResolvedAnalyticFunctionGroup* MainAnalyticGroup(
    const ::googlesql::ResolvedMatchRecognizeScan& scan) {
  if (scan.analytic_function_group_list_size() == 0) {
    return nullptr;
  }
  return scan.analytic_function_group_list(0);
}

absl::Status CheckUnsupportedMatchRecognizeOptions(
    const ::googlesql::ResolvedMatchRecognizeScan& scan) {
  for (int i = 0; i < scan.option_list_size(); ++i) {
    const ::googlesql::ResolvedOption* option = scan.option_list(i);
    if (option == nullptr || option->value() == nullptr) continue;
    if (absl::AsciiStrToLower(option->name()) != "use_longest_match") {
      return MakeSemanticError(SemanticErrorReason::kNotImplemented,
                               absl::StrCat("semantic: MATCH_RECOGNIZE option ",
                                            option->name(),
                                            " is not implemented"));
    }
    if (option->value()->node_kind() != ::googlesql::RESOLVED_LITERAL) {
      return MakeSemanticError(
          SemanticErrorReason::kNotImplemented,
          "semantic: MATCH_RECOGNIZE use_longest_match with query "
          "parameters is not implemented");
    }
    const auto* literal =
        option->value()->GetAs<::googlesql::ResolvedLiteral>();
    if (literal != nullptr && !literal->value().is_null() &&
        literal->value().bool_value()) {
      return MakeSemanticError(
          SemanticErrorReason::kNotImplemented,
          "semantic: MATCH_RECOGNIZE use_longest_match=TRUE is not "
          "implemented");
    }
  }
  if (scan.after_match_skip_mode() == ::googlesql::ResolvedMatchRecognizeScan::
                                          AFTER_MATCH_SKIP_MODE_UNSPECIFIED) {
    return absl::OkStatus();
  }
  if (scan.after_match_skip_mode() ==
      ::googlesql::ResolvedMatchRecognizeScan::NEXT_ROW) {
    return MakeSemanticError(
        SemanticErrorReason::kNotImplemented,
        "semantic: MATCH_RECOGNIZE AFTER MATCH SKIP TO NEXT ROW is not "
        "implemented");
  }
  return absl::OkStatus();
}

absl::StatusOr<std::vector<Value>> EvalPartitionKeys(
    const ::googlesql::ResolvedWindowPartitioning* partition_by,
    const ColumnBindings& row,
    const EvalContext& ctx) {
  std::vector<Value> keys;
  if (partition_by == nullptr) {
    return keys;
  }
  keys.reserve(partition_by->partition_by_list_size());
  for (int i = 0; i < partition_by->partition_by_list_size(); ++i) {
    const ::googlesql::ResolvedColumnRef* col_ref =
        partition_by->partition_by_list(i);
    if (col_ref == nullptr) {
      return absl::InternalError(
          "semantic: MATCH_RECOGNIZE partition_by_list has null ref");
    }
    const int col_id = col_ref->column().column_id();
    auto it = row.find(col_id);
    if (it == row.end()) {
      return MakeSemanticError(SemanticErrorReason::kInvalidArgument,
                               "semantic: MATCH_RECOGNIZE partition column "
                               "missing from input row");
    }
    keys.push_back(it->second);
  }
  return keys;
}

void SortRowsByOrderBy(const ::googlesql::ResolvedWindowOrdering* order_by,
                       std::vector<ColumnBindings>* rows) {
  if (order_by == nullptr || rows == nullptr) return;
  std::stable_sort(
      rows->begin(),
      rows->end(),
      [&](const ColumnBindings& a, const ColumnBindings& b) {
        for (int i = 0; i < order_by->order_by_item_list_size(); ++i) {
          const ::googlesql::ResolvedOrderByItem* item =
              order_by->order_by_item_list(i);
          if (item == nullptr || item->column_ref() == nullptr) {
            continue;
          }
          const int col_id = item->column_ref()->column().column_id();
          auto av = a.find(col_id);
          auto bv = b.find(col_id);
          Value va = av == a.end() ? Value() : av->second;
          Value vb = bv == b.end() ? Value() : bv->second;
          if (ValueEqual(va, vb)) continue;
          if (va.is_null() || vb.is_null()) {
            bool nulls_first;
            switch (item->null_order()) {
              case ::googlesql::ResolvedOrderByItem::NULLS_FIRST:
                nulls_first = true;
                break;
              case ::googlesql::ResolvedOrderByItem::NULLS_LAST:
                nulls_first = false;
                break;
              default:
                nulls_first = !item->is_descending();
                break;
            }
            if (va.is_null() && vb.is_null()) continue;
            if (va.is_null()) return nulls_first;
            return !nulls_first;
          }
          bool less = ValueLess(va, vb);
          if (item->is_descending()) less = !less;
          return less;
        }
        return false;
      });
}

absl::StatusOr<std::vector<bool>> EvalPatternVariableRow(
    const ::googlesql::ResolvedMatchRecognizeScan& scan,
    const ColumnBindings& row,
    EvalContext& ctx) {
  std::vector<bool> predicate_vals;
  predicate_vals.reserve(scan.pattern_variable_definition_list_size());
  absl::flat_hash_map<std::string, ::googlesql::Value> by_name;
  EvalContext row_ctx = ctx;
  row_ctx.columns = &row;
  PopulateColumnNameBindings(scan.input_scan(), row, by_name);
  if (ctx.columns_by_name != nullptr) {
    for (const auto& [name, val] : *ctx.columns_by_name) {
      by_name[name] = val;
    }
  }
  row_ctx.columns_by_name = &by_name;
  for (int i = 0; i < scan.pattern_variable_definition_list_size(); ++i) {
    const ::googlesql::ResolvedMatchRecognizeVariableDefinition* def =
        scan.pattern_variable_definition_list(i);
    if (def == nullptr || def->predicate() == nullptr) {
      return absl::InternalError(
          "semantic: MATCH_RECOGNIZE DEFINE entry has null predicate");
    }
    auto ok = EvalBoolExpr(def->predicate(), row_ctx);
    if (!ok.ok()) return ok.status();
    predicate_vals.push_back(*ok);
  }
  return predicate_vals;
}

struct MatchRowContext {
  ColumnBindings row;
  int match_id = 0;
  int match_row_number = 0;
  std::string classifier;
};

absl::StatusOr<std::vector<MatchRowContext>> BuildMatchRowContexts(
    const ::googlesql::ResolvedMatchRecognizeScan& scan,
    const std::vector<ColumnBindings>& partition_rows,
    const Match& match) {
  std::vector<MatchRowContext> contexts;
  contexts.reserve(match.pattern_vars_by_row.size());
  for (size_t offset = 0; offset < match.pattern_vars_by_row.size(); ++offset) {
    const int row_index = match.start_row_index + static_cast<int>(offset);
    if (row_index < 0 ||
        static_cast<size_t>(row_index) >= partition_rows.size()) {
      return absl::InternalError(
          "semantic: MATCH_RECOGNIZE match row index out of range");
    }
    const int var_index = match.pattern_vars_by_row[offset];
    if (var_index < 0 ||
        var_index >= scan.pattern_variable_definition_list_size()) {
      return absl::InternalError(
          "semantic: MATCH_RECOGNIZE pattern variable index out of range");
    }
    const ::googlesql::ResolvedMatchRecognizeVariableDefinition* def =
        scan.pattern_variable_definition_list(var_index);
    MatchRowContext context;
    context.row = partition_rows[static_cast<size_t>(row_index)];
    context.match_id = match.match_id;
    context.match_row_number = static_cast<int>(offset) + 1;
    context.classifier = def != nullptr ? std::string(def->name()) : "";
    contexts.push_back(std::move(context));
  }
  return contexts;
}

absl::StatusOr<std::vector<size_t>> FilterRowsForMeasureGroup(
    const ::googlesql::ResolvedMeasureGroup& group,
    const std::vector<MatchRowContext>& contexts) {
  std::vector<size_t> indices;
  indices.reserve(contexts.size());
  const ::googlesql::ResolvedMatchRecognizePatternVariableRef* symbol_ref =
      group.pattern_variable_ref();
  for (size_t i = 0; i < contexts.size(); ++i) {
    if (symbol_ref != nullptr && contexts[i].classifier != symbol_ref->name()) {
      continue;
    }
    indices.push_back(i);
  }
  return indices;
}

absl::StatusOr<ColumnBindings> EvalMatchMeasures(
    const ::googlesql::ResolvedMatchRecognizeScan& scan,
    const std::vector<ColumnBindings>& partition_rows,
    const std::vector<MatchRowContext>& contexts,
    const EvalContext& ctx) {
  ColumnBindings out_row;
  const int match_number_id = scan.match_number_column().column_id();
  const int match_row_number_id = scan.match_row_number_column().column_id();
  const int classifier_id = scan.classifier_column().column_id();

  std::vector<ColumnBindings> augmented_rows;
  augmented_rows.reserve(contexts.size());
  for (const MatchRowContext& context : contexts) {
    ColumnBindings augmented = context.row;
    augmented[match_number_id] = Value::Int64(context.match_id);
    augmented[match_row_number_id] = Value::Int64(context.match_row_number);
    augmented[classifier_id] = Value::String(context.classifier);
    augmented_rows.push_back(std::move(augmented));
  }

  for (int g = 0; g < scan.measure_group_list_size(); ++g) {
    const ::googlesql::ResolvedMeasureGroup* group = scan.measure_group_list(g);
    if (group == nullptr) {
      return absl::InternalError(
          "semantic: MATCH_RECOGNIZE measure_group_list has null entry");
    }
    auto indices_or = FilterRowsForMeasureGroup(*group, contexts);
    if (!indices_or.ok()) return indices_or.status();
    const std::vector<size_t>& indices = *indices_or;

    for (int a = 0; a < group->aggregate_list_size(); ++a) {
      const ::googlesql::ResolvedComputedColumnBase* cc =
          group->aggregate_list(a);
      if (cc == nullptr || cc->expr() == nullptr) {
        return absl::InternalError(
            "semantic: MATCH_RECOGNIZE measure aggregate has null expr");
      }
      const auto* agg =
          cc->expr()->GetAs<::googlesql::ResolvedAggregateFunctionCall>();
      if (agg == nullptr) {
        return MakeSemanticError(
            SemanticErrorReason::kNotImplemented,
            "semantic: MATCH_RECOGNIZE measure is not an aggregate call");
      }
      std::vector<ColumnBindings> agg_input_rows;
      agg_input_rows.reserve(indices.size());
      for (size_t idx : indices) {
        agg_input_rows.push_back(augmented_rows[idx]);
      }
      std::vector<size_t> all_indices;
      all_indices.reserve(agg_input_rows.size());
      for (size_t i = 0; i < agg_input_rows.size(); ++i) {
        all_indices.push_back(i);
      }
      EvalContext agg_ctx = ctx;
      auto agg_value = EvalAggregateForRows(
          *agg, scan.input_scan(), agg_input_rows, all_indices, agg_ctx);
      if (!agg_value.ok()) return agg_value.status();
      out_row.emplace(cc->column().column_id(), *std::move(agg_value));
    }
  }

  for (int i = 0; i < scan.column_list_size(); ++i) {
    const ::googlesql::ResolvedColumn& col = scan.column_list(i);
    const int col_id = col.column_id();
    if (out_row.contains(col_id)) {
      continue;
    }
    if (contexts.empty()) {
      return absl::InternalError(
          "semantic: MATCH_RECOGNIZE match has no rows for output");
    }
    auto it = contexts.front().row.find(col_id);
    if (it == contexts.front().row.end()) {
      return MakeSemanticError(
          SemanticErrorReason::kInvalidArgument,
          absl::StrCat("semantic: MATCH_RECOGNIZE output column ",
                       col.name(),
                       " missing from match row"));
    }
    out_row.emplace(col_id, it->second);
  }
  return out_row;
}

absl::StatusOr<std::vector<Match>> RunPartitionMatching(
    const ::googlesql::ResolvedMatchRecognizeScan& scan,
    const std::vector<ColumnBindings>& partition_rows,
    EvalContext& ctx) {
  auto pattern_or = CompiledPattern::Create(scan, PatternOptions{});
  if (!pattern_or.ok()) {
    return pattern_or.status();
  }
  MatchOptions match_options;
  auto partition_or = (*pattern_or)->CreateMatchPartition(match_options);
  if (!partition_or.ok()) return partition_or.status();
  std::unique_ptr<MatchPartition> matcher = std::move(*partition_or);

  std::vector<Match> matches;
  for (size_t row_index = 0; row_index < partition_rows.size(); ++row_index) {
    auto predicate_or =
        EvalPatternVariableRow(scan, partition_rows[row_index], ctx);
    if (!predicate_or.ok()) return predicate_or.status();
    auto result_or = matcher->AddRow(*predicate_or);
    if (!result_or.ok()) return result_or.status();
    matches.insert(matches.end(),
                   result_or->new_matches.begin(),
                   result_or->new_matches.end());
  }

  auto finalize_or = matcher->Finalize();
  if (!finalize_or.ok()) return finalize_or.status();
  matches.insert(matches.end(),
                 finalize_or->new_matches.begin(),
                 finalize_or->new_matches.end());
  return matches;
}

}  // namespace

absl::StatusOr<std::vector<ColumnBindings>> MaterializeMatchRecognizeScan(
    const ::googlesql::ResolvedMatchRecognizeScan& scan, EvalContext& ctx) {
  auto options_status = CheckUnsupportedMatchRecognizeOptions(scan);
  if (!options_status.ok()) return options_status;

  auto input_or = MaterializeScanImpl(scan.input_scan(), ctx);
  if (!input_or.ok()) return input_or.status();
  std::vector<ColumnBindings> rows = *std::move(input_or);
  if (rows.empty()) {
    return std::vector<ColumnBindings>{};
  }

  const ::googlesql::ResolvedAnalyticFunctionGroup* analytic_group =
      MainAnalyticGroup(scan);
  const ::googlesql::ResolvedWindowPartitioning* partition_by =
      analytic_group != nullptr ? analytic_group->partition_by() : nullptr;
  const ::googlesql::ResolvedWindowOrdering* order_by =
      analytic_group != nullptr ? analytic_group->order_by() : nullptr;

  std::map<std::string, std::vector<size_t>> partitions;
  for (size_t row_index = 0; row_index < rows.size(); ++row_index) {
    auto keys_or = EvalPartitionKeys(partition_by, rows[row_index], ctx);
    if (!keys_or.ok()) return keys_or.status();
    partitions[GroupKeyFingerprint(*keys_or)].push_back(row_index);
  }

  std::vector<ColumnBindings> output;
  for (auto& [_, row_indices] : partitions) {
    std::vector<ColumnBindings> partition_rows;
    partition_rows.reserve(row_indices.size());
    for (size_t row_index : row_indices) {
      partition_rows.push_back(rows[row_index]);
    }
    SortRowsByOrderBy(order_by, &partition_rows);

    auto matches_or = RunPartitionMatching(scan, partition_rows, ctx);
    if (!matches_or.ok()) return matches_or.status();

    for (const Match& match : *matches_or) {
      auto contexts_or = BuildMatchRowContexts(scan, partition_rows, match);
      if (!contexts_or.ok()) return contexts_or.status();
      if (contexts_or->empty()) {
        continue;
      }
      auto out_row_or =
          EvalMatchMeasures(scan, partition_rows, *contexts_or, ctx);
      if (!out_row_or.ok()) return out_row_or.status();
      output.push_back(std::move(*out_row_or));
    }
  }
  return output;
}

}  // namespace scan_eval_internal
}  // namespace semantic
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
