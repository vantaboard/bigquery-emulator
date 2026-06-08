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
#include "backend/engine/semantic/functions/string_extra_internal.h"
#include "backend/engine/semantic/functions/string_funcs.h"
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

using string_extra_internal::AnyNull;
using string_extra_internal::AsStringOrBytes;
using string_extra_internal::StringOrBytesFromView;

absl::StatusOr<Value> Ascii(const std::vector<Value>& args) {
  if (args.size() != 1) {
    return absl::InvalidArgumentError("semantic: ASCII expects one argument");
  }
  if (args[0].is_null()) return Value::NullInt64();
  absl::Status error;
  int64_t out = 0;
  if (args[0].type_kind() == ::googlesql::TYPE_BYTES) {
    if (args[0].bytes_value().empty()) return Value::Int64(0);
    if (!::googlesql::functions::FirstByteOfBytesToASCII(
            args[0].bytes_value(), &out, &error)) {
      return error;
    }
    return Value::Int64(out);
  }
  if (args[0].string_value().empty()) return Value::Int64(0);
  if (!::googlesql::functions::FirstCharOfStringToASCII(
          args[0].string_value(), &out, &error)) {
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
    if (!::googlesql::functions::LengthBytes(
            args[0].bytes_value(), &out, &error)) {
      return error;
    }
  } else {
    if (!::googlesql::functions::LengthBytes(
            args[0].string_value(), &out, &error)) {
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
    if (!::googlesql::functions::LengthBytes(
            args[0].bytes_value(), &out, &error)) {
      return error;
    }
  } else {
    if (!::googlesql::functions::LengthUtf8(
            args[0].string_value(), &out, &error)) {
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
  if (!::googlesql::functions::CodePointToString(
          args[0].int64_value(), &out, &error)) {
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
  if (!::googlesql::functions::FirstCharToCodePoint(
          args[0].string_value(), &out, &error)) {
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
    if (!::googlesql::functions::LowerBytes(
            args[0].bytes_value(), &out, &error)) {
      return error;
    }
    return Value::Bytes(std::move(out));
  }
  if (!::googlesql::functions::LowerUtf8(
          args[0].string_value(), &out, &error)) {
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
    if (!::googlesql::functions::UpperBytes(
            args[0].bytes_value(), &out, &error)) {
      return error;
    }
    return Value::Bytes(std::move(out));
  }
  if (!::googlesql::functions::UpperUtf8(
          args[0].string_value(), &out, &error)) {
    return error;
  }
  return Value::String(std::move(out));
}

absl::StatusOr<Value> TrimFamily(absl::string_view name,
                                 const std::vector<Value>& args) {
  if (args.empty() || args.size() > 2) {
    return absl::InvalidArgumentError(
        absl::StrCat("semantic: ", name, " expects one or two arguments"));
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
      if (!::googlesql::functions::LeftTrimSpacesUtf8(
              args[0].string_value(), &out, &error)) {
        return error;
      }
    } else if (name == "rtrim") {
      if (!::googlesql::functions::RightTrimSpacesUtf8(
              args[0].string_value(), &out, &error)) {
        return error;
      }
    } else if (!::googlesql::functions::TrimSpacesUtf8(
                   args[0].string_value(), &out, &error)) {
      return error;
    }
    return Value::String(std::string(out));
  }
  if (is_bytes) {
    if (name == "ltrim") {
      if (!::googlesql::functions::LeftTrimBytes(
              args[0].bytes_value(), args[1].bytes_value(), &out, &error)) {
        return error;
      }
    } else if (name == "rtrim") {
      if (!::googlesql::functions::RightTrimBytes(
              args[0].bytes_value(), args[1].bytes_value(), &out, &error)) {
        return error;
      }
    } else if (!::googlesql::functions::TrimBytes(args[0].bytes_value(),
                                                  args[1].bytes_value(),
                                                  &out,
                                                  &error)) {
      return error;
    }
    return Value::Bytes(std::string(out));
  }
  if (name == "ltrim") {
    if (!::googlesql::functions::LeftTrimUtf8(
            args[0].string_value(), args[1].string_value(), &out, &error)) {
      return error;
    }
  } else if (name == "rtrim") {
    if (!::googlesql::functions::RightTrimUtf8(
            args[0].string_value(), args[1].string_value(), &out, &error)) {
      return error;
    }
  } else if (!::googlesql::functions::TrimUtf8(args[0].string_value(),
                                               args[1].string_value(),
                                               &out,
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
    if (!::googlesql::functions::ReplaceBytes(args[0].bytes_value(),
                                              args[1].bytes_value(),
                                              args[2].bytes_value(),
                                              &out,
                                              &error)) {
      return error;
    }
    return Value::Bytes(std::move(out));
  }
  if (!::googlesql::functions::ReplaceUtf8(args[0].string_value(),
                                           args[1].string_value(),
                                           args[2].string_value(),
                                           &out,
                                           &error)) {
    return error;
  }
  return Value::String(std::move(out));
}

absl::StatusOr<Value> Translate(const std::vector<Value>& args) {
  if (args.size() != 3) {
    return absl::InvalidArgumentError(
        "semantic: TRANSLATE expects three arguments");
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
    if (!::googlesql::functions::TranslateBytes(args[0].bytes_value(),
                                                args[1].bytes_value(),
                                                args[2].bytes_value(),
                                                &out,
                                                &error)) {
      return error;
    }
    return Value::Bytes(std::move(out));
  }
  if (!::googlesql::functions::TranslateUtf8(args[0].string_value(),
                                             args[1].string_value(),
                                             args[2].string_value(),
                                             &out,
                                             &error)) {
    return error;
  }
  return Value::String(std::move(out));
}

}  // namespace functions
}  // namespace semantic
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
