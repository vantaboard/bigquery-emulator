
#include "googlesql/public/numeric_value.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace semantic {
namespace eval_expr_internal {
namespace {

using ::googlesql::BigNumericValue;
using ::googlesql::NumericValue;

template <typename NV>
absl::StatusOr<Value> WrapNumericValue(absl::StatusOr<NV> result) {
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
  } else if constexpr (std::is_same_v<NV, NumericValue>) {
    return Value::Numeric(*result);
  }
  return absl::InternalError("semantic: WrapNumericValue: unsupported type");
}

absl::StatusOr<BigNumericValue> CoerceToBigNumeric(const Value& v) {
  switch (v.type_kind()) {
    case ::googlesql::TYPE_BIGNUMERIC:
      return v.bignumeric_value();
    case ::googlesql::TYPE_NUMERIC:
      return BigNumericValue(v.numeric_value());
    case ::googlesql::TYPE_INT64:
      return BigNumericValue(v.int64_value());
    default:
      return absl::InvalidArgumentError(absl::StrCat("semantic: cannot coerce ",
                                                     v.type()->DebugString(),
                                                     " to BIGNUMERIC"));
  }
}

absl::StatusOr<NumericValue> CoerceToNumeric(const Value& v) {
  switch (v.type_kind()) {
    case ::googlesql::TYPE_NUMERIC:
      return v.numeric_value();
    case ::googlesql::TYPE_INT64: {
      auto n = NumericValue::FromString(absl::StrCat(v.int64_value()));
      if (!n.ok()) return n.status();
      return *n;
    }
    default:
      return absl::InvalidArgumentError(absl::StrCat(
          "semantic: cannot coerce ", v.type()->DebugString(), " to NUMERIC"));
  }
}

bool EitherKind(const Value& a, const Value& b, ::googlesql::TypeKind kind) {
  return a.type_kind() == kind || b.type_kind() == kind;
}

}  // namespace

std::optional<absl::StatusOr<Value>> TryPromotedNumericAdd(const Value& a,
                                                           const Value& b) {
  if (EitherKind(a, b, ::googlesql::TYPE_BIGNUMERIC)) {
    auto lhs = CoerceToBigNumeric(a);
    if (!lhs.ok()) return lhs.status();
    auto rhs = CoerceToBigNumeric(b);
    if (!rhs.ok()) return rhs.status();
    return WrapNumericValue(lhs->Add(*rhs));
  }
  if (EitherKind(a, b, ::googlesql::TYPE_NUMERIC)) {
    auto lhs = CoerceToNumeric(a);
    if (!lhs.ok()) return lhs.status();
    auto rhs = CoerceToNumeric(b);
    if (!rhs.ok()) return rhs.status();
    return WrapNumericValue(lhs->Add(*rhs));
  }
  return std::nullopt;
}

std::optional<absl::StatusOr<Value>> TryPromotedNumericSub(const Value& a,
                                                           const Value& b) {
  if (EitherKind(a, b, ::googlesql::TYPE_BIGNUMERIC)) {
    auto lhs = CoerceToBigNumeric(a);
    if (!lhs.ok()) return lhs.status();
    auto rhs = CoerceToBigNumeric(b);
    if (!rhs.ok()) return rhs.status();
    return WrapNumericValue(lhs->Subtract(*rhs));
  }
  if (EitherKind(a, b, ::googlesql::TYPE_NUMERIC)) {
    auto lhs = CoerceToNumeric(a);
    if (!lhs.ok()) return lhs.status();
    auto rhs = CoerceToNumeric(b);
    if (!rhs.ok()) return rhs.status();
    return WrapNumericValue(lhs->Subtract(*rhs));
  }
  return std::nullopt;
}

std::optional<absl::StatusOr<Value>> TryPromotedNumericMul(const Value& a,
                                                           const Value& b) {
  if (EitherKind(a, b, ::googlesql::TYPE_BIGNUMERIC)) {
    auto lhs = CoerceToBigNumeric(a);
    if (!lhs.ok()) return lhs.status();
    auto rhs = CoerceToBigNumeric(b);
    if (!rhs.ok()) return rhs.status();
    return WrapNumericValue(lhs->Multiply(*rhs));
  }
  if (EitherKind(a, b, ::googlesql::TYPE_NUMERIC)) {
    auto lhs = CoerceToNumeric(a);
    if (!lhs.ok()) return lhs.status();
    auto rhs = CoerceToNumeric(b);
    if (!rhs.ok()) return rhs.status();
    return WrapNumericValue(lhs->Multiply(*rhs));
  }
  return std::nullopt;
}

std::optional<absl::StatusOr<Value>> TryPromotedNumericDiv(const Value& a,
                                                           const Value& b) {
  if (EitherKind(a, b, ::googlesql::TYPE_BIGNUMERIC)) {
    auto lhs = CoerceToBigNumeric(a);
    if (!lhs.ok()) return lhs.status();
    auto rhs = CoerceToBigNumeric(b);
    if (!rhs.ok()) return rhs.status();
    auto result = lhs->Divide(*rhs);
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
  if (EitherKind(a, b, ::googlesql::TYPE_NUMERIC)) {
    auto lhs = CoerceToNumeric(a);
    if (!lhs.ok()) return lhs.status();
    auto rhs = CoerceToNumeric(b);
    if (!rhs.ok()) return rhs.status();
    auto result = lhs->Divide(*rhs);
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
  return std::nullopt;
}

}  // namespace eval_expr_internal
}  // namespace semantic
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
