#include <cmath>
#include <cstdint>
#include <string>
#include <utility>

#include "absl/status/statusor.h"
#include "absl/strings/ascii.h"
#include "absl/strings/match.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/string_view.h"
#include "absl/time/time.h"
#include "backend/engine/semantic/error.h"
#include "backend/engine/semantic/eval_expr_internal.h"
#include "backend/engine/semantic/functions/datetime_funcs_internal.h"
#include "backend/engine/semantic/value.h"
#include "googlesql/public/functions/parse_date_time.h"
#include "googlesql/public/functions/string.h"
#include "googlesql/public/numeric_value.h"
#include "googlesql/public/type.h"
#include "googlesql/public/types/type_parameters.h"
#include "googlesql/resolved_ast/resolved_ast.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace semantic {
namespace eval_expr_internal {

namespace {

using functions::datetime_internal::DefaultTimeZone;
using functions::datetime_internal::kFormatOpts;
using functions::datetime_internal::kMicros;

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

  if (target->kind() == ::googlesql::TYPE_DOUBLE) {
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
  if (target->kind() == ::googlesql::TYPE_STRING) {
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
        if (cast.return_null_on_error()) return NullOfType(target);
        return error;
      }
      return Value::String(std::move(out));
    }
  }
  if (target->kind() == ::googlesql::TYPE_BYTES &&
      inner.type_kind() == ::googlesql::TYPE_STRING) {
    return Value::Bytes(std::string(inner.string_value()));
  }
  if (target->kind() == ::googlesql::TYPE_DATE) {
    if (inner.type_kind() == ::googlesql::TYPE_DATE) return inner;
    if (inner.type_kind() == ::googlesql::TYPE_STRING) {
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
      if (cast.return_null_on_error()) return Value::NullDate();
      return MakeSemanticError(SemanticErrorReason::kInvalidArgument,
                               absl::StrCat("semantic: CAST STRING to DATE "
                                            "failed for '",
                                            text,
                                            "'"));
    }
  }
  if (target->kind() == ::googlesql::TYPE_DATETIME) {
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
    if (inner.type_kind() == ::googlesql::TYPE_STRING) {
      ::googlesql::DatetimeValue out;
      if (auto s = ::googlesql::functions::ParseStringToDatetime(
              "%F %T",
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
  }
  if (target->kind() == ::googlesql::TYPE_TIMESTAMP) {
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
    if (inner.type_kind() == ::googlesql::TYPE_STRING) {
      int64_t micros = 0;
      std::string text(inner.string_value());
      if (absl::EndsWith(text, " UTC")) {
        text.resize(text.size() - 4);
      }
      absl::Time t;
      std::string err;
      if (absl::ParseTime(absl::RFC3339_full, text, &t, &err) ||
          absl::ParseTime("%Y-%m-%d %H:%M:%E*S%Ez", text, &t, &err) ||
          absl::ParseTime("%Y-%m-%d %H:%M:%E*S", text, &t, &err)) {
        return Value::TimestampFromUnixMicros(absl::ToUnixMicros(t));
      }
      if (auto s = ::googlesql::functions::ParseStringToTimestamp(
              "%F %T",
              text,
              DefaultTimeZone(),
              /*parse_version2=*/true,
              &micros);
          s.ok()) {
        return Value::TimestampFromUnixMicros(micros);
      }
      if (auto s = ::googlesql::functions::ConvertStringToTimestamp(
              text,
              DefaultTimeZone(),
              kMicros,
              /*allow_tz_in_str=*/true,
              &micros);
          s.ok()) {
        return Value::TimestampFromUnixMicros(micros);
      }
      return MakeSemanticError(SemanticErrorReason::kInvalidArgument,
                               absl::StrCat("semantic: CAST STRING to "
                                            "TIMESTAMP failed for '",
                                            inner.string_value(),
                                            "'"));
    }
  }
  if (target->kind() == ::googlesql::TYPE_BOOL) {
    if (inner.type_kind() == ::googlesql::TYPE_BOOL) return inner;
    if (inner.type_kind() == ::googlesql::TYPE_STRING) {
      absl::string_view s = inner.string_value();
      if (absl::EqualsIgnoreCase(s, "true")) return Value::Bool(true);
      if (absl::EqualsIgnoreCase(s, "false")) return Value::Bool(false);
      return MakeSemanticError(SemanticErrorReason::kInvalidArgument,
                               "semantic: CAST STRING to BOOL failed");
    }
  }
  if (target->kind() == ::googlesql::TYPE_INT64) {
    if (inner.type_kind() == ::googlesql::TYPE_INT64) return inner;
    if (inner.type_kind() == ::googlesql::TYPE_DOUBLE) {
      return Value::Int64(
          static_cast<int64_t>(std::trunc(inner.double_value())));
    }
    if (inner.type_kind() == ::googlesql::TYPE_FLOAT) {
      return Value::Int64(
          static_cast<int64_t>(std::trunc(inner.float_value())));
    }
    if (inner.type_kind() == ::googlesql::TYPE_NUMERIC) {
      auto d = inner.numeric_value().ToDouble();
      return Value::Int64(static_cast<int64_t>(std::trunc(d)));
    }
    if (inner.type_kind() == ::googlesql::TYPE_STRING) {
      if (inner.is_null()) return Value::NullInt64();
      absl::string_view s = inner.string_value();
      if (absl::StartsWithIgnoreCase(s, "0x")) {
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
  }
  if (target->kind() == ::googlesql::TYPE_BIGNUMERIC) {
    if (inner.type_kind() == ::googlesql::TYPE_BIGNUMERIC) return inner;
    if (inner.type_kind() == ::googlesql::TYPE_INT64) {
      return Value::BigNumeric(
          ::googlesql::BigNumericValue(inner.int64_value()));
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
    if (inner.type_kind() == ::googlesql::TYPE_STRING) {
      auto n = ::googlesql::BigNumericValue::FromString(inner.string_value());
      if (!n.ok()) {
        return MakeSemanticError(SemanticErrorReason::kInvalidArgument,
                                 n.status().message());
      }
      return Value::BigNumeric(*n);
    }
  }
  if (target->kind() == ::googlesql::TYPE_NUMERIC) {
    if (inner.type_kind() == ::googlesql::TYPE_NUMERIC) {
      return finalize(inner);
    }
    if (inner.type_kind() == ::googlesql::TYPE_BIGNUMERIC) {
      auto n = inner.bignumeric_value().ToNumericValue();
      if (!n.ok()) {
        if (n.status().code() == absl::StatusCode::kOutOfRange) {
          return MakeSemanticError(SemanticErrorReason::kOverflow,
                                   n.status().message());
        }
        return MakeSemanticError(SemanticErrorReason::kInvalidArgument,
                                 n.status().message());
      }
      return finalize(Value::Numeric(*n));
    }
    if (inner.type_kind() == ::googlesql::TYPE_INT64) {
      auto n = ::googlesql::NumericValue::FromString(
          absl::StrCat(inner.int64_value()));
      if (!n.ok()) {
        return MakeSemanticError(SemanticErrorReason::kInvalidArgument,
                                 n.status().message());
      }
      return finalize(Value::Numeric(*n));
    }
    if (inner.type_kind() == ::googlesql::TYPE_DOUBLE) {
      auto n = ::googlesql::NumericValue::FromDouble(inner.double_value());
      if (!n.ok()) {
        if (n.status().code() == absl::StatusCode::kOutOfRange) {
          return MakeSemanticError(SemanticErrorReason::kOverflow,
                                   n.status().message());
        }
        return MakeSemanticError(SemanticErrorReason::kInvalidArgument,
                                 n.status().message());
      }
      return finalize(Value::Numeric(*n));
    }
    if (inner.type_kind() == ::googlesql::TYPE_FLOAT) {
      auto n = ::googlesql::NumericValue::FromDouble(
          static_cast<double>(inner.float_value()));
      if (!n.ok()) {
        if (n.status().code() == absl::StatusCode::kOutOfRange) {
          return MakeSemanticError(SemanticErrorReason::kOverflow,
                                   n.status().message());
        }
        return MakeSemanticError(SemanticErrorReason::kInvalidArgument,
                                 n.status().message());
      }
      return finalize(Value::Numeric(*n));
    }
    if (inner.type_kind() == ::googlesql::TYPE_STRING) {
      auto n = ::googlesql::NumericValue::FromString(inner.string_value());
      if (!n.ok()) {
        return MakeSemanticError(SemanticErrorReason::kInvalidArgument,
                                 n.status().message());
      }
      return finalize(Value::Numeric(*n));
    }
  }
  if (cast.return_null_on_error()) {
    return finalize(NullOfType(target));
  }
  return MakeSemanticError(
      SemanticErrorReason::kNotImplemented,
      absl::StrCat("semantic: CAST from ",
                   source != nullptr ? source->DebugString() : "<null>",
                   " to ",
                   target->DebugString(),
                   " is not yet implemented"));
}

}  // namespace eval_expr_internal
}  // namespace semantic
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
