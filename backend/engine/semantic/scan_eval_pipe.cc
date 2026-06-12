#include <utility>
#include <vector>

#include "absl/status/statusor.h"
#include "absl/strings/str_cat.h"
#include "backend/engine/semantic/scan_eval_internal.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace semantic {
namespace scan_eval_internal {

namespace {

using ::bigquery_emulator::backend::engine::semantic::EvalContext;

absl::StatusOr<std::vector<ColumnBindings>> ProjectRowsToColumnList(
    const std::vector<ColumnBindings>& rows,
    const std::vector<::googlesql::ResolvedColumn>& columns) {
  std::vector<ColumnBindings> out;
  out.reserve(rows.size());
  for (const ColumnBindings& in_row : rows) {
    ColumnBindings row;
    row.reserve(columns.size());
    for (const ::googlesql::ResolvedColumn& col : columns) {
      const int col_id = col.column_id();
      auto it = in_row.find(col_id);
      if (it == in_row.end()) {
        return absl::InternalError(
            absl::StrCat("semantic: row missing column '", col.name(), "'"));
      }
      row.emplace(col_id, it->second);
    }
    out.push_back(std::move(row));
  }
  return out;
}

SubpipelineEvalScope* ActiveSubpipelineScope(const EvalContext& ctx,
                                             SubpipelineEvalScope& local) {
  if (ctx.subpipeline == nullptr) {
    return &local;
  }
  return ctx.subpipeline;
}

}  // namespace

absl::StatusOr<std::vector<ColumnBindings>> MaterializeSubpipelineInputScan(
    const ::googlesql::ResolvedSubpipelineInputScan& scan,
    const EvalContext& ctx) {
  if (ctx.subpipeline == nullptr || ctx.subpipeline->input_stack.empty()) {
    return absl::InternalError(
        "semantic: SubpipelineInputScan without active subpipeline input");
  }
  return ProjectRowsToColumnList(ctx.subpipeline->input_stack.back(),
                                 scan.column_list());
}

absl::StatusOr<std::vector<ColumnBindings>> MaterializeSubpipeline(
    const ::googlesql::ResolvedSubpipeline& subpipeline,
    const std::vector<ColumnBindings>& input_rows,
    EvalContext& ctx) {
  if (subpipeline.scan() == nullptr) {
    return absl::InternalError("semantic: ResolvedSubpipeline has null scan");
  }
  SubpipelineEvalScope local_scope;
  SubpipelineEvalScope* previous = ctx.subpipeline;
  SubpipelineEvalScope* scope = ActiveSubpipelineScope(ctx, local_scope);
  ctx.subpipeline = scope;
  const size_t old_size = scope->input_stack.size();
  scope->input_stack.push_back(input_rows);
  auto result = MaterializeScanImpl(subpipeline.scan(), ctx);
  if (scope->input_stack.size() != old_size) {
    ctx.subpipeline = previous;
    return absl::InternalError(
        "semantic: subpipeline input stack imbalance after materialization");
  }
  ctx.subpipeline = previous;
  return result;
}

absl::StatusOr<std::vector<ColumnBindings>> MaterializePipeIfScan(
    const ::googlesql::ResolvedPipeIfScan& pipe, EvalContext& ctx) {
  auto input_or = MaterializeScanImpl(pipe.input_scan(), ctx);
  if (!input_or.ok()) return input_or.status();

  if (pipe.selected_case() == -1) {
    return ProjectRowsToColumnList(*input_or, pipe.column_list());
  }
  if (pipe.selected_case() < 0 ||
      pipe.selected_case() >= pipe.if_case_list_size()) {
    return absl::InternalError(
        "semantic: PipeIfScan selected_case out of range");
  }
  const ::googlesql::ResolvedPipeIfCase* if_case =
      pipe.if_case_list(pipe.selected_case());
  if (if_case == nullptr || if_case->subpipeline() == nullptr) {
    return absl::InternalError(
        "semantic: selected PipeIfCase missing resolved subpipeline");
  }
  auto sub_or = MaterializeSubpipeline(*if_case->subpipeline(), *input_or, ctx);
  if (!sub_or.ok()) return sub_or.status();
  return ProjectRowsToColumnList(*sub_or, pipe.column_list());
}

absl::StatusOr<std::vector<ColumnBindings>> MaterializePipeTeeScan(
    const ::googlesql::ResolvedPipeTeeScan& tee, EvalContext& ctx) {
  auto input_or = MaterializeScanImpl(tee.input_scan(), ctx);
  if (!input_or.ok()) return input_or.status();

  for (int i = 0; i < tee.subpipeline_list_size(); ++i) {
    const ::googlesql::ResolvedGeneralizedQuerySubpipeline* branch =
        tee.subpipeline_list(i);
    if (branch == nullptr || branch->subpipeline() == nullptr) {
      return absl::InternalError(
          "semantic: PipeTeeScan subpipeline entry is malformed");
    }
    auto side_or =
        MaterializeSubpipeline(*branch->subpipeline(), *input_or, ctx);
    if (!side_or.ok()) return side_or.status();
  }
  return ProjectRowsToColumnList(*input_or, tee.column_list());
}

absl::StatusOr<std::vector<ColumnBindings>> MaterializePipeForkScan(
    const ::googlesql::ResolvedPipeForkScan& fork, EvalContext& ctx) {
  auto input_or = MaterializeScanImpl(fork.input_scan(), ctx);
  if (!input_or.ok()) return input_or.status();

  for (int i = 0; i < fork.subpipeline_list_size(); ++i) {
    const ::googlesql::ResolvedGeneralizedQuerySubpipeline* branch =
        fork.subpipeline_list(i);
    if (branch == nullptr || branch->subpipeline() == nullptr) {
      return absl::InternalError(
          "semantic: PipeForkScan subpipeline entry is malformed");
    }
    auto branch_or =
        MaterializeSubpipeline(*branch->subpipeline(), *input_or, ctx);
    if (!branch_or.ok()) return branch_or.status();
  }
  return std::vector<ColumnBindings>{};
}

}  // namespace scan_eval_internal
}  // namespace semantic
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
