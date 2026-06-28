#include "backend/sqltools/sql_references.h"

#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "absl/container/flat_hash_set.h"
#include "absl/status/status.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/str_split.h"
#include "backend/catalog/storage_table.h"
#include "backend/catalog/view_registry.h"
#include "backend/engine/coordinator/sql_preprocess.h"
#include "googlesql/public/analyzer.h"
#include "googlesql/public/analyzer_options.h"
#include "googlesql/public/analyzer_output.h"
#include "googlesql/public/parse_resume_location.h"
#include "googlesql/public/parse_tokens.h"
#include "googlesql/public/types/type_factory.h"
#include "googlesql/resolved_ast/resolved_ast.h"
#include "googlesql/resolved_ast/resolved_ast_visitor.h"

namespace bigquery_emulator {
namespace backend {
namespace sqltools {
namespace {

class ReferencedTableCollector : public ::googlesql::ResolvedASTVisitor {
 public:
  absl::Status VisitResolvedTableScan(
      const ::googlesql::ResolvedTableScan* node) override {
    if (node == nullptr) return absl::OkStatus();
    if (node->table() != nullptr) {
      entries_.push_back(Entry{node->table(), node->alias()});
    }
    return ::googlesql::ResolvedASTVisitor::VisitResolvedTableScan(node);
  }

  struct Entry {
    const ::googlesql::Table* table;
    std::string alias;
  };

  const std::vector<Entry>& entries() const {
    return entries_;
  }

