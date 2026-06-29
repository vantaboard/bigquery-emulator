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
#include "absl/types/span.h"
#include "backend/engine/control/control_op_internal.h"
#include "backend/schema/googlesql_to_bq.h"
#include "backend/schema/schema.h"
#include "backend/storage/storage.h"
#include "googlesql/resolved_ast/resolved_ast.h"
#include "googlesql/resolved_ast/resolved_node_kind.pb.h"
#include "proto/emulator.pb.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace control {
namespace internal {

namespace {

void ApplyAnnotations(const ::googlesql::ResolvedColumnDefinition* def,
                      v1::FieldSchema* field) {
  if (def == nullptr) return;
  if (field->mode() == "REPEATED") return;
  const ::googlesql::ResolvedColumnAnnotations* ann = def->annotations();
  if (ann != nullptr && ann->not_null()) {
    field->set_mode("REQUIRED");
  }
}

absl::StatusOr<std::optional<schema::ColumnSchema>> ProcessAddColumnAction(
    const ::googlesql::ResolvedAlterAction* action,
    const schema::TableSchema& existing,
    const storage::TableId& target) {
  if (action == nullptr) return std::optional<schema::ColumnSchema>{};
  if (action->node_kind() != ::googlesql::RESOLVED_ADD_COLUMN_ACTION) {
    return absl::UnimplementedError(
        absl::StrCat("control op executor: ALTER TABLE action ",
                     action->node_kind_string(),
                     " is not implemented"));
  }
  const auto* add = action->GetAs<::googlesql::ResolvedAddColumnAction>();
  const ::googlesql::ResolvedColumnDefinition* def = add->column_definition();
  if (def == nullptr) {
    return absl::InternalError(
        "control op executor: ADD COLUMN action has null column definition");
  }
  const bool already = std::any_of(
      existing.columns.begin(),
      existing.columns.end(),
      [&](const schema::ColumnSchema& c) { return c.name == def->name(); });
  if (already) {
    if (add->is_if_not_exists()) return std::optional<schema::ColumnSchema>{};
    return absl::AlreadyExistsError(
        absl::StrCat("control op executor: ALTER TABLE ADD COLUMN: column '",
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

absl::Status ApplyDropColumnAction(
    const ::googlesql::ResolvedAlterAction* action,
    schema::TableSchema* schema,
    const storage::TableId& target) {
  if (action == nullptr) return absl::OkStatus();
  if (action->node_kind() != ::googlesql::RESOLVED_DROP_COLUMN_ACTION) {
    return absl::UnimplementedError(
        absl::StrCat("control op executor: ALTER TABLE action ",
                     action->node_kind_string(),
                     " is not implemented"));
  }
  const auto* drop = action->GetAs<::googlesql::ResolvedDropColumnAction>();
  auto it = std::find_if(
      schema->columns.begin(),
      schema->columns.end(),
      [&](const schema::ColumnSchema& c) { return c.name == drop->name(); });
  if (it == schema->columns.end()) {
    if (drop->is_if_exists()) return absl::OkStatus();
    return absl::NotFoundError(
        absl::StrCat("control op executor: ALTER TABLE DROP COLUMN: column '",
                     drop->name(),
                     "' not found on table ",
                     target.project_id,
                     ".",
                     target.dataset_id,
                     ".",
                     target.table_id));
  }
  schema->columns.erase(it);
  return absl::OkStatus();
}

absl::Status ApplyRenameColumnAction(
    const ::googlesql::ResolvedAlterAction* action,
    schema::TableSchema* schema,
    const storage::TableId& target) {
  if (action == nullptr) return absl::OkStatus();
  if (action->node_kind() != ::googlesql::RESOLVED_RENAME_COLUMN_ACTION) {
    return absl::UnimplementedError(
        absl::StrCat("control op executor: ALTER TABLE action ",
                     action->node_kind_string(),
                     " is not implemented"));
  }
  const auto* rename = action->GetAs<::googlesql::ResolvedRenameColumnAction>();
  auto it = std::find_if(
      schema->columns.begin(),
      schema->columns.end(),
      [&](const schema::ColumnSchema& c) { return c.name == rename->name(); });
  if (it == schema->columns.end()) {
    if (rename->is_if_exists()) return absl::OkStatus();
    return absl::NotFoundError(
        absl::StrCat("control op executor: ALTER TABLE RENAME COLUMN: column '",
                     rename->name(),
                     "' not found on table ",
                     target.project_id,
                     ".",
                     target.dataset_id,
                     ".",
                     target.table_id));
  }
  it->name = rename->new_name();
  return absl::OkStatus();
}

absl::Status ApplySetOptionsAction(
    const ::googlesql::ResolvedAlterAction* action) {
  if (action == nullptr) return absl::OkStatus();
  if (action->node_kind() != ::googlesql::RESOLVED_SET_OPTIONS_ACTION) {
    return absl::UnimplementedError(
        absl::StrCat("control op executor: ALTER TABLE action ",
                     action->node_kind_string(),
                     " is not implemented"));
  }
  // Table options are metadata-only in the emulator today.
  return absl::OkStatus();
}

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

absl::StatusOr<std::vector<storage::Row>> CopyRowsWithSchema(
    storage::Storage& storage,
    const storage::TableId& target,
    const schema::TableSchema& old_schema,
    const schema::TableSchema& new_schema) {
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
    if (row.cells.size() != old_schema.columns.size()) {
      return absl::InternalError(
          "control op executor: ALTER TABLE row shape mismatch");
    }
    storage::Row projected;
    projected.cells.reserve(new_schema.columns.size());
    for (const schema::ColumnSchema& col : new_schema.columns) {
      auto it = std::find_if(
          old_schema.columns.begin(),
          old_schema.columns.end(),
          [&](const schema::ColumnSchema& c) { return c.name == col.name; });
      if (it == old_schema.columns.end()) {
        projected.cells.push_back(storage::Value::Null());
        continue;
      }
      const size_t idx = static_cast<size_t>(it - old_schema.columns.begin());
      projected.cells.push_back(row.cells[idx]);
    }
    rows.push_back(std::move(projected));
  }
  return rows;
}

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

absl::Status ApplyAlterAction(const ::googlesql::ResolvedAlterAction* action,
                              const storage::TableId& target,
                              schema::TableSchema* schema,
                              std::vector<schema::ColumnSchema>* added_columns,
                              bool* schema_changed) {
  switch (action->node_kind()) {
    case ::googlesql::RESOLVED_ADD_COLUMN_ACTION: {
      absl::StatusOr<std::optional<schema::ColumnSchema>> column =
          ProcessAddColumnAction(action, *schema, target);
      if (!column.ok()) return column.status();
      if (column->has_value()) {
        added_columns->push_back(*std::move(*column));
        schema->columns.push_back(added_columns->back());
        *schema_changed = true;
      }
      return absl::OkStatus();
    }
    case ::googlesql::RESOLVED_DROP_COLUMN_ACTION: {
      absl::Status s = ApplyDropColumnAction(action, schema, target);
      if (!s.ok()) return s;
      *schema_changed = true;
      return absl::OkStatus();
    }
    case ::googlesql::RESOLVED_RENAME_COLUMN_ACTION: {
      absl::Status s = ApplyRenameColumnAction(action, schema, target);
      if (!s.ok()) return s;
      *schema_changed = true;
      return absl::OkStatus();
    }
    case ::googlesql::RESOLVED_SET_OPTIONS_ACTION:
      return ApplySetOptionsAction(action);
    default:
      return absl::UnimplementedError(
          absl::StrCat("control op executor: ALTER TABLE action ",
                       action->node_kind_string(),
                       " is not implemented"));
  }
}

}  // namespace

absl::Status RunAlterTable(storage::Storage& storage,
                           absl::string_view project_id,
                           absl::string_view default_dataset_id,
                           const ::googlesql::ResolvedAlterTableStmt* stmt) {
  if (stmt == nullptr) {
    return absl::InternalError(
        "ControlOpExecutor::ExecuteDdl: ALTER TABLE has null resolved "
        "statement");
  }
  absl::StatusOr<storage::TableId> target =
      NamePathToTableId(stmt->name_path(), project_id, default_dataset_id);
  if (!target.ok()) return target.status();

  absl::StatusOr<schema::TableSchema> existing = storage.GetSchema(*target);
  if (!existing.ok()) {
    if (existing.status().code() == absl::StatusCode::kNotFound &&
        stmt->is_if_exists()) {
      return absl::OkStatus();
    }
    return existing.status();
  }

  schema::TableSchema new_schema = *existing;
  std::vector<schema::ColumnSchema> added_columns;
  added_columns.reserve(stmt->alter_action_list_size());
  bool schema_changed = false;

  for (int i = 0; i < stmt->alter_action_list_size(); ++i) {
    const ::googlesql::ResolvedAlterAction* action = stmt->alter_action_list(i);
    if (action == nullptr) continue;
    absl::Status s = ApplyAlterAction(
        action, *target, &new_schema, &added_columns, &schema_changed);
    if (!s.ok()) return s;
  }

  if (!schema_changed) return absl::OkStatus();

  absl::StatusOr<std::vector<storage::Row>> rows =
      added_columns.empty()
          ? CopyRowsWithSchema(storage, *target, *existing, new_schema)
          : CopyRowsForAlter(storage, *target, added_columns);
  if (!rows.ok()) return rows.status();

  return RebuildTableWithColumns(
      storage, *target, new_schema, absl::MakeConstSpan(*rows));
}

}  // namespace internal
}  // namespace control
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
