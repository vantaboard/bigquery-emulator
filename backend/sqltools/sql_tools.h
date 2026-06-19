#ifndef BIGQUERY_EMULATOR_BACKEND_SQLTOOLS_SQL_TOOLS_H_
#define BIGQUERY_EMULATOR_BACKEND_SQLTOOLS_SQL_TOOLS_H_

#include <cstddef>
#include <map>
#include <string>
#include <vector>

#include "absl/status/statusor.h"
#include "absl/strings/string_view.h"
#include "googlesql/public/catalog.h"
#include "googlesql/public/formatter_options.h"
#include "googlesql/public/language_options.h"

namespace bigquery_emulator {
namespace backend {
namespace sqltools {

struct SqlDiagnostic {
  int line = 0;
  int column = 0;
  std::string message;
  std::string severity = "error";
  int end_line = 0;
  int end_column = 0;
  int start_byte = -1;
  int end_byte = -1;
};

struct FormatOptions {
  bool strict = false;
  ::googlesql::FormatterOptions formatter_options;
};

struct FormatResult {
  std::string formatted_sql;
  std::vector<SqlDiagnostic> diagnostics;
};

struct ParseResult {
  std::vector<SqlDiagnostic> diagnostics;
  std::vector<std::string> statement_kinds;
};

struct SqlToken {
  std::string kind;
  std::string image;
  int start_byte = 0;
  int end_byte = 0;
};

struct TokenizeOptions {
  bool include_comments = false;
};

struct TokenizeResult {
  std::vector<SqlToken> tokens;
  std::vector<SqlDiagnostic> diagnostics;
};

// Catalog-backed name lists supplied by the caller (typically populated
// from emulator storage for dataset/table enumeration). The core library
// stays storage-agnostic; handlers fill this from GoogleSqlCatalog's
// backing store.
struct CatalogColumnEntry {
  std::string name;
  std::string type;
};

struct CatalogTableEntry {
  std::string label;
  std::string fqn;
  std::string kind;
  std::string detail;
};

struct CatalogRoutineEntry {
  std::string label;
  std::string fqn;
  std::string kind = "routine";
  std::string detail;
};

struct InScopeTableRef {
  std::string alias;
  std::string table_key;
  std::vector<CatalogColumnEntry> columns;
};

struct CatalogNames {
  std::vector<std::string> datasets;
  std::vector<CatalogTableEntry> tables;
  std::vector<CatalogRoutineEntry> routines;
  std::vector<CatalogColumnEntry> columns;
  std::map<std::string, std::vector<CatalogColumnEntry>> columns_by_table;
  std::vector<InScopeTableRef> in_scope_tables;
};

struct CompletionCandidate {
  std::string label;
  std::string kind;
  std::string insert_text;
  std::string detail;
};

struct CompleteResult {
  std::vector<CompletionCandidate> candidates;
  int replacement_start = 0;
  int replacement_end = 0;
};

::googlesql::LanguageOptions MakeSqlToolsLanguageOptions();

// Builds a diagnostic from a GoogleSQL status, optionally enriching
// span fields when `sql` is provided.
SqlDiagnostic DiagnosticFromStatusWithSql(const absl::Status& status,
                                          absl::string_view sql = {});

absl::StatusOr<FormatResult> FormatSqlText(absl::string_view sql,
                                           const FormatOptions& options);

absl::StatusOr<ParseResult> ParseSqlText(
    absl::string_view sql, const ::googlesql::LanguageOptions& language);

absl::StatusOr<TokenizeResult> TokenizeSqlText(
    absl::string_view sql,
    const ::googlesql::LanguageOptions& language,
    const TokenizeOptions& options);

absl::StatusOr<CompleteResult> CompleteSqlText(
    absl::string_view sql,
    size_t cursor_byte_offset,
    const ::googlesql::LanguageOptions& language,
    ::googlesql::Catalog* catalog,
    const CatalogNames& catalog_names,
    absl::string_view default_dataset_id = "");

}  // namespace sqltools
}  // namespace backend
}  // namespace bigquery_emulator

#endif  // BIGQUERY_EMULATOR_BACKEND_SQLTOOLS_SQL_TOOLS_H_
