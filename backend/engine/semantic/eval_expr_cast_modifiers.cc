
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

absl::StatusOr<Value> ApplyNumericPrecisionScale(
    const ::googlesql::NumericValue& in,
    const ::googlesql::NumericTypeParametersProto& params) {
  ::googlesql::NumericValue out = in;
  if (out.HasFractionalPart()) {
    auto rounded = out.Round(params.scale());
    if (!rounded.ok()) return rounded.status();
    out = *rounded;
  }
  const int64_t precision = params.precision();
  const int64_t scale = params.scale();
  std::string upper_bound_str;
  if (scale == 0) {
    upper_bound_str.assign(static_cast<size_t>(precision), '9');
  } else {
    upper_bound_str.assign(static_cast<size_t>(precision + 1), '9');
    upper_bound_str[static_cast<size_t>(precision - scale)] = '.';
  }
  auto upper = ::googlesql::NumericValue::FromString(upper_bound_str);
  if (!upper.ok()) return upper.status();
  auto lower =
      ::googlesql::NumericValue::FromString(absl::StrCat("-", upper_bound_str));
  if (!lower.ok()) return lower.status();
  if (out < *lower || out > *upper) {
    return absl::OutOfRangeError(
        absl::StrCat("semantic: NUMERIC(",
                     precision,
                     ", ",
                     scale,
                     ") value out of range after CAST"));
  }
  return Value::Numeric(out);
}

absl::Status ApplyStringMaxLength(const Value& value, int64_t max_length) {
  int64_t length = 0;
  absl::Status length_error;
  if (!::googlesql::functions::LengthUtf8(
          value.string_value(), &length, &length_error)) {
    return length_error;
  }
  if (max_length < length) {
    return absl::OutOfRangeError(absl::StrCat("semantic: STRING(",
                                              max_length,
                                              ") has maximum length ",
                                              max_length,
                                              " but got length ",
                                              length));
  }
  return absl::OkStatus();
}

absl::StatusOr<Value> ApplyBignumericPrecisionScale(
    const ::googlesql::BigNumericValue& in,
    const ::googlesql::NumericTypeParametersProto& params,
    bool return_null_on_error) {
  auto out = in;
  if (out.HasFractionalPart()) {
    auto rounded = out.Round(params.scale());
    if (!rounded.ok()) {
      if (return_null_on_error) return Value::NullBigNumeric();
      return rounded.status();
    }
    out = *rounded;
  }
  return Value::BigNumeric(out);
}

absl::StatusOr<Value> ApplyNumericTypeModifiers(
    Value value,
    const ::googlesql::NumericTypeParametersProto& params,
    bool return_null_on_error) {
  if (value.type_kind() == ::googlesql::TYPE_NUMERIC) {
    auto adjusted = ApplyNumericPrecisionScale(value.numeric_value(), params);
    if (!adjusted.ok()) {
      if (return_null_on_error) return Value::NullNumeric();
      return adjusted.status();
    }
    return *adjusted;
  }
  if (value.type_kind() == ::googlesql::TYPE_BIGNUMERIC) {
    return ApplyBignumericPrecisionScale(
        value.bignumeric_value(), params, return_null_on_error);
  }
  return value;
}

}  // namespace

absl::StatusOr<Value> ApplyCastTypeModifiers(
    Value value,
    const ::googlesql::TypeModifiers& modifiers,
    bool return_null_on_error) {
  const ::googlesql::TypeParameters& params = modifiers.type_parameters();
  if (params.IsEmpty() || value.is_null()) {
    return value;
  }
  if (params.IsStringTypeParameters()) {
    if (value.type_kind() != ::googlesql::TYPE_STRING) {
      return value;
    }
    const auto& string_params = params.string_type_parameters();
    const int64_t max_length = string_params.has_is_max_length()
                                   ? std::numeric_limits<int64_t>::max()
                                   : string_params.max_length();
    if (absl::Status s = ApplyStringMaxLength(value, max_length); !s.ok()) {
      if (return_null_on_error) return NullOfType(value.type());
      return s;
    }
    return value;
  }
  if (params.IsNumericTypeParameters()) {
    return ApplyNumericTypeModifiers(std::move(value),
                                     params.numeric_type_parameters(),
                                     return_null_on_error);
  }
  return MakeSemanticError(
      SemanticErrorReason::kNotImplemented,
      "semantic: CAST type_modifiers for this target type are deferred");
}

}  // namespace eval_expr_internal
}  // namespace semantic
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
