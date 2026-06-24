#include "backend/sqltools/sql_tools.h"

#include "backend/sqltools/sql_references.h"
#include "googlesql/public/builtin_function_options.h"
#include "googlesql/public/function.h"
#include "googlesql/public/function_signature.h"
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
    catalog_ = std::make_unique<::googlesql::SimpleCatalog>("test-project",
                                                            &type_factory_);
    catalog_->AddBuiltinFunctionsAndTypes(
        ::googlesql::BuiltinFunctionOptions(language_));
  }

  void AddScalarFunction(const std::string& name) {
    ::googlesql::FunctionSignature signature(
        ::googlesql::FunctionArgumentType(::googlesql::types::Int64Type()),
        /*arguments=*/{},
        /*context_id=*/static_cast<int64_t>(0));
    auto function = std::make_unique<::googlesql::Function>(
        std::vector<std::string>{name},
        /*group=*/"External_function",
        ::googlesql::Function::SCALAR,
        std::vector<::googlesql::FunctionSignature>{signature});
    catalog_->AddFunction(function.get());
    owned_functions_.push_back(std::move(function));
  }

  ::googlesql::LanguageOptions language_;
  ::googlesql::TypeFactory type_factory_;
  std::unique_ptr<::googlesql::SimpleCatalog> catalog_;
  std::vector<std::unique_ptr<const ::googlesql::Function>> owned_functions_;
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

TEST_F(SqlToolsTest, CompleteProjectQualifiedTableCandidate) {
  CatalogNames names;
  names.tables.push_back(
      CatalogTableEntry{"proj.ds.events", "proj.ds.events", "table", "table"});
  const std::string sql = "SELECT * FROM proj.d";
  const absl::StatusOr<CompleteResult> result =
      CompleteSqlText(sql, sql.size(), language_, catalog_.get(), names, "ds");
  ASSERT_TRUE(result.ok()) << result.status();
  bool found_fqn = false;
  for (const CompletionCandidate& candidate : result->candidates) {
    if (candidate.label == "proj.ds.events") {
      found_fqn = true;
      break;
    }
  }
  EXPECT_TRUE(found_fqn);
}

TEST_F(SqlToolsTest, CompleteIncompleteSqlUsesHeuristicColumns) {
  CatalogNames names;
  names.columns_by_table["analytics.events"] = {
      CatalogColumnEntry{"id", "INT64"},
      CatalogColumnEntry{"name", "STRING"},
  };
  PopulateInScopeTablesFromHeuristic(
      "SELECT na FROM analytics.events WHERE ", language_, "analytics", &names);
  ASSERT_EQ(names.in_scope_tables.size(), 1u);
  ASSERT_EQ(names.in_scope_tables[0].columns.size(), 2u);

  const std::string sql = "SELECT na FROM analytics.events WHERE ";
  const absl::StatusOr<CompleteResult> result = CompleteSqlText(
      sql, sql.size(), language_, catalog_.get(), names, "analytics");
  ASSERT_TRUE(result.ok()) << result.status();
  bool found_name = false;
  for (const CompletionCandidate& candidate : result->candidates) {
    if (candidate.label == "name") {
      found_name = true;
      EXPECT_EQ(candidate.kind, "column");
      break;
    }
  }
  EXPECT_TRUE(found_name);
}

TEST_F(SqlToolsTest, CompleteUserRoutineNotDuplicatedAsFunction) {
  AddScalarFunction("add_one");
  CatalogNames names;
  names.routines.push_back(CatalogRoutineEntry{"ds.add_one",
                                               "test-project.ds.add_one",
                                               "routine",
                                               "SQL scalar function"});
  names.routines.push_back(CatalogRoutineEntry{
      "add_one", "test-project.ds.add_one", "routine", "SQL scalar function"});

  const std::string sql = "SELECT add_";
  const absl::StatusOr<CompleteResult> result =
      CompleteSqlText(sql, sql.size(), language_, catalog_.get(), names, "ds");
  ASSERT_TRUE(result.ok()) << result.status();

  bool found_routine = false;
  bool found_function = false;
  for (const CompletionCandidate& candidate : result->candidates) {
    if (candidate.label == "add_one" && candidate.kind == "routine") {
      found_routine = true;
      EXPECT_EQ(candidate.fqn, "test-project.ds.add_one");
      EXPECT_EQ(candidate.insert_text, "add_one(");
    }
    if (candidate.label == "add_one" && candidate.kind == "function") {
      found_function = true;
    }
  }
  EXPECT_TRUE(found_routine);
  EXPECT_FALSE(found_function);
}

TEST_F(SqlToolsTest, CompleteRoutineCandidateIncludesFqn) {
  CatalogNames names;
  names.routines.push_back(CatalogRoutineEntry{
      "proj.ds.my_fn",
      "proj.ds.my_fn",
      "routine",
      "SQL scalar function",
  });
  const std::string sql = "CREATE FUNCTION ";
  const absl::StatusOr<CompleteResult> result =
      CompleteSqlText(sql, sql.size(), language_, catalog_.get(), names, "ds");
  ASSERT_TRUE(result.ok()) << result.status();
  bool found = false;
  for (const CompletionCandidate& candidate : result->candidates) {
    if (candidate.label == "proj.ds.my_fn") {
      found = true;
      EXPECT_EQ(candidate.kind, "routine");
      EXPECT_EQ(candidate.fqn, "proj.ds.my_fn");
      break;
    }
  }
  EXPECT_TRUE(found);
}

}  // namespace
}  // namespace sqltools
}  // namespace backend
}  // namespace bigquery_emulator
