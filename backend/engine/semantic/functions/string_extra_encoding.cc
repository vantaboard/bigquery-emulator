#include "backend/engine/semantic/functions/string_extra_internal.h"
#include "backend/engine/semantic/functions/string_funcs.h"

#include <cctype>
#include <cstring>
#include <memory>
#include <string>
#include <vector>

#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "absl/strings/ascii.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/string_view.h"
#include "backend/engine/semantic/error.h"
#include "backend/engine/semantic/eval_context.h"
#include "backend/engine/semantic/value.h"
#include "googlesql/public/functions/hash.h"
#include "googlesql/public/functions/normalize_mode.pb.h"
#include "googlesql/public/functions/numeric.h"
#include "googlesql/public/functions/regexp.h"
#include "googlesql/public/functions/string.h"
#include "googlesql/public/functions/string_format.h"
#include "googlesql/public/numeric_value.h"
#include "googlesql/public/options.pb.h"
#include "googlesql/public/type.h"
#include "googlesql/public/type.pb.h"
#include "googlesql/public/value.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace semantic {
namespace functions {

using ::googlesql::functions::Hasher;
using string_extra_internal::AsStringOrBytes;
using string_extra_internal::DecodeBase32;
using string_extra_internal::EncodeBase32;
using string_extra_internal::HashBytes;

absl::StatusOr<Value> ToHex(const std::vector<Value>& args) {
  if (args.size() != 1) {
    return absl::InvalidArgumentError("semantic: TO_HEX expects one argument");
  }
  if (args[0].is_null()) return Value::NullString();
  absl::Status error;
  std::string out;
  if (!::googlesql::functions::ToHex(AsStringOrBytes(args[0]), &out, &error)) {
    return error;
  }
  return Value::String(std::move(out));
}

absl::StatusOr<Value> FromHex(const std::vector<Value>& args) {
  if (args.size() != 1) {
    return absl::InvalidArgumentError(
        "semantic: FROM_HEX expects one argument");
  }
  if (args[0].is_null()) return Value::NullBytes();
  absl::Status error;
  std::string out;
  if (!::googlesql::functions::FromHex(args[0].string_value(), &out, &error)) {
    return error;
  }
  return Value::Bytes(std::move(out));
}

absl::StatusOr<Value> ToBase64(const std::vector<Value>& args) {
  if (args.size() != 1) {
    return absl::InvalidArgumentError(
        "semantic: TO_BASE64 expects one argument");
  }
  if (args[0].is_null()) return Value::NullString();
  absl::Status error;
  std::string out;
  if (!::googlesql::functions::ToBase64(args[0].bytes_value(), &out, &error)) {
    return error;
  }
  return Value::String(std::move(out));
}

absl::StatusOr<Value> FromBase64(const std::vector<Value>& args) {
  if (args.size() != 1) {
    return absl::InvalidArgumentError(
        "semantic: FROM_BASE64 expects one argument");
  }
  if (args[0].is_null()) return Value::NullBytes();
  absl::Status error;
  std::string out;
  if (!::googlesql::functions::FromBase64(
          args[0].string_value(), &out, &error)) {
    return error;
  }
  return Value::Bytes(std::move(out));
}

absl::StatusOr<Value> ToBase32(const std::vector<Value>& args) {
  if (args.size() != 1) {
    return absl::InvalidArgumentError(
        "semantic: TO_BASE32 expects one argument");
  }
  if (args[0].is_null()) return Value::NullString();
  return Value::String(EncodeBase32(args[0].bytes_value()));
}

absl::StatusOr<Value> FromBase32(const std::vector<Value>& args) {
  if (args.size() != 1) {
    return absl::InvalidArgumentError(
        "semantic: FROM_BASE32 expects one argument");
  }
  if (args[0].is_null()) return Value::NullBytes();
  absl::Status error;
  std::string out;
  if (!DecodeBase32(args[0].string_value(), &out, &error)) {
    return error;
  }
  return Value::Bytes(std::move(out));
}

absl::StatusOr<Value> Md5(const std::vector<Value>& args) {
  return HashBytes(Hasher::kMd5, args);
}
absl::StatusOr<Value> Sha1(const std::vector<Value>& args) {
  return HashBytes(Hasher::kSha1, args);
}
absl::StatusOr<Value> Sha256(const std::vector<Value>& args) {
  return HashBytes(Hasher::kSha256, args);
}
absl::StatusOr<Value> Sha512(const std::vector<Value>& args) {
  return HashBytes(Hasher::kSha512, args);
}

absl::StatusOr<Value> FarmFingerprintFunc(const std::vector<Value>& args) {
  if (args.size() != 1) {
    return absl::InvalidArgumentError(
        "semantic: FARM_FINGERPRINT expects one argument");
  }
  if (args[0].is_null()) return Value::NullInt64();
  absl::string_view input;
  if (args[0].type_kind() == ::googlesql::TYPE_STRING) {
    input = args[0].string_value();
  } else if (args[0].type_kind() == ::googlesql::TYPE_BYTES) {
    input = args[0].bytes_value();
  } else {
    return absl::InvalidArgumentError(
        "semantic: FARM_FINGERPRINT argument must be STRING or BYTES");
  }
  return Value::Int64(::googlesql::functions::FarmFingerprint(input));
}


}  // namespace functions
}  // namespace semantic
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
