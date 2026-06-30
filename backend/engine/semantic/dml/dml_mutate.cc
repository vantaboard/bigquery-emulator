#include <cstddef>
#include <cstdint>
#include <string>
#include <utility>
#include <vector>

#include "absl/container/flat_hash_map.h"
#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "absl/strings/ascii.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/string_view.h"
#include "backend/catalog/storage_table.h"
#include "backend/engine/semantic/dml/dml_executor.h"
#include "backend/engine/semantic/dml/dml_executor_internal.h"
#include "backend/engine/semantic/error.h"
#include "backend/engine/semantic/eval_expr.h"
#include "backend/engine/semantic/row_source.h"
#include "backend/engine/semantic/scan_eval.h"
#include "backend/engine/semantic/value.h"
#include "backend/schema/schema.h"
#include "backend/storage/storage.h"
#include "googlesql/public/value.h"
#include "googlesql/resolved_ast/resolved_ast.h"
#include "googlesql/resolved_ast/resolved_node_kind.pb.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace semantic {
namespace dml {

absl::StatusOr<DmlStats> ExecuteDelete(
    const ::googlesql::ResolvedDeleteStmt& del,
    storage::Storage& storage,
    EvalContext& ctx,
    std::unique_ptr<RowSource>* returning_out) {
  absl::Status guard = RejectUnsupportedDmlFeatures(
      /*generated_column_count=*/0, "DELETE");
  if (!guard.ok()) return guard;

  auto target_or = StorageTargetFor(del, "DELETE");
  if (!target_or.ok()) return target_or.status();
  const catalog::StorageTable* target = *target_or;
  const schema::TableSchema& schema = target->bq_schema();

  auto by_id = BuildColumnIndexByColumnId(*del.table_scan(), schema);
  if (!by_id.ok()) return by_id.status();

  auto rows_or = ScanAllRows(storage, target->storage_table_id());
  if (!rows_or.ok()) return rows_or.status();
  std::vector<storage::Row> rows = *std::move(rows_or);

  // Walk the row source, evaluate `where_expr` per row. A row
  // survives iff the predicate evaluates to TRUE; FALSE and
  // SQL-NULL both delete the row -- BigQuery treats `WHERE NULL`
  // identically to `WHERE FALSE` for `DELETE` (the predicate is
  // a "delete me" filter, not a "keep me" one, so the no-WHERE
  // case maps to TRUE-everywhere in the analyzer).
  std::vector<storage::Row> kept;
  kept.reserve(rows.size());
  std::vector<ColumnBindings> returning_contexts;
  std::vector<std::string> returning_actions;
  int64_t deleted = 0;
  for (const storage::Row& row : rows) {
    auto bind = BindRow(row, *del.table_scan(), *by_id, schema);
    if (!bind.ok()) return bind.status();
    ColumnBindings local = *std::move(bind);
    ctx.columns = &local;
    auto matched_or = EvalWherePredicate(del.where_expr(), ctx);
    ctx.columns = nullptr;
    if (!matched_or.ok()) return matched_or.status();
    if (*matched_or) {
      ++deleted;
      if (del.returning() != nullptr && returning_out != nullptr) {
        returning_contexts.push_back(local);
        returning_actions.push_back("DELETE");
      }
    } else {
      kept.push_back(row);
    }
  }
  ctx.columns = nullptr;

  DmlStats stats;
  stats.deleted_row_count = deleted;
  absl::Status assert_ok = CheckAssertRowsModified(
      del.assert_rows_modified(), DmlStatementKind::kDelete, stats, ctx);
  if (!assert_ok.ok()) return assert_ok;

  if (deleted > 0) {
    absl::Status overwrote =
        // cpp-lint:allow(status-discarded) -- captured into overwrote
        storage.OverwriteRows(target->storage_table_id(), kept);
    if (!overwrote.ok()) return overwrote;
  }

  if (del.returning() != nullptr && returning_out != nullptr &&
      !returning_contexts.empty()) {
    return FinalizeMutateWithReturning(del.returning(),
                                       returning_out,
                                       std::move(returning_contexts),
                                       std::move(returning_actions),
                                       stats,
                                       ctx);
  }
  return stats;
}

// UPDATE applies one or more `update_item_list` items per matched
// row. Scalar `SET col = <expr>` and deep-STRUCT `SET s.a.b = ...`
// land on the semantic value layer; `UPDATE ... FROM ...` joins
// the target table against `from_scan` and surfaces BigQuery's
// multiple-match error when more than one source row matches a
// target row.
absl::StatusOr<DmlStats> ExecuteUpdate(
    const ::googlesql::ResolvedUpdateStmt& upd,
    storage::Storage& storage,
    EvalContext& ctx,
    std::unique_ptr<RowSource>* returning_out) {
  absl::Status guard = RejectUnsupportedDmlFeatures(
      upd.generated_column_expr_list_size(), "UPDATE");
  if (!guard.ok()) return guard;

  auto target_or = StorageTargetFor(upd, "UPDATE");
  if (!target_or.ok()) return target_or.status();
  const catalog::StorageTable* target = *target_or;
  const schema::TableSchema& schema = target->bq_schema();

  auto assignments_or = ParseUpdateAssignments(upd, schema);
  if (!assignments_or.ok()) return assignments_or.status();
  const std::vector<SetAssignment>& sets = assignments_or->first;
  const std::vector<NestedArrayDeleteAssignment>& nested_deletes =
      assignments_or->second;

  auto by_id = BuildColumnIndexByColumnId(*upd.table_scan(), schema);
  if (!by_id.ok()) return by_id.status();

  auto rows_or = ScanAllRows(storage, target->storage_table_id());
  if (!rows_or.ok()) return rows_or.status();
  std::vector<storage::Row> rows = *std::move(rows_or);

  std::vector<ColumnBindings> from_rows;
  if (upd.from_scan() != nullptr) {
    auto materialized = MaterializeScan(upd.from_scan(), ctx);
    if (!materialized.ok()) return materialized.status();
    from_rows = *std::move(materialized);
  }

  std::vector<storage::Row> rewritten;
  rewritten.reserve(rows.size());
  std::vector<ColumnBindings> returning_contexts;
  std::vector<std::string> returning_actions;
  int64_t updated = 0;
  for (const storage::Row& row : rows) {
    auto bind = BindRow(row, *upd.table_scan(), *by_id, schema);
    if (!bind.ok()) return bind.status();
    absl::Status processed = ProcessOneUpdateRow(upd,
                                                 row,
                                                 *bind,
                                                 sets,
                                                 nested_deletes,
                                                 from_rows,
                                                 *by_id,
                                                 schema,
                                                 ctx,
                                                 &rewritten,
                                                 &returning_contexts,
                                                 &returning_actions,
                                                 returning_out,
                                                 &updated);
    if (!processed.ok()) return processed;
  }
  ctx.columns = nullptr;

  DmlStats stats;
  stats.updated_row_count = updated;
  absl::Status assert_ok = CheckAssertRowsModified(
      upd.assert_rows_modified(), DmlStatementKind::kUpdate, stats, ctx);
  if (!assert_ok.ok()) return assert_ok;

  if (updated > 0) {
    absl::Status overwrote =
        // cpp-lint:allow(status-discarded) -- captured into overwrote
        storage.OverwriteRows(target->storage_table_id(), rewritten);
    if (!overwrote.ok()) return overwrote;
  }
  if (upd.returning() != nullptr && returning_out != nullptr &&
      !returning_contexts.empty()) {
    return FinalizeMutateWithReturning(upd.returning(),
                                       returning_out,
                                       std::move(returning_contexts),
                                       std::move(returning_actions),
                                       stats,
                                       ctx);
  }
  return stats;
}

absl::StatusOr<DmlResult> ExecuteDml(const QueryRequest& request,
                                     const ::googlesql::ResolvedStatement& stmt,
                                     ::googlesql::Catalog* catalog,
                                     storage::Storage* storage) {
  (void)catalog;  // analysis is owned by the coordinator above us.

  // Storage is only required for the kinds the executor actually
  // mutates (INSERT / UPDATE / DELETE today). Statements that route
  // through here without a DML kind (e.g. unit tests that hand a
  // SELECT to `ExecuteDml` to assert the kNotImplemented surface)
  // get the same `kNotImplemented` envelope they would get with a
  // configured storage backend.
  const auto kind = stmt.node_kind();
  const bool is_writer_kind = kind == ::googlesql::RESOLVED_INSERT_STMT ||
                              kind == ::googlesql::RESOLVED_UPDATE_STMT ||
                              kind == ::googlesql::RESOLVED_DELETE_STMT;
  if (is_writer_kind && storage == nullptr) {
    return absl::FailedPreconditionError(
        "semantic/dml: ExecuteDml called with null storage");
  }

  ParameterBindings bindings;
  if (!request.parameters.empty()) {
    auto built = BuildParameterBindings(request);
    if (!built.ok()) return built.status();
    bindings = *std::move(built);
  }
  EvalContext ctx;
  ctx.project_id = request.project_id;
  ctx.parameters = &bindings;

  DmlResult out;
  switch (kind) {
    case ::googlesql::RESOLVED_INSERT_STMT: {
      const auto* insert = stmt.GetAs<::googlesql::ResolvedInsertStmt>();
      auto stats = ExecuteInsert(*insert, *storage, ctx, &out.returning_rows);
      if (!stats.ok()) return stats.status();
      out.stats = *std::move(stats);
      return out;
    }
    case ::googlesql::RESOLVED_DELETE_STMT: {
      const auto* del = stmt.GetAs<::googlesql::ResolvedDeleteStmt>();
      auto stats = ExecuteDelete(*del, *storage, ctx, &out.returning_rows);
      if (!stats.ok()) return stats.status();
      out.stats = *std::move(stats);
      return out;
    }
    case ::googlesql::RESOLVED_UPDATE_STMT: {
      const auto* upd = stmt.GetAs<::googlesql::ResolvedUpdateStmt>();
      auto stats = ExecuteUpdate(*upd, *storage, ctx, &out.returning_rows);
      if (!stats.ok()) return stats.status();
      out.stats = *std::move(stats);
      return out;
    }
    case ::googlesql::RESOLVED_MERGE_STMT: {
      auto stats = ExecuteMerge(
          *stmt.GetAs<::googlesql::ResolvedMergeStmt>(), *storage, ctx);
      if (!stats.ok()) return stats.status();
      out.stats = *std::move(stats);
      return out;
    }
    case ::googlesql::RESOLVED_GENERALIZED_QUERY_STMT: {
      const auto* gq = stmt.GetAs<::googlesql::ResolvedGeneralizedQueryStmt>();
      const ::googlesql::ResolvedScan* body =
          gq == nullptr ? nullptr : gq->query();
      if (body == nullptr ||
          body->node_kind() != ::googlesql::RESOLVED_PIPE_INSERT_SCAN) {
        return MakeSemanticError(
            SemanticErrorReason::kNotImplemented,
            "semantic/dml: generalized query statement is not a pipe INSERT");
      }
      const auto* pipe = body->GetAs<::googlesql::ResolvedPipeInsertScan>();
      if (pipe == nullptr || pipe->insert_stmt() == nullptr) {
        return absl::InternalError(
            "semantic/dml: ResolvedPipeInsertScan missing insert_stmt");
      }
      auto stats = ExecuteInsert(
          *pipe->insert_stmt(), *storage, ctx, &out.returning_rows);
      if (!stats.ok()) return stats.status();
      out.stats = *std::move(stats);
      return out;
    }
    case ::googlesql::RESOLVED_TRUNCATE_STMT:
      return MakeSemanticError(
          SemanticErrorReason::kNotImplemented,
          "semantic/dml: TRUNCATE TABLE is owned by control-op-executor "
          "(catalog metadata op), not the DML executor");
    default:
      return MakeSemanticError(
          SemanticErrorReason::kNotImplemented,
          absl::StrCat(
              "semantic/dml: ExecuteDml does not handle ",
              stmt.node_kind_string(),
              "; only INSERT / UPDATE / DELETE / MERGE statement kinds "
              "route through the local DML executor"));
  }
}

}  // namespace dml
}  // namespace semantic
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
