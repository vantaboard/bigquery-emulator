#include <string>
#include <vector>

#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/string_view.h"
#include "backend/engine/semantic/functions/string_funcs.h"
#include "backend/engine/semantic/value.h"
#include "googlesql/public/functions/normalize_mode.pb.h"
#include "googlesql/public/functions/string.h"
#include "googlesql/public/type.pb.h"
#include "googlesql/public/value.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace semantic {
namespace functions {
namespace {

using ::googlesql::Value;

enum class SubstrTriState { kFalse, kTrue, kNullFromNullPath };

SubstrTriState ContainsSubstrRecursive(const Value& v,
                                       absl::string_view normalized_needle);

absl::StatusOr<std::string> NormalizeForContainsSubstr(
    absl::string_view input) {
  std::string out;
  absl::Status error;
  if (!::googlesql::functions::Normalize(
          input,
          ::googlesql::functions::NormalizeMode::NFKC,
          /*is_casefold=*/true,
          &out,
          &error)) {
    return error;
  }
  return out;
}

std::string ScalarToPlainText(const Value& v) {
  switch (v.type_kind()) {
    case ::googlesql::TYPE_BOOL:
      return v.bool_value() ? "true" : "false";
    case ::googlesql::TYPE_INT64:
      return std::to_string(v.int64_value());
    case ::googlesql::TYPE_DOUBLE:
      return absl::StrCat(v.double_value());
    case ::googlesql::TYPE_STRING:
      return std::string(v.string_value());
    case ::googlesql::TYPE_BYTES:
      return std::string(v.bytes_value());
    case ::googlesql::TYPE_JSON:
      return std::string(v.json_string());
    default:
      return std::string(v.DebugString());
  }
}

SubstrTriState SearchComposite(const Value& v,
                               absl::string_view normalized_needle,
                               bool is_array) {
  const int count = is_array ? v.num_elements() : v.num_fields();
  bool saw_null_without_match = false;
  for (int i = 0; i < count; ++i) {
    const Value& child = is_array ? v.element(i) : v.field(i);
    SubstrTriState child_result = SubstrTriState::kFalse;
    child_result = ContainsSubstrRecursive(child, normalized_needle);
    if (child_result == SubstrTriState::kTrue) {
      return SubstrTriState::kTrue;
    }
    if (child_result == SubstrTriState::kNullFromNullPath) {
      saw_null_without_match = true;
    }
  }
  return saw_null_without_match ? SubstrTriState::kNullFromNullPath
                                : SubstrTriState::kFalse;
}

SubstrTriState ContainsSubstrRecursive(const Value& v,
                                       absl::string_view normalized_needle) {
  if (v.is_null()) {
    return SubstrTriState::kNullFromNullPath;
  }
  if (v.type_kind() == ::googlesql::TYPE_ARRAY) {
    return SearchComposite(v, normalized_needle, /*is_array=*/true);
  }
  if (v.type_kind() == ::googlesql::TYPE_STRUCT) {
    return SearchComposite(v, normalized_needle, /*is_array=*/false);
  }

  auto normalized_haystack = NormalizeForContainsSubstr(ScalarToPlainText(v));
  if (!normalized_haystack.ok()) {
    return SubstrTriState::kFalse;
  }
  if (normalized_haystack->find(normalized_needle) != std::string::npos) {
    return SubstrTriState::kTrue;
  }
  return SubstrTriState::kFalse;
}

}  // namespace

absl::StatusOr<Value> ContainsSubstr(const std::vector<Value>& args) {
  if (args.size() < 2 || args.size() > 3) {
    return absl::InvalidArgumentError(
        "semantic: CONTAINS_SUBSTR expects two or three arguments");
  }
  if (args[1].is_null()) {
    return absl::InvalidArgumentError(
        "semantic: CONTAINS_SUBSTR search value cannot be NULL");
  }
  if (args[0].is_null()) {
    return Value::NullBool();
  }

  auto normalized_needle = NormalizeForContainsSubstr(args[1].string_value());
  if (!normalized_needle.ok()) {
    return normalized_needle.status();
  }

  SubstrTriState result = SubstrTriState::kFalse;
  result = ContainsSubstrRecursive(args[0], *normalized_needle);
  if (result == SubstrTriState::kTrue) {
    return Value::Bool(true);
  }
  if (result == SubstrTriState::kNullFromNullPath) {
    return Value::NullBool();
  }
  return Value::Bool(false);
}

}  // namespace functions
}  // namespace semantic
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
