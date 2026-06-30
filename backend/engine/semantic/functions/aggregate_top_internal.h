#ifndef BIGQUERY_EMULATOR_BACKEND_ENGINE_SEMANTIC_FUNCTIONS_AGGREGATE_TOP_INTERNAL_H_
#define BIGQUERY_EMULATOR_BACKEND_ENGINE_SEMANTIC_FUNCTIONS_AGGREGATE_TOP_INTERNAL_H_

#include <cstdint>
#include <map>
#include <string>
#include <vector>

#include "absl/status/statusor.h"
#include "backend/engine/semantic/value.h"
#include "googlesql/public/type.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace semantic {
namespace functions {
namespace aggregate_top_internal {

struct RowValue {
  Value v{};
  bool is_null = false;
};

struct TopCountEntry {
  Value key{};
  int64_t sum = 0;
  bool weight_was_non_null = false;
};

std::map<std::string, TopCountEntry> AccumulateTopCounts(
    const std::vector<RowValue>& rows);
std::map<std::string, TopCountEntry> AccumulateTopSums(
    const std::vector<std::vector<Value>>& input_column_values);
int64_t TopKLimitFromArgColumn(
    const std::vector<std::vector<Value>>& input_column_values,
    size_t arg_index,
    int64_t default_k);
std::vector<TopCountEntry> SortTopCountEntries(
    std::map<std::string, TopCountEntry> counts, int64_t k);
std::vector<TopCountEntry> SortTopSumEntries(
    std::map<std::string, TopCountEntry> sums, int64_t k);
absl::StatusOr<Value> BuildTopCountArray(
    const std::vector<TopCountEntry>& sorted,
    const ::googlesql::Type* return_type);
absl::StatusOr<Value> BuildTopSumArray(
    const std::vector<TopCountEntry>& sorted,
    const ::googlesql::Type* return_type);

}  // namespace aggregate_top_internal
}  // namespace functions
}  // namespace semantic
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator

#endif  // BIGQUERY_EMULATOR_BACKEND_ENGINE_SEMANTIC_FUNCTIONS_AGGREGATE_TOP_INTERNAL_H_
