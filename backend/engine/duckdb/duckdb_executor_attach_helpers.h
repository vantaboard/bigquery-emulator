#ifndef BIGQUERY_EMULATOR_BACKEND_ENGINE_DUCKDB_DUCKDB_EXECUTOR_ATTACH_HELPERS_H_
#define BIGQUERY_EMULATOR_BACKEND_ENGINE_DUCKDB_DUCKDB_EXECUTOR_ATTACH_HELPERS_H_

#include <optional>
#include <string>
#include <vector>

#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "absl/strings/string_view.h"
#include "backend/catalog/storage_table.h"
#include "backend/engine/duckdb/duckdb_executor_internal.h"
#include "backend/engine/phase_recorder.h"
#include "backend/schema/schema.h"
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
    std::optional<std::int64_t> as_of_ms);

absl::Status AttachParquetTableAt(::duckdb_connection conn,
                                  const schema::TableSchema& schema,
                                  absl::string_view quoted_table_name,
                                  absl::string_view parquet_path,
                                  absl::string_view row_access_filter_sql,
                                  PhaseRecorder* phase_recorder);

absl::StatusOr<std::vector<storage::Row>> ScanAllTableRows(
    storage::Storage* storage,
    const storage::TableId& id,
    PhaseRecorder* phase_recorder);

absl::Status InsertScannedRowsAt(::duckdb_connection conn,
                                 const catalog::StorageTable& table,
                                 const schema::TableSchema& schema,
                                 absl::string_view quoted_table_name,
                                 const std::vector<storage::Row>& rows,
                                 PhaseRecorder* phase_recorder);

absl::Status ApplyRowAccessFilterDelete(
    ::duckdb_connection conn,
    absl::string_view quoted_table_name,
    absl::string_view row_access_filter_sql);

absl::Status AttachCollectedQueryTables(::duckdb_connection conn,
                                        storage::Storage* storage,
                                        const TableScanCollector& collector,
                                        PhaseRecorder* phase_recorder);

}  // namespace internal
}  // namespace duckdb
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator

#endif  // BIGQUERY_EMULATOR_BACKEND_ENGINE_DUCKDB_DUCKDB_EXECUTOR_ATTACH_HELPERS_H_
