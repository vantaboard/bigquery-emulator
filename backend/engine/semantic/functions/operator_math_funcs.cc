#include <cmath>
#include <cstdint>
#include <vector>

#include "absl/status/statusor.h"
#include "absl/strings/numbers.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/string_view.h"
#include "backend/engine/semantic/functions/operator_funcs.h"
#include "backend/engine/semantic/value.h"
#include "googlesql/public/type.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace semantic {
namespace functions {

namespace {

absl::StatusOr<double> NumericArgToDouble(const Value& v) {
  switch (v.type_kind()) {
    case ::googlesql::TYPE_DOUBLE:
      return v.double_value();
    case ::googlesql::TYPE_FLOAT:
      return static_cast<double>(v.float_value());
    case ::googlesql::TYPE_INT64:
      return static_cast<double>(v.int64_value());
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

double RoundHalfAwayFromZero(double x, int64_t precision) {
  if (precision == 0) {
    if (x >= 0.0) return std::floor(x + 0.5);
    return std::ceil(x - 0.5);
  }
  const double factor = std::pow(10.0, static_cast<double>(precision));
  const double scaled = x * factor;
  double rounded;
  if (scaled >= 0.0) {
    rounded = std::floor(scaled + 0.5);
  } else {
    rounded = std::ceil(scaled - 0.5);
  }
  return rounded / factor;
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
  return UnaryMathOnNumeric(
      args, "FLOOR", static_cast<double (*)(double)>(std::floor));
}

absl::StatusOr<Value> Ceil(const std::vector<Value>& args) {
  return UnaryMathOnNumeric(
      args, "CEIL", static_cast<double (*)(double)>(std::ceil));
}

absl::StatusOr<Value> Mod(const std::vector<Value>& args) {
  if (args.size() != 2) {
    return absl::InvalidArgumentError(
        "semantic: MOD expects exactly two arguments");
  }
  if (args[0].is_null() || args[1].is_null()) {
    return Value::NullInt64();
  }
  if (args[0].type_kind() != ::googlesql::TYPE_INT64 ||
      args[1].type_kind() != ::googlesql::TYPE_INT64) {
    return absl::InvalidArgumentError("semantic: MOD requires INT64 operands");
  }
  const int64_t dividend = args[0].int64_value();
  const int64_t divisor = args[1].int64_value();
  if (divisor == 0) {
    return absl::InvalidArgumentError("semantic: division by zero: MOD");
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

absl::StatusOr<Value> Pow(const std::vector<Value>& args) {
  if (args.size() != 2) {
    return absl::InvalidArgumentError(
        "semantic: POW expects exactly two arguments");
  }
  if (args[0].is_null() || args[1].is_null()) return Value::NullDouble();
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
