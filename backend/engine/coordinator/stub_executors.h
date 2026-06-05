#ifndef BIGQUERY_EMULATOR_BACKEND_ENGINE_COORDINATOR_STUB_EXECUTORS_H_
#define BIGQUERY_EMULATOR_BACKEND_ENGINE_COORDINATOR_STUB_EXECUTORS_H_

// Placeholder executor for the `kUnsupported` route. Returns
// `absl::UnimplementedError` with a disposition-aware message that
// points the operator at `docs/ENGINE_POLICY.md`.

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
