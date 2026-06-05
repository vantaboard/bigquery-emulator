#include "backend/engine/semantic/eval_expr.h"

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
#include "absl/strings/ascii.h"
#include "absl/strings/match.h"
#include "absl/strings/numbers.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/string_view.h"
#include "backend/engine/semantic/error.h"
#include "backend/engine/semantic/frame_stack.h"
#include "backend/engine/semantic/functions/datetime_funcs.h"
#include "backend/engine/semantic/functions/dispatch.h"
#include "backend/engine/semantic/functions/json_funcs.h"
#include "backend/engine/semantic/functions/operator_funcs.h"
#include "backend/engine/semantic/stubs/dispatch.h"
#include "backend/engine/semantic/system_variables.h"
#include "backend/engine/semantic/value.h"
#include "googlesql/public/constant.h"
#include "googlesql/public/function.h"
#include "googlesql/public/functions/date_time_util.h"
#include "googlesql/public/functions/datetime.pb.h"
#include "googlesql/public/numeric_value.h"
#include "googlesql/public/sql_function.h"
#include "googlesql/public/templated_sql_function.h"
#include "googlesql/public/type.h"
#include "googlesql/public/type.pb.h"
#include "googlesql/public/types/struct_type.h"
#include "googlesql/public/value.h"
#include "googlesql/resolved_ast/resolved_ast.h"
#include "googlesql/resolved_ast/resolved_node_kind.pb.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace semantic {

// Implemented in `scan_eval.cc`; linked via the `scan_eval` target.
absl::StatusOr<Value> EvalSubqueryExpr(
    const ::googlesql::ResolvedSubqueryExpr& node, const EvalContext& ctx);

