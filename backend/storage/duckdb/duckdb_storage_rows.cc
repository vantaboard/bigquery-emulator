#include <cstdint>
#include <filesystem>
#include <memory>
#include <optional>
#include <string>
#include <system_error>
#include <utility>
#include <vector>

#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/string_view.h"
#include "absl/synchronization/mutex.h"
#include "absl/time/clock.h"
#include "absl/time/time.h"
#include "absl/types/span.h"
#include "backend/schema/schema.h"
#include "backend/storage/duckdb/duckdb_storage.h"
#include "backend/storage/duckdb/duckdb_storage_internal.h"
#include "backend/storage/duckdb/duckdb_storage_version_log.h"
#include "backend/storage/storage.h"
#include "duckdb.h"

namespace bigquery_emulator {
namespace backend {
namespace storage {
namespace duckdb {

namespace fs = std::filesystem;

namespace internal {

absl::StatusOr<Value> ReadRepeatedCell(::duckdb_result* result,
                                       idx_t col,
                                       idx_t row,
                                       const schema::ColumnSchema& column) {
  auto* str = ::duckdb_value_varchar(result, col, row);
  const std::string text = str == nullptr ? std::string("") : str;
  if (str != nullptr) ::duckdb_free(str);
  schema::ColumnSchema element = column;
  element.mode = schema::ColumnMode::kNullable;
  auto parsed = internal::ParseDuckDBListVarchar(text, element);
  if (!parsed.ok()) {
    return absl::InternalError(
        absl::StrCat("ReadCell: failed to decode REPEATED column `",
                     column.name,
                     "`: ",
                     parsed.status().message()));
  }
  return *parsed;
}

absl::StatusOr<Value> ReadStructCell(::duckdb_result* result,
                                     idx_t col,
                                     idx_t row,
                                     const schema::ColumnSchema& column) {
  auto* str = ::duckdb_value_varchar(result, col, row);
  const std::string text = str == nullptr ? std::string("") : str;
  if (str != nullptr) {
    ::duckdb_free(str);
  }
  if (text.empty()) {
    return Value::Null();
  }
  auto parsed = internal::ParseDuckDBStructVarchar(text, column);
  if (!parsed.ok()) {
    return absl::InternalError(
        absl::StrCat("ReadCell: failed to decode STRUCT column `",
                     column.name,
                     "`: ",
                     parsed.status().message()));
  }
  return *parsed;
}

absl::StatusOr<Value> ReadVarcharCell(::duckdb_result* result,
                                      idx_t col,
                                      idx_t row) {
  auto* str = ::duckdb_value_varchar(result, col, row);
  Value out = Value::String(str == nullptr ? std::string("") : str);
  if (str != nullptr) ::duckdb_free(str);
  return out;
}

absl::StatusOr<Value> ReadScalarCell(::duckdb_result* result,
                                     idx_t col,
                                     idx_t row,
                                     const schema::ColumnSchema& column) {
  switch (column.type) {
    case schema::ColumnType::kBool:
      return Value::Bool(::duckdb_value_boolean(result, col, row));
    case schema::ColumnType::kInt64:
      return Value::Int64(::duckdb_value_int64(result, col, row));
    case schema::ColumnType::kFloat64:
      return Value::Float64(::duckdb_value_double(result, col, row));
    case schema::ColumnType::kBytes: {
      ::duckdb_blob blob = ::duckdb_value_blob(result, col, row);
      std::string bytes;
      if (blob.data != nullptr) {
        bytes.assign(static_cast<const char*>(blob.data), blob.size);
        ::duckdb_free(blob.data);
      }
      return Value::Bytes(std::move(bytes));
    }
    case schema::ColumnType::kString:
    case schema::ColumnType::kDate:
    case schema::ColumnType::kTime:
    case schema::ColumnType::kDatetime:
    case schema::ColumnType::kTimestamp:
    case schema::ColumnType::kNumeric:
    case schema::ColumnType::kBignumeric:
    case schema::ColumnType::kJson:
    case schema::ColumnType::kGeography:
    case schema::ColumnType::kArray:
    case schema::ColumnType::kStruct:
    case schema::ColumnType::kUnknown:
      return ReadVarcharCell(result, col, row);
  }
  return absl::InternalError("ReadCell: unreachable column type");
}

// Reads a single DuckDB cell into a storage::Value, using the
// column type from `column` to pick the right C-API accessor. NULL
// cells become Value::Null() regardless of column type.
absl::StatusOr<Value> ReadCell(::duckdb_result* result,
                               idx_t col,
                               idx_t row,
                               const schema::ColumnSchema& column) {
  if (column.mode == schema::ColumnMode::kRepeated) {
    return ReadRepeatedCell(result, col, row, column);
  }
  if (::duckdb_value_is_null(result, col, row)) return Value::Null();
  if (column.type == schema::ColumnType::kStruct) {
    return ReadStructCell(result, col, row, column);
  }
  if (column.type == schema::ColumnType::kArray) {
    return ReadVarcharCell(result, col, row);
  }
  return ReadScalarCell(result, col, row, column);
}

}  // namespace internal

absl::Status DuckDBStorage::AppendRows(const TableId& id,
                                       absl::Span<const Row> rows) {
  absl::MutexLock lock(&mu_);
  const fs::path ds_dir = DatasetDir(id.project_id, id.dataset_id);
  const fs::path meta_path = TableMetaPath(id);
  auto meta_or = internal::EnsureTableMetaExists(id, ds_dir, meta_path);
  if (!meta_or.ok()) return meta_or.status();
  if (rows.empty()) return absl::OkStatus();

  auto contents_or = internal::ReadFile(meta_path);
  if (!contents_or.ok()) return contents_or.status();
  auto schema_or = internal::ParseTableMetaJson(*contents_or);
  if (!schema_or.ok()) return schema_or.status();
  const schema::TableSchema& schema = *schema_or;
  const schema::TableSchema physical = internal::ParquetStorageSchema(schema);
  auto valid = internal::ValidateRowsShape(
      "AppendRows", id, rows, physical.columns.size());
  if (!valid.ok()) return valid;

  const std::string parquet_path = TableParquetPath(id);
  const std::string tmp_path = absl::StrCat(parquet_path, ".tmp");
  // Use a deterministic temp-table name so a crashed previous
  // AppendRows leaves a recoverable scratch object; CREATE OR
  // REPLACE wipes it on the next call.
  const std::string tmp_table = "main.__bqemu_append";

  // 1. Stage a fresh temp table with the explicit column schema.
  // CREATE OR REPLACE TEMP TABLE coerces a stale row from an
  // interrupted previous run if any.
  const std::string col_list = internal::RenderColumnList(physical);
  auto status = internal::RunSql(
      impl_.get(),
      absl::StrCat("CREATE OR REPLACE TEMP TABLE ", tmp_table, " ", col_list));
  if (!status.ok()) return status;

  // 2. Carry over existing rows if a parquet snapshot exists. The
  // file is created by CreateTable so the normal path always has
  // one; a hand-curated dataset directory may not, in which case
  // we treat the table as empty.
  std::error_code ec;
  if (fs::exists(parquet_path, ec)) {
    const std::string select_cols = internal::RenderColumnIdentList(physical);
    status = internal::RunSql(
        impl_.get(),
        absl::StrCat("INSERT INTO ",
                     tmp_table,
                     " SELECT ",
                     select_cols,
                     " FROM read_parquet('",
                     internal::EscapeStringLiteralInner(parquet_path),
                     "')"));
    if (!status.ok()) {
      internal::TryDropTempTable(impl_.get(), tmp_table);
      return status;
    }
  }

  // 3. Emit one multi-row INSERT VALUES so we hit the planner once
  // for the whole batch. The literal renderer raises on shape
  // mismatches; on error we tear down the temp table and bail
  // without touching the parquet file.
  auto insert_or = internal::BuildBatchInsertSql(tmp_table, rows, physical);
  if (!insert_or.ok()) {
    internal::TryDropTempTable(impl_.get(), tmp_table);
    return insert_or.status();
  }
  status = internal::RunSql(impl_.get(), *insert_or);
  if (!status.ok()) {
    internal::TryDropTempTable(impl_.get(), tmp_table);
    return status;
  }

  // 4 + 5. Snapshot to a sibling tmp parquet then atomic rename.
  const std::int64_t mutation_ts_ms = absl::ToUnixMillis(absl::Now());
  absl::Status archive = internal::ArchiveParquetBeforeReplace(
      *this, id, parquet_path, mutation_ts_ms);
  if (!archive.ok()) return archive;
  return internal::SnapshotTempTableToParquet(
      "AppendRows", impl_.get(), tmp_table, tmp_path, parquet_path);
}

absl::Status DuckDBStorage::OverwriteRows(const TableId& id,
                                          absl::Span<const Row> rows) {
  absl::MutexLock lock(&mu_);
  const fs::path ds_dir = DatasetDir(id.project_id, id.dataset_id);
  const fs::path meta_path = TableMetaPath(id);
  auto meta_or = internal::EnsureTableMetaExists(id, ds_dir, meta_path);
  if (!meta_or.ok()) return meta_or.status();

  auto contents_or = internal::ReadFile(meta_path);
  if (!contents_or.ok()) return contents_or.status();
  auto schema_or = internal::ParseTableMetaJson(*contents_or);
  if (!schema_or.ok()) return schema_or.status();
  const schema::TableSchema& schema = *schema_or;
  const schema::TableSchema physical = internal::ParquetStorageSchema(schema);
  auto valid = internal::ValidateRowsShape(
      "OverwriteRows", id, rows, physical.columns.size());
  if (!valid.ok()) return valid;

  const std::string parquet_path = TableParquetPath(id);
  const std::string tmp_path = absl::StrCat(parquet_path, ".tmp");
  const std::string tmp_table = "main.__bqemu_overwrite";

  // Stage a fresh temp table holding just the new rows. We deliberately
  // do NOT carry over existing rows (unlike AppendRows) -- this is the
  // overwrite contract: replace the parquet file with whatever the
  // caller hands us, including the empty-vector case.
  const std::string col_list = internal::RenderColumnList(physical);
  auto status = internal::RunSql(
      impl_.get(),
      absl::StrCat("CREATE OR REPLACE TEMP TABLE ", tmp_table, " ", col_list));
  if (!status.ok()) return status;

  if (!rows.empty()) {
    auto insert_or = internal::BuildBatchInsertSql(tmp_table, rows, physical);
    if (!insert_or.ok()) {
      internal::TryDropTempTable(impl_.get(), tmp_table);
      return insert_or.status();
    }
    status = internal::RunSql(impl_.get(), *insert_or);
    if (!status.ok()) {
      internal::TryDropTempTable(impl_.get(), tmp_table);
      return status;
    }
  }

  const std::int64_t mutation_ts_ms = absl::ToUnixMillis(absl::Now());
  absl::Status archive = internal::ArchiveParquetBeforeReplace(
      *this, id, parquet_path, mutation_ts_ms);
  if (!archive.ok()) return archive;
  return internal::SnapshotTempTableToParquet(
      "OverwriteRows", impl_.get(), tmp_table, tmp_path, parquet_path);
}

namespace internal {

// Runs `sql` (a SELECT) against `impl`'s connection and materializes
// every emitted row into `*out`. Shared between `ScanRows` and
// `CreateReadStream`: both differ only in whether they push LIMIT /
// OFFSET into the SQL, but the post-execute decoding loop is
// identical, and lifting it out avoids two near-duplicate copies of
// the duckdb_result handling.
//
// `tag` is the caller-facing name plumbed into error messages
// ("ScanRows" / "CreateReadStream") so a failure in either branch is
// attributed to the right surface.
absl::Status ExecuteSelect(DuckDBStorage::Impl* impl,
                           absl::string_view sql,
                           const schema::TableSchema& schema,
                           absl::string_view tag,
                           const TableId& id,
                           absl::string_view parquet_path,
                           std::vector<Row>* out) {
  const std::string sql_str(sql);
  ::duckdb_result result;
  const auto state = ::duckdb_query(impl->connection, sql_str.c_str(), &result);
  if (state != ::DuckDBSuccess) {
    const auto* err = ::duckdb_result_error(&result);
    std::string detail = err == nullptr ? std::string("") : std::string(err);
    ::duckdb_destroy_result(&result);
    return absl::InternalError(absl::StrCat(
        tag, ": duckdb_query failed for ", parquet_path, ": ", detail));
  }
  const idx_t row_count = ::duckdb_row_count(&result);
  const idx_t col_count = ::duckdb_column_count(&result);
  if (col_count != schema.columns.size()) {
    ::duckdb_destroy_result(&result);
    return absl::InternalError(
        absl::StrCat(tag,
                     ": parquet file has ",
                     col_count,
                     " column(s) but sidecar schema declares ",
                     schema.columns.size(),
                     " for table ",
                     id.project_id,
                     ".",
                     id.dataset_id,
                     ".",
                     id.table_id));
  }
  out->reserve(out->size() + row_count);
  for (idx_t r = 0; r < row_count; ++r) {
    Row row;
    row.cells.reserve(col_count);
    for (idx_t c = 0; c < col_count; ++c) {
      auto cell_or = internal::ReadCell(&result, c, r, schema.columns[c]);
      if (!cell_or.ok()) {
        ::duckdb_destroy_result(&result);
        return cell_or.status();
      }
      row.cells.push_back(std::move(*cell_or));
    }
    out->push_back(std::move(row));
  }
  ::duckdb_destroy_result(&result);
  return absl::OkStatus();
}

}  // namespace internal

absl::StatusOr<std::unique_ptr<RowIterator>> DuckDBStorage::ScanRows(
    const TableId& id) const {
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
  const schema::TableSchema physical = internal::ParquetStorageSchema(schema);

  const std::string parquet_path = TableParquetPath(id);
  std::vector<Row> rows;
  if (!fs::exists(parquet_path, ec)) {
    return std::unique_ptr<RowIterator>(
        new internal::VectorRowIterator(std::move(rows)));
  }

  // Explicit projection: ScanRows promises rows in column-list
  // order regardless of how the parquet file laid them out (the
  // file is written by us, but a user could hand-edit it).
  const std::string select_cols = internal::RenderColumnIdentList(physical);
  const std::string sql =
      absl::StrCat("SELECT ",
                   select_cols,
                   " FROM read_parquet('",
                   internal::EscapeStringLiteralInner(parquet_path),
                   "')");
  auto status = internal::ExecuteSelect(
      impl_.get(), sql, physical, "ScanRows", id, parquet_path, &rows);
  if (!status.ok()) return status;
  return std::unique_ptr<RowIterator>(
      new internal::VectorRowIterator(std::move(rows)));
}

absl::StatusOr<std::int64_t> DuckDBStorage::CountRows(const TableId& id) const {
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
  const std::string parquet_path = TableParquetPath(id);
  if (!fs::exists(parquet_path, ec)) {
    return std::int64_t{0};
  }
  return internal::CountParquetRows(impl_.get(), parquet_path, "");
}

std::optional<std::string> DuckDBStorage::ParquetSnapshotPath(
    const TableId& id) const {
  absl::MutexLock lock(&mu_);
  const fs::path ds_dir = DatasetDir(id.project_id, id.dataset_id);
  std::error_code ec;
  if (!fs::exists(ds_dir, ec)) {
    return std::nullopt;
  }
  const fs::path meta_path = TableMetaPath(id);
  if (!fs::exists(meta_path, ec)) {
    return std::nullopt;
  }
  const std::string parquet_path = TableParquetPath(id);
  if (!fs::exists(parquet_path, ec)) {
    return std::nullopt;
  }
  return parquet_path;
}

absl::StatusOr<std::optional<std::string>> DuckDBStorage::ParquetSnapshotPathAt(
    const TableId& id, std::int64_t as_of_ms) const {
  absl::MutexLock lock(&mu_);
  const fs::path ds_dir = DatasetDir(id.project_id, id.dataset_id);
  std::error_code ec;
  if (!fs::exists(ds_dir, ec)) {
    return std::nullopt;
  }
  const fs::path meta_path = TableMetaPath(id);
  if (!fs::exists(meta_path, ec)) {
    return std::nullopt;
  }
  const std::string live_parquet_path = TableParquetPath(id);
  const std::int64_t now_ms = absl::ToUnixMillis(absl::Now());
  return internal::ResolveParquetSnapshotAt(
      *this, id, live_parquet_path, as_of_ms, now_ms);
}

}  // namespace duckdb
}  // namespace storage
}  // namespace backend
}  // namespace bigquery_emulator
