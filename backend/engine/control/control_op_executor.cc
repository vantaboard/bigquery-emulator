

#include "googlesql/public/catalog.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace control {

namespace {

// Returns the libduckdb C-API version. Calling it once at executor
// construction pulls libduckdb's symbol table into the link line so
// the binary's DT_NEEDED list keeps libduckdb.so even under
// `--as-needed`. Mirrors the same anchor `DuckDbExecutor` carries.
const char* DuckDBLibraryVersion() {
  return ::duckdb_library_version();
}

}  // namespace

ControlOpExecutor::ControlOpExecutor(storage::Storage* storage)
    : storage_(storage) {
  (void)DuckDBLibraryVersion();
}

ControlOpExecutor::~ControlOpExecutor() = default;

absl::StatusOr<std::unique_ptr<RowSource>> ControlOpExecutor::ExecuteQuery(
    const QueryRequest& request,
    const ::googlesql::ResolvedStatement& stmt,
    ::googlesql::Catalog* catalog) {
  (void)request;
  (void)catalog;
  return absl::InvalidArgumentError(absl::StrCat(
      "ControlOpExecutor::ExecuteQuery: control-op statements never produce "
      "a row stream; coordinator routed ",
      stmt.node_kind_string(),
      " through the wrong dispatch surface (control_op statements use "
      "ExecuteDdl)"));
}

absl::StatusOr<DmlResult> ControlOpExecutor::ExecuteDml(
    const QueryRequest& request,
    const ::googlesql::ResolvedStatement& stmt,
    ::googlesql::Catalog* catalog) {
  (void)request;
  (void)catalog;
  return absl::InvalidArgumentError(absl::StrCat(
      "ControlOpExecutor::ExecuteDml: control-op statements never produce a "
      "DML stats summary; coordinator routed ",
      stmt.node_kind_string(),
      " through the wrong dispatch surface (control_op statements use "
      "ExecuteDdl)"));
}

