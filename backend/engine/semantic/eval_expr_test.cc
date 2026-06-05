// Tests for `EvalExpr`.
//
// We drive a real `AnalyzeStatement` against a tiny
// `SimpleCatalog` (mirroring `route_classifier_test.cc`) so the
// `ResolvedExpr` the evaluator sees is exactly the shape the
// engine sees at runtime. Each test analyzes one SQL fragment,
// reaches into the resolved AST for the expression of interest,
// and runs `EvalExpr` over it directly -- no executor / row
// source plumbing.

#include "backend/engine/semantic/eval_expr.h"

#include <cmath>
#include <cstdint>
#include <limits>
#include <memory>
#include <optional>
#include <string>

#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/string_view.h"
#include "backend/engine/semantic/error.h"
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
namespace {

::googlesql::AnalyzerOptions MakeAnalyzerOptions() {
  ::googlesql::LanguageOptions language;
  language.EnableMaximumLanguageFeatures();
  language.set_product_mode(::googlesql::PRODUCT_EXTERNAL);
  ::googlesql::AnalyzerOptions options(language);
  options.CreateDefaultArenasIfNotSet();
  return options;
}

class EvalExprTest : public ::testing::Test {
 protected:
  void SetUp() override {
    type_factory_ = std::make_unique<::googlesql::TypeFactory>();
    catalog_ = std::make_unique<::googlesql::SimpleCatalog>(
        "eval_catalog", type_factory_.get());
    catalog_->AddBuiltinFunctions(
        ::googlesql::BuiltinFunctionOptions::AllReleasedFunctions());
  }

