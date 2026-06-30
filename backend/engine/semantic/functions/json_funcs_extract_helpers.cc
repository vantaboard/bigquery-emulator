#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "backend/engine/semantic/functions/json_funcs_extract_internal.h"
#include "backend/engine/semantic/value.h"
#include "googlesql/public/functions/json.h"
#include "googlesql/public/type.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace semantic {
namespace functions {
namespace json_extract_internal {

using ::googlesql::functions::JsonPathEvaluator;

absl::StatusOr<JsonArrayExtractResult> ExtractUnquotedStringArray(
    JsonPathEvaluator& evaluator, absl::string_view json_text) {
  JsonArrayExtractResult out;
  std::vector<std::optional<std::string>> elems;
  absl::Status st =
      evaluator.ExtractStringArray(json_text, &elems, &out.is_null);
  if (!st.ok()) return st;
  if (out.is_null) return out;
  out.values.reserve(elems.size());
  for (const auto& elem : elems) {
    if (!elem.has_value()) {
      out.values.push_back(Value::NullString());
    } else {
      out.values.push_back(Value::String(*elem));
    }
  }
  return out;
}

absl::StatusOr<JsonArrayExtractResult> ExtractRawJsonArray(
    JsonPathEvaluator& evaluator,
    absl::string_view json_text,
    const ::googlesql::ArrayType* arr_type) {
  JsonArrayExtractResult out;
  const ::googlesql::Type* elem_type = arr_type->element_type();
  const bool elem_is_json =
      elem_type != nullptr && elem_type->kind() == ::googlesql::TYPE_JSON;
  auto make_elem = [&](std::string text) -> Value {
    if (elem_is_json) {
      return Value::UnvalidatedJsonString(std::move(text));
    }
    return Value::String(std::move(text));
  };

  std::vector<std::string> elems;
  absl::Status st = evaluator.ExtractArray(json_text, &elems, &out.is_null);
  if (!st.ok()) return st;
  if (out.is_null) return out;
  out.values.reserve(elems.size());
  for (std::string& elem : elems) {
    out.values.push_back(make_elem(std::move(elem)));
  }
  return out;
}

}  // namespace json_extract_internal
}  // namespace functions
}  // namespace semantic
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
