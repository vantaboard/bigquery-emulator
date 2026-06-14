#include "backend/engine/coordinator/local_coordinator_analyze.h"

#include <memory>
#include <utility>

#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "absl/strings/match.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/str_split.h"
#include "absl/strings/strip.h"
#include "absl/time/time.h"
#include "backend/catalog/googlesql_catalog.h"
#include "backend/engine/coordinator/sql_preprocess.h"
#include "backend/engine/engine.h"
#include "backend/engine/semantic/system_variables.h"
#include "backend/engine/semantic/value.h"
#include "backend/schema/googlesql_to_bq.h"
#include "backend/schema/schema.h"
#include "googlesql/public/analyzer.h"
#include "googlesql/public/analyzer_options.h"
#include "googlesql/public/analyzer_output.h"
#include "googlesql/public/catalog.h"
#include "googlesql/public/language_options.h"
#include "googlesql/public/options.pb.h"
#include "googlesql/public/types/array_type.h"
#include "googlesql/public/types/struct_type.h"
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
::googlesql::AnalyzerOptions MakeAnalyzerOptionsBase() {
  ::googlesql::LanguageOptions language;
  language.EnableMaximumLanguageFeaturesForDevelopment();
  // WITH(<name> AS <expr>, ...) <body> scalar bindings (GA feature).
  language.EnableLanguageFeature(::googlesql::FEATURE_WITH_EXPRESSION);
  language.EnableLanguageFeature(::googlesql::FEATURE_MATCH_RECOGNIZE);
  language.EnableLanguageFeature(
      ::googlesql::FEATURE_STRATIFIED_RESERVOIR_TABLESAMPLE);
  language.EnableLanguageFeature(::googlesql::FEATURE_KLL_WEIGHTS);
  language.EnableLanguageFeature(::googlesql::FEATURE_CREATE_TABLE_CLONE);
  language.EnableLanguageFeature(::googlesql::FEATURE_CREATE_SNAPSHOT_TABLE);
  language.EnableLanguageFeature(::googlesql::FEATURE_CLONE_DATA);
  language.EnableLanguageFeature(::googlesql::FEATURE_REMOTE_MODEL);
  language.set_product_mode(::googlesql::PRODUCT_EXTERNAL);
  language.set_name_resolution_mode(::googlesql::NAME_RESOLUTION_DEFAULT);
  ::googlesql::AnalyzerOptions options(language);
  options.set_error_message_mode(::googlesql::ERROR_MESSAGE_ONE_LINE);
  options.set_attach_error_location_payload(true);
  // Keep literal CAST as ResolvedCast nodes so the semantic executor's
  // BigQuery-parity truncation (FLOAT64/NUMERIC -> INT64) runs instead
  // of the analyzer's folded constant (which rounds 3.7 -> 4).
  options.set_fold_literal_cast(false);
  // We do NOT flip `prune_unused_columns` on: doing so changes the
  // resolved column lifetime, which has historically broken the
  // engine's downstream uses of the resolved AST.
  // We disable the analyzer's PIVOT / UNPIVOT rewriters so the
  // route classifier sees the raw `ResolvedPivotScan` /
  // `ResolvedUnpivotScan` nodes, which the
  // `advanced-relational-routing` plan dispositions as
  // `duckdb_rewrite`. The transpiler's `EmitPivotScan` /
  // `EmitUnpivotScan` lower these directly to DuckDB SQL (FILTER
  // aggregates / UNION ALL); letting the analyzer rewrite them
  // upstream would bypass the disposition decision and route the
  // (rewritten) shape on whatever path matches the rewriter's
  // output instead.
  options.disable_rewrite(::googlesql::REWRITE_PIVOT);
  options.disable_rewrite(::googlesql::REWRITE_UNPIVOT);
  // Disable the generalized-query-stmt rewriter so the pipe-DDL
  // forms (`FROM ... |> EXPORT DATA` and `FROM ... |> CREATE
  // TABLE`) survive in the resolved tree as
  // `ResolvedPipeExportDataScan` / `ResolvedPipeCreateTableScan`
  // nodes the classifier dispatches to the control-op route. With
  // the default rewriter enabled, those scans would be unwrapped
  // to a plain `ResolvedExportDataStmt` / `ResolvedCreateTableAs-
  // SelectStmt` and lose their per-shape disposition row in
  // `node_dispositions.yaml`. The `advanced-relational-routing`
  // plan dispositions explicitly target the pipe forms, so the
  // rewriter is off here.
  options.disable_rewrite(::googlesql::REWRITE_GENERALIZED_QUERY_STMT);
  // Permit the analyzer to produce `ResolvedGeneralizedQueryStmt`
  // alongside the default `ResolvedQueryStmt`. With the rewriter
  // disabled (above), this is the only statement kind that
  // carries a `ResolvedPipeExportDataScan` /
  // `ResolvedPipeCreateTableScan` body. The default supported-
  // statement set is `{RESOLVED_QUERY_STMT}` (see
  // `LanguageOptions::supported_statement_kinds_`); we add the
  // generalized form on top.
  options.mutable_language()->AddSupportedStatementKind(
      ::googlesql::RESOLVED_GENERALIZED_QUERY_STMT);
  // Naive TIMESTAMP literals (no timezone suffix) must resolve as UTC
  // to match BigQuery / query port (see window_dense_rank_with_group).
  options.set_default_time_zone(absl::UTCTimeZone());
  options.CreateDefaultArenasIfNotSet();
  return options;
}

