#include <cmath>

#include "googlesql/public/functions/parse_date_time.h"
#include "googlesql/public/functions/string.h"
#include "googlesql/public/numeric_value.h"
#include "googlesql/public/type.h"
#include "googlesql/public/types/type_parameters.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace semantic {
namespace eval_expr_internal {

namespace {

using functions::datetime_internal::DefaultTimeZone;
using functions::datetime_internal::kFormatOpts;
using functions::datetime_internal::kMicros;

absl::StatusOr<Value> CastToDouble(Value inner) {
  if (inner.type_kind() == ::googlesql::TYPE_STRING) {
    double parsed = 0;
    if (!absl::SimpleAtod(inner.string_value(), &parsed)) {
      return MakeSemanticError(SemanticErrorReason::kInvalidArgument,
                               "semantic: CAST STRING to FLOAT64 failed");
    }
    return Value::Double(parsed);
  }
  auto d = ToDouble(inner);
  if (!d.ok()) return d.status();
  return Value::Double(*d);
}

absl::StatusOr<Value> CastToString(Value inner, bool return_null_on_error) {
  if (inner.type_kind() == ::googlesql::TYPE_INT64) {
    return Value::String(absl::StrCat(inner.int64_value()));
  }
  if (inner.type_kind() == ::googlesql::TYPE_BOOL) {
    return Value::String(inner.bool_value() ? "true" : "false");
  }
  if (inner.type_kind() == ::googlesql::TYPE_DOUBLE) {
    return Value::String(absl::StrCat(inner.double_value()));
  }
  if (inner.type_kind() == ::googlesql::TYPE_NUMERIC) {
    return Value::String(inner.numeric_value().ToString());
  }
  if (inner.type_kind() == ::googlesql::TYPE_BIGNUMERIC) {
    return Value::String(inner.bignumeric_value().ToString());
  }
  if (inner.type_kind() == ::googlesql::TYPE_DATE) {
    std::string out;
    if (absl::Status s = ::googlesql::functions::FormatDateToString(
            "%F", inner.date_value(), kFormatOpts, &out);
        !s.ok()) {
      return s;
    }
    return Value::String(std::move(out));
  }
  if (inner.type_kind() == ::googlesql::TYPE_DATETIME) {
    std::string out;
    if (absl::Status s =
            ::googlesql::functions::FormatDatetimeToStringWithOptions(
                "%F %T", inner.datetime_value(), kFormatOpts, &out);
        !s.ok()) {
      return s;
    }
    return Value::String(std::move(out));
  }
  if (inner.type_kind() == ::googlesql::TYPE_BYTES) {
    absl::Status error;
    std::string out;
    if (!::googlesql::functions::SafeConvertBytes(
            inner.bytes_value(), &out, &error)) {
      if (return_null_on_error) return Value::NullString();
      return error;
    }
    return Value::String(std::move(out));
  }
  return MakeSemanticError(SemanticErrorReason::kNotImplemented,
                           "semantic: CAST to STRING not implemented for type");
}

absl::StatusOr<Value> CastToDate(Value inner, bool return_null_on_error) {
  if (inner.type_kind() == ::googlesql::TYPE_DATE) return inner;
  if (inner.type_kind() != ::googlesql::TYPE_STRING) {
    return MakeSemanticError(SemanticErrorReason::kNotImplemented,
                             "semantic: CAST to DATE not implemented for type");
  }
  int32_t date = 0;
  const std::string text(inner.string_value());
  if (auto s = ::googlesql::functions::ParseStringToDate(
          "%Y-%m-%d", text, /*parse_version2=*/true, &date);
      s.ok()) {
    return Value::Date(date);
  }
  if (auto s = ::googlesql::functions::ParseStringToDate(
          "%F", text, /*parse_version2=*/true, &date);
      s.ok()) {
    return Value::Date(date);
  }
  if (return_null_on_error) return Value::NullDate();
  return MakeSemanticError(SemanticErrorReason::kInvalidArgument,
                           absl::StrCat("semantic: CAST STRING to DATE "
                                        "failed for '",
                                        text,
                                        "'"));
}

absl::StatusOr<Value> CastToDatetime(Value inner) {
  if (inner.type_kind() == ::googlesql::TYPE_DATETIME) return inner;
  if (inner.type_kind() == ::googlesql::TYPE_DATE) {
    ::googlesql::DatetimeValue out;
    if (auto s = ::googlesql::functions::ConstructDatetime(
            inner.date_value(), ::googlesql::TimeValue(), &out);
        !s.ok()) {
      return s;
    }
    return Value::Datetime(out);
  }
  if (inner.type_kind() != ::googlesql::TYPE_STRING) {
    return MakeSemanticError(SemanticErrorReason::kNotImplemented,
                             "semantic: CAST to DATETIME not implemented");
  }
  ::googlesql::DatetimeValue out;
  if (auto s =
          ::googlesql::functions::ParseStringToDatetime("%F %T",
                                                        inner.string_value(),
                                                        kMicros,
                                                        /*parse_version2=*/true,
                                                        &out);
      !s.ok()) {
    return MakeSemanticError(SemanticErrorReason::kInvalidArgument,
                             s.message());
  }
  return Value::Datetime(out);
}

absl::StatusOr<Value> CastToTimestamp(Value inner) {
  if (inner.type_kind() == ::googlesql::TYPE_TIMESTAMP) return inner;
  if (inner.type_kind() == ::googlesql::TYPE_DATE) {
    int64_t micros = 0;
    if (auto s = ::googlesql::functions::ConvertDateToTimestamp(
            inner.date_value(), kMicros, DefaultTimeZone(), &micros);
        !s.ok()) {
      return s;
    }
    return Value::TimestampFromUnixMicros(micros);
  }
  if (inner.type_kind() != ::googlesql::TYPE_STRING) {
    return MakeSemanticError(SemanticErrorReason::kNotImplemented,
                             "semantic: CAST to TIMESTAMP not implemented");
  }
  auto parsed = ParseTimestampWireString(inner.string_value());
  if (!parsed.ok()) {
    return MakeSemanticError(SemanticErrorReason::kInvalidArgument,
                             absl::StrCat("semantic: CAST STRING to "
                                          "TIMESTAMP failed for '",
                                          inner.string_value(),
                                          "'"));
  }
  return *parsed;
}

absl::StatusOr<Value> CastToBool(Value inner) {
  if (inner.type_kind() == ::googlesql::TYPE_BOOL) return inner;
  if (inner.type_kind() != ::googlesql::TYPE_STRING) {
    return MakeSemanticError(SemanticErrorReason::kNotImplemented,
                             "semantic: CAST to BOOL not implemented for type");
  }
  absl::string_view s = inner.string_value();
  if (absl::EqualsIgnoreCase(s, "true")) return Value::Bool(true);
  if (absl::EqualsIgnoreCase(s, "false")) return Value::Bool(false);
  return MakeSemanticError(SemanticErrorReason::kInvalidArgument,
                           "semantic: CAST STRING to BOOL failed");
}

absl::StatusOr<Value> CastHexStringToInt64(absl::string_view s) {
  s.remove_prefix(2);
  uint64_t parsed = 0;
  for (char c : s) {
    parsed <<= 4;
    if (c >= '0' && c <= '9') {
      parsed |= static_cast<uint64_t>(c - '0');
    } else if (c >= 'a' && c <= 'f') {
      parsed |= static_cast<uint64_t>(c - 'a' + 10);
    } else if (c >= 'A' && c <= 'F') {
      parsed |= static_cast<uint64_t>(c - 'A' + 10);
    } else {
      return MakeSemanticError(SemanticErrorReason::kInvalidArgument,
                               "semantic: CAST STRING to INT64 failed");
    }
  }
  return Value::Int64(static_cast<int64_t>(parsed));
}

absl::StatusOr<Value> CastDecimalStringToInt64(absl::string_view s) {
  bool negative = false;
  if (!s.empty() && s[0] == '+') {
    s.remove_prefix(1);
  } else if (!s.empty() && s[0] == '-') {
    negative = true;
    s.remove_prefix(1);
  }
  while (s.size() > 1 && s[0] == '0') {
    s.remove_prefix(1);
  }
  int64_t parsed = 0;
  if (s.empty()) {
    parsed = 0;
  } else if (!absl::SimpleAtoi(s, &parsed)) {
    return MakeSemanticError(SemanticErrorReason::kInvalidArgument,
                             "semantic: CAST STRING to INT64 failed");
  }
  if (negative) {
    parsed = -parsed;
  }
  return Value::Int64(parsed);
}

absl::StatusOr<Value> CastToInt64(Value inner) {
  if (inner.type_kind() == ::googlesql::TYPE_INT64) return inner;
  if (inner.type_kind() == ::googlesql::TYPE_DOUBLE) {
    return Value::Int64(static_cast<int64_t>(std::trunc(inner.double_value())));
  }
  if (inner.type_kind() == ::googlesql::TYPE_FLOAT) {
    return Value::Int64(static_cast<int64_t>(std::trunc(inner.float_value())));
  }
  if (inner.type_kind() == ::googlesql::TYPE_NUMERIC) {
    auto d = inner.numeric_value().ToDouble();
    return Value::Int64(static_cast<int64_t>(std::trunc(d)));
  }
  if (inner.type_kind() != ::googlesql::TYPE_STRING) {
    return MakeSemanticError(SemanticErrorReason::kNotImplemented,
                             "semantic: CAST to INT64 not implemented");
  }
  if (inner.is_null()) return Value::NullInt64();
  absl::string_view s = inner.string_value();
  if (absl::StartsWithIgnoreCase(s, "0x")) {
    return CastHexStringToInt64(s);
  }
  return CastDecimalStringToInt64(s);
}

absl::StatusOr<Value> CastToBignumeric(Value inner) {
  if (inner.type_kind() == ::googlesql::TYPE_BIGNUMERIC) return inner;
  if (inner.type_kind() == ::googlesql::TYPE_INT64) {
    return Value::BigNumeric(::googlesql::BigNumericValue(inner.int64_value()));
  }
  if (inner.type_kind() == ::googlesql::TYPE_NUMERIC) {
    return Value::BigNumeric(
        ::googlesql::BigNumericValue(inner.numeric_value()));
  }
  if (inner.type_kind() == ::googlesql::TYPE_DOUBLE) {
    auto n = ::googlesql::BigNumericValue::FromDouble(inner.double_value());
    if (!n.ok()) {
      if (n.status().code() == absl::StatusCode::kOutOfRange) {
        return MakeSemanticError(SemanticErrorReason::kOverflow,
                                 n.status().message());
      }
      return MakeSemanticError(SemanticErrorReason::kInvalidArgument,
                               n.status().message());
    }
    return Value::BigNumeric(*n);
  }
  if (inner.type_kind() != ::googlesql::TYPE_STRING) {
    return MakeSemanticError(SemanticErrorReason::kNotImplemented,
                             "semantic: CAST to BIGNUMERIC not implemented");
  }
  auto n = ::googlesql::BigNumericValue::FromString(inner.string_value());
  if (!n.ok()) {
    return MakeSemanticError(SemanticErrorReason::kInvalidArgument,
                             n.status().message());
  }
  return Value::BigNumeric(*n);
}

absl::StatusOr<Value> CastNumericFromBignumeric(Value inner) {
  auto n = inner.bignumeric_value().ToNumericValue();
  if (!n.ok()) {
    if (n.status().code() == absl::StatusCode::kOutOfRange) {
      return MakeSemanticError(SemanticErrorReason::kOverflow,
                               n.status().message());
    }
    return MakeSemanticError(SemanticErrorReason::kInvalidArgument,
                             n.status().message());
  }
  return Value::Numeric(*n);
}

absl::StatusOr<Value> CastNumericFromInt64(Value inner) {
  auto n =
      ::googlesql::NumericValue::FromString(absl::StrCat(inner.int64_value()));
  if (!n.ok()) {
    return MakeSemanticError(SemanticErrorReason::kInvalidArgument,
                             n.status().message());
  }
  return Value::Numeric(*n);
}

absl::StatusOr<Value> CastNumericFromDouble(double value) {
  auto n = ::googlesql::NumericValue::FromDouble(value);
  if (!n.ok()) {
    if (n.status().code() == absl::StatusCode::kOutOfRange) {
      return MakeSemanticError(SemanticErrorReason::kOverflow,
                               n.status().message());
    }
    return MakeSemanticError(SemanticErrorReason::kInvalidArgument,
                             n.status().message());
  }
  return Value::Numeric(*n);
}

absl::StatusOr<Value> CastNumericFromString(absl::string_view text) {
  auto n = ::googlesql::NumericValue::FromString(text);
  if (!n.ok()) {
    return MakeSemanticError(SemanticErrorReason::kInvalidArgument,
                             n.status().message());
  }
  return Value::Numeric(*n);
}

absl::StatusOr<Value> CastToNumeric(Value inner) {
  if (inner.type_kind() == ::googlesql::TYPE_NUMERIC) return inner;
  if (inner.type_kind() == ::googlesql::TYPE_BIGNUMERIC) {
    return CastNumericFromBignumeric(inner);
  }
  if (inner.type_kind() == ::googlesql::TYPE_INT64) {
    return CastNumericFromInt64(inner);
  }
  if (inner.type_kind() == ::googlesql::TYPE_DOUBLE) {
    return CastNumericFromDouble(inner.double_value());
  }
  if (inner.type_kind() == ::googlesql::TYPE_FLOAT) {
    return CastNumericFromDouble(static_cast<double>(inner.float_value()));
  }
  if (inner.type_kind() == ::googlesql::TYPE_STRING) {
    return CastNumericFromString(inner.string_value());
  }
  return MakeSemanticError(SemanticErrorReason::kNotImplemented,
                           "semantic: CAST to NUMERIC not implemented");
}

absl::StatusOr<Value> CastByTargetKind(Value inner,
                                       const ::googlesql::Type* target,
                                       bool return_null_on_error) {
  switch (target->kind()) {
    case ::googlesql::TYPE_DOUBLE:
      return CastToDouble(inner);
    case ::googlesql::TYPE_STRING:
      return CastToString(inner, return_null_on_error);
    case ::googlesql::TYPE_BYTES:
      if (inner.type_kind() == ::googlesql::TYPE_STRING) {
        return Value::Bytes(std::string(inner.string_value()));
      }
      break;
    case ::googlesql::TYPE_DATE:
      return CastToDate(inner, return_null_on_error);
    case ::googlesql::TYPE_DATETIME:
      return CastToDatetime(inner);
    case ::googlesql::TYPE_TIMESTAMP:
      return CastToTimestamp(inner);
    case ::googlesql::TYPE_BOOL:
      return CastToBool(inner);
    case ::googlesql::TYPE_INT64:
      return CastToInt64(inner);
    case ::googlesql::TYPE_BIGNUMERIC:
      return CastToBignumeric(inner);
    case ::googlesql::TYPE_NUMERIC:
      return CastToNumeric(inner);
    default:
      break;
  }
  return MakeSemanticError(SemanticErrorReason::kNotImplemented,
                           absl::StrCat("semantic: CAST to ",
                                        target->DebugString(),
                                        " not implemented for source type"));
}

}  // namespace

absl::StatusOr<Value> EvalResolvedCast(const ::googlesql::ResolvedCast& cast,
                                       Value inner,
                                       const ::googlesql::Type* source) {
  const ::googlesql::Type* target = cast.type();
  if (target == nullptr) {
    return absl::InvalidArgumentError("semantic: ResolvedCast has null type");
  }
  const bool has_type_modifiers = !cast.type_modifiers().IsEmpty();
  auto finalize = [&](Value v) -> absl::StatusOr<Value> {
    if (!has_type_modifiers) return std::move(v);
    return ApplyCastTypeModifiers(
        std::move(v), cast.type_modifiers(), cast.return_null_on_error());
  };
  if (cast.extended_cast() != nullptr) {
    auto extended = EvalExtendedCast(cast, inner, source);
    if (!extended.ok()) return extended.status();
    return finalize(*std::move(extended));
  }
  if (auto formatted = TryEvalCastFormatAndTimezone(cast, inner, target);
      formatted.has_value()) {
    if (!formatted->ok()) return formatted->status();
    return finalize(*std::move(*formatted));
  }
  if (auto casted = TryCastValueToType(
          inner, source, target, cast.return_null_on_error())) {
    if (!casted->ok()) return casted->status();
    return finalize(std::move(**casted));
  }

  if (source != nullptr && source->Equals(target)) {
    return finalize(inner);
  }
  if (inner.is_null()) return finalize(NullOfType(target));

  auto typed = CastByTargetKind(inner, target, cast.return_null_on_error());
  if (!typed.ok()) {
    if (cast.return_null_on_error()) {
      return finalize(NullOfType(target));
    }
    if (GetSemanticErrorReason(typed.status()) ==
        SemanticErrorReason::kNotImplemented) {
      return MakeSemanticError(
          SemanticErrorReason::kNotImplemented,
          absl::StrCat("semantic: CAST from ",
                       source != nullptr ? source->DebugString() : "<null>",
                       " to ",
                       target->DebugString(),
                       " is not yet implemented"));
    }
    return typed.status();
  }
  return finalize(*std::move(typed));
}

}  // namespace eval_expr_internal
}  // namespace semantic
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
