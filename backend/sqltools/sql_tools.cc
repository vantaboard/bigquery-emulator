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

SqlDiagnostic DiagnosticFromStatus(const absl::Status& status) {
  SqlDiagnostic diag;
  diag.message = std::string(status.message());
  ::googlesql::ErrorLocation location;
  if (::googlesql::GetErrorLocation(status, &location)) {
    diag.line = static_cast<int>(location.line());
    diag.column = static_cast<int>(location.column());
  }
  return diag;
}

std::string TokenKindLabel(const ::googlesql::ParseToken& token) {
  if (token.IsEndOfInput()) return "end_of_input";
  if (token.IsComment()) return "comment";
  if (token.IsValue()) return "value";
  if (token.IsIdentifier()) return "identifier";
  if (token.IsKeyword()) return "keyword";
  return "unknown";
}

bool IsTableContextKeyword(absl::string_view keyword) {
  static const absl::flat_hash_set<std::string>* kKeywords =
      new absl::flat_hash_set<std::string>{
          "FROM",   "JOIN",  "INNER", "LEFT",   "RIGHT", "FULL",
          "CROSS",  "INTO",  "UPDATE", "TABLE", "MERGE", "USING",
          "DELETE", "TRUNCATE"};
  return kKeywords->contains(std::string(keyword));
}

bool IsColumnContextKeyword(absl::string_view keyword) {
  static const absl::flat_hash_set<std::string>* kKeywords =
      new absl::flat_hash_set<std::string>{
          "SELECT", "WHERE", "ON",     "BY",      "HAVING", "QUALIFY",
          "SET",    "AND",   "OR",     "NOT",     "WHEN",   "THEN",
          "ELSE",   "CASE",  "GROUP",  "ORDER",   "PARTITION"};
  return kKeywords->contains(std::string(keyword));
}

std::string TokenText(const ::googlesql::ParseToken& token) {
  if (token.IsIdentifier()) return token.GetIdentifier();
  if (token.IsKeyword()) return token.GetKeyword();
  return std::string(token.GetImage());
}

void AppendUniqueCandidate(std::vector<CompletionCandidate>* out,
                           CompletionCandidate candidate) {
  for (const CompletionCandidate& existing : *out) {
    if (existing.label == candidate.label && existing.kind == candidate.kind) {
      return;
    }
  }
  out->push_back(std::move(candidate));
}

void AppendPrefixMatches(absl::string_view prefix,
                         absl::string_view kind,
                         const std::vector<std::string>& names,
                         std::vector<CompletionCandidate>* out) {
  for (const std::string& name : names) {
    if (prefix.empty() ||
        absl::StartsWithIgnoreCase(name, prefix)) {
      AppendUniqueCandidate(out,
                            CompletionCandidate{name, std::string(kind), name});
    }
  }
}

const std::vector<std::string>& SqlKeywords() {
  static const std::vector<std::string>* kKeywords = new std::vector<std::string>{
      "ALL",        "AND",       "ANY",         "ARRAY",     "AS",       "ASC",
      "BETWEEN",    "BY",        "CASE",        "CAST",      "COLLATE",  "CREATE",
      "CROSS",      "CUBE",      "CURRENT",     "DEFAULT",   "DELETE",   "DESC",
      "DISTINCT",   "DROP",      "ELSE",        "END",       "EXCEPT",   "EXISTS",
      "FALSE",      "FOR",       "FROM",        "FULL",      "GROUP",    "GROUPING",
      "HAVING",     "IF",        "IN",          "INNER",     "INSERT",   "INTERSECT",
      "INTO",       "IS",        "JOIN",        "LEFT",      "LIKE",     "LIMIT",
      "MERGE",      "NOT",       "NULL",        "NULLS",     "OF",       "OFFSET",
      "ON",         "OR",        "ORDER",       "OUTER",     "OVER",     "PARTITION",
      "PIVOT",      "QUALIFY",   "REGEXP",      "RIGHT",     "ROLLUP",   "ROW",
      "ROWS",       "SAFE_CAST", "SELECT",      "SET",       "SOME",     "STRUCT",
      "TABLE",      "THEN",      "TO",          "TRUE",      "UNION",    "UNNEST",
      "UPDATE",     "USING",     "WHEN",        "WHERE",     "WINDOW",   "WITH",
      "WITHIN"};
  return *kKeywords;
}

