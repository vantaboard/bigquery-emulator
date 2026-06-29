#ifndef BIGQUERY_EMULATOR_BACKEND_ENGINE_DUCKDB_TRANSPILER_TRANSPILER_EMIT_QUERY_HELPERS_H_
#define BIGQUERY_EMULATOR_BACKEND_ENGINE_DUCKDB_TRANSPILER_TRANSPILER_EMIT_QUERY_HELPERS_H_

#include <functional>
#include <string>
#include <vector>

#include "absl/strings/string_view.h"
#include "googlesql/resolved_ast/resolved_ast.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace duckdb {
namespace transpiler {

void FilterOutputOrderForQueryStmt(const ::googlesql::ResolvedQueryStmt* node,
                                   std::vector<std::string>* output_order_items,
                                   std::vector<int>* output_order_column_ids);

void AppendQueryOrderByClause(const ::googlesql::ResolvedQueryStmt* node,
                              absl::string_view inner,
                              const std::vector<std::string>& outputs,
                              const std::vector<std::string>& outer_refs,
                              bool join_id_aliases_in_query,
                              bool input_rn_ordering,
                              bool output_includes_input_rn,
                              std::vector<std::string>* output_order_items,
                              std::vector<int>* output_order_column_ids,
                              bool* input_rn_ordering_out,
                              std::string* sql);

bool IsNoOpProjectScan(const ::googlesql::ResolvedProjectScan* node);

bool BuildProjectScanProjections(
    const ::googlesql::ResolvedProjectScan* node,
    bool input_id_aliases,
    const std::function<
        std::string(const ::googlesql::ResolvedComputedColumn*)>& emit_computed,
    std::vector<std::string>* projections);

void AppendProjectScanOrderColumns(
    const std::vector<std::string>& output_order_items,
    const std::vector<int>& output_order_column_ids,
    bool join_id_aliases,
    bool input_id_aliases,
    bool input_has_rn_column,
    bool suppress_rn,
    std::vector<std::string>* projections);

}  // namespace transpiler
}  // namespace duckdb
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator

#endif  // BIGQUERY_EMULATOR_BACKEND_ENGINE_DUCKDB_TRANSPILER_TRANSPILER_EMIT_QUERY_HELPERS_H_
