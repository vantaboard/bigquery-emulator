#include "backend/engine/fallback/fallback_engine.h"

#include <memory>
#include <utility>

#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "backend/engine/engine.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace fallback {

FallbackEngine::FallbackEngine(Engine* primary, Engine* fallback)
    : primary_(primary), fallback_(fallback) {}

FallbackEngine::~FallbackEngine() = default;

absl::StatusOr<std::unique_ptr<AnalyzedQuery>> FallbackEngine::Analyze(
    const QueryRequest& request, ::googlesql::Catalog* catalog) {
  if (primary_ == nullptr || fallback_ == nullptr) {
    return absl::FailedPreconditionError(
        "FallbackEngine: primary and fallback engines must be non-null");
  }
  absl::StatusOr<std::unique_ptr<AnalyzedQuery>> result =
      primary_->Analyze(request, catalog);
  if (result.ok()) return result;
  if (result.status().code() != absl::StatusCode::kUnimplemented) {
    return result;
  }
  return fallback_->Analyze(request, catalog);
}

absl::StatusOr<DryRunResult> FallbackEngine::DryRun(
    const QueryRequest& request, ::googlesql::Catalog* catalog) {
  if (primary_ == nullptr || fallback_ == nullptr) {
    return absl::FailedPreconditionError(
        "FallbackEngine: primary and fallback engines must be non-null");
  }
  absl::StatusOr<DryRunResult> result = primary_->DryRun(request, catalog);
  if (result.ok()) return result;
  if (result.status().code() != absl::StatusCode::kUnimplemented) {
    return result;
  }
  return fallback_->DryRun(request, catalog);
}

absl::StatusOr<std::unique_ptr<RowSource>> FallbackEngine::ExecuteQuery(
    const QueryRequest& request, ::googlesql::Catalog* catalog) {
  if (primary_ == nullptr || fallback_ == nullptr) {
    return absl::FailedPreconditionError(
        "FallbackEngine: primary and fallback engines must be non-null");
  }
  absl::StatusOr<std::unique_ptr<RowSource>> result =
      primary_->ExecuteQuery(request, catalog);
  if (result.ok()) return result;
  if (result.status().code() != absl::StatusCode::kUnimplemented) {
    return result;
  }
  return fallback_->ExecuteQuery(request, catalog);
}

}  // namespace fallback
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
