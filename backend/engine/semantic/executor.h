#ifndef BIGQUERY_EMULATOR_BACKEND_ENGINE_SEMANTIC_EXECUTOR_H_
#define BIGQUERY_EMULATOR_BACKEND_ENGINE_SEMANTIC_EXECUTOR_H_

// `SemanticExecutor` is the local row/array/value interpreter the
// `LocalCoordinatorEngine` dispatches to whenever
// `RouteClassifier::Classify` picks the `kSemanticExecutor` route.
//
// Today's surface (per
// `.cursor/plans/semantic-executor-core.plan.md`) is scalar-only
// SELECT: a `ResolvedQueryStmt` whose `query()` is a
// `ResolvedProjectScan` over a `ResolvedSingleRowScan` (no FROM
// clause). The executor walks the resolved AST, evaluates each
// output column's expression via `EvalExpr`, converts the result
// into `storage::Value`, and streams a single one-row Arrow batch.
// Downstream plans add FROM-clause shapes that compose with
// `DuckDbExecutor` through a `RowSource` adapter.

#include <memory>

#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "backend/engine/coordinator/executor.h"
#include "backend/engine/engine.h"

namespace googlesql {
class Catalog;
class ResolvedStatement;
}  // namespace googlesql

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace semantic {

// `SemanticExecutor` implements `coordinator::Executor` and so
// installs cleanly into the coordinator's per-route dispatch
// table. The class is stateless beyond the executor interface; the
// per-request state (parameter bindings, output schema, ...) lives
// on the call stack inside `ExecuteQuery`.
class SemanticExecutor : public coordinator::Executor {
 public:
  SemanticExecutor() = default;
  ~SemanticExecutor() override;

  SemanticExecutor(const SemanticExecutor&) = delete;
  SemanticExecutor& operator=(const SemanticExecutor&) = delete;

  [[nodiscard]] absl::StatusOr<std::unique_ptr<RowSource>> ExecuteQuery(
      const QueryRequest& request,
      const ::googlesql::ResolvedStatement& stmt,
      ::googlesql::Catalog* catalog) override;

  // DML / DDL are owned by `dml-local-executor.plan.md` and the
  // control-op executor respectively. The semantic executor today
  // surfaces NOT_IMPLEMENTED for both, so the coordinator's
  // dispatch table doesn't have to special-case the missing
  // surfaces.
  [[nodiscard]] absl::StatusOr<DmlStats> ExecuteDml(
      const QueryRequest& request,
      const ::googlesql::ResolvedStatement& stmt,
      ::googlesql::Catalog* catalog) override;

  [[nodiscard]] absl::Status ExecuteDdl(
      const QueryRequest& request,
      const ::googlesql::ResolvedStatement& stmt,
      ::googlesql::Catalog* catalog) override;
};

}  // namespace semantic
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator

#endif  // BIGQUERY_EMULATOR_BACKEND_ENGINE_SEMANTIC_EXECUTOR_H_