namespace {

// BigQuery internal functions (e.g. `IF`) often have an empty
// `FullName(false)`; fall back to `Name()` so dispatch keys match
// `DispatchFunctionByName` / lazy `if` handling.
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
absl::StatusOr<Value> CompareLess(const Value& a, const Value& b) {
  return Value::Bool(a.LessThan(b));
}
absl::StatusOr<Value> CompareEqual(const Value& a, const Value& b) {
  return Value::Bool(a.Equals(b));
}

// ---- Builtin dispatch ------------------------------------------------------

// Convert a NULL argument to the matching NULL result for operators
// that propagate NULL when ANY argument is NULL. Returns the
// matching `NullOfType(return_type)` Value if any arg is NULL, or
// `std::nullopt` if all args are non-null.
std::optional<Value> NullIfAnyNull(const std::vector<Value>& args,
                                   const ::googlesql::Type* return_type) {
  for (const auto& v : args) {
    if (v.is_null()) return NullOfType(return_type);
  }
  return std::nullopt;
}

absl::StatusOr<Value> DispatchAdd(const std::vector<Value>& args,
                                  const ::googlesql::Type* return_type) {
  if (args.size() != 2) {
    return absl::InvalidArgumentError(
        "semantic: '+' expects exactly two arguments");
  }
  if (auto n = NullIfAnyNull(args, return_type)) return *n;
  return ArithmeticAdd(args[0], args[1]);
}

absl::StatusOr<Value> DispatchSub(const std::vector<Value>& args,
                                  const ::googlesql::Type* return_type) {
  if (args.size() != 2) {
    return absl::InvalidArgumentError(
        "semantic: '-' expects exactly two arguments");
  }
  if (auto n = NullIfAnyNull(args, return_type)) return *n;
  return ArithmeticSub(args[0], args[1]);
}

absl::StatusOr<Value> DispatchMul(const std::vector<Value>& args,
                                  const ::googlesql::Type* return_type) {
  if (args.size() != 2) {
    return absl::InvalidArgumentError(
        "semantic: '*' expects exactly two arguments");
  }
  if (auto n = NullIfAnyNull(args, return_type)) return *n;
  return ArithmeticMul(args[0], args[1]);
}

absl::StatusOr<Value> DispatchDiv(const std::vector<Value>& args,
                                  const ::googlesql::Type* return_type) {
  if (args.size() != 2) {
    return absl::InvalidArgumentError(
        "semantic: '/' expects exactly two arguments");
  }
  if (auto n = NullIfAnyNull(args, return_type)) return *n;
  return ArithmeticDiv(args[0], args[1]);
}

absl::StatusOr<Value> DispatchUnaryMinus(const std::vector<Value>& args,
                                         const ::googlesql::Type* return_type) {
  if (args.size() != 1) {
    return absl::InvalidArgumentError(
        "semantic: unary '-' expects exactly one argument");
  }
  if (args[0].is_null()) return NullOfType(return_type);
  return UnaryMinus(args[0]);
}

absl::StatusOr<Value> DispatchAbs(const std::vector<Value>& args,
                                  const ::googlesql::Type* return_type) {
  if (args.size() != 1) {
    return absl::InvalidArgumentError(
        "semantic: ABS expects exactly one argument");
  }
  if (args[0].is_null()) return NullOfType(return_type);
  const Value& v = args[0];
  if (v.type_kind() == ::googlesql::TYPE_INT64) {
    const int64_t x = v.int64_value();
    return Value::Int64(x < 0 ? -x : x);
  }
  if (v.type_kind() == ::googlesql::TYPE_DOUBLE) {
    return Value::Double(std::abs(v.double_value()));
  }
  return MakeSemanticError(SemanticErrorReason::kInvalidArgument,
                           "semantic: ABS requires numeric argument");
}

absl::StatusOr<Value> DispatchEqual(const std::vector<Value>& args) {
  if (args.size() != 2) {
    return absl::InvalidArgumentError(
        "semantic: '=' expects exactly two arguments");
  }
  if (args[0].is_null() || args[1].is_null()) return Value::NullBool();
  return CompareEqual(args[0], args[1]);
}

absl::StatusOr<Value> DispatchNotEqual(const std::vector<Value>& args) {
  if (args.size() != 2) {
    return absl::InvalidArgumentError(
        "semantic: '!=' expects exactly two arguments");
  }
  if (args[0].is_null() || args[1].is_null()) return Value::NullBool();
  return Value::Bool(!args[0].Equals(args[1]));
}

absl::StatusOr<Value> DispatchLess(const std::vector<Value>& args) {
  if (args.size() != 2) {
    return absl::InvalidArgumentError(
        "semantic: '<' expects exactly two arguments");
  }
  if (args[0].is_null() || args[1].is_null()) return Value::NullBool();
  return CompareLess(args[0], args[1]);
}

absl::StatusOr<Value> DispatchLessOrEqual(const std::vector<Value>& args) {
  if (args.size() != 2) {
    return absl::InvalidArgumentError(
        "semantic: '<=' expects exactly two arguments");
  }
  if (args[0].is_null() || args[1].is_null()) return Value::NullBool();
  return Value::Bool(args[0].LessThan(args[1]) || args[0].Equals(args[1]));
}

absl::StatusOr<Value> DispatchGreater(const std::vector<Value>& args) {
  if (args.size() != 2) {
    return absl::InvalidArgumentError(
        "semantic: '>' expects exactly two arguments");
  }
  if (args[0].is_null() || args[1].is_null()) return Value::NullBool();
  return Value::Bool(args[1].LessThan(args[0]));
}

absl::StatusOr<Value> DispatchGreaterOrEqual(const std::vector<Value>& args) {
  if (args.size() != 2) {
    return absl::InvalidArgumentError(
        "semantic: '>=' expects exactly two arguments");
  }
  if (args[0].is_null() || args[1].is_null()) return Value::NullBool();
  return Value::Bool(args[1].LessThan(args[0]) || args[0].Equals(args[1]));
}

// Three-valued logic for AND: TRUE AND TRUE -> TRUE, ANY FALSE ->
// FALSE, NULL with non-FALSE -> NULL. The order matches BigQuery's
// short-circuit semantics on a TRUE operand discovered last.
absl::StatusOr<Value> DispatchAnd(const std::vector<Value>& args) {
  bool seen_null = false;
  for (const auto& v : args) {
    if (v.is_null()) {
      seen_null = true;
      continue;
    }
    if (v.type_kind() != ::googlesql::TYPE_BOOL) {
      return absl::InvalidArgumentError("semantic: AND argument is not BOOL");
    }
    if (!v.bool_value()) return Value::Bool(false);
  }
  if (seen_null) return Value::NullBool();
  return Value::Bool(true);
}

absl::StatusOr<Value> DispatchOr(const std::vector<Value>& args) {
  bool seen_null = false;
  for (const auto& v : args) {
    if (v.is_null()) {
      seen_null = true;
      continue;
    }
    if (v.type_kind() != ::googlesql::TYPE_BOOL) {
      return absl::InvalidArgumentError("semantic: OR argument is not BOOL");
    }
    if (v.bool_value()) return Value::Bool(true);
  }
  if (seen_null) return Value::NullBool();
  return Value::Bool(false);
}

absl::StatusOr<Value> DispatchNot(const std::vector<Value>& args) {
  if (args.size() != 1) {
    return absl::InvalidArgumentError(
        "semantic: NOT expects exactly one argument");
  }
  if (args[0].is_null()) return Value::NullBool();
  if (args[0].type_kind() != ::googlesql::TYPE_BOOL) {
    return absl::InvalidArgumentError("semantic: NOT argument is not BOOL");
  }
  return Value::Bool(!args[0].bool_value());
}

absl::StatusOr<Value> DispatchIsNull(const std::vector<Value>& args,
                                     bool negate) {
  if (args.size() != 1) {
    return absl::InvalidArgumentError(
        "semantic: IS [NOT] NULL expects exactly one argument");
  }
  bool is_null = args[0].is_null();
  return Value::Bool(negate ? !is_null : is_null);
}

absl::StatusOr<Value> DispatchCoalesce(const std::vector<Value>& args,
                                       const ::googlesql::Type* return_type) {
  for (const auto& v : args) {
    if (!v.is_null()) return v;
  }
  return NullOfType(return_type);
}

absl::StatusOr<Value> DispatchIfNull(const std::vector<Value>& args) {
  if (args.size() != 2) {
    return absl::InvalidArgumentError(
        "semantic: IFNULL expects exactly two arguments");
  }
  if (!args[0].is_null()) return args[0];
  return args[1];
}

absl::StatusOr<Value> DispatchNullIf(const std::vector<Value>& args,
                                     const ::googlesql::Type* return_type) {
  if (args.size() != 2) {
    return absl::InvalidArgumentError(
        "semantic: NULLIF expects exactly two arguments");
  }
  if (args[0].is_null() || args[1].is_null()) {
    return args[0];
  }
  if (args[0].Equals(args[1])) return NullOfType(return_type);
  return args[0];
}

// `IF(cond, then, else)`. NULL `cond` evaluates the else branch
// (BigQuery treats NULL as not-TRUE).
absl::StatusOr<Value> DispatchIf(const std::vector<Value>& args) {
  if (args.size() != 3) {
    return absl::InvalidArgumentError(
        "semantic: IF expects exactly three arguments");
  }
  const Value& cond = args[0];
  if (!cond.is_null() && cond.type_kind() == ::googlesql::TYPE_BOOL &&
      cond.bool_value()) {
    return args[1];
  }
  return args[2];
}

// `$case_with_value(input, when1, then1, ..., else)`. The analyzer
// always appends an explicit ELSE arm (NULL of return_type when the
// SQL omits one), so the argument list size is always even (input
// + N when/then pairs + 1 else).
absl::StatusOr<Value> DispatchCaseWithValue(
    const std::vector<Value>& args, const ::googlesql::Type* return_type) {
  if (args.size() < 2 || (args.size() % 2) != 0) {
    return absl::InvalidArgumentError(
        "semantic: $case_with_value expects an even argument list "
        "(input + N when/then pairs + else)");
  }
  const Value& input = args[0];
  for (size_t i = 1; i + 1 < args.size(); i += 2) {
    const Value& when = args[i];
    const Value& then = args[i + 1];
    // BigQuery's CASE x WHEN y semantics: when both `input` and
    // `when` are NULL the comparison is NULL (skipped); otherwise
    // they compare with the usual Equals contract.
    if (input.is_null() || when.is_null()) continue;
    if (input.Equals(when)) return then;
  }
  // ELSE arm is the last entry.
  return args.back().is_null() ? NullOfType(return_type) : args.back();
}

absl::StatusOr<Value> DispatchCaseNoValue(
    const std::vector<Value>& args, const ::googlesql::Type* return_type) {
  // `$case_no_value(cond1, then1, cond2, then2, ..., else)`. The
  // analyzer always appends an else arm.
  if (args.size() < 1 || (args.size() % 2) == 0) {
    return absl::InvalidArgumentError(
        "semantic: $case_no_value expects an odd argument list "
        "(N cond/then pairs + else)");
  }
  for (size_t i = 0; i + 1 < args.size(); i += 2) {
    const Value& cond = args[i];
    const Value& then = args[i + 1];
    if (!cond.is_null() && cond.type_kind() == ::googlesql::TYPE_BOOL &&
        cond.bool_value()) {
      return then;
    }
  }
  return args.back().is_null() ? NullOfType(return_type) : args.back();
}

// SAFE arithmetic helpers (SAFE_ADD, SAFE_SUBTRACT, SAFE_MULTIPLY,
// SAFE_NEGATE, SAFE_DIVIDE) wrap the strict operators and convert
// overflow / division-by-zero into NULL of the return type.
absl::StatusOr<Value> WrapSafe(absl::StatusOr<Value> result,
                               const ::googlesql::Type* return_type) {
  if (result.ok()) return result;
  SemanticErrorReason reason = GetSemanticErrorReason(result.status());
  if (reason == SemanticErrorReason::kOverflow ||
      reason == SemanticErrorReason::kDivisionByZero) {
    return NullOfType(return_type);
  }
  return result;
}

absl::StatusOr<Value> DispatchFunctionByName(
    absl::string_view name,
    const std::vector<Value>& args,
    const ::googlesql::Type* return_type,
    const EvalContext* ctx = nullptr) {
  if (name == "$add" || name == "add") {
    return DispatchAdd(args, return_type);
  }
  if (name == "$subtract" || name == "subtract") {
    return DispatchSub(args, return_type);
  }
  if (name == "$multiply" || name == "multiply") {
    return DispatchMul(args, return_type);
  }
  if (name == "$divide" || name == "divide") {
    return DispatchDiv(args, return_type);
  }
  if (name == "$unary_minus" || name == "unary_minus") {
    return DispatchUnaryMinus(args, return_type);
  }
  if (name == "abs") return DispatchAbs(args, return_type);
  if (name == "$make_array") {
    if (return_type == nullptr || !return_type->IsArray()) {
      return absl::InvalidArgumentError(
          "semantic: $make_array requires ARRAY return type");
    }
    return Value::Array(return_type->AsArray(), args);
  }
  if (name == "$equal" || name == "equal") {
    return DispatchEqual(args);
  }
  if (name == "$not_equal" || name == "not_equal") {
    return DispatchNotEqual(args);
  }
  if (name == "$less" || name == "less") {
    return DispatchLess(args);
  }
  if (name == "$less_or_equal" || name == "less_or_equal") {
    return DispatchLessOrEqual(args);
  }
  if (name == "$greater" || name == "greater") {
    return DispatchGreater(args);
  }
  if (name == "$greater_or_equal" || name == "greater_or_equal") {
    return DispatchGreaterOrEqual(args);
  }
  if (name == "$and" || name == "and") {
    return DispatchAnd(args);
  }
  if (name == "$or" || name == "or") {
    return DispatchOr(args);
  }
  if (name == "$not" || name == "not") {
    return DispatchNot(args);
  }
  if (name == "$is_null" || name == "is_null") {
    return DispatchIsNull(args, /*negate=*/false);
  }
  if (name == "$is_not_null" || name == "is_not_null") {
    return DispatchIsNull(args, /*negate=*/true);
  }
  if (name == "$like" || name == "$not_like") {
    return functions::DispatchLike(name, args);
  }
  if (name == "$between" || name == "$not_between") {
    return functions::DispatchBetween(name, args);
  }
  if (name == "$in" || name == "$not_in") {
    return functions::DispatchIn(name, args);
  }
  if (name == "$is_true" || name == "$is_not_true") {
    return functions::DispatchIsTrue(name, args);
  }
  if (name == "$is_false" || name == "$is_not_false") {
    return functions::DispatchIsFalse(name, args);
  }
  if (name == "$is_distinct_from" || name == "$is_not_distinct_from") {
    return functions::DispatchIsDistinctFrom(name, args);
  }
  if (name == "$bitwise_and" || name == "$bitwise_or" ||
      name == "$bitwise_xor" || name == "$bitwise_not" ||
      name == "$bitwise_left_shift" || name == "$bitwise_right_shift") {
    return functions::DispatchBitwise(name, args);
  }
  if (name == "$interval") {
    return functions::DispatchInterval(args, return_type);
  }
  if (name == "if") return DispatchIf(args);
  if (name == "coalesce") return DispatchCoalesce(args, return_type);
  if (name == "ifnull") return DispatchIfNull(args);
  if (name == "nullif") return DispatchNullIf(args, return_type);
  if (name == "$case_with_value") {
    return DispatchCaseWithValue(args, return_type);
  }
  if (name == "$case_no_value") {
    return DispatchCaseNoValue(args, return_type);
  }
  // SAFE_* functions are explicit functions in BigQuery (distinct
  // from the `SAFE.<fn>(...)` SAFE_ERROR_MODE flag). We model them
  // by routing through the underlying strict operator and
  // converting overflow / division-by-zero into NULL.
  if (name == "safe_add") {
    return WrapSafe(DispatchAdd(args, return_type), return_type);
  }
  if (name == "safe_subtract") {
    return WrapSafe(DispatchSub(args, return_type), return_type);
  }
  if (name == "safe_multiply") {
    return WrapSafe(DispatchMul(args, return_type), return_type);
  }
  if (name == "safe_negate") {
    return WrapSafe(DispatchUnaryMinus(args, return_type), return_type);
  }
  if (name == "safe_divide") {
    return WrapSafe(DispatchDiv(args, return_type), return_type);
  }
  if (absl::StartsWith(name, "$extract_")) {
    const std::string part = std::string(name).substr(9);
    if (args.empty() || args[0].is_null()) return NullOfType(return_type);
    if (part == "date" && return_type != nullptr && return_type->IsDate() &&
        args[0].type_kind() == ::googlesql::TYPE_TIMESTAMP) {
      int32_t date = 0;
      if (auto s = ::googlesql::functions::ExtractFromTimestamp(
              ::googlesql::functions::DateTimestampPart::DATE,
              args[0].ToUnixMicros(),
              ::googlesql::functions::TimestampScale::kMicroseconds,
              absl::UTCTimeZone(),
              &date);
          !s.ok()) {
        return s;
      }
      return Value::Date(date);
    }
    std::vector<Value> extract_args;
    extract_args.push_back(args[0]);
    extract_args.push_back(Value::String(absl::AsciiStrToUpper(part)));
    if (args.size() > 1) extract_args.push_back(args[1]);
    auto extracted = functions::Extract(extract_args, return_type);
    if (!extracted.ok()) return extracted.status();
    return *std::move(extracted);
  }
  // Fall through to the per-family dispatch table for functions
  // whose `functions.yaml` row picks the `semantic_executor`
  // disposition with `plan=googlesqlite-09-date-time.plan.md`.
  // `functions::Dispatch` returns nullopt when the name is not
  // wired here; we surface NOT_IMPLEMENTED in that case so the
  // gateway envelope stays the same as for an unknown function.
  if (auto dispatched = functions::Dispatch(name, args, return_type, ctx)) {
    return *std::move(dispatched);
  }
  // Local-stub families (`local_stub` posture, e.g. KEYS.*).
  // `googlesqlite-15-specialized-stubs.plan.md` picks the deterministic
  // BigQuery-shaped-placeholder posture for a handful of families;
  // the route classifier promotes the surrounding query to
  // `kLocalStub`, the coordinator dispatches it onto the semantic
  // executor, and this Dispatch finally invokes the per-family
  // handler. `stubs::Dispatch` returns nullopt for any name not
  // in its table, so the NOT_IMPLEMENTED fall-through below still
  // applies for genuinely-unsupported functions.
  if (auto dispatched = stubs::Dispatch(name, args, return_type)) {
    return *std::move(dispatched);
  }
  return MakeSemanticError(
      SemanticErrorReason::kNotImplemented,
      absl::StrCat("semantic: function '",
                   name,
                   "' is not yet implemented in the semantic executor"));
}

absl::StatusOr<Value> EvalArrayTransform(
    const ::googlesql::ResolvedFunctionCall& call, const EvalContext& ctx) {
  const ::googlesql::ResolvedExpr* array_expr = nullptr;
  const ::googlesql::ResolvedInlineLambda* lambda = nullptr;
  if (call.generic_argument_list_size() >= 2) {
    const ::googlesql::ResolvedFunctionArgument* a0 =
        call.generic_argument_list(0);
    const ::googlesql::ResolvedFunctionArgument* a1 =
        call.generic_argument_list(1);
    if (a0 != nullptr) array_expr = a0->expr();
    if (a1 != nullptr) lambda = a1->inline_lambda();
  }
  if (array_expr == nullptr && call.argument_list_size() >= 1) {
    array_expr = call.argument_list(0);
  }
  if (array_expr == nullptr || lambda == nullptr) {
    return MakeSemanticError(
        SemanticErrorReason::kNotImplemented,
        "semantic: ARRAY_TRANSFORM requires array and inline lambda arguments");
  }
  if (lambda->body() == nullptr || lambda->parameter_list_size() < 1) {
    return absl::InvalidArgumentError(
        "semantic: ARRAY_TRANSFORM lambda is malformed");
  }
  auto array_val = EvalExpr(*array_expr, ctx);
  if (!array_val.ok()) return array_val.status();
  if (array_val->is_null()) return Value::Null(call.type());
  if (!array_val->type()->IsArray()) {
    return MakeSemanticError(SemanticErrorReason::kInvalidArgument,
                             "semantic: ARRAY_TRANSFORM first argument must "
                             "be ARRAY");
  }
  const ::googlesql::ArrayType* out_arr =
      call.type() != nullptr && call.type()->IsArray()
          ? call.type()->AsArray()
          : array_val->type()->AsArray();
  const int param_col_id = lambda->parameter_list(0)->column().column_id();
  std::vector<Value> out_elems;
  out_elems.reserve(array_val->num_elements());
  for (int i = 0; i < array_val->num_elements(); ++i) {
    ColumnBindings bind;
    bind.emplace(param_col_id, array_val->element(i));
    EvalContext row_ctx = ctx;
    row_ctx.columns = &bind;
    auto mapped = EvalExpr(*lambda->body(), row_ctx);
    if (!mapped.ok()) return mapped.status();
    out_elems.push_back(*std::move(mapped));
  }
  return Value::Array(out_arr, std::move(out_elems));
}

}  // namespace

