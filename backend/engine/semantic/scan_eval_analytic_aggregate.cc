#include <algorithm>
#include <cstdint>
#include <string>
#include <utility>
#include <vector>

#include "absl/container/flat_hash_map.h"
#include "absl/status/statusor.h"
#include "absl/strings/ascii.h"
#include "absl/strings/str_cat.h"
#include "backend/engine/semantic/error.h"
#include "backend/engine/semantic/eval_expr.h"
#include "backend/engine/semantic/functions/specialized_funcs.h"
#include "backend/engine/semantic/scan_eval_internal.h"
#include "backend/engine/semantic/value.h"
#include "googlesql/public/type.h"
#include "googlesql/resolved_ast/resolved_ast.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace semantic {
namespace scan_eval_internal {
namespace {

using PartitionOrder =
    absl::flat_hash_map<std::string, std::vector<size_t>>;

PartitionOrder BuildPartitionOrder(const AnalyticGroupLayout& layout) {
  PartitionOrder by_fp;
  // Dense row_numbers are 1-based within each partition after the layout
  // sort; collect indices into ascending rn order per partition.
  std::vector<size_t> order(layout.row_numbers.size());
  for (size_t i = 0; i < order.size(); ++i) order[i] = i;
  std::stable_sort(order.begin(),
                   order.end(),
                   [&](size_t a, size_t b) {
                     if (layout.partition_fps[a] != layout.partition_fps[b]) {
                       return layout.partition_fps[a] < layout.partition_fps[b];
                     }
                     return layout.row_numbers[a] < layout.row_numbers[b];
                   });
  for (size_t idx : order) {
    by_fp[layout.partition_fps[idx]].push_back(idx);
  }
  return by_fp;
}

absl::StatusOr<int64_t> EvalFrameOffsetInt64(
    const ::googlesql::ResolvedExpr* expr, EvalContext& ctx) {
  if (expr == nullptr) {
    return absl::InvalidArgumentError("semantic: frame offset missing");
  }
  auto value_or = EvalExpr(*expr, ctx);
  if (!value_or.ok()) return value_or.status();
  if (value_or->is_null()) {
    return MakeSemanticError(SemanticErrorReason::kInvalidArgument,
                             "semantic: frame offset must not be NULL");
  }
  if (value_or->type_kind() == ::googlesql::TYPE_INT64) {
    return value_or->int64_value();
  }
  if (value_or->type_kind() == ::googlesql::TYPE_DOUBLE) {
    return static_cast<int64_t>(value_or->double_value());
  }
  return MakeSemanticError(SemanticErrorReason::kInvalidArgument,
                           "semantic: frame offset must be INT64");
}

bool BoundIsUnbounded(const ::googlesql::ResolvedWindowFrameExpr* bound) {
  if (bound == nullptr) return false;
  return bound->boundary_type() ==
             ::googlesql::ResolvedWindowFrameExpr::UNBOUNDED_PRECEDING ||
         bound->boundary_type() ==
             ::googlesql::ResolvedWindowFrameExpr::UNBOUNDED_FOLLOWING;
}

bool BoundIsCurrentRow(const ::googlesql::ResolvedWindowFrameExpr* bound) {
  return bound != nullptr &&
         bound->boundary_type() ==
             ::googlesql::ResolvedWindowFrameExpr::CURRENT_ROW;
}

bool BoundIsOffset(const ::googlesql::ResolvedWindowFrameExpr* bound) {
  if (bound == nullptr) return false;
  return bound->boundary_type() ==
             ::googlesql::ResolvedWindowFrameExpr::OFFSET_PRECEDING ||
         bound->boundary_type() ==
             ::googlesql::ResolvedWindowFrameExpr::OFFSET_FOLLOWING;
}

// Expand CURRENT ROW endpoints to the peer group (same ORDER BY keys).
size_t PeerStartIndex(const std::vector<size_t>& ordered,
                      size_t cur_pos,
                      const ::googlesql::ResolvedWindowOrdering* order_spec,
                      const std::vector<ColumnBindings>& input_rows) {
  size_t start = cur_pos;
  const ColumnBindings& cur = input_rows[ordered[cur_pos]];
  while (start > 0 &&
         OrderKeysEqual(order_spec, input_rows[ordered[start - 1]], cur)) {
    --start;
  }
  return start;
}

size_t PeerEndIndex(const std::vector<size_t>& ordered,
                    size_t cur_pos,
                    const ::googlesql::ResolvedWindowOrdering* order_spec,
                    const std::vector<ColumnBindings>& input_rows) {
  size_t end = cur_pos;
  const ColumnBindings& cur = input_rows[ordered[cur_pos]];
  while (end + 1 < ordered.size() &&
         OrderKeysEqual(order_spec, input_rows[ordered[end + 1]], cur)) {
    ++end;
  }
  return end;
}

absl::StatusOr<int64_t> RowsBoundRn(
    const ::googlesql::ResolvedWindowFrameExpr* bound,
    int64_t current_rn,
    int64_t max_rn,
    bool is_start,
    EvalContext& ctx) {
  if (bound == nullptr) {
    return absl::InvalidArgumentError("semantic: ROWS frame bound missing");
  }
  switch (bound->boundary_type()) {
    case ::googlesql::ResolvedWindowFrameExpr::UNBOUNDED_PRECEDING:
      return int64_t{1};
    case ::googlesql::ResolvedWindowFrameExpr::UNBOUNDED_FOLLOWING:
      return max_rn;
    case ::googlesql::ResolvedWindowFrameExpr::CURRENT_ROW:
      return current_rn;
    case ::googlesql::ResolvedWindowFrameExpr::OFFSET_PRECEDING: {
      auto offset_or = EvalFrameOffsetInt64(bound->expression(), ctx);
      if (!offset_or.ok()) return offset_or.status();
      const int64_t rn = current_rn - *offset_or;
      return rn < 1 ? int64_t{1} : rn;
    }
    case ::googlesql::ResolvedWindowFrameExpr::OFFSET_FOLLOWING: {
      auto offset_or = EvalFrameOffsetInt64(bound->expression(), ctx);
      if (!offset_or.ok()) return offset_or.status();
      const int64_t rn = current_rn + *offset_or;
      return rn > max_rn ? max_rn : rn;
    }
    default:
      return MakeSemanticError(
          SemanticErrorReason::kNotImplemented,
          absl::StrCat("semantic: unsupported ROWS ",
                       is_start ? "start" : "end",
                       " bound"));
  }
}

absl::StatusOr<std::vector<size_t>> ResolveRowsFrame(
    const ::googlesql::ResolvedWindowFrame* wf,
    const AnalyticGroupLayout& layout,
    const std::vector<size_t>& ordered,
    size_t cur_pos,
    EvalContext& ctx) {
  const int64_t current_rn = layout.row_numbers[ordered[cur_pos]];
  const int64_t max_rn = static_cast<int64_t>(ordered.size());
  auto start_or =
      RowsBoundRn(wf->start_expr(), current_rn, max_rn, /*is_start=*/true, ctx);
  if (!start_or.ok()) return start_or.status();
  auto end_or =
      RowsBoundRn(wf->end_expr(), current_rn, max_rn, /*is_start=*/false, ctx);
  if (!end_or.ok()) return end_or.status();
  std::vector<size_t> out;
  for (size_t idx : ordered) {
    const int64_t rn = layout.row_numbers[idx];
    if (rn >= *start_or && rn <= *end_or) out.push_back(idx);
  }
  return out;
}

absl::StatusOr<std::vector<size_t>> ResolveRangeValueOffsetFrame(
    const ::googlesql::ResolvedAnalyticFunctionCall& afn,
    const ::googlesql::ResolvedWindowOrdering* order_spec,
    const std::vector<ColumnBindings>& input_rows,
    const std::vector<size_t>& ordered,
    size_t cur_pos,
    const EvalContext& ctx) {
  const ::googlesql::ResolvedWindowFrame* wf = afn.window_frame();
  if (order_spec == nullptr || order_spec->order_by_item_list_size() == 0) {
    return MakeSemanticError(
        SemanticErrorReason::kNotImplemented,
        "semantic: RANGE value offset requires ORDER BY");
  }
  const ::googlesql::ResolvedOrderByItem* order_item =
      order_spec->order_by_item_list(0);
  if (order_item == nullptr || order_item->column_ref() == nullptr) {
    return MakeSemanticError(
        SemanticErrorReason::kNotImplemented,
        "semantic: RANGE value offset missing order key");
  }
  const int order_col_id = order_item->column_ref()->column().column_id();
  const ::googlesql::Type* order_type = order_item->column_ref()->type();
  if (order_type == nullptr ||
      (order_type->kind() != ::googlesql::TYPE_INT64 &&
       order_type->kind() != ::googlesql::TYPE_DOUBLE &&
       order_type->kind() != ::googlesql::TYPE_DATE &&
       order_type->kind() != ::googlesql::TYPE_TIMESTAMP)) {
    return MakeSemanticError(
        SemanticErrorReason::kNotImplemented,
        "semantic: RANGE value offset requires numeric/DATE/TIMESTAMP order");
  }
  const size_t row_index = ordered[cur_pos];
  const Value current_order =
      LookupColumnValue(input_rows[row_index], order_col_id);
  EvalContext row_ctx = ctx;
  row_ctx.columns = &input_rows[row_index];
  auto low_or = FrameBoundValue(wf->start_expr(), current_order, row_ctx);
  if (!low_or.ok()) return low_or.status();
  auto high_or = FrameBoundValue(wf->end_expr(), current_order, row_ctx);
  if (!high_or.ok()) return high_or.status();
  const bool has_low = !low_or->is_null();
  const bool has_high = !high_or->is_null();
  std::vector<size_t> out;
  for (size_t other : ordered) {
    const Value other_order =
        LookupColumnValue(input_rows[other], order_col_id);
    if (ValueInClosedRange(
            other_order, *low_or, has_low, *high_or, has_high)) {
      out.push_back(other);
    }
  }
  return out;
}

// Peer-aware RANGE (or default) frame: UNBOUNDED / CURRENT ROW endpoints
// only. Value offsets are handled by ResolveRangeValueOffsetFrame.
absl::StatusOr<std::vector<size_t>> ResolvePeerRangeFrame(
    const ::googlesql::ResolvedWindowFrame* wf,
    const ::googlesql::ResolvedWindowOrdering* order_spec,
    const std::vector<ColumnBindings>& input_rows,
    const std::vector<size_t>& ordered,
    size_t cur_pos) {
  // Default frame when wf is null: UNBOUNDED PRECEDING .. CURRENT ROW.
  const bool start_unbounded =
      wf == nullptr || BoundIsUnbounded(wf->start_expr()) ||
      (wf->start_expr() != nullptr &&
       wf->start_expr()->boundary_type() ==
           ::googlesql::ResolvedWindowFrameExpr::UNBOUNDED_PRECEDING);
  const bool end_unbounded =
      wf != nullptr && BoundIsUnbounded(wf->end_expr()) &&
      wf->end_expr()->boundary_type() ==
          ::googlesql::ResolvedWindowFrameExpr::UNBOUNDED_FOLLOWING;
  const bool start_current =
      wf != nullptr && BoundIsCurrentRow(wf->start_expr());
  const bool end_current =
      wf == nullptr || BoundIsCurrentRow(wf->end_expr());

  if (wf != nullptr &&
      (BoundIsOffset(wf->start_expr()) || BoundIsOffset(wf->end_expr()))) {
    return MakeSemanticError(
        SemanticErrorReason::kInvalidArgument,
        "semantic: peer RANGE resolver received value-offset bounds");
  }

  size_t start = 0;
  size_t end = ordered.size() - 1;
  if (!start_unbounded) {
    if (start_current) {
      start = PeerStartIndex(ordered, cur_pos, order_spec, input_rows);
    } else {
      return MakeSemanticError(
          SemanticErrorReason::kNotImplemented,
          "semantic: unsupported RANGE start bound");
    }
  }
  if (!end_unbounded) {
    if (end_current) {
      end = PeerEndIndex(ordered, cur_pos, order_spec, input_rows);
    } else {
      return MakeSemanticError(
          SemanticErrorReason::kNotImplemented,
          "semantic: unsupported RANGE end bound");
    }
  }
  if (start > end) return std::vector<size_t>{};
  return std::vector<size_t>(ordered.begin() + start,
                             ordered.begin() + end + 1);
}

absl::StatusOr<std::vector<size_t>> ResolveWindowFrameRows(
    const ::googlesql::ResolvedAnalyticFunctionCall& afn,
    const ::googlesql::ResolvedWindowOrdering* order_spec,
    const AnalyticGroupLayout& layout,
    const std::vector<ColumnBindings>& input_rows,
    const PartitionOrder& partition_order,
    size_t row_index,
    const EvalContext& ctx) {
  const auto it = partition_order.find(layout.partition_fps[row_index]);
  if (it == partition_order.end() || it->second.empty()) {
    return std::vector<size_t>{};
  }
  const std::vector<size_t>& ordered = it->second;
  size_t cur_pos = 0;
  for (size_t i = 0; i < ordered.size(); ++i) {
    if (ordered[i] == row_index) {
      cur_pos = i;
      break;
    }
  }

  // No ORDER BY → entire partition (BigQuery treats the frame as the
  // whole partition regardless of an explicit frame clause).
  if (order_spec == nullptr || order_spec->order_by_item_list_size() == 0) {
    return ordered;
  }

  const ::googlesql::ResolvedWindowFrame* wf = afn.window_frame();
  if (wf != nullptr &&
      wf->frame_unit() == ::googlesql::ResolvedWindowFrame::ROWS) {
    EvalContext row_ctx = ctx;
    row_ctx.columns = &input_rows[row_index];
    return ResolveRowsFrame(wf, layout, ordered, cur_pos, row_ctx);
  }

  // Default (null frame) or RANGE.
  if (wf != nullptr &&
      (BoundIsOffset(wf->start_expr()) || BoundIsOffset(wf->end_expr()))) {
    return ResolveRangeValueOffsetFrame(
        afn, order_spec, input_rows, ordered, cur_pos, ctx);
  }
  if (wf != nullptr && wf->start_expr() != nullptr &&
      BoundIsUnbounded(wf->start_expr()) && BoundIsUnbounded(wf->end_expr())) {
    return ordered;
  }
  return ResolvePeerRangeFrame(
      wf, order_spec, input_rows, ordered, cur_pos);
}

}  // namespace

absl::Status ApplyAnalyticAggregate(
    const ::googlesql::ResolvedAnalyticFunctionCall& afn,
    absl::string_view fname,
    const ::googlesql::ResolvedWindowOrdering* order_spec,
    const AnalyticGroupLayout& layout,
    const std::vector<ColumnBindings>& input_rows,
    int out_col_id,
    const EvalContext& ctx,
    std::vector<ColumnBindings>& out_rows) {
  const bool is_count_star =
      fname == "$count_star" ||
      (fname == "count" && afn.argument_list_size() == 0);
  const bool is_count_expr = fname == "count" && afn.argument_list_size() == 1;
  if (!is_count_star && !is_count_expr && fname != "sum" && fname != "avg" &&
      fname != "min" && fname != "max") {
    return MakeSemanticError(
        SemanticErrorReason::kNotImplemented,
        absl::StrCat(
            "semantic: analytic aggregate '", fname, "' is not implemented"));
  }
  if (!is_count_star &&
      (afn.argument_list_size() != 1 || afn.argument_list(0) == nullptr)) {
    return absl::InvalidArgumentError(absl::StrCat(
        "semantic: analytic ", absl::AsciiStrToUpper(fname),
        " expects one argument"));
  }

  std::vector<Value> arg_values(input_rows.size());
  if (!is_count_star) {
    for (size_t r = 0; r < input_rows.size(); ++r) {
      EvalContext row_ctx = ctx;
      row_ctx.columns = &input_rows[r];
      auto piece = EvalExpr(*afn.argument_list(0), row_ctx);
      if (!piece.ok()) return piece.status();
      arg_values[r] = *std::move(piece);
    }
  }

  const PartitionOrder partition_order = BuildPartitionOrder(layout);
  const bool distinct = afn.distinct();
  const ::googlesql::Type* return_type = afn.type();

  for (size_t r = 0; r < out_rows.size(); ++r) {
    auto frame_or = ResolveWindowFrameRows(
        afn, order_spec, layout, input_rows, partition_order, r, ctx);
    if (!frame_or.ok()) return frame_or.status();
    if (is_count_star) {
      out_rows[r][out_col_id] =
          Value::Int64(static_cast<int64_t>(frame_or->size()));
      continue;
    }
    std::vector<Value> cells;
    cells.reserve(frame_or->size());
    for (size_t idx : *frame_or) {
      cells.push_back(arg_values[idx]);
    }
    if (is_count_expr) {
      int64_t count = 0;
      if (distinct) {
        absl::flat_hash_map<std::string, bool> seen;
        for (const Value& v : cells) {
          if (v.is_null()) continue;
          const std::string key = v.DebugString();
          if (seen.emplace(key, true).second) ++count;
        }
      } else {
        for (const Value& v : cells) {
          if (!v.is_null()) ++count;
        }
      }
      out_rows[r][out_col_id] = Value::Int64(count);
      continue;
    }
    std::vector<std::vector<Value>> cols{std::move(cells)};
    auto agg_or = functions::EvalAggregateBuiltin(
        fname, return_type, distinct, cols);
    if (!agg_or.ok()) return agg_or.status();
    out_rows[r][out_col_id] = *std::move(agg_or);
  }
  return absl::OkStatus();
}

}  // namespace scan_eval_internal
}  // namespace semantic
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
