#ifndef BIGQUERY_EMULATOR_BACKEND_STORAGE_DUCKDB_DUCKDB_STORAGE_TOMBSTONE_REGISTRY_INTERNAL_H_
#define BIGQUERY_EMULATOR_BACKEND_STORAGE_DUCKDB_DUCKDB_STORAGE_TOMBSTONE_REGISTRY_INTERNAL_H_

// Internal helpers shared between duckdb_storage_tombstone_registry*.cc
// translation units. Not part of the public storage surface.

#include <filesystem>
#include <string>
#include <utility>
#include <vector>

#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "absl/strings/string_view.h"
#include "backend/storage/duckdb/duckdb_storage.h"
#include "backend/storage/storage.h"
#include "backend/storage/table_governance.h"

namespace bigquery_emulator {
namespace backend {
namespace storage {
namespace duckdb {
namespace tombstone_registry_internal {

constexpr absl::string_view kViewsTable = "main.__bqemu_views";
constexpr absl::string_view kRoutinesTable = "main.__bqemu_routines";
constexpr absl::string_view kRowAccessPoliciesTable =
    "main.__bqemu_row_access_policies";
constexpr absl::string_view kColumnGovernanceTable =
    "main.__bqemu_column_governance";

std::string RoutineKindToString(RoutineKind kind);
absl::StatusOr<RoutineKind> RoutineKindFromString(absl::string_view s);

std::string RenderQuotedJsonStrings(const std::vector<std::string>& values);
std::vector<std::string> ParseQuotedJsonStringArray(absl::string_view json);
absl::StatusOr<std::string> ParseJsonStringField(absl::string_view json,
                                                 absl::string_view field);
bool ParseJsonBoolField(absl::string_view json, absl::string_view field);
std::string RenderViewRecordJson(const ViewRecord& rec);
absl::StatusOr<ViewRecord> ParseViewRecordJson(absl::string_view json,
                                               const DatasetId& ds);
std::string RenderRoutineRecordJson(const RoutineRecord& rec);
absl::StatusOr<RoutineRecord> ParseRoutineRecordJson(absl::string_view json,
                                                     const DatasetId& ds);
std::vector<absl::string_view> SplitTopLevelJsonObjects(
    absl::string_view array_json);
std::string RenderGovernanceSnapshotJson(
    const std::vector<std::pair<TableId, TableGovernance>>& tables);
DataMaskKind MaskKindFromString(absl::string_view s);

absl::Status RunDatasetScopedDelete(DuckDBStorage::Impl* impl,
                                    absl::string_view table,
                                    const DatasetId& id);
absl::StatusOr<std::vector<ViewRecord>> QueryViewsForDataset(
    DuckDBStorage::Impl* impl, const DatasetId& id);
absl::StatusOr<std::vector<RoutineRecord>> QueryRoutinesForDataset(
    DuckDBStorage::Impl* impl, const DatasetId& id);
std::vector<std::string> ListTableIdsInDatasetDir(
    const std::filesystem::path& ds_dir);

absl::Status UpsertViewLocked(DuckDBStorage::Impl* impl,
                              absl::string_view data_dir,
                              const ViewRecord& record);
absl::Status UpsertRoutineLocked(DuckDBStorage::Impl* impl,
                                 const RoutineRecord& record);

absl::Status RestoreGovernanceFromSnapshotLocked(DuckDBStorage::Impl* impl,
                                                 const DatasetId& ds,
                                                 absl::string_view json);

}  // namespace tombstone_registry_internal
}  // namespace duckdb
}  // namespace storage
}  // namespace backend
}  // namespace bigquery_emulator

#endif  // BIGQUERY_EMULATOR_BACKEND_STORAGE_DUCKDB_DUCKDB_STORAGE_TOMBSTONE_REGISTRY_INTERNAL_H_
