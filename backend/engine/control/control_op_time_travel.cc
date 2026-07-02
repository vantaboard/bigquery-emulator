#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/str_split.h"
#include "absl/strings/string_view.h"
#include "backend/catalog/storage_table.h"
#include "backend/engine/control/control_op_internal.h"
#include "backend/engine/coordinator/routine_rehydrate.h"
#include "backend/engine/coordinator/view_rehydrate.h"
#include "backend/engine/duckdb/duckdb_executor_time_travel.h"
#include "backend/schema/schema.h"
#include "backend/storage/storage.h"
#include "duckdb.h"
#include "googlesql/resolved_ast/resolved_ast.h"
#include "googlesql/resolved_ast/resolved_node_kind.pb.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace control {
namespace internal {

namespace {

const ::googlesql::ResolvedTableScan* UnwrapTableScan(
    const ::googlesql::ResolvedScan* scan) {
  if (scan == nullptr) return nullptr;
  if (scan->node_kind() == ::googlesql::RESOLVED_TABLE_SCAN) {
    return scan->GetAs<::googlesql::ResolvedTableScan>();
  }
  if (scan->node_kind() == ::googlesql::RESOLVED_FILTER_SCAN) {
    const auto* filter = scan->GetAs<::googlesql::ResolvedFilterScan>();
    if (filter == nullptr) return nullptr;
    return UnwrapTableScan(filter->input_scan());
  }
  return nullptr;
}

absl::StatusOr<std::optional<std::int64_t>> AsOfMsFromScan(
    const ::googlesql::ResolvedScan* scan) {
  const auto* table_scan = UnwrapTableScan(scan);
  if (table_scan == nullptr || table_scan->for_system_time_expr() == nullptr) {
    return std::nullopt;
  }
  absl::StatusOr<std::int64_t> as_of_ms =
      duckdb::internal::EvaluateForSystemTimeAsOfMs(
          *table_scan->for_system_time_expr());
  if (!as_of_ms.ok()) return as_of_ms.status();
  return *as_of_ms;
}

absl::StatusOr<const catalog::StorageTable*> StorageTableFromScan(
    const ::googlesql::ResolvedScan* scan) {
  const auto* table_scan = UnwrapTableScan(scan);
  if (table_scan == nullptr || table_scan->table() == nullptr) {
    return absl::InvalidArgumentError(
        "control op executor: clone source must be a base table scan");
  }
  const auto* storage_table =
      dynamic_cast<const catalog::StorageTable*>(table_scan->table());
  if (storage_table == nullptr) {
    return absl::FailedPreconditionError(
        absl::StrCat("control op executor: clone source '",
                     table_scan->table()->Name(),
                     "' is not a storage-backed table"));
  }
  return storage_table;
}

absl::Status MaterializeCloneIntoTarget(
    storage::Storage& storage,
    const storage::TableId& target,
    const catalog::StorageTable& source,
    std::optional<std::int64_t> as_of_ms,
    ::googlesql::ResolvedCreateStatement::CreateMode create_mode) {
  absl::StatusOr<schema::TableSchema> bq_schema =
      storage.GetSchema(source.storage_table_id());
  if (!bq_schema.ok()) return bq_schema.status();

  if (create_mode == ::googlesql::ResolvedCreateStatement::CREATE_OR_REPLACE) {
    absl::Status dropped = storage.DropTable(target);
    if (!dropped.ok() && dropped.code() != absl::StatusCode::kNotFound) {
      return dropped;
    }
  }

  absl::Status created =
      ApplyCreateMode(storage.CreateTable(target, *bq_schema), create_mode);
  if (!created.ok()) return created;

  ::duckdb_database db = nullptr;
  if (::duckdb_open(nullptr, &db) != ::DuckDBSuccess) {
    return absl::InternalError(
        "control op executor: duckdb_open(in-memory) failed");
  }
  ::duckdb_connection conn = nullptr;
  if (::duckdb_connect(db, &conn) != ::DuckDBSuccess) {
    ::duckdb_close(&db);
    return absl::InternalError("control op executor: duckdb_connect failed");
  }

  absl::Status attach = AttachStorageTableAt(
      conn, &storage, source, QuoteIdent(source.Name()), as_of_ms);
  if (!attach.ok()) {
    ::duckdb_disconnect(&conn);
    ::duckdb_close(&db);
    return attach;
  }

  absl::StatusOr<std::vector<storage::Row>> rows =
      DrainTableRows(conn, QuoteIdent(source.Name()), *bq_schema);
  ::duckdb_disconnect(&conn);
  ::duckdb_close(&db);
  if (!rows.ok()) return rows.status();
  if (rows->empty()) return absl::OkStatus();
  return storage.OverwriteRows(target, absl::MakeConstSpan(*rows));
}

absl::Status MaterializeCloneIntoExistingTarget(
    storage::Storage& storage,
    const storage::TableId& target,
    const catalog::StorageTable& source,
    std::optional<std::int64_t> as_of_ms) {
  absl::StatusOr<schema::TableSchema> target_schema = storage.GetSchema(target);
  if (!target_schema.ok()) return target_schema.status();
  absl::StatusOr<schema::TableSchema> source_schema =
      storage.GetSchema(source.storage_table_id());
  if (!source_schema.ok()) return source_schema.status();

  ::duckdb_database db = nullptr;
  if (::duckdb_open(nullptr, &db) != ::DuckDBSuccess) {
    return absl::InternalError(
        "control op executor: duckdb_open(in-memory) failed");
  }
  ::duckdb_connection conn = nullptr;
  if (::duckdb_connect(db, &conn) != ::DuckDBSuccess) {
    ::duckdb_close(&db);
    return absl::InternalError("control op executor: duckdb_connect failed");
  }

  absl::Status attach = AttachStorageTableAt(
      conn, &storage, source, QuoteIdent(source.Name()), as_of_ms);
  if (!attach.ok()) {
    ::duckdb_disconnect(&conn);
    ::duckdb_close(&db);
    return attach;
  }

  absl::StatusOr<std::vector<storage::Row>> rows =
      DrainTableRows(conn, QuoteIdent(source.Name()), *source_schema);
  ::duckdb_disconnect(&conn);
  ::duckdb_close(&db);
  if (!rows.ok()) return rows.status();
  if (rows->empty()) {
    return storage.OverwriteRows(target, absl::Span<const storage::Row>{});
  }
  return storage.OverwriteRows(target, absl::MakeConstSpan(*rows));
}

}  // namespace

absl::Status RunCreateTableClone(
    storage::Storage& storage,
    absl::string_view project_id,
    absl::string_view default_dataset_id,
    const ::googlesql::ResolvedCreateTableStmt* stmt) {
  if (stmt == nullptr) {
    return absl::InternalError(
        "ControlOpExecutor::ExecuteDdl: CREATE TABLE CLONE has null stmt");
  }
  if (stmt->clone_from() == nullptr) {
    return absl::InvalidArgumentError(
        "control op executor: CREATE TABLE CLONE missing clone_from scan");
  }
  absl::StatusOr<storage::TableId> target =
      NamePathToTableId(stmt->name_path(), project_id, default_dataset_id);
  if (!target.ok()) return target.status();
  if (auto ds =
          EnsureDatasetExists(storage, target->project_id, target->dataset_id);
      !ds.ok()) {
    return ds;
  }
  absl::StatusOr<const catalog::StorageTable*> source =
      StorageTableFromScan(stmt->clone_from());
  if (!source.ok()) return source.status();
  absl::StatusOr<std::optional<std::int64_t>> as_of_ms =
      AsOfMsFromScan(stmt->clone_from());
  if (!as_of_ms.ok()) return as_of_ms.status();
  return MaterializeCloneIntoTarget(
      storage, *target, **source, *as_of_ms, stmt->create_mode());
}

absl::Status RunCreateSnapshotTable(
    storage::Storage& storage,
    absl::string_view project_id,
    absl::string_view default_dataset_id,
    const ::googlesql::ResolvedCreateSnapshotTableStmt* stmt) {
  if (stmt == nullptr) {
    return absl::InternalError(
        "ControlOpExecutor::ExecuteDdl: CREATE SNAPSHOT TABLE has null stmt");
  }
  if (stmt->clone_from() == nullptr) {
    return absl::InvalidArgumentError(
        "control op executor: CREATE SNAPSHOT TABLE missing clone_from scan");
  }
  absl::StatusOr<storage::TableId> target =
      NamePathToTableId(stmt->name_path(), project_id, default_dataset_id);
  if (!target.ok()) return target.status();
  if (auto ds =
          EnsureDatasetExists(storage, target->project_id, target->dataset_id);
      !ds.ok()) {
    return ds;
  }
  absl::StatusOr<const catalog::StorageTable*> source =
      StorageTableFromScan(stmt->clone_from());
  if (!source.ok()) return source.status();
  absl::StatusOr<std::optional<std::int64_t>> as_of_ms =
      AsOfMsFromScan(stmt->clone_from());
  if (!as_of_ms.ok()) return as_of_ms.status();
  return MaterializeCloneIntoTarget(
      storage, *target, **source, *as_of_ms, stmt->create_mode());
}

absl::Status RunCloneData(storage::Storage& storage,
                          absl::string_view project_id,
                          absl::string_view default_dataset_id,
                          const ::googlesql::ResolvedCloneDataStmt* stmt) {
  if (stmt == nullptr) {
    return absl::InternalError(
        "ControlOpExecutor::ExecuteDdl: CLONE DATA has null stmt");
  }
  if (stmt->target_table() == nullptr || stmt->clone_from() == nullptr) {
    return absl::InvalidArgumentError(
        "control op executor: CLONE DATA missing target or source scan");
  }
  const auto* target_table = stmt->target_table();
  if (target_table->table() == nullptr) {
    return absl::InvalidArgumentError(
        "control op executor: CLONE DATA target_table has no table");
  }
  const auto* target_storage =
      dynamic_cast<const catalog::StorageTable*>(target_table->table());
  if (target_storage == nullptr) {
    return absl::FailedPreconditionError(
        "control op executor: CLONE DATA target is not storage-backed");
  }
  absl::StatusOr<const catalog::StorageTable*> source =
      StorageTableFromScan(stmt->clone_from());
  if (!source.ok()) return source.status();
  absl::StatusOr<std::optional<std::int64_t>> as_of_ms =
      AsOfMsFromScan(stmt->clone_from());
  if (!as_of_ms.ok()) return as_of_ms.status();
  (void)project_id;
  (void)default_dataset_id;
  return MaterializeCloneIntoExistingTarget(
      storage, target_storage->storage_table_id(), **source, *as_of_ms);
}

namespace {

absl::StatusOr<storage::DatasetId> NamePathToDatasetId(
    const std::vector<std::string>& name_path,
    absl::string_view default_project_id) {
  std::vector<std::string> segments = name_path;
  if (segments.size() == 1 && absl::StrContains(segments[0], '.')) {
    segments = absl::StrSplit(segments[0], '.');
  }
  if (segments.size() == 1) {
    return storage::DatasetId{std::string(default_project_id), segments[0]};
  }
  if (segments.size() == 2) {
    return storage::DatasetId{segments[0], segments[1]};
  }
  return absl::InvalidArgumentError(absl::StrCat(
      "control op executor: DDL schema target must be <dataset> or "
      "<project>.<dataset>; got ",
      segments.size(),
      " segments"));
}

absl::Status RehydrateDatasetCatalog(storage::Storage& storage,
                                     const storage::DatasetId& id) {
  auto routines_or = storage.ListRoutines(id);
  if (!routines_or.ok()) return routines_or.status();
  for (const storage::RoutineRecord& rec : *routines_or) {
    if (rec.is_temp) continue;
    absl::Status reg =
        engine::coordinator::RehydrateRoutineRecord(&storage, rec);
    if (!reg.ok()) return reg;
  }
  auto views_or = storage.ListAllViews();
  if (!views_or.ok()) return views_or.status();
  for (const storage::ViewRecord& rec : *views_or) {
    if (rec.id.project_id != id.project_id ||
        rec.id.dataset_id != id.dataset_id) {
      continue;
    }
    absl::Status reg = engine::coordinator::RehydrateViewRecord(&storage, rec);
    if (!reg.ok()) return reg;
  }
  return absl::OkStatus();
}

}  // namespace

absl::Status RunDropSchema(storage::Storage& storage,
                           absl::string_view project_id,
                           const ::googlesql::ResolvedDropStmt* stmt) {
  if (stmt == nullptr) {
    return absl::InternalError(
        "ControlOpExecutor::ExecuteDdl: DROP SCHEMA has null stmt");
  }
  absl::StatusOr<storage::DatasetId> target =
      NamePathToDatasetId(stmt->name_path(), project_id);
  if (!target.ok()) return target.status();
  const bool cascade =
      stmt->drop_mode() == ::googlesql::ResolvedDropStmt::CASCADE;
  if (!cascade) {
    auto tables_or = storage.ListTables(*target);
    if (!tables_or.ok()) return tables_or.status();
    if (!tables_or->empty()) {
      return absl::FailedPreconditionError(
          absl::StrCat("dataset is not empty: ",
                       target->project_id,
                       ".",
                       target->dataset_id,
                       " (use CASCADE to drop with tables)"));
    }
  }
  absl::Status dropped = storage.DropDataset(*target, /*delete_contents=*/true);
  if (!dropped.ok() && stmt->is_if_exists() &&
      dropped.code() == absl::StatusCode::kNotFound) {
    return absl::OkStatus();
  }
  return dropped;
}

absl::Status RunUndrop(storage::Storage& storage,
                       absl::string_view project_id,
                       absl::string_view default_dataset_id,
                       const ::googlesql::ResolvedUndropStmt* stmt) {
  if (stmt == nullptr) {
    return absl::InternalError(
        "ControlOpExecutor::ExecuteDdl: UNDROP has null stmt");
  }
  const absl::string_view kind = stmt->schema_object_kind();
  if (kind != "SCHEMA") {
    return absl::UnimplementedError(absl::StrCat(
        "control op executor: UNDROP ",
        kind,
        " is not a BigQuery statement (only UNDROP SCHEMA is supported)"));
  }
  (void)default_dataset_id;
  absl::StatusOr<storage::DatasetId> target =
      NamePathToDatasetId(stmt->name_path(), project_id);
  if (!target.ok()) return target.status();
  absl::Status restored = storage.RestoreDataset(*target);
  if (restored.ok()) {
    absl::Status rehydrated = RehydrateDatasetCatalog(storage, *target);
    if (!rehydrated.ok()) return rehydrated;
    return absl::OkStatus();
  }
  if (stmt->is_if_not_exists()) {
    if (restored.code() == absl::StatusCode::kAlreadyExists) {
      return absl::OkStatus();
    }
    if (restored.code() == absl::StatusCode::kNotFound) {
      auto datasets_or = storage.ListDatasets(project_id);
      if (datasets_or.ok()) {
        for (const storage::DatasetId& id : *datasets_or) {
          if (id.project_id == target->project_id &&
              id.dataset_id == target->dataset_id) {
            return absl::OkStatus();
          }
        }
      }
    }
  }
  return restored;
}

absl::Status RunDropSnapshotTable(
    storage::Storage& storage,
    absl::string_view project_id,
    absl::string_view default_dataset_id,
    const ::googlesql::ResolvedDropSnapshotTableStmt* stmt) {
  if (stmt == nullptr) {
    return absl::InternalError(
        "ControlOpExecutor::ExecuteDdl: DROP SNAPSHOT TABLE has null stmt");
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

}  // namespace internal
}  // namespace control
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
