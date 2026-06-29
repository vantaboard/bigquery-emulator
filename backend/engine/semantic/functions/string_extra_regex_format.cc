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

using ::googlesql::ProductMode;
using ::googlesql::functions::RegExp;
using ::googlesql::functions::StringFormatUtf8;
using string_extra_internal::AnyNull;
using string_extra_internal::AsStringOrBytes;
using string_extra_internal::MakeRegExpForValue;
using string_extra_internal::StringOrBytesFromView;

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
  if (!(*re)->Extract(AsStringOrBytes(args[0]),
                      unit,
                      position,
                      occurrence,
                      /*use_legacy_position_behavior=*/true,
                      &out,
                      &is_null,
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
    if (return_type != nullptr && return_type->IsArray()) {
      return Value::Array(return_type->AsArray(), {});
    }
    return Value::Null(return_type);
  }
  if (return_type == nullptr || !return_type->IsArray()) {
    return absl::InvalidArgumentError(
        "semantic: REGEXP_EXTRACT_ALL missing array return type");
  }
  auto re = MakeRegExpForValue(args[1]);
  if (!re.ok()) return re.status();
  std::unique_ptr<const RegExp> regexp = std::move(*re);
  int64_t offset = 0;
  if (args.size() >= 3 && !args[2].is_null()) {
    offset = args[2].int64_value() - 1;
  }
  absl::Status error;
  std::vector<Value> elements;
  RegExp::ExtractAllIterator iter =
      regexp->CreateExtractAllIterator(AsStringOrBytes(args[0]), offset);
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
  std::unique_ptr<const RegExp> regexp = std::move(*re);
  absl::Status error;
  std::string out;
  if (!regexp->Replace(
          AsStringOrBytes(args[0]), AsStringOrBytes(args[2]), &out, &error)) {
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
  } else if (!::googlesql::functions::LengthUtf8(
                 v.string_value(), &out, &error)) {
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
  } else if (!::googlesql::functions::LeftUtf8(
                 v.string_value(), n, &out, &error)) {
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
  auto trunc_pad = LeftOfValue(args[0].type_kind() == ::googlesql::TYPE_BYTES
                                   ? Value::Bytes(padding)
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
  auto trunc_pad = LeftOfValue(args[0].type_kind() == ::googlesql::TYPE_BYTES
                                   ? Value::Bytes(padding)
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
  if (auto status = StringFormatUtf8(args[0].string_value(),
                                     values,
                                     ProductMode::PRODUCT_EXTERNAL,
                                     &output,
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
