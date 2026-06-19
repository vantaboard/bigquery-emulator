#ifndef BIGQUERY_EMULATOR_BACKEND_SQLTOOLS_SQL_REFERENCES_H_
#define BIGQUERY_EMULATOR_BACKEND_SQLTOOLS_SQL_REFERENCES_H_

#include <string>
#include <vector>

#include "absl/status/statusor.h"
#include "absl/strings/string_view.h"
#include "backend/sqltools/sql_tools.h"
#include "googlesql/public/catalog.h"
#include "googlesql/public/language_options.h"

namespace bigquery_emulator {
namespace backend {
namespace sqltools {

struct ReferencedTable {
  std::string project_id;
  std::string dataset_id;
  std::string table_id;
  std::string alias;
  std::string kind;
  std::vector<CatalogColumnEntry> columns;
};

struct AnalyzeResult {
  std::vector<ReferencedTable> referenced_tables;
  std::vector<std::string> statement_kinds;
  std::vector<SqlDiagnostic> diagnostics;
};

// Parses and analyzes SQL to collect referenced tables/views and their
// columns. Diagnostics are returned on parse/analyze failure; referenced
// tables may be empty in that case.
absl::StatusOr<AnalyzeResult> AnalyzeSqlText(
    absl::string_view sql,
    absl::string_view project_id,
    absl::string_view default_dataset_id,
    ::googlesql::Catalog* catalog,
    const ::googlesql::LanguageOptions& language);

// Populates CatalogNames::in_scope_tables from a successful analyze result.
void PopulateInScopeTablesFromAnalyze(const AnalyzeResult& analyze,
                                      CatalogNames* names);

}  // namespace sqltools
}  // namespace backend
}  // namespace bigquery_emulator

#endif  // BIGQUERY_EMULATOR_BACKEND_SQLTOOLS_SQL_REFERENCES_H_
