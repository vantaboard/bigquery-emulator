#ifndef BIGQUERY_EMULATOR_BACKEND_SQLTOOLS_SQL_TOOLS_H_
#define BIGQUERY_EMULATOR_BACKEND_SQLTOOLS_SQL_TOOLS_H_

#include <cstddef>
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
struct CatalogNames {
  std::vector<std::string> datasets;
  std::vector<std::string> tables;
  std::vector<std::string> columns;
};

struct CompletionCandidate {
  std::string label;
  std::string kind;
  std::string insert_text;
};

struct CompleteResult {
  std::vector<CompletionCandidate> candidates;
  int replacement_start = 0;
  int replacement_end = 0;
};

::googlesql::LanguageOptions MakeSqlToolsLanguageOptions();

absl::StatusOr<FormatResult> FormatSqlText(absl::string_view sql,
                                           const FormatOptions& options);

absl::StatusOr<ParseResult> ParseSqlText(
    absl::string_view sql, const ::googlesql::LanguageOptions& language);

absl::StatusOr<TokenizeResult> TokenizeSqlText(
    absl::string_view sql, const ::googlesql::LanguageOptions& language,
    const TokenizeOptions& options);

absl::StatusOr<CompleteResult> CompleteSqlText(
    absl::string_view sql, size_t cursor_byte_offset,
    const ::googlesql::LanguageOptions& language, ::googlesql::Catalog* catalog,
    const CatalogNames& catalog_names,
    absl::string_view default_dataset_id = "");

}  // namespace sqltools
}  // namespace backend
}  // namespace bigquery_emulator

#endif  // BIGQUERY_EMULATOR_BACKEND_SQLTOOLS_SQL_TOOLS_H_
