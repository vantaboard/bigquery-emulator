#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "absl/status/status.h"
#include "absl/strings/str_cat.h"
#include "backend/engine/duckdb/duckdb_executor_attach_helpers.h"
#include "backend/engine/duckdb/duckdb_executor_internal.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace duckdb {
namespace internal {

absl::Status AttachStorageTableAt(::duckdb_connection conn,
                                  storage::Storage* storage,
                                  const catalog::StorageTable& table,
                                  absl::string_view quoted_table_name,
                                  std::optional<std::int64_t> as_of_ms,
                                  absl::string_view row_access_filter_sql,
                                  PhaseRecorder* phase_recorder) {
  const schema::TableSchema& schema = table.bq_schema();
  const storage::TableId& id = table.storage_table_id();

  absl::StatusOr<std::optional<std::string>> parquet_path =
      ResolveParquetSnapshotPath(storage, id, as_of_ms);
  if (!parquet_path.ok()) return parquet_path.status();

  if (parquet_path->has_value()) {
    return AttachParquetTableAt(conn,
                                schema,
                                quoted_table_name,
                                **parquet_path,
                                row_access_filter_sql,
                                phase_recorder);
  }

  const std::string columns = RenderColumnList(schema);
  absl::Status status = RunSqlNoResult(
      conn,
      absl::StrCat(
          "CREATE OR REPLACE TABLE ", quoted_table_name, " ", columns));
  if (!status.ok()) return status;

  absl::StatusOr<std::vector<storage::Row>> rows =
      ScanAllTableRows(storage, id, phase_recorder);
  if (!rows.ok()) return rows.status();

  if (rows->empty()) {
    return ApplyRowAccessFilterDelete(
        conn, quoted_table_name, row_access_filter_sql);
  }

  status = InsertScannedRowsAt(
      conn, table, schema, quoted_table_name, *rows, phase_recorder);
  if (!status.ok()) return status;
  return ApplyRowAccessFilterDelete(
      conn, quoted_table_name, row_access_filter_sql);
}

absl::Status AttachStorageTable(::duckdb_connection conn,
                                storage::Storage* storage,
                                const catalog::StorageTable& table,
                                std::optional<std::int64_t> as_of_ms,
                                absl::string_view row_access_filter_sql,
                                PhaseRecorder* phase_recorder) {
  return AttachStorageTableAt(conn,
                              storage,
                              table,
                              QuoteIdent(table.Name()),
                              as_of_ms,
                              row_access_filter_sql,
                              phase_recorder);
}

}  // namespace internal
}  // namespace duckdb
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
