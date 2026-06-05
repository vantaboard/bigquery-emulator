#include "backend/engine/duckdb/transpiler/transpiler_test_fixture.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace duckdb {
namespace transpiler {

TEST_F(TranspilerTest, EmitParameterNamed) {
  // `SELECT @customer_id` analyzes to a ProjectScan whose only
  // computed column is a `ResolvedParameter` carrying the
  // analyzer-lowercased name (`customer_id`). We assert on both the
  // emitted `$N` placeholder and the bind-order accumulator so a
  // regression in either side surfaces here rather than downstream
  // in the engine integration.
  ::googlesql::AnalyzerOptions options = MakeAnalyzerOptions();
  ASSERT_TRUE(
      options.AddQueryParameter("customer_id", type_factory_->get_int64())
          .ok());
  const ::googlesql::ResolvedStatement* stmt =
      AnalyzeWith("SELECT @customer_id AS x", options);
  const ::googlesql::ResolvedExpr* expr = QueryFirstSelectExpr(stmt);
  ASSERT_NE(expr, nullptr);
  ASSERT_EQ(expr->node_kind(), ::googlesql::RESOLVED_PARAMETER);
  TestTranspiler t;
  EXPECT_EQ(t.EmitParameter(expr->GetAs<::googlesql::ResolvedParameter>()),
            "$1");
  ASSERT_EQ(t.parameter_order().size(), 1u);
  EXPECT_EQ(t.parameter_order()[0].name, "customer_id");
  EXPECT_EQ(t.parameter_order()[0].position, 0);
}

TEST_F(TranspilerTest, EmitParameterReuseSharesSlot) {
  // Two textual references to the same named parameter must share a
  // single DuckDB `$N` slot so the engine binds one value, not two.
  // We hit `EmitParameter` twice on the same (or equivalent) node and
  // assert both emits go to `$1` and the bind-order accumulator
  // carries exactly one entry.
  ::googlesql::AnalyzerOptions options = MakeAnalyzerOptions();
  ASSERT_TRUE(
      options.AddQueryParameter("threshold", type_factory_->get_int64()).ok());
  // Use the parameter twice in distinct projections: GoogleSQL
  // produces two `ResolvedParameter` nodes (one per reference) but
  // both carry the same `name()`, so the dedup collapses them.
  const ::googlesql::ResolvedStatement* stmt =
      AnalyzeWith("SELECT @threshold AS a, @threshold AS b", options);
  ASSERT_NE(stmt, nullptr);
  const auto* q = stmt->GetAs<::googlesql::ResolvedQueryStmt>();
  ASSERT_NE(q, nullptr);
  const ::googlesql::ResolvedScan* scan = q->query();
  ASSERT_EQ(scan->node_kind(), ::googlesql::RESOLVED_PROJECT_SCAN);
  const auto* project = scan->GetAs<::googlesql::ResolvedProjectScan>();
  ASSERT_GE(project->expr_list_size(), 2);
  TestTranspiler t;
  EXPECT_EQ(t.EmitParameter(project->expr_list(0)
                                ->expr()
                                ->GetAs<::googlesql::ResolvedParameter>()),
            "$1");
  EXPECT_EQ(t.EmitParameter(project->expr_list(1)
                                ->expr()
                                ->GetAs<::googlesql::ResolvedParameter>()),
            "$1");
  ASSERT_EQ(t.parameter_order().size(), 1u);
  EXPECT_EQ(t.parameter_order()[0].name, "threshold");
}

TEST_F(TranspilerTest, EmitParameterPositionalAssignsFreshSlots) {
  // Positional parameters carry a 1-based `position()` and are
  // referentially distinct on every analyzer reference; we never
  // dedupe them. Two positional references emit `$1` then `$2` and
  // the bind-order accumulator records both with the analyzer
  // positions intact.
  ::googlesql::AnalyzerOptions options = MakeAnalyzerOptions();
  options.set_parameter_mode(::googlesql::PARAMETER_POSITIONAL);
  ASSERT_TRUE(
      options.AddPositionalQueryParameter(type_factory_->get_int64()).ok());
  ASSERT_TRUE(
      options.AddPositionalQueryParameter(type_factory_->get_string()).ok());
  const ::googlesql::ResolvedStatement* stmt =
      AnalyzeWith("SELECT ? AS a, ? AS b", options);
  ASSERT_NE(stmt, nullptr);
  const auto* q = stmt->GetAs<::googlesql::ResolvedQueryStmt>();
  ASSERT_NE(q, nullptr);
  const ::googlesql::ResolvedScan* scan = q->query();
  ASSERT_EQ(scan->node_kind(), ::googlesql::RESOLVED_PROJECT_SCAN);
  const auto* project = scan->GetAs<::googlesql::ResolvedProjectScan>();
  ASSERT_GE(project->expr_list_size(), 2);
  TestTranspiler t;
  EXPECT_EQ(t.EmitParameter(project->expr_list(0)
                                ->expr()
                                ->GetAs<::googlesql::ResolvedParameter>()),
            "$1");
  EXPECT_EQ(t.EmitParameter(project->expr_list(1)
                                ->expr()
                                ->GetAs<::googlesql::ResolvedParameter>()),
            "$2");
  ASSERT_EQ(t.parameter_order().size(), 2u);
  EXPECT_TRUE(t.parameter_order()[0].name.empty());
  EXPECT_EQ(t.parameter_order()[0].position, 1);
  EXPECT_TRUE(t.parameter_order()[1].name.empty());
  EXPECT_EQ(t.parameter_order()[1].position, 2);
}

TEST_F(TranspilerTest, EmitLimitOffsetScanWithNamedParameter) {
  // `LIMIT @n OFFSET @n` exercises the parameter-in-LIMIT path *and*
  // named-parameter dedup inside a single scan emit: both LIMIT and
  // OFFSET resolve `@n` to `$1` and the accumulator records one
  // entry.
  ::googlesql::AnalyzerOptions options = MakeAnalyzerOptions();
  ASSERT_TRUE(options.AddQueryParameter("n", type_factory_->get_int64()).ok());
  const ::googlesql::ResolvedStatement* stmt = AnalyzeWith(
      "SELECT * FROM people ORDER BY id LIMIT @n OFFSET @n", options);
  const ::googlesql::ResolvedScan* scan = QueryInputScan(stmt);
  ASSERT_NE(scan, nullptr);
  ASSERT_EQ(scan->node_kind(), ::googlesql::RESOLVED_LIMIT_OFFSET_SCAN);
  TestTranspiler t;
  EXPECT_EQ(t.EmitLimitOffsetScan(
                scan->GetAs<::googlesql::ResolvedLimitOffsetScan>()),
            "SELECT * FROM (SELECT * FROM (SELECT \"id\", \"name\" FROM "
            "\"people\") ORDER BY \"id\" ASC NULLS FIRST) LIMIT $1 OFFSET $1");
  ASSERT_EQ(t.parameter_order().size(), 1u);
  EXPECT_EQ(t.parameter_order()[0].name, "n");
}

TEST_F(TranspilerTest, EmitParameterInsideFunctionArgument) {
  // Parameters thread through `EmitFunctionCall`'s argument loop
  // exactly like any other expression: `IFNULL(@s, 'x')` lowers to
  // `IFNULL($1, 'x')` and the parameter accumulator records the
  // single `@s` slot. `IFNULL` is on the function disposition table
  // so the surrounding emit composes fully.
  ::googlesql::AnalyzerOptions options = MakeAnalyzerOptions();
  ASSERT_TRUE(options.AddQueryParameter("s", type_factory_->get_string()).ok());
  const ::googlesql::ResolvedStatement* stmt =
      AnalyzeWith("SELECT IFNULL(@s, 'x') FROM people", options);
  const ::googlesql::ResolvedExpr* expr = QueryFirstSelectExpr(stmt);
  ASSERT_NE(expr, nullptr);
  ASSERT_EQ(expr->node_kind(), ::googlesql::RESOLVED_FUNCTION_CALL);
  TestTranspiler t;
  EXPECT_EQ(
      t.EmitFunctionCall(expr->GetAs<::googlesql::ResolvedFunctionCall>()),
      "IFNULL($1, 'x')");
  ASSERT_EQ(t.parameter_order().size(), 1u);
  EXPECT_EQ(t.parameter_order()[0].name, "s");
}

// --- Cast ---------------------------------------------------------------

TEST_F(TranspilerTest, EmitCastInt64ToString) {
  // `CAST(id AS STRING)` produces a `ResolvedCast` whose `expr` is
  // the column ref and whose target `Type` is STRING. The emit
  // composes both via `EmitColumnRef` + `ToDuckDBSqlType`, so the
  // result threads quoted-identifier and DuckDB type-name conventions
  // together.
  const ::googlesql::ResolvedStatement* stmt =
      Analyze("SELECT CAST(id AS STRING) FROM people");
  const ::googlesql::ResolvedExpr* expr = QueryFirstSelectExpr(stmt);
  ASSERT_NE(expr, nullptr);
  ASSERT_EQ(expr->node_kind(), ::googlesql::RESOLVED_CAST);
  TestTranspiler t;
  EXPECT_EQ(t.EmitCast(expr->GetAs<::googlesql::ResolvedCast>()),
            "CAST(\"id\" AS VARCHAR)");
}

TEST_F(TranspilerTest, EmitCastStringToInt64) {
  // CAST against a column ref of the right source type lands on the
  // expected DuckDB `BIGINT` (BQ INT64 -> DuckDB BIGINT, see
  // `types.cc`). The shape is symmetrical to the int->string case
  // above and pins the type-name mapping for INT64.
  const ::googlesql::ResolvedStatement* stmt =
      Analyze("SELECT CAST(name AS INT64) FROM people");
  const ::googlesql::ResolvedExpr* expr = QueryFirstSelectExpr(stmt);
  ASSERT_NE(expr, nullptr);
  ASSERT_EQ(expr->node_kind(), ::googlesql::RESOLVED_CAST);
  TestTranspiler t;
  EXPECT_EQ(t.EmitCast(expr->GetAs<::googlesql::ResolvedCast>()),
            "CAST(\"name\" AS BIGINT)");
}

TEST_F(TranspilerTest, EmitSafeCastUsesTryCast) {
  // `SAFE_CAST(<expr> AS T)` sets `return_null_on_error()` on the
  // ResolvedCast; we lower it to DuckDB's `TRY_CAST(...)` which
  // matches BigQuery's "return NULL on conversion failure" contract.
  const ::googlesql::ResolvedStatement* stmt =
      Analyze("SELECT SAFE_CAST(name AS INT64) FROM people");
  const ::googlesql::ResolvedExpr* expr = QueryFirstSelectExpr(stmt);
  ASSERT_NE(expr, nullptr);
  ASSERT_EQ(expr->node_kind(), ::googlesql::RESOLVED_CAST);
  TestTranspiler t;
  EXPECT_EQ(t.EmitCast(expr->GetAs<::googlesql::ResolvedCast>()),
            "TRY_CAST(\"name\" AS BIGINT)");
}

TEST_F(TranspilerTest, EmitCastNestedInsideFunctionCall) {
  // CAST nested inside another function call exercises the dispatch
  // path: `EmitFunctionCall` calls `EmitExpr` per argument, which
  // routes the cast through `EmitCast`. The full lower stays on the
  // DuckDB path because both COALESCE (disposition table) and CAST
  // (whitelisted target) are first-class.
  const ::googlesql::ResolvedStatement* stmt =
      Analyze("SELECT COALESCE(CAST(id AS STRING), 'x') FROM people");
  const ::googlesql::ResolvedExpr* expr = QueryFirstSelectExpr(stmt);
  ASSERT_NE(expr, nullptr);
  ASSERT_EQ(expr->node_kind(), ::googlesql::RESOLVED_FUNCTION_CALL);
  TestTranspiler t;
  EXPECT_EQ(
      t.EmitFunctionCall(expr->GetAs<::googlesql::ResolvedFunctionCall>()),
      "COALESCE(CAST(\"id\" AS VARCHAR), 'x')");
}

TEST_F(TranspilerTest, EmitCastArrayThreadsThroughColumnRef) {
  // ARRAY casts thread `ToDuckDBSqlType`'s recursive type expansion;
  // ARRAY<STRING> -> VARCHAR[] mirrors DuckDB's native list-of
  // syntax. We wrap a non-const expression (`[id]`) inside the cast
  // so the analyzer cannot constant-fold the whole expression onto
  // a `ResolvedLiteral` -- a folded array-of-int64 would skip the
  // ResolvedCast entirely and the test would fail with "expected
  // RESOLVED_CAST, got RESOLVED_LITERAL".
  const ::googlesql::ResolvedStatement* stmt =
      Analyze("SELECT CAST([id] AS ARRAY<STRING>) FROM people");
  const ::googlesql::ResolvedExpr* expr = QueryFirstSelectExpr(stmt);
  ASSERT_NE(expr, nullptr);
  ASSERT_EQ(expr->node_kind(), ::googlesql::RESOLVED_CAST);
  TestTranspiler t;
  // `[id]` is a non-const ARRAY constructor; it lowers through
  // `$make_array` to DuckDB's bracket syntax.
  EXPECT_EQ(t.EmitCast(expr->GetAs<::googlesql::ResolvedCast>()),
            "CAST([\"id\"] AS VARCHAR[])");
}

}  // namespace transpiler
}  // namespace duckdb
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
