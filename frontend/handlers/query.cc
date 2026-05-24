#include "frontend/handlers/query.h"

#include <cstdint>
#include <memory>
#include <string>
#include <utility>

#include "absl/status/status.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/string_view.h"
#include "backend/storage/storage.h"

#if defined(BIGQUERY_EMULATOR_HAS_GOOGLESQL)
#include "backend/catalog/googlesql_catalog.h"
#include "backend/schema/googlesql_to_bq.h"
#include "googlesql/public/analyzer.h"
#include "googlesql/public/analyzer_options.h"
#include "googlesql/public/analyzer_output.h"
#include "googlesql/public/error_helpers.h"
#include "googlesql/public/error_location.pb.h"
#include "googlesql/public/language_options.h"
#include "googlesql/public/options.pb.h"
#include "googlesql/public/types/type_factory.h"
#include "googlesql/resolved_ast/resolved_ast.h"
#include "googlesql/resolved_ast/resolved_node_kind.pb.h"
#endif  // BIGQUERY_EMULATOR_HAS_GOOGLESQL

namespace bigquery_emulator {
namespace frontend {

namespace {

#if !defined(BIGQUERY_EMULATOR_HAS_GOOGLESQL)
constexpr char kAnalysisUnimplemented[] =
    "Query.DryRun is not implemented yet (Phase 4 of ROADMAP.md): this "
    "build was produced without GoogleSQL linked in";
#endif

constexpr char kExecutionUnimplemented[] =
    "Query.ExecuteQuery is not implemented yet (Phase 5 of ROADMAP.md)";

#if defined(BIGQUERY_EMULATOR_HAS_GOOGLESQL)
// Map an analyzer / parser error from `googlesql::AnalyzeStatement` to
// a gRPC status. GoogleSQL attaches an `ErrorLocation` payload to
// parse and analysis errors whose `line` / `column` fields are
// 1-based offsets into the original SQL string; we prefix the
// message with `line:column:` so the gateway (and any external
// debugger) can recover the location without re-parsing the payload.
//
// The mapping itself is intentionally narrow: any `INVALID_ARGUMENT`
// from analysis stays `INVALID_ARGUMENT` (the gateway folds it into
// BigQuery's HTTP 400 `reason: invalidQuery`); we forward anything
// else verbatim so storage-level errors (e.g. NotFound on a missing
// catalog table) preserve their original codes.
::grpc::Status AnalyzeStatusToGrpc(const absl::Status& status) {
  if (status.ok()) return ::grpc::Status::OK;
  ::grpc::StatusCode code = ::grpc::StatusCode::INTERNAL;
  switch (status.code()) {
    case absl::StatusCode::kInvalidArgument:
      code = ::grpc::StatusCode::INVALID_ARGUMENT;
      break;
    case absl::StatusCode::kNotFound:
      code = ::grpc::StatusCode::NOT_FOUND;
      break;
    case absl::StatusCode::kFailedPrecondition:
      code = ::grpc::StatusCode::FAILED_PRECONDITION;
      break;
    case absl::StatusCode::kUnimplemented:
      code = ::grpc::StatusCode::UNIMPLEMENTED;
      break;
    default:
      code = ::grpc::StatusCode::INTERNAL;
      break;
  }
  std::string message(status.message());
  ::googlesql::ErrorLocation location;
  if (::googlesql::GetErrorLocation(status, &location)) {
    message = absl::StrCat(location.line(), ":", location.column(), ": ",
                           message);
  }
  return ::grpc::Status(code, message);
}

// Builds an AnalyzerOptions configured the way BigQuery uses
// GoogleSQL: external product mode (NUMERIC / BIGNUMERIC /
// BIGNUMERIC dialects rather than internal scalar names),
// maximum language features (all GoogleSQL surface allowed since
// BigQuery itself enables everything), strict resolution mode.
::googlesql::AnalyzerOptions MakeAnalyzerOptions() {
  ::googlesql::LanguageOptions language;
  language.EnableMaximumLanguageFeatures();
  language.set_product_mode(::googlesql::PRODUCT_EXTERNAL);
  language.set_name_resolution_mode(::googlesql::NAME_RESOLUTION_DEFAULT);
  ::googlesql::AnalyzerOptions options(language);
  // Single-line error messages so the gRPC error string stays
  // one-line-friendly. The `attach_error_location_payload` flag is
  // what tells the analyzer to leave the ErrorLocation payload on
  // the returned `absl::Status`; without it `GetErrorLocation`
  // returns false even on parse errors.
  options.set_error_message_mode(::googlesql::ERROR_MESSAGE_ONE_LINE);
  options.set_attach_error_location_payload(true);
  options.CreateDefaultArenasIfNotSet();
  return options;
}
#endif  // BIGQUERY_EMULATOR_HAS_GOOGLESQL

}  // namespace

QueryService::QueryService(backend::storage::Storage* storage)
    : storage_(storage) {}

::grpc::Status QueryService::DryRun(::grpc::ServerContext* /*context*/,
                                    const v1::QueryRequest* request,
                                    v1::DryRunResponse* response) {
#if !defined(BIGQUERY_EMULATOR_HAS_GOOGLESQL)
  (void)request;
  (void)response;
  return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED,
                        kAnalysisUnimplemented);
#else
  if (response == nullptr) {
    return ::grpc::Status(::grpc::StatusCode::INTERNAL,
                          "QueryService::DryRun: response is null");
  }
  response->Clear();
  if (request == nullptr) {
    return ::grpc::Status(::grpc::StatusCode::INVALID_ARGUMENT,
                          "QueryService::DryRun: request is null");
  }
  if (storage_ == nullptr) {
    return ::grpc::Status(
        ::grpc::StatusCode::FAILED_PRECONDITION,
        "QueryService::DryRun: storage backend is not configured");
  }
  if (request->use_legacy_sql()) {
    return ::grpc::Status(
        ::grpc::StatusCode::INVALID_ARGUMENT,
        "QueryService::DryRun: useLegacySql=true is not supported; the "
        "emulator only implements GoogleSQL");
  }
  if (request->project_id().empty()) {
    return ::grpc::Status(
        ::grpc::StatusCode::INVALID_ARGUMENT,
        "QueryService::DryRun: request.project_id is required");
  }
  if (request->sql().empty()) {
    return ::grpc::Status(::grpc::StatusCode::INVALID_ARGUMENT,
                          "QueryService::DryRun: request.sql is required");
  }

