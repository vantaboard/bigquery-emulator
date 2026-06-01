// Tests for the three stub executors. Each one is supposed to
// return UNIMPLEMENTED with a disposition-aware message; the tests
// pin the route name + plan pointer the message advertises so the
// future conformance routing matrix (planned in
// `conformance-routing-matrix.plan.md`) and any operator-facing log
// pipeline can grep on a stable surface.

#include "backend/engine/coordinator/stub_executors.h"

#include <memory>

#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "absl/strings/string_view.h"
#include "backend/engine/engine.h"
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
namespace coordinator {
namespace {

::googlesql::AnalyzerOptions MakeAnalyzerOptions() {
  ::googlesql::LanguageOptions language;
  language.EnableMaximumLanguageFeatures();
  language.set_product_mode(::googlesql::PRODUCT_EXTERNAL);
  ::googlesql::AnalyzerOptions options(language);
  options.CreateDefaultArenasIfNotSet();
  return options;
}

class StubExecutorsTest : public ::testing::Test {
 protected:
  void SetUp() override {
    type_factory_ = std::make_unique<::googlesql::TypeFactory>();
    catalog_ = std::make_unique<::googlesql::SimpleCatalog>(
        "stub_catalog", type_factory_.get());
    catalog_->AddBuiltinFunctions(
        ::googlesql::BuiltinFunctionOptions::AllReleasedFunctions());
  }

  const ::googlesql::ResolvedStatement* AnalyzeSelect1() {
    last_output_.reset();
    ::googlesql::AnalyzerOptions options = MakeAnalyzerOptions();
    absl::Status s = ::googlesql::AnalyzeStatement("SELECT 1",
                                                   options,
                                                   catalog_.get(),
                                                   type_factory_.get(),
                                                   &last_output_);
    EXPECT_TRUE(s.ok()) << s;
    if (!s.ok() || last_output_ == nullptr) return nullptr;
    return last_output_->resolved_statement();
  }

  QueryRequest MakeRequest() {
    QueryRequest req;
    req.sql = "SELECT 1";
    return req;
  }

  std::unique_ptr<::googlesql::TypeFactory> type_factory_{};
  std::unique_ptr<::googlesql::SimpleCatalog> catalog_{};
  std::unique_ptr<const ::googlesql::AnalyzerOutput> last_output_{};
};

TEST_F(StubExecutorsTest, SemanticExecuteQueryNamesRoute) {
  const ::googlesql::ResolvedStatement* stmt = AnalyzeSelect1();
  ASSERT_NE(stmt, nullptr);
  SemanticExecutor exec;
  auto out = exec.ExecuteQuery(MakeRequest(), *stmt, catalog_.get());
  ASSERT_FALSE(out.ok());
  EXPECT_EQ(out.status().code(), absl::StatusCode::kUnimplemented);
  const std::string msg(out.status().message());
  EXPECT_NE(msg.find("semantic_executor"), std::string::npos) << msg;
  EXPECT_NE(msg.find("semantic-executor-core.plan.md"), std::string::npos)
      << msg;
  EXPECT_NE(msg.find("ExecuteQuery"), std::string::npos) << msg;
}

TEST_F(StubExecutorsTest, SemanticExecuteDmlNamesRoute) {
  const ::googlesql::ResolvedStatement* stmt = AnalyzeSelect1();
  ASSERT_NE(stmt, nullptr);
  SemanticExecutor exec;
  auto out = exec.ExecuteDml(MakeRequest(), *stmt, catalog_.get());
  ASSERT_FALSE(out.ok());
  EXPECT_EQ(out.status().code(), absl::StatusCode::kUnimplemented);
  const std::string msg(out.status().message());
  EXPECT_NE(msg.find("semantic_executor"), std::string::npos) << msg;
  EXPECT_NE(msg.find("ExecuteDml"), std::string::npos) << msg;
}

// `ControlOpExecutor` graduated out of `stub_executors.h`; its
// per-route + per-statement contract is exercised by the real
// executor's own tests at `backend/engine/control/
// control_op_executor_test.cc`.

TEST_F(StubExecutorsTest, UnsupportedExecuteQueryNamesRoute) {
  const ::googlesql::ResolvedStatement* stmt = AnalyzeSelect1();
  ASSERT_NE(stmt, nullptr);
  UnsupportedExecutor exec;
  auto out = exec.ExecuteQuery(MakeRequest(), *stmt, catalog_.get());
  ASSERT_FALSE(out.ok());
  EXPECT_EQ(out.status().code(), absl::StatusCode::kUnimplemented);
  const std::string msg(out.status().message());
  EXPECT_NE(msg.find("unsupported"), std::string::npos) << msg;
  EXPECT_NE(msg.find("specialized-feature-policy.plan.md"), std::string::npos)
      << msg;
}

TEST_F(StubExecutorsTest, UnsupportedReportsStatementKind) {
  // The message must name the statement kind the executor saw so a
  // future operator-facing log filter can grep it out without having
  // to round-trip back through the resolved AST.
  const ::googlesql::ResolvedStatement* stmt = AnalyzeSelect1();
  ASSERT_NE(stmt, nullptr);
  UnsupportedExecutor exec;
  auto out = exec.ExecuteQuery(MakeRequest(), *stmt, catalog_.get());
  ASSERT_FALSE(out.ok());
  EXPECT_NE(std::string(out.status().message()).find("QueryStmt"),
            std::string::npos)
      << out.status().message();
}

}  // namespace
}  // namespace coordinator
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
