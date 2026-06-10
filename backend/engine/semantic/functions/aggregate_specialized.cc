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

struct RowValue {
  Value v;
  bool is_null = false;
};

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
  std::vector<double> values;
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
  std::vector<Value> elements;
  elements.reserve(static_cast<size_t>(n) + 1);
  const std::vector<double>& vals = sorted.values;
  if (vals.empty() && !(respect_nulls && sorted.have_null)) {
    for (int64_t i = 0; i <= n; ++i) {
      elements.push_back(Value::NullInt64());
    }
  } else {
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
  }
  if (arr_type == nullptr) {
    return MakeSemanticError(SemanticErrorReason::kInvalidArgument,
                             "APPROX_QUANTILES requires ARRAY return type");
  }
  return Value::Array(arr_type, std::move(elements));
}

struct TopCountEntry {
  Value key;
  int64_t sum = 0;
  bool weight_was_non_null = false;
};

absl::StatusOr<Value> ApproxTopCount(
    const ::googlesql::ResolvedAggregateFunctionCall& call,
    const std::vector<std::vector<Value>>& input_column_values,
    const ::googlesql::Type* return_type) {
  std::vector<RowValue> rows =
      CollectAggregateInputs(call, input_column_values);
  std::map<std::string, TopCountEntry> counts;
  for (const RowValue& row : rows) {
    const std::string key = row.is_null ? "NULL" : row.v.DebugString();
    auto it = counts.find(key);
    if (it == counts.end()) {
      counts.emplace(
          key,
          TopCountEntry{row.is_null ? Value::NullString() : row.v, 1, true});
    } else {
      it->second.sum += 1;
    }
  }
  int64_t k = 2;
  if (input_column_values.size() > 1 && !input_column_values[1].empty() &&
      !input_column_values[1][0].is_null()) {
    k = input_column_values[1][0].int64_value();
  }
  std::vector<TopCountEntry> sorted;
  sorted.reserve(counts.size());
  for (auto& kv : counts)
    sorted.push_back(std::move(kv.second));
  std::sort(sorted.begin(),
            sorted.end(),
            [](const TopCountEntry& a, const TopCountEntry& b) {
              if (a.sum != b.sum) return a.sum > b.sum;
              return a.key.DebugString() < b.key.DebugString();
            });
  if (sorted.size() > static_cast<size_t>(k)) {
    sorted.resize(static_cast<size_t>(k));
  }
  const ::googlesql::ArrayType* arr_type =
      return_type != nullptr && return_type->IsArray() ? return_type->AsArray()
                                                       : nullptr;
  const ::googlesql::StructType* struct_type =
      arr_type != nullptr ? arr_type->element_type()->AsStruct() : nullptr;
  std::vector<Value> out_elems;
  for (const TopCountEntry& e : sorted) {
    std::vector<Value> fields = {e.key, Value::Int64(e.sum)};
    if (struct_type != nullptr) {
      out_elems.push_back(Value::Struct(struct_type, std::move(fields)));
    }
  }
  if (arr_type == nullptr) {
    return MakeSemanticError(SemanticErrorReason::kInvalidArgument,
                             "APPROX_TOP_COUNT requires ARRAY<STRUCT>");
  }
  return Value::Array(arr_type, std::move(out_elems));
}

absl::StatusOr<Value> ApproxTopSum(
    const ::googlesql::ResolvedAggregateFunctionCall& call,
    const std::vector<std::vector<Value>>& input_column_values,
    const ::googlesql::Type* return_type) {
  if (input_column_values.size() < 2) {
    return absl::InvalidArgumentError(
        "APPROX_TOP_SUM expects value and weight");
  }
  const size_t nrows = input_column_values[0].size();
  std::map<std::string, TopCountEntry> sums;
  for (size_t r = 0; r < nrows; ++r) {
    const Value& key_v = input_column_values[0][r];
    const Value& weight_v = input_column_values[1][r];
    const std::string key = key_v.is_null() ? "NULL" : key_v.DebugString();
    auto it = sums.find(key);
    if (it == sums.end()) {
      TopCountEntry entry{
          key_v.is_null() ? Value::NullString() : key_v, 0, false};
      if (!weight_v.is_null() &&
          weight_v.type_kind() == ::googlesql::TYPE_INT64) {
        entry.sum = weight_v.int64_value();
        entry.weight_was_non_null = true;
      }
      sums.emplace(key, std::move(entry));
    } else if (!weight_v.is_null() &&
               weight_v.type_kind() == ::googlesql::TYPE_INT64) {
      it->second.sum += weight_v.int64_value();
      it->second.weight_was_non_null = true;
    }
  }
  int64_t k = 2;
  if (input_column_values.size() > 2 && !input_column_values[2].empty() &&
      !input_column_values[2][0].is_null()) {
    k = input_column_values[2][0].int64_value();
  }
  std::vector<TopCountEntry> sorted;
  for (auto& kv : sums)
    sorted.push_back(std::move(kv.second));
  std::sort(sorted.begin(),
            sorted.end(),
            [](const TopCountEntry& a, const TopCountEntry& b) {
              if (a.sum != b.sum) return a.sum > b.sum;
              if (a.weight_was_non_null != b.weight_was_non_null) {
                return a.weight_was_non_null > b.weight_was_non_null;
              }
              return a.key.DebugString() < b.key.DebugString();
            });
  if (sorted.size() > static_cast<size_t>(k)) {
    sorted.resize(static_cast<size_t>(k));
  }
  const ::googlesql::ArrayType* arr_type =
      return_type != nullptr && return_type->IsArray() ? return_type->AsArray()
                                                       : nullptr;
  const ::googlesql::StructType* struct_type =
      arr_type != nullptr ? arr_type->element_type()->AsStruct() : nullptr;
  std::vector<Value> out_elems;
  for (const TopCountEntry& e : sorted) {
    Value weight_val =
        e.weight_was_non_null ? Value::Int64(e.sum) : Value::NullInt64();
    std::vector<Value> fields = {e.key, std::move(weight_val)};
    if (struct_type != nullptr) {
      out_elems.push_back(Value::Struct(struct_type, std::move(fields)));
    }
  }
  if (arr_type == nullptr) {
    return MakeSemanticError(SemanticErrorReason::kInvalidArgument,
                             "APPROX_TOP_SUM requires ARRAY<STRUCT>");
  }
  return Value::Array(arr_type, std::move(out_elems));
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
