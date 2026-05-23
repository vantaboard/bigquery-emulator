#include "backend/engine/reference_impl/reference_impl_engine.h"

#include <memory>

#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "backend/engine/engine.h"
#include "backend/storage/storage.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace reference_impl {

ReferenceImplEngine::ReferenceImplEngine(storage::Storage* storage)
    : storage_(storage) {}

ReferenceImplEngine::~ReferenceImplEngine() = default;

absl::StatusOr<std::unique_ptr<AnalyzedQuery>> ReferenceImplEngine::Analyze(
    const QueryRequest& /*request*/, googlesql::Catalog* /*catalog*/) {
  return absl::UnimplementedError(
      "reference_impl engine: Analyze is not implemented yet (Phase 5.A)");
}

absl::StatusOr<DryRunResult> ReferenceImplEngine::DryRun(
    const QueryRequest& /*request*/, googlesql::Catalog* /*catalog*/) {
  return absl::UnimplementedError(
      "reference_impl engine: DryRun is not implemented yet (Phase 5.A)");
}

absl::StatusOr<std::unique_ptr<RowSource>> ReferenceImplEngine::ExecuteQuery(
    const QueryRequest& /*request*/, googlesql::Catalog* /*catalog*/) {
  return absl::UnimplementedError(
      "reference_impl engine: ExecuteQuery is not implemented yet "
      "(Phase 5.A)");
}

}  // namespace reference_impl
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