void AppendFunctionCandidates(::googlesql::Catalog* catalog,
                              absl::string_view prefix,
                              std::vector<CompletionCandidate>* out) {
  auto* simple = dynamic_cast<::googlesql::SimpleCatalog*>(catalog);
  if (simple == nullptr) return;
  absl::flat_hash_set<const ::googlesql::Function*> functions;
  if (!simple->GetFunctions(&functions).ok()) return;
  for (const ::googlesql::Function* function : functions) {
    if (function == nullptr) continue;
    const std::string name = function->Name();
    if (prefix.empty() || absl::StartsWithIgnoreCase(name, prefix)) {
      AppendUniqueCandidate(
          out, CompletionCandidate{name, "function", name + "("});
    }
  }
}

void AppendColumnCandidatesForTable(::googlesql::Catalog* catalog,
                                    absl::string_view project_id,
                                    absl::string_view default_dataset,
                                    absl::string_view table_ref,
                                    absl::string_view prefix,
                                    std::vector<CompletionCandidate>* out) {
  std::vector<std::string> path;
  for (absl::string_view part :
       absl::StrSplit(table_ref, '.', absl::SkipEmpty())) {
    path.push_back(std::string(part));
  }
  if (path.size() == 1 && !default_dataset.empty()) {
    path.insert(path.begin(), std::string(default_dataset));
  }
  if (path.size() == 1 && !project_id.empty()) {
    // Single unqualified name with no default dataset: try as table in catalog.
  }
  const ::googlesql::Table* table = nullptr;
  if (catalog->FindTable(path, &table).ok() && table != nullptr) {
    for (int i = 0; i < table->NumColumns(); ++i) {
      const std::string col = std::string(table->GetColumn(i)->Name());
      if (prefix.empty() || absl::StartsWithIgnoreCase(col, prefix)) {
        AppendUniqueCandidate(out, CompletionCandidate{col, "column", col});
      }
    }
  }
}

enum class CompletionContextKind {
  kGeneral,
  kTable,
  kColumn,
  kMember,
};

struct CompletionContext {
  CompletionContextKind kind = CompletionContextKind::kGeneral;
  std::string qualifier;
};

CompletionContext InferCompletionContext(
    const std::vector<::googlesql::ParseToken>& tokens) {
  CompletionContext ctx;
  if (tokens.empty()) return ctx;

  int idx = static_cast<int>(tokens.size()) - 1;
  while (idx >= 0 && tokens[idx].IsEndOfInput()) {
    --idx;
  }
  if (idx < 0) return ctx;

  if (idx > 0 && tokens[idx - 1].GetKeyword() == ".") {
    ctx.kind = CompletionContextKind::kMember;
    ctx.qualifier = TokenText(tokens[idx - 2]);
    return ctx;
  }

  const std::string last_keyword = tokens[idx].GetKeyword();
  if (IsTableContextKeyword(last_keyword)) {
    ctx.kind = CompletionContextKind::kTable;
    return ctx;
  }

  for (int i = idx; i >= 0; --i) {
    if (!tokens[i].IsKeyword()) continue;
    const std::string kw = tokens[i].GetKeyword();
    if (IsTableContextKeyword(kw)) {
      ctx.kind = CompletionContextKind::kTable;
      return ctx;
    }
    if (IsColumnContextKeyword(kw)) {
      ctx.kind = CompletionContextKind::kColumn;
      return ctx;
    }
  }
  return ctx;
}

