#ifndef BIGQUERY_EMULATOR_BACKEND_ENGINE_SEMANTIC_FUNCTIONS_RANGE_FUNCS_H_
#define BIGQUERY_EMULATOR_BACKEND_ENGINE_SEMANTIC_FUNCTIONS_RANGE_FUNCS_H_

#include <vector>

#include "absl/status/statusor.h"
#include "backend/engine/semantic/value.h"
#include "googlesql/public/type.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace semantic {
namespace functions {

absl::StatusOr<Value> RangeCtor(const std::vector<Value>& args,
                                const ::googlesql::Type* return_type);
absl::StatusOr<Value> RangeStart(const std::vector<Value>& args);
absl::StatusOr<Value> RangeEnd(const std::vector<Value>& args);
absl::StatusOr<Value> RangeOverlaps(const std::vector<Value>& args);

}  // namespace functions
}  // namespace semantic
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator

#endif  // BIGQUERY_EMULATOR_BACKEND_ENGINE_SEMANTIC_FUNCTIONS_RANGE_FUNCS_H_
