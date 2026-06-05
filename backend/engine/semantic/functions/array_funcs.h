#ifndef BIGQUERY_EMULATOR_BACKEND_ENGINE_SEMANTIC_FUNCTIONS_ARRAY_FUNCS_H_
#define BIGQUERY_EMULATOR_BACKEND_ENGINE_SEMANTIC_FUNCTIONS_ARRAY_FUNCS_H_

#include <vector>

#include "absl/status/statusor.h"
#include "backend/engine/semantic/value.h"
#include "googlesql/public/type.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace semantic {
namespace functions {

absl::StatusOr<Value> GenerateArray(const std::vector<Value>& args,
                                    const ::googlesql::Type* return_type);
absl::StatusOr<Value> ArrayConcat(const std::vector<Value>& args,
                                  const ::googlesql::Type* return_type);
absl::StatusOr<Value> ArrayLength(const std::vector<Value>& args);
absl::StatusOr<Value> ArrayReverse(const std::vector<Value>& args,
                                   const ::googlesql::Type* return_type);
absl::StatusOr<Value> ArrayAtOffset(const std::vector<Value>& args,
                                    const ::googlesql::Type* return_type,
                                    bool safe = false);
absl::StatusOr<Value> ArrayAtOrdinal(const std::vector<Value>& args,
                                     const ::googlesql::Type* return_type,
                                     bool safe);

}  // namespace functions
}  // namespace semantic
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator

#endif  // BIGQUERY_EMULATOR_BACKEND_ENGINE_SEMANTIC_FUNCTIONS_ARRAY_FUNCS_H_
