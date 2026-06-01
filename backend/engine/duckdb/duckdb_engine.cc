#include "backend/engine/duckdb/duckdb_engine.h"

#include <memory>
#include <string>
#include <utility>

#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "absl/strings/str_cat.h"
#include "backend/engine/duckdb/duckdb_executor.h"
#include "backend/engine/engine.h"
#include "backend/schema/googlesql_to_bq.h"
#include "backend/schema/schema.h"
#include "backend/storage/storage.h"
#include "duckdb.h"
#include "googlesql/public/analyzer.h"
#include "googlesql/public/analyzer_options.h"
#include "googlesql/public/analyzer_output.h"
#include "googlesql/public/catalog.h"
#include "googlesql/public/language_options.h"
#include "googlesql/public/options.pb.h"
#include "googlesql/public/types/type_factory.h"
#include "googlesql/resolved_ast/resolved_ast.h"
#include "googlesql/resolved_ast/resolved_node_kind.pb.h"
#include "proto/emulator.pb.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace duckdb {

namespace {

// Returns the libduckdb C-API version. Calling this in the
// constructor pulls libduckdb's symbol table into the link line
// under `--as-needed` so libduckdb.so stays on the binary's
// DT_NEEDED list.
const char* DuckDBLibraryVersion() {
  return ::duckdb_library_version();
}

// LanguageOptions snapshot used for analyzer-driven name resolution.
// Matches the options the engine threads through `ExecuteQuery` so
// dispatch stays consistent across Analyze and execute paths.
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

// `MakeAnalyzerOptions` plus the all-statement-kind allowlist DML /
// DDL paths need. Matches the same call in
// `frontend/handlers/query.cc::MakeAnalyzerOptions` so handler-level
// analysis and engine-level analysis stay aligned on which shapes
// even reach the resolved-AST surface.
::googlesql::AnalyzerOptions MakeAnalyzerOptionsAllStatements() {
  ::googlesql::AnalyzerOptions options = MakeAnalyzerOptions();
  options.mutable_language()->SetSupportsAllStatementKinds();
  return options;
}

absl::Status ValidateRequest(const QueryRequest& request,
                             const ::googlesql::Catalog* catalog) {
  if (catalog == nullptr) {
    return absl::FailedPreconditionError(
        "DuckDBEngine: catalog must be non-null");
  }
  if (request.sql.empty()) {
    return absl::InvalidArgumentError("DuckDBEngine: request.sql is required");
  }
  if (request.use_legacy_sql) {
    return absl::InvalidArgumentError(
        "DuckDBEngine: useLegacySql=true is not supported; only "
        "GoogleSQL is implemented");
  }
  return absl::OkStatus();
}

// Build the BigQuery-shaped output schema from the analyzer's
// resolved-output-column list. Routed through the proto round-trip
// so the REPEATED-mode contract for ARRAY columns matches the
// surface the gateway emits.
absl::StatusOr<schema::TableSchema> ReflectOutputSchema(
    const ::googlesql::ResolvedQueryStmt& stmt) {
  v1::TableSchema proto;
  absl::Status s = backend::schema::OutputColumnListToTableSchema(
      stmt.output_column_list(), &proto);
  if (!s.ok()) return s;
  return backend::schema::TableSchemaFromProto(proto);
}

// Concrete `AnalyzedQuery` returned by `Analyze`. The resolved AST
// lives on the instance so a `DryRun` -> `ExecuteQuery` flow could
// reuse the analysis without re-parsing (today both methods
// re-analyze independently for simplicity).
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

// Analyze `request.sql` against `catalog` and return the
// `AnalyzerOutput` (which owns the resolved AST). DML / DDL callers
// pass `all_statements=true` so the analyzer accepts every supported
// statement kind, not just SELECT.
absl::StatusOr<std::unique_ptr<const ::googlesql::AnalyzerOutput>>
AnalyzeStatement(const QueryRequest& request,
                 ::googlesql::Catalog* catalog,
                 bool all_statements) {
  ::googlesql::AnalyzerOptions options = all_statements
                                             ? MakeAnalyzerOptionsAllStatements()
                                             : MakeAnalyzerOptions();
  // The arena-backed `TypeFactory` is destroyed when `AnalyzerOutput`
  // is destroyed; we therefore allocate it on the heap and keep it
  // alive for the lifetime of the analyzer output via the existing
  // `AnalyzedQuery` lifetime contract.
  ::googlesql::TypeFactory type_factory;
  std::unique_ptr<const ::googlesql::AnalyzerOutput> output;
  absl::Status analyze = ::googlesql::AnalyzeStatement(
      request.sql, options, catalog, &type_factory, &output);
  if (!analyze.ok()) return analyze;
  if (output == nullptr || output->resolved_statement() == nullptr) {
    return absl::InternalError(
        "DuckDBEngine: analyzer returned no resolved statement");
  }
  return output;
}

}  // namespace

