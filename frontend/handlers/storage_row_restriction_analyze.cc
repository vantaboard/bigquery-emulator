

#include "googlesql/public/analyzer.h"
#include "googlesql/public/analyzer_options.h"
#include "googlesql/public/analyzer_output.h"
#include "googlesql/public/language_options.h"
#include "googlesql/public/types/type_factory.h"

namespace bigquery_emulator {
namespace frontend {
namespace {

absl::string_view Trim(absl::string_view s) {
  while (!s.empty() && absl::ascii_isspace(s.front()))
    s.remove_prefix(1);
  while (!s.empty() && absl::ascii_isspace(s.back()))
    s.remove_suffix(1);
  return s;
}

const ::googlesql::ResolvedExpr* FindFilterExpr(
    const ::googlesql::ResolvedScan* scan) {
  while (scan != nullptr) {
    switch (scan->node_kind()) {
      case ::googlesql::RESOLVED_FILTER_SCAN:
        return scan->GetAs<::googlesql::ResolvedFilterScan>()->filter_expr();
      case ::googlesql::RESOLVED_PROJECT_SCAN:
        scan = scan->GetAs<::googlesql::ResolvedProjectScan>()->input_scan();
        break;
      case ::googlesql::RESOLVED_ORDER_BY_SCAN:
        scan = scan->GetAs<::googlesql::ResolvedOrderByScan>()->input_scan();
        break;
      case ::googlesql::RESOLVED_LIMIT_OFFSET_SCAN:
        scan =
            scan->GetAs<::googlesql::ResolvedLimitOffsetScan>()->input_scan();
        break;
      default:
        return nullptr;
    }
  }
  return nullptr;
}

::googlesql::AnalyzerOptions MakeRowRestrictionAnalyzerOptions() {
  ::googlesql::LanguageOptions language;
  language.EnableMaximumLanguageFeatures();
  language.EnableLanguageFeature(::googlesql::FEATURE_WITH_EXPRESSION);
  language.set_product_mode(::googlesql::PRODUCT_EXTERNAL);
  language.set_name_resolution_mode(::googlesql::NAME_RESOLUTION_DEFAULT);
  ::googlesql::AnalyzerOptions options(language);
  options.set_error_message_mode(::googlesql::ERROR_MESSAGE_ONE_LINE);
  return options;
}

}  // namespace

absl::Status TranspileRowRestriction(absl::string_view restriction,
                                     const backend::storage::TableId& table,
                                     backend::storage::Storage* storage,
                                     std::string* where_sql) {
  if (where_sql == nullptr) {
    return absl::InternalError(
        "row_restriction: where_sql out parameter must be non-null");
  }
  const absl::string_view trimmed = Trim(restriction);
  if (trimmed.empty()) {
    return absl::OkStatus();
  }
  if (storage == nullptr) {
    return absl::FailedPreconditionError(
        "row_restriction: storage backend must be non-null");
  }

  auto type_factory = std::make_unique<::googlesql::TypeFactory>();
  ::googlesql::LanguageOptions language;
  language.EnableMaximumLanguageFeatures();
  language.set_product_mode(::googlesql::PRODUCT_EXTERNAL);
  backend::catalog::GoogleSqlCatalog catalog(table.project_id,
                                             storage,
                                             type_factory.get(),
                                             language,
                                             table.dataset_id);

  const std::string sql = absl::StrCat("SELECT * FROM `",
                                       table.project_id,
                                       ".",
                                       table.dataset_id,
                                       ".",
                                       table.table_id,
                                       "` WHERE (",
                                       trimmed,
                                       ")");

  ::googlesql::AnalyzerOptions options = MakeRowRestrictionAnalyzerOptions();
  std::unique_ptr<const ::googlesql::AnalyzerOutput> output;
  absl::Status analyzed = ::googlesql::AnalyzeStatement(
      sql, options, &catalog, type_factory.get(), &output);
  if (!analyzed.ok()) {
    return absl::InvalidArgumentError(
        absl::StrCat("row_restriction: ", analyzed.message()));
  }
  if (output == nullptr || output->resolved_statement() == nullptr) {
    return absl::InternalError(
        "row_restriction: analyzer returned no resolved statement");
  }
  const ::googlesql::ResolvedStatement* stmt = output->resolved_statement();
  if (stmt->node_kind() != ::googlesql::RESOLVED_QUERY_STMT) {
    return absl::InvalidArgumentError(absl::StrCat(
        "row_restriction: expected a boolean filter expression; got ",
        stmt->node_kind_string()));
  }
  const auto* query = stmt->GetAs<::googlesql::ResolvedQueryStmt>();
  const ::googlesql::ResolvedExpr* filter = FindFilterExpr(query->query());
  if (filter == nullptr) {
    return absl::InvalidArgumentError(
        "row_restriction: analyzer did not produce a filter expression");
  }

  backend::engine::duckdb::transpiler::Transpiler transpiler;
  const std::string emitted = transpiler.Transpile(filter);
  if (emitted.empty()) {
    return absl::InvalidArgumentError(absl::StrCat(
        "row_restriction: expression is not supported by the DuckDB "
        "transpiler in this emulator profile: ",
        restriction));
  }
  *where_sql = emitted;
  return absl::OkStatus();
}

}  // namespace frontend
}  // namespace bigquery_emulator
