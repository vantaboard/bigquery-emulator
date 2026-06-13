#ifndef BIGQUERY_EMULATOR_BACKEND_STORAGE_DUCKDB_DUCKDB_STORAGE_GOVERNANCE_H_
#define BIGQUERY_EMULATOR_BACKEND_STORAGE_DUCKDB_DUCKDB_STORAGE_GOVERNANCE_H_

// Governance persistence helpers for DuckDBStorage (row-access
// policies + column policy tags / masking metadata).

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

absl::Status EnsureGovernanceTables(DuckDBStorage::Impl* impl);

absl::StatusOr<TableGovernance> LoadTableGovernance(DuckDBStorage::Impl* impl,
                                                    const TableId& id);

absl::Status SaveRowAccessPolicies(DuckDBStorage::Impl* impl,
                                   const TableId& id,
                                   const TableGovernance& gov);

absl::Status SaveColumnGovernance(DuckDBStorage::Impl* impl,
                                  const TableId& id,
                                  absl::string_view column_name,
                                  const ColumnGovernanceRecord& column);

}  // namespace duckdb
}  // namespace storage
}  // namespace backend
}  // namespace bigquery_emulator

#endif  // BIGQUERY_EMULATOR_BACKEND_STORAGE_DUCKDB_DUCKDB_STORAGE_GOVERNANCE_H_
