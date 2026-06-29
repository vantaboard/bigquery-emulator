#ifndef BIGQUERY_EMULATOR_BACKEND_ENGINE_DUCKDB_TRANSPILER_TRANSPILER_EMIT_PIVOT_HELPERS_H_
#define BIGQUERY_EMULATOR_BACKEND_ENGINE_DUCKDB_TRANSPILER_TRANSPILER_EMIT_PIVOT_HELPERS_H_

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
using EmitAggregateFn = std::function<std::string(
    const ::googlesql::ResolvedAggregateFunctionCall*)>;
using EmitLiteralFn =
    std::function<std::string(const ::googlesql::ResolvedLiteral*)>;

struct PivotScanParts {
  std::vector<std::string> pivot_values_sql;
  std::vector<std::string> pivot_exprs_sql;
  std::vector<std::string> projections;
  std::vector<std::string> group_by_sql;
};

bool CollectPivotScanParts(const ::googlesql::ResolvedPivotScan* node,
                           absl::string_view for_sql,
                           const EmitExprFn& emit_expr,
                           const EmitAggregateFn& emit_agg,
                           PivotScanParts* out);

std::string BuildPivotSelectSql(absl::string_view inner,
                                  const PivotScanParts& parts);

struct UnpivotScanParts {
  std::vector<std::string> projected_input_sql;
  std::vector<std::string> value_tuples;
  std::vector<std::string> lateral_col_names;
  std::vector<std::string> outer_projections;
  std::string label_col_name;
};

bool CollectUnpivotScanParts(const ::googlesql::ResolvedUnpivotScan* node,
                             const EmitExprFn& emit_expr,
                             const EmitLiteralFn& emit_literal,
                             UnpivotScanParts* out);

std::string BuildUnpivotSelectSql(absl::string_view inner,
                                    const ::googlesql::ResolvedUnpivotScan* node,
                                    const UnpivotScanParts& parts);

}  // namespace transpiler
}  // namespace duckdb
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator

#endif  // BIGQUERY_EMULATOR_BACKEND_ENGINE_DUCKDB_TRANSPILER_TRANSPILER_EMIT_PIVOT_HELPERS_H_
