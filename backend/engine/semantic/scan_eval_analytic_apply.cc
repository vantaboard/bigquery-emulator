
#include "googlesql/public/type.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace semantic {
namespace scan_eval_internal {

namespace {

double PercentileContFromSorted(const std::vector<double>& sorted, double p) {
  if (sorted.empty()) return 0.0;
  if (sorted.size() == 1) return sorted.front();
  const double rank = p * static_cast<double>(sorted.size() - 1);
  const size_t lo = static_cast<size_t>(rank);
  const size_t hi = std::min(lo + 1, sorted.size() - 1);
  const double frac = rank - static_cast<double>(lo);
  return sorted[lo] + frac * (sorted[hi] - sorted[lo]);
}

absl::StatusOr<Value> PercentileContValue(const Value& percentile,
                                          const std::vector<Value>& values) {
  if (percentile.is_null()) return Value::NullDouble();
  const double p = percentile.double_value();
  std::vector<double> nums;
  nums.reserve(values.size());
  for (const Value& v : values) {
    if (v.is_null()) continue;
    if (v.type_kind() == ::googlesql::TYPE_INT64) {
      nums.push_back(static_cast<double>(v.int64_value()));
    } else if (v.type_kind() == ::googlesql::TYPE_DOUBLE) {
      nums.push_back(v.double_value());
    } else if (v.type_kind() == ::googlesql::TYPE_FLOAT) {
      nums.push_back(static_cast<double>(v.float_value()));
    } else if (v.type_kind() == ::googlesql::TYPE_NUMERIC) {
      nums.push_back(v.numeric_value().ToDouble());
    } else {
      return MakeSemanticError(SemanticErrorReason::kNotImplemented,
                               "semantic: PERCENTILE_CONT unsupported type");
    }
  }
  if (nums.empty()) return Value::NullDouble();
  std::sort(nums.begin(), nums.end());
  return Value::Double(PercentileContFromSorted(nums, p));
}

std::string AnalyticFunctionName(
    const ::googlesql::ResolvedAnalyticFunctionCall& afn) {
  std::string fname =
      absl::AsciiStrToLower(afn.function()->FullName(/*include_group=*/false));
  if (fname.empty()) {
    fname = absl::AsciiStrToLower(afn.function()->Name());
  }
  return fname;
}

void ApplyAnalyticRowNumber(const AnalyticGroupLayout& layout,
                            int out_col_id,
                            std::vector<ColumnBindings>& out_rows) {
  for (size_t r = 0; r < out_rows.size(); ++r) {
    out_rows[r][out_col_id] = Value::Int64(layout.row_numbers[r]);
  }
}

bool IsCountRangeFrame(const ::googlesql::ResolvedAnalyticFunctionCall& afn,
                       const ::googlesql::ResolvedWindowOrdering* order_spec) {
  const ::googlesql::ResolvedWindowFrame* wf = afn.window_frame();
  return wf != nullptr &&
         wf->frame_unit() == ::googlesql::ResolvedWindowFrame::RANGE &&
         order_spec != nullptr && order_spec->order_by_item_list_size() > 0;
}

absl::StatusOr<int64_t> CountRowsInRangeFrame(
    size_t row_index,
    const ::googlesql::ResolvedAnalyticFunctionCall& afn,
    const AnalyticGroupLayout& layout,
    const std::vector<ColumnBindings>& input_rows,
    int order_col_id,
    const Value& low,
    bool has_low,
    const Value& high,
    bool has_high,
    const EvalContext& ctx) {
  int64_t count = 0;
  for (size_t other = 0; other < input_rows.size(); ++other) {
    if (layout.partition_fps[other] != layout.partition_fps[row_index]) {
      continue;
    }
    const Value other_order =
        LookupColumnValue(input_rows[other], order_col_id);
    if (!ValueInClosedRange(other_order, low, has_low, high, has_high)) {
      continue;
    }
    if (afn.argument_list_size() == 0 || afn.argument_list(0) == nullptr) {
      count++;
      continue;
    }
    EvalContext other_ctx = ctx;
    other_ctx.columns = &input_rows[other];
    auto arg = EvalExpr(*afn.argument_list(0), other_ctx);
    if (!arg.ok()) return arg.status();
    if (!arg->is_null()) count++;
  }
  return count;
}

absl::Status ApplyAnalyticCountRange(
    const ::googlesql::ResolvedAnalyticFunctionCall& afn,
    const ::googlesql::ResolvedWindowOrdering* order_spec,
    const AnalyticGroupLayout& layout,
    const std::vector<ColumnBindings>& input_rows,
    int out_col_id,
    const EvalContext& ctx,
    std::vector<ColumnBindings>& out_rows) {
  const ::googlesql::ResolvedWindowFrame* wf = afn.window_frame();
  const ::googlesql::ResolvedOrderByItem* order_item =
      order_spec->order_by_item_list(0);
  if (order_item == nullptr || order_item->column_ref() == nullptr) {
    return MakeSemanticError(
        SemanticErrorReason::kNotImplemented,
        "semantic: analytic COUNT RANGE missing order key");
  }
  const int order_col_id = order_item->column_ref()->column().column_id();
  const ::googlesql::Type* order_type = order_item->column_ref()->type();
  if (order_type == nullptr ||
      (order_type->kind() != ::googlesql::TYPE_DATE &&
       order_type->kind() != ::googlesql::TYPE_TIMESTAMP)) {
    return MakeSemanticError(
        SemanticErrorReason::kNotImplemented,
        "semantic: analytic COUNT RANGE requires DATE/TIMESTAMP order");
  }
  for (size_t r = 0; r < out_rows.size(); ++r) {
    const Value current_order = LookupColumnValue(input_rows[r], order_col_id);
    EvalContext row_ctx = ctx;
    row_ctx.columns = &input_rows[r];
    auto low_or = FrameBoundValue(wf->start_expr(), current_order, row_ctx);
    if (!low_or.ok()) return low_or.status();
    auto high_or = FrameBoundValue(wf->end_expr(), current_order, row_ctx);
    if (!high_or.ok()) return high_or.status();
    const bool has_low = !(*low_or).is_null();
    const bool has_high = !(*high_or).is_null();
    auto count_or = CountRowsInRangeFrame(r,
                                          afn,
                                          layout,
                                          input_rows,
                                          order_col_id,
                                          *low_or,
                                          has_low,
                                          *high_or,
                                          has_high,
                                          ctx);
    if (!count_or.ok()) return count_or.status();
    out_rows[r][out_col_id] = Value::Int64(*count_or);
  }
  return absl::OkStatus();
}

absl::Status ApplyAnalyticSum(
    const ::googlesql::ResolvedAnalyticFunctionCall& afn,
    const AnalyticGroupLayout& layout,
    const std::vector<ColumnBindings>& input_rows,
    int out_col_id,
    const EvalContext& ctx,
    std::vector<ColumnBindings>& out_rows) {
  if (afn.argument_list_size() != 1 || afn.argument_list(0) == nullptr) {
    return absl::InvalidArgumentError(
        "semantic: analytic SUM expects one argument");
  }
  absl::flat_hash_map<std::string, Value> partition_sums;
  for (size_t r = 0; r < input_rows.size(); ++r) {
    EvalContext row_ctx = ctx;
    row_ctx.columns = &input_rows[r];
    auto piece = EvalExpr(*afn.argument_list(0), row_ctx);
    if (!piece.ok()) return piece.status();
    auto it = partition_sums.find(layout.partition_fps[r]);
    if (it == partition_sums.end()) {
      partition_sums.emplace(layout.partition_fps[r], *piece);
    } else {
      auto summed = AddValues(it->second, *piece);
      if (!summed.ok()) return summed.status();
      it->second = *std::move(summed);
    }
  }
  for (size_t r = 0; r < out_rows.size(); ++r) {
    auto it = partition_sums.find(layout.partition_fps[r]);
    out_rows[r][out_col_id] =
        (it == partition_sums.end()) ? Value::NullInt64() : it->second;
  }
  return absl::OkStatus();
}

absl::Status ApplyAnalyticPercentileCont(
    const ::googlesql::ResolvedAnalyticFunctionCall& afn,
    const AnalyticGroupLayout& layout,
    const std::vector<ColumnBindings>& input_rows,
    int out_col_id,
    const EvalContext& ctx,
    std::vector<ColumnBindings>& out_rows) {
  if (afn.argument_list_size() != 2 || afn.argument_list(0) == nullptr ||
      afn.argument_list(1) == nullptr) {
    return absl::InvalidArgumentError(
        "semantic: analytic PERCENTILE_CONT expects two arguments");
  }
  absl::flat_hash_map<std::string, std::vector<Value>> partition_values;
  absl::flat_hash_map<std::string, Value> partition_percentile;
  for (size_t r = 0; r < input_rows.size(); ++r) {
    EvalContext row_ctx = ctx;
    row_ctx.columns = &input_rows[r];
    auto piece = EvalExpr(*afn.argument_list(0), row_ctx);
    if (!piece.ok()) return piece.status();
    auto pct = EvalExpr(*afn.argument_list(1), row_ctx);
    if (!pct.ok()) return pct.status();
    partition_values[layout.partition_fps[r]].push_back(*piece);
    partition_percentile.emplace(layout.partition_fps[r], *pct);
  }
  absl::flat_hash_map<std::string, Value> partition_result;
  for (const auto& [fp, vals] : partition_values) {
    auto it = partition_percentile.find(fp);
    if (it == partition_percentile.end()) continue;
    auto out = PercentileContValue(it->second, vals);
    if (!out.ok()) return out.status();
    partition_result.emplace(fp, *out);
  }
  for (size_t r = 0; r < out_rows.size(); ++r) {
    auto it = partition_result.find(layout.partition_fps[r]);
    out_rows[r][out_col_id] =
        (it == partition_result.end()) ? Value::NullDouble() : it->second;
  }
  return absl::OkStatus();
}

absl::Status ApplyAnalyticFunction(
    const ::googlesql::ResolvedComputedColumnBase& cc,
    const ::googlesql::ResolvedAnalyticFunctionGroup& group,
    const ::googlesql::ResolvedWindowOrdering* order_spec,
    const AnalyticGroupLayout& layout,
    const std::vector<ColumnBindings>& input_rows,
    const EvalContext& ctx,
    std::vector<ColumnBindings>& out_rows) {
  if (cc.expr() == nullptr ||
      cc.expr()->node_kind() != ::googlesql::RESOLVED_ANALYTIC_FUNCTION_CALL) {
    return MakeSemanticError(
        SemanticErrorReason::kNotImplemented,
        "semantic: analytic column is not a function call");
  }
  const auto* afn =
      cc.expr()->GetAs<::googlesql::ResolvedAnalyticFunctionCall>();
  if (afn == nullptr || afn->function() == nullptr) {
    return absl::InternalError("semantic: analytic function is null");
  }
  const std::string fname = AnalyticFunctionName(*afn);
  const int out_col_id = cc.column().column_id();
  if (fname == "row_number") {
    ApplyAnalyticRowNumber(layout, out_col_id, out_rows);
    return absl::OkStatus();
  }
  if (fname == "count") {
    if (IsCountRangeFrame(*afn, order_spec)) {
      return ApplyAnalyticCountRange(
          *afn, order_spec, layout, input_rows, out_col_id, ctx, out_rows);
    }
  }
  if (fname == "sum") {
    return ApplyAnalyticSum(
        *afn, layout, input_rows, out_col_id, ctx, out_rows);
  }
  if (fname == "percentile_cont") {
    return ApplyAnalyticPercentileCont(
        *afn, layout, input_rows, out_col_id, ctx, out_rows);
  }
  return MakeSemanticError(
      SemanticErrorReason::kNotImplemented,
      absl::StrCat(
          "semantic: analytic function '", fname, "' is not yet implemented"));
}

}  // namespace

absl::StatusOr<std::vector<ColumnBindings>> MaterializeAnalyticScan(
    const ::googlesql::ResolvedAnalyticScan& analytic, EvalContext& ctx) {
  auto input_or = MaterializeScanImpl(analytic.input_scan(), ctx);
  if (!input_or.ok()) return input_or.status();
  std::vector<ColumnBindings> input_rows = *std::move(input_or);
  std::vector<ColumnBindings> out_rows = input_rows;

  for (int g = 0; g < analytic.function_group_list_size(); ++g) {
    const ::googlesql::ResolvedAnalyticFunctionGroup* group =
        analytic.function_group_list(g);
    if (group == nullptr) continue;

    const ::googlesql::ResolvedWindowOrdering* order_spec = group->order_by();
    const AnalyticGroupLayout layout =
        BuildAnalyticGroupLayout(*group, order_spec, input_rows);

    for (int f = 0; f < group->analytic_function_list_size(); ++f) {
      const ::googlesql::ResolvedComputedColumnBase* cc =
          group->analytic_function_list(f);
      if (cc == nullptr) continue;
      absl::Status applied = ApplyAnalyticFunction(
          *cc, *group, order_spec, layout, input_rows, ctx, out_rows);
      if (!applied.ok()) return applied;
    }
  }

  return out_rows;
}

}  // namespace scan_eval_internal
}  // namespace semantic
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
