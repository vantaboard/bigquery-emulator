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
  // Without this opt-in the analyzer rejects every non-SELECT
  // statement kind in `Prepare()` with a generic
  // "Statement not supported" error. Phase 6a needs INSERT/UPDATE/
  // DELETE/MERGE to flow through to the classifier in
  // `StreamQueryResults` so the handler can return UNIMPLEMENTED
  // (or run INSERT) instead of a misleading INVALID_ARGUMENT.
  language.SetSupportsAllStatementKinds();
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

#if defined(BIGQUERY_EMULATOR_HAS_GOOGLESQL)

namespace {

// Statement classes the gateway needs to distinguish. The analyzer
// returns a richer `ResolvedNodeKind`; we collapse that down to the
// four categories BigQuery's REST API treats differently:
//   * `kSelect` -> `Query.ExecuteQuery` streams a schema + rows;
//     `Query.DryRun` returns the analyzed schema.
//   * `kDml`    -> `Query.ExecuteQuery` runs INSERT/UPDATE/DELETE/
//     MERGE through the engine's DML path and emits a final
//     `dml_stats` summary; `Query.DryRun` for DML is allowed and
//     returns an empty schema with zero estimated bytes (BigQuery
//     does the same).
//   * `kDdl`    -> reserved for CREATE/DROP/ALTER once those land;
//     today we surface `UNIMPLEMENTED` so client libraries see the
//     standard `notImplemented` reason.
//   * `kOther`  -> unclassified statement shape (CALL, EXPORT,
//     scripting, ...); also `UNIMPLEMENTED`.
enum class StatementClass { kSelect, kDml, kDdl, kOther };

StatementClass ClassifyStatement(::googlesql::ResolvedNodeKind kind) {
  switch (kind) {
    case ::googlesql::RESOLVED_QUERY_STMT:
      return StatementClass::kSelect;
    case ::googlesql::RESOLVED_INSERT_STMT:
    case ::googlesql::RESOLVED_UPDATE_STMT:
    case ::googlesql::RESOLVED_DELETE_STMT:
    case ::googlesql::RESOLVED_MERGE_STMT:
    case ::googlesql::RESOLVED_TRUNCATE_STMT:
      return StatementClass::kDml;
    case ::googlesql::RESOLVED_CREATE_DATABASE_STMT:
    case ::googlesql::RESOLVED_CREATE_INDEX_STMT:
    case ::googlesql::RESOLVED_CREATE_SCHEMA_STMT:
    case ::googlesql::RESOLVED_CREATE_EXTERNAL_SCHEMA_STMT:
    case ::googlesql::RESOLVED_CREATE_TABLE_STMT:
    case ::googlesql::RESOLVED_CREATE_TABLE_AS_SELECT_STMT:
    case ::googlesql::RESOLVED_CREATE_EXTERNAL_TABLE_STMT:
    case ::googlesql::RESOLVED_CREATE_MODEL_STMT:
    case ::googlesql::RESOLVED_CREATE_VIEW_STMT:
    case ::googlesql::RESOLVED_CREATE_MATERIALIZED_VIEW_STMT:
    case ::googlesql::RESOLVED_CREATE_APPROX_VIEW_STMT:
    case ::googlesql::RESOLVED_CREATE_PROCEDURE_STMT:
    case ::googlesql::RESOLVED_CREATE_FUNCTION_STMT:
    case ::googlesql::RESOLVED_CREATE_TABLE_FUNCTION_STMT:
    case ::googlesql::RESOLVED_CREATE_CONSTANT_STMT:
    case ::googlesql::RESOLVED_CREATE_ENTITY_STMT:
    case ::googlesql::RESOLVED_CREATE_ROW_ACCESS_POLICY_STMT:
    case ::googlesql::RESOLVED_CREATE_PRIVILEGE_RESTRICTION_STMT:
    case ::googlesql::RESOLVED_CREATE_SNAPSHOT_TABLE_STMT:
    case ::googlesql::RESOLVED_CREATE_PROPERTY_GRAPH_STMT:
    case ::googlesql::RESOLVED_CREATE_CONNECTION_STMT:
    case ::googlesql::RESOLVED_CREATE_SEQUENCE_STMT:
    case ::googlesql::RESOLVED_CLONE_DATA_STMT:
    case ::googlesql::RESOLVED_DROP_STMT:
    case ::googlesql::RESOLVED_DROP_FUNCTION_STMT:
    case ::googlesql::RESOLVED_DROP_TABLE_FUNCTION_STMT:
    case ::googlesql::RESOLVED_DROP_INDEX_STMT:
    case ::googlesql::RESOLVED_DROP_MATERIALIZED_VIEW_STMT:
    case ::googlesql::RESOLVED_DROP_PRIVILEGE_RESTRICTION_STMT:
    case ::googlesql::RESOLVED_DROP_ROW_ACCESS_POLICY_STMT:
    case ::googlesql::RESOLVED_DROP_SNAPSHOT_TABLE_STMT:
    case ::googlesql::RESOLVED_RENAME_STMT:
    case ::googlesql::RESOLVED_ALTER_DATABASE_STMT:
    case ::googlesql::RESOLVED_ALTER_INDEX_STMT:
    case ::googlesql::RESOLVED_ALTER_MATERIALIZED_VIEW_STMT:
    case ::googlesql::RESOLVED_ALTER_APPROX_VIEW_STMT:
    case ::googlesql::RESOLVED_ALTER_MODEL_STMT:
    case ::googlesql::RESOLVED_ALTER_PRIVILEGE_RESTRICTION_STMT:
    case ::googlesql::RESOLVED_ALTER_ROW_ACCESS_POLICY_STMT:
    case ::googlesql::RESOLVED_ALTER_ALL_ROW_ACCESS_POLICIES_STMT:
    case ::googlesql::RESOLVED_ALTER_SCHEMA_STMT:
    case ::googlesql::RESOLVED_ALTER_EXTERNAL_SCHEMA_STMT:
    case ::googlesql::RESOLVED_ALTER_TABLE_SET_OPTIONS_STMT:
    case ::googlesql::RESOLVED_ALTER_TABLE_STMT:
    case ::googlesql::RESOLVED_ALTER_VIEW_STMT:
    case ::googlesql::RESOLVED_ALTER_CONNECTION_STMT:
    case ::googlesql::RESOLVED_ALTER_SEQUENCE_STMT:
    case ::googlesql::RESOLVED_ALTER_ENTITY_STMT:
    case ::googlesql::RESOLVED_GRANT_STMT:
    case ::googlesql::RESOLVED_REVOKE_STMT:
    case ::googlesql::RESOLVED_UNDROP_STMT:
      return StatementClass::kDdl;
    default:
      return StatementClass::kOther;
  }
}

}  // namespace

