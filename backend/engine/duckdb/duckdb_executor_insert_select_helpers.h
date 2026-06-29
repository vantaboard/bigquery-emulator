#ifndef BIGQUERY_EMULATOR_BACKEND_ENGINE_DUCKDB_DUCKDB_EXECUTOR_INSERT_SELECT_HELPERS_H_
#define BIGQUERY_EMULATOR_BACKEND_ENGINE_DUCKDB_DUCKDB_EXECUTOR_INSERT_SELECT_HELPERS_H_

#include <string>
#include <vector>

#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "backend/catalog/storage_table.h"
#include "backend/engine/duckdb/duckdb_executor_internal.h"
#include "backend/engine/engine.h"
#include "backend/schema/schema.h"
#include "backend/storage/storage.h"
#include "duckdb.h"
#include "googlesql/resolved_ast/resolved_ast.h"
#include "proto/emulator.pb.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace duckdb {
namespace internal {

struct InsertSelectTarget {
  const catalog::StorageTable* table = nullptr;
  storage::TableId table_id;
  const schema::TableSchema* schema = nullptr;
};

absl::StatusOr<InsertSelectTarget> ValidateInsertSelectTarget(
    const ::googlesql::ResolvedInsertStmt& insert);

absl::StatusOr<std::string> PrepareInsertSelectSql(
    const QueryRequest& request,
    const ::googlesql::ResolvedInsertStmt& insert,
    TableScanCollector* collector);

struct InsertSelectConnection {
  ::duckdb_database db = nullptr;
  ::duckdb_connection conn = nullptr;

  ~InsertSelectConnection();
};

absl::StatusOr<InsertSelectConnection> OpenInsertSelectConnection();

absl::Status AttachInsertSelectSourceTables(
    ::duckdb_connection conn,
    storage::Storage* storage,
    const TableScanCollector& collector);

absl::StatusOr<std::vector<storage::Row>> ExecuteInsertSelectQuery(
    InsertSelectConnection* connection,
    absl::string_view select_sql,
    const ::googlesql::ResolvedInsertStmt& insert);

absl::StatusOr<std::vector<storage::Row>> RemapInsertSelectRows(
    const ::googlesql::ResolvedInsertStmt& insert,
    const schema::TableSchema& full_schema,
    const std::vector<storage::Row>& selected);

}  // namespace internal
}  // namespace duckdb
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator

#endif  // BIGQUERY_EMULATOR_BACKEND_ENGINE_DUCKDB_DUCKDB_EXECUTOR_INSERT_SELECT_HELPERS_H_
