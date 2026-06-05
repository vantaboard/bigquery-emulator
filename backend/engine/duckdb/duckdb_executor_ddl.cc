#include <algorithm>
#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/string_view.h"
#include "backend/engine/duckdb/duckdb_executor.h"
#include "backend/engine/duckdb/duckdb_executor_internal.h"
#include "backend/schema/googlesql_to_bq.h"
#include "backend/schema/schema.h"
#include "backend/storage/storage.h"
#include "googlesql/resolved_ast/resolved_ast.h"
#include "googlesql/resolved_ast/resolved_node_kind.pb.h"
#include "proto/emulator.pb.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace duckdb {
namespace internal {

// Resolve `name_path` (as produced by `ResolvedCreateStatement::
// name_path()` etc.) to a `storage::TableId` under
// `default_project_id`. Mirrors `GoogleSqlCatalog::FindTable`'s
// rules: two-segment paths default the project, three-segment
// paths override it. Anything else surfaces as INVALID_ARGUMENT so
// the gateway can map it to BigQuery's matching REST error.
absl::StatusOr<storage::TableId> NamePathToTableId(
    const std::vector<std::string>& name_path,
    absl::string_view default_project_id) {
  if (name_path.size() == 2) {
    return storage::TableId{
        std::string(default_project_id), name_path[0], name_path[1]};
  }
  if (name_path.size() == 3) {
    return storage::TableId{name_path[0], name_path[1], name_path[2]};
  }
  return absl::InvalidArgumentError(absl::StrCat(
      "duckdb engine: DDL target name must be <dataset>.<table> or "
      "<project>.<dataset>.<table>; got ",
      name_path.size(),
      " segments"));
}

// Render the BigQuery FieldSchema mode (NULLABLE / REQUIRED /
// REPEATED) from a resolved column definition. The analyzer carries
// the NOT NULL annotation on `annotations()->not_null()`; ARRAY
// types lower onto REPEATED at the field-schema layer (see
// `backend/schema/googlesql_to_bq.cc::TypeToFieldSchema`), so for
// ARRAY columns we leave the mode alone -- TypeToFieldSchema sets
// it to REPEATED already.
void ApplyAnnotations(const ::googlesql::ResolvedColumnDefinition* def,
                      v1::FieldSchema* field) {
  if (def == nullptr) return;
  if (field->mode() == "REPEATED") return;
  const ::googlesql::ResolvedColumnAnnotations* ann = def->annotations();
  if (ann != nullptr && ann->not_null()) {
    field->set_mode("REQUIRED");
  }
}

// ALTER TABLE ADD COLUMN executor. The storage interface has no
// in-place schema evolution today, so we scan-drop-create-append
// to land the new column. `is_if_not_exists()` on the action
// swallows the "column already present" case; we detect that by
// looking up the new column's name in the existing schema before
// touching the storage backend.
// Translate a single RESOLVED_ADD_COLUMN_ACTION into a
// `ColumnSchema`. Returns `nullopt` when `IF NOT EXISTS` skipped the
// column because it already exists, or a non-OK status when the
// action is malformed / refers to an already-existing column without
// IF NOT EXISTS. Used by `RunAlterTableAddColumn` to keep the
// per-action body off the parent's complexity budget.
absl::StatusOr<std::optional<schema::ColumnSchema>> ProcessAddColumnAction(
    const ::googlesql::ResolvedAlterAction* action,
    const schema::TableSchema& existing,
    const storage::TableId& target) {
  if (action == nullptr) return std::optional<schema::ColumnSchema>{};
  if (action->node_kind() != ::googlesql::RESOLVED_ADD_COLUMN_ACTION) {
    return absl::UnimplementedError(absl::StrCat(
        "duckdb engine: ALTER TABLE action ",
        action->node_kind_string(),
        " is not implemented (only ADD COLUMN is supported); ALTER TABLE "
        "is on the control_op route per docs/ENGINE_POLICY.md and plan "
        "docs/ENGINE_POLICY.md, but the per-action handlers "
        "still "
        "live here pending the migration"));
  }
  const auto* add = action->GetAs<::googlesql::ResolvedAddColumnAction>();
  const ::googlesql::ResolvedColumnDefinition* def = add->column_definition();
  if (def == nullptr) {
    return absl::InternalError(
        "DuckDBEngine::ExecuteDdl: ADD COLUMN action has null column "
        "definition");
  }
  const bool already = std::any_of(
      existing.columns.begin(),
      existing.columns.end(),
      [&](const schema::ColumnSchema& c) { return c.name == def->name(); });
  if (already) {
    if (add->is_if_not_exists()) return std::optional<schema::ColumnSchema>{};
    return absl::AlreadyExistsError(
        absl::StrCat("duckdb engine: ALTER TABLE ADD COLUMN: column '",
                     def->name(),
                     "' already exists on table ",
                     target.project_id,
                     ".",
                     target.dataset_id,
                     ".",
                     target.table_id));
  }
  v1::FieldSchema field;
  absl::Status s =
      backend::schema::TypeToFieldSchema(def->type(), def->name(), &field);
  if (!s.ok()) return s;
  ApplyAnnotations(def, &field);
  absl::StatusOr<schema::ColumnSchema> column =
      backend::schema::ColumnSchemaFromProto(field);
  if (!column.ok()) return column.status();
  return std::optional<schema::ColumnSchema>{*std::move(column)};
}

// Drain every row from `target` into an owned vector, padding each
// row's cell list with NULL for every entry in `added_columns` so the
// post-ALTER schema can be applied with a single OverwriteRows-style
// rewrite. Used by `RunAlterTableAddColumn` to keep the row-rewrite
// loop off the parent's complexity budget.
absl::StatusOr<std::vector<storage::Row>> CopyRowsForAlter(
    storage::Storage& storage,
    const storage::TableId& target,
    absl::Span<const schema::ColumnSchema> added_columns) {
  absl::StatusOr<std::unique_ptr<storage::RowIterator>> iter =
      storage.ScanRows(target);
  if (!iter.ok()) return iter.status();
  std::vector<storage::Row> rows;
  std::unique_ptr<storage::RowIterator> rows_iter = std::move(iter).value();
  storage::Row row;
  while (true) {
    absl::StatusOr<bool> has = rows_iter->Next(&row);
    if (!has.ok()) return has.status();
    if (!*has) break;
    rows.push_back(row);
  }
  for (storage::Row& r : rows) {
    for (size_t i = 0; i < added_columns.size(); ++i) {
      r.cells.push_back(storage::Value::Null());
    }
  }
  return rows;
}

// Drop, recreate (with `new_schema`), and re-append `rows` to
// `target` -- the scan-and-rewrite implementation of ALTER TABLE ADD
// COLUMN that this engine relies on because DuckDB's `ALTER TABLE`
// does not propagate to the storage backend's catalog yet.
absl::Status RebuildTableWithColumns(storage::Storage& storage,
                                     const storage::TableId& target,
                                     const schema::TableSchema& new_schema,
                                     absl::Span<const storage::Row> rows) {
  absl::Status dropped = storage.DropTable(target);
  if (!dropped.ok()) return dropped;
  absl::Status created = storage.CreateTable(target, new_schema);
  if (!created.ok()) return created;
  if (rows.empty()) return absl::OkStatus();
  return storage.AppendRows(target, rows);
}

absl::Status RunAlterTableAddColumn(
    storage::Storage& storage,
    absl::string_view project_id,
    const ::googlesql::ResolvedAlterTableStmt* stmt) {
  if (stmt == nullptr) {
    return absl::InternalError(
        "DuckDBEngine::ExecuteDdl: ALTER TABLE has null resolved statement");
  }
  absl::StatusOr<storage::TableId> target =
      NamePathToTableId(stmt->name_path(), project_id);
  if (!target.ok()) return target.status();

  absl::StatusOr<schema::TableSchema> existing = storage.GetSchema(*target);
  if (!existing.ok()) {
    if (existing.status().code() == absl::StatusCode::kNotFound &&
        stmt->is_if_exists()) {
      return absl::OkStatus();
    }
    return existing.status();
  }

  // Collect the new columns we need to add by walking the action
  // list. We only implement RESOLVED_ADD_COLUMN_ACTION today; every
  // other action surfaces as UNIMPLEMENTED so the followup
  // alter-action plans see a stable contract to fill in.
  std::vector<schema::ColumnSchema> added_columns;
  added_columns.reserve(stmt->alter_action_list_size());
  for (int i = 0; i < stmt->alter_action_list_size(); ++i) {
    absl::StatusOr<std::optional<schema::ColumnSchema>> column =
        ProcessAddColumnAction(stmt->alter_action_list(i), *existing, *target);
    if (!column.ok()) return column.status();
    if (column->has_value()) added_columns.push_back(*std::move(*column));
  }
  if (added_columns.empty()) return absl::OkStatus();

  // Scan rows under the *current* schema before we touch anything;
  // `CopyRowsForAlter` pads every existing row's cell list with NULL
  // for each newly added column so storage's per-row shape stays
  // aligned with the post-ALTER schema.
  absl::StatusOr<std::vector<storage::Row>> rows =
      CopyRowsForAlter(storage, *target, added_columns);
  if (!rows.ok()) return rows.status();

  schema::TableSchema new_schema = *existing;
  for (const schema::ColumnSchema& c : added_columns) {
    new_schema.columns.push_back(c);
  }
  return RebuildTableWithColumns(
      storage, *target, new_schema, absl::MakeConstSpan(*rows));
}
}  // namespace internal

