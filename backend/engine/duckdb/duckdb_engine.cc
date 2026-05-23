#include "backend/engine/duckdb/duckdb_engine.h"

#include <memory>

#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "backend/engine/engine.h"
#include "backend/storage/storage.h"

#ifdef BIGQUERY_EMULATOR_HAS_DUCKDB
#include "duckdb.h"
#endif

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace duckdb {

namespace {

// Returns the DuckDB C-API library version string when the build has
// linked libduckdb, or a stub when it hasn't. Beyond a sanity check,
// this also forces the linker to keep libduckdb.so on the DT_NEEDED
// list under `--as-needed` (otherwise the engine wouldn't reference
// any DuckDB symbol until Phase 5.B and the .so would be dropped from
// the final binary).
const char* DuckDBLibraryVersionOrStub() {
#ifdef BIGQUERY_EMULATOR_HAS_DUCKDB
  return ::duckdb_library_version();
#else
  return "<duckdb-disabled>";
#endif
}

}  // namespace

DuckDBEngine::DuckDBEngine(storage::Storage* storage) : storage_(storage) {
  // Pulls the DuckDB symbol into the link line; the value itself is
  // discarded today, Phase 5.B will surface it on /healthz / debug
  // logs once the transpiler lands.
  (void)DuckDBLibraryVersionOrStub();
}

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
