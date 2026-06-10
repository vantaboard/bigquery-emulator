#ifndef BIGQUERY_EMULATOR_BACKEND_ENGINE_SEMANTIC_EXECUTOR_H_
#define BIGQUERY_EMULATOR_BACKEND_ENGINE_SEMANTIC_EXECUTOR_H_

// `SemanticExecutor` is the local row/array/value interpreter the
// `LocalCoordinatorEngine` dispatches to whenever
// `RouteClassifier::Classify` picks the `kSemanticExecutor` route.
//
// Today's surface (per
// `docs/ENGINE_POLICY.md`) is scalar-only
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
#include "backend/engine/semantic/frame_stack.h"
#include "backend/storage/storage.h"
#include "googlesql/public/analyzer.h"

namespace googlesql {
class Catalog;
class ResolvedQueryStmt;
class ResolvedStatement;
}  // namespace googlesql

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace semantic {

// Execute an already-resolved SELECT with optional script-variable
// bindings (used by the coordinator script driver for final SELECT
// statements inside BEGIN..END blocks).
[[nodiscard]] absl::StatusOr<std::unique_ptr<RowSource>>
ExecuteResolvedQueryStmt(const QueryRequest& request,
                         const ::googlesql::ResolvedQueryStmt& query_stmt,
                         const FrameStack* script_variables = nullptr,
                         const ::googlesql::SystemVariableValuesMap*
                             script_system_variables = nullptr);

// `SemanticExecutor` implements `coordinator::Executor` and so
// installs cleanly into the coordinator's per-route dispatch
// table. Beyond the SELECT path the executor also owns the
// storage-aware DML routes (`backend/engine/semantic/dml/`,
// `docs/ENGINE_POLICY.md`); `storage` is a
// non-owning pointer passed through from the coordinator and used
// by the DML path to mutate target tables. `storage` may be null
// in unit tests that exercise only the SELECT path.
class SemanticExecutor : public coordinator::Executor {
 public:
  SemanticExecutor() = default;
  explicit SemanticExecutor(storage::Storage* storage) : storage_(storage) {}
  ~SemanticExecutor() override;

  SemanticExecutor(const SemanticExecutor&) = delete;
  SemanticExecutor& operator=(const SemanticExecutor&) = delete;

  [[nodiscard]] absl::StatusOr<std::unique_ptr<RowSource>> ExecuteQuery(
      const QueryRequest& request,
      const ::googlesql::ResolvedStatement& stmt,
      ::googlesql::Catalog* catalog) override;

  // DML routes through the storage-aware local DML executor in
  // `backend/engine/semantic/dml/`. Statements the DML executor
  // does not yet handle surface a structured `kNotImplemented`
  // so the gateway envelope is the same as for any other
  // "planned but not landed" route. DDL stays on the
  // control-op executor and the semantic executor surfaces a
  // clean error if it ever sees one.
  [[nodiscard]] absl::StatusOr<DmlStats> ExecuteDml(
      const QueryRequest& request,
      const ::googlesql::ResolvedStatement& stmt,
      ::googlesql::Catalog* catalog) override;

  [[nodiscard]] absl::Status ExecuteDdl(
      const QueryRequest& request,
      const ::googlesql::ResolvedStatement& stmt,
      ::googlesql::Catalog* catalog) override;

 private:
  storage::Storage* storage_ = nullptr;  // not owned; may be null
};

}  // namespace semantic
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator

#endif  // BIGQUERY_EMULATOR_BACKEND_ENGINE_SEMANTIC_EXECUTOR_H_
