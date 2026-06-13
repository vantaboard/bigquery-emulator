#include "backend/engine/duckdb/transpiler/transpiler_emit_datetime.h"

#include <optional>
#include <string>

#include "absl/strings/str_cat.h"
#include "backend/engine/duckdb/transpiler/transpiler_internal.h"
#include "googlesql/public/functions/date_time_util.h"
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

}  // namespace

std::optional<std::string> TryEmitDateTimeFunctionCall(
    absl::string_view name,
    const ::googlesql::ResolvedFunctionCall* node,
    const EmitExprFn& emit_expr) {
  if (node == nullptr) return std::nullopt;
  if (name == "$extract" || name == "extract") {
    if (node->argument_list_size() == 2) {
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
    }
    return std::nullopt;
  }
  if (name == "date_add") {
    if (node->argument_list_size() == 3) {
      std::string date_sql = emit_expr(node->argument_list(0));
      std::string amount_sql = emit_expr(node->argument_list(1));
      const auto part = TryIntervalPartEnum(node->argument_list(2));
      if (!date_sql.empty() && !amount_sql.empty() && part.has_value()) {
        return absl::StrCat(
            "bq_date_add(", date_sql, ", ", amount_sql, ", ", *part, ")");
      }
      return std::nullopt;
    }
    if (node->argument_list_size() == 2) {
      std::string date_sql = emit_expr(node->argument_list(0));
      if (date_sql.empty()) return std::nullopt;
      const ::googlesql::ResolvedExpr* interval_expr = node->argument_list(1);
      if (interval_expr != nullptr &&
          interval_expr->node_kind() == ::googlesql::RESOLVED_FUNCTION_CALL) {
        const auto* interval_call =
            interval_expr->GetAs<::googlesql::ResolvedFunctionCall>();
        if (auto iv = TryDecomposeIntervalArgs(interval_call); iv.has_value()) {
          std::string amount_sql =
              emit_expr(interval_call->argument_list(iv->amount_arg_index));
          if (amount_sql.empty()) return std::nullopt;
          return absl::StrCat("bq_date_add(",
                              date_sql,
                              ", ",
                              amount_sql,
                              ", ",
                              iv->part_enum,
                              ")");
        }
      }
    }
    return std::nullopt;
  }
  if (name == "$interval") {
    if (auto iv = TryDecomposeIntervalArgs(node); iv.has_value()) {
      std::string amount_sql =
          emit_expr(node->argument_list(iv->amount_arg_index));
      if (amount_sql.empty()) return std::nullopt;
      return EmitIntervalProduct(amount_sql, iv->part_enum);
    }
    return std::nullopt;
  }
  return std::nullopt;
}

}  // namespace internal
}  // namespace transpiler
}  // namespace duckdb
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