absl::Status DuckDbExecutor::ExecuteDdl(
    const QueryRequest& request,
    const ::googlesql::ResolvedStatement& stmt,
    ::googlesql::Catalog* catalog) {
  (void)catalog;  // analysis has already happened on the coordinator.
  if (storage_ == nullptr) {
    return absl::FailedPreconditionError(
        "DuckDbExecutor::ExecuteDdl: storage backend is not configured");
  }

  const absl::string_view project_id = request.project_id;
  switch (stmt.node_kind()) {
    case ::googlesql::RESOLVED_ALTER_TABLE_STMT:
      return internal::RunAlterTableAddColumn(
          *storage_,
          project_id,
          stmt.GetAs<::googlesql::ResolvedAlterTableStmt>());
    default:
      // CREATE TABLE / CTAS / DROP TABLE / ANALYZE moved to
      // `backend/engine/control/control_op_executor.cc` when
      // `docs/ENGINE_POLICY.md` landed; the route classifier
      // dispatches them to the `ControlOpExecutor` directly. If
      // one reaches us here, the classifier or the YAML
      // disposition row drifted out of sync with this dispatch
      // table -- surface UNIMPLEMENTED so the gateway/e2e suite
      // catches the regression instead of silently routing the
      // statement through DuckDB.
      return absl::UnimplementedError(absl::StrCat(
          "duckdb engine: ExecuteDdl only handles ALTER TABLE today; got ",
          stmt.node_kind_string(),
          ". CREATE TABLE / CREATE TABLE AS SELECT / DROP TABLE / "
          "ANALYZE are owned by ControlOpExecutor "
          "(see backend/engine/control/control_op_executor.cc) and "
          "the route classifier should never dispatch them here."));
  }
}

}  // namespace duckdb
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