absl::StatusOr<Value> EvalIfLazy(const ::googlesql::ResolvedFunctionCall& call,
                                 const EvalContext& ctx) {
  if (call.argument_list_size() != 3) {
    return absl::InvalidArgumentError("semantic: IF expects three arguments");
  }
  auto cond = EvalExpr(*call.argument_list(0), ctx);
  if (!cond.ok()) return cond.status();
  const bool take_then = !cond->is_null() &&
                         cond->type_kind() == ::googlesql::TYPE_BOOL &&
                         cond->bool_value();
  return EvalExpr(*call.argument_list(take_then ? 1 : 2), ctx);
}

absl::StatusOr<Value> EvalIfNullLazy(
    const ::googlesql::ResolvedFunctionCall& call, const EvalContext& ctx) {
  if (call.argument_list_size() != 2) {
    return absl::InvalidArgumentError("semantic: IFNULL expects two arguments");
  }
  auto first = EvalExpr(*call.argument_list(0), ctx);
  if (!first.ok()) return first.status();
  if (!first->is_null()) return *std::move(first);
  return EvalExpr(*call.argument_list(1), ctx);
}

absl::StatusOr<Value> EvalCaseNoValueLazy(
    const ::googlesql::ResolvedFunctionCall& call, const EvalContext& ctx) {
  const int n = call.argument_list_size();
  if (n < 1 || (n % 2) == 0) {
    return absl::InvalidArgumentError(
        "semantic: CASE expects odd argument count");
  }
  for (int i = 0; i + 1 < n; i += 2) {
    auto cond = EvalExpr(*call.argument_list(i), ctx);
    if (!cond.ok()) return cond.status();
    if (!cond->is_null() && cond->type_kind() == ::googlesql::TYPE_BOOL &&
        cond->bool_value()) {
      return EvalExpr(*call.argument_list(i + 1), ctx);
    }
  }
  return EvalExpr(*call.argument_list(n - 1), ctx);
}

