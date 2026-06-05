#include "backend/engine/duckdb/transpiler/transpiler.h"
#include "backend/engine/duckdb/transpiler/transpiler_internal.h"

#include <algorithm>
#include <optional>
#include <string>
#include <vector>

#include "absl/container/flat_hash_map.h"
#include "absl/container/flat_hash_set.h"
#include "absl/log/log.h"
#include "absl/strings/ascii.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/str_join.h"
#include "absl/strings/str_replace.h"
#include "absl/strings/string_view.h"
#include "absl/time/time.h"
#include "backend/engine/disposition.h"
#include "backend/engine/duckdb/transpiler/functions.h"
#include "backend/engine/duckdb/transpiler/types.h"
#include "googlesql/public/catalog.h"
#include "googlesql/public/function.h"
#include "googlesql/public/options.pb.h"
#include "googlesql/public/sql_function.h"
#include "googlesql/public/templated_sql_function.h"
#include "googlesql/public/type.h"
#include "googlesql/public/type.pb.h"
#include "googlesql/public/types/struct_type.h"
#include "googlesql/public/value.h"
#include "googlesql/resolved_ast/resolved_ast.h"
#include "googlesql/resolved_ast/resolved_ast_visitor.h"
#include "googlesql/resolved_ast/resolved_node.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace duckdb {
namespace transpiler {

namespace internal {

bool ScanTreeContainsAnalytic(const ::googlesql::ResolvedScan* scan) {
  if (scan == nullptr) return false;
  switch (scan->node_kind()) {
    case ::googlesql::RESOLVED_ANALYTIC_SCAN:
      return true;
    case ::googlesql::RESOLVED_PROJECT_SCAN: {
      const auto* ps = scan->GetAs<::googlesql::ResolvedProjectScan>();
      return ScanTreeContainsAnalytic(ps->input_scan());
    }
    case ::googlesql::RESOLVED_FILTER_SCAN: {
      const auto* fs = scan->GetAs<::googlesql::ResolvedFilterScan>();
      return ScanTreeContainsAnalytic(fs->input_scan());
    }
    case ::googlesql::RESOLVED_JOIN_SCAN: {
      const auto* js = scan->GetAs<::googlesql::ResolvedJoinScan>();
      return ScanTreeContainsAnalytic(js->left_scan()) ||
             ScanTreeContainsAnalytic(js->right_scan());
    }
    case ::googlesql::RESOLVED_ARRAY_SCAN: {
      const auto* as = scan->GetAs<::googlesql::ResolvedArrayScan>();
      return ScanTreeContainsAnalytic(as->input_scan());
    }
    case ::googlesql::RESOLVED_AGGREGATE_SCAN: {
      const auto* ag = scan->GetAs<::googlesql::ResolvedAggregateScan>();
      return ScanTreeContainsAnalytic(ag->input_scan());
    }
    case ::googlesql::RESOLVED_ORDER_BY_SCAN: {
      const auto* obs = scan->GetAs<::googlesql::ResolvedOrderByScan>();
      return ScanTreeContainsAnalytic(obs->input_scan());
    }
    case ::googlesql::RESOLVED_LIMIT_OFFSET_SCAN: {
      const auto* los = scan->GetAs<::googlesql::ResolvedLimitOffsetScan>();
      return ScanTreeContainsAnalytic(los->input_scan());
    }
    case ::googlesql::RESOLVED_WITH_SCAN: {
      const auto* ws = scan->GetAs<::googlesql::ResolvedWithScan>();
      return ScanTreeContainsAnalytic(ws->query());
    }
    case ::googlesql::RESOLVED_SET_OPERATION_SCAN: {
      const auto* sos = scan->GetAs<::googlesql::ResolvedSetOperationScan>();
      for (int i = 0; i < sos->input_item_list_size(); ++i) {
        const auto* item = sos->input_item_list(i);
        if (item != nullptr && ScanTreeContainsAnalytic(item->scan())) {
          return true;
        }
      }
      return false;
    }
    case ::googlesql::RESOLVED_SAMPLE_SCAN: {
      const auto* ss = scan->GetAs<::googlesql::ResolvedSampleScan>();
      return ScanTreeContainsAnalytic(ss->input_scan());
    }
    case ::googlesql::RESOLVED_PIVOT_SCAN: {
      const auto* ps = scan->GetAs<::googlesql::ResolvedPivotScan>();
      return ScanTreeContainsAnalytic(ps->input_scan());
    }
    case ::googlesql::RESOLVED_UNPIVOT_SCAN: {
      const auto* us = scan->GetAs<::googlesql::ResolvedUnpivotScan>();
      return ScanTreeContainsAnalytic(us->input_scan());
    }
    default:
      return false;
  }
}

bool AnalyticOrderNeedsInputRn(
    const ::googlesql::ResolvedAnalyticFunctionGroup* group) {
  if (group == nullptr) return false;
  static const absl::flat_hash_set<std::string> kNavigationFns = {
      "lag", "lead", "first_value", "last_value", "nth_value"};
  for (int f = 0; f < group->analytic_function_list_size(); ++f) {
    const ::googlesql::ResolvedComputedColumnBase* col =
        group->analytic_function_list(f);
    if (col == nullptr || col->expr() == nullptr ||
        col->expr()->node_kind() !=
            ::googlesql::RESOLVED_ANALYTIC_FUNCTION_CALL) {
      continue;
    }
    const auto* afn =
        col->expr()->GetAs<::googlesql::ResolvedAnalyticFunctionCall>();
    if (afn == nullptr || afn->function() == nullptr) continue;
    if (kNavigationFns.contains(ResolveFunctionName(afn->function()))) {
      return true;
    }
  }
  return false;
}

bool AnalyticGroupNeedsInputRnForEmptyOrder(
    const ::googlesql::ResolvedAnalyticFunctionGroup* group) {
  if (group == nullptr) return false;
  const ::googlesql::ResolvedWindowOrdering* order_by = group->order_by();
  if (order_by != nullptr && order_by->order_by_item_list_size() > 0) {
    return false;
  }
  static const absl::flat_hash_set<std::string> kFns = {"row_number",
                                                        "rank",
                                                        "dense_rank",
                                                        "percent_rank",
                                                        "cume_dist",
                                                        "ntile",
                                                        "lag",
                                                        "lead",
                                                        "first_value",
                                                        "last_value",
                                                        "nth_value"};
  for (int f = 0; f < group->analytic_function_list_size(); ++f) {
    const ::googlesql::ResolvedComputedColumnBase* col =
        group->analytic_function_list(f);
    if (col == nullptr || col->expr() == nullptr ||
        col->expr()->node_kind() !=
            ::googlesql::RESOLVED_ANALYTIC_FUNCTION_CALL) {
      continue;
    }
    const auto* afn =
        col->expr()->GetAs<::googlesql::ResolvedAnalyticFunctionCall>();
    if (afn == nullptr || afn->function() == nullptr) continue;
    if (kFns.contains(ResolveFunctionName(afn->function()))) {
      return true;
    }
  }
  return false;
}

bool AnalyticGroupHasRangeFrame(
    const ::googlesql::ResolvedAnalyticFunctionGroup* group) {
  if (group == nullptr) return false;
  for (int f = 0; f < group->analytic_function_list_size(); ++f) {
    const ::googlesql::ResolvedComputedColumnBase* col =
        group->analytic_function_list(f);
    if (col == nullptr || col->expr() == nullptr ||
        col->expr()->node_kind() !=
            ::googlesql::RESOLVED_ANALYTIC_FUNCTION_CALL) {
      continue;
    }
    const auto* afn =
        col->expr()->GetAs<::googlesql::ResolvedAnalyticFunctionCall>();
    if (afn == nullptr || afn->window_frame() == nullptr) continue;
    if (afn->window_frame()->frame_unit() ==
        ::googlesql::ResolvedWindowFrame::RANGE) {
      return true;
    }
  }
  return false;
}

bool AggregateScanNeedsInputRn(const ::googlesql::ResolvedAggregateScan* node) {
  if (node == nullptr) return false;
  for (int i = 0; i < node->aggregate_list_size(); ++i) {
    const ::googlesql::ResolvedComputedColumnBase* ac = node->aggregate_list(i);
    if (ac == nullptr || ac->expr() == nullptr ||
        ac->expr()->node_kind() !=
            ::googlesql::RESOLVED_AGGREGATE_FUNCTION_CALL) {
      continue;
    }
    const auto* agg =
        ac->expr()->GetAs<::googlesql::ResolvedAggregateFunctionCall>();
    if (agg == nullptr || agg->function() == nullptr) continue;
    const std::string agg_name = ResolveFunctionName(agg->function());
    if (agg_name == "array_concat_agg") {
      return true;
    }
    if (agg_name == "string_agg" && agg->distinct()) {
      return true;
    }
    if (agg_name == "array_agg" && agg->order_by_item_list_size() == 0 &&
        agg->limit() == nullptr) {
      return true;
    }
  }
  return false;
}

}  // namespace internal

}  // namespace transpiler
}  // namespace duckdb
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
