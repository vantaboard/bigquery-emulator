#include "backend/engine/semantic/eval_expr_internal.h"

#include <cmath>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <optional>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "absl/strings/match.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/string_view.h"
#include "backend/engine/semantic/error.h"
#include "backend/engine/semantic/value.h"
#include "googlesql/public/functions/date_time_util.h"
#include "googlesql/public/functions/datetime.pb.h"
#include "googlesql/public/numeric_value.h"
#include "googlesql/public/type.h"
#include "googlesql/public/type.pb.h"
#include "googlesql/public/value.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace semantic {
namespace eval_expr_internal {

std::string LowerFunctionDispatchName(const ::googlesql::Function* fn) {
  if (fn == nullptr) return "";
  std::string name =
      absl::AsciiStrToLower(fn->FullName(/*include_group=*/false));
  if (name.empty()) {
    name = absl::AsciiStrToLower(fn->Name());
  }
  return name;
}

std::string SynthesizeAnonymousFieldName(int idx) {
  return absl::StrCat("_", idx);
}

std::string ResolveStructFieldName(const ::googlesql::StructType& st, int idx) {
  const ::googlesql::StructField& f = st.field(idx);
  if (f.name.empty()) return SynthesizeAnonymousFieldName(idx);
  return f.name;
}

// Build a NULL `Value` of `type`'s kind. Used when an operator
// detects a NULL operand and BigQuery semantics propagate NULL, or
// when SAFE-mode swallows an error.
Value NullOfType(const ::googlesql::Type* type) {
  if (type == nullptr) return Value::NullInt64();
  switch (type->kind()) {
    case ::googlesql::TYPE_BOOL:
      return Value::NullBool();
    case ::googlesql::TYPE_INT64:
      return Value::NullInt64();
    case ::googlesql::TYPE_DOUBLE:
      return Value::NullDouble();
    case ::googlesql::TYPE_STRING:
      return Value::NullString();
    case ::googlesql::TYPE_BYTES:
      return Value::NullBytes();
    case ::googlesql::TYPE_DATE:
      return Value::NullDate();
    case ::googlesql::TYPE_TIME:
      return Value::NullTime();
    case ::googlesql::TYPE_DATETIME:
      return Value::NullDatetime();
    case ::googlesql::TYPE_TIMESTAMP:
      return Value::NullTimestamp();
    case ::googlesql::TYPE_NUMERIC:
      return Value::NullNumeric();
    case ::googlesql::TYPE_BIGNUMERIC:
      return Value::NullBigNumeric();
    case ::googlesql::TYPE_JSON:
      return Value::NullJson();
    case ::googlesql::TYPE_GEOGRAPHY:
      return Value::NullGeography();
    case ::googlesql::TYPE_INTERVAL:
      return Value::NullInterval();
    case ::googlesql::TYPE_UUID:
      return Value::NullUuid();
    default:
      // Fall back: the analyzer always types every output column,
      // and the generic null factory is invalid for compound types.
      // Callers that hit this branch should propagate the failure;
      // we return an invalid `Value` so the caller's status check
      // fires through `ToStorageValue`.
      return Value();
  }
}

// Translate an `absl::StatusOr<NumericValue>` (the shape every
// arithmetic helper on `NumericValue` / `BigNumericValue` returns)
// into a `semantic::Value`. Maps a `kOutOfRange` to a structured
// overflow error so the gateway surfaces the right `reason` token.
template <typename NV>
absl::StatusOr<Value> WrapNumeric(absl::StatusOr<NV> result, bool is_bignum) {
  if (!result.ok()) {
    if (result.status().code() == absl::StatusCode::kOutOfRange) {
      return MakeSemanticError(SemanticErrorReason::kOverflow,
                               result.status().message());
    }
    return MakeSemanticError(SemanticErrorReason::kInvalidArgument,
                             result.status().message());
  }
  if (is_bignum) {
    if constexpr (std::is_same_v<NV, ::googlesql::BigNumericValue>) {
      return Value::BigNumeric(*result);
    }
  }
  if constexpr (std::is_same_v<NV, ::googlesql::NumericValue>) {
    return Value::Numeric(*result);
  }
  return absl::InternalError("semantic: WrapNumeric: type tag mismatch");
}

// Coerce a Value to FLOAT64 for arithmetic. The analyzer guarantees
// the operands have the right shared type after implicit coercion,
// but defense-in-depth: unexpected kinds bail with INVALID_ARGUMENT.
absl::StatusOr<double> ToDouble(const Value& v) {
  switch (v.type_kind()) {
    case ::googlesql::TYPE_DOUBLE:
      return v.double_value();
    case ::googlesql::TYPE_INT64:
      return static_cast<double>(v.int64_value());
    case ::googlesql::TYPE_NUMERIC: {
      // NumericValue::ToDouble may lose precision but is the
      // canonical coercion path for mixed-type arithmetic.
      return v.numeric_value().ToDouble();
    }
    default:
      return absl::InvalidArgumentError(absl::StrCat(
          "semantic: cannot coerce ", v.type()->DebugString(), " to FLOAT64"));
  }
}

// INT64 arithmetic with checked overflow via the GCC/Clang builtins.
absl::StatusOr<Value> AddInt64(int64_t a, int64_t b) {
  int64_t out = 0;
  if (__builtin_add_overflow(a, b, &out)) {
    return MakeSemanticError(SemanticErrorReason::kOverflow,
                             absl::StrCat("Int64 overflow: ", a, " + ", b));
  }
  return Value::Int64(out);
}
absl::StatusOr<Value> SubInt64(int64_t a, int64_t b) {
  int64_t out = 0;
  if (__builtin_sub_overflow(a, b, &out)) {
    return MakeSemanticError(SemanticErrorReason::kOverflow,
                             absl::StrCat("Int64 overflow: ", a, " - ", b));
  }
  return Value::Int64(out);
}
absl::StatusOr<Value> MulInt64(int64_t a, int64_t b) {
  int64_t out = 0;
  if (__builtin_mul_overflow(a, b, &out)) {
    return MakeSemanticError(SemanticErrorReason::kOverflow,
                             absl::StrCat("Int64 overflow: ", a, " * ", b));
  }
  return Value::Int64(out);
}

absl::StatusOr<Value> ArithmeticAdd(const Value& a, const Value& b) {
  if (a.type_kind() == ::googlesql::TYPE_DATE &&
      b.type_kind() == ::googlesql::TYPE_INT64) {
    int32_t out = 0;
    if (auto s = ::googlesql::functions::AddDate(
            a.date_value(),
            ::googlesql::functions::DateTimestampPart::DAY,
            b.int64_value(),
            &out);
        !s.ok()) {
      return s;
    }
    return Value::Date(out);
  }
  if (a.type_kind() == ::googlesql::TYPE_DATE &&
      b.type_kind() == ::googlesql::TYPE_INTERVAL) {
    ::googlesql::DatetimeValue datetime;
    if (auto s = ::googlesql::functions::AddDate(
            a.date_value(), b.interval_value(), &datetime);
        !s.ok()) {
      return s;
    }
    return Value::Datetime(datetime);
  }
  if (a.type_kind() == ::googlesql::TYPE_DATETIME &&
      b.type_kind() == ::googlesql::TYPE_INTERVAL) {
    ::googlesql::DatetimeValue datetime;
    if (auto s = ::googlesql::functions::AddDatetime(
            a.datetime_value(), b.interval_value(), &datetime);
        !s.ok()) {
      return s;
    }
    return Value::Datetime(datetime);
  }
  if (a.type_kind() == ::googlesql::TYPE_TIMESTAMP &&
      b.type_kind() == ::googlesql::TYPE_INTERVAL) {
    auto out = ::googlesql::functions::AddTimestamp(
        a.ToUnixPicos().ToPicoTime(), b.interval_value());
    if (!out.ok()) return out.status();
    return Value::Timestamp(::googlesql::TimestampPicosValue(*out));
  }
  if (a.type_kind() != b.type_kind()) {
    return absl::InvalidArgumentError(
        absl::StrCat("semantic: '+' operands have mismatched kinds: ",
                     a.type()->DebugString(),
                     " vs ",
                     b.type()->DebugString()));
  }
  switch (a.type_kind()) {
    case ::googlesql::TYPE_INT64:
      return AddInt64(a.int64_value(), b.int64_value());
    case ::googlesql::TYPE_DOUBLE: {
      // BigQuery propagates NaN/Inf rather than erroring; no
      // explicit overflow check needed for FLOAT64.
      return Value::Double(a.double_value() + b.double_value());
    }
    case ::googlesql::TYPE_NUMERIC:
      return WrapNumeric(a.numeric_value().Add(b.numeric_value()),
                         /*is_bignum=*/false);
    case ::googlesql::TYPE_BIGNUMERIC:
      return WrapNumeric(a.bignumeric_value().Add(b.bignumeric_value()),
                         /*is_bignum=*/true);
    default:
      return absl::InvalidArgumentError(absl::StrCat(
          "semantic: '+' not implemented for ", a.type()->DebugString()));
  }
}

absl::StatusOr<Value> ArithmeticSub(const Value& a, const Value& b) {
  if (a.type_kind() == ::googlesql::TYPE_DATE &&
      b.type_kind() == ::googlesql::TYPE_INT64) {
    int32_t out = 0;
    if (auto s = ::googlesql::functions::SubDate(
            a.date_value(),
            ::googlesql::functions::DateTimestampPart::DAY,
            b.int64_value(),
            &out);
        !s.ok()) {
      return s;
    }
    return Value::Date(out);
  }
  if (a.type_kind() == ::googlesql::TYPE_DATE &&
      b.type_kind() == ::googlesql::TYPE_DATE) {
    auto iv = ::googlesql::functions::IntervalDiffDates(a.date_value(),
                                                        b.date_value());
    if (!iv.ok()) return iv.status();
    return Value::Interval(*iv);
  }
  if (a.type_kind() == ::googlesql::TYPE_DATETIME &&
      b.type_kind() == ::googlesql::TYPE_DATETIME) {
    absl::Time ta;
    absl::Time tb;
    if (auto s = ::googlesql::functions::ConvertDatetimeToTimestamp(
            a.datetime_value(), absl::UTCTimeZone(), &ta);
        !s.ok()) {
      return s;
    }
    if (auto s = ::googlesql::functions::ConvertDatetimeToTimestamp(
            b.datetime_value(), absl::UTCTimeZone(), &tb);
        !s.ok()) {
      return s;
    }
    auto iv = ::googlesql::functions::IntervalDiffTimestamps(ta, tb);
    if (!iv.ok()) return iv.status();
    return Value::Interval(*iv);
  }
  if (a.type_kind() == ::googlesql::TYPE_TIMESTAMP &&
      b.type_kind() == ::googlesql::TYPE_TIMESTAMP) {
    auto iv =
        ::googlesql::functions::IntervalDiffTimestamps(a.ToTime(), b.ToTime());
    if (!iv.ok()) return iv.status();
    return Value::Interval(*iv);
  }
  if (a.type_kind() != b.type_kind()) {
    return absl::InvalidArgumentError(
        absl::StrCat("semantic: '-' operands have mismatched kinds: ",
                     a.type()->DebugString(),
                     " vs ",
                     b.type()->DebugString()));
  }
  switch (a.type_kind()) {
    case ::googlesql::TYPE_INT64:
      return SubInt64(a.int64_value(), b.int64_value());
    case ::googlesql::TYPE_DOUBLE:
      return Value::Double(a.double_value() - b.double_value());
    case ::googlesql::TYPE_NUMERIC:
      return WrapNumeric(a.numeric_value().Subtract(b.numeric_value()),
                         /*is_bignum=*/false);
    case ::googlesql::TYPE_BIGNUMERIC:
      return WrapNumeric(a.bignumeric_value().Subtract(b.bignumeric_value()),
                         /*is_bignum=*/true);
    default:
      return absl::InvalidArgumentError(absl::StrCat(
          "semantic: '-' not implemented for ", a.type()->DebugString()));
  }
}

absl::StatusOr<Value> ArithmeticMul(const Value& a, const Value& b) {
  if (a.type_kind() != b.type_kind()) {
    return absl::InvalidArgumentError(
        absl::StrCat("semantic: '*' operands have mismatched kinds: ",
                     a.type()->DebugString(),
                     " vs ",
                     b.type()->DebugString()));
  }
  switch (a.type_kind()) {
    case ::googlesql::TYPE_INT64:
      return MulInt64(a.int64_value(), b.int64_value());
    case ::googlesql::TYPE_DOUBLE:
      return Value::Double(a.double_value() * b.double_value());
    case ::googlesql::TYPE_NUMERIC:
      return WrapNumeric(a.numeric_value().Multiply(b.numeric_value()),
                         /*is_bignum=*/false);
    case ::googlesql::TYPE_BIGNUMERIC:
      return WrapNumeric(a.bignumeric_value().Multiply(b.bignumeric_value()),
                         /*is_bignum=*/true);
    default:
      return absl::InvalidArgumentError(absl::StrCat(
          "semantic: '*' not implemented for ", a.type()->DebugString()));
  }
}

absl::StatusOr<Value> ArithmeticDiv(const Value& a, const Value& b) {
  // BigQuery's `<a> / <b>` lowers to FLOAT64 division regardless of
  // operand type (INT64 / INT64 is FLOAT64). The analyzer inserts
  // an explicit CAST inside the call for non-FLOAT64 operands; here
  // we only see FLOAT64-on-FLOAT64 for the `$divide` operator unless
  // the call is wrapped in SAFE_DIVIDE (a separate function).
  if (a.type_kind() == ::googlesql::TYPE_DOUBLE &&
      b.type_kind() == ::googlesql::TYPE_DOUBLE) {
    double divisor = b.double_value();
    if (divisor == 0.0) {
      return MakeSemanticError(
          SemanticErrorReason::kDivisionByZero,
          absl::StrCat("division by zero: ", a.double_value(), " / 0"));
    }
    return Value::Double(a.double_value() / divisor);
  }
  if (a.type_kind() == ::googlesql::TYPE_NUMERIC &&
      b.type_kind() == ::googlesql::TYPE_NUMERIC) {
    auto result = a.numeric_value().Divide(b.numeric_value());
    if (!result.ok()) {
      const std::string msg(result.status().message());
      if (absl::StrContains(msg, "by zero") ||
          absl::StrContains(msg, "Division by zero")) {
        return MakeSemanticError(SemanticErrorReason::kDivisionByZero, msg);
      }
      return MakeSemanticError(SemanticErrorReason::kOverflow, msg);
    }
    return Value::Numeric(*result);
  }
  if (a.type_kind() == ::googlesql::TYPE_BIGNUMERIC &&
      b.type_kind() == ::googlesql::TYPE_BIGNUMERIC) {
    auto result = a.bignumeric_value().Divide(b.bignumeric_value());
    if (!result.ok()) {
      const std::string msg(result.status().message());
      if (absl::StrContains(msg, "by zero") ||
          absl::StrContains(msg, "Division by zero")) {
        return MakeSemanticError(SemanticErrorReason::kDivisionByZero, msg);
      }
      return MakeSemanticError(SemanticErrorReason::kOverflow, msg);
    }
    return Value::BigNumeric(*result);
  }
  return absl::InvalidArgumentError(
      absl::StrCat("semantic: '/' not implemented for (",
                   a.type()->DebugString(),
                   ", ",
                   b.type()->DebugString(),
                   ")"));
}

absl::StatusOr<Value> UnaryMinus(const Value& a) {
  switch (a.type_kind()) {
    case ::googlesql::TYPE_INT64: {
      if (a.int64_value() == std::numeric_limits<int64_t>::min()) {
        return MakeSemanticError(SemanticErrorReason::kOverflow,
                                 "Int64 overflow: -INT64_MIN");
      }
      return Value::Int64(-a.int64_value());
    }
    case ::googlesql::TYPE_DOUBLE:
      return Value::Double(-a.double_value());
    case ::googlesql::TYPE_NUMERIC:
      // NumericValue::Negate is total (returns a NumericValue by
      // value): NUMERIC's max-magnitude representation is symmetric,
      // so unary minus cannot overflow on a valid value.
      return Value::Numeric(a.numeric_value().Negate());
    case ::googlesql::TYPE_BIGNUMERIC: {
      auto neg = a.bignumeric_value().Negate();
      if (!neg.ok()) {
        return MakeSemanticError(SemanticErrorReason::kOverflow,
                                 neg.status().message());
      }
      return Value::BigNumeric(*neg);
    }
    default:
      return absl::InvalidArgumentError(absl::StrCat(
          "semantic: unary '-' not implemented for ", a.type()->DebugString()));
  }
}

// ---- Comparisons -----------------------------------------------------------

// Map a 3-way comparison (a < b, a == b, a > b) onto a BOOL Value
// matching the SQL operator. The analyzer already resolved the
// operand types so they compare directly; we leverage Value::Equals
// / Value::LessThan which both honor BigQuery's collation /
// floating-point conventions (NaN handling, ARRAY ordering, ...).
}  // namespace eval_expr_internal
}  // namespace semantic
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