 private:
  std::vector<Entry> entries_;
};

std::vector<CatalogColumnEntry> ColumnsForTable(
    const ::googlesql::Table* table) {
  std::vector<CatalogColumnEntry> columns;
  if (table == nullptr) return columns;
  for (int i = 0; i < table->NumColumns(); ++i) {
    const ::googlesql::Column* column = table->GetColumn(i);
    if (column == nullptr) continue;
    CatalogColumnEntry entry;
    entry.name = std::string(column->Name());
    if (column->GetType() != nullptr) {
      entry.type = std::string(
          column->GetType()->TypeName(::googlesql::PRODUCT_EXTERNAL));
    }
    columns.push_back(std::move(entry));
  }
  return columns;
}

bool ParseTableFullName(absl::string_view full_name,
                        std::string* project_id,
                        std::string* dataset_id,
                        std::string* table_id) {
  std::vector<std::string> parts =
      absl::StrSplit(full_name, '.', absl::SkipEmpty());
  if (parts.size() == 3) {
    *project_id = parts[0];
    *dataset_id = parts[1];
    *table_id = parts[2];
    return true;
  }
  if (parts.size() == 2) {
    project_id->clear();
    *dataset_id = parts[0];
    *table_id = parts[1];
    return true;
  }
  if (parts.size() == 1) {
    project_id->clear();
    dataset_id->clear();
    *table_id = parts[0];
    return true;
  }
  return false;
}

ReferencedTable TableToReference(const ::googlesql::Table* table,
                                 absl::string_view project_id,
                                 absl::string_view alias) {
  ReferencedTable ref;
  ref.alias = std::string(alias);
  ref.kind = "table";

  if (const catalog::StorageTable* storage_table =
          dynamic_cast<const catalog::StorageTable*>(table);
      storage_table != nullptr) {
    const storage::TableId& id = storage_table->storage_table_id();
    ref.project_id = id.project_id;
    ref.dataset_id = id.dataset_id;
    ref.table_id = id.table_id;
  } else if (table != nullptr) {
    ParseTableFullName(
        table->FullName(), &ref.project_id, &ref.dataset_id, &ref.table_id);
    if (ref.project_id.empty()) {
      ref.project_id = std::string(project_id);
    }
  }

  if (!ref.project_id.empty() && !ref.dataset_id.empty() &&
      !ref.table_id.empty() &&
      catalog::FindProjectView(ref.project_id, ref.dataset_id, ref.table_id) !=
          nullptr) {
    ref.kind = "view";
  }

  ref.columns = ColumnsForTable(table);
  return ref;
}

bool SameReference(const ReferencedTable& a, const ReferencedTable& b) {
  return a.project_id == b.project_id && a.dataset_id == b.dataset_id &&
         a.table_id == b.table_id && a.alias == b.alias;
}

bool IsTableIntroKeyword(absl::string_view keyword) {
  static const absl::flat_hash_set<std::string>* kKeywords =
      new absl::flat_hash_set<std::string>{
          "FROM", "JOIN", "INNER", "LEFT", "RIGHT", "FULL", "CROSS", "USING"};
  return kKeywords->contains(std::string(keyword));
}

bool IsJoinModifierKeyword(absl::string_view keyword) {
  static const absl::flat_hash_set<std::string>* kKeywords =
      new absl::flat_hash_set<std::string>{
          "INNER", "LEFT", "RIGHT", "FULL", "CROSS", "OUTER"};
  return kKeywords->contains(std::string(keyword));
}

bool IsPostTableKeyword(absl::string_view keyword) {
  static const absl::flat_hash_set<std::string>* kKeywords =
      new absl::flat_hash_set<std::string>{
          "ON",     "WHERE", "GROUP",     "ORDER",  "HAVING",  "LIMIT",
          "JOIN",   "LEFT",  "RIGHT",     "INNER",  "OUTER",   "CROSS",
          "FULL",   "UNION", "INTERSECT", "EXCEPT", "QUALIFY", "WINDOW",
          "SET",    "MERGE", "USING",     "WHEN",   "THEN",    "END",
          "SELECT", "FROM"};
  return kKeywords->contains(std::string(keyword));
}

bool ReadDottedName(const std::vector<::googlesql::ParseToken>& tokens,
                    size_t* index,
                    std::string* out) {
  if (*index >= tokens.size()) return false;
  if (!tokens[*index].IsIdentifier()) return false;
  *out = tokens[*index].GetIdentifier();
  ++(*index);
  while (*index + 1 < tokens.size() && tokens[*index].GetKeyword() == ".") {
    ++(*index);
    if (*index >= tokens.size() || !tokens[*index].IsIdentifier()) {
      return false;
    }
    absl::StrAppend(out, ".", tokens[*index].GetIdentifier());
    ++(*index);
  }
  return true;
}

std::vector<CatalogColumnEntry> LookupColumnsForTableRef(
    absl::string_view table_ref,
    absl::string_view default_dataset_id,
    const CatalogNames& names) {
  std::vector<std::string> keys;
  keys.push_back(std::string(table_ref));
  const std::vector<std::string> parts =
      absl::StrSplit(table_ref, '.', absl::SkipEmpty());
  if (parts.size() == 1 && !default_dataset_id.empty()) {
    keys.push_back(absl::StrCat(default_dataset_id, ".", table_ref));
  }
  for (const std::string& key : keys) {
    const auto it = names.columns_by_table.find(key);
    if (it != names.columns_by_table.end()) {
      return it->second;
    }
  }
  for (const CatalogTableEntry& table : names.tables) {
    if (!absl::EqualsIgnoreCase(table.label, table_ref) &&
        !absl::EqualsIgnoreCase(table.fqn, table_ref)) {
      continue;
    }
    const auto by_label = names.columns_by_table.find(table.label);
    if (by_label != names.columns_by_table.end()) {
      return by_label->second;
    }
    const auto by_fqn = names.columns_by_table.find(table.fqn);
    if (by_fqn != names.columns_by_table.end()) {
      return by_fqn->second;
    }
  }
  return {};
}

bool SameInScopeTable(const InScopeTableRef& a, const InScopeTableRef& b) {
  return a.table_key == b.table_key && a.alias == b.alias;
}

void AppendInScopeTable(CatalogNames* names, InScopeTableRef scope) {
  for (const InScopeTableRef& existing : names->in_scope_tables) {
    if (SameInScopeTable(existing, scope)) {
      return;
    }
  }
  names->in_scope_tables.push_back(std::move(scope));
}

}  // namespace

absl::StatusOr<AnalyzeResult> AnalyzeSqlText(
    absl::string_view sql,
    absl::string_view project_id,
    absl::string_view default_dataset_id,
    ::googlesql::Catalog* catalog,
    const ::googlesql::LanguageOptions& language) {
  AnalyzeResult result;
  if (catalog == nullptr) {
    return absl::InvalidArgumentError("AnalyzeSqlText: catalog is required");
  }
  if (sql.empty()) {
    return result;
  }

  ::googlesql::AnalyzerOptions options(language);
  options.set_error_message_mode(::googlesql::ERROR_MESSAGE_ONE_LINE);
  options.set_attach_error_location_payload(true);
  options.CreateDefaultArenasIfNotSet();

  const std::string preprocessed_sql =
      engine::coordinator::PreprocessSqlForAnalyzer(std::string(sql));
  ::googlesql::TypeFactory type_factory;
  std::unique_ptr<const ::googlesql::AnalyzerOutput> output;
  const absl::Status analyze_status = ::googlesql::AnalyzeStatement(
      preprocessed_sql, options, catalog, &type_factory, &output);
  if (!analyze_status.ok()) {
    result.diagnostics.push_back(
        DiagnosticFromStatusWithSql(analyze_status, sql));
    return result;
  }
  if (output == nullptr || output->resolved_statement() == nullptr) {
    return result;
  }

  const ::googlesql::ResolvedStatement* statement =
      output->resolved_statement();
  result.statement_kinds.push_back(
      ::googlesql::ResolvedNodeKindToString(statement->node_kind()));

  ReferencedTableCollector collector;
  const absl::Status visit_status = statement->Accept(&collector);
  if (!visit_status.ok()) {
    return visit_status;
  }

  for (const ReferencedTableCollector::Entry& entry : collector.entries()) {
    ReferencedTable ref =
        TableToReference(entry.table, project_id, entry.alias);
    bool duplicate = false;
    for (const ReferencedTable& existing : result.referenced_tables) {
      if (SameReference(existing, ref)) {
        duplicate = true;
        break;
      }
    }
    if (!duplicate) {
      result.referenced_tables.push_back(std::move(ref));
    }
  }
  return result;
}

void PopulateInScopeTablesFromAnalyze(const AnalyzeResult& analyze,
                                      CatalogNames* names) {
  if (names == nullptr) return;
  names->in_scope_tables.clear();
  for (const ReferencedTable& ref : analyze.referenced_tables) {
    InScopeTableRef scope;
    scope.alias = ref.alias;
    if (!ref.dataset_id.empty() && !ref.table_id.empty()) {
      scope.table_key = absl::StrCat(ref.dataset_id, ".", ref.table_id);
    } else {
      scope.table_key = ref.table_id;
    }
    scope.columns = ref.columns;
    AppendInScopeTable(names, std::move(scope));
  }
}

void PopulateInScopeTablesFromHeuristic(
    absl::string_view sql,
    const ::googlesql::LanguageOptions& language,
    absl::string_view default_dataset_id,
    CatalogNames* names) {
  if (names == nullptr || sql.empty()) return;

  ::googlesql::ParseTokenOptions token_options;
  token_options.language_options = language;
  ::googlesql::ParseResumeLocation resume =
      ::googlesql::ParseResumeLocation::FromStringView(sql);
  std::vector<::googlesql::ParseToken> tokens;
  if (!::googlesql::GetParseTokens(token_options, &resume, &tokens).ok()) {
    return;
  }

  for (size_t i = 0; i < tokens.size(); ++i) {
    if (!tokens[i].IsKeyword()) continue;
    if (!IsTableIntroKeyword(tokens[i].GetKeyword())) continue;

    size_t j = i + 1;
    while (j < tokens.size() && tokens[j].IsKeyword() &&
           IsJoinModifierKeyword(tokens[j].GetKeyword())) {
      ++j;
    }
    if (j >= tokens.size()) continue;

    std::string table_ref;
    if (!ReadDottedName(tokens, &j, &table_ref)) continue;

    std::string alias;
    if (j < tokens.size() && tokens[j].IsKeyword() &&
        tokens[j].GetKeyword() == "AS") {
      ++j;
      if (j < tokens.size() && tokens[j].IsIdentifier()) {
        alias = tokens[j].GetIdentifier();
        ++j;
      }
    } else if (j < tokens.size() && tokens[j].IsIdentifier()) {
      alias = tokens[j].GetIdentifier();
      ++j;
    } else if (j < tokens.size() && tokens[j].IsKeyword() &&
               !IsPostTableKeyword(tokens[j].GetKeyword())) {
      alias = tokens[j].GetKeyword();
      ++j;
    }

    InScopeTableRef scope;
    scope.table_key = table_ref;
    scope.alias = alias;
    scope.columns =
        LookupColumnsForTableRef(table_ref, default_dataset_id, *names);
    AppendInScopeTable(names, std::move(scope));
  }
}

}  // namespace sqltools
}  // namespace backend
}  // namespace bigquery_emulator