void FindReplacementSpan(absl::string_view sql, size_t cursor,
                         int* replacement_start, int* replacement_end,
                         std::string* prefix) {
  *replacement_start = static_cast<int>(cursor);
  *replacement_end = static_cast<int>(cursor);
  prefix->clear();
  if (cursor > sql.size()) cursor = sql.size();
  size_t start = cursor;
  while (start > 0) {
    const char c = sql[start - 1];
    if (!std::isalnum(static_cast<unsigned char>(c)) && c != '_') break;
    --start;
  }
  *replacement_start = static_cast<int>(start);
  *replacement_end = static_cast<int>(cursor);
  prefix->assign(sql.substr(start, cursor - start));
}

::googlesql::ParserOptions MakeParserOptions(
    const ::googlesql::LanguageOptions& language) {
  ::googlesql::ParserOptions options(language);
  options.CreateDefaultArenasIfNotSet();
  return options;
}

}  // namespace

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
    status = ::googlesql::LenientFormatSql(sql, &result.formatted_sql,
                                           options.formatter_options);
  }
  if (!status.ok()) {
    result.diagnostics.push_back(DiagnosticFromStatus(status));
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
      result.diagnostics.push_back(DiagnosticFromStatus(parse_status));
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
    absl::string_view sql, const ::googlesql::LanguageOptions& language,
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
    result.diagnostics.push_back(DiagnosticFromStatus(token_status));
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

absl::StatusOr<CompleteResult> CompleteSqlText(
    absl::string_view sql, size_t cursor_byte_offset,
    const ::googlesql::LanguageOptions& language, ::googlesql::Catalog* catalog,
    const CatalogNames& catalog_names, absl::string_view default_dataset_id) {
  CompleteResult result;
  if (catalog == nullptr) {
    return absl::InvalidArgumentError("CompleteSqlText: catalog is required");
  }
  if (cursor_byte_offset > sql.size()) {
    cursor_byte_offset = sql.size();
  }
  const absl::string_view prefix_sql = sql.substr(0, cursor_byte_offset);

  std::string prefix;
  FindReplacementSpan(sql, cursor_byte_offset, &result.replacement_start,
                      &result.replacement_end, &prefix);

  ::googlesql::ParseTokenOptions token_options;
  token_options.language_options = language;
  ::googlesql::ParseResumeLocation resume =
      ::googlesql::ParseResumeLocation::FromStringView(prefix_sql);
  std::vector<::googlesql::ParseToken> tokens;
  const absl::Status token_status =
      ::googlesql::GetParseTokens(token_options, &resume, &tokens);
  if (!token_status.ok()) {
    return token_status;
  }

  const CompletionContext ctx = InferCompletionContext(tokens);

  switch (ctx.kind) {
    case CompletionContextKind::kTable:
      AppendPrefixMatches(prefix, "dataset", catalog_names.datasets,
                          &result.candidates);
      AppendPrefixMatches(prefix, "table", catalog_names.tables,
                          &result.candidates);
      break;
    case CompletionContextKind::kMember:
      AppendColumnCandidatesForTable(catalog, catalog->FullName(),
                                     default_dataset_id, ctx.qualifier, prefix,
                                     &result.candidates);
      AppendPrefixMatches(prefix, "column", catalog_names.columns,
                          &result.candidates);
      break;
    case CompletionContextKind::kColumn:
      AppendPrefixMatches(prefix, "column", catalog_names.columns,
                          &result.candidates);
      AppendFunctionCandidates(catalog, prefix, &result.candidates);
      AppendPrefixMatches(prefix, "keyword", SqlKeywords(), &result.candidates);
      break;
    case CompletionContextKind::kGeneral:
      AppendPrefixMatches(prefix, "keyword", SqlKeywords(), &result.candidates);
      AppendFunctionCandidates(catalog, prefix, &result.candidates);
      AppendPrefixMatches(prefix, "dataset", catalog_names.datasets,
                          &result.candidates);
      AppendPrefixMatches(prefix, "table", catalog_names.tables,
                          &result.candidates);
      break;
  }

  std::sort(result.candidates.begin(), result.candidates.end(),
            [](const CompletionCandidate& a, const CompletionCandidate& b) {
              return a.label < b.label;
            });
  return result;
}

}  // namespace sqltools
}  // namespace backend
}  // namespace bigquery_emulator
