#ifndef BIGQUERY_EMULATOR_BACKEND_ENGINE_DUCKDB_TRANSPILER_TRANSPILER_EMIT_AGGREGATE_FUNCTION_HELPERS_H_
#define BIGQUERY_EMULATOR_BACKEND_ENGINE_DUCKDB_TRANSPILER_TRANSPILER_EMIT_AGGREGATE_FUNCTION_HELPERS_H_

#include <string>
#include <vector>

#include "absl/strings/string_view.h"
#include "backend/engine/duckdb/transpiler/functions.h"
#include "googlesql/resolved_ast/resolved_ast.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace duckdb {
namespace transpiler {

std::string EmitDispositionAggregateCall(
    absl::string_view name,
    const ::googlesql::ResolvedAggregateFunctionCall* node,
    const std::vector<std::string>& args,
    absl::string_view order_suffix,
    const FnEntry* entry,
    const std::string& filter_suffix);

}  // namespace transpiler
}  // namespace duckdb
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator

#endif  // BIGQUERY_EMULATOR_BACKEND_ENGINE_DUCKDB_TRANSPILER_TRANSPILER_EMIT_AGGREGATE_FUNCTION_HELPERS_H_
