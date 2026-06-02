#include "backend/engine/coordinator/local_coordinator_engine.h"

#include <memory>
#include <utility>

#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "absl/strings/str_cat.h"
#include "backend/engine/coordinator/executor.h"
#include "backend/engine/coordinator/route_classifier.h"
#include "backend/engine/disposition.h"
#include "backend/engine/engine.h"
#include "backend/engine/semantic/value.h"
#include "backend/schema/googlesql_to_bq.h"
#include "backend/schema/schema.h"
#include "googlesql/public/analyzer.h"
#include "googlesql/public/analyzer_options.h"
#include "googlesql/public/analyzer_output.h"
#include "googlesql/public/catalog.h"
#include "googlesql/public/language_options.h"
#include "googlesql/public/options.pb.h"
#include "googlesql/public/type.h"
#include "googlesql/public/type.pb.h"
#include "googlesql/public/types/type_factory.h"
#include "googlesql/resolved_ast/resolved_ast.h"
#include "googlesql/resolved_ast/resolved_node_kind.pb.h"
#include "proto/emulator.pb.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace coordinator {

namespace {

// Analyzer options the coordinator uses for `Analyze` / `DryRun`
// (SELECT-only). Mirrors `duckdb_engine.cc::MakeAnalyzerOptions`
// verbatim so we don't drift name-resolution / feature toggles
// between the legacy DuckDBEngine path (deleted at the end of this
// plan) and the new coordinator.
::googlesql::AnalyzerOptions MakeAnalyzerOptions() {
  ::googlesql::LanguageOptions language;
  language.EnableMaximumLanguageFeatures();
  language.set_product_mode(::googlesql::PRODUCT_EXTERNAL);
  language.set_name_resolution_mode(::googlesql::NAME_RESOLUTION_DEFAULT);
  ::googlesql::AnalyzerOptions options(language);
  options.set_error_message_mode(::googlesql::ERROR_MESSAGE_ONE_LINE);
  options.set_attach_error_location_payload(true);
  // We do NOT flip `prune_unused_columns` on: doing so changes the
  // resolved column lifetime, which has historically broken the
  // engine's downstream uses of the resolved AST.
  options.CreateDefaultArenasIfNotSet();
  return options;
}

// `MakeAnalyzerOptions` plus the all-statement-kind allowlist
// DML / DDL paths need.
::googlesql::AnalyzerOptions MakeAnalyzerOptionsAllStatements() {
  ::googlesql::AnalyzerOptions options = MakeAnalyzerOptions();
  options.mutable_language()->SetSupportsAllStatementKinds();
  return options;
}

absl::Status ValidateRequest(const QueryRequest& request,
                             const ::googlesql::Catalog* catalog) {
  if (catalog == nullptr) {
    return absl::FailedPreconditionError(
        "LocalCoordinatorEngine: catalog must be non-null");
  }
  if (request.sql.empty()) {
    return absl::InvalidArgumentError(
        "LocalCoordinatorEngine: request.sql is required");
  }
  if (request.use_legacy_sql) {
    return absl::InvalidArgumentError(
        "LocalCoordinatorEngine: useLegacySql=true is not supported; "
        "only GoogleSQL is implemented");
  }
  return absl::OkStatus();
}

// Build the BigQuery-shaped output schema for a SELECT statement.
// Mirrors the helper in `duckdb_engine.cc`; the proto round-trip is
// required because `OutputColumnListToTableSchema` is the only
// caller-visible API that materializes the REPEATED-mode contract
// for ARRAY columns the gateway emits.
absl::StatusOr<schema::TableSchema> ReflectOutputSchema(
    const ::googlesql::ResolvedQueryStmt& stmt) {
  v1::TableSchema proto;
  absl::Status s = backend::schema::OutputColumnListToTableSchema(
      stmt.output_column_list(), &proto);
  if (!s.ok()) return s;
  return backend::schema::TableSchemaFromProto(proto);
}

// Concrete `AnalyzedQuery` returned by `Analyze`. Owns the
// analyzer output (and therefore the resolved AST) so callers can
// hold the analyzed value across multiple gateway calls. The
// schema is computed once and cached.
class AnalyzedQueryImpl : public AnalyzedQuery {
 public:
  AnalyzedQueryImpl(std::unique_ptr<const ::googlesql::AnalyzerOutput> output,
                    schema::TableSchema schema)
      : output_(std::move(output)), schema_(std::move(schema)) {}

