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
      /*has_array_offset_column=*/del.array_offset_column() != nullptr,
      /*generated_column_count=*/0,
      "DELETE");
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
    Value pred_val;
    if (del.where_expr() != nullptr) {
      auto v = EvalExpr(*del.where_expr(), ctx);
      ctx.columns = nullptr;
      if (!v.ok()) return v.status();
      pred_val = *std::move(v);
    } else {
      ctx.columns = nullptr;
      pred_val = Value::Bool(true);
    }
    const bool matched = pred_val.is_valid() && !pred_val.is_null() &&
                         pred_val.type_kind() == ::googlesql::TYPE_BOOL &&
                         pred_val.bool_value();
    if (matched) {
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
    auto returning_or = BuildReturningRowSource(*del.returning(),
                                                std::move(returning_contexts),
                                                std::move(returning_actions),
                                                ctx);
    if (!returning_or.ok()) return returning_or.status();
    *returning_out = *std::move(returning_or);
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
      /*has_array_offset_column=*/upd.array_offset_column() != nullptr,
      upd.generated_column_expr_list_size(),
      "UPDATE");
  if (!guard.ok()) return guard;

  auto target_or = StorageTargetFor(upd, "UPDATE");
  if (!target_or.ok()) return target_or.status();
  const catalog::StorageTable* target = *target_or;
  const schema::TableSchema& schema = target->bq_schema();

  struct SetAssignment {
    UpdateTarget target;
    const ::googlesql::ResolvedExpr* set_expr = nullptr;
    const ::googlesql::Type* root_column_type = nullptr;
  };
  std::vector<SetAssignment> sets;
  sets.reserve(upd.update_item_list_size());
  for (int i = 0; i < upd.update_item_list_size(); ++i) {
    const ::googlesql::ResolvedUpdateItem* item = upd.update_item_list(i);
    if (item == nullptr) {
      return absl::InternalError(
          "semantic/dml: UPDATE update_item_list contains a null entry");
    }
    if (!item->update_item_element_list().empty() ||
        !item->delete_list().empty() || !item->update_list().empty() ||
        !item->insert_list().empty() || item->element_column() != nullptr) {
      return MakeSemanticError(
          SemanticErrorReason::kNotImplemented,
          "semantic/dml: nested UPDATE (array element / sub-record) is "
          "deferred; see docs/ENGINE_POLICY.md");
    }
    if (item->target() == nullptr) {
      return absl::InternalError(
          "semantic/dml: ResolvedUpdateItem has null target");
    }
    if (item->set_value() == nullptr || item->set_value()->value() == nullptr) {
      return absl::InternalError(
          "semantic/dml: scalar UPDATE item has null set_value");
    }
    auto parsed = ParseUpdateTarget(*item->target(), schema);
    if (!parsed.ok()) return parsed.status();
    const ::googlesql::Type* root_type = nullptr;
    for (int c = 0; c < upd.table_scan()->column_list_size(); ++c) {
      const ::googlesql::ResolvedColumn& col = upd.table_scan()->column_list(c);
      if (IndexOfColumn(schema, col.name()) == parsed->column_idx) {
        root_type = col.type();
        break;
      }
    }
    if (root_type == nullptr) {
      return absl::InternalError(
          "semantic/dml: UPDATE target column missing from table scan");
    }
    sets.push_back({*std::move(parsed), item->set_value()->value(), root_type});
  }

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

  auto apply_sets = [&](storage::Row& mutated,
                        const ColumnBindings& row_ctx) -> absl::Status {
    ctx.columns = &row_ctx;
    for (const SetAssignment& s : sets) {
      auto v = EvalExpr(*s.set_expr, ctx);
      if (!v.ok()) return v.status();
      if (s.target.struct_field_path.empty()) {
        auto cell = ToStorageValue(*v);
        if (!cell.ok()) return cell.status();
        mutated.cells[s.target.column_idx] = *std::move(cell);
        continue;
      }
      auto current = catalog::StorageValueToGoogleSqlValue(
          mutated.cells[s.target.column_idx],
          s.root_column_type,
          schema.columns[s.target.column_idx]);
      if (!current.ok()) return current.status();
      auto rewritten =
          SetNestedStructField(*current, s.target.struct_field_path, *v);
      if (!rewritten.ok()) return rewritten.status();
      auto cell = ToStorageValue(*rewritten);
      if (!cell.ok()) return cell.status();
      mutated.cells[s.target.column_idx] = *std::move(cell);
    }
    ctx.columns = nullptr;
    return absl::OkStatus();
  };

  std::vector<storage::Row> rewritten;
  rewritten.reserve(rows.size());
  std::vector<ColumnBindings> returning_contexts;
  std::vector<std::string> returning_actions;
  int64_t updated = 0;
  for (const storage::Row& row : rows) {
    auto bind = BindRow(row, *upd.table_scan(), *by_id, schema);
    if (!bind.ok()) return bind.status();
    ColumnBindings target_bind = *std::move(bind);

    if (upd.from_scan() == nullptr) {
      ctx.columns = &target_bind;
      bool matched = false;
      if (upd.where_expr() == nullptr) {
        matched = true;
      } else {
        auto v = EvalExpr(*upd.where_expr(), ctx);
        ctx.columns = nullptr;
        if (!v.ok()) return v.status();
        matched = v->is_valid() && !v->is_null() &&
                  v->type_kind() == ::googlesql::TYPE_BOOL && v->bool_value();
      }
      if (!matched) {
        ctx.columns = nullptr;
        rewritten.push_back(row);
        continue;
      }
      storage::Row mutated = row;
      absl::Status applied = apply_sets(mutated, target_bind);
      if (!applied.ok()) return applied;
      ++updated;
      if (upd.returning() != nullptr && returning_out != nullptr) {
        auto post_bind = BindRow(mutated, *upd.table_scan(), *by_id, schema);
        if (!post_bind.ok()) return post_bind.status();
        returning_contexts.push_back(*std::move(post_bind));
        returning_actions.push_back("UPDATE");
      }
      rewritten.push_back(std::move(mutated));
      continue;
    }

    int match_count = 0;
    ColumnBindings matched_from;
    for (const ColumnBindings& from_bind : from_rows) {
      ColumnBindings merged = MergeColumnBindings(target_bind, from_bind);
      ctx.columns = &merged;
      bool matched = false;
      if (upd.where_expr() == nullptr) {
        matched = true;
      } else {
        auto v = EvalExpr(*upd.where_expr(), ctx);
        if (!v.ok()) {
          ctx.columns = nullptr;
          return v.status();
        }
        matched = v->is_valid() && !v->is_null() &&
                  v->type_kind() == ::googlesql::TYPE_BOOL && v->bool_value();
      }
      if (matched) {
        ++match_count;
        matched_from = std::move(merged);
      }
    }
    ctx.columns = nullptr;
    if (match_count == 0) {
      rewritten.push_back(row);
      continue;
    }
    if (match_count > 1) {
      return MakeSemanticError(
          SemanticErrorReason::kInvalidArgument,
          "semantic/dml: UPDATE/MERGE must match at most one source row for "
          "each target row");
    }
    storage::Row mutated = row;
    absl::Status applied = apply_sets(mutated, matched_from);
    if (!applied.ok()) return applied;
    ++updated;
    if (upd.returning() != nullptr && returning_out != nullptr) {
      auto post_bind = BindRow(mutated, *upd.table_scan(), *by_id, schema);
      if (!post_bind.ok()) return post_bind.status();
      returning_contexts.push_back(*std::move(post_bind));
      returning_actions.push_back("UPDATE");
    }
    rewritten.push_back(std::move(mutated));
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
    auto returning_or = BuildReturningRowSource(*upd.returning(),
                                                std::move(returning_contexts),
                                                std::move(returning_actions),
                                                ctx);
    if (!returning_or.ok()) return returning_or.status();
    *returning_out = *std::move(returning_or);
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
