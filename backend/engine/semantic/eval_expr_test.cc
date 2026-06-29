// Tests for `EvalExpr` (core expression forms).

#include <gtest/gtest.h>

#include "backend/engine/semantic/error.h"
#include "backend/engine/semantic/eval_expr_test_fixture.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace semantic {

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

}  // namespace semantic
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