absl::StatusOr<Value> EvalSqlUdfBody(
    const ::googlesql::ResolvedFunctionCall& call,
    const ::googlesql::ResolvedExpr& body,
    const EvalContext& ctx) {
  const ::googlesql::Function* fn = call.function();
  if (fn == nullptr) {
    return absl::InvalidArgumentError(
        "semantic: SQL UDF call has null function");
  }
  std::vector<std::string> arg_names;
  if (fn->GetGroup() ==
      ::googlesql::TemplatedSQLFunction::kTemplatedSQLFunctionGroup) {
    arg_names = static_cast<const ::googlesql::TemplatedSQLFunction*>(fn)
                    ->GetArgumentNames();
  } else if (fn->GetGroup() == ::googlesql::SQLFunction::kSQLFunctionGroup) {
    arg_names =
        static_cast<const ::googlesql::SQLFunction*>(fn)->GetArgumentNames();
  } else {
    return absl::InvalidArgumentError(
        "semantic: SQL UDF call is not a templated or SQL function");
  }
  if (arg_names.size() != static_cast<size_t>(call.argument_list_size())) {
    return absl::InvalidArgumentError(
        absl::StrCat("semantic: SQL UDF argument count mismatch (expected ",
                     arg_names.size(),
                     ", got ",
                     call.argument_list_size(),
                     ")"));
  }
  FrameStack arg_frames;
  arg_frames.PushFrame();
  for (int i = 0; i < call.argument_list_size(); ++i) {
    auto v = EvalExpr(*call.argument_list(i), ctx);
    if (!v.ok()) return v.status();
    absl::Status declared = arg_frames.Declare(arg_names[i], *std::move(v));
    if (!declared.ok()) return declared;
  }
  EvalContext inner = ctx;
  inner.arguments = &arg_frames;
  return EvalExpr(body, inner);
}

