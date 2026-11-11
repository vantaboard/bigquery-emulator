#include <cmath>
#include <memory>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "absl/strings/ascii.h"
#include "absl/strings/match.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/str_join.h"
#include "absl/strings/str_replace.h"
#include "absl/strings/str_split.h"
#include "absl/strings/string_view.h"
#include "absl/types/span.h"
#include "backend/catalog/create_function_util.h"
#include "backend/catalog/googlesql_catalog.h"
#include "backend/catalog/storage_table.h"
#include "backend/catalog/udf_registry.h"
#include "backend/catalog/view_persistence.h"
#include "backend/catalog/view_registry.h"
#include "backend/engine/control/control_op_internal.h"
#include "backend/engine/duckdb/arrow_to_bq.h"
#include "backend/engine/duckdb/transpiler/transpiler.h"
#include "backend/engine/duckdb/udf/registrar.h"
#include "backend/engine/engine.h"
#include "backend/engine/semantic/value.h"
#include "backend/schema/googlesql_to_bq.h"
#include "backend/schema/schema.h"
#include "backend/storage/storage.h"
#include "duckdb.h"
#include "googlesql/public/catalog.h"
#include "googlesql/resolved_ast/resolved_ast.h"
#include "googlesql/resolved_ast/resolved_ast_visitor.h"
#include "googlesql/resolved_ast/resolved_node_kind.pb.h"
#include "proto/emulator.pb.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace control {
namespace internal {

// --- Name-path resolution -------------------------------------------------

// Resolve `name_path` (as produced by `ResolvedCreateStatement::
// name_path()` / `ResolvedDropStmt::name_path()` etc.) to a
// `storage::TableId` under `default_project_id`. One-segment paths
// use `default_dataset_id` when set (BigQuery `defaultDataset` on
// jobs.query). Two-segment paths default the project; three-segment
// paths override it.
//
// BigQuery accepts a fully backtick-quoted path -- `CREATE TABLE
// `ds.t`` or `CREATE TABLE `proj.ds.t`` -- where the analyzer hands
// back the whole dotted string as a *single* name-path segment. The
// read path (GoogleSqlCatalog::FindTable) already splits these so a
// `SELECT * FROM `ds.t`` resolves to dataset/table; DDL targets must
// split identically or `CREATE OR REPLACE TABLE `ds.t`` is rejected as
// a bogus single-segment name. Flatten a single dotted segment into
// its component identifiers before applying the 1/2/3-segment rules.
absl::StatusOr<storage::TableId> NamePathToTableId(
    const std::vector<std::string>& name_path,
    absl::string_view default_project_id,
    absl::string_view default_dataset_id) {
  std::vector<std::string> segments = name_path;
  if (segments.size() == 1 && absl::StrContains(segments[0], '.')) {
    segments = absl::StrSplit(segments[0], '.');
  }
  if (segments.size() == 1) {
    if (default_dataset_id.empty()) {
      return absl::InvalidArgumentError(absl::StrCat(
          "control op executor: DDL target name must be <dataset>.<table> or "
          "<project>.<dataset>.<table>; got 1 segment (",
          segments[0],
          ") with no defaultDataset in the query request"));
    }
    return storage::TableId{std::string(default_project_id),
                            std::string(default_dataset_id),
                            segments[0]};
  }
  if (segments.size() == 2) {
    return storage::TableId{
        std::string(default_project_id), segments[0], segments[1]};
  }
  if (segments.size() == 3) {
    return storage::TableId{segments[0], segments[1], segments[2]};
  }
  return absl::InvalidArgumentError(absl::StrCat(
      "control op executor: DDL target name must be <dataset>.<table> or "
      "<project>.<dataset>.<table>; got ",
      segments.size(),
      " segments"));
}

// Create the dataset directory if it does not exist yet. query port
// port tests issue `CREATE TABLE ds.t` without a prior datasets.insert.
absl::Status EnsureDatasetExists(storage::Storage& storage,
                                 absl::string_view project_id,
                                 absl::string_view dataset_id) {
  if (dataset_id.empty()) {
    return absl::InvalidArgumentError(
        "EnsureDatasetExists: dataset_id must be non-empty");
  }
  storage::DatasetId id{std::string(project_id), std::string(dataset_id)};
  absl::Status created = storage.CreateDataset(id, "US");
  if (created.ok()) return absl::OkStatus();
  if (created.code() == absl::StatusCode::kAlreadyExists) {
    return absl::OkStatus();
  }
  return created;
}

// --- Schema helpers -------------------------------------------------------

void ApplyAnnotations(const ::googlesql::ResolvedColumnDefinition* def,
                      v1::FieldSchema* field) {
  if (def == nullptr) return;
  if (field->mode() == "REPEATED") return;
  const ::googlesql::ResolvedColumnAnnotations* ann = def->annotations();
  if (ann != nullptr && ann->not_null()) {
    field->set_mode("REQUIRED");
  }
}

absl::StatusOr<schema::TableSchema> ColumnDefinitionListToTableSchema(
    const std::vector<
        std::unique_ptr<const ::googlesql::ResolvedColumnDefinition>>&
        column_definition_list) {
  v1::TableSchema proto;
  for (const auto& def : column_definition_list) {
    if (def == nullptr) {
      return absl::InvalidArgumentError(
          "control op executor: column_definition_list has a null entry");
    }
    v1::FieldSchema* field = proto.add_fields();
    absl::Status s =
        backend::schema::TypeToFieldSchema(def->type(), def->name(), field);
    if (!s.ok()) return s;
    ApplyAnnotations(def.get(), field);
  }
  return backend::schema::TableSchemaFromProto(proto);
}

// Apply CREATE-mode semantics on top of a `Storage::CreateTable`
// status. CREATE_IF_NOT_EXISTS swallows ALREADY_EXISTS; the other
// modes propagate it.
absl::Status ApplyCreateMode(
    absl::Status existing_status,
    ::googlesql::ResolvedCreateStatement::CreateMode create_mode) {
  if (existing_status.ok()) return absl::OkStatus();
  if (create_mode ==
          ::googlesql::ResolvedCreateStatement::CREATE_IF_NOT_EXISTS &&
      existing_status.code() == absl::StatusCode::kAlreadyExists) {
    return absl::OkStatus();
  }
  return existing_status;
}

// --- CREATE TABLE handler -------------------------------------------------
//
// statementType: `CREATE_TABLE` per BigQuery REST documentation
// (`Job.statistics.query.statementType`).
absl::Status RunCreateTable(storage::Storage& storage,
                            absl::string_view project_id,
                            absl::string_view default_dataset_id,
                            const ::googlesql::ResolvedCreateTableStmt* stmt) {
  if (stmt == nullptr) {
    return absl::InternalError(
        "ControlOpExecutor::ExecuteDdl: CREATE TABLE has null resolved "
        "statement");
  }
  if (stmt->clone_from() != nullptr) {
    return RunCreateTableClone(storage, project_id, default_dataset_id, stmt);
  }
  absl::StatusOr<storage::TableId> target =
      NamePathToTableId(stmt->name_path(), project_id, default_dataset_id);
  if (!target.ok()) return target.status();
  if (auto ds =
          EnsureDatasetExists(storage, target->project_id, target->dataset_id);
      !ds.ok()) {
    return ds;
  }
  absl::StatusOr<schema::TableSchema> bq_schema =
      ColumnDefinitionListToTableSchema(stmt->column_definition_list());
  if (!bq_schema.ok()) return bq_schema.status();

  if (stmt->create_mode() ==
      ::googlesql::ResolvedCreateStatement::CREATE_OR_REPLACE) {
    absl::Status dropped = storage.DropTable(*target);
    if (!dropped.ok() && dropped.code() != absl::StatusCode::kNotFound) {
      return dropped;
    }
  }
  return ApplyCreateMode(storage.CreateTable(*target, *bq_schema),
                         stmt->create_mode());
}

// --- DROP TABLE handler ---------------------------------------------------
// statementType: `DROP_TABLE`. `DROP VIEW` / `DROP SCHEMA` delegate to
// sibling handlers; other `DROP <kind>` forms surface UNIMPLEMENTED.
absl::Status RunDropTable(storage::Storage& storage,
                          absl::string_view project_id,
                          absl::string_view default_dataset_id,
                          const ::googlesql::ResolvedDropStmt* stmt) {
  if (stmt == nullptr) {
    return absl::InternalError(
        "ControlOpExecutor::ExecuteDdl: DROP has null resolved statement");
  }
  if (stmt->object_type() == "VIEW") {
    if (stmt->name_path().empty()) {
      return absl::InvalidArgumentError(
          "control op executor: DROP VIEW has empty name_path");
    }
    const storage::ViewId vid = catalog::ViewIdFromNamePath(
        stmt->name_path(), project_id, default_dataset_id);
    absl::Status dropped = catalog::DropProjectView(project_id, vid.view_id);
    if (!dropped.ok() && stmt->is_if_exists() &&
        dropped.code() == absl::StatusCode::kNotFound) {
      return absl::OkStatus();
    }
    if (!dropped.ok()) return dropped;
    return catalog::DeletePersistedView(&storage, vid);
  }
  if (stmt->object_type() == "SCHEMA") {
    return RunDropSchema(storage, project_id, stmt);
  }
  if (stmt->object_type() != "TABLE") {
    return absl::UnimplementedError(
        absl::StrCat("control op executor: DROP ",
                     stmt->object_type(),
                     " is not implemented yet (only DROP TABLE / DROP VIEW are "
                     "supported); see docs/ENGINE_POLICY.md"));
  }
  absl::StatusOr<storage::TableId> target =
      NamePathToTableId(stmt->name_path(), project_id, default_dataset_id);
  if (!target.ok()) return target.status();
  absl::Status dropped = storage.DropTable(*target);
  if (!dropped.ok() && stmt->is_if_exists() &&
      dropped.code() == absl::StatusCode::kNotFound) {
    return absl::OkStatus();
  }
  return dropped;
}

absl::StatusOr<int64_t> RunTruncateTable(
    storage::Storage& storage, const ::googlesql::ResolvedTruncateStmt* stmt) {
  if (stmt == nullptr) {
    return absl::InternalError(
        "ControlOpExecutor::ExecuteDdl: TRUNCATE has null resolved statement");
  }
  if (stmt->where_expr() != nullptr) {
    return absl::UnimplementedError(
        "control op executor: TRUNCATE TABLE ... WHERE is not implemented yet");
  }
  if (stmt->table_scan() == nullptr || stmt->table_scan()->table() == nullptr) {
    return absl::InvalidArgumentError(
        "control op executor: TRUNCATE TABLE has no resolved target");
  }
  const auto* storage_table =
      dynamic_cast<const catalog::StorageTable*>(stmt->table_scan()->table());
  if (storage_table == nullptr) {
    return absl::FailedPreconditionError(
        absl::StrCat("control op executor: TRUNCATE target '",
                     stmt->table_scan()->table()->FullName(),
                     "' is not backed by storage"));
  }
  const storage::TableId& id = storage_table->storage_table_id();
  absl::StatusOr<schema::TableSchema> schema = storage.GetSchema(id);
  if (!schema.ok()) return schema.status();
  absl::StatusOr<std::int64_t> before = storage.CountRows(id);
  if (!before.ok()) return before.status();
  absl::Status cleared = storage.OverwriteRows(id, {});
  if (!cleared.ok()) return cleared;
  return *before;
}

// --- ANALYZE handler ------------------------------------------------------
//
// statementType: `ANALYZE`. The emulator's storage layer does not
// keep optimizer statistics, so ANALYZE is a no-op metadata refresh:
// we validate that every named table exists (NOT_FOUND otherwise)
// and return OK. Callers see the operation succeed without any
// observable side-effect, which matches the BigQuery posture where
}  // namespace internal
}  // namespace control
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
