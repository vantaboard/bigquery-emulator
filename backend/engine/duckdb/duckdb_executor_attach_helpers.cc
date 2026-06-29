#include "backend/engine/duckdb/duckdb_executor_attach_helpers.h"

#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>
#include <vector>

#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/string_view.h"
#include "absl/time/clock.h"
#include "absl/time/time.h"
#include "backend/catalog/storage_table.h"
#include "backend/engine/duckdb/duckdb_executor_internal.h"
#include "backend/engine/phase_recorder.h"
#include "backend/schema/schema.h"
#include "backend/storage/duckdb/duckdb_storage_internal.h"
#include "backend/storage/storage.h"
#include "duckdb.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace duckdb {
namespace internal {

absl::StatusOr<std::optional<std::string>> ResolveParquetSnapshotPath(
    storage::Storage* storage,
    const storage::TableId& id,
    std::optional<std::int64_t> as_of_ms) {
  if (as_of_ms.has_value()) {
    absl::StatusOr<std::optional<std::string>> path_or =
        storage->ParquetSnapshotPathAt(id, *as_of_ms);
    if (!path_or.ok()) return path_or.status();
    return *path_or;
  }
  return storage->ParquetSnapshotPath(id);
}

absl::Status AttachParquetTableAt(::duckdb_connection conn,
                                  const schema::TableSchema& schema,
                                  absl::string_view quoted_table_name,
                                  absl::string_view parquet_path,
                                  absl::string_view row_access_filter_sql,
                                  PhaseRecorder* phase_recorder) {
  const absl::Time attach_start = absl::Now();
  const std::string select_cols =
      storage::duckdb::internal::RenderColumnIdentList(schema);
  const std::string escaped = EscapeStringLiteralInner(parquet_path);
  const std::string columns = RenderColumnList(schema);
  const std::string table_name(quoted_table_name);
  std::string where_clause;
  if (!row_access_filter_sql.empty()) {
    where_clause = absl::StrCat(" WHERE ", row_access_filter_sql);
  }
  absl::Status status = RunSqlNoResult(
      conn, absl::StrCat("CREATE OR REPLACE TABLE ", table_name, " ", columns));
  if (!status.ok()) return status;
  status = RunSqlNoResult(conn,
                          absl::StrCat("INSERT INTO ",
                                       table_name,
                                       " SELECT ",
                                       select_cols,
                                       " FROM read_parquet('",
                                       escaped,
                                       "')",
                                       where_clause));
  RecordPhase(
      phase_recorder, "table_attach_parquet", absl::Now() - attach_start);
  return status;
}

absl::StatusOr<std::vector<storage::Row>> ScanAllTableRows(
    storage::Storage* storage,
    const storage::TableId& id,
    PhaseRecorder* phase_recorder) {
  const absl::Time scan_start = absl::Now();
  absl::StatusOr<std::unique_ptr<storage::RowIterator>> iter =
      storage->ScanRows(id);
  if (!iter.ok()) return iter.status();

  std::unique_ptr<storage::RowIterator> rows_iter = std::move(iter).value();
  std::vector<storage::Row> rows;
  storage::Row row;
  while (true) {
    absl::StatusOr<bool> has = rows_iter->Next(&row);
    if (!has.ok()) return has.status();
    if (!*has) break;
    rows.push_back(row);
  }
  RecordPhase(phase_recorder, "scan_rows", absl::Now() - scan_start);
  return rows;
}

absl::Status InsertScannedRowsAt(::duckdb_connection conn,
                                 const catalog::StorageTable& table,
                                 const schema::TableSchema& schema,
                                 absl::string_view quoted_table_name,
                                 const std::vector<storage::Row>& rows,
                                 PhaseRecorder* phase_recorder) {
  const absl::Time insert_start = absl::Now();
  std::string insert_sql;
  absl::StrAppend(&insert_sql, "INSERT INTO ", quoted_table_name, " VALUES ");
  const size_t ncols = schema.columns.size();
  for (size_t r = 0; r < rows.size(); ++r) {
    if (rows[r].cells.size() != ncols) {
      return absl::InvalidArgumentError(absl::StrCat("DuckDBEngine: row[",
                                                     r,
                                                     "] has ",
                                                     rows[r].cells.size(),
                                                     " cells but table '",
                                                     table.Name(),
                                                     "' has ",
                                                     ncols,
                                                     " columns"));
    }
    if (r > 0) absl::StrAppend(&insert_sql, ", ");
    absl::StrAppend(&insert_sql, "(");
    for (size_t c = 0; c < ncols; ++c) {
      if (c > 0) absl::StrAppend(&insert_sql, ", ");
      auto cell_or = RenderCellLiteral(rows[r].cells[c], schema.columns[c]);
      if (!cell_or.ok()) return cell_or.status();
      absl::StrAppend(&insert_sql, *cell_or);
    }
    absl::StrAppend(&insert_sql, ")");
  }
  absl::Status status = RunSqlNoResult(conn, insert_sql);
  RecordPhase(phase_recorder, "insert_render_exec", absl::Now() - insert_start);
  return status;
}

absl::Status ApplyRowAccessFilterDelete(
    ::duckdb_connection conn,
    absl::string_view quoted_table_name,
    absl::string_view row_access_filter_sql) {
  if (row_access_filter_sql.empty()) return absl::OkStatus();
  return RunSqlNoResult(conn,
                        absl::StrCat("DELETE FROM ",
                                     quoted_table_name,
                                     " WHERE NOT (",
                                     row_access_filter_sql,
                                     ")"));
}

}  // namespace internal
}  // namespace duckdb
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
