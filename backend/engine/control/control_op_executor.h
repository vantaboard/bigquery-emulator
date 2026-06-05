#ifndef BIGQUERY_EMULATOR_BACKEND_ENGINE_CONTROL_CONTROL_OP_EXECUTOR_H_
#define BIGQUERY_EMULATOR_BACKEND_ENGINE_CONTROL_CONTROL_OP_EXECUTOR_H_

// `ControlOpExecutor` is the `coordinator::Executor` adapter for the
// `kControlOp` route. DDL roots, metadata refreshes, catalog
// mutations, and the EXPORT / pipe-EXPORT family land here -- not on
// the DuckDB SQL evaluator.
//
// Per `docs/ENGINE_POLICY.md`:
//
//   * DDL / metadata / catalog ops bypass the DuckDB SQL evaluator
//     entirely. The handler is responsible for both (a) the catalog
//     mutation (`Storage` + `googlesql::Catalog` adapter) AND (b) the
//     BigQuery-shaped `Job` envelope (`statistics.query.statementType`).
//   * Each per-statement handler owns its piece of the
//     `ResolvedStatement` surface and produces an `absl::Status` (DDL
//     /metadata) or future row-stream / DML reply (pipe variants
//     surface UNIMPLEMENTED today). Where DuckDB SQL is the cleanest
//     implementation primitive (e.g. CTAS lowers the inner SELECT to
//     DuckDB), the handler uses it as an implementation detail; the
//     handler still owns the response shape.
//   * The classifier-then-executor contract is one-shot: a
//     `kControlOp` row never silently falls back to the DuckDB SQL
//     evaluator. Shapes the executor cannot ship today surface
//     `UNIMPLEMENTED` with a message naming the deferred plan.
//
// The previous stub lived in
// `backend/engine/coordinator/stub_executors.{h,cc}`; the
// coordinator switched its `control_op_executor_` member to this
// type when this plan landed.
//
// Lifetime / threading
//
//   * Constructed once at engine startup with a non-owning
//     `Storage*`. The pointer is required by every catalog-mutating
//     handler (`CreateTable`, `DropTable`, ...).
//   * Shared across every gRPC request handler thread; all methods
//     are thread-safe (the analyzer call has already happened on the
//     coordinator and the executor only mutates `Storage`, which
//     serializes its own state).

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
namespace control {

class ControlOpExecutor : public coordinator::Executor {
 public:
  // `storage` must outlive the executor instance. Required by every
  // DDL handler because they mutate storage rows / schemas
  // directly.
  explicit ControlOpExecutor(storage::Storage* storage);
  ~ControlOpExecutor() override;

  ControlOpExecutor(const ControlOpExecutor&) = delete;
  ControlOpExecutor& operator=(const ControlOpExecutor&) = delete;

  // Control-op statements never produce a row stream and never
  // produce a DML stats summary; a coordinator dispatch into either
  // method is a routing bug. We surface INVALID_ARGUMENT (not
  // UNIMPLEMENTED) so the bug shows up loudly in tests.
  [[nodiscard]] absl::StatusOr<std::unique_ptr<RowSource>> ExecuteQuery(
      const QueryRequest& request,
      const ::googlesql::ResolvedStatement& stmt,
      ::googlesql::Catalog* catalog) override;

  [[nodiscard]] absl::StatusOr<DmlStats> ExecuteDml(
      const QueryRequest& request,
      const ::googlesql::ResolvedStatement& stmt,
      ::googlesql::Catalog* catalog) override;

  // Per-statement dispatch table for every DDL / metadata / catalog
  // statement classified to `kControlOp`. Handlers that have not yet
  // shipped surface `UNIMPLEMENTED` with the deferred plan named in
  // the message.
  [[nodiscard]] absl::Status ExecuteDdl(
      const QueryRequest& request,
      const ::googlesql::ResolvedStatement& stmt,
      ::googlesql::Catalog* catalog) override;

 private:
  storage::Storage* storage_;  // not owned
};

}  // namespace control
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator

#endif  // BIGQUERY_EMULATOR_BACKEND_ENGINE_CONTROL_CONTROL_OP_EXECUTOR_H_