  const schema::TableSchema& output_schema() const override {
    return schema_;
  }

 private:
  std::unique_ptr<const ::googlesql::AnalyzerOutput> output_{};
  schema::TableSchema schema_{};
};

// Map a `QueryParameter::type_kind` spelling to a
// `::googlesql::Type*` instance the analyzer accepts on
// `AddQueryParameter` / `AddPositionalQueryParameter`. Mirrors
// `backend/engine/semantic/value.cc::ParseTypeKindName` and the
// builtin type accessors in `googlesql/public/types/type_factory.h`.
// Returns INVALID_ARGUMENT when the type kind is unknown or one of
// the compound types (ARRAY / STRUCT) that the upstream parameter
// plumbing has not yet wired through.
absl::StatusOr<const ::googlesql::Type*> ParameterTypeForKind(
    absl::string_view type_kind_name) {
  ::googlesql::TypeKind kind = semantic::ParseTypeKindName(type_kind_name);
  switch (kind) {
    case ::googlesql::TYPE_BOOL:
      return ::googlesql::types::BoolType();
    case ::googlesql::TYPE_INT64:
      return ::googlesql::types::Int64Type();
    case ::googlesql::TYPE_DOUBLE:
      return ::googlesql::types::DoubleType();
    case ::googlesql::TYPE_STRING:
      return ::googlesql::types::StringType();
    case ::googlesql::TYPE_BYTES:
      return ::googlesql::types::BytesType();
    case ::googlesql::TYPE_DATE:
      return ::googlesql::types::DateType();
    case ::googlesql::TYPE_TIME:
      return ::googlesql::types::TimeType();
    case ::googlesql::TYPE_DATETIME:
      return ::googlesql::types::DatetimeType();
    case ::googlesql::TYPE_TIMESTAMP:
      return ::googlesql::types::TimestampType();
    case ::googlesql::TYPE_NUMERIC:
      return ::googlesql::types::NumericType();
    case ::googlesql::TYPE_BIGNUMERIC:
      return ::googlesql::types::BigNumericType();
    case ::googlesql::TYPE_JSON:
      return ::googlesql::types::JsonType();
    case ::googlesql::TYPE_GEOGRAPHY:
      return ::googlesql::types::GeographyType();
    case ::googlesql::TYPE_INTERVAL:
      return ::googlesql::types::IntervalType();
    case ::googlesql::TYPE_UUID:
      return ::googlesql::types::UuidType();
    default:
      return absl::InvalidArgumentError(absl::StrCat(
          "LocalCoordinatorEngine: parameter type kind '",
          type_kind_name,
          "' is not supported by the coordinator's analyzer plumbing"));
  }
}

// Add the request's parameter declarations to `options`. The
// analyzer needs to see every parameter the SQL references at
// `AnalyzeStatement` time so it can produce typed
// `ResolvedParameter` nodes the semantic executor binds against.
absl::Status PopulateParameterOptions(const QueryRequest& request,
                                      ::googlesql::AnalyzerOptions& options) {
  if (request.parameters.empty()) return absl::OkStatus();
  bool has_named = false;
  bool has_positional = false;
  for (const QueryParameter& p : request.parameters) {
    if (p.name.empty()) {
      has_positional = true;
    } else {
      has_named = true;
    }
  }
  if (has_named && has_positional) {
    return absl::InvalidArgumentError(
        "LocalCoordinatorEngine: request mixes named and positional "
        "parameters");
  }
  options.set_parameter_mode(has_positional ? ::googlesql::PARAMETER_POSITIONAL
                                            : ::googlesql::PARAMETER_NAMED);
  for (const QueryParameter& p : request.parameters) {
    auto type_or = ParameterTypeForKind(p.type_kind);
    if (!type_or.ok()) return type_or.status();
    if (p.name.empty()) {
      absl::Status s = options.AddPositionalQueryParameter(*type_or);
      if (!s.ok()) return s;
    } else {
      absl::Status s = options.AddQueryParameter(p.name, *type_or);
      if (!s.ok()) return s;
    }
  }
  return absl::OkStatus();
}

// Analyze `request.sql` against `catalog` and return the
// `AnalyzerOutput` (which owns the resolved AST).
absl::StatusOr<std::unique_ptr<const ::googlesql::AnalyzerOutput>>
AnalyzeStatementImpl(const QueryRequest& request,
                     ::googlesql::Catalog* catalog,
                     bool all_statements) {
  ::googlesql::AnalyzerOptions options =
      all_statements ? MakeAnalyzerOptionsAllStatements()
                     : MakeAnalyzerOptions();
  absl::Status param_status = PopulateParameterOptions(request, options);
  if (!param_status.ok()) return param_status;
  ::googlesql::TypeFactory type_factory;
  std::unique_ptr<const ::googlesql::AnalyzerOutput> output;
  absl::Status analyze = ::googlesql::AnalyzeStatement(
      request.sql, options, catalog, &type_factory, &output);
  if (!analyze.ok()) return analyze;
  if (output == nullptr || output->resolved_statement() == nullptr) {
    return absl::InternalError(
        "LocalCoordinatorEngine: analyzer returned no resolved statement");
  }
  return output;
}

}  // namespace

