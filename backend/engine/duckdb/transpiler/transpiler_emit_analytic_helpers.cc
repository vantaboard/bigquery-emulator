#include "backend/engine/duckdb/transpiler/transpiler_emit_analytic_helpers.h"

#include <optional>
#include <string>

#include "absl/strings/str_cat.h"
#include "absl/strings/str_join.h"
#include "absl/strings/string_view.h"
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

}  // namespace transpiler
}  // namespace duckdb
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
