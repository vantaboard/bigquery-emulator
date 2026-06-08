#ifndef BIGQUERY_EMULATOR_BACKEND_ENGINE_COORDINATOR_LOCAL_COORDINATOR_ANALYZE_H_
#define BIGQUERY_EMULATOR_BACKEND_ENGINE_COORDINATOR_LOCAL_COORDINATOR_ANALYZE_H_

// Analyzer plumbing for `LocalCoordinatorEngine`. Split out to keep
// the coordinator implementation under the file-length lint cap.

#include <memory>

#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "absl/strings/string_view.h"
#include "backend/catalog/googlesql_catalog.h"
#include "backend/engine/engine.h"
#include "googlesql/public/analyzer_options.h"
#include "googlesql/public/analyzer_output.h"
#include "proto/emulator.pb.h"

namespace googlesql {
class Catalog;
class Type;
class TypeFactory;
}  // namespace googlesql

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace coordinator {

// Shared analyzer options for the coordinator and the gRPC frontend.
// Keeps rewrite toggles (PIVOT / UNPIVOT / pipe DDL) and literal-cast
// folding aligned so route classification matches execution.
::googlesql::AnalyzerOptions MakeCoordinatorAnalyzerOptions(
    bool all_statements = false);

absl::Status ValidateRequest(const QueryRequest& request,
                             const ::googlesql::Catalog* catalog);

absl::Status PopulateParameterOptions(const QueryRequest& request,
                                      ::googlesql::AnalyzerOptions& options,
                                      ::googlesql::TypeFactory* type_factory);

absl::Status PopulateAnalyzerParameters(const QueryRequest& request,
                                        ::googlesql::AnalyzerOptions& options,
                                        ::googlesql::TypeFactory* type_factory);

absl::StatusOr<const ::googlesql::Type*> ParameterTypeForKind(
    absl::string_view type_kind_name);

absl::StatusOr<const ::googlesql::Type*> ParameterTypeForQueryParameter(
    const QueryParameter& parameter, ::googlesql::TypeFactory* type_factory);

absl::StatusOr<std::unique_ptr<const ::googlesql::AnalyzerOutput>>
AnalyzeStatementImpl(const QueryRequest& request,
                     ::googlesql::Catalog* catalog,
                     bool all_statements);

// Build analyzer options for `request` (parameters + system variables).
absl::StatusOr<::googlesql::AnalyzerOptions> BuildAnalyzerOptionsForRequest(
    const QueryRequest& request,
    catalog::GoogleSqlCatalog* catalog,
    bool all_statements);

absl::StatusOr<std::unique_ptr<AnalyzedQuery>> AnalyzeSelectQuery(
    const QueryRequest& request, ::googlesql::Catalog* catalog);

}  // namespace coordinator
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator

#endif  // BIGQUERY_EMULATOR_BACKEND_ENGINE_COORDINATOR_LOCAL_COORDINATOR_ANALYZE_H_
