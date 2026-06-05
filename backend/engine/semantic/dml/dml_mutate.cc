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
    EvalContext& ctx) {
  absl::Status guard = RejectUnsupportedDmlFeatures(
      del.returning(),
      del.assert_rows_modified(),
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
    } else {
      kept.push_back(row);
    }
  }
  ctx.columns = nullptr;

  if (deleted > 0) {
    absl::Status overwrote =
        // cpp-lint:allow(status-discarded) -- captured into overwrote
        storage.OverwriteRows(target->storage_table_id(), kept);
    if (!overwrote.ok()) return overwrote;
  }

  DmlStats stats;
  stats.deleted_row_count = deleted;
  return stats;
}

// UPDATE applies one or more `update_item_list` items per matched
// row. Today we only handle the scalar-SET form
// (`update_item->target() == ResolvedColumnRef` AND `set_value()`
// is non-null AND the nested-update lists are empty); the deep-
// STRUCT path (`SET s.a.b = ...`, where `target()` is a
// `ResolvedGetStructField` chain) is owned by the
// plan. Each unsupported subshape surfaces a structured
// `kNotImplemented`.
absl::StatusOr<DmlStats> ExecuteUpdate(
    const ::googlesql::ResolvedUpdateStmt& upd,
    storage::Storage& storage,
    EvalContext& ctx) {
  absl::Status guard = RejectUnsupportedDmlFeatures(
      upd.returning(),
      upd.assert_rows_modified(),
      /*has_array_offset_column=*/upd.array_offset_column() != nullptr,
      upd.generated_column_expr_list_size(),
      "UPDATE");
  if (!guard.ok()) return guard;
  if (upd.from_scan() != nullptr) {
    return MakeSemanticError(
        SemanticErrorReason::kNotImplemented,
        "semantic/dml: UPDATE ... FROM ... is not yet supported");
  }

  auto target_or = StorageTargetFor(upd, "UPDATE");
  if (!target_or.ok()) return target_or.status();
  const catalog::StorageTable* target = *target_or;
  const schema::TableSchema& schema = target->bq_schema();

  // Validate every update item up-front (cheap; lets us bail
  // before scanning rows when the executor cannot land the SET
  // shape). For the scalar form, `target()` must resolve to a
  // top-level `ResolvedColumnRef` over the same table scan.
  struct ScalarSet {
    int column_idx = -1;  // index into bq_schema_.columns
    const ::googlesql::ResolvedExpr* set_expr = nullptr;
  };
  std::vector<ScalarSet> sets;
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
    if (item->target()->node_kind() != ::googlesql::RESOLVED_COLUMN_REF) {
      return MakeSemanticError(
          SemanticErrorReason::kNotImplemented,
          absl::StrCat("semantic/dml: UPDATE SET target kind ",
                       item->target()->node_kind_string(),
                       " (deep STRUCT mutation) is not yet supported; see "
                       "docs/ENGINE_POLICY.md"));
    }
    if (item->set_value() == nullptr || item->set_value()->value() == nullptr) {
      return absl::InternalError(
          "semantic/dml: scalar UPDATE item has null set_value");
    }
    const auto* col_ref =
        item->target()->GetAs<::googlesql::ResolvedColumnRef>();
    const int idx = IndexOfColumn(schema, col_ref->column().name());
    if (idx < 0) {
      return absl::InternalError(
          absl::StrCat("semantic/dml: UPDATE target column '",
                       col_ref->column().name(),
                       "' not found in storage table schema"));
    }
    sets.push_back({idx, item->set_value()->value()});
  }

  auto by_id = BuildColumnIndexByColumnId(*upd.table_scan(), schema);
  if (!by_id.ok()) return by_id.status();

  auto rows_or = ScanAllRows(storage, target->storage_table_id());
  if (!rows_or.ok()) return rows_or.status();
  std::vector<storage::Row> rows = *std::move(rows_or);

  std::vector<storage::Row> rewritten;
  rewritten.reserve(rows.size());
  int64_t updated = 0;
  for (const storage::Row& row : rows) {
    auto bind = BindRow(row, *upd.table_scan(), *by_id, schema);
    if (!bind.ok()) return bind.status();
    ColumnBindings local = *std::move(bind);
    ctx.columns = &local;
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
    if (!matched) {
      ctx.columns = nullptr;
      rewritten.push_back(row);
      continue;
    }
    storage::Row mutated = row;
    for (const ScalarSet& s : sets) {
      auto v = EvalExpr(*s.set_expr, ctx);
      if (!v.ok()) {
        ctx.columns = nullptr;
        return v.status();
      }
      auto cell = ToStorageValue(*v);
      if (!cell.ok()) {
        ctx.columns = nullptr;
        return cell.status();
      }
      mutated.cells[s.column_idx] = *std::move(cell);
    }
    ctx.columns = nullptr;
    ++updated;
    rewritten.push_back(std::move(mutated));
  }
  ctx.columns = nullptr;

  if (updated > 0) {
    absl::Status overwrote =
        // cpp-lint:allow(status-discarded) -- captured into overwrote
        storage.OverwriteRows(target->storage_table_id(), rewritten);
    if (!overwrote.ok()) return overwrote;
  }

  DmlStats stats;
  stats.updated_row_count = updated;
  return stats;
}

absl::StatusOr<DmlStats> ExecuteDml(const QueryRequest& request,
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

  switch (kind) {
    case ::googlesql::RESOLVED_INSERT_STMT:
      return ExecuteInsert(
          *stmt.GetAs<::googlesql::ResolvedInsertStmt>(), *storage, ctx);
    case ::googlesql::RESOLVED_DELETE_STMT:
      return ExecuteDelete(
          *stmt.GetAs<::googlesql::ResolvedDeleteStmt>(), *storage, ctx);
    case ::googlesql::RESOLVED_UPDATE_STMT:
      return ExecuteUpdate(
          *stmt.GetAs<::googlesql::ResolvedUpdateStmt>(), *storage, ctx);
    case ::googlesql::RESOLVED_MERGE_STMT:
      // Simple MERGE branches stay on the DuckDB fast path
      // (`duckdb_rewrite`); the harder matrix
      // (`WHEN NOT MATCHED BY SOURCE`, multi-action sequences)
      // is owned by the plan and deferred from this
      // subagent.
      return MakeSemanticError(
          SemanticErrorReason::kNotImplemented,
          "semantic/dml: MERGE harder branches (WHEN NOT MATCHED BY "
          "SOURCE / multi-action) are not yet supported; see "
          "docs/ENGINE_POLICY.md");
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