absl::StatusOr<Value> EvalFunctionCall(
    const ::googlesql::ResolvedFunctionCall& call, const EvalContext& ctx) {
  if (call.function() == nullptr) {
    return absl::InvalidArgumentError(
        "semantic: ResolvedFunctionCall has null function");
  }
  const ::googlesql::Function* fn = call.function();
  if (const std::shared_ptr<::googlesql::ResolvedFunctionCallInfo>& info =
          call.function_call_info();
      info != nullptr && fn != nullptr &&
      fn->GetGroup() ==
          ::googlesql::TemplatedSQLFunction::kTemplatedSQLFunctionGroup) {
    const auto* templated =
        static_cast<const ::googlesql::TemplatedSQLFunctionCall*>(info.get());
    if (templated->expr() != nullptr) {
      return EvalSqlUdfBody(call, *templated->expr(), ctx);
    }
  }
  if (fn != nullptr &&
      fn->GetGroup() == ::googlesql::SQLFunction::kSQLFunctionGroup) {
    const auto* sql_fn = static_cast<const ::googlesql::SQLFunction*>(fn);
    if (sql_fn->FunctionExpression() != nullptr) {
      return EvalSqlUdfBody(call, *sql_fn->FunctionExpression(), ctx);
    }
  }
  const std::string name = LowerFunctionDispatchName(call.function());
  if (name == "array_transform") {
    return EvalArrayTransform(call, ctx);
  }
  if (name == "if") {
    return EvalIfLazy(call, ctx);
  }
  if (name == "ifnull") {
    return EvalIfNullLazy(call, ctx);
  }
  if (name == "$case_no_value") {
    return EvalCaseNoValueLazy(call, ctx);
  }
  if (name == "error" && call.argument_list_size() == 1) {
    auto msg = EvalExpr(*call.argument_list(0), ctx);
    if (!msg.ok()) return msg.status();
    std::vector<Value> args = {*std::move(msg)};
    return DispatchFunctionByName(name, args, call.type(), &ctx);
  }
  std::vector<Value> args;
  args.reserve(call.argument_list_size());
  for (int i = 0; i < call.argument_list_size(); ++i) {
    const ::googlesql::ResolvedExpr* arg = call.argument_list(i);
    if (arg == nullptr) {
      return absl::InvalidArgumentError(
          "semantic: ResolvedFunctionCall argument is null");
    }
    auto v = EvalExpr(*arg, ctx);
    if (!v.ok()) {
      // SAFE_ERROR_MODE swallows evaluation failures from any
      // operand and converts them into NULL of the return type.
      if (call.error_mode() ==
              ::googlesql::ResolvedFunctionCallBase::SAFE_ERROR_MODE &&
          (v.status().code() == absl::StatusCode::kInvalidArgument ||
           v.status().code() == absl::StatusCode::kOutOfRange)) {
        return NullOfType(call.type());
      }
      return v.status();
    }
    args.push_back(*std::move(v));
  }
  // Use `FullName(/*include_group=*/false)` so namespaced families
  // like `KEYS.NEW_KEYSET` / `NET.HOST` / `HLL_COUNT.MERGE`
  // resolve to their dotted, lowercased dispatch key
  // (`keys.new_keyset`, `net.host`, `hll_count.merge`). The route
  // classifier (`route_classifier.cc::CheckFunction`) uses the
  // same name shape when promoting `local_stub` / `semantic_executor`
  // dispositions, so the names line up across the two sides. For
  // non-namespaced functions (`concat`, `abs`, `safe_divide`)
  // `FullName(false) == Name()`, so this is a no-op.
  auto result = DispatchFunctionByName(name, args, call.type(), &ctx);
  if (!result.ok() &&
      call.error_mode() ==
          ::googlesql::ResolvedFunctionCallBase::SAFE_ERROR_MODE) {
    SemanticErrorReason reason = GetSemanticErrorReason(result.status());
    if (reason == SemanticErrorReason::kOverflow ||
        reason == SemanticErrorReason::kDivisionByZero ||
        reason == SemanticErrorReason::kInvalidArgument) {
      return NullOfType(call.type());
    }
  }
  return result;
}

