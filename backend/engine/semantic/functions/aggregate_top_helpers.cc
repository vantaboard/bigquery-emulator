#include <algorithm>
#include <cstdint>
#include <map>
#include <string>
#include <utility>
#include <vector>

#include "absl/status/statusor.h"
#include "backend/engine/semantic/error.h"
#include "backend/engine/semantic/functions/aggregate_top_internal.h"
#include "backend/engine/semantic/value.h"
#include "googlesql/public/type.h"
#include "googlesql/public/types/struct_type.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace semantic {
namespace functions {
namespace aggregate_top_internal {

std::map<std::string, TopCountEntry> AccumulateTopCounts(
    const std::vector<RowValue>& rows) {
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
  return counts;
}

std::map<std::string, TopCountEntry> AccumulateTopSums(
    const std::vector<std::vector<Value>>& input_column_values) {
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
  return sums;
}

int64_t TopKLimitFromArgColumn(
    const std::vector<std::vector<Value>>& input_column_values,
    size_t arg_index,
    int64_t default_k) {
  if (input_column_values.size() <= arg_index ||
      input_column_values[arg_index].empty() ||
      input_column_values[arg_index][0].is_null()) {
    return default_k;
  }
  return input_column_values[arg_index][0].int64_value();
}

std::vector<TopCountEntry> SortTopCountEntries(
    std::map<std::string, TopCountEntry> counts, int64_t k) {
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
  return sorted;
}

std::vector<TopCountEntry> SortTopSumEntries(
    std::map<std::string, TopCountEntry> sums, int64_t k) {
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
  return sorted;
}

absl::StatusOr<Value> BuildTopCountArray(
    const std::vector<TopCountEntry>& sorted,
    const ::googlesql::Type* return_type) {
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

absl::StatusOr<Value> BuildTopSumArray(const std::vector<TopCountEntry>& sorted,
                                       const ::googlesql::Type* return_type) {
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

}  // namespace aggregate_top_internal
}  // namespace functions
}  // namespace semantic
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
