#ifndef BIGQUERY_EMULATOR_BACKEND_ENGINE_COORDINATOR_STUB_EXECUTORS_H_
#define BIGQUERY_EMULATOR_BACKEND_ENGINE_COORDINATOR_STUB_EXECUTORS_H_

// Placeholder executors for the non-DuckDB / non-control-op routes
// the `RouteClassifier` can pick today. Each one returns
// `absl::UnimplementedError` with a disposition-aware message that
// points the operator at the downstream plan that fills in the real
// behavior (see
// `.cursor/plans/engine-router-foundation.plan.md` step 4 -- the
// stubs make the coordinator a complete dispatcher today while the
// real implementations land in:
//
//   * `local-exec-07-semantic-core-expr.plan.md` -- `SemanticExecutor`
//   * `local-exec-15-specialized-stubs.plan.md` -- `UnsupportedExecutor`
//
// `ControlOpExecutor` used to live here too, but
// `.cursor/plans/local-exec-01-ddl-catalog.plan.md` lifted it into its own
// `backend/engine/control/` package once it grew real per-statement
// handlers (CREATE TABLE / CTAS / DROP TABLE / ANALYZE today, with
// the rest of the control_op surface dispatched as
// UNIMPLEMENTED-with-pointer in the same per-statement table). The
// coordinator's `control_op_executor_` member now references that
// package, not this header.
//
// `SemanticExecutor` also graduated out of this stub file when
// `.cursor/plans/local-exec-07-semantic-core-expr.plan.md` landed -- the
// real local row/array/value interpreter lives at
// `backend/engine/semantic/executor.{h,cc}`. The coordinator's
// `semantic_executor_` member references the new package.
//
// Until the remaining plans land, the coordinator hands every query
// the `RouteClassifier` routes off DuckDB straight into one of
// these stubs. The stubs never mutate state and never re-enter the
// DuckDB path; "single planned route per shape" is enforced at the
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

// UnsupportedExecutor handles the `kUnsupported` route -- any
// statement / node / function the classifier sees flagged as
// `unsupported` in the disposition registry. The reply identifies
// the offending node so the operator can find the right downstream
// plan; the catch-all owner is `local-exec-15-specialized-stubs.plan.md`.
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
