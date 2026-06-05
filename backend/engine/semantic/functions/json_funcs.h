#ifndef BIGQUERY_EMULATOR_BACKEND_ENGINE_SEMANTIC_FUNCTIONS_JSON_FUNCS_H_
#define BIGQUERY_EMULATOR_BACKEND_ENGINE_SEMANTIC_FUNCTIONS_JSON_FUNCS_H_

#include <vector>

#include "absl/status/statusor.h"
#include "backend/engine/semantic/value.h"
#include "googlesql/public/type.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace semantic {
namespace functions {

// JSON extract/query/value/scalar helpers (docs/ENGINE_POLICY.md).
absl::StatusOr<Value> JsonExtract(const std::vector<Value>& args,
                                  const ::googlesql::Type* return_type);
absl::StatusOr<Value> JsonQuery(const std::vector<Value>& args,
                                const ::googlesql::Type* return_type);
absl::StatusOr<Value> JsonValue(const std::vector<Value>& args,
                                const ::googlesql::Type* return_type);
absl::StatusOr<Value> JsonExtractScalar(const std::vector<Value>& args,
                                        const ::googlesql::Type* return_type);

absl::StatusOr<Value> JsonExtractArray(const std::vector<Value>& args,
                                       const ::googlesql::Type* return_type);
absl::StatusOr<Value> JsonExtractStringArray(
    const std::vector<Value>& args, const ::googlesql::Type* return_type);
absl::StatusOr<Value> JsonQueryArray(const std::vector<Value>& args,
                                     const ::googlesql::Type* return_type);
absl::StatusOr<Value> JsonValueArray(const std::vector<Value>& args,
                                     const ::googlesql::Type* return_type);

absl::StatusOr<Value> ParseJson(const std::vector<Value>& args);
absl::StatusOr<Value> ToJsonString(const std::vector<Value>& args);

// JSON -> scalar cast helpers invoked as BOOL(JSON '...'), INT64(...), etc.
absl::StatusOr<Value> JsonCastBool(const std::vector<Value>& args);
absl::StatusOr<Value> JsonCastInt64(const std::vector<Value>& args);
absl::StatusOr<Value> JsonCastFloat64(const std::vector<Value>& args);
absl::StatusOr<Value> JsonCastString(const std::vector<Value>& args);

absl::StatusOr<Value> JsonTypeFunc(const std::vector<Value>& args);
absl::StatusOr<Value> JsonSubscript(const std::vector<Value>& args,
                                    const ::googlesql::Type* return_type);
absl::StatusOr<Value> JsonGetField(const Value& base,
                                   absl::string_view field_name,
                                   const ::googlesql::Type* return_type);

}  // namespace functions
}  // namespace semantic
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator

#endif  // BIGQUERY_EMULATOR_BACKEND_ENGINE_SEMANTIC_FUNCTIONS_JSON_FUNCS_H_
