#include "frontend/handlers/sqltools.h"

#include "absl/status/statusor.h"
#include "backend/catalog/googlesql_catalog.h"
#include "backend/sqltools/catalog_names.h"
#include "backend/sqltools/sql_references.h"
#include "backend/sqltools/sql_tools.h"
#include "frontend/handlers/query_internal.h"
#include "googlesql/public/types/type_factory.h"

namespace bigquery_emulator {
namespace frontend {
namespace {

using internal::AnalyzeStatusToGrpc;

void CopyDiagnostic(const backend::sqltools::SqlDiagnostic& in,
                    v1::SqlDiagnostic* out) {
  out->set_line(in.line);
  out->set_column(in.column);
  out->set_message(in.message);
  out->set_severity(in.severity);
  if (in.end_line > 0) {
    out->set_end_line(in.end_line);
  }
  if (in.end_column > 0) {
    out->set_end_column(in.end_column);
  }
  if (in.start_byte >= 0) {
    out->set_start_byte(in.start_byte);
  }
  if (in.end_byte >= 0) {
    out->set_end_byte(in.end_byte);
  }
}

::googlesql::FormatterOptions FormatterOptionsFromRequest(
    const v1::FormatSqlRequest& request) {
  ::googlesql::FormatterOptions options;
  if (request.line_length_limit() > 0) {
    options.SetLineLengthLimit(request.line_length_limit());
  }
  if (request.indentation_spaces() > 0) {
    options.SetIndentationSpaces(request.indentation_spaces());
  }
  return options;
}

}  // namespace

SqlToolsService::SqlToolsService(backend::storage::Storage* storage)
    : storage_(storage) {}

::grpc::Status SqlToolsService::Format(::grpc::ServerContext* /*context*/,
                                       const v1::FormatSqlRequest* request,
                                       v1::FormatSqlResponse* response) {
  if (response == nullptr) {
    return ::grpc::Status(::grpc::StatusCode::INTERNAL,
                          "SqlToolsService::Format: response is null");
  }
  response->Clear();
  if (request == nullptr || request->sql().empty()) {
    return ::grpc::Status(::grpc::StatusCode::INVALID_ARGUMENT,
                          "SqlToolsService::Format: sql is required");
  }

  backend::sqltools::FormatOptions options;
  options.strict = request->strict();
  options.formatter_options = FormatterOptionsFromRequest(*request);
  const absl::StatusOr<backend::sqltools::FormatResult> result =
      backend::sqltools::FormatSqlText(request->sql(), options);
  if (!result.ok()) {
    return AnalyzeStatusToGrpc(result.status());
  }
  response->set_formatted_sql(result->formatted_sql);
  for (const backend::sqltools::SqlDiagnostic& diag : result->diagnostics) {
    CopyDiagnostic(diag, response->add_diagnostics());
  }
  return ::grpc::Status::OK;
}

::grpc::Status SqlToolsService::Parse(::grpc::ServerContext* /*context*/,
                                      const v1::ParseSqlRequest* request,
                                      v1::ParseSqlResponse* response) {
  if (response == nullptr) {
    return ::grpc::Status(::grpc::StatusCode::INTERNAL,
                          "SqlToolsService::Parse: response is null");
  }
  response->Clear();
  if (request == nullptr || request->sql().empty()) {
    return ::grpc::Status(::grpc::StatusCode::INVALID_ARGUMENT,
                          "SqlToolsService::Parse: sql is required");
  }

  const ::googlesql::LanguageOptions language =
      backend::sqltools::MakeSqlToolsLanguageOptions();
  const absl::StatusOr<backend::sqltools::ParseResult> result =
      backend::sqltools::ParseSqlText(request->sql(), language);
  if (!result.ok()) {
    return AnalyzeStatusToGrpc(result.status());
  }
  for (const backend::sqltools::SqlDiagnostic& diag : result->diagnostics) {
    CopyDiagnostic(diag, response->add_diagnostics());
  }
  for (const std::string& kind : result->statement_kinds) {
    response->add_statement_kinds(kind);
  }
  return ::grpc::Status::OK;
}

::grpc::Status SqlToolsService::Tokenize(::grpc::ServerContext* /*context*/,
                                         const v1::TokenizeSqlRequest* request,
                                         v1::TokenizeSqlResponse* response) {
  if (response == nullptr) {
    return ::grpc::Status(::grpc::StatusCode::INTERNAL,
                          "SqlToolsService::Tokenize: response is null");
  }
  response->Clear();
  if (request == nullptr || request->sql().empty()) {
    return ::grpc::Status(::grpc::StatusCode::INVALID_ARGUMENT,
                          "SqlToolsService::Tokenize: sql is required");
  }

  const ::googlesql::LanguageOptions language =
      backend::sqltools::MakeSqlToolsLanguageOptions();
  backend::sqltools::TokenizeOptions options;
  options.include_comments = request->include_comments();
  const absl::StatusOr<backend::sqltools::TokenizeResult> result =
      backend::sqltools::TokenizeSqlText(request->sql(), language, options);
  if (!result.ok()) {
    return AnalyzeStatusToGrpc(result.status());
  }
  for (const backend::sqltools::SqlDiagnostic& diag : result->diagnostics) {
    CopyDiagnostic(diag, response->add_diagnostics());
  }
  for (const backend::sqltools::SqlToken& token : result->tokens) {
    v1::SqlToken* out = response->add_tokens();
    out->set_kind(token.kind);
    out->set_image(token.image);
    out->set_start_byte(token.start_byte);
    out->set_end_byte(token.end_byte);
  }
  return ::grpc::Status::OK;
}

::grpc::Status SqlToolsService::Complete(::grpc::ServerContext* /*context*/,
                                         const v1::CompleteSqlRequest* request,
                                         v1::CompleteSqlResponse* response) {
  if (response == nullptr) {
    return ::grpc::Status(::grpc::StatusCode::INTERNAL,
                          "SqlToolsService::Complete: response is null");
  }
  response->Clear();
  if (request == nullptr) {
    return ::grpc::Status(::grpc::StatusCode::INVALID_ARGUMENT,
                          "SqlToolsService::Complete: request is null");
  }
  if (request->project_id().empty()) {
    return ::grpc::Status(::grpc::StatusCode::INVALID_ARGUMENT,
                          "SqlToolsService::Complete: project_id is required");
  }
  if (request->sql().empty() && request->cursor_byte_offset() != 0) {
    return ::grpc::Status(::grpc::StatusCode::INVALID_ARGUMENT,
                          "SqlToolsService::Complete: empty sql requires "
                          "cursor_byte_offset 0");
  }
  if (storage_ == nullptr) {
    return ::grpc::Status(
        ::grpc::StatusCode::FAILED_PRECONDITION,
        "SqlToolsService::Complete: storage backend is not configured");
  }

  ::googlesql::TypeFactory type_factory;
  const ::googlesql::LanguageOptions language =
      backend::sqltools::MakeSqlToolsLanguageOptions();
  backend::catalog::GoogleSqlCatalog catalog(request->project_id(),
                                             storage_,
                                             &type_factory,
                                             language,
                                             request->default_dataset_id());

  backend::sqltools::CatalogNames names;
  const absl::Status populate =
      backend::sqltools::PopulateCatalogNamesFromStorage(
          request->project_id(),
          request->default_dataset_id(),
          storage_,
          &names);
  if (!populate.ok()) {
    return AnalyzeStatusToGrpc(populate);
  }

  if (!request->sql().empty()) {
    const absl::StatusOr<backend::sqltools::AnalyzeResult> analyze =
        backend::sqltools::AnalyzeSqlText(request->sql(),
                                          request->project_id(),
                                          request->default_dataset_id(),
                                          &catalog,
                                          language);
    if (analyze.ok() && !analyze->referenced_tables.empty()) {
      backend::sqltools::PopulateInScopeTablesFromAnalyze(*analyze, &names);
    } else {
      backend::sqltools::PopulateInScopeTablesFromHeuristic(
          request->sql(), language, request->default_dataset_id(), &names);
    }
  }

  const size_t cursor =
      request->cursor_byte_offset() < 0
          ? request->sql().size()
          : static_cast<size_t>(request->cursor_byte_offset());
  const absl::StatusOr<backend::sqltools::CompleteResult> result =
      backend::sqltools::CompleteSqlText(request->sql(),
                                         cursor,
                                         language,
                                         &catalog,
                                         names,
                                         request->default_dataset_id());
  if (!result.ok()) {
    return AnalyzeStatusToGrpc(result.status());
  }

  response->set_replacement_start(result->replacement_start);
  response->set_replacement_end(result->replacement_end);
  for (const backend::sqltools::CompletionCandidate& candidate :
       result->candidates) {
    v1::SqlCompletionCandidate* out = response->add_candidates();
    out->set_label(candidate.label);
    out->set_kind(candidate.kind);
    out->set_insert_text(candidate.insert_text);
    if (!candidate.detail.empty()) {
      out->set_detail(candidate.detail);
    }
    if (!candidate.fqn.empty()) {
      out->set_fqn(candidate.fqn);
    }
  }
  return ::grpc::Status::OK;
}

::grpc::Status SqlToolsService::Analyze(::grpc::ServerContext* /*context*/,
                                        const v1::AnalyzeSqlRequest* request,
                                        v1::AnalyzeSqlResponse* response) {
  if (response == nullptr) {
    return ::grpc::Status(::grpc::StatusCode::INTERNAL,
                          "SqlToolsService::Analyze: response is null");
  }
  response->Clear();
  if (request == nullptr) {
    return ::grpc::Status(::grpc::StatusCode::INVALID_ARGUMENT,
                          "SqlToolsService::Analyze: request is null");
  }
  if (request->project_id().empty()) {
    return ::grpc::Status(::grpc::StatusCode::INVALID_ARGUMENT,
                          "SqlToolsService::Analyze: project_id is required");
  }
  if (request->sql().empty()) {
    return ::grpc::Status(::grpc::StatusCode::INVALID_ARGUMENT,
                          "SqlToolsService::Analyze: sql is required");
  }
  if (storage_ == nullptr) {
    return ::grpc::Status(
        ::grpc::StatusCode::FAILED_PRECONDITION,
        "SqlToolsService::Analyze: storage backend is not configured");
  }

  ::googlesql::TypeFactory type_factory;
  const ::googlesql::LanguageOptions language =
      backend::sqltools::MakeSqlToolsLanguageOptions();
  backend::catalog::GoogleSqlCatalog catalog(request->project_id(),
                                             storage_,
                                             &type_factory,
                                             language,
                                             request->default_dataset_id());

  const absl::StatusOr<backend::sqltools::AnalyzeResult> result =
      backend::sqltools::AnalyzeSqlText(request->sql(),
                                        request->project_id(),
                                        request->default_dataset_id(),
                                        &catalog,
                                        language);
  if (!result.ok()) {
    return AnalyzeStatusToGrpc(result.status());
  }

  for (const backend::sqltools::SqlDiagnostic& diag : result->diagnostics) {
    CopyDiagnostic(diag, response->add_diagnostics());
  }
  for (const std::string& kind : result->statement_kinds) {
    response->add_statement_kinds(kind);
  }
  for (const backend::sqltools::ReferencedTable& table :
       result->referenced_tables) {
    v1::ReferencedTable* out = response->add_referenced_tables();
    out->set_project_id(table.project_id);
    out->set_dataset_id(table.dataset_id);
    out->set_table_id(table.table_id);
    out->set_alias(table.alias);
    out->set_kind(table.kind);
  }
  return ::grpc::Status::OK;
}

}  // namespace frontend
}  // namespace bigquery_emulator
