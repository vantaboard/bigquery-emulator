// End-to-end tests for the semantic-executor function dispatch.
//
// Walks the analyzer the same way `eval_expr_test.cc` does so the
// `ResolvedFunctionCall` the dispatch sees is exactly what the
// engine sees at runtime. Each test drives a SQL fragment that
// resolves to a dispatch-table function and asserts both the
// observable cell and any structured error reason.

#include "backend/engine/semantic/functions/dispatch.h"

#include <cmath>
#include <memory>
#include <optional>
#include <string>

#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/string_view.h"
#include "backend/engine/semantic/error.h"
#include "backend/engine/semantic/eval_expr.h"
#include "backend/engine/semantic/value.h"
#include "googlesql/public/analyzer.h"
#include "googlesql/public/analyzer_options.h"
#include "googlesql/public/analyzer_output.h"
#include "googlesql/public/builtin_function_options.h"
#include "googlesql/public/catalog.h"
#include "googlesql/public/language_options.h"
#include "googlesql/public/options.pb.h"
#include "googlesql/public/simple_catalog.h"
#include "googlesql/public/types/type_factory.h"
#include "googlesql/public/value.h"
#include "googlesql/resolved_ast/resolved_ast.h"
#include "gtest/gtest.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace semantic {
namespace functions {
namespace {

::googlesql::AnalyzerOptions MakeAnalyzerOptions() {
  ::googlesql::LanguageOptions language;
  language.EnableMaximumLanguageFeatures();
  language.set_product_mode(::googlesql::PRODUCT_EXTERNAL);
  ::googlesql::AnalyzerOptions options(language);
  options.CreateDefaultArenasIfNotSet();
  return options;
}

class DispatchTest : public ::testing::Test {
 protected:
  void SetUp() override {
    type_factory_ = std::make_unique<::googlesql::TypeFactory>();
    catalog_ = std::make_unique<::googlesql::SimpleCatalog>(
        "dispatch_catalog", type_factory_.get());
    catalog_->AddBuiltinFunctions(
        ::googlesql::BuiltinFunctionOptions::AllReleasedFunctions());
  }

  const ::googlesql::ResolvedExpr* AnalyzeExpr(absl::string_view expr) {
    last_output_.reset();
    const std::string sql = absl::StrCat("SELECT ", expr);
    absl::Status s = ::googlesql::AnalyzeStatement(sql,
                                                   MakeAnalyzerOptions(),
                                                   catalog_.get(),
                                                   type_factory_.get(),
                                                   &last_output_);
    EXPECT_TRUE(s.ok()) << s;
    if (!s.ok() || last_output_ == nullptr) return nullptr;
    const auto* stmt = last_output_->resolved_statement()
                           ->GetAs<::googlesql::ResolvedQueryStmt>();
    if (stmt == nullptr) return nullptr;
    const auto* project =
        stmt->query()->GetAs<::googlesql::ResolvedProjectScan>();
    if (project == nullptr || project->expr_list_size() == 0) return nullptr;
    return project->expr_list(0)->expr();
  }

  std::unique_ptr<::googlesql::TypeFactory> type_factory_{};
  std::unique_ptr<::googlesql::SimpleCatalog> catalog_{};
  std::unique_ptr<const ::googlesql::AnalyzerOutput> last_output_{};
};

TEST_F(DispatchTest, BitCountReturnsSetBitCount) {
  // Drive `BIT_COUNT(7)` end-to-end through the analyzer + executor.
  const auto* expr = AnalyzeExpr("BIT_COUNT(7)");
  ASSERT_NE(expr, nullptr);
  auto v = EvalExpr(*expr, EvalContext{});
  ASSERT_TRUE(v.ok()) << v.status();
  EXPECT_EQ(v->int64_value(), 3);
}

TEST_F(DispatchTest, BitCountOnNegativeOneReturnsSixtyFour) {
  // The two's-complement edge case the row exists to pin.
  const auto* expr = AnalyzeExpr("BIT_COUNT(-1)");
  ASSERT_NE(expr, nullptr);
  auto v = EvalExpr(*expr, EvalContext{});
  ASSERT_TRUE(v.ok()) << v.status();
  EXPECT_EQ(v->int64_value(), 64);
}

TEST_F(DispatchTest, IeeeDivideByZeroProducesInf) {
  // IEEE_DIVIDE never errors: 1.0 / 0.0 -> +Inf per IEEE 754, not
  // a structured `kDivisionByZero` status the way `/` would.
  const auto* expr = AnalyzeExpr("IEEE_DIVIDE(1.0, 0.0)");
  ASSERT_NE(expr, nullptr);
  auto v = EvalExpr(*expr, EvalContext{});
  ASSERT_TRUE(v.ok()) << v.status();
  EXPECT_TRUE(std::isinf(v->double_value()));
  EXPECT_GT(v->double_value(), 0.0);
}

TEST_F(DispatchTest, IeeeDivideZeroOverZeroProducesNan) {
  const auto* expr = AnalyzeExpr("IEEE_DIVIDE(0.0, 0.0)");
  ASSERT_NE(expr, nullptr);
  auto v = EvalExpr(*expr, EvalContext{});
  ASSERT_TRUE(v.ok()) << v.status();
  EXPECT_TRUE(std::isnan(v->double_value()));
}

TEST_F(DispatchTest, IeeeDivideFinitePath) {
  const auto* expr = AnalyzeExpr("IEEE_DIVIDE(10.0, 4.0)");
  ASSERT_NE(expr, nullptr);
  auto v = EvalExpr(*expr, EvalContext{});
  ASSERT_TRUE(v.ok()) << v.status();
  EXPECT_DOUBLE_EQ(v->double_value(), 2.5);
}

// The dispatch table returns nullopt for unknown function names;
// `EvalFunctionCall` is expected to surface a `kNotImplemented`
// status in that case. The marker here is a sanity check that the
// table is consulted; the actual status is asserted in the
// matching `eval_expr_test.cc` case.
TEST(DispatchTableTest, UnknownNameReturnsNullopt) {
  std::vector<Value> args = {Value::Int64(1)};
  auto v = Dispatch("__not_a_function__", args, nullptr);
  EXPECT_FALSE(v.has_value());
}

}  // namespace
}  // namespace functions
}  // namespace semantic
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
