#include <cmath>
#include <cstdint>
#include <string>
#include <utility>

#include "absl/status/statusor.h"
#include "absl/strings/ascii.h"
#include "absl/strings/match.h"
#include "absl/strings/numbers.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/string_view.h"
#include "backend/engine/semantic/error.h"
#include "backend/engine/semantic/eval_expr_internal.h"
#include "backend/engine/semantic/functions/datetime_funcs_internal.h"
#include "backend/engine/semantic/value.h"
#include "googlesql/public/functions/date_time_util.h"
#include "googlesql/public/numeric_value.h"
#include "googlesql/public/type.h"
#include "googlesql/resolved_ast/resolved_ast.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace semantic {
namespace eval_expr_internal {

namespace {

using functions::datetime_internal::kFormatOpts;

}  // namespace

absl::StatusOr<Value> EvalResolvedCast(const ::googlesql::ResolvedCast& cast,
                                       Value inner,
                                       const ::googlesql::Type* source) {
  const ::googlesql::Type* target = cast.type();
  if (target == nullptr) {
    return absl::InvalidArgumentError("semantic: ResolvedCast has null type");
  }
  if (source != nullptr && source->Equals(target)) {
    return inner;
  }
  if (inner.is_null()) return NullOfType(target);

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
  if (target->kind() == ::googlesql::TYPE_NUMERIC) {
    if (inner.type_kind() == ::googlesql::TYPE_NUMERIC) return inner;
    if (inner.type_kind() == ::googlesql::TYPE_INT64) {
      auto n = ::googlesql::NumericValue::FromString(
          absl::StrCat(inner.int64_value()));
      if (!n.ok()) {
        return MakeSemanticError(SemanticErrorReason::kInvalidArgument,
                                 n.status().message());
      }
      return Value::Numeric(*n);
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
      return Value::Numeric(*n);
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
      return Value::Numeric(*n);
    }
    if (inner.type_kind() == ::googlesql::TYPE_STRING) {
      auto n = ::googlesql::NumericValue::FromString(inner.string_value());
      if (!n.ok()) {
        return MakeSemanticError(SemanticErrorReason::kInvalidArgument,
                                 n.status().message());
      }
      return Value::Numeric(*n);
    }
  }
  if (cast.return_null_on_error()) {
    return NullOfType(target);
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
