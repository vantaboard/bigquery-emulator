#ifndef BIGQUERY_EMULATOR_BACKEND_ENGINE_COORDINATOR_STUB_EXECUTORS_H_
#define BIGQUERY_EMULATOR_BACKEND_ENGINE_COORDINATOR_STUB_EXECUTORS_H_

// Placeholder executors for the three non-DuckDB routes the
// `RouteClassifier` can pick today. Each one returns
// `absl::UnimplementedError` with a disposition-aware message that
// points the operator at the downstream plan that fills in the real
// behavior (see
// `.cursor/plans/engine-router-foundation.plan.md` step 4 -- the
// stubs make the coordinator a complete dispatcher today while the
// real implementations land in:
//
//   * `semantic-executor-core.plan.md` -- `SemanticExecutor`
//   * `control-op-executor.plan.md`   -- `ControlOpExecutor`
//   * `specialized-feature-policy.plan.md` -- `UnsupportedExecutor`
//
// Until those plans land, the coordinator hands every query the
// `RouteClassifier` routes off DuckDB straight into one of these
// stubs. The stubs never mutate state and never re-enter the DuckDB
// path; "single planned route per shape" is enforced at the
// coordinator level, not at the executor level.

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
namespace coordinator {

// SemanticExecutor handles the `kSemanticExecutor` route -- queries
// the classifier flagged as needing a BigQuery-exact semantic
// implementation (SAFE_DIVIDE, SAFE_CAST, ...). All methods return
// UNIMPLEMENTED with a pointer at `semantic-executor-core.plan.md`
// for now.
class SemanticExecutor : public Executor {
 public:
  SemanticExecutor() = default;
  ~SemanticExecutor() override;

  SemanticExecutor(const SemanticExecutor&) = delete;
  SemanticExecutor& operator=(const SemanticExecutor&) = delete;

  [[nodiscard]] absl::StatusOr<std::unique_ptr<RowSource>> ExecuteQuery(
      const QueryRequest& request,
      const ::googlesql::ResolvedStatement& stmt,
      ::googlesql::Catalog* catalog) override;

  [[nodiscard]] absl::StatusOr<DmlStats> ExecuteDml(
      const QueryRequest& request,
      const ::googlesql::ResolvedStatement& stmt,
      ::googlesql::Catalog* catalog) override;

  [[nodiscard]] absl::Status ExecuteDdl(
      const QueryRequest& request,
      const ::googlesql::ResolvedStatement& stmt,
      ::googlesql::Catalog* catalog) override;
};

// ControlOpExecutor handles the `kControlOp` route -- DDL roots,
// transaction-control statements, session-state statements, and any
// other "control operation" that mutates engine / catalog state
// without producing a row stream. All methods return UNIMPLEMENTED
// with a pointer at `control-op-executor.plan.md` for now.
class ControlOpExecutor : public Executor {
 public:
  ControlOpExecutor() = default;
  ~ControlOpExecutor() override;

  ControlOpExecutor(const ControlOpExecutor&) = delete;
  ControlOpExecutor& operator=(const ControlOpExecutor&) = delete;

  [[nodiscard]] absl::StatusOr<std::unique_ptr<RowSource>> ExecuteQuery(
      const QueryRequest& request,
      const ::googlesql::ResolvedStatement& stmt,
      ::googlesql::Catalog* catalog) override;

  [[nodiscard]] absl::StatusOr<DmlStats> ExecuteDml(
      const QueryRequest& request,
      const ::googlesql::ResolvedStatement& stmt,
      ::googlesql::Catalog* catalog) override;

  [[nodiscard]] absl::Status ExecuteDdl(
      const QueryRequest& request,
      const ::googlesql::ResolvedStatement& stmt,
      ::googlesql::Catalog* catalog) override;
};

// UnsupportedExecutor handles the `kUnsupported` route -- any
// statement / node / function the classifier sees flagged as
// `unsupported` in the disposition registry. The reply identifies
// the offending node so the operator can find the right downstream
// plan; the catch-all owner is `specialized-feature-policy.plan.md`.
class UnsupportedExecutor : public Executor {
 public:
  UnsupportedExecutor() = default;
  ~UnsupportedExecutor() override;

  UnsupportedExecutor(const UnsupportedExecutor&) = delete;
  UnsupportedExecutor& operator=(const UnsupportedExecutor&) = delete;

  [[nodiscard]] absl::StatusOr<std::unique_ptr<RowSource>> ExecuteQuery(
      const QueryRequest& request,
      const ::googlesql::ResolvedStatement& stmt,
      ::googlesql::Catalog* catalog) override;

  [[nodiscard]] absl::StatusOr<DmlStats> ExecuteDml(
      const QueryRequest& request,
      const ::googlesql::ResolvedStatement& stmt,
      ::googlesql::Catalog* catalog) override;

  [[nodiscard]] absl::Status ExecuteDdl(
      const QueryRequest& request,
      const ::googlesql::ResolvedStatement& stmt,
      ::googlesql::Catalog* catalog) override;
};

}  // namespace coordinator
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator

#endif  // BIGQUERY_EMULATOR_BACKEND_ENGINE_COORDINATOR_STUB_EXECUTORS_H_
