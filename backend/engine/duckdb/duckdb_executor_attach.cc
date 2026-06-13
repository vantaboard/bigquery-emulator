#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "absl/strings/str_cat.h"
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

absl::Status AttachStorageTableAt(::duckdb_connection conn,
                                  storage::Storage* storage,
                                  const catalog::StorageTable& table,
                                  absl::string_view quoted_table_name,
                                  std::optional<std::int64_t> as_of_ms,
                                  absl::string_view row_access_filter_sql,
                                  PhaseRecorder* phase_recorder) {
  const schema::TableSchema& schema = table.bq_schema();
  const std::string table_name(quoted_table_name);
  const storage::TableId& id = table.storage_table_id();

  std::optional<std::string> parquet_path;
  if (as_of_ms.has_value()) {
    absl::StatusOr<std::optional<std::string>> path_or =
        storage->ParquetSnapshotPathAt(id, *as_of_ms);
    if (!path_or.ok()) return path_or.status();
    parquet_path = std::move(*path_or);
  } else {
    parquet_path = storage->ParquetSnapshotPath(id);
  }

  if (parquet_path) {
    const absl::Time attach_start = absl::Now();
    const std::string select_cols =
        storage::duckdb::internal::RenderColumnIdentList(schema);
    const std::string escaped = EscapeStringLiteralInner(*parquet_path);
    const std::string columns = RenderColumnList(schema);
    std::string where_clause;
    if (!row_access_filter_sql.empty()) {
      where_clause = absl::StrCat(" WHERE ", row_access_filter_sql);
    }
    absl::Status status = RunSqlNoResult(
        conn,
        absl::StrCat("CREATE OR REPLACE TABLE ", table_name, " ", columns));
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

  const std::string columns = RenderColumnList(schema);

  absl::Status status = RunSqlNoResult(
      conn, absl::StrCat("CREATE OR REPLACE TABLE ", table_name, " ", columns));
  if (!status.ok()) return status;

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
  if (rows.empty()) {
    if (!row_access_filter_sql.empty()) {
      return RunSqlNoResult(conn,
                            absl::StrCat("DELETE FROM ",
                                         table_name,
                                         " WHERE NOT (",
                                         row_access_filter_sql,
                                         ")"));
    }
    return absl::OkStatus();
  }

  const absl::Time insert_start = absl::Now();
  std::string insert_sql;
  absl::StrAppend(&insert_sql, "INSERT INTO ", table_name, " VALUES ");
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
  status = RunSqlNoResult(conn, insert_sql);
  RecordPhase(phase_recorder, "insert_render_exec", absl::Now() - insert_start);
  if (!status.ok()) return status;
  if (!row_access_filter_sql.empty()) {
    return RunSqlNoResult(conn,
                          absl::StrCat("DELETE FROM ",
                                       table_name,
                                       " WHERE NOT (",
                                       row_access_filter_sql,
                                       ")"));
  }
  return status;
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