#endif  // BIGQUERY_EMULATOR_HAS_GOOGLESQL

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
                       "implemented yet (Phase 6b of ROADMAP.md); got ",
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

  // Pre-classify the statement so we can pick the right engine entry
  // point (ExecuteQuery for SELECT, ExecuteDml for INSERT/.../MERGE)
  // and reject DDL with a friendly UNIMPLEMENTED. We pay for one
  // analyzer pass here even though the engine re-analyzes inside
  // `PreparedQuery::Prepare` / `PreparedModify::Prepare`; the cost is
  // dominated by the catalog setup and Phase 6c will fold the two
  // analyses together.
  std::unique_ptr<const ::googlesql::AnalyzerOutput> classify_output;
  absl::Status classify_status = ::googlesql::AnalyzeStatement(
      request.sql(), analyzer_options, &catalog, &type_factory,
      &classify_output);
  if (!classify_status.ok()) {
    return AnalyzeStatusToGrpc(classify_status);
  }
  if (classify_output == nullptr ||
      classify_output->resolved_statement() == nullptr) {
    return ::grpc::Status(::grpc::StatusCode::INTERNAL,
                          "QueryService::ExecuteQuery: analyzer returned "
                          "no resolved statement");
  }
  const ::googlesql::ResolvedStatement* stmt =
      classify_output->resolved_statement();
  const StatementClass cls = ClassifyStatement(stmt->node_kind());

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

  switch (cls) {
    case StatementClass::kSelect:
      break;  // Fall through to the SELECT path below.
    case StatementClass::kDml: {
      absl::StatusOr<backend::engine::DmlStats> stats =
          active_engine->ExecuteDml(engine_request, &catalog);
      if (!stats.ok()) {
        return AnalyzeStatusToGrpc(stats.status());
      }
      // DML reply: no schema and no row messages, just a single
      // `dml_stats` summary the gateway folds into BigQuery's
      // `dmlStats` / `numDmlAffectedRows` envelope.
      v1::QueryResultRow stats_message;
      auto* proto_stats = stats_message.mutable_dml_stats();
      proto_stats->set_inserted_row_count(stats->inserted_row_count);
      proto_stats->set_updated_row_count(stats->updated_row_count);
      proto_stats->set_deleted_row_count(stats->deleted_row_count);
      if (!write(stats_message)) {
        return ::grpc::Status(
            ::grpc::StatusCode::CANCELLED,
            "QueryService::ExecuteQuery: client cancelled stream before "
            "dml_stats");
      }
      return ::grpc::Status::OK;
    }
    case StatementClass::kDdl: {
      // Plan-35 ENGINE POLICY (extends HANDOFF.md §4.3 path 3's
      // "DuckDB-only MERGE" pattern): DDL lives on the DuckDB
      // engine. The reference-impl engine returns UNIMPLEMENTED for
      // ExecuteDdl, which surfaces here as gRPC UNIMPLEMENTED so
      // the gateway maps it to BigQuery's `notImplemented` reason;
      // the FallbackEngine wrapper (operator launched with
      // `--engine=reference_impl --on_unknown_fn=fallback`) catches
      // it transparently and retries against DuckDB before this
      // status ever escapes.
      absl::Status ddl_status =
          active_engine->ExecuteDdl(engine_request, &catalog);
      if (!ddl_status.ok()) {
        return AnalyzeStatusToGrpc(ddl_status);
      }
      // DDL reply: an empty stream. The gateway reads zero
      // messages, surfaces `jobComplete=true` with no schema and no
      // rows, and the BigQuery REST client sees the matching
      // "DDL succeeded" envelope.
      return ::grpc::Status::OK;
    }
    case StatementClass::kOther:
      return ::grpc::Status(
          ::grpc::StatusCode::UNIMPLEMENTED,
          absl::StrCat("QueryService::ExecuteQuery: statement kind ",
                       stmt->node_kind_string(),
                       " is not supported by the emulator"));
  }

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
