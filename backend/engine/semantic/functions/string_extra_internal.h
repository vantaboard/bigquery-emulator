#ifndef BIGQUERY_EMULATOR_BACKEND_ENGINE_SEMANTIC_FUNCTIONS_STRING_EXTRA_INTERNAL_H_
#define BIGQUERY_EMULATOR_BACKEND_ENGINE_SEMANTIC_FUNCTIONS_STRING_EXTRA_INTERNAL_H_

#include <memory>
#include <string>
#include <vector>

#include "absl/status/statusor.h"
#include "absl/strings/string_view.h"
#include "backend/engine/semantic/value.h"
#include "googlesql/public/functions/hash.h"
#include "googlesql/public/functions/regexp.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace semantic {
namespace functions {
namespace string_extra_internal {

bool AnyNull(const std::vector<Value>& args);
absl::string_view AsStringOrBytes(const Value& v);
Value StringOrBytesFromView(const Value& template_value, absl::string_view out);
absl::StatusOr<std::unique_ptr<const ::googlesql::functions::RegExp>>
MakeRegExpForValue(const Value& pattern);
absl::StatusOr<Value> HashBytes(::googlesql::functions::Hasher::Algorithm algo,
                                const std::vector<Value>& args);
std::string EncodeBase32(absl::string_view input);
bool DecodeBase32(absl::string_view input,
                  std::string* out,
                  absl::Status* error);

}  // namespace string_extra_internal
}  // namespace functions
}  // namespace semantic
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator

#endif  // BIGQUERY_EMULATOR_BACKEND_ENGINE_SEMANTIC_FUNCTIONS_STRING_EXTRA_INTERNAL_H_
