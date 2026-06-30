#include <algorithm>
#include <cmath>
#include <cstdint>
#include <map>
#include <optional>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "absl/strings/ascii.h"
#include "absl/strings/str_cat.h"
#include "backend/engine/semantic/error.h"
#include "backend/engine/semantic/functions/aggregate_top_internal.h"
#include "backend/engine/semantic/functions/hll_funcs.h"
#include "backend/engine/semantic/functions/specialized_funcs.h"
#include "backend/engine/semantic/value.h"
#include "googlesql/public/type.h"
#include "googlesql/resolved_ast/resolved_ast.h"
#include "googlesql/resolved_ast/resolved_node_kind.pb.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace semantic {
namespace functions {

using aggregate_top_internal::AccumulateTopCounts;
using aggregate_top_internal::AccumulateTopSums;
using aggregate_top_internal::BuildTopCountArray;
using aggregate_top_internal::BuildTopSumArray;
using aggregate_top_internal::RowValue;
using aggregate_top_internal::SortTopCountEntries;
using aggregate_top_internal::SortTopSumEntries;
using aggregate_top_internal::TopKLimitFromArgColumn;

std::vector<RowValue> CollectAggregateInputs(
    const ::googlesql::ResolvedAggregateFunctionCall& call,
    const std::vector<std::vector<Value>>& input_column_values) {
  std::vector<RowValue> out;
  if (call.argument_list_size() == 0) return out;
  const size_t nrows =
      input_column_values.empty() ? 0 : input_column_values[0].size();
  out.reserve(nrows);
  for (size_t r = 0; r < nrows; ++r) {
    RowValue row;
    if (call.argument_list_size() > 0) {
      const Value& cell = input_column_values[0][r];
      row.is_null = cell.is_null();
      row.v = cell;
    }
    out.push_back(std::move(row));
  }
  return out;
}

absl::StatusOr<Value> ApproxCountDistinct(
    const ::googlesql::ResolvedAggregateFunctionCall& call,
    const std::vector<std::vector<Value>>& input_column_values) {
  std::vector<RowValue> rows =
      CollectAggregateInputs(call, input_column_values);
  std::set<std::string> seen;
  for (const RowValue& row : rows) {
    if (row.is_null) continue;
    seen.insert(row.v.DebugString());
  }
  return Value::Int64(static_cast<int64_t>(seen.size()));
}

struct SortedNumeric {
  std::vector<double> values{};
  bool have_null = false;
};

SortedNumeric SortedNumericValues(const std::vector<RowValue>& rows,
                                  bool distinct) {
  SortedNumeric out;
  std::set<std::string> seen;
  for (const RowValue& row : rows) {
    if (row.is_null) {
      out.have_null = true;
      continue;
    }
    if (row.v.type_kind() != ::googlesql::TYPE_INT64 &&
        row.v.type_kind() != ::googlesql::TYPE_DOUBLE) {
      continue;
    }
    const std::string key = row.v.DebugString();
    if (distinct) {
      if (!seen.insert(key).second) continue;
    }
    out.values.push_back(row.v.type_kind() == ::googlesql::TYPE_INT64
                             ? static_cast<double>(row.v.int64_value())
                             : row.v.double_value());
  }
  std::sort(out.values.begin(), out.values.end());
  return out;
}

std::vector<Value> BuildApproxQuantileElements(
    int64_t n,
    const SortedNumeric& sorted,
    bool respect_nulls,
    bool distinct,
    const std::vector<RowValue>& rows) {
  std::vector<Value> elements;
  elements.reserve(static_cast<size_t>(n) + 1);
  const std::vector<double>& vals = sorted.values;
  if (vals.empty() && !(respect_nulls && sorted.have_null)) {
    for (int64_t i = 0; i <= n; ++i) {
      elements.push_back(Value::NullInt64());
    }
    return elements;
  }
  int64_t null_slots = 0;
  if (respect_nulls && sorted.have_null) {
    null_slots = distinct
                     ? 1
                     : static_cast<int64_t>(std::count_if(
                           rows.begin(), rows.end(), [](const RowValue& row) {
                             return row.is_null;
                           }));
  }
  const int64_t population = null_slots + static_cast<int64_t>(vals.size());
  for (int64_t i = 0; i <= n; ++i) {
    if (population <= 0) {
      elements.push_back(Value::NullInt64());
      continue;
    }
    const double pos = static_cast<double>(i) *
                       static_cast<double>(population - 1) /
                       static_cast<double>(n);
    const int64_t idx = static_cast<int64_t>(
        pos < 0 ? 0 : std::min(pos, static_cast<double>(population - 1)));
    if (idx < null_slots) {
      elements.push_back(Value::NullInt64());
      continue;
    }
    const size_t val_idx = static_cast<size_t>(idx - null_slots);
    if (vals.empty()) {
      elements.push_back(Value::NullInt64());
    } else {
      const size_t pick =
          std::min(val_idx, static_cast<size_t>(vals.size() - 1));
      elements.push_back(Value::Int64(static_cast<int64_t>(vals[pick])));
    }
  }
  return elements;
}

absl::StatusOr<Value> ApproxQuantiles(
    const ::googlesql::ResolvedAggregateFunctionCall& call,
    const std::vector<std::vector<Value>>& input_column_values,
    const ::googlesql::Type* return_type) {
  if (call.argument_list_size() < 2) {
    return absl::InvalidArgumentError("APPROX_QUANTILES expects two arguments");
  }
  const bool distinct = call.distinct();
  const bool respect_nulls =
      call.null_handling_modifier() ==
      ::googlesql::ResolvedNonScalarFunctionCallBase::RESPECT_NULLS;
  std::vector<RowValue> rows =
      CollectAggregateInputs(call, input_column_values);
  SortedNumeric sorted = SortedNumericValues(rows, distinct);
  int64_t n = 2;
  if (input_column_values.size() > 1 && !input_column_values[1].empty() &&
      !input_column_values[1][0].is_null() &&
      input_column_values[1][0].type_kind() == ::googlesql::TYPE_INT64) {
    n = input_column_values[1][0].int64_value();
  }
  const ::googlesql::ArrayType* arr_type =
      return_type != nullptr && return_type->IsArray() ? return_type->AsArray()
                                                       : nullptr;
  std::vector<Value> elements =
      BuildApproxQuantileElements(n, sorted, respect_nulls, distinct, rows);
  if (arr_type == nullptr) {
    return MakeSemanticError(SemanticErrorReason::kInvalidArgument,
                             "APPROX_QUANTILES requires ARRAY return type");
  }
  return Value::Array(arr_type, std::move(elements));
}

absl::StatusOr<Value> ApproxTopCount(
    const ::googlesql::ResolvedAggregateFunctionCall& call,
    const std::vector<std::vector<Value>>& input_column_values,
    const ::googlesql::Type* return_type) {
  std::vector<RowValue> rows =
      CollectAggregateInputs(call, input_column_values);
  const int64_t k = TopKLimitFromArgColumn(
      input_column_values, /*arg_index=*/1, /*default_k=*/2);
  auto sorted = SortTopCountEntries(AccumulateTopCounts(rows), k);
  return BuildTopCountArray(sorted, return_type);
}

absl::StatusOr<Value> ApproxTopSum(
    const ::googlesql::ResolvedAggregateFunctionCall& call,
    const std::vector<std::vector<Value>>& input_column_values,
    const ::googlesql::Type* return_type) {
  if (input_column_values.size() < 2) {
    return absl::InvalidArgumentError(
        "APPROX_TOP_SUM expects value and weight");
  }
  const int64_t k = TopKLimitFromArgColumn(
      input_column_values, /*arg_index=*/2, /*default_k=*/2);
  auto sorted = SortTopSumEntries(AccumulateTopSums(input_column_values), k);
  return BuildTopSumArray(sorted, return_type);
}

absl::StatusOr<Value> ArrayConcatAgg(
    const ::googlesql::ResolvedAggregateFunctionCall& call,
    const std::vector<std::vector<Value>>& input_column_values,
    const ::googlesql::Type* return_type) {
  if (call.argument_list_size() < 1) {
    return absl::InvalidArgumentError(
        "ARRAY_CONCAT_AGG expects one array argument");
  }
  const ::googlesql::ArrayType* arr_type =
      return_type != nullptr && return_type->IsArray() ? return_type->AsArray()
                                                       : nullptr;
  if (arr_type == nullptr) {
    return MakeSemanticError(SemanticErrorReason::kInvalidArgument,
                             "ARRAY_CONCAT_AGG requires ARRAY return type");
  }
  std::vector<Value> out;
  const size_t nrows =
      input_column_values.empty() ? 0 : input_column_values[0].size();
  for (size_t r = 0; r < nrows; ++r) {
    const Value& arr = input_column_values[0][r];
    if (arr.is_null()) continue;
    for (int i = 0; i < arr.num_elements(); ++i) {
      out.push_back(arr.element(i));
    }
  }
  return Value::Array(arr_type, std::move(out));
}

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
    const std::vector<std::vector<Value>>& input_column_values) {
  const bool distinct = call.distinct();
  std::vector<RowValue> rows =
      CollectAggregateInputs(call, input_column_values);
  if (distinct) {
    std::set<std::string> seen;
    int64_t count = 0;
    for (const RowValue& row : rows) {
      if (row.is_null) continue;
      if (seen.insert(row.v.DebugString()).second) {
        ++count;
      }
    }
    return Value::Int64(count);
  }
  int64_t count = 0;
  for (const RowValue& row : rows) {
    if (!row.is_null) {
      ++count;
    }
  }
  return Value::Int64(count);
}

