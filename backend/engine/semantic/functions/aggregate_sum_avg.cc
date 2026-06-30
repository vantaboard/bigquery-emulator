#include <optional>
#include <string>
#include <vector>

#include "absl/status/statusor.h"
#include "absl/strings/str_cat.h"
#include "backend/engine/semantic/error.h"
#include "backend/engine/semantic/functions/aggregate_sum_avg_internal.h"
#include "backend/engine/semantic/functions/specialized_funcs.h"
#include "backend/engine/semantic/value.h"
#include "googlesql/public/type.h"
#include "googlesql/resolved_ast/resolved_ast.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace semantic {
namespace functions {

namespace {

using aggregate_sum_avg_internal::AvgBigNumericCells;
using aggregate_sum_avg_internal::AvgDoubleCells;
using aggregate_sum_avg_internal::AvgInt64Cells;
using aggregate_sum_avg_internal::AvgNumericCells;
using aggregate_sum_avg_internal::IsSupportedMinMaxType;
using aggregate_sum_avg_internal::NullOfAggregateType;
using aggregate_sum_avg_internal::ShouldReplaceMinMax;
using aggregate_sum_avg_internal::SumDoubleCells;
using aggregate_sum_avg_internal::SumInt64Cells;
using aggregate_sum_avg_internal::SumNumericAggregateCells;
using aggregate_sum_avg_internal::SumNumericCells;

struct BuiltinAggregateView {
  const ::googlesql::Type* return_type = nullptr;
  bool distinct_flag = false;
  int argument_list_size() const {
    return 1;
  }
  const ::googlesql::Type* type() const {
    return return_type;
  }
  bool distinct() const {
    return distinct_flag;
  }
};

template <typename CallLike>
absl::StatusOr<Value> SumAggregateImpl(
    const CallLike& call,
    const std::vector<std::vector<Value>>& input_column_values) {
  if (input_column_values.size() != 1) {
    return MakeSemanticError(SemanticErrorReason::kInvalidArgument,
                             "semantic: SUM expects one argument column");
  }
  if (input_column_values[0].empty()) {
    return NullOfAggregateType(call.type());
  }
  const ::googlesql::Type* out_type = call.type();
  const std::vector<Value>& cells = input_column_values[0];
  if (out_type != nullptr && out_type->kind() == ::googlesql::TYPE_NUMERIC) {
    return SumNumericAggregateCells(out_type, cells);
  }
  switch (cells.front().type_kind()) {
    case ::googlesql::TYPE_INT64:
      return SumInt64Cells(out_type, cells);
    case ::googlesql::TYPE_DOUBLE:
      return SumDoubleCells(out_type, cells);
    case ::googlesql::TYPE_NUMERIC:
      return SumNumericCells(out_type, cells);
    default:
      return MakeSemanticError(
          SemanticErrorReason::kNotImplemented,
          "semantic: SUM is not implemented for this argument type");
  }
}

template <typename CallLike>
absl::StatusOr<Value> AvgAggregateImpl(
    const CallLike& call,
    const std::vector<std::vector<Value>>& input_column_values) {
  if (input_column_values.size() != 1 || input_column_values[0].empty()) {
    return NullOfAggregateType(call.type());
  }
  const std::vector<Value>& cells = input_column_values[0];
  switch (cells.front().type_kind()) {
    case ::googlesql::TYPE_INT64:
      return AvgInt64Cells(call.type(), cells);
    case ::googlesql::TYPE_DOUBLE:
      return AvgDoubleCells(call.type(), cells);
    case ::googlesql::TYPE_NUMERIC:
      return AvgNumericCells(call.type(), cells);
    case ::googlesql::TYPE_BIGNUMERIC:
      return AvgBigNumericCells(call.type(), cells);
    default:
      return MakeSemanticError(
          SemanticErrorReason::kNotImplemented,
          "semantic: AVG is not implemented for this argument type");
  }
}

template <typename CallLike>
absl::StatusOr<Value> MinMaxAggregateImpl(
    const CallLike& call,
    const std::vector<std::vector<Value>>& input_column_values,
    bool pick_max) {
  if (input_column_values.size() != 1) {
    return MakeSemanticError(SemanticErrorReason::kInvalidArgument,
                             "semantic: MIN/MAX expects one argument column");
  }
  if (input_column_values[0].empty()) {
    return NullOfAggregateType(call.type());
  }
  const std::vector<Value>& cells = input_column_values[0];
  std::optional<Value> best;
  for (const Value& v : cells) {
    if (v.is_null()) continue;
    if (!best.has_value()) {
      best = v;
      continue;
    }
    const Value& cur = *best;
    if (cur.type_kind() != v.type_kind()) {
      return MakeSemanticError(SemanticErrorReason::kInvalidArgument,
                               "semantic: MIN/MAX argument type mismatch");
    }
    if (!IsSupportedMinMaxType(v.type_kind())) {
      return MakeSemanticError(
          SemanticErrorReason::kNotImplemented,
          "semantic: MIN/MAX is not implemented for this argument type");
    }
    if (ShouldReplaceMinMax(cur, v, pick_max)) {
      best = v;
    }
  }
  if (!best.has_value()) return NullOfAggregateType(call.type());
  return *best;
}

}  // namespace

absl::StatusOr<Value> SumAggregate(
    const ::googlesql::ResolvedAggregateFunctionCall& call,
    const std::vector<std::vector<Value>>& input_column_values) {
  return SumAggregateImpl(call, input_column_values);
}

absl::StatusOr<Value> AvgAggregate(
    const ::googlesql::ResolvedAggregateFunctionCall& call,
    const std::vector<std::vector<Value>>& input_column_values) {
  return AvgAggregateImpl(call, input_column_values);
}

absl::StatusOr<Value> MinAggregate(
    const ::googlesql::ResolvedAggregateFunctionCall& call,
    const std::vector<std::vector<Value>>& input_column_values) {
  return MinMaxAggregateImpl(call, input_column_values, /*pick_max=*/false);
}

absl::StatusOr<Value> MaxAggregate(
    const ::googlesql::ResolvedAggregateFunctionCall& call,
    const std::vector<std::vector<Value>>& input_column_values) {
  return MinMaxAggregateImpl(call, input_column_values, /*pick_max=*/true);
}

absl::StatusOr<Value> EvalAggregateBuiltin(
    absl::string_view name,
    const ::googlesql::Type* return_type,
    bool distinct,
    const std::vector<std::vector<Value>>& input_column_values) {
  BuiltinAggregateView view{return_type, distinct};
  if (name == "sum") {
    return SumAggregateImpl(view, input_column_values);
  }
  if (name == "avg") {
    return AvgAggregateImpl(view, input_column_values);
  }
  if (name == "min") {
    return MinMaxAggregateImpl(view, input_column_values, /*pick_max=*/false);
  }
  if (name == "max") {
    return MinMaxAggregateImpl(view, input_column_values, /*pick_max=*/true);
  }
  return MakeSemanticError(
      SemanticErrorReason::kNotImplemented,
      absl::StrCat("semantic: aggregate '", name, "' is not implemented"));
}

}  // namespace functions
}  // namespace semantic
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
