

#include "googlesql/public/catalog.h"
#include "googlesql/public/function.h"
#include "googlesql/public/sql_function.h"
#include "googlesql/public/templated_sql_function.h"
#include "googlesql/public/type.h"
#include "googlesql/public/types/struct_type.h"
#include "googlesql/public/value.h"
#include "googlesql/resolved_ast/resolved_node.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace duckdb {
namespace transpiler {

Transpiler::Transpiler() = default;
Transpiler::~Transpiler() = default;

std::string Transpiler::TranspileScan(const ::googlesql::ResolvedScan* scan) {
  return EmitScan(scan);
}

std::string Transpiler::Transpile(const ::googlesql::ResolvedNode* node) {
  // Single-entry dispatch: we walk by `node_kind()` so the per-shape
  // `Emit*` methods can recurse through `EmitExpr` / `EmitScan` /
  // `Transpile` without each one knowing the full node hierarchy.
  // Returning "" is the contract `DuckDBEngine::ExecuteQuery` reads
  // as "not yet supported" -- the engine surfaces UNIMPLEMENTED to
  // the gateway through the empty-string contract.
  if (node == nullptr) return "";
  switch (node->node_kind()) {
    case ::googlesql::RESOLVED_QUERY_STMT:
      return EmitQueryStmt(node->GetAs<::googlesql::ResolvedQueryStmt>());
    case ::googlesql::RESOLVED_TABLE_SCAN:
    case ::googlesql::RESOLVED_FILTER_SCAN:
    case ::googlesql::RESOLVED_PROJECT_SCAN:
    case ::googlesql::RESOLVED_SINGLE_ROW_SCAN:
    case ::googlesql::RESOLVED_JOIN_SCAN:
    case ::googlesql::RESOLVED_ARRAY_SCAN:
    case ::googlesql::RESOLVED_AGGREGATE_SCAN:
    case ::googlesql::RESOLVED_ORDER_BY_SCAN:
    case ::googlesql::RESOLVED_LIMIT_OFFSET_SCAN:
    case ::googlesql::RESOLVED_ANALYTIC_SCAN:
    case ::googlesql::RESOLVED_SET_OPERATION_SCAN:
    case ::googlesql::RESOLVED_SAMPLE_SCAN:
    case ::googlesql::RESOLVED_WITH_SCAN:
    case ::googlesql::RESOLVED_WITH_REF_SCAN:
    case ::googlesql::RESOLVED_PIVOT_SCAN:
    case ::googlesql::RESOLVED_UNPIVOT_SCAN:
    case ::googlesql::RESOLVED_RECURSIVE_SCAN:
    case ::googlesql::RESOLVED_RECURSIVE_REF_SCAN:
      return EmitScan(node->GetAs<::googlesql::ResolvedScan>());
    case ::googlesql::RESOLVED_LITERAL:
    case ::googlesql::RESOLVED_PARAMETER:
    case ::googlesql::RESOLVED_COLUMN_REF:
    case ::googlesql::RESOLVED_FUNCTION_CALL:
    case ::googlesql::RESOLVED_CAST:
    case ::googlesql::RESOLVED_MAKE_STRUCT:
    case ::googlesql::RESOLVED_GET_STRUCT_FIELD:
    case ::googlesql::RESOLVED_GET_JSON_FIELD:
    case ::googlesql::RESOLVED_WITH_EXPR:
    case ::googlesql::RESOLVED_SUBQUERY_EXPR:
      return EmitExpr(node->GetAs<::googlesql::ResolvedExpr>());
    default:
      return "";
  }
}

std::string Transpiler::EmitExpr(const ::googlesql::ResolvedExpr* expr) {
  if (expr == nullptr) return "";
  switch (expr->node_kind()) {
    case ::googlesql::RESOLVED_LITERAL:
      return EmitLiteral(expr->GetAs<::googlesql::ResolvedLiteral>());
    case ::googlesql::RESOLVED_PARAMETER:
      return EmitParameter(expr->GetAs<::googlesql::ResolvedParameter>());
    case ::googlesql::RESOLVED_COLUMN_REF:
      return EmitColumnRef(expr->GetAs<::googlesql::ResolvedColumnRef>());
    case ::googlesql::RESOLVED_FUNCTION_CALL:
      return EmitFunctionCall(expr->GetAs<::googlesql::ResolvedFunctionCall>());
    case ::googlesql::RESOLVED_CAST:
      return EmitCast(expr->GetAs<::googlesql::ResolvedCast>());
    case ::googlesql::RESOLVED_MAKE_STRUCT:
      return EmitMakeStruct(expr->GetAs<::googlesql::ResolvedMakeStruct>());
    case ::googlesql::RESOLVED_GET_STRUCT_FIELD:
      return EmitGetStructField(
          expr->GetAs<::googlesql::ResolvedGetStructField>());
    case ::googlesql::RESOLVED_GET_JSON_FIELD:
      return EmitGetJsonField(expr->GetAs<::googlesql::ResolvedGetJsonField>());
    case ::googlesql::RESOLVED_WITH_EXPR:
      return EmitWithExpr(expr->GetAs<::googlesql::ResolvedWithExpr>());
    case ::googlesql::RESOLVED_SUBQUERY_EXPR:
      return EmitSubqueryExpr(expr->GetAs<::googlesql::ResolvedSubqueryExpr>());
    default:
      return "";
  }
}

std::string Transpiler::EmitScan(const ::googlesql::ResolvedScan* scan) {
  if (scan == nullptr) return "";
  switch (scan->node_kind()) {
    case ::googlesql::RESOLVED_TABLE_SCAN:
      return EmitTableScan(scan->GetAs<::googlesql::ResolvedTableScan>());
    case ::googlesql::RESOLVED_FILTER_SCAN:
      return EmitFilterScan(scan->GetAs<::googlesql::ResolvedFilterScan>());
    case ::googlesql::RESOLVED_PROJECT_SCAN:
      return EmitProjectScan(scan->GetAs<::googlesql::ResolvedProjectScan>());
    case ::googlesql::RESOLVED_SINGLE_ROW_SCAN:
      return EmitSingleRowScan(
          scan->GetAs<::googlesql::ResolvedSingleRowScan>());
    case ::googlesql::RESOLVED_JOIN_SCAN:
      return EmitJoinScan(scan->GetAs<::googlesql::ResolvedJoinScan>());
    case ::googlesql::RESOLVED_ARRAY_SCAN:
      return EmitArrayScan(scan->GetAs<::googlesql::ResolvedArrayScan>());
    case ::googlesql::RESOLVED_AGGREGATE_SCAN:
      return EmitAggregateScan(
          scan->GetAs<::googlesql::ResolvedAggregateScan>());
    case ::googlesql::RESOLVED_ORDER_BY_SCAN:
      return EmitOrderByScan(scan->GetAs<::googlesql::ResolvedOrderByScan>());
    case ::googlesql::RESOLVED_LIMIT_OFFSET_SCAN:
      return EmitLimitOffsetScan(
          scan->GetAs<::googlesql::ResolvedLimitOffsetScan>());
    case ::googlesql::RESOLVED_ANALYTIC_SCAN:
      return EmitAnalyticScan(scan->GetAs<::googlesql::ResolvedAnalyticScan>());
    case ::googlesql::RESOLVED_SET_OPERATION_SCAN:
      return EmitSetOperationScan(
          scan->GetAs<::googlesql::ResolvedSetOperationScan>());
    case ::googlesql::RESOLVED_SAMPLE_SCAN:
      return EmitSampleScan(scan->GetAs<::googlesql::ResolvedSampleScan>());
    case ::googlesql::RESOLVED_WITH_SCAN:
      return EmitWithScan(scan->GetAs<::googlesql::ResolvedWithScan>());
    case ::googlesql::RESOLVED_WITH_REF_SCAN:
      return EmitWithRefScan(scan->GetAs<::googlesql::ResolvedWithRefScan>());
    case ::googlesql::RESOLVED_PIVOT_SCAN:
      return EmitPivotScan(scan->GetAs<::googlesql::ResolvedPivotScan>());
    case ::googlesql::RESOLVED_UNPIVOT_SCAN:
      return EmitUnpivotScan(scan->GetAs<::googlesql::ResolvedUnpivotScan>());
    case ::googlesql::RESOLVED_RECURSIVE_SCAN:
      return EmitRecursiveScan(
          scan->GetAs<::googlesql::ResolvedRecursiveScan>());
    case ::googlesql::RESOLVED_RECURSIVE_REF_SCAN:
      return EmitRecursiveRefScan(
          scan->GetAs<::googlesql::ResolvedRecursiveRefScan>());
    default:
      return "";
  }
}

}  // namespace transpiler
}  // namespace duckdb
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
