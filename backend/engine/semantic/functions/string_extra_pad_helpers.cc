#include <string>

#include "absl/status/statusor.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/string_view.h"
#include "backend/engine/semantic/error.h"
#include "backend/engine/semantic/functions/string_extra_internal.h"
#include "backend/engine/semantic/value.h"
#include "googlesql/public/functions/string.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace semantic {
namespace functions {
namespace string_extra_internal {

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

namespace {

Value NullPadResult(const Value& template_value) {
  if (template_value.type_kind() == ::googlesql::TYPE_BYTES) {
    return Value::NullBytes();
  }
  return Value::NullString();
}

absl::StatusOr<absl::string_view> PadPatternForArgs(
    const std::vector<Value>& args) {
  if (args.size() == 3) {
    absl::string_view pad = AsStringOrBytes(args[2]);
    if (pad.empty()) {
      return absl::InvalidArgumentError(
          "semantic: LPAD/RPAD pattern must not be empty");
    }
    return pad;
  }
  return args[0].type_kind() == ::googlesql::TYPE_BYTES
             ? absl::string_view("\x20", 1)
             : " ";
}

absl::StatusOr<std::string> BuildPadding(const Value& template_value,
                                         absl::string_view pad,
                                         int64_t pad_needed) {
  std::string padding;
  while (true) {
    auto plen = LengthOfValue(template_value.type_kind() == ::googlesql::TYPE_BYTES
                                  ? Value::Bytes(padding)
                                  : Value::String(padding));
    if (!plen.ok()) return plen.status();
    if (*plen >= pad_needed) break;
    padding.append(pad);
  }
  return LeftOfValue(template_value.type_kind() == ::googlesql::TYPE_BYTES
                         ? Value::Bytes(padding)
                         : Value::String(padding),
                     pad_needed);
}

}  // namespace

absl::StatusOr<Value> PadValue(const std::vector<Value>& args, bool pad_left) {
  if (args.size() < 2 || args.size() > 3) {
    return absl::InvalidArgumentError(
        pad_left ? "semantic: LPAD expects two or three arguments"
                 : "semantic: RPAD expects two or three arguments");
  }
  if (args[0].is_null() || args[1].is_null() ||
      (args.size() == 3 && args[2].is_null())) {
    return NullPadResult(args[0]);
  }
  const int64_t target = args[1].int64_value();
  auto current_len = LengthOfValue(args[0]);
  if (!current_len.ok()) return current_len.status();
  auto pad_or = PadPatternForArgs(args);
  if (!pad_or.ok()) return pad_or.status();
  if (*current_len >= target) {
    auto truncated = LeftOfValue(args[0], target);
    if (!truncated.ok()) return truncated.status();
    return StringOrBytesFromView(args[0], *truncated);
  }
  auto trunc_pad_or =
      BuildPadding(args[0], *pad_or, target - *current_len);
  if (!trunc_pad_or.ok()) return trunc_pad_or.status();
  if (pad_left) {
    return StringOrBytesFromView(
        args[0], absl::StrCat(*trunc_pad_or, AsStringOrBytes(args[0])));
  }
  return StringOrBytesFromView(
      args[0], absl::StrCat(AsStringOrBytes(args[0]), *trunc_pad_or));
}

}  // namespace string_extra_internal
}  // namespace functions
}  // namespace semantic
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
