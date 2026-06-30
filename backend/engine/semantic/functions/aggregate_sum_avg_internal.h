#ifndef BIGQUERY_EMULATOR_BACKEND_ENGINE_SEMANTIC_FUNCTIONS_AGGREGATE_SUM_AVG_INTERNAL_H_
#define BIGQUERY_EMULATOR_BACKEND_ENGINE_SEMANTIC_FUNCTIONS_AGGREGATE_SUM_AVG_INTERNAL_H_

#include <vector>

#include "absl/status/statusor.h"
#include "backend/engine/semantic/value.h"
#include "googlesql/public/type.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace semantic {
namespace functions {
namespace aggregate_sum_avg_internal {

absl::StatusOr<Value> NullOfAggregateType(const ::googlesql::Type* type);
absl::StatusOr<Value> SumNumericAggregateCells(
    const ::googlesql::Type* return_type, const std::vector<Value>& cells);
absl::StatusOr<Value> SumInt64Cells(const ::googlesql::Type* return_type,
                                    const std::vector<Value>& cells);
absl::StatusOr<Value> SumDoubleCells(const ::googlesql::Type* return_type,
                                     const std::vector<Value>& cells);
absl::StatusOr<Value> SumNumericCells(const ::googlesql::Type* return_type,
                                      const std::vector<Value>& cells);
absl::StatusOr<Value> AvgInt64Cells(const ::googlesql::Type* return_type,
                                    const std::vector<Value>& cells);
absl::StatusOr<Value> AvgDoubleCells(const ::googlesql::Type* return_type,
                                     const std::vector<Value>& cells);
absl::StatusOr<Value> AvgNumericCells(const ::googlesql::Type* return_type,
                                      const std::vector<Value>& cells);
absl::StatusOr<Value> AvgBigNumericCells(const ::googlesql::Type* return_type,
                                         const std::vector<Value>& cells);
bool ShouldReplaceMinMax(const Value& cur, const Value& v, bool pick_max);
bool IsSupportedMinMaxType(::googlesql::TypeKind kind);

}  // namespace aggregate_sum_avg_internal
}  // namespace functions
}  // namespace semantic
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator

#endif  // BIGQUERY_EMULATOR_BACKEND_ENGINE_SEMANTIC_FUNCTIONS_AGGREGATE_SUM_AVG_INTERNAL_H_
