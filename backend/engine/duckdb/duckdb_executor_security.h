#ifndef BIGQUERY_EMULATOR_BACKEND_ENGINE_DUCKDB_DUCKDB_EXECUTOR_SECURITY_H_
#define BIGQUERY_EMULATOR_BACKEND_ENGINE_DUCKDB_DUCKDB_EXECUTOR_SECURITY_H_

#include <optional>
#include <string>
#include <vector>

#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "absl/strings/string_view.h"
#include "backend/catalog/table_governance.h"
#include "backend/schema/schema.h"
#include "backend/storage/storage.h"
#include "backend/storage/table_governance.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace duckdb {
namespace internal {

struct OutputColumnMask {
  catalog::DataMaskKind mask = catalog::DataMaskKind::kNone;
  catalog::ColumnGovernanceRecord governance;
  schema::ColumnType column_type = schema::ColumnType::kUnknown;
};

// Builds per-output-column masks by matching result column names to
// governed table columns from `table_governance`.
catalog::TableGovernance StorageGovernanceToCatalog(
    const storage::TableGovernance& storage_gov);

std::vector<OutputColumnMask> BuildOutputColumnMasks(
    const schema::TableSchema& output_schema,
    const catalog::TableGovernance& table_governance,
    absl::string_view principal_email);

// Applies row/column governance to one rendered row.
absl::Status ApplyOutputColumnMasks(absl::Span<const OutputColumnMask> masks,
                                    storage::Row* row);

}  // namespace internal
}  // namespace duckdb
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator

#endif  // BIGQUERY_EMULATOR_BACKEND_ENGINE_DUCKDB_DUCKDB_EXECUTOR_SECURITY_H_
