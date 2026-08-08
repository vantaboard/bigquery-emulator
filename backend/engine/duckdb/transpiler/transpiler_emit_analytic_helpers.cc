#include "backend/engine/duckdb/transpiler/transpiler_emit_analytic_helpers.h"

#include <string>
#include <vector>

#include "absl/container/flat_hash_map.h"
#include "absl/container/flat_hash_set.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/str_join.h"
#include "absl/types/span.h"
#include "backend/engine/duckdb/transpiler/transpiler_internal.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace duckdb {
namespace transpiler {

namespace {

std::string BuildPercentileDiscRespectNullsScalarSql(
    absl::string_view p_expr, absl::string_view col_name) {
  return absl::StrCat("NULLIF(list_extract(list(",
                      internal::QuoteIdent(internal::kBqPctCoalesceCol),
                      " ORDER BY ",
                      internal::QuoteIdent(internal::kBqPctCoalesceCol),
                      " ASC), CAST(FLOOR((",
                      p_expr,
                      ") * (COUNT(*) - 1)) + 1 AS BIGINT)), ",
                      internal::kBqPctNullSentinel,
                      ") AS ",
                      internal::QuoteIdent(col_name));
}

bool AnalyticScanIsOnlyPercentileDiscRespectNulls(
    const ::googlesql::ResolvedAnalyticScan* node) {
  if (node == nullptr || node->function_group_list_size() == 0) return false;
  for (int g = 0; g < node->function_group_list_size(); ++g) {
    const ::googlesql::ResolvedAnalyticFunctionGroup* group =
        node->function_group_list(g);
    if (group == nullptr) return false;
    if (group->partition_by() != nullptr &&
        group->partition_by()->partition_by_list_size() > 0) {
      return false;
    }
    for (int f = 0; f < group->analytic_function_list_size(); ++f) {
      const ::googlesql::ResolvedComputedColumnBase* fn_col =
          group->analytic_function_list(f);
      if (fn_col == nullptr || fn_col->expr() == nullptr ||
          fn_col->expr()->node_kind() !=
              ::googlesql::RESOLVED_ANALYTIC_FUNCTION_CALL) {
        return false;
      }
      const auto* afn =
          fn_col->expr()->GetAs<::googlesql::ResolvedAnalyticFunctionCall>();
      if (afn == nullptr || afn->function() == nullptr ||
          internal::ResolveFunctionName(afn->function()) != "percentile_disc" ||
          afn->null_handling_modifier() !=
              ::googlesql::ResolvedNonScalarFunctionCallBase::RESPECT_NULLS) {
        return false;
      }
    }
  }
  return true;
}

}  // namespace

std::optional<std::string> FindPercentileDiscSortKey(
    const ::googlesql::ResolvedAnalyticScan* node,
    const AnalyticEmitExprFn& emit_expr) {
  for (int g = 0; g < node->function_group_list_size(); ++g) {
    const ::googlesql::ResolvedAnalyticFunctionGroup* group =
        node->function_group_list(g);
    if (group == nullptr) continue;
    for (int f = 0; f < group->analytic_function_list_size(); ++f) {
      const ::googlesql::ResolvedComputedColumnBase* fn_col =
          group->analytic_function_list(f);
      if (fn_col == nullptr || fn_col->expr() == nullptr ||
          fn_col->expr()->node_kind() !=
              ::googlesql::RESOLVED_ANALYTIC_FUNCTION_CALL) {
        continue;
      }
      const auto* afn =
          fn_col->expr()->GetAs<::googlesql::ResolvedAnalyticFunctionCall>();
      if (afn == nullptr || afn->function() == nullptr ||
          internal::ResolveFunctionName(afn->function()) != "percentile_disc" ||
          afn->null_handling_modifier() !=
              ::googlesql::ResolvedNonScalarFunctionCallBase::RESPECT_NULLS ||
          afn->argument_list_size() == 0) {
        continue;
      }
      return emit_expr(afn->argument_list(0));
    }
  }
  return std::nullopt;
}

std::string TryEmitPercentileDiscOnlyScan(
    const ::googlesql::ResolvedAnalyticScan* node,
    absl::string_view input,
    const AnalyticEmitExprFn& emit_expr) {
  if (!AnalyticScanIsOnlyPercentileDiscRespectNulls(node)) return "";
  std::vector<std::string> pct_projections;
  std::vector<std::string> pct_refs;
  for (int g = 0; g < node->function_group_list_size(); ++g) {
    const ::googlesql::ResolvedAnalyticFunctionGroup* group =
        node->function_group_list(g);
    if (group == nullptr) return "";
    for (int f = 0; f < group->analytic_function_list_size(); ++f) {
      const ::googlesql::ResolvedComputedColumnBase* fn_col =
          group->analytic_function_list(f);
      if (fn_col == nullptr || fn_col->expr() == nullptr ||
          fn_col->expr()->node_kind() !=
              ::googlesql::RESOLVED_ANALYTIC_FUNCTION_CALL) {
        return "";
      }
      const auto* afn =
          fn_col->expr()->GetAs<::googlesql::ResolvedAnalyticFunctionCall>();
      if (afn == nullptr || afn->argument_list_size() < 2) return "";
      std::string p_expr = emit_expr(afn->argument_list(1));
      if (p_expr.empty()) return "";
      pct_projections.push_back(BuildPercentileDiscRespectNullsScalarSql(
          p_expr, fn_col->column().name()));
      pct_refs.push_back(
          absl::StrCat("_pct.", internal::QuoteIdent(fn_col->column().name())));
    }
  }
  if (pct_projections.empty()) return "";
  return absl::StrCat("SELECT _base.*, ",
                      absl::StrJoin(pct_refs, ", "),
                      " FROM (",
                      input,
                      ") _base CROSS JOIN (SELECT ",
                      absl::StrJoin(pct_projections, ", "),
                      " FROM (",
                      input,
                      ")) _pct");
}

std::string WrapInputWithPctCoalesce(absl::string_view input,
                                     absl::string_view sort_key) {
  return absl::StrCat("SELECT *, IF(",
                      sort_key,
                      " IS NULL, ",
                      internal::kBqPctNullSentinel,
                      ", ",
                      sort_key,
                      ") AS ",
                      internal::QuoteIdent(internal::kBqPctCoalesceCol),
                      " FROM (",
                      input,
                      ")");
}

std::string MaybeNormalizeAnalyticJoinAliases(
    const ::googlesql::ResolvedAnalyticScan* node,
    std::string sql,
    bool input_used_join_aliases,
    bool input_has_rn_column,
    std::vector<std::string>* order_items,
    absl::Span<const int> order_column_ids) {
  if (!input_used_join_aliases || node == nullptr) {
    return sql;
  }

  absl::flat_hash_set<int> analytic_ids;
  for (int g = 0; g < node->function_group_list_size(); ++g) {
    const ::googlesql::ResolvedAnalyticFunctionGroup* group =
        node->function_group_list(g);
    if (group == nullptr) continue;
    for (int f = 0; f < group->analytic_function_list_size(); ++f) {
      const ::googlesql::ResolvedComputedColumnBase* col =
          group->analytic_function_list(f);
      if (col != nullptr) {
        analytic_ids.insert(col->column().column_id());
      }
    }
  }

  std::vector<std::string> normalized;
  normalized.reserve(static_cast<size_t>(node->column_list_size()) + 1);
  absl::flat_hash_map<int, std::string> id_to_name;
  id_to_name.reserve(static_cast<size_t>(node->column_list_size()));
  for (int i = 0; i < node->column_list_size(); ++i) {
    const ::googlesql::ResolvedColumn& col = node->column_list(i);
    id_to_name[col.column_id()] = col.name();
    if (analytic_ids.contains(col.column_id())) {
      normalized.push_back(internal::QuoteIdent(col.name()));
    } else {
      normalized.push_back(
          absl::StrCat(internal::JoinColumnIdAlias(col.column_id()),
                       " AS ",
                       internal::QuoteIdent(col.name())));
    }
  }
  if (input_has_rn_column) {
    normalized.push_back(internal::QuoteIdent(internal::kBqInputRnCol));
  }
  sql = absl::StrCat(
      "SELECT ", absl::StrJoin(normalized, ", "), " FROM (", sql, ")");

  if (order_items != nullptr) {
    for (size_t i = 0; i < order_items->size(); ++i) {
      const int col_id = i < order_column_ids.size() ? order_column_ids[i] : -1;
      if (col_id < 0) continue;
      auto it = id_to_name.find(col_id);
      if (it == id_to_name.end()) continue;
      const std::string leading =
          internal::OrderItemLeadingColumn((*order_items)[i]);
      if (leading.empty()) continue;
      (*order_items)[i] =
          absl::StrCat(internal::QuoteIdent(it->second),
                       (*order_items)[i].substr(leading.size()));
    }
  }
  return sql;
}

}  // namespace transpiler
}  // namespace duckdb
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