absl::Status ControlOpExecutor::ExecuteDdl(
    const QueryRequest& request,
    const ::googlesql::ResolvedStatement& stmt,
    ::googlesql::Catalog* catalog) {
  (void)catalog;  // analysis already happened on the coordinator.
  if (storage_ == nullptr) {
    return absl::FailedPreconditionError(
        "ControlOpExecutor::ExecuteDdl: storage backend is not configured");
  }

  const absl::string_view project_id = request.project_id;
  switch (stmt.node_kind()) {
    case ::googlesql::RESOLVED_CREATE_TABLE_STMT:
      return internal::RunCreateTable(
          *storage_,
          project_id,
          request.default_dataset_id,
          stmt.GetAs<::googlesql::ResolvedCreateTableStmt>());
    case ::googlesql::RESOLVED_CREATE_TABLE_AS_SELECT_STMT:
      return internal::RunCreateTableAsSelect(
          *storage_,
          project_id,
          request,
          stmt.GetAs<::googlesql::ResolvedCreateTableAsSelectStmt>(),
          &stmt);
    case ::googlesql::RESOLVED_DROP_STMT:
      return internal::RunDropTable(
          *storage_,
          project_id,
          request.default_dataset_id,
          stmt.GetAs<::googlesql::ResolvedDropStmt>());
    case ::googlesql::RESOLVED_TRUNCATE_STMT: {
      absl::StatusOr<int64_t> deleted = internal::RunTruncateTable(
          *storage_, stmt.GetAs<::googlesql::ResolvedTruncateStmt>());
      if (!deleted.ok()) return deleted.status();
      return absl::OkStatus();
    }
    // Deferred control-op shapes. Each handler returns
    // UNIMPLEMENTED with a focused message that names the missing
    // infrastructure. See docs/ENGINE_POLICY.md for deferred
    // control-op families.
    case ::googlesql::RESOLVED_CREATE_VIEW_STMT:
      // Registered in `LocalCoordinatorEngine::ExecuteDdl` via
      // `view_registry` (SQLView inlining at analyze time).
      return absl::OkStatus();
    case ::googlesql::RESOLVED_ALTER_TABLE_STMT:
      return internal::RunAlterTable(
          *storage_,
          project_id,
          request.default_dataset_id,
          stmt.GetAs<::googlesql::ResolvedAlterTableStmt>());
    case ::googlesql::RESOLVED_CREATE_MATERIALIZED_VIEW_STMT:
      return internal::RunCreateMaterializedView(
          *storage_,
          project_id,
          request,
          stmt.GetAs<::googlesql::ResolvedCreateMaterializedViewStmt>(),
          stmt);
    case ::googlesql::RESOLVED_CREATE_FUNCTION_STMT:
      // Registered in `LocalCoordinatorEngine::ExecuteDdl` with pinned
      // `AnalyzerOutput` so UDF type pointers stay valid across RPCs.
      return absl::OkStatus();
    case ::googlesql::RESOLVED_CREATE_TABLE_FUNCTION_STMT:
      // Registered in `LocalCoordinatorEngine::ExecuteDdl` via
      // `tvf_registry`.
      return absl::OkStatus();
    case ::googlesql::RESOLVED_CREATE_PROCEDURE_STMT:
      // Registered in `LocalCoordinatorEngine::ExecuteDdl` via
      // `procedure_registry`.
      return absl::OkStatus();
    case ::googlesql::RESOLVED_DROP_FUNCTION_STMT: {
      const auto* drop_fn = stmt.GetAs<::googlesql::ResolvedDropFunctionStmt>();
      if (drop_fn == nullptr || drop_fn->name_path().empty()) {
        return absl::InternalError(
            "control op executor: DROP FUNCTION has null stmt or empty "
            "name_path");
      }
      const storage::RoutineId rid = catalog::RoutineIdFromNamePath(
          drop_fn->name_path(), project_id, request.default_dataset_id);
      const std::string routine_name = drop_fn->name_path().back();
      absl::Status dropped =
          catalog::DropProjectFunction(project_id, routine_name);
      if (!dropped.ok()) {
        dropped = catalog::DropProjectProcedure(project_id, routine_name);
      }
      if (!dropped.ok() && drop_fn->is_if_exists() &&
          dropped.code() == absl::StatusCode::kNotFound) {
        return absl::OkStatus();
      }
      if (!dropped.ok()) return dropped;
      return catalog::DeletePersistedRoutine(storage_, rid);
    }
    case ::googlesql::RESOLVED_DROP_TABLE_FUNCTION_STMT: {
      const auto* drop_tvf =
          stmt.GetAs<::googlesql::ResolvedDropTableFunctionStmt>();
      if (drop_tvf == nullptr || drop_tvf->name_path().empty()) {
        return absl::InternalError(
            "control op executor: DROP TABLE FUNCTION has null stmt or "
            "empty name_path");
      }
      const storage::RoutineId rid = catalog::RoutineIdFromNamePath(
          drop_tvf->name_path(), project_id, request.default_dataset_id);
      absl::Status dropped =
          catalog::DropProjectTvf(project_id, drop_tvf->name_path().back());
      if (!dropped.ok() && drop_tvf->is_if_exists() &&
          dropped.code() == absl::StatusCode::kNotFound) {
        return absl::OkStatus();
      }
      if (!dropped.ok()) return dropped;
      return catalog::DeletePersistedRoutine(storage_, rid);
    }
    case ::googlesql::RESOLVED_AUX_LOAD_DATA_STMT:
      return internal::RunLoadData(
          *storage_,
          project_id,
          request.default_dataset_id,
          stmt.GetAs<::googlesql::ResolvedAuxLoadDataStmt>());
    case ::googlesql::RESOLVED_EXPORT_DATA_STMT:
      return internal::RunExportData(
          *storage_,
          request,
          stmt.GetAs<::googlesql::ResolvedExportDataStmt>(),
          stmt);
    case ::googlesql::RESOLVED_CREATE_SNAPSHOT_TABLE_STMT:
      return internal::RunCreateSnapshotTable(
          *storage_,
          project_id,
          request.default_dataset_id,
          stmt.GetAs<::googlesql::ResolvedCreateSnapshotTableStmt>());
    case ::googlesql::RESOLVED_CLONE_DATA_STMT:
      return internal::RunCloneData(
          *storage_,
          project_id,
          request.default_dataset_id,
          stmt.GetAs<::googlesql::ResolvedCloneDataStmt>());
    case ::googlesql::RESOLVED_UNDROP_STMT:
      return internal::RunUndrop(*storage_,
                                 project_id,
                                 request.default_dataset_id,
                                 stmt.GetAs<::googlesql::ResolvedUndropStmt>());
    case ::googlesql::RESOLVED_DROP_SNAPSHOT_TABLE_STMT:
      return internal::RunDropSnapshotTable(
          *storage_,
          project_id,
          request.default_dataset_id,
          stmt.GetAs<::googlesql::ResolvedDropSnapshotTableStmt>());
    default:
      return absl::UnimplementedError(
          absl::StrCat("control op executor: ExecuteDdl does not implement ",
                       stmt.node_kind_string(),
                       " yet; the row carries disposition=control_op in "
                       "node_dispositions.yaml but no handler in "
                       "control_op_executor.cc dispatches it"));
  }
}

}  // namespace control
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
