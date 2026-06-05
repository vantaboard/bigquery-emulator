#ifndef BIGQUERY_EMULATOR_BACKEND_ENGINE_COORDINATOR_LOCAL_COORDINATOR_ENGINE_H_
#define BIGQUERY_EMULATOR_BACKEND_ENGINE_COORDINATOR_LOCAL_COORDINATOR_ENGINE_H_

// `LocalCoordinatorEngine` is the top-level `Engine` implementation
// the emulator binary constructs. It owns the analyzer plumbing,
// the `RouteClassifier`, and one instance per route-typed
// `Executor`; for each request it:
//
//   1. Analyses `request.sql` against the supplied `Catalog`.
//   2. Hands the resolved root to `RouteClassifier::Classify`.
//   3. Dispatches to the matching `Executor` (DuckDB, semantic,
//      control-op, or unsupported) based on the chosen
//      `Disposition`.
//
// The coordinator never re-dispatches at runtime -- per
// `docs/ENGINE_POLICY.md` the
// classifier picks one route per shape and the coordinator honors
// that decision verbatim. A transpiler `UNIMPLEMENTED` inside the
// DuckDB executor is treated as a fast-path bug, not a routing
// retry signal.
//
// Lifetime / threading
//
//   * Constructed once at startup with a non-owning `Storage*` (the
//     same pointer the underlying `DuckDbExecutor` needs for DML /
//     DDL).
//   * Shared across every gRPC request handler thread; all methods
//     are thread-safe (the analyzer call is independent per
//     request, the classifier is stateless, and the executors are
//     either stateless or backed by thread-safe storage).

#include <memory>

#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "backend/engine/control/control_op_executor.h"
#include "backend/engine/coordinator/executor.h"
#include "backend/engine/coordinator/route_classifier.h"
#include "backend/engine/coordinator/stub_executors.h"
#include "backend/engine/duckdb/duckdb_executor.h"
#include "backend/engine/engine.h"
#include "backend/engine/semantic/executor.h"
#include "backend/storage/storage.h"
#include "googlesql/public/analyzer_options.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace coordinator {

class LocalCoordinatorEngine : public Engine {
 public:
  // `storage` must outlive the coordinator. The pointer is
  // forwarded to the underlying `DuckDbExecutor`; the other
  // executors are stateless stubs today.
  explicit LocalCoordinatorEngine(storage::Storage* storage);
  ~LocalCoordinatorEngine() override;

  LocalCoordinatorEngine(const LocalCoordinatorEngine&) = delete;
  LocalCoordinatorEngine& operator=(const LocalCoordinatorEngine&) = delete;

  absl::StatusOr<std::unique_ptr<AnalyzedQuery>> Analyze(
      const QueryRequest& request, ::googlesql::Catalog* catalog) override;

  absl::StatusOr<DryRunResult> DryRun(const QueryRequest& request,
                                      ::googlesql::Catalog* catalog) override;

  absl::StatusOr<std::unique_ptr<RowSource>> ExecuteQuery(
      const QueryRequest& request, ::googlesql::Catalog* catalog) override;

  absl::StatusOr<DmlStats> ExecuteDml(const QueryRequest& request,
                                      ::googlesql::Catalog* catalog) override;

  absl::Status ExecuteDdl(const QueryRequest& request,
                          ::googlesql::Catalog* catalog) override;

 private:
  // Pick the `Executor` the supplied resolved statement routes to.
  // Returns nullptr for the (theoretically unreachable) case where
  // the classifier reports an unknown disposition; callers map that
  // to INTERNAL.
  Executor* RouteFor(const ::googlesql::ResolvedStatement& stmt);

  RouteClassifier classifier_{};
  duckdb::DuckDbExecutor duckdb_executor_;
  semantic::SemanticExecutor semantic_executor_;
  control::ControlOpExecutor control_op_executor_;
  UnsupportedExecutor unsupported_executor_{};
};

// Wire `request.parameters` into `options` before `AnalyzeStatement`.
// Used by the coordinator and by `frontend/handlers/query.cc`'s
// pre-classify pass so DML with named parameters classifies correctly.
absl::Status PopulateAnalyzerParameters(const QueryRequest& request,
                                        ::googlesql::AnalyzerOptions& options);

}  // namespace coordinator
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator

#endif  // BIGQUERY_EMULATOR_BACKEND_ENGINE_COORDINATOR_LOCAL_COORDINATOR_ENGINE_H_
