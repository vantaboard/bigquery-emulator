#include "frontend/handlers/query.h"

#include <cstdint>
#include <functional>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "absl/status/status.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/string_view.h"
#include "backend/engine/engine.h"
#include "backend/storage/storage.h"

#if defined(BIGQUERY_EMULATOR_HAS_GOOGLESQL)
#include "backend/catalog/googlesql_catalog.h"
#include "backend/engine/reference_impl/reference_impl_engine.h"
#include "backend/schema/googlesql_to_bq.h"
#include "backend/schema/schema.h"
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

constexpr char kExecutionUnimplemented[] =
    "Query.ExecuteQuery is not implemented yet (Phase 5 of ROADMAP.md): "
    "this build was produced without GoogleSQL linked in";
#endif

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

// Mirrors `frontend/handlers/catalog.cc::ValueToCell`: marshals an
// engine-agnostic `backend::storage::Value` onto the `v1::Cell`
// oneof the proto carries on the wire. Duplicated here (instead of
// extracted) because the catalog and query handlers are the only
// two callers and a shared utility would otherwise pull a third
// `cell_marshal` library into the build graph just to host one
// function. The two implementations must stay in sync; both follow
// the BigQuery REST `f`/`v` shape (primitives flattened to their
// decimal-string formatting so STRING / INT64 / FLOAT64 / BOOL all
// land on the wire as `string_value`).
void ValueToCell(const backend::storage::Value& value, v1::Cell* out) {
  using Kind = backend::storage::Value::Kind;
  out->Clear();
  switch (value.kind()) {
    case Kind::kNull:
      out->set_null_value(true);
      return;
    case Kind::kBool:
      out->set_string_value(value.bool_value() ? "true" : "false");
      return;
    case Kind::kInt64:
      out->set_string_value(absl::StrCat(value.int64_value()));
      return;
    case Kind::kFloat64:
      out->set_string_value(absl::StrCat(value.float64_value()));
      return;
    case Kind::kString:
    case Kind::kBytes:
      out->set_string_value(value.string_value());
      return;
    case Kind::kArray: {
      auto* arr = out->mutable_array();
      for (const auto& el : value.array_value()) {
        ValueToCell(el, arr->add_elements());
      }
      return;
    }
    case Kind::kStruct: {
      auto* st = out->mutable_struct_value();
      for (const auto& f : value.struct_value()) {
        ValueToCell(f, st->add_fields());
      }
      return;
    }
  }
}

// Translates a `bigquery_emulator.v1.QueryRequest` proto into the
// engine-facing `backend::engine::QueryRequest` struct. The two have
// the same fields but live in different packages (proto vs. plain
// C++ struct) so the engine never has to depend on the proto
// runtime.
backend::engine::QueryRequest ProtoToEngineRequest(
    const v1::QueryRequest& request) {
  backend::engine::QueryRequest engine_request;
  engine_request.project_id = request.project_id();
  engine_request.default_dataset_id = request.default_dataset_id();
  engine_request.sql = request.sql();
  engine_request.use_legacy_sql = request.use_legacy_sql();
  engine_request.parameters.reserve(request.parameters_size());
  for (const auto& kv : request.parameters()) {
    backend::engine::QueryParameter parameter;
    parameter.name = kv.first;
    parameter.type_kind = kv.second.type_kind();
    parameter.value_json = kv.second.value_json();
    engine_request.parameters.push_back(std::move(parameter));
  }
  return engine_request;
}
#endif  // BIGQUERY_EMULATOR_HAS_GOOGLESQL

}  // namespace

QueryService::QueryService(backend::storage::Storage* storage,
                            backend::engine::Engine* engine)
    : storage_(storage), engine_(engine) {}

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
  ::googlesql::AnalyzerOptions options = MakeAnalyzerOptions();
  backend::catalog::GoogleSqlCatalog catalog(request->project_id(), storage_,
                                              &type_factory,
                                              options.language());

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
    ::grpc::ServerContext* /*context*/, const v1::QueryRequest* request,
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
      storage_, *request,
      [writer](const v1::QueryResultRow& message) -> bool {
        return writer->Write(message);
      },
      engine_);
}

::grpc::Status StreamQueryResults(
    backend::storage::Storage* storage, const v1::QueryRequest& request,
    const std::function<bool(const v1::QueryResultRow&)>& write,
    backend::engine::Engine* engine) {
#if !defined(BIGQUERY_EMULATOR_HAS_GOOGLESQL)
  (void)storage;
  (void)request;
  (void)write;
  (void)engine;
  return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED,
                        kExecutionUnimplemented);
