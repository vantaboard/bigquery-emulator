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
namespace string_extra_internal {

using ::googlesql::NumericValue;
using ::googlesql::ProductMode;
using ::googlesql::functions::Hasher;
using ::googlesql::functions::MakeRegExpBytes;
using ::googlesql::functions::MakeRegExpUtf8;
using ::googlesql::functions::RegExp;
using ::googlesql::functions::StringFormatUtf8;

bool AnyNull(const std::vector<Value>& args) {
  for (const Value& v : args) {
    if (v.is_null()) return true;
  }
  return false;
}

absl::string_view AsStringOrBytes(const Value& v) {
  return v.type_kind() == ::googlesql::TYPE_BYTES ? v.bytes_value()
                                                  : v.string_value();
}

Value StringOrBytesFromView(const Value& template_value,
                            absl::string_view out) {
  if (template_value.type_kind() == ::googlesql::TYPE_BYTES) {
    return Value::Bytes(std::string(out));
  }
  return Value::String(std::string(out));
}

absl::StatusOr<std::unique_ptr<const RegExp>> MakeRegExpForValue(
    const Value& pattern) {
  if (pattern.type_kind() == ::googlesql::TYPE_BYTES) {
    return MakeRegExpBytes(pattern.bytes_value());
  }
  return MakeRegExpUtf8(pattern.string_value());
}

std::string EncodeBase32(absl::string_view input) {
  static const char* kAlphabet = "ABCDEFGHIJKLMNOPQRSTUVWXYZ234567";
  std::string out;
  int buffer = 0;
  int bits = 0;
  for (unsigned char c : input) {
    buffer = (buffer << 8) | c;
    bits += 8;
    while (bits >= 5) {
      bits -= 5;
      out.push_back(kAlphabet[(buffer >> bits) & 0x1F]);
    }
  }
  if (bits > 0) {
    out.push_back(kAlphabet[(buffer << (5 - bits)) & 0x1F]);
  }
  while (out.size() % 8 != 0) {
    out.push_back('=');
  }
  return out;
}

bool DecodeBase32(absl::string_view input,
                  std::string* out,
                  absl::Status* error) {
  static const char* kAlphabet = "ABCDEFGHIJKLMNOPQRSTUVWXYZ234567";
  int buffer = 0;
  int bits = 0;
  for (char c : input) {
    if (c == '=') break;
    const char* p = std::strchr(kAlphabet, static_cast<char>(std::toupper(c)));
    if (p == nullptr) {
      *error = absl::InvalidArgumentError("semantic: invalid base32 input");
      return false;
    }
    buffer = (buffer << 5) | static_cast<int>(p - kAlphabet);
    bits += 5;
    if (bits >= 8) {
      bits -= 8;
      out->push_back(static_cast<char>((buffer >> bits) & 0xFF));
    }
  }
  return true;
}

absl::StatusOr<Value> HashBytes(Hasher::Algorithm algo,
                                const std::vector<Value>& args) {
  if (args.size() != 1) {
    return absl::InvalidArgumentError("semantic: hash expects one argument");
  }
  if (args[0].is_null()) return Value::NullBytes();
  absl::string_view input = AsStringOrBytes(args[0]);
  auto hasher = Hasher::Create(algo);
  if (hasher == nullptr) {
    return absl::InternalError("semantic: hash algorithm unavailable");
  }
  return Value::Bytes(hasher->Hash(input));
}

}  // namespace string_extra_internal
}  // namespace functions
}  // namespace semantic
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
