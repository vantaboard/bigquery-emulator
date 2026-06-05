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
    if (!::googlesql::functions::ReverseBytes(
            args[0].bytes_value(), &out, &error)) {
      return error;
    }
    return Value::Bytes(std::move(out));
  }
  if (!::googlesql::functions::ReverseUtf8(
          args[0].string_value(), &out, &error)) {
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
    if (!::googlesql::functions::StartsWithBytes(
            args[0].bytes_value(), args[1].bytes_value(), &out, &error)) {
      return error;
    }
  } else if (!::googlesql::functions::StartsWithUtf8(args[0].string_value(),
                                                     args[1].string_value(),
                                                     &out,
                                                     &error)) {
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
    if (!::googlesql::functions::EndsWithBytes(
            args[0].bytes_value(), args[1].bytes_value(), &out, &error)) {
      return error;
    }
  } else if (!::googlesql::functions::EndsWithUtf8(args[0].string_value(),
                                                   args[1].string_value(),
                                                   &out,
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
    if (!::googlesql::functions::LeftBytes(
            args[0].bytes_value(), args[1].int64_value(), &out, &error)) {
      return error;
    }
    return Value::Bytes(std::string(out));
  }
  if (!::googlesql::functions::LeftUtf8(
          args[0].string_value(), args[1].int64_value(), &out, &error)) {
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
      if (!::googlesql::functions::SubstrBytes(
              args[0].bytes_value(), pos, &out, &error)) {
        return error;
      }
    } else if (!::googlesql::functions::SubstrWithLengthBytes(
                   args[0].bytes_value(),
                   pos,
                   args[2].int64_value(),
                   &out,
                   &error)) {
      return error;
    }
    return Value::Bytes(std::string(out));
  }
  if (args.size() == 2) {
    if (!::googlesql::functions::SubstrUtf8(
            args[0].string_value(), pos, &out, &error)) {
      return error;
    }
  } else if (!::googlesql::functions::SubstrWithLengthUtf8(
                 args[0].string_value(),
                 pos,
                 args[2].int64_value(),
                 &out,
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
    if (!::googlesql::functions::StrposBytes(
            args[0].bytes_value(), args[1].bytes_value(), &out, &error)) {
      return error;
    }
  } else if (!::googlesql::functions::StrposUtf8(args[0].string_value(),
                                                 args[1].string_value(),
                                                 &out,
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
    if (!::googlesql::functions::SplitBytes(
            args[0].bytes_value(), delim, &parts, &error)) {
      return error;
    }
    std::vector<Value> elements;
    elements.reserve(parts.size());
    for (const std::string& p : parts) {
      elements.push_back(Value::Bytes(p));
    }
    return Value::Array(return_type->AsArray(), elements);
  }
  if (!::googlesql::functions::SplitUtf8(
          args[0].string_value(), delim, &parts, &error)) {
    return error;
  }
  std::vector<Value> elements;
  elements.reserve(parts.size());
  for (const std::string& p : parts) {
    elements.push_back(Value::String(p));
  }
  return Value::Array(return_type->AsArray(), elements);
}

}  // namespace functions
}  // namespace semantic
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
