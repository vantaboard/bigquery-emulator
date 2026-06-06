#include "backend/engine/semantic/functions/operator_funcs.h"

#include <cmath>
#include <cstdint>
#include <string>
#include <vector>

#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/string_view.h"
#include "backend/engine/semantic/error.h"
#include "backend/engine/semantic/functions/datetime_funcs.h"
#include "backend/engine/semantic/value.h"
#include "googlesql/public/functions/datetime.pb.h"
#include "googlesql/public/interval_value.h"
#include "googlesql/public/type.h"
#include "googlesql/public/type.pb.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace semantic {
namespace functions {

namespace {

using ::googlesql::IntervalValue;
using ::googlesql::functions::DateTimestampPart;

bool SqlLikeMatch(absl::string_view value, absl::string_view pattern) {
  size_t vi = 0;
  size_t pi = 0;
  int64_t star_vi = -1;
  int64_t star_pi = -1;
  while (vi < value.size()) {
    if (pi < pattern.size() &&
        (pattern[pi] == value[vi] || pattern[pi] == '_')) {
      if (pattern[pi] == '_') {
        ++vi;
        ++pi;
        continue;
      }
      ++vi;
      ++pi;
      continue;
    }
    if (pi < pattern.size() && pattern[pi] == '%') {
      star_vi = static_cast<int64_t>(vi);
      star_pi = static_cast<int64_t>(pi);
      ++pi;
      continue;
    }
    if (star_pi >= 0) {
      pi = static_cast<size_t>(star_pi + 1);
      vi = static_cast<size_t>(star_vi + 1);
      star_vi = static_cast<int64_t>(vi);
      continue;
    }
    return false;
  }
  while (pi < pattern.size() && pattern[pi] == '%') {
    ++pi;
  }
  return pi == pattern.size();
}

absl::StatusOr<Value> BoolOrNull(bool b) {
  return Value::Bool(b);
}

absl::StatusOr<int64_t> RequireInt64(const Value& v, absl::string_view op) {
  if (v.type_kind() != ::googlesql::TYPE_INT64) {
    return absl::InvalidArgumentError(
        absl::StrCat("semantic: ", op, " requires INT64 operands"));
  }
  return v.int64_value();
}

absl::StatusOr<DateTimestampPart> PartFromArg(const Value& v) {
  if (v.type_kind() == ::googlesql::TYPE_INT64) {
    const int part_int = static_cast<int>(v.int64_value());
    if (::googlesql::functions::DateTimestampPart_IsValid(part_int)) {
      return static_cast<DateTimestampPart>(part_int);
    }
    return absl::InvalidArgumentError(absl::StrCat(
        "semantic: invalid DateTimestampPart enum value ", v.int64_value()));
  }
  if (v.type_kind() == ::googlesql::TYPE_STRING) {
    DateTimestampPart part = DateTimestampPart::YEAR;
    if (!::googlesql::functions::DateTimestampPart_Parse(v.string_value(),
                                                         &part)) {
      return absl::InvalidArgumentError(absl::StrCat(
          "semantic: unknown interval part '", v.string_value(), "'"));
    }
    return part;
  }
  if (v.type_kind() == ::googlesql::TYPE_ENUM) {
    const int part_int = v.enum_value();
    if (!::googlesql::functions::DateTimestampPart_IsValid(part_int)) {
      return absl::InvalidArgumentError(absl::StrCat(
          "semantic: invalid DateTimestampPart enum value ", part_int));
    }
    return static_cast<DateTimestampPart>(part_int);
  }
  return absl::InvalidArgumentError(absl::StrCat(
      "semantic: interval part must be INT64 enum or STRING name; got ",
      v.type()->DebugString()));
}

}  // namespace

absl::StatusOr<Value> DispatchLike(absl::string_view name,
                                   const std::vector<Value>& args) {
  if (args.size() != 2) {
    return absl::InvalidArgumentError(
        "semantic: LIKE expects exactly two arguments");
  }
  if (args[0].is_null() || args[1].is_null()) {
    return Value::NullBool();
  }
  if (args[0].type_kind() != ::googlesql::TYPE_STRING ||
      args[1].type_kind() != ::googlesql::TYPE_STRING) {
    return absl::InvalidArgumentError(
        "semantic: LIKE requires STRING arguments");
  }
  bool matched = SqlLikeMatch(args[0].string_value(), args[1].string_value());
  if (name == "$not_like") {
    matched = !matched;
  }
  return Value::Bool(matched);
}

absl::StatusOr<Value> DispatchBetween(absl::string_view name,
                                      const std::vector<Value>& args) {
  if (args.size() != 3) {
    return absl::InvalidArgumentError(
        "semantic: BETWEEN expects exactly three arguments");
  }
  if (args[0].is_null() || args[1].is_null() || args[2].is_null()) {
    return Value::NullBool();
  }
  Value value = args[0];
  Value low = args[1];
  Value high = args[2];
  if (!value.type()->Equals(low.type()) || !value.type()->Equals(high.type())) {
    // CSV-loaded DATE columns often surface as STRING in the semantic
    // executor while DATE('...') bounds are TYPE_DATE.
    if (value.type_kind() == ::googlesql::TYPE_STRING &&
        low.type_kind() == ::googlesql::TYPE_DATE &&
        high.type_kind() == ::googlesql::TYPE_DATE) {
      Value parsed;
      if (TryParseIsoDateString(value, &parsed)) {
        value = parsed;
      }
    }
    if (!value.type()->Equals(low.type()) ||
        !value.type()->Equals(high.type())) {
      return absl::InvalidArgumentError(
          "semantic: BETWEEN operands have mismatched types");
    }
  }
  const Value& cmp_value = value;
  const Value& cmp_low = low;
  const Value& cmp_high = high;
  bool in_range = (cmp_low.LessThan(cmp_value) || cmp_low.Equals(cmp_value)) &&
                  (cmp_value.LessThan(cmp_high) || cmp_value.Equals(cmp_high));
  if (name == "$not_between") {
    in_range = !in_range;
  }
  return Value::Bool(in_range);
}

absl::StatusOr<Value> DispatchIn(absl::string_view name,
                                 const std::vector<Value>& args) {
  if (args.size() < 2) {
    return absl::InvalidArgumentError(
        "semantic: IN expects at least two arguments");
  }
  if (args[0].is_null()) {
    return Value::NullBool();
  }
  const Value& lhs = args[0];
  bool saw_null_rhs = false;
  for (size_t i = 1; i < args.size(); ++i) {
    if (args[i].is_null()) {
      saw_null_rhs = true;
      continue;
    }
    if (lhs.Equals(args[i])) {
      bool found = true;
      if (name == "$not_in") {
        found = false;
      }
      return Value::Bool(found);
    }
  }
  if (saw_null_rhs) {
    return Value::NullBool();
  }
  bool found = false;
  if (name == "$not_in") {
    found = true;
  }
  return Value::Bool(found);
}

absl::StatusOr<Value> DispatchIsTrue(absl::string_view name,
                                     const std::vector<Value>& args) {
  if (args.size() != 1) {
    return absl::InvalidArgumentError(
        "semantic: IS TRUE expects exactly one argument");
  }
  if (args[0].type_kind() != ::googlesql::TYPE_BOOL) {
    return absl::InvalidArgumentError(
        "semantic: IS TRUE requires BOOL argument");
  }
  if (args[0].is_null()) {
    if (name == "$is_not_true") {
      return Value::Bool(true);
    }
    return Value::NullBool();
  }
  const bool b = args[0].bool_value();
  if (name == "$is_true") {
    return Value::Bool(b);
  }
  return Value::Bool(!b);
}

absl::StatusOr<Value> DispatchIsFalse(absl::string_view name,
                                      const std::vector<Value>& args) {
  if (args.size() != 1) {
    return absl::InvalidArgumentError(
        "semantic: IS FALSE expects exactly one argument");
  }
  if (args[0].type_kind() != ::googlesql::TYPE_BOOL) {
    return absl::InvalidArgumentError(
        "semantic: IS FALSE requires BOOL argument");
  }
  if (args[0].is_null()) {
    if (name == "$is_not_false") {
      return Value::Bool(true);
    }
    return Value::NullBool();
  }
  const bool b = args[0].bool_value();
  if (name == "$is_false") {
    return Value::Bool(!b);
  }
  return Value::Bool(b);
}

absl::StatusOr<Value> DispatchIsDistinctFrom(absl::string_view name,
                                             const std::vector<Value>& args) {
  if (args.size() != 2) {
    return absl::InvalidArgumentError(
        "semantic: IS [NOT] DISTINCT FROM expects exactly two arguments");
  }
  bool distinct = false;
  if (args[0].is_null() && args[1].is_null()) {
    distinct = false;
  } else if (args[0].is_null() || args[1].is_null()) {
    distinct = true;
  } else {
    distinct = !args[0].Equals(args[1]);
  }
  if (name == "$is_not_distinct_from") {
    distinct = !distinct;
  }
  return Value::Bool(distinct);
}

absl::StatusOr<Value> DispatchBitwise(absl::string_view name,
                                      const std::vector<Value>& args) {
  if (name == "$bitwise_not") {
    if (args.size() != 1) {
      return absl::InvalidArgumentError(
          "semantic: bitwise NOT expects exactly one argument");
    }
    if (args[0].is_null()) {
      return Value::NullInt64();
    }
    auto v = RequireInt64(args[0], name);
    if (!v.ok()) return v.status();
    return Value::Int64(~(*v));
  }
  if (args.size() != 2) {
    return absl::InvalidArgumentError(
        absl::StrCat("semantic: ", name, " expects exactly two arguments"));
  }
  if (args[0].is_null() || args[1].is_null()) {
    return Value::NullInt64();
  }
  auto a = RequireInt64(args[0], name);
  if (!a.ok()) return a.status();
  auto b = RequireInt64(args[1], name);
  if (!b.ok()) return b.status();
  if (name == "$bitwise_and") {
    return Value::Int64(*a & *b);
  }
  if (name == "$bitwise_or") {
    return Value::Int64(*a | *b);
  }
  if (name == "$bitwise_xor") {
    return Value::Int64(*a ^ *b);
  }
  if (name == "$bitwise_left_shift") {
    if (*b < 0 || *b >= 64) {
      return MakeSemanticError(SemanticErrorReason::kInvalidArgument,
                               "semantic: shift count out of range");
    }
    return Value::Int64(static_cast<uint64_t>(*a) << *b);
  }
  if (name == "$bitwise_right_shift") {
    if (*b < 0 || *b >= 64) {
      return MakeSemanticError(SemanticErrorReason::kInvalidArgument,
                               "semantic: shift count out of range");
    }
    return Value::Int64(static_cast<int64_t>(static_cast<uint64_t>(*a) >> *b));
  }
  return absl::InvalidArgumentError(
      absl::StrCat("semantic: unknown bitwise operator ", name));
}

absl::StatusOr<Value> DispatchInterval(const std::vector<Value>& args,
                                       const ::googlesql::Type* return_type) {
  (void)return_type;
  if (args.size() != 2) {
    return absl::InvalidArgumentError(
        "semantic: $interval expects exactly two arguments");
  }
  if (args[0].is_null() || args[1].is_null()) {
    return Value::NullInterval();
  }
  if (args[0].type_kind() == ::googlesql::TYPE_INTERVAL) {
    return args[0];
  }
  auto part0 = PartFromArg(args[0]);
  auto part1 = PartFromArg(args[1]);
  int64_t amount = 0;
  DateTimestampPart part = DateTimestampPart::DAY;
  if (part0.ok() && !part1.ok()) {
    auto amount_or = RequireInt64(args[1], "$interval");
    if (!amount_or.ok()) return amount_or.status();
    part = *part0;
    amount = *amount_or;
  } else if (!part0.ok() && part1.ok()) {
    auto amount_or = RequireInt64(args[0], "$interval");
    if (!amount_or.ok()) return amount_or.status();
    part = *part1;
    amount = *amount_or;
  } else if (part0.ok() && part1.ok()) {
    if (args[0].type_kind() == ::googlesql::TYPE_INT64 &&
        (args[1].type_kind() == ::googlesql::TYPE_ENUM ||
         args[1].type_kind() == ::googlesql::TYPE_STRING)) {
      amount = args[0].int64_value();
      part = *part1;
    } else if (args[1].type_kind() == ::googlesql::TYPE_INT64 &&
               (args[0].type_kind() == ::googlesql::TYPE_ENUM ||
                args[0].type_kind() == ::googlesql::TYPE_STRING)) {
      amount = args[1].int64_value();
      part = *part0;
    } else {
      return absl::InvalidArgumentError(
          "semantic: $interval arguments are ambiguous (two part "
          "specifiers)");
    }
  } else {
    return absl::InvalidArgumentError(
        absl::StrCat("semantic: $interval could not interpret arguments (",
                     args[0].type()->DebugString(),
                     ", ",
                     args[1].type()->DebugString(),
                     ")"));
  }
  auto iv = IntervalValue::FromInteger(amount, part, /*allow_nanos=*/true);
  if (!iv.ok()) return iv.status();
  return Value::Interval(*iv);
}

absl::StatusOr<Value> JustifyDays(const std::vector<Value>& args) {
  if (args.size() != 1) {
    return absl::InvalidArgumentError(
        "semantic: JUSTIFY_DAYS expects exactly one argument");
  }
  if (args[0].is_null()) {
    return Value::NullInterval();
  }
  if (args[0].type_kind() != ::googlesql::TYPE_INTERVAL) {
    return absl::InvalidArgumentError(
        "semantic: JUSTIFY_DAYS requires INTERVAL argument");
  }
  return Value::Interval(*::googlesql::JustifyDays(args[0].interval_value()));
}

absl::StatusOr<Value> JustifyHours(const std::vector<Value>& args) {
  if (args.size() != 1) {
    return absl::InvalidArgumentError(
        "semantic: JUSTIFY_HOURS expects exactly one argument");
  }
  if (args[0].is_null()) {
    return Value::NullInterval();
  }
  if (args[0].type_kind() != ::googlesql::TYPE_INTERVAL) {
    return absl::InvalidArgumentError(
        "semantic: JUSTIFY_HOURS requires INTERVAL argument");
  }
  return Value::Interval(*::googlesql::JustifyHours(args[0].interval_value()));
}

namespace {

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
  switch (args[0].type_kind()) {
    case ::googlesql::TYPE_INT64:
      return Value::Int64(static_cast<int64_t>(RoundHalfAwayFromZero(
          static_cast<double>(args[0].int64_value()), precision)));
    case ::googlesql::TYPE_FLOAT:
      return Value::Double(RoundHalfAwayFromZero(
          static_cast<double>(args[0].float_value()), precision));
    case ::googlesql::TYPE_DOUBLE:
      return Value::Double(
          RoundHalfAwayFromZero(args[0].double_value(), precision));
    case ::googlesql::TYPE_NUMERIC:
    case ::googlesql::TYPE_BIGNUMERIC:
      return absl::InvalidArgumentError(
          "semantic: ROUND on NUMERIC types is not yet implemented");
    default:
      return absl::InvalidArgumentError(
          "semantic: ROUND requires a numeric argument");
  }
}

}  // namespace functions
}  // namespace semantic
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