LocalCoordinatorEngine::LocalCoordinatorEngine(storage::Storage* storage)
    : duckdb_executor_(storage),
      semantic_executor_(storage),
      control_op_executor_(storage) {}

LocalCoordinatorEngine::~LocalCoordinatorEngine() = default;

Executor* LocalCoordinatorEngine::RouteFor(
    const ::googlesql::ResolvedStatement& stmt) {
  RouteDecision decision = classifier_.Classify(stmt);
  switch (decision.disposition) {
    case Disposition::kDuckdbNative:
    case Disposition::kDuckdbRewrite:
    case Disposition::kDuckdbUdf:
      // All three DuckDB-flavored routes dispatch through the
      // `DuckDbExecutor`. `kDuckdbUdf` is included because the
      // disposition is still owned by DuckDB (the polyfill UDF
      // library is loaded into DuckDB) even though
      // `duckdb-polyfill-udf-library.plan.md` has not yet shipped
      // its polyfills; the transpiler's empty-string contract
      // surfaces UNIMPLEMENTED for shapes the polyfill set does
      // not cover, which is consistent with the no-silent-
      // approximation rule.
      return &duckdb_executor_;
    case Disposition::kSemanticExecutor:
      return &semantic_executor_;
    case Disposition::kControlOp:
      return &control_op_executor_;
    case Disposition::kUnsupported:
      return &unsupported_executor_;
  }
  // The switch is exhaustive over the disposition enum; this
  // branch is reachable only on an unknown tag (compiler upgrade
  // adding a new enum value without updating this file). Callers
  // map nullptr to INTERNAL.
  return nullptr;
}

