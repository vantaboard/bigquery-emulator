#include "backend/engine/semantic/eval_context.h"
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

namespace {

using ::googlesql::BigNumericValue;
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

bool DecodeBase32(absl::string_view input, std::string* out,
                  absl::Status* error) {
  static const char* kAlphabet = "ABCDEFGHIJKLMNOPQRSTUVWXYZ234567";
  int buffer = 0;
  int bits = 0;
  for (char c : input) {
    if (c == '=') break;
    const char* p =
        std::strchr(kAlphabet, static_cast<char>(std::toupper(c)));
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

}  // namespace

absl::StatusOr<Value> Ascii(const std::vector<Value>& args) {
  if (args.size() != 1) {
    return absl::InvalidArgumentError("semantic: ASCII expects one argument");
  }
  if (args[0].is_null()) return Value::NullInt64();
  absl::Status error;
  int64_t out = 0;
  if (args[0].type_kind() == ::googlesql::TYPE_BYTES) {
    if (args[0].bytes_value().empty()) return Value::Int64(0);
    if (!::googlesql::functions::FirstByteOfBytesToASCII(args[0].bytes_value(),
                                                         &out, &error)) {
      return error;
    }
    return Value::Int64(out);
  }
  if (args[0].string_value().empty()) return Value::Int64(0);
  if (!::googlesql::functions::FirstCharOfStringToASCII(args[0].string_value(),
                                                        &out, &error)) {
    return error;
  }
  return Value::Int64(out);
}

absl::StatusOr<Value> ByteLength(const std::vector<Value>& args) {
  if (args.size() != 1) {
    return absl::InvalidArgumentError(
        "semantic: BYTE_LENGTH expects one argument");
  }
  if (args[0].is_null()) return Value::NullInt64();
  absl::Status error;
  int64_t out = 0;
  if (args[0].type_kind() == ::googlesql::TYPE_BYTES) {
    if (!::googlesql::functions::LengthBytes(args[0].bytes_value(), &out,
                                             &error)) {
      return error;
    }
  } else {
    if (!::googlesql::functions::LengthBytes(args[0].string_value(), &out,
                                             &error)) {
      return error;
    }
  }
  return Value::Int64(out);
}

absl::StatusOr<Value> Length(const std::vector<Value>& args) {
  if (args.size() != 1) {
    return absl::InvalidArgumentError("semantic: LENGTH expects one argument");
  }
  if (args[0].is_null()) return Value::NullInt64();
  absl::Status error;
  int64_t out = 0;
  if (args[0].type_kind() == ::googlesql::TYPE_BYTES) {
    if (!::googlesql::functions::LengthBytes(args[0].bytes_value(), &out,
                                             &error)) {
      return error;
    }
  } else {
    if (!::googlesql::functions::LengthUtf8(args[0].string_value(), &out,
                                            &error)) {
      return error;
    }
  }
  return Value::Int64(out);
}

absl::StatusOr<Value> CharLength(const std::vector<Value>& args) {
  return Length(args);
}

absl::StatusOr<Value> OctetLength(const std::vector<Value>& args) {
  return ByteLength(args);
}

absl::StatusOr<Value> Chr(const std::vector<Value>& args) {
  if (args.size() != 1) {
    return absl::InvalidArgumentError("semantic: CHR expects one argument");
  }
  if (args[0].is_null()) return Value::NullString();
  if (args[0].type_kind() != ::googlesql::TYPE_INT64) {
    return absl::InvalidArgumentError("semantic: CHR expects INT64");
  }
  if (args[0].int64_value() == 0) return Value::String("");
  absl::Status error;
  std::string out;
  if (!::googlesql::functions::CodePointToString(args[0].int64_value(), &out,
                                                 &error)) {
    return error;
  }
  return Value::String(std::move(out));
}

absl::StatusOr<Value> Unicode(const std::vector<Value>& args) {
  if (args.size() != 1) {
    return absl::InvalidArgumentError("semantic: UNICODE expects one argument");
  }
  if (args[0].is_null()) return Value::NullInt64();
  if (args[0].string_value().empty()) return Value::Int64(0);
  absl::Status error;
  int64_t out = 0;
  if (!::googlesql::functions::FirstCharToCodePoint(args[0].string_value(),
                                                    &out, &error)) {
    return error;
  }
  return Value::Int64(out);
}

absl::StatusOr<Value> Concat(const std::vector<Value>& args) {
  if (args.empty()) {
    return absl::InvalidArgumentError("semantic: CONCAT expects arguments");
  }
  if (AnyNull(args)) return Value::NullString();
  std::string out;
  for (const Value& v : args) {
    switch (v.type_kind()) {
      case ::googlesql::TYPE_STRING:
        absl::StrAppend(&out, v.string_value());
        break;
      case ::googlesql::TYPE_BYTES:
        absl::StrAppend(&out, v.bytes_value());
        break;
      case ::googlesql::TYPE_INT64:
        absl::StrAppend(&out, v.int64_value());
        break;
      case ::googlesql::TYPE_DOUBLE:
        absl::StrAppend(&out, v.double_value());
        break;
      case ::googlesql::TYPE_BOOL:
        absl::StrAppend(&out, v.bool_value() ? "true" : "false");
        break;
      default:
        absl::StrAppend(&out, v.DebugString());
        break;
    }
  }
  return Value::String(std::move(out));
}

absl::StatusOr<Value> Lower(const std::vector<Value>& args) {
  if (args.size() != 1) {
    return absl::InvalidArgumentError("semantic: LOWER expects one argument");
  }
  if (args[0].is_null()) return Value::NullString();
  absl::Status error;
  std::string out;
  if (args[0].type_kind() == ::googlesql::TYPE_BYTES) {
    if (!::googlesql::functions::LowerBytes(args[0].bytes_value(), &out,
                                            &error)) {
      return error;
    }
    return Value::Bytes(std::move(out));
  }
  if (!::googlesql::functions::LowerUtf8(args[0].string_value(), &out,
                                         &error)) {
    return error;
  }
  return Value::String(std::move(out));
}

absl::StatusOr<Value> Upper(const std::vector<Value>& args) {
  if (args.size() != 1) {
    return absl::InvalidArgumentError("semantic: UPPER expects one argument");
  }
  if (args[0].is_null()) return Value::NullString();
  absl::Status error;
  std::string out;
  if (args[0].type_kind() == ::googlesql::TYPE_BYTES) {
    if (!::googlesql::functions::UpperBytes(args[0].bytes_value(), &out,
                                            &error)) {
      return error;
    }
    return Value::Bytes(std::move(out));
  }
  if (!::googlesql::functions::UpperUtf8(args[0].string_value(), &out,
                                         &error)) {
    return error;
  }
  return Value::String(std::move(out));
}

absl::StatusOr<Value> TrimFamily(absl::string_view name,
                                 const std::vector<Value>& args) {
  if (args.empty() || args.size() > 2) {
    return absl::InvalidArgumentError(absl::StrCat(
        "semantic: ", name, " expects one or two arguments"));
  }
  if (args[0].is_null()) return Value::NullString();
  if (args.size() == 2 && args[1].is_null()) return Value::NullString();
  absl::Status error;
  absl::string_view out;
  const bool is_bytes = args[0].type_kind() == ::googlesql::TYPE_BYTES;
  if (args.size() == 1) {
    if (is_bytes) {
      return MakeSemanticError(SemanticErrorReason::kNotImplemented,
                               "semantic: TRIM on BYTES without chars NYI");
    }
    if (name == "ltrim") {
      if (!::googlesql::functions::LeftTrimSpacesUtf8(args[0].string_value(),
                                                      &out, &error)) {
        return error;
      }
    } else if (name == "rtrim") {
      if (!::googlesql::functions::RightTrimSpacesUtf8(args[0].string_value(),
                                                       &out, &error)) {
        return error;
      }
    } else if (!::googlesql::functions::TrimSpacesUtf8(args[0].string_value(),
                                                       &out, &error)) {
      return error;
    }
    return Value::String(std::string(out));
  }
  if (is_bytes) {
    if (name == "ltrim") {
      if (!::googlesql::functions::LeftTrimBytes(args[0].bytes_value(),
                                                 args[1].bytes_value(), &out,
                                                 &error)) {
        return error;
      }
    } else if (name == "rtrim") {
      if (!::googlesql::functions::RightTrimBytes(args[0].bytes_value(),
                                                  args[1].bytes_value(), &out,
                                                  &error)) {
        return error;
      }
    } else if (!::googlesql::functions::TrimBytes(args[0].bytes_value(),
                                                  args[1].bytes_value(), &out,
                                                  &error)) {
      return error;
    }
    return Value::Bytes(std::string(out));
  }
  if (name == "ltrim") {
    if (!::googlesql::functions::LeftTrimUtf8(args[0].string_value(),
                                              args[1].string_value(), &out,
                                              &error)) {
      return error;
    }
  } else if (name == "rtrim") {
    if (!::googlesql::functions::RightTrimUtf8(args[0].string_value(),
                                               args[1].string_value(), &out,
                                               &error)) {
      return error;
    }
  } else if (!::googlesql::functions::TrimUtf8(args[0].string_value(),
                                               args[1].string_value(), &out,
                                               &error)) {
    return error;
  }
  return Value::String(std::string(out));
}

absl::StatusOr<Value> Trim(const std::vector<Value>& args) {
  return TrimFamily("trim", args);
}
absl::StatusOr<Value> Ltrim(const std::vector<Value>& args) {
  return TrimFamily("ltrim", args);
}
absl::StatusOr<Value> Rtrim(const std::vector<Value>& args) {
  return TrimFamily("rtrim", args);
}

absl::StatusOr<Value> Replace(const std::vector<Value>& args) {
  if (args.size() != 3) {
    return absl::InvalidArgumentError(
        "semantic: REPLACE expects three arguments");
  }
  if (AnyNull(args)) {
    if (args[0].type_kind() == ::googlesql::TYPE_BYTES) {
      return Value::NullBytes();
    }
    return Value::NullString();
  }
  absl::Status error;
  std::string out;
  if (args[0].type_kind() == ::googlesql::TYPE_BYTES) {
    if (!::googlesql::functions::ReplaceBytes(
            args[0].bytes_value(), args[1].bytes_value(), args[2].bytes_value(),
            &out, &error)) {
      return error;
    }
    return Value::Bytes(std::move(out));
  }
  if (!::googlesql::functions::ReplaceUtf8(args[0].string_value(),
                                           args[1].string_value(),
                                           args[2].string_value(), &out,
                                           &error)) {
    return error;
  }
  return Value::String(std::move(out));
}

absl::StatusOr<Value> Reverse(const std::vector<Value>& args) {
  if (args.size() != 1) {
    return absl::InvalidArgumentError("semantic: REVERSE expects one argument");
  }
  if (args[0].is_null()) {
    return args[0].type_kind() == ::googlesql::TYPE_BYTES ? Value::NullBytes()
                                                          : Value::NullString();
  }
  absl::Status error;
  std::string out;
  if (args[0].type_kind() == ::googlesql::TYPE_BYTES) {
    if (!::googlesql::functions::ReverseBytes(args[0].bytes_value(), &out,
                                              &error)) {
      return error;
    }
    return Value::Bytes(std::move(out));
  }
  if (!::googlesql::functions::ReverseUtf8(args[0].string_value(), &out,
                                           &error)) {
    return error;
  }
  return Value::String(std::move(out));
}

absl::StatusOr<Value> StartsWith(const std::vector<Value>& args) {
  if (args.size() != 2) {
    return absl::InvalidArgumentError(
        "semantic: STARTS_WITH expects two arguments");
  }
  if (args[0].is_null() || args[1].is_null()) return Value::NullBool();
  absl::Status error;
  bool out = false;
  if (args[0].type_kind() == ::googlesql::TYPE_BYTES) {
    if (!::googlesql::functions::StartsWithBytes(args[0].bytes_value(),
                                                 args[1].bytes_value(), &out,
                                                 &error)) {
      return error;
    }
  } else if (!::googlesql::functions::StartsWithUtf8(
                 args[0].string_value(), args[1].string_value(), &out, &error)) {
    return error;
  }
  return Value::Bool(out);
}

absl::StatusOr<Value> EndsWith(const std::vector<Value>& args) {
  if (args.size() != 2) {
    return absl::InvalidArgumentError(
        "semantic: ENDS_WITH expects two arguments");
  }
  if (args[0].is_null() || args[1].is_null()) return Value::NullBool();
  absl::Status error;
  bool out = false;
  if (args[0].type_kind() == ::googlesql::TYPE_BYTES) {
    if (!::googlesql::functions::EndsWithBytes(args[0].bytes_value(),
                                               args[1].bytes_value(), &out,
                                               &error)) {
      return error;
    }
  } else if (!::googlesql::functions::EndsWithUtf8(args[0].string_value(),
                                                   args[1].string_value(), &out,
                                                   &error)) {
    return error;
  }
  return Value::Bool(out);
}

absl::StatusOr<Value> Left(const std::vector<Value>& args) {
  if (args.size() != 2) {
    return absl::InvalidArgumentError("semantic: LEFT expects two arguments");
  }
  if (args[0].is_null() || args[1].is_null()) {
    if (args[0].type_kind() == ::googlesql::TYPE_BYTES) {
      return Value::NullBytes();
    }
    return Value::NullString();
  }
  absl::Status error;
  absl::string_view out;
  if (args[0].type_kind() == ::googlesql::TYPE_BYTES) {
    if (!::googlesql::functions::LeftBytes(args[0].bytes_value(),
                                           args[1].int64_value(), &out,
                                           &error)) {
      return error;
    }
    return Value::Bytes(std::string(out));
  }
  if (!::googlesql::functions::LeftUtf8(args[0].string_value(),
                                        args[1].int64_value(), &out, &error)) {
    return error;
  }
  return Value::String(std::string(out));
}

absl::StatusOr<Value> Substr(const std::vector<Value>& args) {
  if (args.size() < 2 || args.size() > 3) {
    return absl::InvalidArgumentError(
        "semantic: SUBSTR expects two or three arguments");
  }
  if (AnyNull(args)) {
    if (args[0].type_kind() == ::googlesql::TYPE_BYTES) {
      return Value::NullBytes();
    }
    return Value::NullString();
  }
  absl::Status error;
  absl::string_view out;
  const int64_t pos = args[1].int64_value();
  if (args[0].type_kind() == ::googlesql::TYPE_BYTES) {
    if (args.size() == 2) {
      if (!::googlesql::functions::SubstrBytes(args[0].bytes_value(), pos, &out,
                                               &error)) {
        return error;
      }
    } else if (!::googlesql::functions::SubstrWithLengthBytes(
                   args[0].bytes_value(), pos, args[2].int64_value(), &out,
                   &error)) {
      return error;
    }
    return Value::Bytes(std::string(out));
  }
  if (args.size() == 2) {
    if (!::googlesql::functions::SubstrUtf8(args[0].string_value(), pos, &out,
                                            &error)) {
      return error;
    }
  } else if (!::googlesql::functions::SubstrWithLengthUtf8(
                 args[0].string_value(), pos, args[2].int64_value(), &out,
                 &error)) {
    return error;
  }
  return Value::String(std::string(out));
}

absl::StatusOr<Value> Strpos(const std::vector<Value>& args) {
  if (args.size() != 2) {
    return absl::InvalidArgumentError("semantic: STRPOS expects two arguments");
  }
  if (args[0].is_null() || args[1].is_null()) return Value::NullInt64();
  absl::Status error;
  int64_t out = 0;
  if (args[0].type_kind() == ::googlesql::TYPE_BYTES) {
    if (!::googlesql::functions::StrposBytes(args[0].bytes_value(),
                                             args[1].bytes_value(), &out,
                                             &error)) {
      return error;
    }
  } else if (!::googlesql::functions::StrposUtf8(args[0].string_value(),
                                                 args[1].string_value(), &out,
                                                 &error)) {
    return error;
  }
  return Value::Int64(out);
}

absl::StatusOr<Value> Split(const std::vector<Value>& args,
                            const ::googlesql::Type* return_type) {
  if (args.empty() || args.size() > 2) {
    return absl::InvalidArgumentError(
        "semantic: SPLIT expects one or two arguments");
  }
  if (args[0].is_null()) return Value::Array(return_type->AsArray(), {});
  absl::string_view delim = ",";
  if (args.size() == 2) {
    if (args[1].is_null()) {
      return Value::Array(return_type->AsArray(), {});
    }
    delim = AsStringOrBytes(args[1]);
  }
  absl::Status error;
  std::vector<std::string> parts;
  if (args[0].type_kind() == ::googlesql::TYPE_BYTES) {
    if (!::googlesql::functions::SplitBytes(args[0].bytes_value(), delim,
                                            &parts, &error)) {
      return error;
    }
    std::vector<Value> elements;
    elements.reserve(parts.size());
    for (const std::string& p : parts) {
      elements.push_back(Value::Bytes(p));
    }
    return Value::Array(return_type->AsArray(), elements);
  }
  if (!::googlesql::functions::SplitUtf8(args[0].string_value(), delim, &parts,
                                         &error)) {
    return error;
  }
  std::vector<Value> elements;
  elements.reserve(parts.size());
  for (const std::string& p : parts) {
    elements.push_back(Value::String(p));
  }
  return Value::Array(return_type->AsArray(), elements);
}

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
    return absl::InvalidArgumentError("semantic: FROM_HEX expects one argument");
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
    return absl::InvalidArgumentError("semantic: TO_BASE64 expects one argument");
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
  if (!::googlesql::functions::FromBase64(args[0].string_value(), &out,
                                          &error)) {
    return error;
  }
  return Value::Bytes(std::move(out));
}

absl::StatusOr<Value> ToBase32(const std::vector<Value>& args) {
  if (args.size() != 1) {
    return absl::InvalidArgumentError("semantic: TO_BASE32 expects one argument");
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

absl::StatusOr<Value> InitcapFunc(const std::vector<Value>& args) {
  if (args.empty() || args.size() > 2) {
    return absl::InvalidArgumentError(
        "semantic: INITCAP expects one or two arguments");
  }
  if (AnyNull(args)) return Value::NullString();
  if (args[0].type_kind() != ::googlesql::TYPE_STRING) {
    return absl::InvalidArgumentError("semantic: INITCAP value must be STRING");
  }
  absl::Status error;
  std::string out;
  if (args.size() == 1) {
    if (!::googlesql::functions::InitialCapitalizeDefault(
            args[0].string_value(), &out, &error)) {
      return error;
    }
  } else {
    absl::string_view delimiters = args[1].string_value();
    if (!::googlesql::functions::InitialCapitalize(args[0].string_value(),
                                                   delimiters, &out, &error)) {
      return error;
    }
  }
  return Value::String(std::move(out));
}

absl::StatusOr<Value> NormalizeFunc(const std::vector<Value>& args) {
  if (args.empty() || args.size() > 2) {
    return absl::InvalidArgumentError(
        "semantic: NORMALIZE expects one or two arguments");
  }
  if (args[0].is_null()) return Value::NullString();
  ::googlesql::functions::NormalizeMode mode =
      ::googlesql::functions::NormalizeMode::NFC;
  if (args.size() == 2 && !args[1].is_null()) {
    if (args[1].type_kind() == ::googlesql::TYPE_ENUM) {
      mode = static_cast<::googlesql::functions::NormalizeMode>(
          args[1].enum_value());
    } else if (args[1].type_kind() == ::googlesql::TYPE_STRING) {
      if (!::googlesql::functions::NormalizeMode_Parse(args[1].string_value(),
                                                       &mode)) {
        return absl::InvalidArgumentError("semantic: invalid NORMALIZE mode");
      }
    }
  }
  absl::Status error;
  std::string out;
  if (!::googlesql::functions::Normalize(args[0].string_value(), mode,
                                         /*is_casefold=*/false, &out, &error)) {
    return error;
  }
  return Value::String(std::move(out));
}

absl::StatusOr<Value> NormalizeAndCasefoldFunc(const std::vector<Value>& args) {
  if (args.empty() || args.size() > 2) {
    return absl::InvalidArgumentError(
        "semantic: NORMALIZE_AND_CASEFOLD expects one or two arguments");
  }
  if (args[0].is_null()) return Value::NullString();
  ::googlesql::functions::NormalizeMode mode =
      ::googlesql::functions::NormalizeMode::NFC;
  if (args.size() == 2 && !args[1].is_null()) {
    if (args[1].type_kind() == ::googlesql::TYPE_ENUM) {
      mode = static_cast<::googlesql::functions::NormalizeMode>(
          args[1].enum_value());
    } else if (args[1].type_kind() == ::googlesql::TYPE_STRING) {
      if (!::googlesql::functions::NormalizeMode_Parse(args[1].string_value(),
                                                       &mode)) {
        return absl::InvalidArgumentError(
            "semantic: invalid NORMALIZE_AND_CASEFOLD mode");
      }
    }
  }
  absl::Status error;
  std::string out;
  if (!::googlesql::functions::Normalize(args[0].string_value(), mode,
                                         /*is_casefold=*/true, &out, &error)) {
    return error;
  }
  return Value::String(std::move(out));
}

absl::StatusOr<Value> SafeConvertBytesToString(const std::vector<Value>& args) {
  if (args.size() != 1) {
    return absl::InvalidArgumentError(
        "semantic: SAFE_CONVERT_BYTES_TO_STRING expects one argument");
  }
  if (args[0].is_null()) return Value::NullString();
  absl::Status error;
  std::string out;
  if (!::googlesql::functions::SafeConvertBytes(args[0].bytes_value(), &out,
                                                &error)) {
    return error;
  }
  return Value::String(std::move(out));
}

absl::StatusOr<Value> CodePointsToString(const std::vector<Value>& args) {
  if (args.size() != 1) {
    return absl::InvalidArgumentError(
        "semantic: CODE_POINTS_TO_STRING expects one argument");
  }
  if (args[0].is_null()) return Value::NullString();
  if (!args[0].type()->IsArray()) {
    return absl::InvalidArgumentError(
        "semantic: CODE_POINTS_TO_STRING expects ARRAY<INT64>");
  }
  std::vector<int64_t> cps;
  for (int i = 0; i < args[0].num_elements(); ++i) {
    if (args[0].element(i).is_null()) return Value::NullString();
    const int64_t cp = args[0].element(i).int64_value();
    if (cp == 0) continue;
    cps.push_back(cp);
  }
  absl::Status error;
  std::string out;
  if (!::googlesql::functions::CodePointsToString(cps, &out, &error)) {
    return error;
  }
  return Value::String(std::move(out));
}

absl::StatusOr<Value> CodePointsToBytes(const std::vector<Value>& args) {
  if (args.size() != 1) {
    return absl::InvalidArgumentError(
        "semantic: CODE_POINTS_TO_BYTES expects one argument");
  }
  if (args[0].is_null()) return Value::NullBytes();
  std::vector<int64_t> cps;
  for (int i = 0; i < args[0].num_elements(); ++i) {
    if (args[0].element(i).is_null()) return Value::NullBytes();
    cps.push_back(args[0].element(i).int64_value());
  }
  absl::Status error;
  std::string out;
  if (!::googlesql::functions::CodePointsToBytes(cps, &out, &error)) {
    return error;
  }
  return Value::Bytes(std::move(out));
}

absl::StatusOr<Value> ToCodePoints(const std::vector<Value>& args,
                                   const ::googlesql::Type* return_type) {
  if (args.size() != 1) {
    return absl::InvalidArgumentError(
        "semantic: TO_CODE_POINTS expects one argument");
  }
  if (args[0].is_null()) {
    if (return_type != nullptr && return_type->IsArray()) {
      return Value::Array(return_type->AsArray(), {});
    }
    return Value::Null(return_type);
  }
  if (args[0].type_kind() == ::googlesql::TYPE_STRING &&
      args[0].string_value().empty()) {
    return Value::Array(return_type->AsArray(), {});
  }
  absl::Status error;
  std::vector<int64_t> cps;
  if (args[0].type_kind() == ::googlesql::TYPE_BYTES) {
    if (!::googlesql::functions::BytesToCodePoints(args[0].bytes_value(), &cps,
                                                   &error)) {
      return error;
    }
  } else if (!::googlesql::functions::StringToCodePoints(
                 args[0].string_value(), &cps, &error)) {
    return error;
  }
  std::vector<Value> elements;
  elements.reserve(cps.size());
  for (int64_t cp : cps) {
    elements.push_back(Value::Int64(cp));
  }
  return Value::Array(return_type->AsArray(), elements);
}

absl::StatusOr<Value> LeastGreatest(absl::string_view name,
                                    const std::vector<Value>& args,
                                    const ::googlesql::Type* return_type) {
  if (args.empty()) {
    return absl::InvalidArgumentError(
        absl::StrCat("semantic: ", name, " expects arguments"));
  }
  Value best = args[0];
  for (size_t i = 1; i < args.size(); ++i) {
    if (args[i].is_null()) return Value::Null(return_type);
    if (name == "least") {
      if (args[i].LessThan(best)) best = args[i];
    } else if (best.LessThan(args[i])) {
      best = args[i];
    }
  }
  return best;
}

absl::StatusOr<Value> Least(const std::vector<Value>& args,
                            const ::googlesql::Type* return_type) {
  return LeastGreatest("least", args, return_type);
}

absl::StatusOr<Value> Greatest(const std::vector<Value>& args,
                               const ::googlesql::Type* return_type) {
  return LeastGreatest("greatest", args, return_type);
}

absl::StatusOr<Value> ParseNumericFunc(const std::vector<Value>& args) {
  if (args.size() != 1) {
    return absl::InvalidArgumentError(
        "semantic: PARSE_NUMERIC expects one argument");
  }
  if (args[0].is_null()) return Value::NullNumeric();
  NumericValue out;
  absl::Status error;
  if (!::googlesql::functions::ParseNumeric(args[0].string_value(), &out,
                                            &error)) {
    return error;
  }
  return Value::Numeric(out);
}

namespace {

using ::googlesql::BigNumericValue;
using ::googlesql::NumericValue;

std::optional<std::string> ExpandScientificNotation(absl::string_view input) {
  const size_t epos = input.find('e');
  if (epos == absl::string_view::npos) return std::nullopt;
  std::string mantissa(input.substr(0, epos));
  int64_t exponent = 0;
  if (!absl::SimpleAtoi(input.substr(epos + 1), &exponent)) {
    return std::nullopt;
  }
  const size_t dot = mantissa.find('.');
  if (dot == std::string::npos) {
    if (exponent >= 0) {
      mantissa.append(static_cast<size_t>(exponent), '0');
    }
    return mantissa;
  }
  std::string digits = mantissa.substr(0, dot);
  digits.append(mantissa.substr(dot + 1));
  const int64_t decimal_places = static_cast<int64_t>(mantissa.size() - dot - 1);
  exponent -= decimal_places;
  if (exponent >= 0) {
    digits.append(static_cast<size_t>(exponent), '0');
    return digits;
  }
  const int64_t shift = -exponent;
  if (shift >= static_cast<int64_t>(digits.size())) {
    digits.insert(0, static_cast<size_t>(shift - digits.size() + 1), '0');
    digits.insert(1, ".");
    return digits;
  }
  digits.insert(static_cast<size_t>(digits.size() - shift), ".");
  return digits;
}

void MaybeSetBignumericRenderOverride(const BigNumericValue& parsed,
                                      absl::string_view expanded,
                                      const EvalContext* ctx) {
  if (ctx == nullptr) return;
  const std::string rendered = parsed.ToString();
  if (rendered.size() < expanded.size() &&
      absl::StartsWith(expanded, rendered)) {
    ctx->bignumeric_render_override = std::string(expanded);
  }
}

}  // namespace

absl::StatusOr<Value> ParseBignumeric(const std::vector<Value>& args,
                                      const EvalContext* ctx) {
  if (args.size() != 1) {
    return absl::InvalidArgumentError(
        "semantic: PARSE_BIGNUMERIC expects one argument");
  }
  if (args[0].is_null()) return Value::NullBigNumeric();
  std::string input = args[0].string_value();
  for (char& c : input) {
    if (c == 'E') c = 'e';
  }
  BigNumericValue out;
  absl::Status error;
  if (::googlesql::functions::ParseBigNumeric(input, &out, &error)) {
    return Value::BigNumeric(out);
  }
  if (input.find('e') != std::string::npos) {
    const std::optional<std::string> expanded = ExpandScientificNotation(input);
    if (expanded.has_value()) {
      for (const std::string& candidate :
           {*expanded, absl::StrCat(*expanded, ".0")}) {
        if (auto parsed = BigNumericValue::FromStringWithRounding(
                candidate, 76, /*round_half_even=*/false);
            parsed.ok()) {
          MaybeSetBignumericRenderOverride(*parsed, *expanded, ctx);
          return Value::BigNumeric(*parsed);
        }
        if (::googlesql::functions::ParseBigNumeric(candidate, &out, &error)) {
          MaybeSetBignumericRenderOverride(out, *expanded, ctx);
          return Value::BigNumeric(out);
        }
      }
      const size_t epos = input.find('e');
      std::string mantissa(input.substr(0, epos));
      int64_t exponent = 0;
      if (absl::SimpleAtoi(input.substr(epos + 1), &exponent)) {
        const size_t dot = mantissa.find('.');
        int64_t digits_before_dot = static_cast<int64_t>(dot == std::string::npos
                                                             ? mantissa.size()
                                                             : dot);
        int64_t norm_exp = exponent + digits_before_dot -
                           (dot == std::string::npos ? 0 : 1);
        std::string norm_mantissa =
            dot == std::string::npos
                ? mantissa
                : absl::StrCat(mantissa.substr(0, dot), mantissa.substr(dot + 1));
        while (norm_mantissa.size() > 1 && norm_mantissa[0] == '0') {
          norm_mantissa.erase(0, 1);
          --norm_exp;
        }
        const std::string normalized =
            absl::StrCat(norm_mantissa, "e", norm_exp);
        if (auto parsed = BigNumericValue::FromStringWithRounding(
                normalized, 76, /*round_half_even=*/false);
            parsed.ok()) {
          MaybeSetBignumericRenderOverride(*parsed, *expanded, ctx);
          return Value::BigNumeric(*parsed);
        }
        if (::googlesql::functions::ParseBigNumeric(normalized, &out, &error)) {
          MaybeSetBignumericRenderOverride(out, *expanded, ctx);
          return Value::BigNumeric(out);
        }
      }
    }
    const size_t epos = input.find('e');
    std::string mantissa_str(input.substr(0, epos));
    int64_t exponent = 0;
    if (!absl::SimpleAtoi(input.substr(epos + 1), &exponent)) {
      return error;
    }
    const size_t dot = mantissa_str.find('.');
    if (dot != std::string::npos) {
      exponent -= static_cast<int64_t>(mantissa_str.size() - dot - 1);
    }
    if (auto mantissa_or = BigNumericValue::FromStringWithRounding(
            mantissa_str, 76, /*round_half_even=*/false);
        mantissa_or.ok()) {
      BigNumericValue scaled = *mantissa_or;
      if (exponent > 0) {
        for (int64_t i = 0; i < exponent; ++i) {
          auto next = scaled.Multiply(BigNumericValue(10));
          if (!next.ok()) return next.status();
          scaled = *next;
        }
      } else if (exponent < 0) {
        for (int64_t i = 0; i < -exponent; ++i) {
          auto next = scaled.Divide(BigNumericValue(10));
          if (!next.ok()) return next.status();
          scaled = *next;
        }
      }
      if (expanded.has_value()) {
        MaybeSetBignumericRenderOverride(scaled, *expanded, ctx);
      }
      return Value::BigNumeric(scaled);
    }
  }
  if (auto rounded = BigNumericValue::FromStringWithRounding(
          input, 76, /*round_half_even=*/false);
      rounded.ok()) {
    return Value::BigNumeric(*rounded);
  }
  return error;
}

absl::StatusOr<Value> RegexpExtract(const std::vector<Value>& args) {
  if (args.size() < 2 || args.size() > 4) {
    return absl::InvalidArgumentError(
        "semantic: REGEXP_EXTRACT expects 2 to 4 arguments");
  }
  if (args[0].is_null() || args[1].is_null()) {
    return Value::NullString();
  }
  auto re = MakeRegExpForValue(args[1]);
  if (!re.ok()) return re.status();
  int64_t position = 1;
  int64_t occurrence = 1;
  if (args.size() >= 3) {
    if (args[2].is_null()) return Value::NullString();
    position = args[2].int64_value();
  }
  if (args.size() == 4) {
    if (args[3].is_null()) return Value::NullString();
    occurrence = args[3].int64_value();
  }
  absl::Status error;
  absl::string_view out;
  bool is_null = false;
  const auto unit = args[0].type_kind() == ::googlesql::TYPE_BYTES
                        ? RegExp::PositionUnit::kBytes
                        : RegExp::PositionUnit::kUtf8Chars;
  if (!(*re)
           ->Extract(AsStringOrBytes(args[0]), unit, position, occurrence,
                     /*use_legacy_position_behavior=*/true, &out, &is_null,
                     &error)) {
    return error;
  }
  if (is_null) return Value::NullString();
  return StringOrBytesFromView(args[0], out);
}

absl::StatusOr<Value> RegexpExtractAll(const std::vector<Value>& args,
                                       const ::googlesql::Type* return_type) {
  if (args.size() < 2 || args.size() > 4) {
    return absl::InvalidArgumentError(
        "semantic: REGEXP_EXTRACT_ALL expects 2 to 4 arguments");
  }
  if (args[0].is_null() || args[1].is_null()) {
    return Value::Null(return_type);
  }
  auto re = MakeRegExpForValue(args[1]);
  if (!re.ok()) return re.status();
  int64_t offset = 0;
  if (args.size() >= 3 && !args[2].is_null()) {
    offset = args[2].int64_value() - 1;
  }
  absl::Status error;
  std::vector<Value> elements;
  RegExp::ExtractAllIterator iter =
      (*re)->CreateExtractAllIterator(AsStringOrBytes(args[0]), offset);
  absl::string_view match;
  while (iter.Next(&match, &error)) {
    elements.push_back(StringOrBytesFromView(args[0], match));
  }
  if (!error.ok()) return error;
  return Value::Array(return_type->AsArray(), elements);
}

absl::StatusOr<Value> RegexpReplace(const std::vector<Value>& args) {
  if (args.size() != 3) {
    return absl::InvalidArgumentError(
        "semantic: REGEXP_REPLACE expects three arguments");
  }
  if (AnyNull(args)) {
    if (args[0].type_kind() == ::googlesql::TYPE_BYTES) {
      return Value::NullBytes();
    }
    return Value::NullString();
  }
  auto re = MakeRegExpForValue(args[1]);
  if (!re.ok()) return re.status();
  absl::Status error;
  std::string out;
  if (!(*re)->Replace(AsStringOrBytes(args[0]), AsStringOrBytes(args[2]), &out,
                      &error)) {
    return error;
  }
  return StringOrBytesFromView(args[0], out);
}

absl::StatusOr<Value> RegexpInstr(const std::vector<Value>& args) {
  if (args.size() < 2 || args.size() > 5) {
    return absl::InvalidArgumentError(
        "semantic: REGEXP_INSTR expects 2 to 5 arguments");
  }
  if (AnyNull(args)) return Value::NullInt64();
  auto re = MakeRegExpForValue(args[1]);
  if (!re.ok()) return re.status();
  RegExp::InstrParams params;
  params.input_str = AsStringOrBytes(args[0]);
  params.position_unit = args[0].type_kind() == ::googlesql::TYPE_BYTES
                             ? RegExp::PositionUnit::kBytes
                             : RegExp::PositionUnit::kUtf8Chars;
  if (args.size() >= 3) params.position = args[2].int64_value();
  if (args.size() >= 4) params.occurrence_index = args[3].int64_value();
  if (args.size() == 5) {
    params.return_position = args[4].int64_value() == 0
                                 ? RegExp::ReturnPosition::kStartOfMatch
                                 : RegExp::ReturnPosition::kEndOfMatch;
  }
  int64_t out = 0;
  params.out = &out;
  absl::Status error;
  if (!(*re)->Instr(params, /*use_legacy_position_behavior=*/true, &error)) {
    return error;
  }
  return Value::Int64(out);
}

absl::StatusOr<int64_t> LengthOfValue(const Value& v) {
  absl::Status error;
  int64_t out = 0;
  if (v.type_kind() == ::googlesql::TYPE_BYTES) {
    if (!::googlesql::functions::LengthBytes(v.bytes_value(), &out, &error)) {
      return error;
    }
  } else if (!::googlesql::functions::LengthUtf8(v.string_value(), &out,
                                                   &error)) {
    return error;
  }
  return out;
}

absl::StatusOr<std::string> LeftOfValue(const Value& v, int64_t n) {
  absl::Status error;
  absl::string_view out;
  if (v.type_kind() == ::googlesql::TYPE_BYTES) {
    if (!::googlesql::functions::LeftBytes(v.bytes_value(), n, &out, &error)) {
      return error;
    }
  } else if (!::googlesql::functions::LeftUtf8(v.string_value(), n, &out,
                                               &error)) {
    return error;
  }
  return std::string(out);
}

absl::StatusOr<Value> Lpad(const std::vector<Value>& args) {
  if (args.size() < 2 || args.size() > 3) {
    return absl::InvalidArgumentError(
        "semantic: LPAD expects two or three arguments");
  }
  if (args[0].is_null() || args[1].is_null() ||
      (args.size() == 3 && args[2].is_null())) {
    if (args[0].type_kind() == ::googlesql::TYPE_BYTES) {
      return Value::NullBytes();
    }
    return Value::NullString();
  }
  const int64_t target = args[1].int64_value();
  auto current_len = LengthOfValue(args[0]);
  if (!current_len.ok()) return current_len.status();
  absl::string_view pad;
  if (args.size() == 3) {
    pad = AsStringOrBytes(args[2]);
    if (pad.empty()) {
      return absl::InvalidArgumentError(
          "semantic: LPAD pattern must not be empty");
    }
  } else {
    pad = args[0].type_kind() == ::googlesql::TYPE_BYTES
              ? absl::string_view("\x20", 1)
              : " ";
  }
  if (*current_len >= target) {
    auto truncated = LeftOfValue(args[0], target);
    if (!truncated.ok()) return truncated.status();
    return StringOrBytesFromView(args[0], *truncated);
  }
  const int64_t pad_needed = target - *current_len;
  std::string padding;
  while (true) {
    auto plen = LengthOfValue(args[0].type_kind() == ::googlesql::TYPE_BYTES
                                  ? Value::Bytes(padding)
                                  : Value::String(padding));
    if (!plen.ok()) return plen.status();
    if (*plen >= pad_needed) break;
    padding.append(pad);
  }
  auto trunc_pad = LeftOfValue(
      args[0].type_kind() == ::googlesql::TYPE_BYTES ? Value::Bytes(padding)
                                                     : Value::String(padding),
      pad_needed);
  if (!trunc_pad.ok()) return trunc_pad.status();
  return StringOrBytesFromView(
      args[0], absl::StrCat(*trunc_pad, AsStringOrBytes(args[0])));
}

absl::StatusOr<Value> Rpad(const std::vector<Value>& args) {
  if (args.size() < 2 || args.size() > 3) {
    return absl::InvalidArgumentError(
        "semantic: RPAD expects two or three arguments");
  }
  if (args[0].is_null() || args[1].is_null() ||
      (args.size() == 3 && args[2].is_null())) {
    if (args[0].type_kind() == ::googlesql::TYPE_BYTES) {
      return Value::NullBytes();
    }
    return Value::NullString();
  }
  const int64_t target = args[1].int64_value();
  auto current_len = LengthOfValue(args[0]);
  if (!current_len.ok()) return current_len.status();
  absl::string_view pad;
  if (args.size() == 3) {
    pad = AsStringOrBytes(args[2]);
    if (pad.empty()) {
      return absl::InvalidArgumentError(
          "semantic: RPAD pattern must not be empty");
    }
  } else {
    pad = args[0].type_kind() == ::googlesql::TYPE_BYTES
              ? absl::string_view("\x20", 1)
              : " ";
  }
  if (*current_len >= target) {
    auto truncated = LeftOfValue(args[0], target);
    if (!truncated.ok()) return truncated.status();
    return StringOrBytesFromView(args[0], *truncated);
  }
  const int64_t pad_needed = target - *current_len;
  std::string padding;
  while (true) {
    auto plen = LengthOfValue(args[0].type_kind() == ::googlesql::TYPE_BYTES
                                  ? Value::Bytes(padding)
                                  : Value::String(padding));
    if (!plen.ok()) return plen.status();
    if (*plen >= pad_needed) break;
    padding.append(pad);
  }
  auto trunc_pad = LeftOfValue(
      args[0].type_kind() == ::googlesql::TYPE_BYTES ? Value::Bytes(padding)
                                                     : Value::String(padding),
      pad_needed);
  if (!trunc_pad.ok()) return trunc_pad.status();
  return StringOrBytesFromView(
      args[0], absl::StrCat(AsStringOrBytes(args[0]), *trunc_pad));
}

absl::StatusOr<Value> FormatString(const std::vector<Value>& args) {
  if (args.empty()) {
    return absl::InvalidArgumentError("semantic: FORMAT expects arguments");
  }
  if (args[0].is_null()) return Value::NullString();
  std::vector<Value> values(args.begin() + 1, args.end());
  for (const Value& v : values) {
    if (v.is_null()) return Value::NullString();
  }
  std::string output;
  bool is_null = false;
  if (auto status = StringFormatUtf8(args[0].string_value(), values,
                                     ProductMode::PRODUCT_EXTERNAL, &output,
                                     &is_null);
      !status.ok()) {
    return status;
  }
  if (is_null) return Value::NullString();
  return Value::String(std::move(output));
}

}  // namespace functions
}  // namespace semantic
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