  ::googlesql::TypeFactory type_factory;
  backend::catalog::GoogleSqlCatalog catalog(request->project_id(), storage_,
                                              &type_factory);

  ::googlesql::AnalyzerOptions options = MakeAnalyzerOptions();
  std::unique_ptr<const ::googlesql::AnalyzerOutput> output;
  absl::Status analyze = ::googlesql::AnalyzeStatement(
      request->sql(), options, &catalog, &type_factory, &output);
  if (!analyze.ok()) {
    return AnalyzeStatusToGrpc(analyze);
  }
  if (output == nullptr || output->resolved_statement() == nullptr) {
    return ::grpc::Status(::grpc::StatusCode::INTERNAL,
                          "QueryService::DryRun: analyzer returned no "
                          "resolved statement");
  }
  const ::googlesql::ResolvedStatement* stmt = output->resolved_statement();
  if (stmt->node_kind() != ::googlesql::RESOLVED_QUERY_STMT) {
    return ::grpc::Status(
        ::grpc::StatusCode::INVALID_ARGUMENT,
        absl::StrCat("QueryService::DryRun: only SELECT-shaped queries "
                     "are supported in DryRun; got ",
                     stmt->node_kind_string()));
  }
  const auto* query_stmt = stmt->GetAs<::googlesql::ResolvedQueryStmt>();
  absl::Status reflect = backend::schema::OutputColumnListToTableSchema(
      query_stmt->output_column_list(), response->mutable_schema());
  if (!reflect.ok()) {
    return AnalyzeStatusToGrpc(reflect);
  }

  // estimated_bytes_processed: BigQuery's dry-run statistic includes a
  // size estimate per referenced table, summed and rounded. The
  // emulator does not yet have a cost model, so we emit zero — the
  // schema is the contractual return value of DryRun; the byte
  // estimate is informational. Plan
  // `dryrun-gateway-e2e_w8f9a0b1.plan.md` will fold a per-table row-
  // count + per-column byte-width estimate in once the storage layer
  // exposes those.
  response->set_estimated_bytes_processed(static_cast<int64_t>(0));
  return ::grpc::Status::OK;
#endif  // BIGQUERY_EMULATOR_HAS_GOOGLESQL
}

::grpc::Status QueryService::ExecuteQuery(
    ::grpc::ServerContext* /*context*/, const v1::QueryRequest* /*request*/,
    ::grpc::ServerWriter<v1::QueryResultRow>* /*writer*/) {
  return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED,
                        kExecutionUnimplemented);
}

}  // namespace frontend
}  // namespace bigquery_emulator
