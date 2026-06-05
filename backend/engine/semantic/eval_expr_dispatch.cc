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

}  // namespace eval_expr_internal
}  // namespace semantic
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
