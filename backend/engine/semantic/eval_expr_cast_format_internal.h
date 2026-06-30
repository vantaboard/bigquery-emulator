#ifndef BIGQUERY_EMULATOR_BACKEND_ENGINE_SEMANTIC_EVAL_EXPR_CAST_FORMAT_INTERNAL_H_
#define BIGQUERY_EMULATOR_BACKEND_ENGINE_SEMANTIC_EVAL_EXPR_CAST_FORMAT_INTERNAL_H_

#include "absl/status/statusor.h"
#include "absl/strings/string_view.h"
#include "backend/engine/semantic/value.h"
#include "googlesql/resolved_ast/resolved_ast.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace semantic {
namespace eval_expr_internal {
namespace cast_format_internal {

absl::StatusOr<absl::string_view> CastFormatLiteral(
    const ::googlesql::ResolvedCast& cast);
absl::StatusOr<Value> FormatDatetimeToString(
    const ::googlesql::ResolvedCast& cast,
    Value inner,
    absl::string_view format_str);
absl::StatusOr<Value> FormatDateToString(const ::googlesql::ResolvedCast& cast,
                                         Value inner,
                                         absl::string_view format_str);
absl::StatusOr<Value> FormatTimestampToString(
    const ::googlesql::ResolvedCast& cast,
    Value inner,
    absl::string_view format_str);
absl::StatusOr<Value> ParseStringToDateValue(
    const ::googlesql::ResolvedCast& cast,
    absl::string_view format_str,
    absl::string_view text);
absl::StatusOr<Value> ParseStringToDatetimeValue(
    const ::googlesql::ResolvedCast& cast,
    absl::string_view format_str,
    absl::string_view text);
absl::StatusOr<Value> ParseStringToTimestampValue(
    const ::googlesql::ResolvedCast& cast,
    absl::string_view format_str,
    absl::string_view text);
absl::StatusOr<Value> TimestampToStringInZone(
    const ::googlesql::ResolvedCast& cast, Value inner);
absl::StatusOr<Value> TimestampToDatetimeInZone(
    const ::googlesql::ResolvedCast& cast, Value inner);

}  // namespace cast_format_internal
}  // namespace eval_expr_internal
}  // namespace semantic
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator

#endif  // BIGQUERY_EMULATOR_BACKEND_ENGINE_SEMANTIC_EVAL_EXPR_CAST_FORMAT_INTERNAL_H_
