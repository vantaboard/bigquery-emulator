#ifndef BIGQUERY_EMULATOR_BACKEND_ENGINE_SEMANTIC_SCAN_EVAL_INTERNAL_H_
#define BIGQUERY_EMULATOR_BACKEND_ENGINE_SEMANTIC_SCAN_EVAL_INTERNAL_H_

#include <cstdint>
#include <string>
#include <vector>

#include "absl/container/flat_hash_map.h"
#include "absl/status/statusor.h"
#include "absl/strings/string_view.h"
#include "backend/engine/semantic/eval_expr.h"
#include "backend/engine/semantic/scan_eval.h"
#include "backend/engine/semantic/value.h"
#include "googlesql/resolved_ast/resolved_ast.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace semantic {
namespace scan_eval_internal {

const ::googlesql::ResolvedScan* StripBarrierScans(
    const ::googlesql::ResolvedScan* scan);
bool ValueLess(const Value& a, const Value& b);
bool ValueEqual(const Value& a, const Value& b);
int CompareArrayAggOrderKey(const ::googlesql::ResolvedOrderByItem& item,
                            const Value& va,
                            const Value& vb);
absl::StatusOr<bool> EvalBoolExpr(const ::googlesql::ResolvedExpr* expr,
                                  EvalContext& ctx);
void PopulateColumnNameBindings(
    const ::googlesql::ResolvedScan* scan,
    const ColumnBindings& row,
    absl::flat_hash_map<std::string, ::googlesql::Value>& out);
void PopulateColumnNameBindingsDeep(
    const ::googlesql::ResolvedScan* scan,
    const ColumnBindings& row,
    absl::flat_hash_map<std::string, ::googlesql::Value>& out);
absl::StatusOr<std::vector<ColumnBindings>> ProjectRows(
    const ::googlesql::ResolvedProjectScan& project,
    const std::vector<ColumnBindings>& input_rows,
    const EvalContext& ctx);
absl::StatusOr<std::vector<ColumnBindings>> MaterializeTableScan(
    const ::googlesql::ResolvedTableScan& scan);
absl::StatusOr<std::vector<ColumnBindings>> MaterializeSingleRowScan(
    const ::googlesql::ResolvedSingleRowScan& scan);
absl::StatusOr<std::vector<ColumnBindings>> MaterializeWithRefScan(
    const ::googlesql::ResolvedWithRefScan& scan, EvalContext& ctx);
absl::StatusOr<std::vector<ColumnBindings>> MaterializeSetOperationScan(
    const ::googlesql::ResolvedSetOperationScan& scan, EvalContext& ctx);
absl::StatusOr<std::vector<ColumnBindings>> MaterializeJoinScan(
    const ::googlesql::ResolvedJoinScan& join, EvalContext& ctx);
absl::StatusOr<std::vector<ColumnBindings>> MaterializeArrayScan(
    const ::googlesql::ResolvedArrayScan& array, EvalContext& ctx);
absl::StatusOr<int64_t> EvalLimitOffsetInt64(
    const ::googlesql::ResolvedExpr* expr,
    EvalContext& ctx,
    absl::string_view label);
absl::StatusOr<std::vector<ColumnBindings>> MaterializeLimitOffsetScan(
    const ::googlesql::ResolvedLimitOffsetScan& scan, EvalContext& ctx);
absl::StatusOr<std::vector<ColumnBindings>> MaterializeScanImpl(
    const ::googlesql::ResolvedScan* scan, EvalContext& ctx);

absl::StatusOr<Value> EvalArrayAgg(
    const ::googlesql::ResolvedAggregateFunctionCall& call,
    const std::vector<std::vector<Value>>& input_column_values,
    const std::vector<ColumnBindings>& input_rows,
    EvalContext& ctx);
absl::StatusOr<Value> EvalStringAgg(
    const ::googlesql::ResolvedAggregateFunctionCall& call,
    const std::vector<std::vector<Value>>& input_column_values,
    const std::vector<ColumnBindings>& input_rows,
    EvalContext& ctx);
absl::StatusOr<std::vector<size_t>> FilterRowsByHavingModifier(
    const ::googlesql::ResolvedAggregateFunctionCall& call,
    const std::vector<ColumnBindings>& input_rows,
    const std::vector<size_t>& row_indices,
    const EvalContext& ctx);
absl::StatusOr<std::vector<ColumnBindings>> MaterializeAggregateScan(
    const ::googlesql::ResolvedAggregateScan& aggregate, EvalContext& ctx);
absl::StatusOr<std::vector<ColumnBindings>> MaterializeAnalyticScan(
    const ::googlesql::ResolvedAnalyticScan& analytic, EvalContext& ctx);
absl::StatusOr<std::vector<ColumnBindings>> MaterializeSampleScan(
    const ::googlesql::ResolvedSampleScan& scan, EvalContext& ctx);
absl::StatusOr<std::vector<ColumnBindings>> MaterializeSubpipelineInputScan(
    const ::googlesql::ResolvedSubpipelineInputScan& scan,
    const EvalContext& ctx);
absl::StatusOr<std::vector<ColumnBindings>> MaterializeSubpipeline(
    const ::googlesql::ResolvedSubpipeline& subpipeline,
    const std::vector<ColumnBindings>& input_rows,
    EvalContext& ctx);
absl::StatusOr<std::vector<ColumnBindings>> MaterializePipeIfScan(
    const ::googlesql::ResolvedPipeIfScan& pipe, EvalContext& ctx);
absl::StatusOr<std::vector<ColumnBindings>> MaterializePipeTeeScan(
    const ::googlesql::ResolvedPipeTeeScan& tee, EvalContext& ctx);
absl::StatusOr<std::vector<ColumnBindings>> MaterializePipeForkScan(
    const ::googlesql::ResolvedPipeForkScan& fork, EvalContext& ctx);
absl::StatusOr<std::vector<ColumnBindings>> MaterializeGroupRowsScan(
    const ::googlesql::ResolvedGroupRowsScan& scan, const EvalContext& ctx);
std::string GroupKeyFingerprint(const std::vector<Value>& keys);
absl::StatusOr<Value> EvalAggregateForRows(
    const ::googlesql::ResolvedAggregateFunctionCall& agg,
    const ::googlesql::ResolvedScan* input_scan,
    const std::vector<ColumnBindings>& input_rows,
    const std::vector<size_t>& row_indices,
    EvalContext& ctx);
absl::StatusOr<std::vector<ColumnBindings>> MaterializeMatchRecognizeScan(
    const ::googlesql::ResolvedMatchRecognizeScan& scan, EvalContext& ctx);

}  // namespace scan_eval_internal
}  // namespace semantic
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator

#endif  // BIGQUERY_EMULATOR_BACKEND_ENGINE_SEMANTIC_SCAN_EVAL_INTERNAL_H_
