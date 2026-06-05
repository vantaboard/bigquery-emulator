#ifndef BIGQUERY_EMULATOR_BACKEND_ENGINE_SEMANTIC_FUNCTIONS_OPERATOR_FUNCS_H_
#define BIGQUERY_EMULATOR_BACKEND_ENGINE_SEMANTIC_FUNCTIONS_OPERATOR_FUNCS_H_

#include <vector>

#include "absl/status/statusor.h"
#include "absl/strings/string_view.h"
#include "backend/engine/semantic/value.h"
#include "googlesql/public/type.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace semantic {
namespace functions {

absl::StatusOr<Value> DispatchLike(absl::string_view name,
                                   const std::vector<Value>& args);
absl::StatusOr<Value> DispatchBetween(absl::string_view name,
                                      const std::vector<Value>& args);
absl::StatusOr<Value> DispatchIn(absl::string_view name,
                                 const std::vector<Value>& args);
absl::StatusOr<Value> DispatchIsTrue(absl::string_view name,
                                     const std::vector<Value>& args);
absl::StatusOr<Value> DispatchIsFalse(absl::string_view name,
                                      const std::vector<Value>& args);
absl::StatusOr<Value> DispatchIsDistinctFrom(absl::string_view name,
                                             const std::vector<Value>& args);
absl::StatusOr<Value> DispatchBitwise(absl::string_view name,
                                      const std::vector<Value>& args);
absl::StatusOr<Value> DispatchInterval(const std::vector<Value>& args,
                                       const ::googlesql::Type* return_type);
absl::StatusOr<Value> JustifyDays(const std::vector<Value>& args);
absl::StatusOr<Value> JustifyHours(const std::vector<Value>& args);
absl::StatusOr<Value> Round(const std::vector<Value>& args);

}  // namespace functions
}  // namespace semantic
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator

#endif  // BIGQUERY_EMULATOR_BACKEND_ENGINE_SEMANTIC_FUNCTIONS_OPERATOR_FUNCS_H_