#else
  if (!write) {
    return ::grpc::Status(::grpc::StatusCode::INTERNAL,
                          "QueryService::ExecuteQuery: write callback is empty");
  }
  if (storage == nullptr) {
    return ::grpc::Status(
        ::grpc::StatusCode::FAILED_PRECONDITION,
        "QueryService::ExecuteQuery: storage backend is not configured");
  }
  if (request.use_legacy_sql()) {
    return ::grpc::Status(
        ::grpc::StatusCode::INVALID_ARGUMENT,
        "QueryService::ExecuteQuery: useLegacySql=true is not supported; "
        "the emulator only implements GoogleSQL");
  }
  if (request.project_id().empty()) {
    return ::grpc::Status(
        ::grpc::StatusCode::INVALID_ARGUMENT,
        "QueryService::ExecuteQuery: request.project_id is required");
  }
  if (request.sql().empty()) {
    return ::grpc::Status(
        ::grpc::StatusCode::INVALID_ARGUMENT,
        "QueryService::ExecuteQuery: request.sql is required");
  }

  // The catalog adapter materializes `googlesql::Table*`s out of
  // `storage` lazily; its `TypeFactory` must outlive the
  // RowSource the engine returns because the iterator's value
  // pointers reach back into the catalog's type allocations. We
  // therefore pin both the type factory and the catalog as locals
  // here for the duration of the stream.
  ::googlesql::TypeFactory type_factory;
  ::googlesql::AnalyzerOptions analyzer_options = MakeAnalyzerOptions();
  backend::catalog::GoogleSqlCatalog catalog(
      request.project_id(), storage, &type_factory,
      analyzer_options.language());

  // Engine selection:
  //   * If the caller provided one (the `binaries/emulator_main` wire
  //     path always does), use it verbatim -- including any
  //     `FallbackEngine` wrapping the operator already configured via
  //     `--on_unknown_fn=fallback`.
  //   * Otherwise (the legacy `query_test.cc` path that only knows
  //     about `storage`) construct a per-call reference-impl engine
  //     so the existing tests still pin the original behavior.
  std::unique_ptr<backend::engine::reference_impl::ReferenceImplEngine>
      owned_engine;
  backend::engine::Engine* active_engine = engine;
  if (active_engine == nullptr) {
    owned_engine = std::make_unique<
        backend::engine::reference_impl::ReferenceImplEngine>(storage);
    active_engine = owned_engine.get();
  }
  backend::engine::QueryRequest engine_request = ProtoToEngineRequest(request);
  absl::StatusOr<std::unique_ptr<backend::engine::RowSource>> source_or =
      active_engine->ExecuteQuery(engine_request, &catalog);
  if (!source_or.ok()) {
    return AnalyzeStatusToGrpc(source_or.status());
  }
  std::unique_ptr<backend::engine::RowSource> source =
      std::move(source_or).value();
  if (source == nullptr) {
    return ::grpc::Status(::grpc::StatusCode::INTERNAL,
                          "QueryService::ExecuteQuery: engine returned a "
                          "null RowSource");
  }

  // First message: the schema. We always emit it (even for queries
  // that return zero rows) so the gateway can synthesize a BigQuery
  // REST `schema` field on the response without having to wait for
  // the first row.
  v1::QueryResultRow schema_message;
  backend::schema::TableSchemaToProto(source->schema(),
                                       schema_message.mutable_schema());
  if (!write(schema_message)) {
    return ::grpc::Status(
        ::grpc::StatusCode::CANCELLED,
        "QueryService::ExecuteQuery: client cancelled stream before schema");
  }

  // Subsequent messages: one per result row. `RowSource::Next`
  // returns false on end-of-stream and a non-OK status on a
  // mid-stream failure; both surface to the caller through the
  // standard gRPC status code mapping.
  backend::storage::Row row;
  while (true) {
    absl::StatusOr<bool> next = source->Next(&row);
    if (!next.ok()) {
      return AnalyzeStatusToGrpc(next.status());
    }
    if (!*next) break;
    v1::QueryResultRow row_message;
    for (const auto& cell : row.cells) {
      ValueToCell(cell, row_message.add_cells());
    }
    if (!write(row_message)) {
      return ::grpc::Status(
          ::grpc::StatusCode::CANCELLED,
          "QueryService::ExecuteQuery: client cancelled stream mid-row");
    }
  }
  return ::grpc::Status::OK;
#endif  // BIGQUERY_EMULATOR_HAS_GOOGLESQL
}

}  // namespace frontend
}  // namespace bigquery_emulator