absl::StatusOr<Value> AnyValueAggregate(
    const ::googlesql::ResolvedAggregateFunctionCall& call,
    const std::vector<std::vector<Value>>& input_column_values) {
  std::vector<RowValue> rows =
      CollectAggregateInputs(call, input_column_values);
  for (const RowValue& row : rows) {
    if (!row.is_null) return row.v;
  }
  return Value::Null(call.type());
}

absl::StatusOr<Value> StddevAggregate(
    const ::googlesql::ResolvedAggregateFunctionCall& call,
    const std::vector<std::vector<Value>>& input_column_values) {
  std::vector<RowValue> rows =
      CollectAggregateInputs(call, input_column_values);
  std::vector<double> values;
  values.reserve(rows.size());
  for (const RowValue& row : rows) {
    if (row.is_null) continue;
    if (row.v.type_kind() == ::googlesql::TYPE_INT64) {
      values.push_back(static_cast<double>(row.v.int64_value()));
    } else if (row.v.type_kind() == ::googlesql::TYPE_DOUBLE) {
      values.push_back(row.v.double_value());
    } else if (row.v.type_kind() == ::googlesql::TYPE_FLOAT) {
      values.push_back(static_cast<double>(row.v.float_value()));
    } else {
      return MakeSemanticError(SemanticErrorReason::kInvalidArgument,
                               "semantic: STDDEV expects numeric arguments");
    }
  }
  if (values.size() < 2) return Value::NullDouble();
  double mean = 0.0;
  for (double v : values)
    mean += v;
  mean /= static_cast<double>(values.size());
  double sum_sq = 0.0;
  for (double v : values) {
    const double d = v - mean;
    sum_sq += d * d;
  }
  return Value::Double(
      std::sqrt(sum_sq / static_cast<double>(values.size() - 1)));
}