absl::StatusOr<std::unique_ptr<AnalyzedQuery>> LocalCoordinatorEngine::Analyze(
    const QueryRequest& request, ::googlesql::Catalog* catalog) {
  absl::Status valid = ValidateRequest(request, catalog);
  if (!valid.ok()) return valid;

  absl::StatusOr<std::unique_ptr<const ::googlesql::AnalyzerOutput>> output =
      AnalyzeStatementImpl(request, catalog, /*all_statements=*/false);
  if (!output.ok()) return output.status();
  const ::googlesql::ResolvedStatement* stmt = (*output)->resolved_statement();
  if (stmt->node_kind() != ::googlesql::RESOLVED_QUERY_STMT) {
    return absl::InvalidArgumentError(absl::StrCat(
        "LocalCoordinatorEngine::Analyze: only SELECT-shaped queries are "
        "supported; got ",
        stmt->node_kind_string()));
  }
  absl::StatusOr<schema::TableSchema> output_schema =
      ReflectOutputSchema(*stmt->GetAs<::googlesql::ResolvedQueryStmt>());
  if (!output_schema.ok()) return output_schema.status();
  return std::make_unique<AnalyzedQueryImpl>(std::move(*output),
                                             std::move(*output_schema));
}

absl::StatusOr<DryRunResult> LocalCoordinatorEngine::DryRun(
    const QueryRequest& request, ::googlesql::Catalog* catalog) {
  absl::StatusOr<std::unique_ptr<AnalyzedQuery>> analyzed =
      Analyze(request, catalog);
  if (!analyzed.ok()) return analyzed.status();
  DryRunResult result;
  result.schema = (*analyzed)->output_schema();
  // No cost model yet; the gateway's wire envelope expects a
  // non-negative byte estimate.
  result.estimated_bytes_processed = 0;
  return result;
}

absl::StatusOr<std::unique_ptr<RowSource>> LocalCoordinatorEngine::ExecuteQuery(
    const QueryRequest& request, ::googlesql::Catalog* catalog) {
  absl::Status valid = ValidateRequest(request, catalog);
  if (!valid.ok()) return valid;
  absl::StatusOr<std::unique_ptr<const ::googlesql::AnalyzerOutput>> output =
      AnalyzeStatementImpl(request, catalog, /*all_statements=*/false);
  if (!output.ok()) return output.status();
  const ::googlesql::ResolvedStatement* stmt = (*output)->resolved_statement();
  Executor* executor = RouteFor(*stmt);
  if (executor == nullptr) {
    return absl::InternalError(
        "LocalCoordinatorEngine::ExecuteQuery: classifier returned an "
        "unknown disposition");
  }
  return executor->ExecuteQuery(request, *stmt, catalog);
}

absl::StatusOr<DmlStats> LocalCoordinatorEngine::ExecuteDml(
    const QueryRequest& request, ::googlesql::Catalog* catalog) {
  absl::Status valid = ValidateRequest(request, catalog);
  if (!valid.ok()) return valid;
  absl::StatusOr<std::unique_ptr<const ::googlesql::AnalyzerOutput>> output =
      AnalyzeStatementImpl(request, catalog, /*all_statements=*/true);
  if (!output.ok()) return output.status();
  const ::googlesql::ResolvedStatement* stmt = (*output)->resolved_statement();
  Executor* executor = RouteFor(*stmt);
  if (executor == nullptr) {
    return absl::InternalError(
        "LocalCoordinatorEngine::ExecuteDml: classifier returned an "
        "unknown disposition");
  }
  return executor->ExecuteDml(request, *stmt, catalog);
}

absl::Status LocalCoordinatorEngine::ExecuteDdl(const QueryRequest& request,
                                                ::googlesql::Catalog* catalog) {
  absl::Status valid = ValidateRequest(request, catalog);
  if (!valid.ok()) return valid;
  absl::StatusOr<std::unique_ptr<const ::googlesql::AnalyzerOutput>> output =
      AnalyzeStatementImpl(request, catalog, /*all_statements=*/true);
  if (!output.ok()) return output.status();
  const ::googlesql::ResolvedStatement* stmt = (*output)->resolved_statement();
  Executor* executor = RouteFor(*stmt);
  if (executor == nullptr) {
    return absl::InternalError(
        "LocalCoordinatorEngine::ExecuteDdl: classifier returned an "
        "unknown disposition");
  }
  return executor->ExecuteDdl(request, *stmt, catalog);
}

}  // namespace coordinator
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
