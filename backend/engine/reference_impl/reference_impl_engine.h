#ifndef BIGQUERY_EMULATOR_BACKEND_ENGINE_REFERENCE_IMPL_REFERENCE_IMPL_ENGINE_H_
#define BIGQUERY_EMULATOR_BACKEND_ENGINE_REFERENCE_IMPL_REFERENCE_IMPL_ENGINE_H_

// ReferenceImplEngine is the GoogleSQL reference-impl execution
// backend. It implements `Engine::Analyze` / `DryRun` / `ExecuteQuery`
// by driving the analyzer's `AnalyzeStatement` and the reference-impl
// `PreparedQuery` evaluator. Row iteration is delegated to a
// `Storage`-backed `googlesql::Table` adapter (`backend/catalog/storage_table`)
// that the supplied `Catalog` materializes when the analyzer touches
// it.
//
// The constructor takes a non-owning `Storage*` even though the
// engine itself never reads rows directly -- the storage pointer
// flows through the catalog the gateway hands to `ExecuteQuery`. We
// keep the parameter because the engine is constructed once at
// startup and the storage handle gives it a stable lifetime
// reference for future plans that wire async cancellation.
//
// Lifetime contract for the `Catalog*` parameter on each method:
// the gateway constructs a `backend::catalog::GoogleSqlCatalog` per
// `Query.*` RPC, sized for that request's project context, and
// destroys it once the engine call returns. The engine MUST NOT
// retain raw pointers into the catalog past the returned
// `AnalyzedQuery` / `RowSource`'s lifetime; both wrappers carry
// internal references to the catalog through GoogleSQL's analyzer
// output and prepared-query state so the catalog has to outlive
// them. The gateway plan (`execute-query-stream_y0b1c2d3.plan.md`)
// makes that ordering explicit.

#include <memory>

#include "absl/status/statusor.h"
#include "backend/engine/engine.h"
#include "backend/storage/storage.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace reference_impl {

class ReferenceImplEngine : public Engine {
 public:
  // `storage` must outlive this engine instance. The scaffold does not
  // dereference the pointer; Phase 5.A wires it through a
  // `googlesql::Table` adapter.
  explicit ReferenceImplEngine(storage::Storage* storage);
  ~ReferenceImplEngine() override;

  ReferenceImplEngine(const ReferenceImplEngine&) = delete;
  ReferenceImplEngine& operator=(const ReferenceImplEngine&) = delete;

  absl::StatusOr<std::unique_ptr<AnalyzedQuery>> Analyze(
      const QueryRequest& request, googlesql::Catalog* catalog) override;

  absl::StatusOr<DryRunResult> DryRun(
      const QueryRequest& request, googlesql::Catalog* catalog) override;

  absl::StatusOr<std::unique_ptr<RowSource>> ExecuteQuery(
      const QueryRequest& request, googlesql::Catalog* catalog) override;

  // Execute a DML statement (Phase 6a: only INSERT is wired today;
  // UPDATE / DELETE / MERGE return UNIMPLEMENTED). The handler in
  // `frontend/handlers/query.cc` classifies the analyzed AST and
  // routes here when it sees `RESOLVED_INSERT_STMT`.
  //
  // `catalog` MUST be a `backend::catalog::GoogleSqlCatalog`
  // wrapping `storage_`: the engine recovers the destination
  // `storage::TableId` by `dynamic_cast`-ing the
  // `ResolvedTableScan::table()` pointer back to a
  // `backend::catalog::StorageTable*` (which the catalog adapter
  // hands out for every materialized table). A non-`StorageTable`
  // table pointer surfaces as `FAILED_PRECONDITION`.
  absl::StatusOr<DmlStats> ExecuteDml(const QueryRequest& request,
                                      googlesql::Catalog* catalog) override;

  // DDL is intentionally UNIMPLEMENTED on the reference-impl engine.
  // Plan 35's engine-policy decision (extending HANDOFF.md §4.3
  // path 3's "DuckDB-only MERGE" pattern to cover DDL) lands
  // CREATE / DROP / ALTER on the DuckDB engine; the reference-impl
  // path stays UNIMPLEMENTED so the FallbackEngine wrapper routes
  // DDL to DuckDB without callers having to grep status messages
  // for the kind. The conformance harness in plans 40-42 will
  // surface any divergence between the two engines on DDL fixtures.
  absl::Status ExecuteDdl(const QueryRequest& request,
                          googlesql::Catalog* catalog) override;

 private:
  storage::Storage* storage_;  // not owned
};

}  // namespace reference_impl
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator

#endif  // BIGQUERY_EMULATOR_BACKEND_ENGINE_REFERENCE_IMPL_REFERENCE_IMPL_ENGINE_H_