  // Analyze `SELECT <expr>` and return the resolved expression
  // for the first output column. Stores the analyzer output on
  // `last_output_` so column / function pointers stay valid for
  // the duration of the test. When `options` is missing the
  // helper falls back to `MakeAnalyzerOptions()`; tests that
  // declare query parameters pass the populated options in
  // explicitly.
  const ::googlesql::ResolvedExpr* AnalyzeExpr(
      absl::string_view expr,
      std::optional<::googlesql::AnalyzerOptions> options_in = std::nullopt) {
    ::googlesql::AnalyzerOptions options =
        options_in.has_value() ? *std::move(options_in) : MakeAnalyzerOptions();
    last_output_.reset();
    const std::string sql = absl::StrCat("SELECT ", expr);
    absl::Status s = ::googlesql::AnalyzeStatement(
        sql, options, catalog_.get(), type_factory_.get(), &last_output_);
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

TEST_F(EvalExprTest, LiteralInt64) {
  const auto* expr = AnalyzeExpr("42");
  ASSERT_NE(expr, nullptr);
  auto v = EvalExpr(*expr, EvalContext{});
  ASSERT_TRUE(v.ok()) << v.status();
  EXPECT_EQ(v->int64_value(), 42);
}

TEST_F(EvalExprTest, LiteralStringRoundTrips) {
  const auto* expr = AnalyzeExpr("'hello'");
  ASSERT_NE(expr, nullptr);
  auto v = EvalExpr(*expr, EvalContext{});
  ASSERT_TRUE(v.ok()) << v.status();
  EXPECT_EQ(v->string_value(), "hello");
}

TEST_F(EvalExprTest, AdditionInt64) {
  const auto* expr = AnalyzeExpr("1 + 2");
  ASSERT_NE(expr, nullptr);
  auto v = EvalExpr(*expr, EvalContext{});
  ASSERT_TRUE(v.ok()) << v.status();
  EXPECT_EQ(v->int64_value(), 3);
}

TEST_F(EvalExprTest, AdditionNullOperandIsNull) {
  const auto* expr = AnalyzeExpr("CAST(NULL AS INT64) + 1");
  ASSERT_NE(expr, nullptr);
  auto v = EvalExpr(*expr, EvalContext{});
  ASSERT_TRUE(v.ok()) << v.status();
  EXPECT_TRUE(v->is_null());
}

TEST_F(EvalExprTest, Int64OverflowSurfacesAsSemanticError) {
  const auto* expr = AnalyzeExpr("9223372036854775807 + 1");
  ASSERT_NE(expr, nullptr);
  auto v = EvalExpr(*expr, EvalContext{});
  ASSERT_FALSE(v.ok());
  EXPECT_EQ(GetSemanticErrorReason(v.status()), SemanticErrorReason::kOverflow);
}

TEST_F(EvalExprTest, DivisionByZeroFloat64SurfacesReason) {
  const auto* expr = AnalyzeExpr("CAST(1 AS FLOAT64) / 0");
  ASSERT_NE(expr, nullptr);
  auto v = EvalExpr(*expr, EvalContext{});
  ASSERT_FALSE(v.ok());
  EXPECT_EQ(GetSemanticErrorReason(v.status()),
            SemanticErrorReason::kDivisionByZero);
}

TEST_F(EvalExprTest, ComparisonLessThan) {
  const auto* expr = AnalyzeExpr("1 < 2");
  ASSERT_NE(expr, nullptr);
  auto v = EvalExpr(*expr, EvalContext{});
  ASSERT_TRUE(v.ok()) << v.status();
  EXPECT_TRUE(v->bool_value());
}

TEST_F(EvalExprTest, ComparisonNullOperandIsNull) {
  const auto* expr = AnalyzeExpr("CAST(NULL AS INT64) = 1");
  ASSERT_NE(expr, nullptr);
  auto v = EvalExpr(*expr, EvalContext{});
  ASSERT_TRUE(v.ok()) << v.status();
  EXPECT_TRUE(v->is_null());
}

TEST_F(EvalExprTest, LogicalAndShortCircuitOnFalse) {
  const auto* expr = AnalyzeExpr("FALSE AND CAST(NULL AS BOOL)");
  ASSERT_NE(expr, nullptr);
  auto v = EvalExpr(*expr, EvalContext{});
  ASSERT_TRUE(v.ok()) << v.status();
  EXPECT_FALSE(v->bool_value());
}

TEST_F(EvalExprTest, LogicalOrShortCircuitOnTrue) {
  const auto* expr = AnalyzeExpr("TRUE OR CAST(NULL AS BOOL)");
  ASSERT_NE(expr, nullptr);
  auto v = EvalExpr(*expr, EvalContext{});
  ASSERT_TRUE(v.ok()) << v.status();
  EXPECT_TRUE(v->bool_value());
}

TEST_F(EvalExprTest, IfReturnsThenBranchOnTrue) {
  const auto* expr = AnalyzeExpr("IF(1 < 2, 'yes', 'no')");
  ASSERT_NE(expr, nullptr);
  auto v = EvalExpr(*expr, EvalContext{});
  ASSERT_TRUE(v.ok()) << v.status();
  EXPECT_EQ(v->string_value(), "yes");
}

TEST_F(EvalExprTest, IfReturnsElseBranchOnFalse) {
  const auto* expr = AnalyzeExpr("IF(1 > 2, 'yes', 'no')");
  ASSERT_NE(expr, nullptr);
  auto v = EvalExpr(*expr, EvalContext{});
  ASSERT_TRUE(v.ok()) << v.status();
  EXPECT_EQ(v->string_value(), "no");
}

TEST_F(EvalExprTest, CoalescePicksFirstNonNull) {
  const auto* expr = AnalyzeExpr("COALESCE(CAST(NULL AS INT64), 2, 3)");
  ASSERT_NE(expr, nullptr);
  auto v = EvalExpr(*expr, EvalContext{});
  ASSERT_TRUE(v.ok()) << v.status();
  EXPECT_EQ(v->int64_value(), 2);
}

TEST_F(EvalExprTest, CoalesceAllNullProducesNull) {
  const auto* expr =
      AnalyzeExpr("COALESCE(CAST(NULL AS INT64), CAST(NULL AS INT64))");
  ASSERT_NE(expr, nullptr);
  auto v = EvalExpr(*expr, EvalContext{});
  ASSERT_TRUE(v.ok()) << v.status();
  EXPECT_TRUE(v->is_null());
}

TEST_F(EvalExprTest, IfNullReturnsFallbackOnNull) {
  const auto* expr = AnalyzeExpr("IFNULL(CAST(NULL AS INT64), 7)");
  ASSERT_NE(expr, nullptr);
  auto v = EvalExpr(*expr, EvalContext{});
  ASSERT_TRUE(v.ok()) << v.status();
  EXPECT_EQ(v->int64_value(), 7);
}

TEST_F(EvalExprTest, NullIfReturnsNullWhenEqual) {
  const auto* expr = AnalyzeExpr("NULLIF(1, 1)");
  ASSERT_NE(expr, nullptr);
  auto v = EvalExpr(*expr, EvalContext{});
  ASSERT_TRUE(v.ok()) << v.status();
  EXPECT_TRUE(v->is_null());
}

TEST_F(EvalExprTest, NullIfReturnsFirstWhenDifferent) {
  const auto* expr = AnalyzeExpr("NULLIF(1, 2)");
  ASSERT_NE(expr, nullptr);
  auto v = EvalExpr(*expr, EvalContext{});
  ASSERT_TRUE(v.ok()) << v.status();
  EXPECT_EQ(v->int64_value(), 1);
}

TEST_F(EvalExprTest, CaseWhenBranchesEvaluate) {
  const auto* expr =
      AnalyzeExpr("CASE WHEN 1 = 1 THEN 'a' WHEN 2 = 2 THEN 'b' ELSE 'c' END");
  ASSERT_NE(expr, nullptr);
  auto v = EvalExpr(*expr, EvalContext{});
  ASSERT_TRUE(v.ok()) << v.status();
  EXPECT_EQ(v->string_value(), "a");
}

TEST_F(EvalExprTest, CaseWithValueBranchesEvaluate) {
  const auto* expr =
      AnalyzeExpr("CASE 2 WHEN 1 THEN 'a' WHEN 2 THEN 'b' ELSE 'c' END");
  ASSERT_NE(expr, nullptr);
  auto v = EvalExpr(*expr, EvalContext{});
  ASSERT_TRUE(v.ok()) << v.status();
  EXPECT_EQ(v->string_value(), "b");
}

TEST_F(EvalExprTest, CaseFallsThroughToElseOnNoMatch) {
  const auto* expr =
      AnalyzeExpr("CASE WHEN 1 = 2 THEN 'a' WHEN 1 = 3 THEN 'b' ELSE 'c' END");
  ASSERT_NE(expr, nullptr);
  auto v = EvalExpr(*expr, EvalContext{});
  ASSERT_TRUE(v.ok()) << v.status();
  EXPECT_EQ(v->string_value(), "c");
}

TEST_F(EvalExprTest, SafeAddOverflowProducesNull) {
  const auto* expr = AnalyzeExpr("SAFE_ADD(9223372036854775807, 1)");
  ASSERT_NE(expr, nullptr);
  auto v = EvalExpr(*expr, EvalContext{});
  ASSERT_TRUE(v.ok()) << v.status();
  EXPECT_TRUE(v->is_null());
}

TEST_F(EvalExprTest, SafeDivideByZeroProducesNull) {
  const auto* expr = AnalyzeExpr("SAFE_DIVIDE(1, 0)");
  ASSERT_NE(expr, nullptr);
  auto v = EvalExpr(*expr, EvalContext{});
  ASSERT_TRUE(v.ok()) << v.status();
  EXPECT_TRUE(v->is_null());
}

TEST_F(EvalExprTest, ParameterByNameResolves) {
  ::googlesql::AnalyzerOptions options = MakeAnalyzerOptions();
  ASSERT_TRUE(
      options.AddQueryParameter("p", ::googlesql::types::Int64Type()).ok());
  const auto* expr = AnalyzeExpr("@p + 1", options);
  ASSERT_NE(expr, nullptr);
  ParameterBindings bindings;
  bindings.by_name["p"] = Value::Int64(40);
  EvalContext ctx;
  ctx.parameters = &bindings;
  auto v = EvalExpr(*expr, ctx);
  ASSERT_TRUE(v.ok()) << v.status();
  EXPECT_EQ(v->int64_value(), 41);
}

TEST_F(EvalExprTest, ParameterByPositionResolves) {
  ::googlesql::AnalyzerOptions options = MakeAnalyzerOptions();
  options.set_parameter_mode(::googlesql::PARAMETER_POSITIONAL);
  ASSERT_TRUE(
      options.AddPositionalQueryParameter(::googlesql::types::Int64Type())
          .ok());
  const auto* expr = AnalyzeExpr("? + 1", options);
  ASSERT_NE(expr, nullptr);
  ParameterBindings bindings;
  bindings.by_position.push_back(Value::Int64(40));
  EvalContext ctx;
  ctx.parameters = &bindings;
  auto v = EvalExpr(*expr, ctx);
  ASSERT_TRUE(v.ok()) << v.status();
  EXPECT_EQ(v->int64_value(), 41);
}

TEST_F(EvalExprTest, UnaryMinusInt64Min) {
  const auto* expr = AnalyzeExpr("-(-9223372036854775807 - 1)");
  ASSERT_NE(expr, nullptr);
  auto v = EvalExpr(*expr, EvalContext{});
  // -INT64_MIN overflows; the inner subtraction reaches INT64_MIN,
  // then unary minus overflows.
  ASSERT_FALSE(v.ok());
  EXPECT_EQ(GetSemanticErrorReason(v.status()), SemanticErrorReason::kOverflow);
}

TEST_F(EvalExprTest, IsNullForNullOperand) {
  const auto* expr = AnalyzeExpr("CAST(NULL AS INT64) IS NULL");
  ASSERT_NE(expr, nullptr);
  auto v = EvalExpr(*expr, EvalContext{});
  ASSERT_TRUE(v.ok()) << v.status();
  EXPECT_TRUE(v->bool_value());
}

TEST_F(EvalExprTest, IsNotNullForNonNullOperand) {
  const auto* expr = AnalyzeExpr("1 IS NOT NULL");
  ASSERT_NE(expr, nullptr);
  auto v = EvalExpr(*expr, EvalContext{});
  ASSERT_TRUE(v.ok()) << v.status();
  EXPECT_TRUE(v->bool_value());
}

TEST_F(EvalExprTest, LikeOperator) {
  const auto* expr = AnalyzeExpr(R"("abcd" LIKE "a%d")");
  ASSERT_NE(expr, nullptr);
  auto v = EvalExpr(*expr, EvalContext{});
  ASSERT_TRUE(v.ok()) << v.status();
  EXPECT_TRUE(v->bool_value());
}

TEST_F(EvalExprTest, BetweenOperatorOnDates) {
  const auto* expr =
      AnalyzeExpr(R"(DATE "2022-09-10" BETWEEN "2022-09-01" AND "2022-10-01")");
  ASSERT_NE(expr, nullptr);
  auto v = EvalExpr(*expr, EvalContext{});
  ASSERT_TRUE(v.ok()) << v.status();
  EXPECT_TRUE(v->bool_value());
}

TEST_F(EvalExprTest, InOperatorNullLhs) {
  const auto* expr = AnalyzeExpr("NULL IN (1)");
  ASSERT_NE(expr, nullptr);
  auto v = EvalExpr(*expr, EvalContext{});
  ASSERT_TRUE(v.ok()) << v.status();
  EXPECT_TRUE(v->is_null());
}

TEST_F(EvalExprTest, BitwiseAndOperator) {
  const auto* expr = AnalyzeExpr("3 & 1");
  ASSERT_NE(expr, nullptr);
  auto v = EvalExpr(*expr, EvalContext{});
  ASSERT_TRUE(v.ok()) << v.status();
  EXPECT_EQ(v->int64_value(), 1);
}

TEST_F(EvalExprTest, IsDistinctFromNullAndNull) {
  const auto* expr = AnalyzeExpr("NULL IS DISTINCT FROM NULL");
  ASSERT_NE(expr, nullptr);
  auto v = EvalExpr(*expr, EvalContext{});
  ASSERT_TRUE(v.ok()) << v.status();
  EXPECT_FALSE(v->bool_value());
}

TEST_F(EvalExprTest, IntervalLiteralDay) {
  const auto* expr = AnalyzeExpr("INTERVAL 29 DAY");
  ASSERT_NE(expr, nullptr);
  auto v = EvalExpr(*expr, EvalContext{});
  ASSERT_TRUE(v.ok()) << v.status();
  EXPECT_FALSE(v->is_null());
  EXPECT_EQ(v->type_kind(), ::googlesql::TYPE_INTERVAL);
}

TEST_F(EvalExprTest, JustifyDaysOn29Days) {
  const auto* expr = AnalyzeExpr("JUSTIFY_DAYS(INTERVAL 29 DAY)");
  ASSERT_NE(expr, nullptr);
  auto v = EvalExpr(*expr, EvalContext{});
  ASSERT_TRUE(v.ok()) << v.status();
  EXPECT_EQ(v->interval_value().ToString(), "0-0 29 0:0:0");
}

TEST_F(EvalExprTest, JustifyHoursNegativeMinuteLiteral) {
  const auto* expr = AnalyzeExpr("JUSTIFY_HOURS(INTERVAL -12345 MINUTE)");
  ASSERT_NE(expr, nullptr);
  auto v = EvalExpr(*expr, EvalContext{});
  ASSERT_TRUE(v.ok()) << v.status();
  EXPECT_EQ(v->interval_value().ToString(), "0-0 -8 -13:45:0");
}

TEST_F(EvalExprTest, ResolvedArgumentRefResolvesAgainstFrameStack) {
  // `ResolvedArgumentRef` resolves through `EvalContext::arguments`
  // (a `FrameStack`). The analyzer usually only emits this kind
  // inside a UDF / TVF body, but the node API supports direct
  // construction, so we build one with `MakeResolvedArgumentRef`
  // and verify the executor reads the matching frame binding.
  FrameStack args;
  ASSERT_TRUE(args.Declare("x", Value::Int64(7)).ok());

  std::unique_ptr<::googlesql::ResolvedArgumentRef> ref =
      ::googlesql::MakeResolvedArgumentRef(
          ::googlesql::types::Int64Type(),
          "x",
          ::googlesql::ResolvedArgumentDef::SCALAR);
  EvalContext ctx;
  ctx.arguments = &args;
  auto v = EvalExpr(*ref, ctx);
  ASSERT_TRUE(v.ok()) << v.status();
  EXPECT_EQ(v->int64_value(), 7);
}

TEST_F(EvalExprTest, ResolvedArgumentRefCaseInsensitiveMatch) {
  // The script driver / UDF call site lowers identifier names on
  // declare. The `FrameStack` is case-insensitive on lookup, so a
  // body reference that case-shifts an argument still resolves.
  FrameStack args;
  ASSERT_TRUE(args.Declare("FooBar", Value::Int64(11)).ok());

  std::unique_ptr<::googlesql::ResolvedArgumentRef> ref =
      ::googlesql::MakeResolvedArgumentRef(
          ::googlesql::types::Int64Type(),
          "FOOBAR",
          ::googlesql::ResolvedArgumentDef::SCALAR);
  EvalContext ctx;
  ctx.arguments = &args;
  auto v = EvalExpr(*ref, ctx);
  ASSERT_TRUE(v.ok()) << v.status();
  EXPECT_EQ(v->int64_value(), 11);
}

TEST_F(EvalExprTest, ResolvedArgumentRefWithoutFrameStackSurfacesError) {
  // No frame on the context -- the executor must surface a clean
  // `kInvalidArgument` naming the missing argument rather than
  // substituting NULL.
  std::unique_ptr<::googlesql::ResolvedArgumentRef> ref =
      ::googlesql::MakeResolvedArgumentRef(
          ::googlesql::types::Int64Type(),
          "y",
          ::googlesql::ResolvedArgumentDef::SCALAR);
  auto v = EvalExpr(*ref, EvalContext{});
  ASSERT_FALSE(v.ok());
  EXPECT_EQ(GetSemanticErrorReason(v.status()),
            SemanticErrorReason::kInvalidArgument);
  EXPECT_EQ(v.status().code(), absl::StatusCode::kInvalidArgument);
}

TEST_F(EvalExprTest, ResolvedArgumentRefMissingBindingSurfacesError) {
  // Frame exists but does not bind the referenced name.
  FrameStack args;
  ASSERT_TRUE(args.Declare("x", Value::Int64(1)).ok());

  std::unique_ptr<::googlesql::ResolvedArgumentRef> ref =
      ::googlesql::MakeResolvedArgumentRef(
          ::googlesql::types::Int64Type(),
          "missing",
          ::googlesql::ResolvedArgumentDef::SCALAR);
  EvalContext ctx;
  ctx.arguments = &args;
  auto v = EvalExpr(*ref, ctx);
  ASSERT_FALSE(v.ok());
  EXPECT_EQ(GetSemanticErrorReason(v.status()),
            SemanticErrorReason::kInvalidArgument);
}

TEST_F(EvalExprTest, ResolvedConstantResolvesToCatalogValue) {
  // Register a `SimpleConstant` on the test catalog and analyze
  // `SELECT <constant>`. The analyzer emits a `ResolvedConstant`
  // whose `constant()` points at the registered entry; `EvalExpr`
  // must return the bound value verbatim (no NULL substitution).
  std::unique_ptr<::googlesql::SimpleConstant> meaning;
  ASSERT_TRUE(::googlesql::SimpleConstant::Create(
                  {"meaning_of_life"}, ::googlesql::Value::Int64(42), &meaning)
                  .ok());
  catalog_->AddOwnedConstant(meaning.release());

  const auto* expr = AnalyzeExpr("meaning_of_life");
  ASSERT_NE(expr, nullptr);
  ASSERT_EQ(expr->node_kind(), ::googlesql::RESOLVED_CONSTANT)
      << "analyzer should have resolved bare identifier to ResolvedConstant; "
         "got "
      << expr->node_kind_string();
  auto v = EvalExpr(*expr, EvalContext{});
  ASSERT_TRUE(v.ok()) << v.status();
  EXPECT_EQ(v->int64_value(), 42);
}

TEST_F(EvalExprTest, FloatNanArithmeticProducesNan) {
  const auto* expr = AnalyzeExpr("CAST('NaN' AS FLOAT64) + 1");
  ASSERT_NE(expr, nullptr);
  auto v = EvalExpr(*expr, EvalContext{});
  ASSERT_TRUE(v.ok()) << v.status();
  EXPECT_TRUE(std::isnan(v->double_value()));
}

}  // namespace
}  // namespace semantic
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
