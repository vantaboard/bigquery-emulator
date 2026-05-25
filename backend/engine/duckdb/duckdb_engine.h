#ifndef BIGQUERY_EMULATOR_BACKEND_ENGINE_DUCKDB_DUCKDB_ENGINE_H_
#define BIGQUERY_EMULATOR_BACKEND_ENGINE_DUCKDB_DUCKDB_ENGINE_H_

// DuckDBEngine is the Phase 3c scaffold for the DuckDB-backed engine.
// Every `Engine` method returns `absl::UnimplementedError` so the CLI
// factory in `binaries/emulator_main/main.cc` can already construct
// the engine while the ZetaSQL → DuckDB SQL transpiler (see ROADMAP
// Phase 5.B) lands in a later plan.
//
// The constructor takes a non-owning `Storage*` because the DuckDB
// engine will attach the active storage backend's Parquet/Arrow files
// as DuckDB tables at query time. We thread the pointer through now so
// the scaffold compiles against the real interface and not a stub.

#include <memory>

#include "absl/status/statusor.h"
#include "backend/engine/engine.h"
#include "backend/storage/storage.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace duckdb {

class DuckDBEngine : public Engine {
 public:
  // `storage` must outlive this engine instance. The scaffold does not
  // dereference the pointer; Phase 5.B wires it into a per-query
  // DuckDB connection that attaches the storage's backing files.
  explicit DuckDBEngine(storage::Storage* storage);
  ~DuckDBEngine() override;

  DuckDBEngine(const DuckDBEngine&) = delete;
  DuckDBEngine& operator=(const DuckDBEngine&) = delete;

  absl::StatusOr<std::unique_ptr<AnalyzedQuery>> Analyze(
      const QueryRequest& request, googlesql::Catalog* catalog) override;

  absl::StatusOr<DryRunResult> DryRun(
      const QueryRequest& request, googlesql::Catalog* catalog) override;

  absl::StatusOr<std::unique_ptr<RowSource>> ExecuteQuery(
      const QueryRequest& request, googlesql::Catalog* catalog) override;

  // Plan-34 (DuckDB-only MERGE). Other DML kinds (INSERT/UPDATE/DELETE)
  // intentionally return UNIMPLEMENTED so the `FallbackEngine` wrapper
  // routes them to the reference-impl engine, which already runs them
  // through `PreparedModify`. MERGE lands here because the GoogleSQL
  // reference-impl algebrizer does not yet algebrize `ResolvedMergeStmt`
  // at the statement root; see the comment block at the matching switch
  // in `backend/engine/reference_impl/reference_impl_engine.cc` for the
  // engine-asymmetry rationale.
  absl::StatusOr<DmlStats> ExecuteDml(
      const QueryRequest& request, googlesql::Catalog* catalog) override;

  // Plan-35 (DuckDB-only DDL). Implements CREATE TABLE,
  // CREATE TABLE AS SELECT, DROP TABLE, and ALTER TABLE ADD COLUMN
  // by analyzing the GoogleSQL statement, mapping the resolved name
  // path to a `storage::TableId`, and driving the underlying
  // `Storage` (scan + rewrite where necessary for ALTER, plus a
  // per-query DuckDB connection for CTAS). The reference-impl
  // engine returns UNIMPLEMENTED so the `FallbackEngine` wrapper
  // routes DDL here; see the matching comment in
  // `backend/engine/reference_impl/reference_impl_engine.cc::
  // ExecuteDdl` for the engine-asymmetry rationale.
  absl::Status ExecuteDdl(const QueryRequest& request,
                          googlesql::Catalog* catalog) override;

 private:
  storage::Storage* storage_;  // not owned
};

}  // namespace duckdb
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator

#endif  // BIGQUERY_EMULATOR_BACKEND_ENGINE_DUCKDB_DUCKDB_ENGINE_H_
