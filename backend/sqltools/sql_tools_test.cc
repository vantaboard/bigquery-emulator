#include "backend/sqltools/sql_tools.h"

#include "googlesql/public/builtin_function_options.h"
#include "googlesql/public/simple_catalog.h"
#include "googlesql/public/types/type_factory.h"
#include "gtest/gtest.h"

namespace bigquery_emulator {
namespace backend {
namespace sqltools {
namespace {

class SqlToolsTest : public ::testing::Test {
 protected:
  void SetUp() override {
    language_ = MakeSqlToolsLanguageOptions();
    catalog_ = std::make_unique<::googlesql::SimpleCatalog>("test-project");
    catalog_->AddBuiltinFunctionsAndTypes(
        ::googlesql::BuiltinFunctionOptions(language_));
  }

  ::googlesql::LanguageOptions language_;
  ::googlesql::TypeFactory type_factory_;
  std::unique_ptr<::googlesql::SimpleCatalog> catalog_;
};

TEST_F(SqlToolsTest, FormatLenientProducesIndentedSql) {
  const absl::StatusOr<FormatResult> result =
      FormatSqlText("select 1", FormatOptions{});
  ASSERT_TRUE(result.ok()) << result.status();
  EXPECT_NE(result->formatted_sql.find("SELECT"), std::string::npos);
  EXPECT_NE(result->formatted_sql.find("1"), std::string::npos);
}

TEST_F(SqlToolsTest, ParseValidSelectReturnsStatementKind) {
  const absl::StatusOr<ParseResult> result =
      ParseSqlText("SELECT 1", language_);
  ASSERT_TRUE(result.ok()) << result.status();
  EXPECT_TRUE(result->diagnostics.empty());
  ASSERT_EQ(result->statement_kinds.size(), 1u);
  EXPECT_EQ(result->statement_kinds[0], "QueryStatement");
}

TEST_F(SqlToolsTest, ParseInvalidSqlReturnsDiagnostic) {
  const absl::StatusOr<ParseResult> result = ParseSqlText("SELEC 1", language_);
  ASSERT_TRUE(result.ok()) << result.status();
  EXPECT_FALSE(result->diagnostics.empty());
}

TEST_F(SqlToolsTest, TokenizeSelectReturnsKeywords) {
  TokenizeOptions options;
  const absl::StatusOr<TokenizeResult> result =
      TokenizeSqlText("SELECT 1", language_, options);
  ASSERT_TRUE(result.ok()) << result.status();
  ASSERT_GE(result->tokens.size(), 2u);
  EXPECT_EQ(result->tokens[0].kind, "keyword");
  EXPECT_EQ(result->tokens[0].image, "SELECT");
}

TEST_F(SqlToolsTest, CompleteAfterSelectIncludesKeywords) {
  CatalogNames names;
  const std::string sql = "SELECT ";
  const absl::StatusOr<CompleteResult> result =
      CompleteSqlText(sql, sql.size(), language_, catalog_.get(), names, "");
  ASSERT_TRUE(result.ok()) << result.status();
  bool found_from = false;
  for (const CompletionCandidate& candidate : result->candidates) {
    if (candidate.label == "FROM") {
      found_from = true;
      break;
    }
  }
  EXPECT_TRUE(found_from);
}

TEST_F(SqlToolsTest, ParseInvalidSqlReturnsDiagnosticWithSpan) {
  const absl::StatusOr<ParseResult> result = ParseSqlText("SELEC 1", language_);
  ASSERT_TRUE(result.ok()) << result.status();
  ASSERT_FALSE(result->diagnostics.empty());
  const SqlDiagnostic& diag = result->diagnostics.front();
  EXPECT_GE(diag.start_byte, 0);
  EXPECT_GT(diag.end_byte, diag.start_byte);
}

TEST_F(SqlToolsTest, CompleteEmptyEditorAtCursorZero) {
  CatalogNames names;
  names.datasets = {"analytics"};
  const absl::StatusOr<CompleteResult> result =
      CompleteSqlText("", 0, language_, catalog_.get(), names, "analytics");
  ASSERT_TRUE(result.ok()) << result.status();
  bool found_select = false;
  for (const CompletionCandidate& candidate : result->candidates) {
    if (candidate.label == "SELECT") {
      found_select = true;
      break;
    }
  }
  EXPECT_TRUE(found_select);
}

TEST_F(SqlToolsTest, CompleteAfterFromUsesCatalogTables) {
  CatalogNames names;
  names.tables.push_back(
      CatalogTableEntry{"analytics.events", "p.analytics.events", "table", ""});
  names.tables.push_back(
      CatalogTableEntry{"events", "p.analytics.events", "table", ""});
  const std::string sql = "SELECT * FROM ev";
  const absl::StatusOr<CompleteResult> result = CompleteSqlText(
      sql, sql.size(), language_, catalog_.get(), names, "analytics");
  ASSERT_TRUE(result.ok()) << result.status();
  bool found_events = false;
  for (const CompletionCandidate& candidate : result->candidates) {
    if (candidate.label == "events") {
      found_events = true;
      break;
    }
  }
  EXPECT_TRUE(found_events);
}

}  // namespace
}  // namespace sqltools
}  // namespace backend
}  // namespace bigquery_emulator
