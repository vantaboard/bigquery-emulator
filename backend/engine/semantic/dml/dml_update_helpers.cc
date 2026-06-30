#include <utility>
#include <vector>

#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "backend/engine/semantic/dml/dml_executor_internal.h"
#include "backend/engine/semantic/row_source.h"
#include "backend/schema/schema.h"
#include "backend/storage/storage.h"
#include "googlesql/resolved_ast/resolved_ast.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace semantic {
namespace dml {

namespace {

absl::Status RecordUpdatedRow(
    const ::googlesql::ResolvedUpdateStmt& upd,
    storage::Row mutated,
    const absl::flat_hash_map<int, int>& by_id,
    const schema::TableSchema& schema,
    std::vector<storage::Row>* rewritten,
    std::vector<ColumnBindings>* returning_contexts,
    std::vector<std::string>* returning_actions,
    std::unique_ptr<RowSource>* returning_out,
    int64_t* updated) {
  ++(*updated);
  if (upd.returning() != nullptr && returning_out != nullptr) {
    auto post_bind = BindRow(mutated, *upd.table_scan(), by_id, schema);
    if (!post_bind.ok()) return post_bind.status();
    returning_contexts->push_back(*std::move(post_bind));
    returning_actions->push_back("UPDATE");
  }
  rewritten->push_back(std::move(mutated));
  return absl::OkStatus();
}

absl::Status ApplyUpdateToMatchedRow(
    const ::googlesql::ResolvedUpdateStmt& upd,
    const storage::Row& row,
    const ColumnBindings& eval_ctx,
    const std::vector<SetAssignment>& sets,
    const std::vector<NestedArrayDeleteAssignment>& nested_deletes,
    const absl::flat_hash_map<int, int>& by_id,
    const schema::TableSchema& schema,
    EvalContext& ctx,
    std::vector<storage::Row>* rewritten,
    std::vector<ColumnBindings>* returning_contexts,
    std::vector<std::string>* returning_actions,
    std::unique_ptr<RowSource>* returning_out,
    int64_t* updated) {
  storage::Row mutated = row;
  absl::Status applied = ApplyUpdateSets(
      mutated, sets, nested_deletes, eval_ctx, schema, ctx);
  if (!applied.ok()) return applied;
  return RecordUpdatedRow(upd,
                          std::move(mutated),
                          by_id,
                          schema,
                          rewritten,
                          returning_contexts,
                          returning_actions,
                          returning_out,
                          updated);
}

}  // namespace

absl::Status ProcessOneUpdateRow(
    const ::googlesql::ResolvedUpdateStmt& upd,
    const storage::Row& row,
    const ColumnBindings& target_bind,
    const std::vector<SetAssignment>& sets,
    const std::vector<NestedArrayDeleteAssignment>& nested_deletes,
    const std::vector<ColumnBindings>& from_rows,
    const absl::flat_hash_map<int, int>& by_id,
    const schema::TableSchema& schema,
    EvalContext& ctx,
    std::vector<storage::Row>* rewritten,
    std::vector<ColumnBindings>* returning_contexts,
    std::vector<std::string>* returning_actions,
    std::unique_ptr<RowSource>* returning_out,
    int64_t* updated) {
  if (upd.from_scan() == nullptr) {
    ctx.columns = &target_bind;
    auto matched_or = EvalWherePredicate(upd.where_expr(), ctx);
    ctx.columns = nullptr;
    if (!matched_or.ok()) return matched_or.status();
    if (!*matched_or) {
      rewritten->push_back(row);
      return absl::OkStatus();
    }
    return ApplyUpdateToMatchedRow(upd,
                                   row,
                                   target_bind,
                                   sets,
                                   nested_deletes,
                                   by_id,
                                   schema,
                                   ctx,
                                   rewritten,
                                   returning_contexts,
                                   returning_actions,
                                   returning_out,
                                   updated);
  }

  ColumnBindings matched_from;
  auto from_match_or =
      CountFromScanMatches(upd, target_bind, from_rows, ctx, &matched_from);
  if (!from_match_or.ok()) return from_match_or.status();
  if (!*from_match_or) {
    rewritten->push_back(row);
    return absl::OkStatus();
  }
  return ApplyUpdateToMatchedRow(upd,
                                 row,
                                 matched_from,
                                 sets,
                                 nested_deletes,
                                 by_id,
                                 schema,
                                 ctx,
                                 rewritten,
                                 returning_contexts,
                                 returning_actions,
                                 returning_out,
                                 updated);
}

}  // namespace dml
}  // namespace semantic
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
