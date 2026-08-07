#include "backend/engine/duckdb/transpiler/transpiler_emit_datetime.h"

#include <cstdint>
#include <optional>
#include <string>

#include "absl/strings/str_cat.h"
#include "backend/engine/duckdb/transpiler/transpiler_internal.h"
#include "googlesql/public/functions/date_time_util.h"
#include "googlesql/public/interval_value.h"
#include "googlesql/public/value.h"
#include "googlesql/resolved_ast/resolved_ast.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace duckdb {
namespace transpiler {
namespace internal {

using ::googlesql::functions::DateTimestampPart;

namespace {

std::optional<int> TryIntervalPartEnum(const ::googlesql::ResolvedExpr* expr) {
  if (expr == nullptr || expr->node_kind() != ::googlesql::RESOLVED_LITERAL) {
    return std::nullopt;
  }
  const auto* lit = expr->GetAs<::googlesql::ResolvedLiteral>();
  if (lit == nullptr) return std::nullopt;
  const ::googlesql::Value& v = lit->value();
  if (v.type_kind() == ::googlesql::TYPE_ENUM) {
    const int part_int = v.enum_value();
    if (::googlesql::functions::DateTimestampPart_IsValid(part_int)) {
      return part_int;
    }
    return std::nullopt;
  }
  if (v.type_kind() == ::googlesql::TYPE_INT64) {
    const int part_int = static_cast<int>(v.int64_value());
    if (::googlesql::functions::DateTimestampPart_IsValid(part_int)) {
      return part_int;
    }
    return std::nullopt;
  }
  if (v.type_kind() == ::googlesql::TYPE_STRING) {
    DateTimestampPart part = DateTimestampPart::YEAR;
    if (::googlesql::functions::DateTimestampPart_Parse(v.string_value(),
                                                        &part)) {
      return static_cast<int>(part);
    }
  }
  return std::nullopt;
}

struct IntervalDecomposition {
  int part_enum = 0;
  int amount_arg_index = -1;
};

std::optional<IntervalDecomposition> TryDecomposeIntervalArgs(
    const ::googlesql::ResolvedFunctionCall* node) {
  if (node == nullptr || node->function() == nullptr ||
      ResolveFunctionName(node->function()) != "$interval" ||
      node->argument_list_size() != 2) {
    return std::nullopt;
  }
  const auto part0 = TryIntervalPartEnum(node->argument_list(0));
  const auto part1 = TryIntervalPartEnum(node->argument_list(1));
  if (part0.has_value() && !part1.has_value()) {
    return IntervalDecomposition{*part0, 1};
  }
  if (!part0.has_value() && part1.has_value()) {
    return IntervalDecomposition{*part1, 0};
  }
  return std::nullopt;
}

std::string IntervalUnitSql(int part_enum) {
  switch (static_cast<DateTimestampPart>(part_enum)) {
    case DateTimestampPart::MICROSECOND:
      return "MICROSECOND";
    case DateTimestampPart::MILLISECOND:
      return "MILLISECOND";
    case DateTimestampPart::SECOND:
      return "SECOND";
    case DateTimestampPart::MINUTE:
      return "MINUTE";
    case DateTimestampPart::HOUR:
      return "HOUR";
    case DateTimestampPart::DAY:
      return "DAY";
    case DateTimestampPart::WEEK:
      return "WEEK";
    case DateTimestampPart::MONTH:
      return "MONTH";
    case DateTimestampPart::QUARTER:
      return "QUARTER";
    case DateTimestampPart::YEAR:
      return "YEAR";
    default:
      return "DAY";
  }
}

std::string EmitIntervalProduct(absl::string_view amount_sql, int part_enum) {
  return absl::StrCat("(CAST(",
                      amount_sql,
                      " AS BIGINT) * INTERVAL 1 ",
                      IntervalUnitSql(part_enum),
                      ")");
}

std::optional<std::string> TryEmitExtractCall(
    const ::googlesql::ResolvedFunctionCall* node,
    const EmitExprFn& emit_expr) {
  if (node->argument_list_size() != 2) return std::nullopt;
  const auto part0 = TryIntervalPartEnum(node->argument_list(0));
  const auto part1 = TryIntervalPartEnum(node->argument_list(1));
  if (part0.has_value() && !part1.has_value()) {
    std::string value_sql = emit_expr(node->argument_list(1));
    if (value_sql.empty()) return std::nullopt;
    return absl::StrCat("bq_extract(", *part0, ", ", value_sql, ")");
  }
  if (!part0.has_value() && part1.has_value()) {
    std::string value_sql = emit_expr(node->argument_list(0));
    if (value_sql.empty()) return std::nullopt;
    return absl::StrCat("bq_extract(", *part1, ", ", value_sql, ")");
  }
  return std::nullopt;
}

std::optional<std::string> TryEmitDateAddThreeArg(
    const ::googlesql::ResolvedFunctionCall* node,
    const EmitExprFn& emit_expr) {
  if (node->argument_list_size() != 3) return std::nullopt;
  std::string date_sql = emit_expr(node->argument_list(0));
  std::string amount_sql = emit_expr(node->argument_list(1));
  const auto part = TryIntervalPartEnum(node->argument_list(2));
  if (date_sql.empty() || amount_sql.empty() || !part.has_value()) {
    return std::nullopt;
  }
  return absl::StrCat(
      "bq_date_add(", date_sql, ", ", amount_sql, ", ", *part, ")");
}

std::optional<std::string> TryEmitDateAddIntervalArg(
    const ::googlesql::ResolvedFunctionCall* node,
    const EmitExprFn& emit_expr) {
  if (node->argument_list_size() != 2) return std::nullopt;
  std::string date_sql = emit_expr(node->argument_list(0));
  if (date_sql.empty()) return std::nullopt;
  const ::googlesql::ResolvedExpr* interval_expr = node->argument_list(1);
  if (interval_expr == nullptr ||
      interval_expr->node_kind() != ::googlesql::RESOLVED_FUNCTION_CALL) {
    return std::nullopt;
  }
  const auto* interval_call =
      interval_expr->GetAs<::googlesql::ResolvedFunctionCall>();
  const auto iv = TryDecomposeIntervalArgs(interval_call);
  if (!iv.has_value()) return std::nullopt;
  std::string amount_sql =
      emit_expr(interval_call->argument_list(iv->amount_arg_index));
  if (amount_sql.empty()) return std::nullopt;
  return absl::StrCat(
      "bq_date_add(", date_sql, ", ", amount_sql, ", ", iv->part_enum, ")");
}

std::optional<std::string> TryEmitDateAddCall(
    const ::googlesql::ResolvedFunctionCall* node,
    const EmitExprFn& emit_expr) {
  if (auto three = TryEmitDateAddThreeArg(node, emit_expr); three.has_value()) {
    return three;
  }
  return TryEmitDateAddIntervalArg(node, emit_expr);
}

std::optional<std::string> TryEmitIntervalCall(
    const ::googlesql::ResolvedFunctionCall* node,
    const EmitExprFn& emit_expr) {
  const auto iv = TryDecomposeIntervalArgs(node);
  if (!iv.has_value()) return std::nullopt;
  std::string amount_sql = emit_expr(node->argument_list(iv->amount_arg_index));
  if (amount_sql.empty()) return std::nullopt;
  return EmitIntervalProduct(amount_sql, iv->part_enum);
}

// TIMESTAMP_ADD / TIMESTAMP_SUB only accept fixed-width parts (BQ
// analyzer contract); everything else stays on the semantic route.
bool IsFixedWidthTimestampPart(int part_enum) {
  switch (static_cast<DateTimestampPart>(part_enum)) {
    case DateTimestampPart::DAY:
    case DateTimestampPart::HOUR:
    case DateTimestampPart::MINUTE:
    case DateTimestampPart::SECOND:
    case DateTimestampPart::MILLISECOND:
    case DateTimestampPart::MICROSECOND:
      return true;
    default:
      return false;
  }
}

// `INTERVAL 7 DAY` constant-folds to a ResolvedLiteral of INTERVAL
// type. Decompose it to total microseconds when it has no calendar
// (month) component and no sub-microsecond remainder. IntervalValue
// bounds (|days| <= 3.66e6, |micros| <= 3.2e11 s) keep the result
// far inside int64 range.
std::optional<int64_t> TryLiteralIntervalMicros(
    const ::googlesql::ResolvedExpr* expr) {
  if (expr == nullptr || expr->node_kind() != ::googlesql::RESOLVED_LITERAL) {
    return std::nullopt;
  }
  const auto* lit = expr->GetAs<::googlesql::ResolvedLiteral>();
  if (lit == nullptr || lit->value().is_null() ||
      lit->value().type_kind() != ::googlesql::TYPE_INTERVAL) {
    return std::nullopt;
  }
  const ::googlesql::IntervalValue& iv = lit->value().interval_value();
  if (iv.get_months() != 0 || iv.get_nano_fractions() != 0) {
    return std::nullopt;
  }
  return iv.get_days() * int64_t{86400000000} + iv.get_micros();
}

constexpr int kMicrosecondPartEnum =
    static_cast<int>(DateTimestampPart::MICROSECOND);

// Emits `bq_timestamp_add(...)` / `bq_timestamp_sub(...)`. Always
// returns a value for these function names: either the lowered SQL
// or "" (transpile failure) -- never std::nullopt, which would fall
// through to the generic dispatch and emit the macro with the wrong
// arity. `CanLowerTimestampAddSub` mirrors the accepted shapes for
// the route classifier.
std::string EmitTimestampAddSubCall(
    absl::string_view name,
    const ::googlesql::ResolvedFunctionCall* node,
    const EmitExprFn& emit_expr) {
  const char* macro =
      (name == "timestamp_sub") ? "bq_timestamp_sub" : "bq_timestamp_add";
  if (node->argument_list_size() == 3) {
    std::string ts_sql = emit_expr(node->argument_list(0));
    std::string amount_sql = emit_expr(node->argument_list(1));
    const auto part = TryIntervalPartEnum(node->argument_list(2));
    if (ts_sql.empty() || amount_sql.empty() || !part.has_value() ||
        !IsFixedWidthTimestampPart(*part)) {
      return "";
    }
    return absl::StrCat(macro, "(", ts_sql, ", ", amount_sql, ", ", *part, ")");
  }
  if (node->argument_list_size() != 2) return "";
  std::string ts_sql = emit_expr(node->argument_list(0));
  if (ts_sql.empty()) return "";
  const ::googlesql::ResolvedExpr* interval_expr = node->argument_list(1);
  if (const auto micros = TryLiteralIntervalMicros(interval_expr);
      micros.has_value()) {
    return absl::StrCat(
        macro, "(", ts_sql, ", ", *micros, ", ", kMicrosecondPartEnum, ")");
  }
  if (interval_expr == nullptr ||
      interval_expr->node_kind() != ::googlesql::RESOLVED_FUNCTION_CALL) {
    return "";
  }
  const auto* interval_call =
      interval_expr->GetAs<::googlesql::ResolvedFunctionCall>();
  const auto iv = TryDecomposeIntervalArgs(interval_call);
  if (!iv.has_value() || !IsFixedWidthTimestampPart(iv->part_enum)) return "";
  std::string amount_sql =
      emit_expr(interval_call->argument_list(iv->amount_arg_index));
  if (amount_sql.empty()) return "";
  return absl::StrCat(
      macro, "(", ts_sql, ", ", amount_sql, ", ", iv->part_enum, ")");
}

}  // namespace

std::optional<std::string> TryEmitDateTimeFunctionCall(
    absl::string_view name,
    const ::googlesql::ResolvedFunctionCall* node,
    const EmitExprFn& emit_expr) {
  if (node == nullptr) return std::nullopt;
  if (name == "$extract" || name == "extract") {
    return TryEmitExtractCall(node, emit_expr);
  }
  if (name == "date_add") {
    return TryEmitDateAddCall(node, emit_expr);
  }
  if (name == "timestamp_add" || name == "timestamp_sub") {
    return EmitTimestampAddSubCall(name, node, emit_expr);
  }
  if (name == "$interval") {
    return TryEmitIntervalCall(node, emit_expr);
  }
  return std::nullopt;
}

bool CanLowerTimestampAddSub(const ::googlesql::ResolvedFunctionCall* node) {
  if (node == nullptr) return false;
  if (node->argument_list_size() == 3) {
    const auto part = TryIntervalPartEnum(node->argument_list(2));
    return part.has_value() && IsFixedWidthTimestampPart(*part);
  }
  if (node->argument_list_size() != 2) return false;
  const ::googlesql::ResolvedExpr* interval_expr = node->argument_list(1);
  if (TryLiteralIntervalMicros(interval_expr).has_value()) return true;
  if (interval_expr == nullptr ||
      interval_expr->node_kind() != ::googlesql::RESOLVED_FUNCTION_CALL) {
    return false;
  }
  const auto iv = TryDecomposeIntervalArgs(
      interval_expr->GetAs<::googlesql::ResolvedFunctionCall>());
  return iv.has_value() && IsFixedWidthTimestampPart(iv->part_enum);
}

}  // namespace internal
}  // namespace transpiler
}  // namespace duckdb
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
