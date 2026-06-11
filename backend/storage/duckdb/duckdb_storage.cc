#include "backend/storage/duckdb/duckdb_storage.h"

#include <filesystem>
#include <memory>
#include <string>
#include <system_error>
#include <utility>

#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/string_view.h"
#include "absl/synchronization/mutex.h"
#include "backend/storage/duckdb/duckdb_storage_internal.h"
#include "duckdb.h"

namespace bigquery_emulator {
namespace backend {
namespace storage {
namespace duckdb {

namespace fs = std::filesystem;

namespace internal {

absl::Status RunSql(DuckDBStorage::Impl* impl, absl::string_view sql) {
  if (impl == nullptr || impl->connection == nullptr) {
    return absl::FailedPreconditionError(
        "DuckDBStorage: connection is not initialized");
  }
  ::duckdb_result result;
  const std::string sql_str(sql);
  const auto state = ::duckdb_query(impl->connection, sql_str.c_str(), &result);
  if (state != ::DuckDBSuccess) {
    const auto* err = ::duckdb_result_error(&result);
    std::string detail = err == nullptr ? std::string("") : std::string(err);
    ::duckdb_destroy_result(&result);
    return DuckDBError(absl::StatusCode::kInternal,
                       absl::StrCat("DuckDB query failed: ", sql_str),
                       detail);
  }
  ::duckdb_destroy_result(&result);
  return absl::OkStatus();
}

absl::Status FilesystemStatus(absl::string_view what,
                              const std::error_code& ec) {
  if (!ec) return absl::OkStatus();
  return absl::Status(absl::StatusCode::kInternal,
                      absl::StrCat(what, ": ", ec.message()));
}

}  // namespace internal

DuckDBStorage::DuckDBStorage(std::string data_dir, std::unique_ptr<Impl> impl)
    : data_dir_(std::move(data_dir)), impl_(std::move(impl)) {}

DuckDBStorage::~DuckDBStorage() = default;

absl::StatusOr<std::unique_ptr<DuckDBStorage>> DuckDBStorage::Open(
    absl::string_view data_dir) {
  if (data_dir.empty()) {
    return absl::InvalidArgumentError(
        "DuckDBStorage::Open: data_dir must be non-empty");
  }
  fs::path root = fs::path(std::string(data_dir));
  std::error_code ec;
  fs::create_directories(root, ec);
  if (ec) {
    return absl::FailedPreconditionError(
        absl::StrCat("DuckDBStorage::Open: could not create data_dir ",
                     root.string(),
                     ": ",
                     ec.message()));
  }
  const fs::path catalog_path = root / std::string(internal::kCatalogDbFile);
  auto impl = std::make_unique<Impl>();
  const auto open_state = ::duckdb_open(catalog_path.c_str(), &impl->database);
  if (open_state != ::DuckDBSuccess) {
    return absl::InternalError(absl::StrCat(
        "DuckDBStorage::Open: duckdb_open failed for ", catalog_path.string()));
  }
  const auto conn_state = ::duckdb_connect(impl->database, &impl->connection);
  if (conn_state != ::DuckDBSuccess) {
    return absl::InternalError(
        absl::StrCat("DuckDBStorage::Open: duckdb_connect failed for ",
                     catalog_path.string()));
  }
  auto storage = std::unique_ptr<DuckDBStorage>(
      new DuckDBStorage(std::string(data_dir), std::move(impl)));
  absl::Status init = storage->InitCatalogTables();
  if (!init.ok()) {
    return init;
  }
  return storage;
}

std::string DuckDBStorage::DatasetDir(absl::string_view project_id,
                                      absl::string_view dataset_id) const {
  return (fs::path(data_dir_) / std::string(project_id) /
          std::string(dataset_id))
      .string();
}

std::string DuckDBStorage::DatasetDir(const DatasetId& id) const {
  return DatasetDir(id.project_id, id.dataset_id);
}

std::string DuckDBStorage::DatasetMetaPath(const DatasetId& id) const {
  return (fs::path(DatasetDir(id)) / std::string(internal::kDatasetMetaFile))
      .string();
}

std::string DuckDBStorage::TableMetaPath(const TableId& id) const {
  return (fs::path(DatasetDir(id.project_id, id.dataset_id)) /
          absl::StrCat(id.table_id, internal::kTableMetaSuffix))
      .string();
}

std::string DuckDBStorage::TableParquetPath(const TableId& id) const {
  return (fs::path(DatasetDir(id.project_id, id.dataset_id)) /
          absl::StrCat(id.table_id, ".parquet"))
      .string();
}

std::string DuckDBStorage::DuckDBSchemaName(absl::string_view project_id,
                                            absl::string_view dataset_id) {
  return absl::StrCat(project_id, "__", dataset_id);
}

std::string DuckDBStorage::DuckDBSchemaName(const DatasetId& id) {
  return DuckDBSchemaName(id.project_id, id.dataset_id);
}

}  // namespace duckdb
}  // namespace storage
}  // namespace backend
}  // namespace bigquery_emulator
