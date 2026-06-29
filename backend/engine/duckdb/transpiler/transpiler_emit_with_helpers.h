#ifndef BIGQUERY_EMULATOR_BACKEND_ENGINE_DUCKDB_TRANSPILER_TRANSPILER_EMIT_WITH_HELPERS_H_
#define BIGQUERY_EMULATOR_BACKEND_ENGINE_DUCKDB_TRANSPILER_TRANSPILER_EMIT_WITH_HELPERS_H_

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

using EmitExprFn = std::function<std::string(const ::googlesql::ResolvedExpr*)>;
using EmitScanFn = std::function<std::string(const ::googlesql::ResolvedScan*)>;

struct WithEntryEmitResult {
  std::string cte_sql;
  bool has_rn = false;
};

std::string WithScanColumnAnchor(int idx);

WithEntryEmitResult EmitNonRecursiveWithEntry(
    const ::googlesql::ResolvedWithEntry* entry,
    const ::googlesql::ResolvedScan* sub_scan,
    bool body_needs_input_rn,
    const EmitScanFn& emit_scan);

std::string FormatRecursiveWithEntry(
    absl::string_view query_name,
    const std::vector<std::string>& anchor_names,
    absl::string_view body_sql);

int FindRecursionDepthColumnIndex(
    const ::googlesql::ResolvedRecursiveScan* node,
    const ::googlesql::ResolvedRecursionDepthModifier* depth_mod);

std::string BuildRecursiveScanArm(
    const ::googlesql::ResolvedRecursiveScan* node,
    const std::vector<std::string>& anchor_names,
    int depth_col_idx,
    const ::googlesql::ResolvedRecursionDepthModifier* depth_mod,
    const EmitExprFn& emit_expr,
    const EmitScanFn& emit_scan,
    const ::googlesql::ResolvedSetOperationItem* item,
    bool is_recursive_arm);

}  // namespace transpiler
}  // namespace duckdb
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator

#endif  // BIGQUERY_EMULATOR_BACKEND_ENGINE_DUCKDB_TRANSPILER_TRANSPILER_EMIT_WITH_HELPERS_H_