absl::StatusOr<Value> VarSampAggregate(
    const ::googlesql::ResolvedAggregateFunctionCall& call,
    const std::vector<std::vector<Value>>& input_column_values) {
  std::vector<RowValue> rows =
      CollectAggregateInputs(call, input_column_values);
  std::vector<double> values;
  values.reserve(rows.size());
  for (const RowValue& row : rows) {
    if (row.is_null) continue;
    if (row.v.type_kind() == ::googlesql::TYPE_INT64) {
      values.push_back(static_cast<double>(row.v.int64_value()));
    } else if (row.v.type_kind() == ::googlesql::TYPE_DOUBLE) {
      values.push_back(row.v.double_value());
    } else if (row.v.type_kind() == ::googlesql::TYPE_FLOAT) {
      values.push_back(static_cast<double>(row.v.float_value()));
    } else {
      return MakeSemanticError(SemanticErrorReason::kInvalidArgument,
                               "semantic: VAR_SAMP expects numeric arguments");
    }
  }
  if (values.size() < 2) return Value::NullDouble();
  double mean = 0.0;
  for (double v : values)
    mean += v;
  mean /= static_cast<double>(values.size());
  double sum_sq = 0.0;
  for (double v : values) {
    const double d = v - mean;
    sum_sq += d * d;
  }
  return Value::Double(sum_sq / static_cast<double>(values.size() - 1));
}

absl::StatusOr<Value> CountIfAggregate(
    const ::googlesql::ResolvedAggregateFunctionCall& call,
    const std::vector<std::vector<Value>>& input_column_values) {
  std::vector<RowValue> rows =
      CollectAggregateInputs(call, input_column_values);
  int64_t count = 0;
  for (const RowValue& row : rows) {
    if (row.is_null) continue;
    if (row.v.type_kind() == ::googlesql::TYPE_BOOL && row.v.bool_value()) {
      ++count;
    }
  }
  return Value::Int64(count);
}

}  // namespace functions
}  // namespace semantic
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
