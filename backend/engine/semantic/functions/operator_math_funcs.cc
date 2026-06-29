#include <cmath>
#include <cstdint>

#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "absl/strings/numbers.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/string_view.h"
#include "backend/engine/semantic/error.h"
#include "googlesql/public/functions/math.h"
#include "googlesql/public/numeric_value.h"
#include "googlesql/public/type.h"
#include "googlesql/public/type.pb.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace semantic {
namespace functions {

namespace {

using ::googlesql::BigNumericValue;
using ::googlesql::NumericValue;

absl::StatusOr<double> NumericArgToDouble(const Value& v) {
  switch (v.type_kind()) {
    case ::googlesql::TYPE_DOUBLE:
      return v.double_value();
    case ::googlesql::TYPE_FLOAT:
      return static_cast<double>(v.float_value());
    case ::googlesql::TYPE_INT64:
      return static_cast<double>(v.int64_value());
    case ::googlesql::TYPE_NUMERIC:
      return v.numeric_value().ToDouble();
    case ::googlesql::TYPE_BIGNUMERIC:
      return v.bignumeric_value().ToDouble();
    case ::googlesql::TYPE_STRING: {
      double parsed = 0;
      if (!absl::SimpleAtod(v.string_value(), &parsed)) {
        return absl::InvalidArgumentError(
            "semantic: cannot coerce STRING to FLOAT64");
      }
      return parsed;
    }
    default:
      return absl::InvalidArgumentError(absl::StrCat(
          "semantic: cannot coerce ", v.type()->DebugString(), " to FLOAT64"));
  }
}

absl::StatusOr<BigNumericValue> ValueToBigNumeric(const Value& v) {
  switch (v.type_kind()) {
    case ::googlesql::TYPE_BIGNUMERIC:
      return v.bignumeric_value();
    case ::googlesql::TYPE_NUMERIC:
      return BigNumericValue(v.numeric_value());
    case ::googlesql::TYPE_INT64:
      return BigNumericValue(v.int64_value());
    case ::googlesql::TYPE_DOUBLE: {
      auto n = BigNumericValue::FromDouble(v.double_value());
      if (!n.ok()) return n.status();
      return *n;
    }
    default:
      return absl::InvalidArgumentError(absl::StrCat("semantic: cannot coerce ",
                                                     v.type()->DebugString(),
                                                     " to BIGNUMERIC"));
  }
}

absl::StatusOr<NumericValue> ValueToNumeric(const Value& v) {
  switch (v.type_kind()) {
    case ::googlesql::TYPE_NUMERIC:
      return v.numeric_value();
    case ::googlesql::TYPE_INT64: {
      auto n = NumericValue::FromString(absl::StrCat(v.int64_value()));
      if (!n.ok()) return n.status();
      return *n;
    }
    case ::googlesql::TYPE_BIGNUMERIC: {
      auto n = v.bignumeric_value().ToNumericValue();
      if (!n.ok()) return n.status();
      return *n;
    }
    default:
      return absl::InvalidArgumentError(absl::StrCat(
          "semantic: cannot coerce ", v.type()->DebugString(), " to NUMERIC"));
  }
}

double TruncDoubleTowardZero(double x, int64_t digits) {
  if (digits == 0) return std::trunc(x);
  const double factor = std::pow(10.0, static_cast<double>(digits));
  return std::trunc(x * factor) / factor;
}

double RoundHalfAwayFromZero(double x, int64_t precision) {
  if (precision == 0) {
    if (x >= 0.0) return std::floor(x + 0.5);
    return std::ceil(x - 0.5);
  }
  const double factor = std::pow(10.0, static_cast<double>(precision));
  const double scaled = x * factor;
  double rounded = 0.0;
  if (scaled >= 0.0) {
    rounded = std::floor(scaled + 0.5);
  } else {
    rounded = std::ceil(scaled - 0.5);
  }
  return rounded / factor;
}

template <typename NV>
absl::StatusOr<Value> WrapNumericResult(absl::StatusOr<NV> result) {
  if (!result.ok()) {
    if (result.status().code() == absl::StatusCode::kOutOfRange) {
      return MakeSemanticError(SemanticErrorReason::kOverflow,
                               result.status().message());
    }
    return MakeSemanticError(SemanticErrorReason::kInvalidArgument,
                             result.status().message());
  }
  if constexpr (std::is_same_v<NV, BigNumericValue>) {
    return Value::BigNumeric(*result);
  } else {
    return Value::Numeric(*result);
  }
}

}  // namespace

absl::StatusOr<Value> UnaryMathOnNumeric(const std::vector<Value>& args,
                                         absl::string_view fn_name,
                                         double (*op)(double)) {
  if (args.size() != 1) {
    return absl::InvalidArgumentError(
        absl::StrCat("semantic: ", fn_name, " expects exactly one argument"));
  }
  if (args[0].is_null()) return Value::NullDouble();
  auto x = NumericArgToDouble(args[0]);
  if (!x.ok()) return x.status();
  return Value::Double(op(*x));
}

absl::StatusOr<Value> Floor(const std::vector<Value>& args) {
  if (args.size() != 1) {
    return absl::InvalidArgumentError(
        "semantic: FLOOR expects exactly one argument");
  }
  if (args[0].is_null()) return Value::Null(args[0].type());
  if (args[0].type_kind() == ::googlesql::TYPE_BIGNUMERIC) {
    auto out = args[0].bignumeric_value().Floor();
    return WrapNumericResult(out);
  }
  if (args[0].type_kind() == ::googlesql::TYPE_NUMERIC) {
    auto out = args[0].numeric_value().Floor();
    return WrapNumericResult(out);
  }
  return UnaryMathOnNumeric(
      args, "FLOOR", static_cast<double (*)(double)>(std::floor));
}

absl::StatusOr<Value> Ceil(const std::vector<Value>& args) {
  if (args.size() != 1) {
    return absl::InvalidArgumentError(
        "semantic: CEIL expects exactly one argument");
  }
  if (args[0].is_null()) return Value::Null(args[0].type());
  if (args[0].type_kind() == ::googlesql::TYPE_BIGNUMERIC) {
    auto out = args[0].bignumeric_value().Ceiling();
    return WrapNumericResult(out);
  }
  if (args[0].type_kind() == ::googlesql::TYPE_NUMERIC) {
    auto out = args[0].numeric_value().Ceiling();
    return WrapNumericResult(out);
  }
  return UnaryMathOnNumeric(
      args, "CEIL", static_cast<double (*)(double)>(std::ceil));
}

absl::StatusOr<Value> Trunc(const std::vector<Value>& args) {
  if (args.empty() || args.size() > 2) {
    return absl::InvalidArgumentError(
        "semantic: TRUNC expects one or two arguments");
  }
  if (args[0].is_null()) return Value::Null(args[0].type());
  int64_t digits = 0;
  if (args.size() == 2) {
    if (args[1].is_null()) return Value::Null(args[0].type());
    if (args[1].type_kind() != ::googlesql::TYPE_INT64) {
      return absl::InvalidArgumentError(
          "semantic: TRUNC precision must be INT64");
    }
    digits = args[1].int64_value();
  }
  if (args[0].type_kind() == ::googlesql::TYPE_BIGNUMERIC) {
    return Value::BigNumeric(args[0].bignumeric_value().Trunc(digits));
  }
  if (args[0].type_kind() == ::googlesql::TYPE_NUMERIC) {
    return Value::Numeric(args[0].numeric_value().Trunc(digits));
  }
  auto x = NumericArgToDouble(args[0]);
  if (!x.ok()) return x.status();
  return Value::Double(TruncDoubleTowardZero(*x, digits));
}

absl::StatusOr<Value> Sign(const std::vector<Value>& args) {
  if (args.size() != 1) {
    return absl::InvalidArgumentError(
        "semantic: SIGN expects exactly one argument");
  }
  if (args[0].is_null()) return Value::NullInt64();
  switch (args[0].type_kind()) {
    case ::googlesql::TYPE_INT64: {
      const int64_t x = args[0].int64_value();
      return Value::Int64(x == 0 ? 0 : (x > 0 ? 1 : -1));
    }
    case ::googlesql::TYPE_DOUBLE:
      return Value::Int64(args[0].double_value() == 0.0
                              ? 0
                              : (args[0].double_value() > 0.0 ? 1 : -1));
    case ::googlesql::TYPE_NUMERIC:
      return Value::Int64(args[0].numeric_value().Sign());
    case ::googlesql::TYPE_BIGNUMERIC:
      return Value::Int64(args[0].bignumeric_value().Sign());
    default:
      return absl::InvalidArgumentError(
          "semantic: SIGN requires a numeric argument");
  }
}

absl::StatusOr<Value> Div(const std::vector<Value>& args) {
  if (args.size() != 2) {
    return absl::InvalidArgumentError(
        "semantic: DIV expects exactly two arguments");
  }
  if (args[0].is_null() || args[1].is_null()) {
    return Value::NullInt64();
  }
  if (args[0].type_kind() != ::googlesql::TYPE_INT64 ||
      args[1].type_kind() != ::googlesql::TYPE_INT64) {
    return absl::InvalidArgumentError("semantic: DIV requires INT64 operands");
  }
  const int64_t dividend = args[0].int64_value();
  const int64_t divisor = args[1].int64_value();
  if (divisor == 0) {
    return MakeSemanticError(SemanticErrorReason::kDivisionByZero,
                             "semantic: division by zero: DIV");
  }
  return Value::Int64(dividend / divisor);
}

namespace {

absl::StatusOr<Value> ModBigNumericValues(const Value& lhs_value,
                                          const Value& rhs_value) {
  auto lhs = ValueToBigNumeric(lhs_value);
  if (!lhs.ok()) return lhs.status();
  auto rhs = ValueToBigNumeric(rhs_value);
  if (!rhs.ok()) return rhs.status();
  if (*rhs == BigNumericValue(0)) {
    return MakeSemanticError(SemanticErrorReason::kDivisionByZero,
                             "semantic: division by zero: MOD");
  }
  return WrapNumericResult(lhs->Mod(*rhs));
}

absl::StatusOr<Value> ModNumericValues(const Value& lhs_value,
                                       const Value& rhs_value) {
  auto lhs = ValueToNumeric(lhs_value);
  if (!lhs.ok()) return lhs.status();
  auto rhs = ValueToNumeric(rhs_value);
  if (!rhs.ok()) return rhs.status();
  if (*rhs == NumericValue(0)) {
    return MakeSemanticError(SemanticErrorReason::kDivisionByZero,
                             "semantic: division by zero: MOD");
  }
  return WrapNumericResult(lhs->Mod(*rhs));
}

}  // namespace

absl::StatusOr<Value> Mod(const std::vector<Value>& args) {
  if (args.size() != 2) {
    return absl::InvalidArgumentError(
        "semantic: MOD expects exactly two arguments");
  }
  if (args[0].is_null() || args[1].is_null()) {
    return Value::Null(args[0].type());
  }
  const auto kind0 = args[0].type_kind();
  const auto kind1 = args[1].type_kind();
  if (kind0 == ::googlesql::TYPE_BIGNUMERIC ||
      kind1 == ::googlesql::TYPE_BIGNUMERIC) {
    return ModBigNumericValues(args[0], args[1]);
  }
  if (kind0 == ::googlesql::TYPE_NUMERIC ||
      kind1 == ::googlesql::TYPE_NUMERIC) {
    return ModNumericValues(args[0], args[1]);
  }
  if (kind0 != ::googlesql::TYPE_INT64 || kind1 != ::googlesql::TYPE_INT64) {
    return absl::InvalidArgumentError(
        "semantic: MOD requires numeric operands");
  }
  const int64_t dividend = args[0].int64_value();
  const int64_t divisor = args[1].int64_value();
  if (divisor == 0) {
    return MakeSemanticError(SemanticErrorReason::kDivisionByZero,
                             "semantic: division by zero: MOD");
  }
  int64_t remainder = dividend % divisor;
  if ((remainder < 0 && divisor > 0) || (remainder > 0 && divisor < 0)) {
    remainder += divisor;
  }
  return Value::Int64(remainder);
}

absl::StatusOr<Value> Log(const std::vector<Value>& args) {
  if (args.size() != 1) {
    return absl::InvalidArgumentError(
        "semantic: LOG expects exactly one argument");
  }
  if (args[0].is_null()) return Value::NullDouble();
  auto x = NumericArgToDouble(args[0]);
  if (!x.ok()) return x.status();
  if (*x <= 0.0) {
    return absl::InvalidArgumentError(
        "semantic: LOG requires a positive argument");
  }
  return Value::Double(std::log(*x));
}

absl::StatusOr<Value> Sqrt(const std::vector<Value>& args) {
  if (args.size() != 1) {
    return absl::InvalidArgumentError(
        "semantic: SQRT expects exactly one argument");
  }
  if (args[0].is_null()) return Value::Null(args[0].type());
  if (args[0].type_kind() == ::googlesql::TYPE_BIGNUMERIC) {
    ::googlesql::BigNumericValue out;
    absl::Status error;
    if (!::googlesql::functions::Sqrt(
            args[0].bignumeric_value(), &out, &error)) {
      return error;
    }
    return Value::BigNumeric(out);
  }
  if (args[0].type_kind() == ::googlesql::TYPE_NUMERIC) {
    ::googlesql::NumericValue out;
    absl::Status error;
    if (!::googlesql::functions::Sqrt(args[0].numeric_value(), &out, &error)) {
      return error;
    }
    return Value::Numeric(out);
  }
  return UnaryMathOnNumeric(
      args, "SQRT", static_cast<double (*)(double)>(std::sqrt));
}

absl::StatusOr<Value> Sin(const std::vector<Value>& args) {
  return UnaryMathOnNumeric(
      args, "SIN", static_cast<double (*)(double)>(std::sin));
}

absl::StatusOr<Value> Cos(const std::vector<Value>& args) {
  return UnaryMathOnNumeric(
      args, "COS", static_cast<double (*)(double)>(std::cos));
}

absl::StatusOr<Value> Asin(const std::vector<Value>& args) {
  return UnaryMathOnNumeric(
      args, "ASIN", static_cast<double (*)(double)>(std::asin));
}

absl::StatusOr<Value> Acos(const std::vector<Value>& args) {
  return UnaryMathOnNumeric(
      args, "ACOS", static_cast<double (*)(double)>(std::acos));
}

absl::StatusOr<Value> Atan2(const std::vector<Value>& args) {
  if (args.size() != 2) {
    return absl::InvalidArgumentError(
        "semantic: ATAN2 expects exactly two arguments");
  }
  if (args[0].is_null() || args[1].is_null()) return Value::NullDouble();
  auto y = NumericArgToDouble(args[0]);
  if (!y.ok()) return y.status();
  auto x = NumericArgToDouble(args[1]);
  if (!x.ok()) return x.status();
  return Value::Double(std::atan2(*y, *x));
}

absl::StatusOr<Value> Pow(const std::vector<Value>& args) {
  if (args.size() != 2) {
    return absl::InvalidArgumentError(
        "semantic: POW expects exactly two arguments");
  }
  if (args[0].is_null() || args[1].is_null()) {
    return Value::Null(args[0].type());
  }
  const auto kind0 = args[0].type_kind();
  const auto kind1 = args[1].type_kind();
  if (kind0 == ::googlesql::TYPE_BIGNUMERIC ||
      kind1 == ::googlesql::TYPE_BIGNUMERIC) {
    auto base = ValueToBigNumeric(args[0]);
    if (!base.ok()) return base.status();
    auto exp = ValueToBigNumeric(args[1]);
    if (!exp.ok()) return exp.status();
    return WrapNumericResult(base->Power(*exp));
  }
  if (kind0 == ::googlesql::TYPE_NUMERIC ||
      kind1 == ::googlesql::TYPE_NUMERIC) {
    auto base = ValueToNumeric(args[0]);
    if (!base.ok()) return base.status();
    auto exp = ValueToNumeric(args[1]);
    if (!exp.ok()) return exp.status();
    return WrapNumericResult(base->Power(*exp));
  }
  auto base = NumericArgToDouble(args[0]);
  if (!base.ok()) return base.status();
  auto exp = NumericArgToDouble(args[1]);
  if (!exp.ok()) return exp.status();
  return Value::Double(std::pow(*base, *exp));
}

absl::StatusOr<Value> Round(const std::vector<Value>& args) {
  if (args.empty() || args.size() > 2) {
    return absl::InvalidArgumentError(
        "semantic: ROUND expects one or two arguments");
  }
  if (args[0].is_null()) return Value::NullDouble();
  int64_t precision = 0;
  if (args.size() == 2) {
    if (args[1].is_null()) return Value::NullDouble();
    if (args[1].type_kind() != ::googlesql::TYPE_INT64) {
      return absl::InvalidArgumentError(
          "semantic: ROUND precision must be INT64");
    }
    precision = args[1].int64_value();
  }
  auto x = NumericArgToDouble(args[0]);
  if (!x.ok()) return x.status();
  return Value::Double(RoundHalfAwayFromZero(*x, precision));
}

}  // namespace functions
}  // namespace semantic
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
