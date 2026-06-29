
namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace semantic {
namespace functions {

absl::StatusOr<Value> SumAggregate(
    const ::googlesql::ResolvedAggregateFunctionCall& call,
    const std::vector<std::vector<Value>>& input_column_values);
absl::StatusOr<Value> AvgAggregate(
    const ::googlesql::ResolvedAggregateFunctionCall& call,
    const std::vector<std::vector<Value>>& input_column_values);
absl::StatusOr<Value> MinAggregate(
    const ::googlesql::ResolvedAggregateFunctionCall& call,
    const std::vector<std::vector<Value>>& input_column_values);
absl::StatusOr<Value> MaxAggregate(
    const ::googlesql::ResolvedAggregateFunctionCall& call,
    const std::vector<std::vector<Value>>& input_column_values);
absl::StatusOr<Value> CountAggregate(
    const ::googlesql::ResolvedAggregateFunctionCall& call,
    const std::vector<std::vector<Value>>& input_column_values);
absl::StatusOr<Value> CountIfAggregate(
    const ::googlesql::ResolvedAggregateFunctionCall& call,
    const std::vector<std::vector<Value>>& input_column_values);
absl::StatusOr<Value> AnyValueAggregate(
    const ::googlesql::ResolvedAggregateFunctionCall& call,
    const std::vector<std::vector<Value>>& input_column_values);
absl::StatusOr<Value> StddevAggregate(
    const ::googlesql::ResolvedAggregateFunctionCall& call,
    const std::vector<std::vector<Value>>& input_column_values);
absl::StatusOr<Value> VarSampAggregate(
    const ::googlesql::ResolvedAggregateFunctionCall& call,
    const std::vector<std::vector<Value>>& input_column_values);
absl::StatusOr<Value> ApproxCountDistinct(
    const ::googlesql::ResolvedAggregateFunctionCall& call,
    const std::vector<std::vector<Value>>& input_column_values);
absl::StatusOr<Value> ApproxQuantiles(
    const ::googlesql::ResolvedAggregateFunctionCall& call,
    const std::vector<std::vector<Value>>& input_column_values,
    const ::googlesql::Type* return_type);
absl::StatusOr<Value> ApproxTopCount(
    const ::googlesql::ResolvedAggregateFunctionCall& call,
    const std::vector<std::vector<Value>>& input_column_values,
    const ::googlesql::Type* return_type);
absl::StatusOr<Value> ApproxTopSum(
    const ::googlesql::ResolvedAggregateFunctionCall& call,
    const std::vector<std::vector<Value>>& input_column_values,
    const ::googlesql::Type* return_type);
absl::StatusOr<Value> ArrayConcatAgg(
    const ::googlesql::ResolvedAggregateFunctionCall& call,
    const std::vector<std::vector<Value>>& input_column_values,
    const ::googlesql::Type* return_type);

namespace {

template <typename FinishFn>
absl::StatusOr<Value> DispatchStdAggregates(
    absl::string_view name,
    const ::googlesql::ResolvedAggregateFunctionCall& call,
    const std::vector<std::vector<Value>>& input_column_values,
    FinishFn finish) {
  if (name == "sum" || name == "agg") {
    return finish(SumAggregate(call, input_column_values));
  }
  if (name == "avg") {
    return finish(AvgAggregate(call, input_column_values));
  }
  if (name == "min") {
    return finish(MinAggregate(call, input_column_values));
  }
  if (name == "max") {
    return finish(MaxAggregate(call, input_column_values));
  }
  if (name == "count") {
    return finish(CountAggregate(call, input_column_values));
  }
  if (name == "countif") {
    return finish(CountIfAggregate(call, input_column_values));
  }
  if (name == "any_value") {
    return finish(AnyValueAggregate(call, input_column_values));
  }
  if (name == "stddev" || name == "stddev_samp" || name == "stdev") {
    return finish(StddevAggregate(call, input_column_values));
  }
  if (name == "var_samp" || name == "variance") {
    return finish(VarSampAggregate(call, input_column_values));
  }
  return MakeSemanticError(SemanticErrorReason::kNotImplemented,
                           "semantic: not a standard aggregate");
}

template <typename FinishFn>
absl::StatusOr<Value> DispatchApproxAggregates(
    absl::string_view name,
    const ::googlesql::ResolvedAggregateFunctionCall& call,
    const std::vector<std::vector<Value>>& input_column_values,
    FinishFn finish) {
  if (name == "approx_count_distinct") {
    return finish(ApproxCountDistinct(call, input_column_values));
  }
  if (name == "approx_quantiles") {
    return finish(ApproxQuantiles(call, input_column_values, call.type()));
  }
  if (name == "approx_top_count") {
    return finish(ApproxTopCount(call, input_column_values, call.type()));
  }
  if (name == "approx_top_sum") {
    return finish(ApproxTopSum(call, input_column_values, call.type()));
  }
  return MakeSemanticError(SemanticErrorReason::kNotImplemented,
                           "semantic: not an approx aggregate");
}

template <typename FinishFn>
absl::StatusOr<Value> DispatchSketchAggregates(
    absl::string_view name,
    const ::googlesql::ResolvedAggregateFunctionCall& call,
    const std::vector<std::vector<Value>>& input_column_values,
    FinishFn finish) {
  if (name == "hll_count.init") {
    return finish(HllCountInitAggregate(call, input_column_values));
  }
  if (name == "hll_count.merge") {
    return finish(HllCountMergeAggregate(input_column_values));
  }
  if (name == "hll_count.merge_partial") {
    return finish(HllCountMergePartialAggregate(input_column_values));
  }
  if (name == "kll_quantiles.init_int64") {
    return finish(KllQuantilesInitInt64Aggregate(call, input_column_values));
  }
  if (name == "kll_quantiles.init_float64") {
    return finish(KllQuantilesInitFloat64Aggregate(call, input_column_values));
  }
  if (name == "kll_quantiles.merge_partial") {
    return finish(KllQuantilesMergePartialAggregate(input_column_values));
  }
  if (name == "kll_quantiles.merge_int64") {
    return finish(
        KllQuantilesMergeInt64Aggregate(input_column_values, call.type()));
  }
  if (name == "kll_quantiles.merge_float64") {
    return finish(
        KllQuantilesMergeFloat64Aggregate(input_column_values, call.type()));
  }
  if (name == "kll_quantiles.merge_point_int64") {
    return finish(KllQuantilesMergePointInt64Aggregate(input_column_values));
  }
  if (name == "kll_quantiles.merge_point_float64") {
    return finish(KllQuantilesMergePointFloat64Aggregate(input_column_values));
  }
  return MakeSemanticError(SemanticErrorReason::kNotImplemented,
                           "semantic: not a sketch aggregate");
}

template <typename FinishFn>
absl::StatusOr<Value> DispatchLogicalAggregates(
    absl::string_view name,
    const std::vector<std::vector<Value>>& input_column_values,
    FinishFn finish) {
  if (name == "logical_or") {
    bool any = false;
    if (!input_column_values.empty()) {
      for (const Value& v : input_column_values[0]) {
        if (v.is_null()) continue;
        if (v.bool_value()) {
          any = true;
          break;
        }
      }
    }
    return finish(Value::Bool(any));
  }
  if (name == "logical_and") {
    if (input_column_values.empty() || input_column_values[0].empty()) {
      return finish(Value::Bool(false));
    }
    bool all = true;
    for (const Value& v : input_column_values[0]) {
      if (v.is_null() || !v.bool_value()) {
        all = false;
        break;
      }
    }
    return finish(Value::Bool(all));
  }
  return MakeSemanticError(SemanticErrorReason::kNotImplemented,
                           "semantic: not a logical aggregate");
}

}  // namespace

absl::StatusOr<Value> MaybeWrapSafeAggregate(
    const ::googlesql::ResolvedAggregateFunctionCall& call,
    absl::StatusOr<Value> result) {
  if (call.error_mode() !=
      ::googlesql::ResolvedFunctionCallBase::SAFE_ERROR_MODE) {
    return result;
  }
  if (result.ok()) return result;
  SemanticErrorReason reason = GetSemanticErrorReason(result.status());
  if (reason == SemanticErrorReason::kOverflow ||
      reason == SemanticErrorReason::kDivisionByZero) {
    return Value::Null(call.type());
  }
  return result;
}

absl::StatusOr<Value> EvalAggregateCall(
    const ::googlesql::ResolvedAggregateFunctionCall& call,
    const std::vector<std::vector<Value>>& input_column_values) {
  if (call.function() == nullptr) {
    return absl::InvalidArgumentError("aggregate call has null function");
  }
  std::string name =
      absl::AsciiStrToLower(call.function()->FullName(/*include_group=*/false));
  if (name.empty()) {
    name = absl::AsciiStrToLower(call.function()->Name());
  }
  const auto finish = [&](absl::StatusOr<Value> result) {
    return MaybeWrapSafeAggregate(call, std::move(result));
  };
  if (auto std_agg =
          DispatchStdAggregates(name, call, input_column_values, finish);
      std_agg.ok() || GetSemanticErrorReason(std_agg.status()) !=
                          SemanticErrorReason::kNotImplemented) {
    return std_agg;
  }
  if (auto approx =
          DispatchApproxAggregates(name, call, input_column_values, finish);
      approx.ok() || GetSemanticErrorReason(approx.status()) !=
                         SemanticErrorReason::kNotImplemented) {
    return approx;
  }
  if (auto sketch =
          DispatchSketchAggregates(name, call, input_column_values, finish);
      sketch.ok() || GetSemanticErrorReason(sketch.status()) !=
                         SemanticErrorReason::kNotImplemented) {
    return sketch;
  }
  if (name == "array_concat_agg") {
    return finish(ArrayConcatAgg(call, input_column_values, call.type()));
  }
  if (auto logical =
          DispatchLogicalAggregates(name, input_column_values, finish);
      logical.ok() || GetSemanticErrorReason(logical.status()) !=
                          SemanticErrorReason::kNotImplemented) {
    return logical;
  }
  return finish(MakeSemanticError(
      SemanticErrorReason::kNotImplemented,
      absl::StrCat("semantic: aggregate '", name, "' is not implemented")));
}

}  // namespace functions
}  // namespace semantic
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
