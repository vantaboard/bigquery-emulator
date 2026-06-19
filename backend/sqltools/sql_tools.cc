#include "backend/sqltools/sql_tools.h"

#include <algorithm>
#include <cctype>
#include <string>
#include <utility>
#include <vector>

#include "absl/container/flat_hash_set.h"
#include "absl/status/status.h"
#include "absl/strings/ascii.h"
#include "absl/strings/match.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/str_split.h"
#include "googlesql/parser/ast_node.h"
#include "googlesql/parser/parser.h"
#include "googlesql/public/error_helpers.h"
#include "googlesql/public/error_location.pb.h"
#include "googlesql/public/function.h"
#include "googlesql/public/lenient_formatter.h"
#include "googlesql/public/parse_resume_location.h"
#include "googlesql/public/parse_tokens.h"
#include "googlesql/public/simple_catalog.h"
#include "googlesql/public/sql_formatter.h"

namespace bigquery_emulator {
namespace backend {
namespace sqltools {
namespace {

std::string TokenKindLabel(const ::googlesql::ParseToken& token) {
  if (token.IsEndOfInput()) return "end_of_input";
  if (token.IsComment()) return "comment";
  if (token.IsValue()) return "value";
  if (token.IsIdentifier()) return "identifier";
  if (token.IsKeyword()) return "keyword";
  return "unknown";
}

::googlesql::ParserOptions MakeParserOptions(
    const ::googlesql::LanguageOptions& language) {
  ::googlesql::ParserOptions options(language);
  options.CreateDefaultArenasIfNotSet();
  return options;
}

}  // namespace

namespace {

int ByteOffsetFromLineAndColumn(absl::string_view sql, int line, int column) {
  if (line <= 0 || column <= 0) {
    return -1;
  }
  int current_line = 1;
  int current_column = 1;
  for (int i = 0; i < static_cast<int>(sql.size()); ++i) {
    if (current_line == line && current_column == column) {
      return i;
    }
    if (sql[static_cast<size_t>(i)] == '\n') {
      ++current_line;
      current_column = 1;
    } else {
      ++current_column;
    }
  }
  if (current_line == line && current_column == column) {
    return static_cast<int>(sql.size());
  }
  return -1;
}

}  // namespace

SqlDiagnostic DiagnosticFromStatusWithSql(const absl::Status& status,
                                          absl::string_view sql) {
  SqlDiagnostic diag;
  diag.message = std::string(status.message());

  ::googlesql::ErrorLocation location;
  if (::googlesql::GetErrorLocation(status, &location)) {
    diag.line = static_cast<int>(location.line());
    diag.column = static_cast<int>(location.column());
  }

  if (sql.empty() || diag.line <= 0 || diag.column <= 0) {
    return diag;
  }

  const int start_byte =
      ByteOffsetFromLineAndColumn(sql, diag.line, diag.column);
  if (start_byte < 0) {
    return diag;
  }

  diag.start_byte = start_byte;
  int end_byte = start_byte + 1;

  ::googlesql::ParseTokenOptions token_options;
  token_options.language_options = MakeSqlToolsLanguageOptions();
  ::googlesql::ParseResumeLocation token_resume =
      ::googlesql::ParseResumeLocation::FromStringView(sql);
  std::vector<::googlesql::ParseToken> tokens;
  if (::googlesql::GetParseTokens(token_options, &token_resume, &tokens).ok()) {
    for (const ::googlesql::ParseToken& token : tokens) {
      const ::googlesql::ParseLocationRange range = token.GetLocationRange();
      const int token_start = static_cast<int>(range.start().GetByteOffset());
      const int token_end = static_cast<int>(range.end().GetByteOffset());
      if (start_byte >= token_start && start_byte < token_end) {
        end_byte = token_end;
        break;
      }
    }
  }

  if (end_byte <= start_byte) {
    end_byte = std::min(static_cast<int>(sql.size()), start_byte + 1);
  }
  diag.end_byte = end_byte;

  int end_line = diag.line;
  int end_column = diag.column;
  for (int i = start_byte; i < end_byte && i < static_cast<int>(sql.size());
       ++i) {
    if (sql[static_cast<size_t>(i)] == '\n') {
      ++end_line;
      end_column = 1;
    } else {
      ++end_column;
    }
  }
  diag.end_line = end_line;
  diag.end_column = end_column;
  return diag;
}

::googlesql::LanguageOptions MakeSqlToolsLanguageOptions() {
  ::googlesql::LanguageOptions language;
  language.EnableMaximumLanguageFeaturesForDevelopment();
  language.EnableLanguageFeature(::googlesql::FEATURE_WITH_EXPRESSION);
  language.EnableLanguageFeature(::googlesql::FEATURE_MATCH_RECOGNIZE);
  language.EnableLanguageFeature(
      ::googlesql::FEATURE_STRATIFIED_RESERVOIR_TABLESAMPLE);
  language.EnableLanguageFeature(::googlesql::FEATURE_KLL_WEIGHTS);
  language.EnableLanguageFeature(::googlesql::FEATURE_CREATE_TABLE_CLONE);
  language.EnableLanguageFeature(::googlesql::FEATURE_CREATE_SNAPSHOT_TABLE);
  language.EnableLanguageFeature(::googlesql::FEATURE_CLONE_DATA);
  language.EnableLanguageFeature(::googlesql::FEATURE_REMOTE_MODEL);
  language.EnableLanguageFeature(::googlesql::FEATURE_ENABLE_MEASURES);
  language.set_product_mode(::googlesql::PRODUCT_EXTERNAL);
  language.set_name_resolution_mode(::googlesql::NAME_RESOLUTION_DEFAULT);
  language.SetSupportsAllStatementKinds();
  return language;
}

absl::StatusOr<FormatResult> FormatSqlText(absl::string_view sql,
                                           const FormatOptions& options) {
  FormatResult result;
  absl::Status status;
  if (options.strict) {
    status = ::googlesql::FormatSql(sql, &result.formatted_sql);
  } else {
    status = ::googlesql::LenientFormatSql(
        sql, &result.formatted_sql, options.formatter_options);
  }
  if (!status.ok()) {
    result.diagnostics.push_back(DiagnosticFromStatusWithSql(status, sql));
  }
  return result;
}

absl::StatusOr<ParseResult> ParseSqlText(
    absl::string_view sql, const ::googlesql::LanguageOptions& language) {
  ParseResult result;
  ::googlesql::ParseResumeLocation resume =
      ::googlesql::ParseResumeLocation::FromStringView(sql);
  const ::googlesql::ParserOptions parser_options = MakeParserOptions(language);
  bool at_end = false;
  while (!at_end) {
    std::unique_ptr<::googlesql::ParserOutput> output;
    const absl::Status parse_status = ::googlesql::ParseNextStatement(
        &resume, parser_options, &output, &at_end);
    if (!parse_status.ok()) {
      result.diagnostics.push_back(
          DiagnosticFromStatusWithSql(parse_status, sql));
      break;
    }
    if (output != nullptr && output->statement() != nullptr) {
      result.statement_kinds.push_back(::googlesql::ASTNode::NodeKindToString(
          output->statement()->node_kind()));
    }
  }
  return result;
}

absl::StatusOr<TokenizeResult> TokenizeSqlText(
    absl::string_view sql,
    const ::googlesql::LanguageOptions& language,
    const TokenizeOptions& options) {
  TokenizeResult result;
  ::googlesql::ParseTokenOptions token_options;
  token_options.language_options = language;
  token_options.include_comments = options.include_comments;
  ::googlesql::ParseResumeLocation resume =
      ::googlesql::ParseResumeLocation::FromStringView(sql);
  std::vector<::googlesql::ParseToken> parse_tokens;
  const absl::Status token_status =
      ::googlesql::GetParseTokens(token_options, &resume, &parse_tokens);
  if (!token_status.ok()) {
    result.diagnostics.push_back(
        DiagnosticFromStatusWithSql(token_status, sql));
    return result;
  }
  result.tokens.reserve(parse_tokens.size());
  for (const ::googlesql::ParseToken& token : parse_tokens) {
    SqlToken out;
    out.kind = TokenKindLabel(token);
    out.image = std::string(token.GetImage());
    const ::googlesql::ParseLocationRange range = token.GetLocationRange();
    out.start_byte = static_cast<int>(range.start().GetByteOffset());
    out.end_byte = static_cast<int>(range.end().GetByteOffset());
    result.tokens.push_back(std::move(out));
  }
  return result;
}

}  // namespace sqltools
}  // namespace backend
}  // namespace bigquery_emulator
