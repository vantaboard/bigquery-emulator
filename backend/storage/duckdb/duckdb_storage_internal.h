#ifndef BIGQUERY_EMULATOR_BACKEND_STORAGE_DUCKDB_DUCKDB_STORAGE_INTERNAL_H_
#define BIGQUERY_EMULATOR_BACKEND_STORAGE_DUCKDB_DUCKDB_STORAGE_INTERNAL_H_

// Shared helpers for the DuckDB storage translation units.

#include "backend/storage/duckdb/duckdb_storage.h"

#include <filesystem>
#include <string>
#include <vector>

#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "absl/strings/string_view.h"
#include "absl/types/span.h"
#include "backend/schema/schema.h"
#include "backend/storage/row_restriction.h"
#include "backend/storage/storage.h"
#include "duckdb.h"

namespace bigquery_emulator {
namespace backend {
namespace storage {
namespace duckdb {

struct DuckDBStorage::Impl {
  ::duckdb_database database = nullptr;
  ::duckdb_connection connection = nullptr;

  ~Impl() {
    if (connection != nullptr) {
      ::duckdb_disconnect(&connection);
    }
    if (database != nullptr) {
      ::duckdb_close(&database);
    }
  }
};

namespace internal {

namespace fs = std::filesystem;

constexpr absl::string_view kTableMetaSuffix = ".meta.json";
constexpr absl::string_view kDatasetMetaFile = "_dataset.meta.json";
constexpr absl::string_view kCatalogDbFile = "catalog.duckdb";

absl::Status DuckDBError(absl::StatusCode code,
                         absl::string_view what,
                         absl::string_view detail);

absl::Status RunSql(DuckDBStorage::Impl* impl, absl::string_view sql);

absl::Status FilesystemStatus(absl::string_view what,
                              const std::error_code& ec);

absl::Status WriteFileAtomic(const fs::path& path, absl::string_view contents);

absl::StatusOr<std::string> ReadFile(const fs::path& path);

absl::StatusOr<std::string> RenderTableMetaJson(
    const schema::TableSchema& schema);

std::string RenderDatasetMetaJson(absl::string_view location);

absl::StatusOr<schema::TableSchema> ParseTableMetaJson(absl::string_view json);

std::string QuoteIdent(absl::string_view ident);

std::string EscapeStringLiteralInner(absl::string_view s);

absl::StatusOr<std::string> RenderCellLiteral(
    const Value& cell, const schema::ColumnSchema& column);

std::string RenderColumnList(const schema::TableSchema& schema);

std::string RenderPredicateClause(const EqualityPredicate& pred);

std::string RenderColumnIdentList(const schema::TableSchema& schema);

std::string RenderSelectedColumnIdentList(absl::Span<const std::string> names);

absl::StatusOr<schema::TableSchema> ProjectSchema(
    const schema::TableSchema& schema, absl::Span<const std::string> names);

void TryDropTempTable(DuckDBStorage::Impl* impl,
                      absl::string_view qualified_name);

absl::StatusOr<fs::path> EnsureTableMetaExists(const TableId& id,
                                               const fs::path& ds_dir,
                                               const fs::path& meta_path);

absl::Status ValidateRowsShape(absl::string_view tag,
                               const TableId& id,
                               absl::Span<const Row> rows,
                               size_t ncols);

absl::StatusOr<std::string> BuildBatchInsertSql(
    absl::string_view tmp_table,
    absl::Span<const Row> rows,
    const schema::TableSchema& schema);

absl::Status SnapshotTempTableToParquet(absl::string_view tag,
                                        DuckDBStorage::Impl* impl,
                                        absl::string_view tmp_table,
                                        const std::string& tmp_path,
                                        const std::string& parquet_path);

absl::StatusOr<Value> ReadCell(::duckdb_result* result,
                               idx_t col,
                               idx_t row,
                               const schema::ColumnSchema& column);

absl::Status ExecuteSelect(DuckDBStorage::Impl* impl,
                           absl::string_view sql,
                           const schema::TableSchema& schema,
                           absl::string_view tag,
                           const TableId& id,
                           absl::string_view parquet_path,
                           std::vector<Row>* out);

}  // namespace internal
}  // namespace duckdb
}  // namespace storage
}  // namespace backend
}  // namespace bigquery_emulator

#endif  // BIGQUERY_EMULATOR_BACKEND_STORAGE_DUCKDB_DUCKDB_STORAGE_INTERNAL_H_
