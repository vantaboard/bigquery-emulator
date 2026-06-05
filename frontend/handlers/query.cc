#include "frontend/handlers/query.h"

#include <cstdint>
#include <functional>
#include <memory>
#include <string>
#include <utility>

#include "absl/status/status.h"
#include "absl/strings/str_cat.h"
#include "backend/catalog/googlesql_catalog.h"
#include "backend/catalog/udf_registry.h"
#include "backend/engine/coordinator/local_coordinator_engine.h"
#include "backend/engine/coordinator/route_classifier.h"
#include "backend/engine/disposition.h"
#include "backend/engine/engine.h"
#include "backend/engine/semantic/system_variables.h"
#include "backend/schema/googlesql_to_bq.h"
#include "backend/schema/schema.h"
#include "backend/storage/storage.h"
#include "frontend/handlers/query_internal.h"
#include "googlesql/public/analyzer.h"
#include "googlesql/public/analyzer_options.h"
#include "googlesql/public/analyzer_output.h"
#include "googlesql/public/types/type_factory.h"
#include "googlesql/resolved_ast/resolved_ast.h"

namespace bigquery_emulator {
namespace frontend {

using internal::AnalyzeStatusToGrpc;
using internal::ClassifyStatement;
using internal::EmitDdlResult;
using internal::EmitDmlStats;
using internal::MakeAnalyzerOptions;
using internal::ProtoToEngineRequest;
using internal::StatementClass;
using internal::StatementTypeFor;
using internal::StreamRows;
using internal::ValidateQueryRequest;

QueryService::QueryService(backend::storage::Storage* storage,
                           backend::engine::Engine* engine)
    : storage_(storage), engine_(engine) {}

::grpc::Status QueryService::DryRun(::grpc::ServerContext* /*context*/,
                                    const v1::QueryRequest* request,
                                    v1::DryRunResponse* response) {
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
  ::googlesql::AnalyzerOptions options = MakeAnalyzerOptions();
  backend::catalog::GoogleSqlCatalog catalog(request->project_id(),
                                             storage_,
                                             &type_factory,
                                             options.language(),
                                             request->default_dataset_id());

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
  const StatementClass cls = ClassifyStatement(stmt->node_kind());
  switch (cls) {
    case StatementClass::kSelect: {
      const auto* query_stmt = stmt->GetAs<::googlesql::ResolvedQueryStmt>();
      absl::Status reflect = backend::schema::OutputColumnListToTableSchema(
          query_stmt->output_column_list(), response->mutable_schema());
      if (!reflect.ok()) {
        return AnalyzeStatusToGrpc(reflect);
      }
      break;
    }
    case StatementClass::kDml:
      // BigQuery's `jobs.query?dryRun=true` for DML returns a
      // completed reply with no schema and `totalBytesProcessed=0`
      // (since dry-run doesn't run the statement). We mirror that:
      // the schema is left empty and the byte estimate is set
      // below.
      break;
    case StatementClass::kDdl:
      return ::grpc::Status(
          ::grpc::StatusCode::UNIMPLEMENTED,
          absl::StrCat("QueryService::DryRun: DDL statements are not "
                       "implemented yet; got ",
                       stmt->node_kind_string()));
    case StatementClass::kOther:
      return ::grpc::Status(
          ::grpc::StatusCode::UNIMPLEMENTED,
          absl::StrCat("QueryService::DryRun: statement kind ",
                       stmt->node_kind_string(),
                       " is not supported by the emulator"));
  }

  // estimated_bytes_processed: BigQuery's dry-run statistic includes a
  // size estimate per referenced table, summed and rounded. The
  // emulator does not yet have a cost model, so we emit zero — the
  // schema is the contractual return value of DryRun; the byte
  // estimate is informational. Plan
  // `docs/ENGINE_POLICY.md` will fold a per-table row-
  // count + per-column byte-width estimate in once the storage layer
  // exposes those.
  response->set_estimated_bytes_processed(static_cast<int64_t>(0));
  return ::grpc::Status::OK;
}

::grpc::Status QueryService::ExecuteQuery(
    ::grpc::ServerContext* /*context*/,
    const v1::QueryRequest* request,
    ::grpc::ServerWriter<v1::QueryResultRow>* writer) {
  if (writer == nullptr) {
    return ::grpc::Status(::grpc::StatusCode::INTERNAL,
                          "QueryService::ExecuteQuery: writer is null");
  }
  if (request == nullptr) {
    return ::grpc::Status(::grpc::StatusCode::INVALID_ARGUMENT,
                          "QueryService::ExecuteQuery: request is null");
  }
  // The lambda is invoked synchronously by StreamQueryResults; no
  // need to capture state by value.
  return StreamQueryResults(
      storage_,
      *request,
      [writer](const v1::QueryResultRow& message) -> bool {
        return writer->Write(message);
      },
      engine_);
}

::grpc::Status StreamQueryResults(
    backend::storage::Storage* storage,
    const v1::QueryRequest& request,
    const std::function<bool(const v1::QueryResultRow&)>& write,
    backend::engine::Engine* engine) {
  ::grpc::Status validated = ValidateQueryRequest(storage, request, write);
  if (!validated.ok()) return validated;

  // The catalog adapter materializes `googlesql::Table*`s out of
  // `storage` lazily; its `TypeFactory` must outlive the
  // RowSource the engine returns because the iterator's value
  // pointers reach back into the catalog's type allocations. We
  // therefore pin both the type factory and the catalog as locals
  // here for the duration of the stream.
  ::googlesql::TypeFactory stack_type_factory;
  ::googlesql::TypeFactory* type_factory = &stack_type_factory;
  if (::googlesql::TypeFactory* reg_factory =
          backend::catalog::LookupProjectTypeFactory(request.project_id());
      reg_factory != nullptr) {
    type_factory = reg_factory;
  }
  ::googlesql::AnalyzerOptions analyzer_options = MakeAnalyzerOptions();
  backend::catalog::GoogleSqlCatalog catalog(request.project_id(),
                                             storage,
                                             type_factory,
                                             analyzer_options.language(),
                                             request.default_dataset_id());

  backend::engine::QueryRequest engine_request = ProtoToEngineRequest(request);
  if (absl::Status param_status =
          backend::engine::coordinator::PopulateAnalyzerParameters(
              engine_request, analyzer_options);
      !param_status.ok()) {
    return AnalyzeStatusToGrpc(param_status);
  }
  if (absl::Status sys_status =
          backend::engine::semantic::RegisterAnalyzerSystemVariables(
              type_factory, analyzer_options);
      !sys_status.ok()) {
    return AnalyzeStatusToGrpc(sys_status);
  }

  // Pre-classify the statement so we can pick the right engine entry
  // point (ExecuteQuery for SELECT, ExecuteDml for INSERT/.../MERGE)
  // and reject DDL with a friendly UNIMPLEMENTED. We pay for one
  // analyzer pass here even though the engine re-analyzes inside
  // `PreparedQuery::Prepare` / `PreparedModify::Prepare`; the cost is
  // dominated by the catalog setup and a follow-up will fold the
  // two analyses together.
  std::unique_ptr<const ::googlesql::AnalyzerOutput> classify_output;
  absl::Status classify_status =
      ::googlesql::AnalyzeStatement(request.sql(),
                                    analyzer_options,
                                    &catalog,
                                    type_factory,
                                    &classify_output);
  if (!classify_status.ok()) return AnalyzeStatusToGrpc(classify_status);
  if (classify_output == nullptr ||
      classify_output->resolved_statement() == nullptr) {
    return ::grpc::Status(::grpc::StatusCode::INTERNAL,
                          "QueryService::ExecuteQuery: analyzer returned "
                          "no resolved statement");
  }
  const ::googlesql::ResolvedStatement* stmt =
      classify_output->resolved_statement();
  const StatementClass cls = ClassifyStatement(stmt->node_kind());

  // Engine selection: the production wire path
  // (`binaries/emulator_main`) always supplies a DuckDB engine. Unit
  // tests must construct one too; we no longer fall back to a
  // per-call default engine because the reference-impl engine has
  // been removed.
  if (engine == nullptr) {
    return ::grpc::Status(
        ::grpc::StatusCode::FAILED_PRECONDITION,
        "QueryService::ExecuteQuery: engine backend is not configured");
  }
  const absl::string_view statement_type = StatementTypeFor(*stmt);
  // Classify the route here, alongside the statement-type lookup, so
  // every successful reply carries the canonical disposition string
  // on the trailing `emulator_route` field. The classifier is
  // stateless and re-walks the resolved AST; the cost is a single
  // tree walk paid once per query (the engine internally re-walks
  // when it dispatches, but that's the same redundant work
  // `StatementTypeFor` accepted for the same conservative reason --
  // keep the frontend's per-query bookkeeping independent of any
  // future engine-side metadata channel).
  const backend::engine::coordinator::RouteClassifier route_classifier;
  const backend::engine::coordinator::RouteDecision decision =
      route_classifier.Classify(*stmt);
  const absl::string_view emulator_route =
      backend::engine::DispositionToString(decision.disposition);

  switch (cls) {
    case StatementClass::kSelect:
      return StreamRows(engine,
                        engine_request,
                        &catalog,
                        statement_type,
                        emulator_route,
                        write);
    case StatementClass::kDml:
      return EmitDmlStats(engine,
                          engine_request,
                          &catalog,
                          statement_type,
                          emulator_route,
                          write);
    case StatementClass::kDdl:
      return EmitDdlResult(engine,
                           engine_request,
                           &catalog,
                           statement_type,
                           emulator_route,
                           write);
    case StatementClass::kOther:
      return ::grpc::Status(
          ::grpc::StatusCode::UNIMPLEMENTED,
          absl::StrCat("QueryService::ExecuteQuery: statement kind ",
                       stmt->node_kind_string(),
                       " is not supported by the emulator"));
  }
  return ::grpc::Status(::grpc::StatusCode::INTERNAL,
                        "QueryService::ExecuteQuery: unreachable cls");
}

}  // namespace frontend
}  // namespace bigquery_emulator
