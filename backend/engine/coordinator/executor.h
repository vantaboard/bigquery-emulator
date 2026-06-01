#ifndef BIGQUERY_EMULATOR_BACKEND_ENGINE_COORDINATOR_EXECUTOR_H_
#define BIGQUERY_EMULATOR_BACKEND_ENGINE_COORDINATOR_EXECUTOR_H_

// Route-typed executor adapter.
//
// `Executor` is the interface every per-route execution strategy
// implements. The `LocalCoordinatorEngine` (see
// `local_coordinator_engine.h`) owns one instance per route
// (`DuckDbExecutor`, `SemanticExecutor`, `ControlOpExecutor`,
// `UnsupportedExecutor`) and dispatches each query to the executor
// the `RouteClassifier` picked.
//
// Lifetime / threading
//
//   * Executors are constructed once at engine startup with a
//     non-owning `Storage*` and shared across every request.
//   * All methods must be thread-safe; the gRPC handler can call
//     them concurrently from multiple request threads.
//
// Why this is not the public `Engine` interface
//
//   * `Engine` is the wire-facing contract the gRPC handlers see
//     and the gateway depends on; it takes a `QueryRequest` (a SQL
//     string + bind metadata) and owns the analyzer plumbing
//     internally.
//   * `Executor` operates one level lower: the coordinator has
//     already analyzed the SQL into a `ResolvedStatement`, and the
//     classifier has already picked a route. Each executor then
//     handles its slice of the resolved-AST surface without
//     re-analyzing. The executor is decoupled from the wire
//     protocol so future routes (semantic, control-op) can ship
//     without touching `Engine` or the gRPC layer.
//
// The three methods mirror the `Engine` interface's execution
// methods one-for-one so the coordinator's dispatch table is a
// trivial forward; `Analyze` and `DryRun` stay on the coordinator
// because they do not pick a route (they only need the analyzer +
// schema reflection).

#include <memory>

#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "backend/engine/engine.h"

namespace googlesql {
class Catalog;
class ResolvedStatement;
}  // namespace googlesql

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace coordinator {

// Executor is the per-route execution-strategy interface the local
// coordinator dispatches to.
//
// Each executor consumes an already-analyzed `ResolvedStatement` and
// returns the same wire-facing reply shape the matching `Engine`
// method does. Executors that do not yet implement a given method
// return `absl::StatusCode::kUnimplemented` with a disposition-aware
// error message so the gRPC layer maps it to `UNIMPLEMENTED` for the
// gateway.
class Executor {
 public:
  virtual ~Executor();

  // Execute a SELECT-shaped (`RESOLVED_QUERY_STMT`) statement.
  // `stmt` is the resolved root; the lifetime of `stmt` must
  // outlast the call (the coordinator owns the analyzer output for
  // the duration of the call, and the returned `RowSource` is
  // detached from `stmt` once it leaves the executor).
  [[nodiscard]] virtual absl::StatusOr<std::unique_ptr<RowSource>>
  ExecuteQuery(const QueryRequest& request,
               const ::googlesql::ResolvedStatement& stmt,
               ::googlesql::Catalog* catalog) = 0;

  // Execute a DML statement (INSERT / UPDATE / DELETE / MERGE).
  [[nodiscard]] virtual absl::StatusOr<DmlStats> ExecuteDml(
      const QueryRequest& request,
      const ::googlesql::ResolvedStatement& stmt,
      ::googlesql::Catalog* catalog) = 0;

  // Execute a DDL statement (CREATE TABLE / DROP / ALTER / ...).
  [[nodiscard]] virtual absl::Status ExecuteDdl(
      const QueryRequest& request,
      const ::googlesql::ResolvedStatement& stmt,
      ::googlesql::Catalog* catalog) = 0;
};

}  // namespace coordinator
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator

#endif  // BIGQUERY_EMULATOR_BACKEND_ENGINE_COORDINATOR_EXECUTOR_H_