// Registers @@ system variables on `options` once `type_factory` is
// available (called from AnalyzeStatementImpl after catalog lookup).
absl::Status RegisterSystemVariablesOnOptions(
    ::googlesql::TypeFactory* type_factory,
    ::googlesql::AnalyzerOptions& options) {
  return semantic::RegisterAnalyzerSystemVariables(type_factory, options);
}

// `MakeAnalyzerOptionsBase` plus the all-statement-kind allowlist
// DML / DDL paths need.
::googlesql::AnalyzerOptions MakeAnalyzerOptionsAllStatements() {
  ::googlesql::AnalyzerOptions options = MakeAnalyzerOptionsBase();
  options.mutable_language()->SetSupportsAllStatementKinds();
  return options;
}

}  // namespace

::googlesql::AnalyzerOptions MakeCoordinatorAnalyzerOptions(
    bool all_statements) {
  return all_statements ? MakeAnalyzerOptionsAllStatements()
                        : MakeAnalyzerOptionsBase();
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

namespace {

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

}  // namespace

absl::StatusOr<::googlesql::AnalyzerOptions> BuildAnalyzerOptionsForRequest(
    const QueryRequest& request,
    catalog::GoogleSqlCatalog* catalog,
    bool all_statements) {
  if (catalog == nullptr) {
    return absl::FailedPreconditionError(
        "LocalCoordinatorEngine: catalog must be a GoogleSqlCatalog");
  }
  ::googlesql::TypeFactory* type_factory = catalog->type_factory();
  if (type_factory == nullptr) {
    return absl::FailedPreconditionError(
        "LocalCoordinatorEngine: catalog type_factory is null");
  }
  ::googlesql::AnalyzerOptions options =
      MakeCoordinatorAnalyzerOptions(all_statements);
  absl::Status param_status =
      PopulateParameterOptions(request, options, type_factory);
  if (!param_status.ok()) return param_status;
  if (absl::Status sys_status =
          RegisterSystemVariablesOnOptions(type_factory, options);
      !sys_status.ok()) {
    return sys_status;
  }
  return options;
}

// Analyze `request.sql` against `catalog` and return the
// `AnalyzerOutput` (which owns the resolved AST).
absl::StatusOr<std::unique_ptr<const ::googlesql::AnalyzerOutput>>
AnalyzeStatementImpl(const QueryRequest& request,
                     ::googlesql::Catalog* catalog,
                     bool all_statements) {
  auto* bq_catalog = dynamic_cast<catalog::GoogleSqlCatalog*>(catalog);
  if (bq_catalog == nullptr) {
    return absl::FailedPreconditionError(
        "LocalCoordinatorEngine: catalog must be a GoogleSqlCatalog");
  }
  absl::StatusOr<::googlesql::AnalyzerOptions> options =
      BuildAnalyzerOptionsForRequest(request, bq_catalog, all_statements);
  if (!options.ok()) return options.status();
  ::googlesql::TypeFactory* type_factory = bq_catalog->type_factory();
  const std::string preprocessed_sql = PreprocessSqlForAnalyzer(request.sql);
  std::unique_ptr<const ::googlesql::AnalyzerOutput> output;
  absl::Status analyze = ::googlesql::AnalyzeStatement(
      preprocessed_sql, *options, catalog, type_factory, &output);
  if (!analyze.ok()) return analyze;
  if (output == nullptr || output->resolved_statement() == nullptr) {
    return absl::InternalError(
        "LocalCoordinatorEngine: analyzer returned no resolved statement");
  }
  return output;
}

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

absl::StatusOr<const ::googlesql::Type*> ParameterTypeForQueryParameter(
    const QueryParameter& parameter, ::googlesql::TypeFactory* type_factory) {
  if (parameter.type_kind == "STRUCT") {
    if (type_factory == nullptr) {
      return absl::InvalidArgumentError(
          "LocalCoordinatorEngine: STRUCT parameter requires type_factory");
    }
    std::vector<::googlesql::StructType::StructField> fields;
    for (absl::string_view part : absl::StrSplit(parameter.type_json, ',')) {
      part = absl::StripAsciiWhitespace(part);
      if (part.empty()) continue;
      const size_t colon = part.find(':');
      if (colon == absl::string_view::npos) continue;
      const absl::string_view field_name = part.substr(0, colon);
      const absl::string_view field_kind =
          absl::StripAsciiWhitespace(part.substr(colon + 1));
      auto field_type_or = ParameterTypeForKind(field_kind);
      if (!field_type_or.ok()) return field_type_or.status();
      fields.emplace_back(std::string(field_name), *field_type_or);
    }
    if (fields.empty()) {
      return absl::InvalidArgumentError(
          "LocalCoordinatorEngine: STRUCT parameter missing type_json");
    }
    const ::googlesql::StructType* struct_type = nullptr;
    absl::Status s = type_factory->MakeStructType(fields, &struct_type);
    if (!s.ok()) return s;
    return struct_type;
  }
  if (parameter.type_kind == "ARRAY") {
    if (type_factory == nullptr) {
      return absl::InvalidArgumentError(
          "LocalCoordinatorEngine: ARRAY parameter requires type_factory");
    }
    if (parameter.type_json.empty()) {
      return absl::InvalidArgumentError(
          "LocalCoordinatorEngine: ARRAY parameter missing type_json");
    }
    QueryParameter element;
    if (absl::StartsWith(parameter.type_json, "STRUCT:")) {
      element.type_kind = "STRUCT";
      element.type_json = parameter.type_json.substr(7);
    } else {
      element.type_kind = std::string(parameter.type_json);
    }
    auto element_type_or =
        ParameterTypeForQueryParameter(element, type_factory);
    if (!element_type_or.ok()) return element_type_or.status();
    const ::googlesql::ArrayType* array_type = nullptr;
    absl::Status s = type_factory->MakeArrayType(*element_type_or, &array_type);
    if (!s.ok()) return s;
    return array_type;
  }
  return ParameterTypeForKind(parameter.type_kind);
}

absl::Status PopulateParameterOptions(const QueryRequest& request,
                                      ::googlesql::AnalyzerOptions& options,
                                      ::googlesql::TypeFactory* type_factory) {
  if (request.parameters.empty()) return absl::OkStatus();
  bool has_named = false;
  bool has_positional = false;
  // Positional parameters arrive with an empty name after
  // `ProtoToEngineRequest` strips synthetic `pN` keys. Explicit REST
  // named parameters may legally use `p0`/`p1` labels; treat any
  // non-empty name as named so analyzer mode matches `@p0` SQL.
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
    auto type_or = ParameterTypeForQueryParameter(p, type_factory);
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

absl::Status PopulateAnalyzerParameters(
    const QueryRequest& request,
    ::googlesql::AnalyzerOptions& options,
    ::googlesql::TypeFactory* type_factory) {
  return PopulateParameterOptions(request, options, type_factory);
}

absl::StatusOr<std::unique_ptr<AnalyzedQuery>> AnalyzeSelectQuery(
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

}  // namespace coordinator
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
