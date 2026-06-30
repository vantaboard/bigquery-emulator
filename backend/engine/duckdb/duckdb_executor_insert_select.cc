#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "backend/engine/duckdb/duckdb_executor.h"
#include "backend/engine/duckdb/duckdb_executor_insert_select_helpers.h"
#include "backend/engine/duckdb/duckdb_executor_internal.h"
#include "backend/storage/storage.h"
#include "googlesql/resolved_ast/resolved_ast.h"
#include "proto/emulator.pb.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace duckdb {
namespace internal {

absl::StatusOr<DmlStats> RunInsertSelect(
    const QueryRequest& request,
    storage::Storage* storage,
    const ::googlesql::ResolvedInsertStmt& insert) {
  absl::StatusOr<InsertSelectTarget> target =
      ValidateInsertSelectTarget(insert);
  if (!target.ok()) return target.status();

  internal::TableScanCollector collector;
  absl::StatusOr<std::string> select_sql =
      PrepareInsertSelectSql(request, insert, &collector);
  if (!select_sql.ok()) return select_sql.status();

  absl::StatusOr<InsertSelectConnection> connection =
      OpenInsertSelectConnection();
  if (!connection.ok()) return connection.status();

  absl::Status attach =
      AttachInsertSelectSourceTables(connection->conn, storage, collector);
  if (!attach.ok()) return attach;

  absl::StatusOr<std::vector<storage::Row>> selected =
      ExecuteInsertSelectQuery(&*connection, *select_sql, insert);
  if (!selected.ok()) return selected.status();

  absl::StatusOr<std::vector<storage::Row>> rows =
      RemapInsertSelectRows(insert, *target->schema, *selected);
  if (!rows.ok()) return rows.status();

  if (rows->empty()) {
    return DmlStats{};
  }
  absl::Status appended = storage->AppendRows(target->table_id, *rows);
  if (!appended.ok()) return appended;

  DmlStats stats;
  stats.inserted_row_count = static_cast<int64_t>(rows->size());
  return stats;
}

}  // namespace internal
}  // namespace duckdb
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
