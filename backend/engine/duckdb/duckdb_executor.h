#ifndef BIGQUERY_EMULATOR_BACKEND_ENGINE_DUCKDB_DUCKDB_EXECUTOR_H_
#define BIGQUERY_EMULATOR_BACKEND_ENGINE_DUCKDB_DUCKDB_EXECUTOR_H_

// `DuckDbExecutor` is the `coordinator::Executor` adapter that
// drives the DuckDB-backed fast path. It carries the same transpiler
// / per-query DuckDB connection / Arrow result-row logic that lived
// on `DuckDBEngine` before the local coordinator refactor; the only
// observable shift is that the executor consumes an
// already-analyzed `ResolvedStatement` (the coordinator owns
// analysis and route classification) instead of re-parsing the SQL
// itself.
//
// `DuckDBEngine` (still defined in `duckdb_engine.{h,cc}`) keeps the
// public `Engine` surface for now and forwards its `ExecuteQuery` /
// `ExecuteDml` / `ExecuteDdl` calls into this executor; the
// `LocalCoordinatorEngine` introduced by
// `docs/ENGINE_POLICY.md` step 5 will
// dispatch to this executor directly and the `DuckDBEngine`
// top-level shim will be deleted (step 6).

#include <memory>

#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "backend/engine/coordinator/executor.h"
#include "backend/engine/engine.h"
#include "backend/storage/storage.h"

namespace googlesql {
class Catalog;
class ResolvedStatement;
}  // namespace googlesql

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace duckdb {

class DuckDbExecutor : public coordinator::Executor {
 public:
  // `storage` must outlive the executor instance. Required by the
  // DML and DDL paths (the SELECT path tolerates a null storage; it
  // only reads through the `Catalog` the coordinator hands in).
  explicit DuckDbExecutor(storage::Storage* storage);
  ~DuckDbExecutor() override;

  DuckDbExecutor(const DuckDbExecutor&) = delete;
  DuckDbExecutor& operator=(const DuckDbExecutor&) = delete;

  // Execute a `RESOLVED_QUERY_STMT`. Rejects other statement kinds
  // with INVALID_ARGUMENT so the coordinator's dispatch contract
  // (one route per statement) is enforced defensively. The
  // returned `RowSource` owns its DuckDB connection + result; the
  // resolved AST only has to outlive the executor call.
  [[nodiscard]] absl::StatusOr<std::unique_ptr<RowSource>> ExecuteQuery(
      const QueryRequest& request,
      const ::googlesql::ResolvedStatement& stmt,
      ::googlesql::Catalog* catalog) override;

  // Execute a `RESOLVED_MERGE_STMT`. INSERT / UPDATE / DELETE land
  // here as a future plan; today they return UNIMPLEMENTED so the
  // gateway surfaces BigQuery's `notImplemented` reason.
  [[nodiscard]] absl::StatusOr<DmlStats> ExecuteDml(
      const QueryRequest& request,
      const ::googlesql::ResolvedStatement& stmt,
      ::googlesql::Catalog* catalog) override;

  // Execute a DDL statement (CREATE TABLE / CREATE TABLE AS SELECT
  // / DROP TABLE / ALTER TABLE ADD COLUMN). Other DDL shapes surface
  // UNIMPLEMENTED.
  [[nodiscard]] absl::Status ExecuteDdl(
      const QueryRequest& request,
      const ::googlesql::ResolvedStatement& stmt,
      ::googlesql::Catalog* catalog) override;

 private:
  storage::Storage* storage_;  // not owned
};

}  // namespace duckdb
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator

#endif  // BIGQUERY_EMULATOR_BACKEND_ENGINE_DUCKDB_DUCKDB_EXECUTOR_H_
