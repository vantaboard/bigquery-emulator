#include "backend/engine/coordinator/route_classifier_visitor.h"
#include "googlesql/public/type.h"
#include "googlesql/resolved_ast/resolved_ast.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace coordinator {

namespace {

bool AnalyticScanHasDateTimestampNumericRangeFrame(
    const ::googlesql::ResolvedAnalyticScan* node) {
  if (node == nullptr) return false;
  for (int g = 0; g < node->function_group_list_size(); ++g) {
    const ::googlesql::ResolvedAnalyticFunctionGroup* group =
        node->function_group_list(g);
    if (group == nullptr || group->order_by() == nullptr) continue;
    const ::googlesql::ResolvedWindowOrdering* order = group->order_by();
    if (order->order_by_item_list_size() == 0) continue;
    const ::googlesql::ResolvedOrderByItem* item = order->order_by_item_list(0);
    if (item == nullptr || item->column_ref() == nullptr ||
        item->column_ref()->type() == nullptr) {
      continue;
    }
    const ::googlesql::Type* order_type = item->column_ref()->type();
    if (order_type->kind() != ::googlesql::TYPE_DATE &&
        order_type->kind() != ::googlesql::TYPE_TIMESTAMP) {
      continue;
    }
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
      if (afn == nullptr || afn->window_frame() == nullptr ||
          afn->window_frame()->frame_unit() !=
              ::googlesql::ResolvedWindowFrame::RANGE) {
        continue;
      }
      const ::googlesql::ResolvedWindowFrame* wf = afn->window_frame();
      const auto* start = wf->start_expr();
      const auto* end = wf->end_expr();
      const bool numeric_start =
          start != nullptr &&
          (start->boundary_type() ==
               ::googlesql::ResolvedWindowFrameExpr::OFFSET_PRECEDING ||
           start->boundary_type() ==
               ::googlesql::ResolvedWindowFrameExpr::OFFSET_FOLLOWING) &&
          start->expression() != nullptr &&
          start->expression()->type() != nullptr &&
          start->expression()->type()->kind() != ::googlesql::TYPE_INTERVAL;
      const bool numeric_end =
          end != nullptr &&
          (end->boundary_type() ==
               ::googlesql::ResolvedWindowFrameExpr::OFFSET_PRECEDING ||
           end->boundary_type() ==
               ::googlesql::ResolvedWindowFrameExpr::OFFSET_FOLLOWING) &&
          end->expression() != nullptr &&
          end->expression()->type() != nullptr &&
          end->expression()->type()->kind() != ::googlesql::TYPE_INTERVAL;
      if (numeric_start || numeric_end) {
        return true;
      }
    }
  }
  return false;
}

}  // namespace

void RouteClassifierVisitor::CheckAnalyticScanDateTimestampRange(
    const ::googlesql::ResolvedAnalyticScan* node) {
  if (AnalyticScanHasDateTimestampNumericRangeFrame(node)) {
    MaybePromote(Disposition::kSemanticExecutor,
                 "ResolvedAnalyticScan(date_timestamp_range)");
  }
}

}  // namespace coordinator
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
