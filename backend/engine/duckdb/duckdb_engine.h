#ifndef BIGQUERY_EMULATOR_BACKEND_ENGINE_DUCKDB_DUCKDB_ENGINE_H_
#define BIGQUERY_EMULATOR_BACKEND_ENGINE_DUCKDB_DUCKDB_ENGINE_H_

// `DuckDBEngine` is a thin `Engine`-surface shim that owns the
// analyzer plumbing and forwards execution to a `DuckDbExecutor`
// (see `duckdb_executor.h`). The transpiler / DuckDB connection /
// Arrow result-row path lives on the executor now; this class only
// exists until the `LocalCoordinatorEngine` introduced by
// `.cursor/plans/engine-router-foundation.plan.md` lands and the
// `emulator_main` factory is rewired to construct the coordinator
// directly (plan step 6).
//
// The constructor takes a non-owning `Storage*` and threads it
// through to the executor. The pointer must outlive this instance.

#include <memory>

#include "absl/status/statusor.h"
#include "backend/engine/duckdb/duckdb_executor.h"
#include "backend/engine/engine.h"
#include "backend/storage/storage.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace duckdb {

class DuckDBEngine : public Engine {
 public:
  // `storage` must outlive this engine instance. The pointer is
  // forwarded to the underlying `DuckDbExecutor`, which dereferences
  // it from the DML / DDL paths (the SELECT path only reads through
  // the analyzer catalog).
  explicit DuckDBEngine(storage::Storage* storage);
  ~DuckDBEngine() override;

  DuckDBEngine(const DuckDBEngine&) = delete;
  DuckDBEngine& operator=(const DuckDBEngine&) = delete;

  absl::StatusOr<std::unique_ptr<AnalyzedQuery>> Analyze(
      const QueryRequest& request, googlesql::Catalog* catalog) override;

  absl::StatusOr<DryRunResult> DryRun(const QueryRequest& request,
                                      googlesql::Catalog* catalog) override;

  absl::StatusOr<std::unique_ptr<RowSource>> ExecuteQuery(
      const QueryRequest& request, googlesql::Catalog* catalog) override;

  // DML. The DuckDB executor implements MERGE end-to-end; INSERT,
  // UPDATE, and DELETE currently return UNIMPLEMENTED so callers see
  // a stable status code when they land on DuckDB.
  absl::StatusOr<DmlStats> ExecuteDml(const QueryRequest& request,
                                      googlesql::Catalog* catalog) override;

  // DDL. Implements CREATE TABLE, CREATE TABLE AS SELECT, DROP
  // TABLE, and ALTER TABLE ADD COLUMN by analyzing the GoogleSQL
  // statement and dispatching to the underlying `DuckDbExecutor`.
  absl::Status ExecuteDdl(const QueryRequest& request,
                          googlesql::Catalog* catalog) override;

 private:
  storage::Storage* storage_;  // not owned
  DuckDbExecutor executor_;
};

}  // namespace duckdb
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator

#endif  // BIGQUERY_EMULATOR_BACKEND_ENGINE_DUCKDB_DUCKDB_ENGINE_H_