absl::StatusOr<Value> EvalExpr(const ::googlesql::ResolvedExpr& expr,
                               const EvalContext& ctx) {
  switch (expr.node_kind()) {
    case ::googlesql::RESOLVED_LITERAL: {
      const auto& lit = *expr.GetAs<::googlesql::ResolvedLiteral>();
      // ResolvedLiteral carries the analyzer-validated `Value`
      // directly; copying it is cheap (refcounted backing for
      // STRING / ARRAY / STRUCT).
      // cpp-lint:allow(statusor-unchecked-value) -- `lit.value()`
      // is `ResolvedLiteral::value()` returning `googlesql::Value`,
      // not a `StatusOr<T>::value()` unwrap.
      return lit.value();
    }
    case ::googlesql::RESOLVED_PARAMETER: {
      const auto& param = *expr.GetAs<::googlesql::ResolvedParameter>();
      if (ctx.parameters == nullptr) {
        return absl::InvalidArgumentError(
            "semantic: ResolvedParameter referenced but no parameter "
            "bindings supplied");
      }
      if (param.is_untyped()) {
        return MakeSemanticError(
            SemanticErrorReason::kInvalidArgument,
            "semantic: untyped parameter has no value to bind to");
      }
      if (!param.name().empty()) {
        const std::string key = absl::AsciiStrToLower(param.name());
        auto it = ctx.parameters->by_name.find(key);
        if (it == ctx.parameters->by_name.end()) {
          return MakeSemanticError(
              SemanticErrorReason::kInvalidArgument,
              absl::StrCat("semantic: no value bound for parameter @",
                           param.name()));
        }
        return it->second;
      }
      if (param.position() > 0) {
        size_t idx = static_cast<size_t>(param.position()) - 1;
        if (idx >= ctx.parameters->by_position.size()) {
          return MakeSemanticError(
              SemanticErrorReason::kInvalidArgument,
              absl::StrCat(
                  "semantic: no value bound for positional parameter #",
                  param.position()));
        }
        return ctx.parameters->by_position[idx];
      }
      return absl::InvalidArgumentError(
          "semantic: ResolvedParameter has neither name nor position");
    }
    case ::googlesql::RESOLVED_FUNCTION_CALL:
      return EvalFunctionCall(*expr.GetAs<::googlesql::ResolvedFunctionCall>(),
                              ctx);
    case ::googlesql::RESOLVED_CAST: {
      const auto& cast = *expr.GetAs<::googlesql::ResolvedCast>();
      if (cast.expr() == nullptr) {
        return absl::InvalidArgumentError(
            "semantic: ResolvedCast has null expr");
      }
      auto inner = EvalExpr(*cast.expr(), ctx);
      if (!inner.ok()) return inner;
      const ::googlesql::Type* target = cast.type();
      if (target == nullptr) {
        return absl::InvalidArgumentError(
            "semantic: ResolvedCast has null type");
      }
      const ::googlesql::Type* source = cast.expr()->type();
      if (source != nullptr && source->Equals(target)) {
        return inner;
      }
      // The semantic executor's CAST surface is intentionally
      // narrow today; the full table lives with
      // `googlesqlite-09-date-time.plan.md`. We cover the
      // implicit-coercion casts the analyzer inserts inside scalar
      // arithmetic (INT64 -> FLOAT64 for `/`, ...).
      if (inner->is_null()) return NullOfType(target);
      if (target->kind() == ::googlesql::TYPE_DOUBLE) {
        auto d = ToDouble(*inner);
        if (!d.ok()) return d.status();
        return Value::Double(*d);
      }
      if (target->kind() == ::googlesql::TYPE_STRING) {
        if (inner->type_kind() == ::googlesql::TYPE_INT64) {
          return Value::String(absl::StrCat(inner->int64_value()));
        }
        if (inner->type_kind() == ::googlesql::TYPE_BOOL) {
          return Value::String(inner->bool_value() ? "true" : "false");
        }
        if (inner->type_kind() == ::googlesql::TYPE_DOUBLE) {
          return Value::String(absl::StrCat(inner->double_value()));
        }
        if (inner->type_kind() == ::googlesql::TYPE_NUMERIC) {
          return Value::String(inner->numeric_value().ToString());
        }
        if (inner->type_kind() == ::googlesql::TYPE_BIGNUMERIC) {
          return Value::String(inner->bignumeric_value().ToString());
        }
      }
      if (target->kind() == ::googlesql::TYPE_INT64) {
        if (inner->type_kind() == ::googlesql::TYPE_INT64) return inner;
        if (inner->type_kind() == ::googlesql::TYPE_STRING) {
          if (inner->is_null()) return Value::NullInt64();
          absl::string_view s = inner->string_value();
          if (!s.empty() && (s[0] == '+' || s[0] == '-')) {
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
          if (!inner->string_value().empty() &&
              inner->string_value()[0] == '-') {
            parsed = -parsed;
          }
          return Value::Int64(parsed);
        }
      }
      return MakeSemanticError(
          SemanticErrorReason::kNotImplemented,
          absl::StrCat("semantic: CAST from ",
                       source != nullptr ? source->DebugString() : "<null>",
                       " to ",
                       target->DebugString(),
                       " is not yet implemented"));
    }
    case ::googlesql::RESOLVED_ARGUMENT_REF: {
      // `ResolvedArgumentRef` reads an argument of the enclosing
      // SQL UDF / TVF invocation. The caller (UDF / TVF executor
      // body) pushes a `FrameStack` frame at invocation, declares
      // each argument by name, and points `ctx.arguments` at the
      // frame stack. The analyzer canonicalizes argument names to
      // lower-case at registration, so the frame stack's case-
      // insensitive `Lookup` returns the right binding even if a
      // body case-shifts the reference (e.g. `RETURN X` vs. the
      // signature's `x`).
      //
      // Argument references arriving here with no frame stack
      // (i.e. `ctx.arguments == nullptr`) mean either: (a) the
      // analyzer emitted a `ResolvedArgumentRef` outside a UDF /
      // TVF body (engine wiring bug), or (b) the body is being
      // evaluated without the invocation frame plumbed through
      // (caller bug). Either way we surface a structured
      // `kInvalidArgument` so the gateway envelope names the
      // missing argument rather than silently substituting NULL.
      const auto& ref = *expr.GetAs<::googlesql::ResolvedArgumentRef>();
      if (ctx.arguments == nullptr) {
        return MakeSemanticError(
            SemanticErrorReason::kInvalidArgument,
            absl::StrCat("semantic: ResolvedArgumentRef '",
                         ref.name(),
                         "' evaluated without an invocation frame; UDF / "
                         "TVF body executors must populate "
                         "EvalContext::arguments before calling EvalExpr"));
      }
      absl::StatusOr<Value> bound = ctx.arguments->Lookup(ref.name());
      if (!bound.ok()) {
        return MakeSemanticError(
            SemanticErrorReason::kInvalidArgument,
            absl::StrCat("semantic: ResolvedArgumentRef '",
                         ref.name(),
                         "' has no binding on the invocation frame: ",
                         bound.status().message()));
      }
      return *std::move(bound);
    }
    case ::googlesql::RESOLVED_CONSTANT: {
      // `ResolvedConstant` carries a non-owning pointer to a
      // `googlesql::Constant` registered on the catalog. The
      // BigQuery emulator's catalog adapter (today only the
      // analyzer's built-in constants, future module-defined
      // constants once `CREATE CONSTANT` lands) stores
      // `SimpleConstant` instances whose `HasValue()` is true and
      // `GetValue()` returns the bound `Value` verbatim. Constants
      // whose value is not available yet (e.g. an unresolved
      // `SQLConstant`) surface as a structured `kInvalidArgument`
      // so the gateway envelope names the constant rather than
      // silently substituting NULL.
      const auto& node = *expr.GetAs<::googlesql::ResolvedConstant>();
      const ::googlesql::Constant* constant = node.constant();
      if (constant == nullptr) {
        return absl::InternalError(
            "semantic: ResolvedConstant has null constant pointer");
      }
      if (!constant->HasValue()) {
        return MakeSemanticError(
            SemanticErrorReason::kInvalidArgument,
            absl::StrCat("semantic: constant '",
                         constant->FullName(),
                         "' has no bound value (catalog returned "
                         "HasValue=false)"));
      }
      absl::StatusOr<Value> value = constant->GetValue();
      if (!value.ok()) {
        return MakeSemanticError(SemanticErrorReason::kInvalidArgument,
                                 absl::StrCat("semantic: constant '",
                                              constant->FullName(),
                                              "' failed to provide its value: ",
                                              value.status().message()));
      }
      return *std::move(value);
    }
    case ::googlesql::RESOLVED_COLUMN_REF: {
      // A `ResolvedColumnRef` reads a column the surrounding scan
      // emits row-at-a-time. The FROM-clause executor binds the
      // current row's `ColumnBindings` onto `ctx.columns` before
      // calling `EvalExpr`; a missing binding is an analyzer /
      // executor mismatch and surfaces a structured INVALID_ARGUMENT.
      // Scalar-only SELECT keeps `ctx.columns == nullptr` and
      // every column reference there is a bug -- the scalar path
      // resolves columns through `ResolvedProjectScan::expr_list`,
      // not through column refs.
      const auto& ref = *expr.GetAs<::googlesql::ResolvedColumnRef>();
      if (ctx.columns == nullptr) {
        return MakeSemanticError(
            SemanticErrorReason::kNotImplemented,
            absl::StrCat(
                "semantic: ResolvedColumnRef '",
                ref.column().name(),
                "' referenced without a row binding; correlated scans "
                "are owned by googlesqlite-12-arrays-generators.plan.md "
                "(Family 4 / googlesqlite-02-withscan-cte.plan.md)"));
      }
      auto it = ctx.columns->find(ref.column().column_id());
      if (it == ctx.columns->end()) {
        return MakeSemanticError(
            SemanticErrorReason::kInvalidArgument,
            absl::StrCat("semantic: no row binding for column '",
                         ref.column().name(),
                         "' (column_id=",
                         ref.column().column_id(),
                         ")"));
      }
      return it->second;
    }
    case ::googlesql::RESOLVED_MAKE_STRUCT: {
      const auto& node = *expr.GetAs<::googlesql::ResolvedMakeStruct>();
      const ::googlesql::Type* type = node.type();
      if (type == nullptr || !type->IsStruct()) {
        return absl::InvalidArgumentError(
            "semantic: ResolvedMakeStruct has non-STRUCT type");
      }
      const ::googlesql::StructType* st = type->AsStruct();
      if (st == nullptr || st->num_fields() != node.field_list_size()) {
        return absl::InvalidArgumentError(
            "semantic: ResolvedMakeStruct field count mismatch");
      }
      std::vector<Value> fields;
      fields.reserve(node.field_list_size());
      for (int i = 0; i < node.field_list_size(); ++i) {
        const ::googlesql::ResolvedExpr* field_expr = node.field_list(i);
        if (field_expr == nullptr) {
          return absl::InvalidArgumentError(
              "semantic: ResolvedMakeStruct field is null");
        }
        auto v = EvalExpr(*field_expr, ctx);
        if (!v.ok()) return v.status();
        fields.push_back(*std::move(v));
      }
      return Value::Struct(st, std::move(fields));
    }
    case ::googlesql::RESOLVED_GET_STRUCT_FIELD: {
      const auto& node = *expr.GetAs<::googlesql::ResolvedGetStructField>();
      if (node.expr() == nullptr) {
        return absl::InvalidArgumentError(
            "semantic: ResolvedGetStructField has null expr");
      }
      auto base = EvalExpr(*node.expr(), ctx);
      if (!base.ok()) return base.status();
      if (base->is_null()) return NullOfType(expr.type());
      if (!base->type()->IsStruct()) {
        return absl::InvalidArgumentError(
            "semantic: GetStructField base is not STRUCT");
      }
      const int idx = node.field_idx();
      if (idx < 0 || idx >= base->num_fields()) {
        return absl::InvalidArgumentError(
            "semantic: GetStructField index out of range");
      }
      return base->field(idx);
    }
    case ::googlesql::RESOLVED_GET_JSON_FIELD: {
      const auto& node = *expr.GetAs<::googlesql::ResolvedGetJsonField>();
      if (node.expr() == nullptr) {
        return absl::InvalidArgumentError(
            "semantic: ResolvedGetJsonField has null expr");
      }
      auto base = EvalExpr(*node.expr(), ctx);
      if (!base.ok()) return base.status();
      return functions::JsonGetField(*base, node.field_name(), expr.type());
    }
    case ::googlesql::RESOLVED_SUBQUERY_EXPR:
      return EvalSubqueryExpr(*expr.GetAs<::googlesql::ResolvedSubqueryExpr>(),
                              ctx);
    case ::googlesql::RESOLVED_SYSTEM_VARIABLE: {
      const auto& node = *expr.GetAs<::googlesql::ResolvedSystemVariable>();
      return GetSystemVariable(ctx.project_id, node.name_path());
    }
    default:
      return MakeSemanticError(SemanticErrorReason::kNotImplemented,
                               absl::StrCat("semantic: ResolvedExpr kind ",
                                            expr.node_kind_string(),
                                            " is not yet implemented"));
  }
}

}  // namespace semantic
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
