#ifndef BIGQUERY_EMULATOR_BACKEND_ENGINE_COORDINATOR_LOCAL_COORDINATOR_ANALYZE_H_
#define BIGQUERY_EMULATOR_BACKEND_ENGINE_COORDINATOR_LOCAL_COORDINATOR_ANALYZE_H_

// Analyzer plumbing for `LocalCoordinatorEngine`. Split out to keep
// the coordinator implementation under the file-length lint cap.

#include <memory>

#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "absl/strings/string_view.h"
#include "backend/engine/engine.h"
#include "googlesql/public/analyzer_options.h"
#include "googlesql/public/analyzer_output.h"
#include "proto/emulator.pb.h"

namespace googlesql {
class Catalog;
class Type;
}  // namespace googlesql

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace coordinator {

absl::Status ValidateRequest(const QueryRequest& request,
                             const ::googlesql::Catalog* catalog);

absl::Status PopulateParameterOptions(const QueryRequest& request,
                                      ::googlesql::AnalyzerOptions& options);

absl::Status PopulateAnalyzerParameters(const QueryRequest& request,
                                        ::googlesql::AnalyzerOptions& options);

absl::StatusOr<const ::googlesql::Type*> ParameterTypeForKind(
    absl::string_view type_kind_name);

absl::StatusOr<std::unique_ptr<const ::googlesql::AnalyzerOutput>>
AnalyzeStatementImpl(const QueryRequest& request,
                     ::googlesql::Catalog* catalog,
                     bool all_statements);

absl::StatusOr<std::unique_ptr<AnalyzedQuery>> AnalyzeSelectQuery(
    const QueryRequest& request, ::googlesql::Catalog* catalog);

}  // namespace coordinator
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator

#endif  // BIGQUERY_EMULATOR_BACKEND_ENGINE_COORDINATOR_LOCAL_COORDINATOR_ANALYZE_H_
