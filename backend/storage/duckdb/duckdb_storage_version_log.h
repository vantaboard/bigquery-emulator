#ifndef BIGQUERY_EMULATOR_BACKEND_STORAGE_DUCKDB_DUCKDB_STORAGE_VERSION_LOG_H_
#define BIGQUERY_EMULATOR_BACKEND_STORAGE_DUCKDB_DUCKDB_STORAGE_VERSION_LOG_H_

// Per-table Parquet version log for BigQuery time travel (FOR SYSTEM_TIME
// AS OF). Archives superseded snapshots under `{table}.versions/` and
// tracks metadata in `{table}.versions.json`.

#include <cstdint>
#include <string>
#include <vector>

#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "absl/strings/string_view.h"
#include "backend/storage/storage.h"

namespace bigquery_emulator {
namespace backend {
namespace storage {
namespace duckdb {

class DuckDBStorage;

namespace internal {

// BigQuery's documented 7-day time-travel window.
constexpr std::int64_t kTimeTravelWindowMs = 7LL * 24 * 60 * 60 * 1000;

struct VersionEntry {
  std::int64_t valid_from_ms = 0;
  std::string file;
};

struct VersionIndex {
  std::int64_t created_ts_ms = 0;
  std::int64_t current_ts_ms = 0;
  std::vector<VersionEntry> versions;
};

std::string TableVersionsDir(const DuckDBStorage& storage, const TableId& id);
std::string TableVersionsIndexPath(const DuckDBStorage& storage,
                                   const TableId& id);
std::string TableTombstoneDir(const DuckDBStorage& storage,
                              const TableId& id,
                              std::int64_t deleted_ms);

absl::StatusOr<VersionIndex> ReadVersionIndex(absl::string_view index_path,
                                              bool allow_missing);
absl::Status WriteVersionIndex(absl::string_view index_path,
                               const VersionIndex& index);

// Initializes `{table}.versions.json` when a table is first created.
absl::Status InitVersionIndex(const DuckDBStorage& storage,
                              const TableId& id,
                              std::int64_t created_ts_ms);

// Before replacing the live parquet, archive the current snapshot and bump
// `current_ts_ms`. Prunes entries/files older than the 7-day window.
absl::Status ArchiveParquetBeforeReplace(const DuckDBStorage& storage,
                                         const TableId& id,
                                         absl::string_view live_parquet_path,
                                         std::int64_t mutation_ts_ms);

// Resolves the parquet path for `as_of_ms`, or returns INVALID_ARGUMENT with
// BigQuery's "Invalid snapshot time" shape when the timestamp is out of
// window or before readable history.
absl::StatusOr<std::string> ResolveParquetSnapshotAt(
    const DuckDBStorage& storage,
    const TableId& id,
    absl::string_view live_parquet_path,
    std::int64_t as_of_ms,
    std::int64_t now_ms);

// Soft-deletes table artifacts into `.tombstones/{table_id}/{deleted_ms}/`.
absl::Status MoveTableToTombstone(const DuckDBStorage& storage,
                                  const TableId& id,
                                  std::int64_t deleted_ms);

// Restores a soft-deleted table from `.tombstones/{table_id}/{deleted_ms}/`.
// When `deleted_ms` is zero, picks the newest tombstone for `id`.
absl::Status RestoreTableFromTombstone(const DuckDBStorage& storage,
                                       const TableId& id,
                                       std::int64_t deleted_ms);

}  // namespace internal
}  // namespace duckdb
}  // namespace storage
}  // namespace backend
}  // namespace bigquery_emulator

#endif  // BIGQUERY_EMULATOR_BACKEND_STORAGE_DUCKDB_DUCKDB_STORAGE_VERSION_LOG_H_
