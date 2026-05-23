#include "backend/engine/duckdb/duckdb_engine.h"

#include <memory>

#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "backend/engine/engine.h"
#include "backend/storage/storage.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace duckdb {

DuckDBEngine::DuckDBEngine(storage::Storage* storage) : storage_(storage) {}

DuckDBEngine::~DuckDBEngine() = default;

absl::StatusOr<std::unique_ptr<AnalyzedQuery>> DuckDBEngine::Analyze(
    const QueryRequest& /*request*/, googlesql::Catalog* /*catalog*/) {
  return absl::UnimplementedError(
      "duckdb engine: Analyze is not implemented yet (Phase 5.B)");
}

absl::StatusOr<DryRunResult> DuckDBEngine::DryRun(
    const QueryRequest& /*request*/, googlesql::Catalog* /*catalog*/) {
  return absl::UnimplementedError(
      "duckdb engine: DryRun is not implemented yet (Phase 5.B)");
}

absl::StatusOr<std::unique_ptr<RowSource>> DuckDBEngine::ExecuteQuery(
    const QueryRequest& /*request*/, googlesql::Catalog* /*catalog*/) {
  return absl::UnimplementedError(
      "duckdb engine: ExecuteQuery is not implemented yet (Phase 5.B)");
}

}  // namespace duckdb
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
