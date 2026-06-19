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
#include "googlesql/public/catalog.h"
#include "googlesql/public/function.h"
#include "googlesql/public/parse_resume_location.h"
#include "googlesql/public/parse_tokens.h"
#include "googlesql/public/simple_catalog.h"

namespace bigquery_emulator {
namespace backend {
namespace sqltools {
namespace {
bool IsTableContextKeyword(absl::string_view keyword) {
  static const absl::flat_hash_set<std::string>* kKeywords =
      new absl::flat_hash_set<std::string>{"FROM",
                                           "JOIN",
                                           "INNER",
                                           "LEFT",
                                           "RIGHT",
                                           "FULL",
                                           "CROSS",
                                           "INTO",
                                           "UPDATE",
                                           "TABLE",
                                           "MERGE",
                                           "USING",
                                           "DELETE",
                                           "TRUNCATE"};
  return kKeywords->contains(std::string(keyword));
}

bool IsColumnContextKeyword(absl::string_view keyword) {
  static const absl::flat_hash_set<std::string>* kKeywords =
      new absl::flat_hash_set<std::string>{"SELECT",
                                           "WHERE",
                                           "ON",
                                           "BY",
                                           "HAVING",
                                           "QUALIFY",
                                           "SET",
                                           "AND",
                                           "OR",
                                           "NOT",
                                           "WHEN",
                                           "THEN",
                                           "ELSE",
                                           "CASE",
                                           "GROUP",
                                           "ORDER",
                                           "PARTITION"};
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

bool NeedsBackticks(absl::string_view identifier) {
  if (identifier.empty()) return true;
  for (char c : identifier) {
    if (!std::isalnum(static_cast<unsigned char>(c)) && c != '_') {
      return true;
    }
  }
  static const absl::flat_hash_set<std::string>* kReserved =
      new absl::flat_hash_set<std::string>{
          "SELECT", "FROM", "WHERE", "TABLE", "VIEW", "JOIN", "GROUP", "ORDER"};
  return kReserved->contains(std::string(absl::AsciiStrToUpper(identifier)));
}

std::string QuoteIdentifierIfNeeded(absl::string_view identifier) {
  if (!NeedsBackticks(identifier)) {
    return std::string(identifier);
  }
  return absl::StrCat("`", identifier, "`");
}

void AppendPrefixMatches(absl::string_view prefix,
                         absl::string_view kind,
                         const std::vector<std::string>& names,
                         std::vector<CompletionCandidate>* out) {
  for (const std::string& name : names) {
    if (prefix.empty() || absl::StartsWithIgnoreCase(name, prefix)) {
      AppendUniqueCandidate(
          out,
          CompletionCandidate{
              name, std::string(kind), QuoteIdentifierIfNeeded(name), ""});
    }
  }
}

void AppendTableCandidates(absl::string_view prefix,
                           const std::vector<CatalogTableEntry>& tables,
                           std::vector<CompletionCandidate>* out) {
  for (const CatalogTableEntry& table : tables) {
    if (!prefix.empty() && !absl::StartsWithIgnoreCase(table.label, prefix) &&
        !absl::StartsWithIgnoreCase(table.fqn, prefix)) {
      continue;
    }
    AppendUniqueCandidate(
        out,
        CompletionCandidate{table.label,
                            table.kind,
                            QuoteIdentifierIfNeeded(table.label),
                            table.detail});
  }
}

void AppendRoutineCandidates(absl::string_view prefix,
                             const std::vector<CatalogRoutineEntry>& routines,
                             std::vector<CompletionCandidate>* out) {
  for (const CatalogRoutineEntry& routine : routines) {
    if (!prefix.empty() && !absl::StartsWithIgnoreCase(routine.label, prefix) &&
        !absl::StartsWithIgnoreCase(routine.fqn, prefix)) {
      continue;
    }
    AppendUniqueCandidate(
        out,
        CompletionCandidate{routine.label,
                            routine.kind,
                            QuoteIdentifierIfNeeded(routine.label),
                            routine.detail});
  }
}

void AppendColumnCandidates(absl::string_view prefix,
                            const std::vector<CatalogColumnEntry>& columns,
                            std::vector<CompletionCandidate>* out) {
  for (const CatalogColumnEntry& column : columns) {
    if (prefix.empty() || absl::StartsWithIgnoreCase(column.name, prefix)) {
      AppendUniqueCandidate(
          out,
          CompletionCandidate{column.name,
                              "column",
                              QuoteIdentifierIfNeeded(column.name),
                              column.type});
    }
  }
}

void AppendInScopeColumnCandidates(
    absl::string_view prefix,
    absl::string_view qualifier,
    const std::vector<InScopeTableRef>& in_scope_tables,
    std::vector<CompletionCandidate>* out) {
  for (const InScopeTableRef& table : in_scope_tables) {
    if (!qualifier.empty() && !table.alias.empty() &&
        !absl::EqualsIgnoreCase(table.alias, qualifier)) {
      continue;
    }
    AppendColumnCandidates(prefix, table.columns, out);
  }
}

void AppendFlatColumnUnion(const CatalogNames& catalog_names,
                           absl::string_view prefix,
                           std::vector<CompletionCandidate>* out) {
  AppendColumnCandidates(prefix, catalog_names.columns, out);
  for (const auto& [table_key, columns] : catalog_names.columns_by_table) {
    (void)table_key;
    AppendColumnCandidates(prefix, columns, out);
  }
  for (const InScopeTableRef& table : catalog_names.in_scope_tables) {
    AppendColumnCandidates(prefix, table.columns, out);
  }
}

const std::vector<std::string>& SqlKeywords() {
  static const std::vector<std::string>* kKeywords =
      new std::vector<std::string>{
          "ALL",      "AND",       "ANY",     "ARRAY",   "AS",      "ASC",
          "BETWEEN",  "BY",        "CASE",    "CAST",    "COLLATE", "CREATE",
          "CROSS",    "CUBE",      "CURRENT", "DEFAULT", "DELETE",  "DESC",
          "DISTINCT", "DROP",      "ELSE",    "END",     "EXCEPT",  "EXISTS",
          "FALSE",    "FOR",       "FROM",    "FULL",    "GROUP",   "GROUPING",
          "HAVING",   "IF",        "IN",      "INNER",   "INSERT",  "INTERSECT",
          "INTO",     "IS",        "JOIN",    "LEFT",    "LIKE",    "LIMIT",
          "MERGE",    "NOT",       "NULL",    "NULLS",   "OF",      "OFFSET",
          "ON",       "OR",        "ORDER",   "OUTER",   "OVER",    "PARTITION",
          "PIVOT",    "QUALIFY",   "REGEXP",  "RIGHT",   "ROLLUP",  "ROW",
          "ROWS",     "SAFE_CAST", "SELECT",  "SET",     "SOME",    "STRUCT",
          "TABLE",    "THEN",      "TO",      "TRUE",    "UNION",   "UNNEST",
          "UPDATE",   "USING",     "WHEN",    "WHERE",   "WINDOW",  "WITH",
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
          out, CompletionCandidate{name, "function", name + "(", ""});
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
        const std::string type =
            table->GetColumn(i)->GetType() != nullptr
                ? std::string(table->GetColumn(i)->GetType()->TypeName(
                      ::googlesql::PRODUCT_EXTERNAL))
                : std::string();
        AppendUniqueCandidate(
            out,
            CompletionCandidate{
                col, "column", QuoteIdentifierIfNeeded(col), type});
      }
    }
  }
}

enum class CompletionContextKind {
  kGeneral,
  kTable,
  kColumn,
  kMember,
  kRoutine,
};

struct CompletionContext {
  CompletionContextKind kind = CompletionContextKind::kGeneral;
  std::string qualifier;
};

bool IsRoutineContextKeyword(absl::string_view keyword) {
  static const absl::flat_hash_set<std::string>* kKeywords =
      new absl::flat_hash_set<std::string>{"FUNCTION", "PROCEDURE", "ROUTINE"};
  return kKeywords->contains(std::string(keyword));
}

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
    if (IsRoutineContextKeyword(kw)) {
      ctx.kind = CompletionContextKind::kRoutine;
      return ctx;
    }
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

void FindReplacementSpan(absl::string_view sql,
                         size_t cursor,
                         int* replacement_start,
                         int* replacement_end,
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

}  // namespace

absl::StatusOr<CompleteResult> CompleteSqlText(
    absl::string_view sql,
    size_t cursor_byte_offset,
    const ::googlesql::LanguageOptions& language,
    ::googlesql::Catalog* catalog,
    const CatalogNames& catalog_names,
    absl::string_view default_dataset_id) {
  CompleteResult result;
  if (catalog == nullptr) {
    return absl::InvalidArgumentError("CompleteSqlText: catalog is required");
  }
  if (cursor_byte_offset > sql.size()) {
    cursor_byte_offset = sql.size();
  }
  const absl::string_view prefix_sql = sql.substr(0, cursor_byte_offset);

  std::string prefix;
  FindReplacementSpan(sql,
                      cursor_byte_offset,
                      &result.replacement_start,
                      &result.replacement_end,
                      &prefix);

  if (prefix_sql.empty()) {
    AppendPrefixMatches(prefix, "keyword", SqlKeywords(), &result.candidates);
    AppendFunctionCandidates(catalog, prefix, &result.candidates);
    AppendPrefixMatches(
        prefix, "dataset", catalog_names.datasets, &result.candidates);
    AppendTableCandidates(prefix, catalog_names.tables, &result.candidates);
    AppendRoutineCandidates(prefix, catalog_names.routines, &result.candidates);
    std::sort(result.candidates.begin(),
              result.candidates.end(),
              [](const CompletionCandidate& a, const CompletionCandidate& b) {
                return a.label < b.label;
              });
    return result;
  }

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
      AppendPrefixMatches(
          prefix, "dataset", catalog_names.datasets, &result.candidates);
      AppendTableCandidates(prefix, catalog_names.tables, &result.candidates);
      break;
    case CompletionContextKind::kMember:
      AppendColumnCandidatesForTable(catalog,
                                     catalog->FullName(),
                                     default_dataset_id,
                                     ctx.qualifier,
                                     prefix,
                                     &result.candidates);
      AppendInScopeColumnCandidates(prefix,
                                    ctx.qualifier,
                                    catalog_names.in_scope_tables,
                                    &result.candidates);
      AppendFlatColumnUnion(catalog_names, prefix, &result.candidates);
      break;
    case CompletionContextKind::kColumn:
      AppendInScopeColumnCandidates(
          prefix, "", catalog_names.in_scope_tables, &result.candidates);
      AppendFlatColumnUnion(catalog_names, prefix, &result.candidates);
      AppendFunctionCandidates(catalog, prefix, &result.candidates);
      AppendPrefixMatches(prefix, "keyword", SqlKeywords(), &result.candidates);
      break;
    case CompletionContextKind::kRoutine:
      AppendRoutineCandidates(
          prefix, catalog_names.routines, &result.candidates);
      AppendPrefixMatches(prefix, "keyword", SqlKeywords(), &result.candidates);
      break;
    case CompletionContextKind::kGeneral:
      AppendPrefixMatches(prefix, "keyword", SqlKeywords(), &result.candidates);
      AppendFunctionCandidates(catalog, prefix, &result.candidates);
      AppendPrefixMatches(
          prefix, "dataset", catalog_names.datasets, &result.candidates);
      AppendTableCandidates(prefix, catalog_names.tables, &result.candidates);
      AppendRoutineCandidates(
          prefix, catalog_names.routines, &result.candidates);
      break;
  }

  std::sort(result.candidates.begin(),
            result.candidates.end(),
            [](const CompletionCandidate& a, const CompletionCandidate& b) {
              return a.label < b.label;
            });
  return result;
}


}  // namespace sqltools
}  // namespace backend
}  // namespace bigquery_emulator
