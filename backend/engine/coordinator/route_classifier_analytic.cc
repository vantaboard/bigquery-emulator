#include "backend/engine/coordinator/route_classifier_visitor.h"
#include "googlesql/public/type.h"
#include "googlesql/resolved_ast/resolved_ast.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace coordinator {

namespace {

bool IsDateOrTimestampOrderType(const ::googlesql::ResolvedOrderByItem* item) {
  if (item == nullptr || item->column_ref() == nullptr ||
      item->column_ref()->type() == nullptr) {
    return false;
  }
  const ::googlesql::TypeKind kind = item->column_ref()->type()->kind();
  return kind == ::googlesql::TYPE_DATE || kind == ::googlesql::TYPE_TIMESTAMP;
}

const ::googlesql::ResolvedAnalyticFunctionCall* AsRangeAnalyticCall(
    const ::googlesql::ResolvedComputedColumnBase* col) {
  if (col == nullptr || col->expr() == nullptr ||
      col->expr()->node_kind() !=
          ::googlesql::RESOLVED_ANALYTIC_FUNCTION_CALL) {
    return nullptr;
  }
  const auto* call =
      col->expr()->GetAs<::googlesql::ResolvedAnalyticFunctionCall>();
  if (call == nullptr || call->window_frame() == nullptr ||
      call->window_frame()->frame_unit() !=
          ::googlesql::ResolvedWindowFrame::RANGE) {
    return nullptr;
  }
  return call;
}

bool IsNumericOffsetBoundary(
    const ::googlesql::ResolvedWindowFrameExpr* boundary) {
  if (boundary == nullptr) return false;
  const ::googlesql::ResolvedWindowFrameExpr::BoundaryType type =
      boundary->boundary_type();
  const bool is_offset =
      type == ::googlesql::ResolvedWindowFrameExpr::OFFSET_PRECEDING ||
      type == ::googlesql::ResolvedWindowFrameExpr::OFFSET_FOLLOWING;
  if (!is_offset || boundary->expression() == nullptr ||
      boundary->expression()->type() == nullptr) {
    return false;
  }
  return boundary->expression()->type()->kind() != ::googlesql::TYPE_INTERVAL;
}

bool GroupHasNumericRangeFrame(
    const ::googlesql::ResolvedAnalyticFunctionGroup* group) {
  for (int f = 0; f < group->analytic_function_list_size(); ++f) {
    const auto* call = AsRangeAnalyticCall(group->analytic_function_list(f));
    if (call == nullptr) continue;
    const ::googlesql::ResolvedWindowFrame* frame = call->window_frame();
    if (IsNumericOffsetBoundary(frame->start_expr()) ||
        IsNumericOffsetBoundary(frame->end_expr())) {
      return true;
    }
  }
  return false;
}

bool AnalyticScanHasDateTimestampNumericRangeFrame(
    const ::googlesql::ResolvedAnalyticScan* node) {
  if (node == nullptr) return false;
  for (int g = 0; g < node->function_group_list_size(); ++g) {
    const ::googlesql::ResolvedAnalyticFunctionGroup* group =
        node->function_group_list(g);
    if (group == nullptr || group->order_by() == nullptr) continue;
    const ::googlesql::ResolvedWindowOrdering* order = group->order_by();
    if (order->order_by_item_list_size() == 0) continue;
    if (!IsDateOrTimestampOrderType(order->order_by_item_list(0))) continue;
    if (GroupHasNumericRangeFrame(group)) return true;
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
