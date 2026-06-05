#include <cctype>
#include <cstring>
#include <memory>
#include <string>
#include <vector>

#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "absl/strings/ascii.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/string_view.h"
#include "backend/engine/semantic/error.h"
#include "backend/engine/semantic/eval_context.h"
#include "backend/engine/semantic/functions/string_extra_internal.h"
#include "backend/engine/semantic/functions/string_funcs.h"
#include "backend/engine/semantic/value.h"
#include "googlesql/public/functions/hash.h"
#include "googlesql/public/functions/normalize_mode.pb.h"
#include "googlesql/public/functions/numeric.h"
#include "googlesql/public/functions/regexp.h"
#include "googlesql/public/functions/string.h"
#include "googlesql/public/functions/string_format.h"
#include "googlesql/public/numeric_value.h"
#include "googlesql/public/options.pb.h"
#include "googlesql/public/type.h"
#include "googlesql/public/type.pb.h"
#include "googlesql/public/value.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace semantic {
namespace functions {

using ::googlesql::BigNumericValue;
using ::googlesql::NumericValue;
using ::googlesql::ProductMode;
using string_extra_internal::AnyNull;
using string_extra_internal::AsStringOrBytes;
using string_extra_internal::MakeRegExpForValue;
using string_extra_internal::StringOrBytesFromView;

absl::StatusOr<Value> InitcapFunc(const std::vector<Value>& args) {
  if (args.empty() || args.size() > 2) {
    return absl::InvalidArgumentError(
        "semantic: INITCAP expects one or two arguments");
  }
  if (AnyNull(args)) return Value::NullString();
  if (args[0].type_kind() != ::googlesql::TYPE_STRING) {
    return absl::InvalidArgumentError("semantic: INITCAP value must be STRING");
  }
  absl::Status error;
  std::string out;
  if (args.size() == 1) {
    if (!::googlesql::functions::InitialCapitalizeDefault(
            args[0].string_value(), &out, &error)) {
      return error;
    }
  } else {
    absl::string_view delimiters = args[1].string_value();
    if (!::googlesql::functions::InitialCapitalize(
            args[0].string_value(), delimiters, &out, &error)) {
      return error;
    }
  }
  return Value::String(std::move(out));
}

absl::StatusOr<Value> NormalizeFunc(const std::vector<Value>& args) {
  if (args.empty() || args.size() > 2) {
    return absl::InvalidArgumentError(
        "semantic: NORMALIZE expects one or two arguments");
  }
  if (args[0].is_null()) return Value::NullString();
  ::googlesql::functions::NormalizeMode mode =
      ::googlesql::functions::NormalizeMode::NFC;
  if (args.size() == 2 && !args[1].is_null()) {
    if (args[1].type_kind() == ::googlesql::TYPE_ENUM) {
      mode = static_cast<::googlesql::functions::NormalizeMode>(
          args[1].enum_value());
    } else if (args[1].type_kind() == ::googlesql::TYPE_STRING) {
      if (!::googlesql::functions::NormalizeMode_Parse(args[1].string_value(),
                                                       &mode)) {
        return absl::InvalidArgumentError("semantic: invalid NORMALIZE mode");
      }
    }
  }
  absl::Status error;
  std::string out;
  if (!::googlesql::functions::Normalize(args[0].string_value(),
                                         mode,
                                         /*is_casefold=*/false,
                                         &out,
                                         &error)) {
    return error;
  }
  return Value::String(std::move(out));
}

absl::StatusOr<Value> NormalizeAndCasefoldFunc(const std::vector<Value>& args) {
  if (args.empty() || args.size() > 2) {
    return absl::InvalidArgumentError(
        "semantic: NORMALIZE_AND_CASEFOLD expects one or two arguments");
  }
  if (args[0].is_null()) return Value::NullString();
  ::googlesql::functions::NormalizeMode mode =
      ::googlesql::functions::NormalizeMode::NFC;
  if (args.size() == 2 && !args[1].is_null()) {
    if (args[1].type_kind() == ::googlesql::TYPE_ENUM) {
      mode = static_cast<::googlesql::functions::NormalizeMode>(
          args[1].enum_value());
    } else if (args[1].type_kind() == ::googlesql::TYPE_STRING) {
      if (!::googlesql::functions::NormalizeMode_Parse(args[1].string_value(),
                                                       &mode)) {
        return absl::InvalidArgumentError(
            "semantic: invalid NORMALIZE_AND_CASEFOLD mode");
      }
    }
  }
  absl::Status error;
  std::string out;
  if (!::googlesql::functions::Normalize(args[0].string_value(),
                                         mode,
                                         /*is_casefold=*/true,
                                         &out,
                                         &error)) {
    return error;
  }
  return Value::String(std::move(out));
}

absl::StatusOr<Value> SafeConvertBytesToString(const std::vector<Value>& args) {
  if (args.size() != 1) {
    return absl::InvalidArgumentError(
        "semantic: SAFE_CONVERT_BYTES_TO_STRING expects one argument");
  }
  if (args[0].is_null()) return Value::NullString();
  absl::Status error;
  std::string out;
  if (!::googlesql::functions::SafeConvertBytes(
          args[0].bytes_value(), &out, &error)) {
    return error;
  }
  return Value::String(std::move(out));
}

absl::StatusOr<Value> CodePointsToString(const std::vector<Value>& args) {
  if (args.size() != 1) {
    return absl::InvalidArgumentError(
        "semantic: CODE_POINTS_TO_STRING expects one argument");
  }
  if (args[0].is_null()) return Value::NullString();
  if (!args[0].type()->IsArray()) {
    return absl::InvalidArgumentError(
        "semantic: CODE_POINTS_TO_STRING expects ARRAY<INT64>");
  }
  std::vector<int64_t> cps;
  for (int i = 0; i < args[0].num_elements(); ++i) {
    if (args[0].element(i).is_null()) return Value::NullString();
    const int64_t cp = args[0].element(i).int64_value();
    if (cp == 0) continue;
    cps.push_back(cp);
  }
  absl::Status error;
  std::string out;
  if (!::googlesql::functions::CodePointsToString(cps, &out, &error)) {
    return error;
  }
  return Value::String(std::move(out));
}

absl::StatusOr<Value> CodePointsToBytes(const std::vector<Value>& args) {
  if (args.size() != 1) {
    return absl::InvalidArgumentError(
        "semantic: CODE_POINTS_TO_BYTES expects one argument");
  }
  if (args[0].is_null()) return Value::NullBytes();
  std::vector<int64_t> cps;
  for (int i = 0; i < args[0].num_elements(); ++i) {
    if (args[0].element(i).is_null()) return Value::NullBytes();
    cps.push_back(args[0].element(i).int64_value());
  }
  absl::Status error;
  std::string out;
  if (!::googlesql::functions::CodePointsToBytes(cps, &out, &error)) {
    return error;
  }
  return Value::Bytes(std::move(out));
}

absl::StatusOr<Value> ToCodePoints(const std::vector<Value>& args,
                                   const ::googlesql::Type* return_type) {
  if (args.size() != 1) {
    return absl::InvalidArgumentError(
        "semantic: TO_CODE_POINTS expects one argument");
  }
  if (args[0].is_null()) {
    if (return_type != nullptr && return_type->IsArray()) {
      return Value::Array(return_type->AsArray(), {});
    }
    return Value::Null(return_type);
  }
  if (args[0].type_kind() == ::googlesql::TYPE_STRING &&
      args[0].string_value().empty()) {
    return Value::Array(return_type->AsArray(), {});
  }
  absl::Status error;
  std::vector<int64_t> cps;
  if (args[0].type_kind() == ::googlesql::TYPE_BYTES) {
    if (!::googlesql::functions::BytesToCodePoints(
            args[0].bytes_value(), &cps, &error)) {
      return error;
    }
  } else if (!::googlesql::functions::StringToCodePoints(
                 args[0].string_value(), &cps, &error)) {
    return error;
  }
  std::vector<Value> elements;
  elements.reserve(cps.size());
  for (int64_t cp : cps) {
    elements.push_back(Value::Int64(cp));
  }
  return Value::Array(return_type->AsArray(), elements);
}

absl::StatusOr<Value> LeastGreatest(absl::string_view name,
                                    const std::vector<Value>& args,
                                    const ::googlesql::Type* return_type) {
  if (args.empty()) {
    return absl::InvalidArgumentError(
        absl::StrCat("semantic: ", name, " expects arguments"));
  }
  Value best = args[0];
  for (size_t i = 1; i < args.size(); ++i) {
    if (args[i].is_null()) return Value::Null(return_type);
    if (name == "least") {
      if (args[i].LessThan(best)) best = args[i];
    } else if (best.LessThan(args[i])) {
      best = args[i];
    }
  }
  return best;
}

absl::StatusOr<Value> Least(const std::vector<Value>& args,
                            const ::googlesql::Type* return_type) {
  return LeastGreatest("least", args, return_type);
}

absl::StatusOr<Value> Greatest(const std::vector<Value>& args,
                               const ::googlesql::Type* return_type) {
  return LeastGreatest("greatest", args, return_type);
}

absl::StatusOr<Value> ParseNumericFunc(const std::vector<Value>& args) {
  if (args.size() != 1) {
    return absl::InvalidArgumentError(
        "semantic: PARSE_NUMERIC expects one argument");
  }
  if (args[0].is_null()) return Value::NullNumeric();
  NumericValue out;
  absl::Status error;
  if (!::googlesql::functions::ParseNumeric(
          args[0].string_value(), &out, &error)) {
    return error;
  }
  return Value::Numeric(out);
}

namespace {

using ::googlesql::BigNumericValue;
using ::googlesql::NumericValue;

std::optional<std::string> ExpandScientificNotation(absl::string_view input) {
  const size_t epos = input.find('e');
  if (epos == absl::string_view::npos) return std::nullopt;
  std::string mantissa(input.substr(0, epos));
  int64_t exponent = 0;
  if (!absl::SimpleAtoi(input.substr(epos + 1), &exponent)) {
    return std::nullopt;
  }
  const size_t dot = mantissa.find('.');
  if (dot == std::string::npos) {
    if (exponent >= 0) {
      mantissa.append(static_cast<size_t>(exponent), '0');
    }
    return mantissa;
  }
  std::string digits = mantissa.substr(0, dot);
  digits.append(mantissa.substr(dot + 1));
  const int64_t decimal_places =
      static_cast<int64_t>(mantissa.size() - dot - 1);
  exponent -= decimal_places;
  if (exponent >= 0) {
    digits.append(static_cast<size_t>(exponent), '0');
    return digits;
  }
  const int64_t shift = -exponent;
  if (shift >= static_cast<int64_t>(digits.size())) {
    digits.insert(0, static_cast<size_t>(shift - digits.size() + 1), '0');
    digits.insert(1, ".");
    return digits;
  }
  digits.insert(static_cast<size_t>(digits.size() - shift), ".");
  return digits;
}

void MaybeSetBignumericRenderOverride(const BigNumericValue& parsed,
                                      absl::string_view expanded,
                                      const EvalContext* ctx) {
  if (ctx == nullptr) return;
  const std::string rendered = parsed.ToString();
  if (rendered.size() < expanded.size() &&
      absl::StartsWith(expanded, rendered)) {
    ctx->bignumeric_render_override = std::string(expanded);
  }
}

}  // namespace

absl::StatusOr<Value> ParseBignumeric(const std::vector<Value>& args,
                                      const EvalContext* ctx) {
  if (args.size() != 1) {
    return absl::InvalidArgumentError(
        "semantic: PARSE_BIGNUMERIC expects one argument");
  }
  if (args[0].is_null()) return Value::NullBigNumeric();
  std::string input = args[0].string_value();
  for (char& c : input) {
    if (c == 'E') c = 'e';
  }
  BigNumericValue out;
  absl::Status error;
  if (::googlesql::functions::ParseBigNumeric(input, &out, &error)) {
    return Value::BigNumeric(out);
  }
  if (input.find('e') != std::string::npos) {
    const std::optional<std::string> expanded = ExpandScientificNotation(input);
    if (expanded.has_value()) {
      for (const std::string& candidate :
           {*expanded, absl::StrCat(*expanded, ".0")}) {
        if (auto parsed = BigNumericValue::FromStringWithRounding(
                candidate, 76, /*round_half_even=*/false);
            parsed.ok()) {
          MaybeSetBignumericRenderOverride(*parsed, *expanded, ctx);
          return Value::BigNumeric(*parsed);
        }
        if (::googlesql::functions::ParseBigNumeric(candidate, &out, &error)) {
          MaybeSetBignumericRenderOverride(out, *expanded, ctx);
          return Value::BigNumeric(out);
        }
      }
      const size_t epos = input.find('e');
      std::string mantissa(input.substr(0, epos));
      int64_t exponent = 0;
      if (absl::SimpleAtoi(input.substr(epos + 1), &exponent)) {
        const size_t dot = mantissa.find('.');
        int64_t digits_before_dot = static_cast<int64_t>(
            dot == std::string::npos ? mantissa.size() : dot);
        int64_t norm_exp =
            exponent + digits_before_dot - (dot == std::string::npos ? 0 : 1);
        std::string norm_mantissa =
            dot == std::string::npos ? mantissa
                                     : absl::StrCat(mantissa.substr(0, dot),
                                                    mantissa.substr(dot + 1));
        while (norm_mantissa.size() > 1 && norm_mantissa[0] == '0') {
          norm_mantissa.erase(0, 1);
          --norm_exp;
        }
        const std::string normalized =
            absl::StrCat(norm_mantissa, "e", norm_exp);
        if (auto parsed = BigNumericValue::FromStringWithRounding(
                normalized, 76, /*round_half_even=*/false);
            parsed.ok()) {
          MaybeSetBignumericRenderOverride(*parsed, *expanded, ctx);
          return Value::BigNumeric(*parsed);
        }
        if (::googlesql::functions::ParseBigNumeric(normalized, &out, &error)) {
          MaybeSetBignumericRenderOverride(out, *expanded, ctx);
          return Value::BigNumeric(out);
        }
      }
    }
    const size_t epos = input.find('e');
    std::string mantissa_str(input.substr(0, epos));
    int64_t exponent = 0;
    if (!absl::SimpleAtoi(input.substr(epos + 1), &exponent)) {
      return error;
    }
    const size_t dot = mantissa_str.find('.');
    if (dot != std::string::npos) {
      exponent -= static_cast<int64_t>(mantissa_str.size() - dot - 1);
    }
    if (auto mantissa_or = BigNumericValue::FromStringWithRounding(
            mantissa_str, 76, /*round_half_even=*/false);
        mantissa_or.ok()) {
      BigNumericValue scaled = *mantissa_or;
      if (exponent > 0) {
        for (int64_t i = 0; i < exponent; ++i) {
          auto next = scaled.Multiply(BigNumericValue(10));
          if (!next.ok()) return next.status();
          scaled = *next;
        }
      } else if (exponent < 0) {
        for (int64_t i = 0; i < -exponent; ++i) {
          auto next = scaled.Divide(BigNumericValue(10));
          if (!next.ok()) return next.status();
          scaled = *next;
        }
      }
      if (expanded.has_value()) {
        MaybeSetBignumericRenderOverride(scaled, *expanded, ctx);
      }
      return Value::BigNumeric(scaled);
    }
  }
  if (auto rounded = BigNumericValue::FromStringWithRounding(
          input, 76, /*round_half_even=*/false);
      rounded.ok()) {
    return Value::BigNumeric(*rounded);
  }
  return error;
}

}  // namespace functions
}  // namespace semantic
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
