// Extended `EvalExpr` coverage (parameters, intervals, argument refs).

#include <cmath>
#include <memory>

#include "backend/engine/semantic/error.h"
#include "backend/engine/semantic/eval_expr_test_fixture.h"
#include "backend/engine/semantic/value.h"
#include "googlesql/public/analyzer_options.h"
#include "googlesql/public/types/type_factory.h"
#include "googlesql/public/value.h"
#include "googlesql/resolved_ast/resolved_ast.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace semantic {

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

TEST_F(EvalExprTest, CastStringDateOnlyToTimestamp) {
  const auto* expr = AnalyzeExpr("CAST('1800-01-01' AS TIMESTAMP)");
  ASSERT_NE(expr, nullptr);
  auto v = EvalExpr(*expr, EvalContext{});
  ASSERT_TRUE(v.ok()) << v.status();
  EXPECT_EQ(v->type_kind(), ::googlesql::TYPE_TIMESTAMP);
}

TEST_F(EvalExprTest, DateBetweenOldDatesEvaluates) {
  const auto* expr = AnalyzeExpr(
      "DATE('1800-01-01') BETWEEN DATE('1800-01-01') AND DATE('1899-12-31')");
  ASSERT_NE(expr, nullptr);
  auto v = EvalExpr(*expr, EvalContext{});
  ASSERT_TRUE(v.ok()) << v.status();
  EXPECT_TRUE(v->bool_value());
}

}  // namespace semantic
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
