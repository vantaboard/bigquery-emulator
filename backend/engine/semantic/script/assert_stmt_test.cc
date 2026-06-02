// Unit tests for `script::ExecuteAssert`.
//
// We drive a real `AnalyzeStatement` against a tiny `SimpleCatalog`
// (mirroring `executor_test.cc`) and run `ExecuteAssert` over the
// analyzer's resolved `ResolvedAssertStmt`. Tests pin every branch
// of the BigQuery-documented `ASSERT` contract:
//
//   * `ASSERT TRUE` -- OK.
//   * `ASSERT FALSE` -- INVALID_ARGUMENT carrying the
//     `kInvalidArgument` semantic-error reason and the default
//     "Assertion failed" message.
//   * `ASSERT FALSE AS '<msg>'` -- same code, message includes
//     `<msg>`.
//   * `ASSERT NULL` -- INVALID_ARGUMENT (NULL is not TRUE).
//   * Expression evaluation failures (e.g. `ASSERT 1.0 / 0 > 0`)
//     propagate the underlying semantic error reason (here:
//     `kDivisionByZero`).

#include "backend/engine/semantic/script/assert_stmt.h"

#include <memory>
#include <string>

#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "backend/engine/engine.h"
#include "backend/engine/semantic/error.h"
#include "backend/engine/semantic/script/script_driver.h"
#include "googlesql/public/analyzer.h"
#include "googlesql/public/analyzer_options.h"
#include "googlesql/public/analyzer_output.h"
#include "googlesql/public/builtin_function_options.h"
#include "googlesql/public/catalog.h"
#include "googlesql/public/language_options.h"
#include "googlesql/public/options.pb.h"
#include "googlesql/public/simple_catalog.h"
#include "googlesql/public/types/type_factory.h"
#include "googlesql/resolved_ast/resolved_ast.h"
#include "gtest/gtest.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace semantic {
namespace script {
namespace {

// Build an analyzer options bundle that supports every statement
// kind (ASSERT is not in the default supported set) and enables
// the maximum language feature surface so the resolver accepts the
// `AS '<msg>'` form.
::googlesql::AnalyzerOptions MakeAssertAnalyzerOptions() {
  ::googlesql::LanguageOptions language;
  language.EnableMaximumLanguageFeatures();
  language.set_product_mode(::googlesql::PRODUCT_EXTERNAL);
  language.SetSupportsAllStatementKinds();
  ::googlesql::AnalyzerOptions options(language);
  options.CreateDefaultArenasIfNotSet();
  return options;
}

class AssertStmtTest : public ::testing::Test {
 protected:
  void SetUp() override {
    type_factory_ = std::make_unique<::googlesql::TypeFactory>();
    catalog_ = std::make_unique<::googlesql::SimpleCatalog>(
        "assert_catalog", type_factory_.get());
    catalog_->AddBuiltinFunctions(
        ::googlesql::BuiltinFunctionOptions::AllReleasedFunctions());
  }

  const ::googlesql::ResolvedAssertStmt* Analyze(absl::string_view sql) {
    last_output_.reset();
    auto options = MakeAssertAnalyzerOptions();
    absl::Status s = ::googlesql::AnalyzeStatement(
        sql, options, catalog_.get(), type_factory_.get(), &last_output_);
    EXPECT_TRUE(s.ok()) << s;
    if (!s.ok() || last_output_ == nullptr) return nullptr;
    const ::googlesql::ResolvedStatement* stmt =
        last_output_->resolved_statement();
    EXPECT_NE(stmt, nullptr);
    if (stmt == nullptr) return nullptr;
    EXPECT_EQ(stmt->node_kind(), ::googlesql::RESOLVED_ASSERT_STMT);
    return stmt->GetAs<::googlesql::ResolvedAssertStmt>();
  }

  QueryRequest MakeRequest(absl::string_view sql) {
    QueryRequest req;
    req.project_id = "test-project";
    req.sql = std::string(sql);
    return req;
  }

  std::unique_ptr<::googlesql::TypeFactory> type_factory_{};
  std::unique_ptr<::googlesql::SimpleCatalog> catalog_{};
  std::unique_ptr<const ::googlesql::AnalyzerOutput> last_output_{};
};

TEST_F(AssertStmtTest, AssertTrueIsNoop) {
  const auto* stmt = Analyze("ASSERT TRUE");
  ASSERT_NE(stmt, nullptr);
  ScriptDriver driver;
  absl::Status status =
      ExecuteAssert(MakeRequest("ASSERT TRUE"), *stmt, driver);
  EXPECT_TRUE(status.ok()) << status;
}

TEST_F(AssertStmtTest, AssertFalseSurfacesInvalidArgument) {
  const auto* stmt = Analyze("ASSERT FALSE");
  ASSERT_NE(stmt, nullptr);
  ScriptDriver driver;
  absl::Status status =
      ExecuteAssert(MakeRequest("ASSERT FALSE"), *stmt, driver);
  ASSERT_FALSE(status.ok());
  EXPECT_EQ(status.code(), absl::StatusCode::kInvalidArgument);
  EXPECT_EQ(GetSemanticErrorReason(status),
            SemanticErrorReason::kInvalidArgument);
  EXPECT_NE(status.message().find("Assertion failed"), std::string::npos)
      << status.message();
}

TEST_F(AssertStmtTest, AssertFalseWithDescriptionIncludesIt) {
  const std::string sql = "ASSERT FALSE AS 'predicate must hold'";
  const auto* stmt = Analyze(sql);
  ASSERT_NE(stmt, nullptr);
  ScriptDriver driver;
  absl::Status status = ExecuteAssert(MakeRequest(sql), *stmt, driver);
  ASSERT_FALSE(status.ok());
  EXPECT_EQ(status.code(), absl::StatusCode::kInvalidArgument);
  EXPECT_NE(status.message().find("predicate must hold"), std::string::npos)
      << status.message();
}

TEST_F(AssertStmtTest, AssertNullTreatedAsFailure) {
  const std::string sql = "ASSERT CAST(NULL AS BOOL)";
  const auto* stmt = Analyze(sql);
  ASSERT_NE(stmt, nullptr);
  ScriptDriver driver;
  absl::Status status = ExecuteAssert(MakeRequest(sql), *stmt, driver);
  ASSERT_FALSE(status.ok());
  EXPECT_EQ(status.code(), absl::StatusCode::kInvalidArgument);
}

TEST_F(AssertStmtTest, AssertWithComparisonOnTruePredicate) {
  const std::string sql = "ASSERT 1 + 1 = 2";
  const auto* stmt = Analyze(sql);
  ASSERT_NE(stmt, nullptr);
  ScriptDriver driver;
  EXPECT_TRUE(ExecuteAssert(MakeRequest(sql), *stmt, driver).ok());
}

TEST_F(AssertStmtTest, AssertPropagatesEvaluationError) {
  // The expression evaluator surfaces a structured division-by-zero
  // error; ASSERT propagates it verbatim so the caller sees the
  // same envelope they would have seen for `SELECT 1.0 / 0`.
  const std::string sql = "ASSERT 1.0 / 0 > 0";
  const auto* stmt = Analyze(sql);
  ASSERT_NE(stmt, nullptr);
  ScriptDriver driver;
  absl::Status status = ExecuteAssert(MakeRequest(sql), *stmt, driver);
  ASSERT_FALSE(status.ok());
  EXPECT_EQ(GetSemanticErrorReason(status),
            SemanticErrorReason::kDivisionByZero);
}

}  // namespace
}  // namespace script
}  // namespace semantic
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
