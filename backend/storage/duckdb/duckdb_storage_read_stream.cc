#include <filesystem>
#include <memory>
#include <string>
#include <system_error>
#include <utility>
#include <vector>

#include "absl/status/statusor.h"
#include "absl/strings/str_cat.h"
#include "absl/synchronization/mutex.h"
#include "backend/schema/schema.h"
#include "backend/storage/duckdb/duckdb_storage.h"
#include "backend/storage/duckdb/duckdb_storage_internal.h"
#include "backend/storage/storage.h"

namespace bigquery_emulator {
namespace backend {
namespace storage {
namespace duckdb {

namespace fs = std::filesystem;

absl::StatusOr<std::unique_ptr<RowIterator>> DuckDBStorage::CreateReadStream(
    const TableId& id, const ReadFilter& filter) const {
  absl::MutexLock lock(&mu_);
  const fs::path ds_dir = DatasetDir(id.project_id, id.dataset_id);
  std::error_code ec;
  if (!fs::exists(ds_dir, ec)) {
    return absl::NotFoundError(
        absl::StrCat("dataset not found: ", id.project_id, ".", id.dataset_id));
  }
  const fs::path meta_path = TableMetaPath(id);
  if (!fs::exists(meta_path, ec)) {
    return absl::NotFoundError(absl::StrCat("table not found: ",
                                            id.project_id,
                                            ".",
                                            id.dataset_id,
                                            ".",
                                            id.table_id));
  }
  auto contents_or = internal::ReadFile(meta_path);
  if (!contents_or.ok()) return contents_or.status();
  auto schema_or = internal::ParseTableMetaJson(*contents_or);
  if (!schema_or.ok()) return schema_or.status();
  const schema::TableSchema& schema = *schema_or;

  const std::string parquet_path = TableParquetPath(id);
  std::vector<Row> rows;
  if (!fs::exists(parquet_path, ec)) {
    return std::unique_ptr<RowIterator>(
        new internal::VectorRowIterator(std::move(rows)));
  }

  // When the caller pins a
  // `selected_fields` list we project rows down to that subset. The
  // projected schema is what `ExecuteSelect` decodes against, so the
  // `Row.cells` vector matches the projected column order exactly.
  // An empty list means "all columns".
  schema::TableSchema effective_schema = schema;
  std::string select_cols;
  if (!filter.selected_fields.empty()) {
    auto projected_or = internal::ProjectSchema(schema, filter.selected_fields);
    if (!projected_or.ok()) return projected_or.status();
    effective_schema = std::move(*projected_or);
    select_cols =
        internal::RenderSelectedColumnIdentList(schema, filter.selected_fields);
  } else {
    select_cols = internal::RenderColumnIdentList(schema);
  }

  // ORDER BY a stable row identifier so OFFSET / LIMIT yield
  // deterministic windows across calls. The Arrow query pipeline
  // uses `read_parquet`'s `file_row_number` extra column for the
  // same purpose; reuse that here so a caller resuming a stream
  // at offset=N gets the same rows it would
  // have received if it had stayed connected. The column is
  // synthesized by DuckDB at scan time and never selected into the
  // result.
  std::string sql =
      absl::StrCat("SELECT ",
                   select_cols,
                   " FROM read_parquet('",
                   internal::EscapeStringLiteralInner(parquet_path),
                   "', file_row_number = true)");
  // Push the analyzer-transpiled restriction (or legacy equality
  // predicate) down into a SQL WHERE clause before ORDER BY.
  if (filter.where_sql.has_value() && !filter.where_sql->empty()) {
    absl::StrAppend(&sql, internal::RenderWhereSqlClause(*filter.where_sql));
  } else if (filter.equality_predicate.has_value()) {
    absl::StrAppend(
        &sql, internal::RenderPredicateClause(*filter.equality_predicate));
  }
  // Merge partition bounds on `file_row_number`. When a predicate WHERE
  // already exists, append with AND; otherwise emit a fresh WHERE.
  if (filter.row_start > 0 || filter.row_end >= 0) {
    const bool has_where = sql.find(" WHERE ") != std::string::npos;
    if (has_where) {
      if (filter.row_start > 0) {
        absl::StrAppend(&sql, " AND file_row_number >= ", filter.row_start);
      }
      if (filter.row_end >= 0) {
        absl::StrAppend(&sql, " AND file_row_number < ", filter.row_end);
      }
    } else {
      absl::StrAppend(
          &sql,
          internal::RenderRowPartitionClause(filter.row_start, filter.row_end));
    }
  }
  absl::StrAppend(&sql, " ORDER BY file_row_number");
  if (filter.row_limit > 0) {
    absl::StrAppend(&sql, " LIMIT ", filter.row_limit);
  }
  if (filter.offset > 0) {
    // DuckDB requires LIMIT to appear before OFFSET. When the caller
    // did not pin a limit we still need to emit one for OFFSET to
    // parse — use the DuckDB `LIMIT ALL` form so the optimizer keeps
    // the cap unbounded.
    if (filter.row_limit <= 0) {
      absl::StrAppend(&sql, " LIMIT ALL");
    }
    absl::StrAppend(&sql, " OFFSET ", filter.offset);
  }
  auto status = internal::ExecuteSelect(impl_.get(),
                                        sql,
                                        effective_schema,
                                        "CreateReadStream",
                                        id,
                                        parquet_path,
                                        &rows);
  if (!status.ok()) return status;
  return std::unique_ptr<RowIterator>(
      new internal::VectorRowIterator(std::move(rows)));
}

}  // namespace duckdb
}  // namespace storage
}  // namespace backend
}  // namespace bigquery_emulator
