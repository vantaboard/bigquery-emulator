#include <gtest/gtest.h>

#include "backend/engine/duckdb/transpiler/transpiler_test_fixture.h"
#include "googlesql/resolved_ast/resolved_ast.h"
#include "googlesql/resolved_ast/resolved_node_kind.pb.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace duckdb {
namespace transpiler {

TEST_F(TranspilerTest, EmitLiteralInt64) {
  const ::googlesql::ResolvedStatement* stmt = Analyze("SELECT 42 AS n");
  const ::googlesql::ResolvedExpr* expr = QueryFirstSelectExpr(stmt);
  ASSERT_NE(expr, nullptr);
  ASSERT_EQ(expr->node_kind(), ::googlesql::RESOLVED_LITERAL);
  TestTranspiler t;
  EXPECT_EQ(t.EmitLiteral(expr->GetAs<::googlesql::ResolvedLiteral>()), "42");
}

TEST_F(TranspilerTest, EmitLiteralString) {
  const ::googlesql::ResolvedStatement* stmt = Analyze("SELECT 'hi' AS s");
  const ::googlesql::ResolvedExpr* expr = QueryFirstSelectExpr(stmt);
  ASSERT_NE(expr, nullptr);
  ASSERT_EQ(expr->node_kind(), ::googlesql::RESOLVED_LITERAL);
  TestTranspiler t;
  // We must emit single-quoted strings: DuckDB reads double-quoted
  // text as an *identifier*, so a `"hi"` literal would be a column
  // reference rather than a string. EmitLiteral overrides
  // GoogleSQL's default double-quoted form on TYPE_STRING for
  // exactly this reason -- otherwise every string-bearing query
  // would surface UNIMPLEMENTED instead of a real result.
  EXPECT_EQ(t.EmitLiteral(expr->GetAs<::googlesql::ResolvedLiteral>()), "'hi'");
}

TEST_F(TranspilerTest, EmitLiteralBoolTrue) {
  const ::googlesql::ResolvedStatement* stmt = Analyze("SELECT TRUE AS b");
  const ::googlesql::ResolvedExpr* expr = QueryFirstSelectExpr(stmt);
  ASSERT_NE(expr, nullptr);
  ASSERT_EQ(expr->node_kind(), ::googlesql::RESOLVED_LITERAL);
  TestTranspiler t;
  EXPECT_EQ(t.EmitLiteral(expr->GetAs<::googlesql::ResolvedLiteral>()), "true");
}

TEST_F(TranspilerTest, EmitColumnRefQuotesIdentifier) {
  // The analyzer collapses a bare `SELECT id FROM people` straight
  // onto the TableScan (no wrapping ProjectScan), so to land a
  // standalone `ResolvedColumnRef` we wrap the column in a
  // function call. `COALESCE(id, 0)` keeps the test focused: the
  // first argument is the ColumnRef we want to assert on.
  const ::googlesql::ResolvedStatement* stmt =
      Analyze("SELECT COALESCE(id, 0) FROM people");
  const ::googlesql::ResolvedExpr* expr = QueryFirstSelectExpr(stmt);
  ASSERT_NE(expr, nullptr);
  ASSERT_EQ(expr->node_kind(), ::googlesql::RESOLVED_FUNCTION_CALL);
  const ::googlesql::ResolvedFunctionCall* call =
      expr->GetAs<::googlesql::ResolvedFunctionCall>();
  ASSERT_GE(call->argument_list_size(), 1);
  const ::googlesql::ResolvedExpr* arg = call->argument_list(0);
  ASSERT_NE(arg, nullptr);
  ASSERT_EQ(arg->node_kind(), ::googlesql::RESOLVED_COLUMN_REF);
  TestTranspiler t;
  EXPECT_EQ(t.EmitColumnRef(arg->GetAs<::googlesql::ResolvedColumnRef>()),
            "\"id\"");
}

TEST_F(TranspilerTest, EmitFunctionCallCoalesce) {
  const ::googlesql::ResolvedStatement* stmt =
      Analyze("SELECT COALESCE(name, 'unknown') AS n FROM people");
  const ::googlesql::ResolvedExpr* expr = QueryFirstSelectExpr(stmt);
  ASSERT_NE(expr, nullptr);
  ASSERT_EQ(expr->node_kind(), ::googlesql::RESOLVED_FUNCTION_CALL);
  TestTranspiler t;
  EXPECT_EQ(
      t.EmitFunctionCall(expr->GetAs<::googlesql::ResolvedFunctionCall>()),
      "COALESCE(\"name\", 'unknown')");
}

TEST_F(TranspilerTest, EmitFunctionCallIfnull) {
  const ::googlesql::ResolvedStatement* stmt =
      Analyze("SELECT IFNULL(name, 'unknown') AS n FROM people");
  const ::googlesql::ResolvedExpr* expr = QueryFirstSelectExpr(stmt);
  ASSERT_NE(expr, nullptr);
  ASSERT_EQ(expr->node_kind(), ::googlesql::RESOLVED_FUNCTION_CALL);
  TestTranspiler t;
  EXPECT_EQ(
      t.EmitFunctionCall(expr->GetAs<::googlesql::ResolvedFunctionCall>()),
      "IFNULL(\"name\", 'unknown')");
}

TEST_F(TranspilerTest, EmitFunctionCallNonLoweringDispositionReturnsEmpty) {
  // `BIT_COUNT` is on the `semantic_executor` route in
  // `functions.yaml` (BQ flavor differs from DuckDB's `bit_count`)
  // with the body deferred to `docs/ENGINE_POLICY.md`.
  // The transpiler has no DuckDB lowering for this disposition, so
  // the emit returns "" and the engine surfaces UNIMPLEMENTED for
  // the whole query.
  const ::googlesql::ResolvedStatement* stmt =
      Analyze("SELECT BIT_COUNT(id) AS n FROM people");
  const ::googlesql::ResolvedExpr* expr = QueryFirstSelectExpr(stmt);
  ASSERT_NE(expr, nullptr);
  ASSERT_EQ(expr->node_kind(), ::googlesql::RESOLVED_FUNCTION_CALL);
  TestTranspiler t;
  EXPECT_EQ(
      t.EmitFunctionCall(expr->GetAs<::googlesql::ResolvedFunctionCall>()), "");
}

TEST_F(TranspilerTest, EmitFunctionCallMappedFunction) {
  // Disposition-table-backed scalars: `ABS(id)` lowers to DuckDB's
  // `ABS(...)`. The casing of the emitted function name comes from
  // the YAML disposition (we render the duckdb_name verbatim); the
  // BQ-side `abs` lookup is case-insensitive.
  const ::googlesql::ResolvedStatement* stmt =
      Analyze("SELECT ABS(id) AS n FROM people");
  const ::googlesql::ResolvedExpr* expr = QueryFirstSelectExpr(stmt);
  ASSERT_NE(expr, nullptr);
  ASSERT_EQ(expr->node_kind(), ::googlesql::RESOLVED_FUNCTION_CALL);
  TestTranspiler t;
  EXPECT_EQ(
      t.EmitFunctionCall(expr->GetAs<::googlesql::ResolvedFunctionCall>()),
      "ABS(\"id\")");
}

TEST_F(TranspilerTest, EmitFunctionCallLengthMaps) {
  // `LENGTH(name)` -> `LENGTH("name")`. Two-arg variants don't exist
  // for LENGTH in either dialect; the single-arg shape is the entire
  // disposition surface.
  const ::googlesql::ResolvedStatement* stmt =
      Analyze("SELECT LENGTH(name) AS n FROM people");
  const ::googlesql::ResolvedExpr* expr = QueryFirstSelectExpr(stmt);
  ASSERT_NE(expr, nullptr);
  ASSERT_EQ(expr->node_kind(), ::googlesql::RESOLVED_FUNCTION_CALL);
  TestTranspiler t;
  EXPECT_EQ(
      t.EmitFunctionCall(expr->GetAs<::googlesql::ResolvedFunctionCall>()),
      "LENGTH(\"name\")");
}

TEST_F(TranspilerTest, EmitFunctionCallReadyDuckdbUdf) {
  // Ready `duckdb_udf` rows emit identically to `duckdb_native`:
  // the transpiler renders `<duckdb_name>(<args>)` and DuckDB
  // resolves the call to the registered polyfill macro. `MOD`
  // flipped to ready in the numeric-family commit; the row carries
  // `duckdb_name=bq_mod`.
  const ::googlesql::ResolvedStatement* stmt =
      Analyze("SELECT MOD(id, 3) AS m FROM people");
  const ::googlesql::ResolvedExpr* expr = QueryFirstSelectExpr(stmt);
  ASSERT_NE(expr, nullptr);
  ASSERT_EQ(expr->node_kind(), ::googlesql::RESOLVED_FUNCTION_CALL);
  TestTranspiler t;
  EXPECT_EQ(
      t.EmitFunctionCall(expr->GetAs<::googlesql::ResolvedFunctionCall>()),
      "bq_mod(\"id\", 3)");
}

TEST_F(TranspilerTest, EmitFunctionCallSafeModeReturnsEmpty) {
  // SAFE.<fn>(...) sets `error_mode = SAFE_ERROR_MODE`. DuckDB has no
  // native SAFE analog yet, so the emit short-circuits to "" before
  // consulting the disposition table -- this would otherwise emit
  // ABS("id") and silently lose the SAFE error semantics.
  const ::googlesql::ResolvedStatement* stmt =
      Analyze("SELECT SAFE.ABS(id) AS n FROM people");
  const ::googlesql::ResolvedExpr* expr = QueryFirstSelectExpr(stmt);
  ASSERT_NE(expr, nullptr);
  ASSERT_EQ(expr->node_kind(), ::googlesql::RESOLVED_FUNCTION_CALL);
  TestTranspiler t;
  EXPECT_EQ(
      t.EmitFunctionCall(expr->GetAs<::googlesql::ResolvedFunctionCall>()), "");
}

}  // namespace transpiler
}  // namespace duckdb
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
