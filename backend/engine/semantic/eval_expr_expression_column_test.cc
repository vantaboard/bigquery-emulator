#include "backend/engine/semantic/error.h"
#include "backend/engine/semantic/eval_expr.h"
#include "backend/engine/semantic/eval_expr_test_fixture.h"
#include "backend/engine/semantic/expression_column_bindings.h"
#include "backend/engine/semantic/frame_stack.h"
#include "googlesql/public/analyzer.h"
#include "googlesql/public/type.h"
#include "googlesql/public/value.h"
#include "googlesql/resolved_ast/resolved_ast.h"
#include "googlesql/resolved_ast/resolved_node_kind.pb.h"
#include "gtest/gtest.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace semantic {
namespace {

TEST_F(EvalExprTest, ResolvedExpressionColumnBindsFromColumnsByName) {
  FrameStack variables;
  ASSERT_TRUE(variables.Declare("base", ::googlesql::Value::Int64(10)).ok());

  ::googlesql::AnalyzerOptions options = MakeAnalyzerOptions();
  ASSERT_TRUE(
      RegisterExpressionColumnsOnAnalyzerOptions(variables, options).ok());

  std::unique_ptr<const ::googlesql::AnalyzerOutput> output;
  absl::Status analyzed = ::googlesql::AnalyzeExpression(
      "base + 5", options, catalog_.get(), type_factory_.get(), &output);
  ASSERT_TRUE(analyzed.ok()) << analyzed;
  ASSERT_NE(output, nullptr);
  ASSERT_NE(output->resolved_expr(), nullptr);

  EvalContext ctx;
  absl::flat_hash_map<std::string, ::googlesql::Value> columns_by_name;
  PopulateEvalContextExpressionColumns(variables, ctx, &columns_by_name);

  absl::StatusOr<Value> result = EvalExpr(*output->resolved_expr(), ctx);
  ASSERT_TRUE(result.ok()) << result.status();
  EXPECT_TRUE(result->type()->IsInt64());
  EXPECT_EQ(result->int64_value(), 15);
}

TEST_F(EvalExprTest, ResolvedExpressionColumnFallsBackToScriptVariables) {
  std::unique_ptr<::googlesql::ResolvedExpressionColumn> col =
      ::googlesql::MakeResolvedExpressionColumn(::googlesql::types::Int64Type(),
                                                "counter");
  FrameStack variables;
  ASSERT_TRUE(variables.Declare("counter", ::googlesql::Value::Int64(42)).ok());

  EvalContext ctx;
  ctx.script_variables = &variables;
  absl::StatusOr<Value> result = EvalExpr(*col, ctx);
  ASSERT_TRUE(result.ok()) << result.status();
  EXPECT_EQ(result->int64_value(), 42);
}

TEST_F(EvalExprTest, ResolvedExpressionColumnMissingBindingSurfacesError) {
  std::unique_ptr<::googlesql::ResolvedExpressionColumn> col =
      ::googlesql::MakeResolvedExpressionColumn(::googlesql::types::Int64Type(),
                                                "missing");

  EvalContext ctx;
  absl::StatusOr<Value> result = EvalExpr(*col, ctx);
  ASSERT_FALSE(result.ok());
  EXPECT_EQ(GetSemanticErrorReason(result.status()),
            SemanticErrorReason::kInvalidArgument);
}

TEST_F(EvalExprTest,
       ResolvedCatalogColumnRefGraphPropertySurfacesNotImplemented) {
  std::unique_ptr<::googlesql::ResolvedCatalogColumnRef> ref =
      ::googlesql::MakeResolvedCatalogColumnRef(::googlesql::types::Int64Type(),
                                                /*column=*/nullptr,
                                                "graph_prop");

  EvalContext ctx;
  absl::StatusOr<Value> result = EvalExpr(*ref, ctx);
  ASSERT_FALSE(result.ok());
  EXPECT_EQ(GetSemanticErrorReason(result.status()),
            SemanticErrorReason::kNotImplemented);
}

}  // namespace
}  // namespace semantic
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