DuckDBEngine::DuckDBEngine(storage::Storage* storage)
    : storage_(storage), executor_(storage) {
  (void)DuckDBLibraryVersion();
}

DuckDBEngine::~DuckDBEngine() = default;

absl::StatusOr<std::unique_ptr<AnalyzedQuery>> DuckDBEngine::Analyze(
    const QueryRequest& request, ::googlesql::Catalog* catalog) {
  absl::Status valid = ValidateRequest(request, catalog);
  if (!valid.ok()) return valid;

  absl::StatusOr<std::unique_ptr<const ::googlesql::AnalyzerOutput>> output =
      AnalyzeStatement(request, catalog, /*all_statements=*/false);
  if (!output.ok()) return output.status();
  const ::googlesql::ResolvedStatement* stmt = (*output)->resolved_statement();
  if (stmt->node_kind() != ::googlesql::RESOLVED_QUERY_STMT) {
    return absl::InvalidArgumentError(
        absl::StrCat("DuckDBEngine::Analyze: only SELECT-shaped queries are "
                     "supported; got ",
                     stmt->node_kind_string()));
  }
  absl::StatusOr<schema::TableSchema> output_schema =
      ReflectOutputSchema(*stmt->GetAs<::googlesql::ResolvedQueryStmt>());
  if (!output_schema.ok()) return output_schema.status();
  return std::make_unique<AnalyzedQueryImpl>(std::move(*output),
                                             std::move(*output_schema));
}

absl::StatusOr<DryRunResult> DuckDBEngine::DryRun(
    const QueryRequest& request, ::googlesql::Catalog* catalog) {
  absl::StatusOr<std::unique_ptr<AnalyzedQuery>> analyzed =
      Analyze(request, catalog);
  if (!analyzed.ok()) return analyzed.status();
  DryRunResult result;
  result.schema = (*analyzed)->output_schema();
  // We don't have a cost model yet; surface zero so the gateway's
  // wire envelope stays well-formed.
  result.estimated_bytes_processed = 0;
  return result;
}

absl::StatusOr<std::unique_ptr<RowSource>> DuckDBEngine::ExecuteQuery(
    const QueryRequest& request, ::googlesql::Catalog* catalog) {
  absl::Status valid = ValidateRequest(request, catalog);
  if (!valid.ok()) return valid;
  absl::StatusOr<std::unique_ptr<const ::googlesql::AnalyzerOutput>> output =
      AnalyzeStatement(request, catalog, /*all_statements=*/false);
  if (!output.ok()) return output.status();
  return executor_.ExecuteQuery(
      request, *(*output)->resolved_statement(), catalog);
}

absl::StatusOr<DmlStats> DuckDBEngine::ExecuteDml(
    const QueryRequest& request, ::googlesql::Catalog* catalog) {
  absl::Status valid = ValidateRequest(request, catalog);
  if (!valid.ok()) return valid;
  absl::StatusOr<std::unique_ptr<const ::googlesql::AnalyzerOutput>> output =
      AnalyzeStatement(request, catalog, /*all_statements=*/true);
  if (!output.ok()) return output.status();
  return executor_.ExecuteDml(
      request, *(*output)->resolved_statement(), catalog);
}

absl::Status DuckDBEngine::ExecuteDdl(const QueryRequest& request,
                                      ::googlesql::Catalog* catalog) {
  absl::Status valid = ValidateRequest(request, catalog);
  if (!valid.ok()) return valid;
  absl::StatusOr<std::unique_ptr<const ::googlesql::AnalyzerOutput>> output =
      AnalyzeStatement(request, catalog, /*all_statements=*/true);
  if (!output.ok()) return output.status();
  return executor_.ExecuteDdl(
      request, *(*output)->resolved_statement(), catalog);
}

}  // namespace duckdb
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
