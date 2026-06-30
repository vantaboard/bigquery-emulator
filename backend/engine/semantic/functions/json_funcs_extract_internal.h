#ifndef BIGQUERY_EMULATOR_BACKEND_ENGINE_SEMANTIC_FUNCTIONS_JSON_FUNCS_EXTRACT_INTERNAL_H_
#define BIGQUERY_EMULATOR_BACKEND_ENGINE_SEMANTIC_FUNCTIONS_JSON_FUNCS_EXTRACT_INTERNAL_H_

#include <string>
#include <vector>

#include "absl/status/statusor.h"
#include "absl/strings/string_view.h"
#include "backend/engine/semantic/value.h"
#include "googlesql/public/functions/json.h"
#include "googlesql/public/type.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace semantic {
namespace functions {
namespace json_extract_internal {

struct JsonArrayExtractResult {
  bool is_null = false;
  std::vector<Value> values{};
};

absl::StatusOr<JsonArrayExtractResult> ExtractUnquotedStringArray(
    ::googlesql::functions::JsonPathEvaluator& evaluator,
    absl::string_view json_text);

absl::StatusOr<JsonArrayExtractResult> ExtractRawJsonArray(
    ::googlesql::functions::JsonPathEvaluator& evaluator,
    absl::string_view json_text,
    const ::googlesql::ArrayType* arr_type);

}  // namespace json_extract_internal
}  // namespace functions
}  // namespace semantic
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator

#endif  // BIGQUERY_EMULATOR_BACKEND_ENGINE_SEMANTIC_FUNCTIONS_JSON_FUNCS_EXTRACT_INTERNAL_H_
