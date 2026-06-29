#ifndef BIGQUERY_EMULATOR_BACKEND_ENGINE_DUCKDB_TRANSPILER_TRANSPILER_EMIT_EXPR_HELPERS_H_
#define BIGQUERY_EMULATOR_BACKEND_ENGINE_DUCKDB_TRANSPILER_TRANSPILER_EMIT_EXPR_HELPERS_H_

#include <string>

#include "absl/strings/string_view.h"
#include "googlesql/public/type.h"
#include "googlesql/resolved_ast/resolved_ast.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace duckdb {
namespace transpiler {

bool CastHasUnsupportedFeatures(const ::googlesql::ResolvedCast* node);

std::string NormalizeTimestampCastInner(absl::string_view inner,
                                          const ::googlesql::ResolvedExpr* expr);

std::string TryEmitStructTypeCast(const std::string& inner,
                                    const ::googlesql::Type* target,
                                    const ::googlesql::ResolvedExpr* expr);

std::string FormatCastExpression(absl::string_view inner,
                                   absl::string_view type_sql,
                                   bool return_null_on_error);

}  // namespace transpiler
}  // namespace duckdb
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator

#endif  // BIGQUERY_EMULATOR_BACKEND_ENGINE_DUCKDB_TRANSPILER_TRANSPILER_EMIT_EXPR_HELPERS_H_
