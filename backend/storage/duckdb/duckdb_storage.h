#ifndef BIGQUERY_EMULATOR_BACKEND_STORAGE_DUCKDB_DUCKDB_STORAGE_H_
#define BIGQUERY_EMULATOR_BACKEND_STORAGE_DUCKDB_DUCKDB_STORAGE_H_

// DuckDBStorage is the persistent, file-backed `Storage` implementation.
//
// Layout under `data_dir`:
//
//   <data_dir>/
//     catalog.duckdb                            # DuckDB catalog file
//     <project_id>/                             # one dir per project
//       <dataset_id>/                           # one dir per dataset
//         _dataset.meta.json                    # dataset-level metadata
//         <table_id>.parquet                    # data file
//         <table_id>.meta.json                  # per-table sidecar
//
// The DuckDB catalog file tracks dataset existence (as DuckDB schemas)
// and table existence (as DuckDB views over the matching parquet file).
// BigQuery-specific metadata that does not fit cleanly in DuckDB
// (description, labels, friendlyName, etag, and the BigQuery-typed
// schema) lives in the JSON sidecars so a developer can inspect, edit,
// or hand-author a dataset/table without going through the emulator.
//
// This header is the *core* skeleton: it owns the connection, the
// directory layout, the metadata sidecar, and dataset/table CRUD.
// The actual Parquet I/O for `AppendRows` / `ScanRows` lands in the
// follow-up plan `duckdb-storage-ddl_p1e2f3a4`; both methods return
// UNIMPLEMENTED until then.
//
// Concurrency: every public method acquires a single absl::Mutex.
// DuckDB itself is thread-safe per-connection but we serialize at
// the C++ level so dataset / table directory mutations stay in
// lockstep with catalog rows.

#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "absl/base/thread_annotations.h"
#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "absl/strings/string_view.h"
#include "absl/synchronization/mutex.h"
#include "absl/types/span.h"
#include "backend/schema/schema.h"
#include "backend/storage/storage.h"

namespace bigquery_emulator {
namespace backend {
namespace storage {
namespace duckdb {

class DuckDBStorage : public Storage {
 public:
  // Constructs a DuckDBStorage rooted at `data_dir`. The directory is
  // created (recursively) if it does not exist. Opens a DuckDB
  // connection backed by `<data_dir>/catalog.duckdb` so dataset /
  // table existence survives process restarts.
  //
  // Returns INVALID_ARGUMENT when `data_dir` is empty, FAILED_PRECONDITION
  // when the directory can not be created (e.g. permission denied), or
  // INTERNAL when DuckDB itself refuses to open the catalog file. On
  // success the caller owns the returned unique_ptr; the connection
  // closes on destruction.
  static absl::StatusOr<std::unique_ptr<DuckDBStorage>> Open(
      absl::string_view data_dir);

  ~DuckDBStorage() override;

  DuckDBStorage(const DuckDBStorage&) = delete;
  DuckDBStorage& operator=(const DuckDBStorage&) = delete;

  // Path the storage was opened with. Stable for the lifetime of the
  // instance; exposed mainly for tests / logs.
  const std::string& data_dir() const {
    return data_dir_;
  }

  // ------------------------------------------------------------------
  // Storage interface
  // ------------------------------------------------------------------
  absl::Status CreateDataset(const DatasetId& id,
                             absl::string_view location) override;
  absl::Status DropDataset(const DatasetId& id, bool delete_contents) override;
  absl::StatusOr<std::vector<DatasetId>> ListDatasets(
      absl::string_view project_id) const override;

  absl::Status CreateTable(const TableId& id,
                           const schema::TableSchema& schema) override;
  absl::Status DropTable(const TableId& id) override;
  absl::StatusOr<std::vector<TableId>> ListTables(
      const DatasetId& dataset_id) const override;

  absl::StatusOr<schema::TableSchema> GetSchema(
      const TableId& id) const override;

  // The core skeleton returns UNIMPLEMENTED for these two. The DDL
  // plan (`duckdb-storage-ddl_p1e2f3a4`) lowers them onto Parquet
  // I/O via DuckDB's `read_parquet` + INSERT statements.
  absl::Status AppendRows(const TableId& id,
                          absl::Span<const Row> rows) override;
  absl::Status OverwriteRows(const TableId& id,
                             absl::Span<const Row> rows) override;
  absl::StatusOr<std::unique_ptr<RowIterator>> ScanRows(
      const TableId& id) const override;
  absl::StatusOr<std::unique_ptr<RowIterator>> CreateReadStream(
      const TableId& id, const ReadFilter& filter) const override;
  absl::StatusOr<std::int64_t> CountRows(const TableId& id) const override;

  std::optional<std::string> ParquetSnapshotPath(
      const TableId& id) const override;

  absl::Status UpsertRoutine(const RoutineRecord& record) override;
  absl::Status DeleteRoutine(const RoutineId& id) override;
  absl::StatusOr<RoutineRecord> GetRoutine(const RoutineId& id) const override;
  absl::StatusOr<std::vector<RoutineRecord>> ListRoutines(
      const DatasetId& dataset_id) const override;
  absl::StatusOr<std::vector<RoutineRecord>> ListAllRoutines() const override;

  // Ensures catalog metadata tables (e.g. `__bqemu_routines`) exist.
  // Called from `Open` and idempotently before routine CRUD.
  absl::Status InitCatalogTables();

  // Pimpl: keeps the DuckDB C handles out of this header so the
  // engine-agnostic Storage signatures stay enforceable from the
  // include graph alone (callers cannot accidentally reach into
  // `duckdb_database` / `duckdb_connection`). Public so the
  // translation unit's helper functions can take an `Impl*` directly
  // — the struct itself is only ever defined inside duckdb_storage.cc.
  struct Impl;

 private:
  DuckDBStorage(std::string data_dir, std::unique_ptr<Impl> impl);

  // Filesystem layout helpers. All take ids by string_view and emit
  // absolute paths under `data_dir_`.
  std::string DatasetDir(absl::string_view project_id,
                         absl::string_view dataset_id) const;
  std::string DatasetDir(const DatasetId& id) const;
  std::string DatasetMetaPath(const DatasetId& id) const;
  std::string TableMetaPath(const TableId& id) const;
  std::string TableParquetPath(const TableId& id) const;

  // Stable DuckDB schema name for a (project, dataset) pair. We can't
  // just use the dataset_id because two projects may share a dataset
  // id; collapse them into one safe identifier so DuckDB stays happy.
  static std::string DuckDBSchemaName(absl::string_view project_id,
                                      absl::string_view dataset_id);
  static std::string DuckDBSchemaName(const DatasetId& id);

  std::string data_dir_;
  mutable absl::Mutex mu_;
  // The pointer is set once at construction and never reassigned; the
  // *contents* of the connection are guarded by `mu_` because DuckDB
  // is thread-safe per-connection but the dataset/table directory
  // mutations need to stay coherent with the DuckDB catalog rows we
  // emit alongside them.
  std::unique_ptr<Impl> impl_{};
};

}  // namespace duckdb
}  // namespace storage
}  // namespace backend
}  // namespace bigquery_emulator

#endif  // BIGQUERY_EMULATOR_BACKEND_STORAGE_DUCKDB_DUCKDB_STORAGE_H_
