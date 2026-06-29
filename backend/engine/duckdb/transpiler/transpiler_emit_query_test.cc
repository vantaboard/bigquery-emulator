#include "backend/engine/duckdb/transpiler/transpiler_test_fixture.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace duckdb {
namespace transpiler {

TEST_F(TranspilerTest, EmitSingleRowScanEmitsSelectOne) {
  // `SELECT 1` analyzes to a ResolvedProjectScan over a
  // ResolvedSingleRowScan. The single-row scan is the analyzer's
  // representation of "no FROM clause" -- a relation with one row
  // and no columns. We emit `SELECT 1` so the wrapping ProjectScan
  // can splice it into `FROM (<inner>)` like every other scan emit.
  const ::googlesql::ResolvedStatement* stmt = Analyze("SELECT 1");
  const ::googlesql::ResolvedScan* scan = QueryInputScan(stmt);
  ASSERT_NE(scan, nullptr);
  ASSERT_EQ(scan->node_kind(), ::googlesql::RESOLVED_SINGLE_ROW_SCAN);
  TestTranspiler t;
  EXPECT_EQ(
      t.EmitSingleRowScan(scan->GetAs<::googlesql::ResolvedSingleRowScan>()),
      "SELECT 1");
}

TEST_F(TranspilerTest, EmitComputedColumnLiteral) {
  // `SELECT 1` lands a ProjectScan whose `expr_list[0]` is a
  // ResolvedComputedColumn binding the `1` literal to the
  // synthesized output column. EmitComputedColumn lowers the bound
  // expression and adds `AS "<column-name>"`. We assert on the
  // analyzer's synthesized column name (`$col1`) so any drift in the
  // analyzer's auto-aliasing surfaces here rather than downstream.
  const ::googlesql::ResolvedStatement* stmt = Analyze("SELECT 1");
  ASSERT_NE(stmt, nullptr);
  const auto* q = stmt->GetAs<::googlesql::ResolvedQueryStmt>();
  ASSERT_NE(q, nullptr);
  const ::googlesql::ResolvedScan* scan = q->query();
  ASSERT_NE(scan, nullptr);
  ASSERT_EQ(scan->node_kind(), ::googlesql::RESOLVED_PROJECT_SCAN);
  const auto* project = scan->GetAs<::googlesql::ResolvedProjectScan>();
  ASSERT_GE(project->expr_list_size(), 1);
  const ::googlesql::ResolvedComputedColumn* cc = project->expr_list(0);
  ASSERT_NE(cc, nullptr);
  TestTranspiler t;
  EXPECT_EQ(t.EmitComputedColumn(cc), "1 AS \"$col1\"");
}

TEST_F(TranspilerTest, EmitComputedColumnFallsBackOnUnloweredExpr) {
  // Pick a function whose disposition has no DuckDB lowering so
  // its `EmitFunctionCall` returns "" -- the wrapping
  // EmitComputedColumn must propagate the empty-string fallback
  // contract rather than emit `<unset> AS "<col>"`.
  // `BIT_COUNT` is on the `semantic_executor` route in the YAML
  // disposition table (BQ flavor differs from DuckDB's `bit_count`).
  const ::googlesql::ResolvedStatement* stmt =
      Analyze("SELECT BIT_COUNT(id) FROM people");
  ASSERT_NE(stmt, nullptr);
  const auto* q = stmt->GetAs<::googlesql::ResolvedQueryStmt>();
  ASSERT_NE(q, nullptr);
  const ::googlesql::ResolvedScan* scan = q->query();
  ASSERT_NE(scan, nullptr);
  ASSERT_EQ(scan->node_kind(), ::googlesql::RESOLVED_PROJECT_SCAN);
  const auto* project = scan->GetAs<::googlesql::ResolvedProjectScan>();
  ASSERT_GE(project->expr_list_size(), 1);
  const ::googlesql::ResolvedComputedColumn* cc = project->expr_list(0);
  ASSERT_NE(cc, nullptr);
  TestTranspiler t;
  EXPECT_EQ(t.EmitComputedColumn(cc), "");
}

TEST_F(TranspilerTest, EmitProjectScanSelectLiteral) {
  // The full ProjectScan emit for `SELECT 1` threads:
  //   * EmitSingleRowScan -> "SELECT 1"
  //   * EmitComputedColumn -> "1 AS \"$col1\""
  // and stitches them as `SELECT <projection> FROM (<inner>)`.
  const ::googlesql::ResolvedStatement* stmt = Analyze("SELECT 1");
  ASSERT_NE(stmt, nullptr);
  const auto* q = stmt->GetAs<::googlesql::ResolvedQueryStmt>();
  ASSERT_NE(q, nullptr);
  const ::googlesql::ResolvedScan* scan = q->query();
  ASSERT_NE(scan, nullptr);
  ASSERT_EQ(scan->node_kind(), ::googlesql::RESOLVED_PROJECT_SCAN);
  TestTranspiler t;
  EXPECT_EQ(t.EmitProjectScan(scan->GetAs<::googlesql::ResolvedProjectScan>()),
            "SELECT 1 AS \"$col1\" FROM (SELECT 1)");
}

TEST_F(TranspilerTest, EmitProjectScanElidesNoOpPermutation) {
  // For `SELECT name, id FROM people` the analyzer wraps the
  // TableScan in a no-op ProjectScan: `expr_list` is empty and
  // `column_list` is a permutation of the input scan's column list
  // by column id. The emit should drop the wrap and return the inner
  // TableScan SQL directly so the outer `EmitQueryStmt`'s projection
  // is the only one that does the reordering -- otherwise we stack
  // `SELECT "name", "id" FROM (SELECT "id", "name" ...)` redundantly
  // on top of the TableScan emit. Same applies to identity-only
  // projections (`SELECT id, name FROM people`) and analyzer-pruned
  // shapes where `column_list` is a single-column subset of the
  // table's columns; both reduce to the inner TableScan SQL.
  const ::googlesql::ResolvedStatement* stmt =
      Analyze("SELECT name, id FROM people");
  ASSERT_NE(stmt, nullptr);
  const auto* q = stmt->GetAs<::googlesql::ResolvedQueryStmt>();
  ASSERT_NE(q, nullptr);
  const ::googlesql::ResolvedScan* scan = q->query();
  ASSERT_NE(scan, nullptr);
  ASSERT_EQ(scan->node_kind(), ::googlesql::RESOLVED_PROJECT_SCAN);
  const auto* project = scan->GetAs<::googlesql::ResolvedProjectScan>();
  ASSERT_EQ(project->expr_list_size(), 0);
  TestTranspiler t;
  // Emit must equal the *input* TableScan's emit, not a wrapping
  // SELECT around it.
  EXPECT_EQ(t.EmitProjectScan(project),
            "SELECT \"id\", \"name\" FROM \"people\"");
}

TEST_F(TranspilerTest, EmitOutputColumnCollapsesAliasWhenNamesMatch) {
  // For `SELECT 1` the output column's user-visible name and the
  // physical column's name both resolve to `$col1`, so the alias
  // collapses to just `"$col1"` -- DuckDB carries the column name
  // straight through the outermost SELECT.
  const ::googlesql::ResolvedStatement* stmt = Analyze("SELECT 1");
  ASSERT_NE(stmt, nullptr);
  const auto* q = stmt->GetAs<::googlesql::ResolvedQueryStmt>();
  ASSERT_NE(q, nullptr);
  ASSERT_EQ(q->output_column_list_size(), 1);
  TestTranspiler t;
  EXPECT_EQ(t.EmitOutputColumn(q->output_column_list(0)), "\"$col1\"");
}

TEST_F(TranspilerTest, EmitOutputColumnEmitsAliasWhenNamesDiffer) {
  // `SELECT id AS user_id FROM people` lands an output column whose
  // user-visible name (`user_id`) differs from the physical column
  // name (`id`); the emit must surface both as `"id" AS "user_id"`.
  const ::googlesql::ResolvedStatement* stmt =
      Analyze("SELECT id AS user_id FROM people");
  ASSERT_NE(stmt, nullptr);
  const auto* q = stmt->GetAs<::googlesql::ResolvedQueryStmt>();
  ASSERT_NE(q, nullptr);
  ASSERT_EQ(q->output_column_list_size(), 1);
  TestTranspiler t;
  EXPECT_EQ(t.EmitOutputColumn(q->output_column_list(0)),
            "\"id\" AS \"user_id\"");
}

TEST_F(TranspilerTest, EmitQueryStmtSelectLiteral) {
  // End-to-end for `SELECT 1`: the analyzer wraps a ProjectScan
  // around a SingleRowScan and the QueryStmt's output_column_list
  // carries the synthesized `$col1` alias. The emit wires
  // EmitProjectScan + EmitOutputColumn into the final SQL.
  const ::googlesql::ResolvedStatement* stmt = Analyze("SELECT 1");
  ASSERT_NE(stmt, nullptr);
  ASSERT_EQ(stmt->node_kind(), ::googlesql::RESOLVED_QUERY_STMT);
  TestTranspiler t;
  EXPECT_EQ(t.EmitQueryStmt(stmt->GetAs<::googlesql::ResolvedQueryStmt>()),
            "SELECT \"$col1\" FROM (SELECT 1 AS \"$col1\" FROM (SELECT 1))");
}

TEST_F(TranspilerTest, EmitQueryStmtSelectLiteralWithExplicitAlias) {
  // `SELECT 1 AS x` rebinds the synthesized column id to the
  // user-spelled alias; both `output_column_list[0].name()` and the
  // column's `name()` resolve to `x`, so the AS alias collapses on
  // the outermost SELECT.
  const ::googlesql::ResolvedStatement* stmt = Analyze("SELECT 1 AS x");
  ASSERT_NE(stmt, nullptr);
  ASSERT_EQ(stmt->node_kind(), ::googlesql::RESOLVED_QUERY_STMT);
  TestTranspiler t;
  EXPECT_EQ(t.EmitQueryStmt(stmt->GetAs<::googlesql::ResolvedQueryStmt>()),
            "SELECT \"x\" FROM (SELECT 1 AS \"x\" FROM (SELECT 1))");
}

TEST_F(TranspilerTest, EmitQueryStmtTableProjectionPreservesColumnOrder) {
  // `SELECT id, name FROM people` should round-trip with both
  // columns in their declared order. The analyzer collapses this
  // straight onto the TableScan (no wrapping ProjectScan because
  // the projection matches the table's column list 1:1) and the
  // QueryStmt mapping just renames each column to itself.
  const ::googlesql::ResolvedStatement* stmt =
      Analyze("SELECT id, name FROM people");
  ASSERT_NE(stmt, nullptr);
  ASSERT_EQ(stmt->node_kind(), ::googlesql::RESOLVED_QUERY_STMT);
  TestTranspiler t;
  EXPECT_EQ(t.EmitQueryStmt(stmt->GetAs<::googlesql::ResolvedQueryStmt>()),
            "SELECT \"id\", \"name\" FROM (SELECT \"id\", \"name\" "
            "FROM \"people\")");
}

TEST_F(TranspilerTest, EmitQueryStmtExpressionProjection) {
  // A non-trivial projection (`COALESCE(name, 'unknown') AS n`)
  // forces the analyzer to wrap the TableScan in a ProjectScan
  // whose `expr_list` carries the ComputedColumn binding. The
  // outermost SELECT then projects the synthesized column under the
  // user-spelled alias.
  const ::googlesql::ResolvedStatement* stmt =
      Analyze("SELECT COALESCE(name, 'unknown') AS n FROM people");
  ASSERT_NE(stmt, nullptr);
  ASSERT_EQ(stmt->node_kind(), ::googlesql::RESOLVED_QUERY_STMT);
  TestTranspiler t;
  EXPECT_EQ(t.EmitQueryStmt(stmt->GetAs<::googlesql::ResolvedQueryStmt>()),
            "SELECT \"n\" FROM (SELECT COALESCE(\"name\", 'unknown') AS "
            "\"n\" FROM (SELECT \"id\", \"name\" FROM \"people\"))");
}

TEST_F(TranspilerTest, EmitQueryStmtReorderedOutputColumns) {
  // `SELECT name, id FROM people` reorders the table's column list.
  // The analyzer wraps the TableScan in a ProjectScan that mirrors
  // the table's storage order in `column_list` but the QueryStmt's
  // `output_column_list` reflects the user-spelled order, so the
  // outermost SELECT projects `name` before `id`.
  const ::googlesql::ResolvedStatement* stmt =
      Analyze("SELECT name, id FROM people");
  ASSERT_NE(stmt, nullptr);
  ASSERT_EQ(stmt->node_kind(), ::googlesql::RESOLVED_QUERY_STMT);
  TestTranspiler t;
  EXPECT_EQ(t.EmitQueryStmt(stmt->GetAs<::googlesql::ResolvedQueryStmt>()),
            "SELECT \"name\", \"id\" FROM (SELECT \"id\", \"name\" "
            "FROM \"people\")");
}

TEST_F(TranspilerTest, EmitQueryStmtAliasedColumnSurfacesAlias) {
  // `SELECT id AS user_id FROM people` keeps the physical column as
  // `id` inside the inner scan but renames it to `user_id` on the
  // outermost SELECT. The projection carries `<col> AS <alias>` so
  // the wire-side schema matches the user's spelling. The analyzer
  // does not prune the underlying TableScan's column_list down to
  // `[id]` here -- it keeps the full `[id, name]` table column list
  // and lets the wrapping ProjectScan narrow to `[id]`. The
  // EmitProjectScan no-op elision deliberately skips narrowing
  // layers (`column_list` is a strict subset of input, sizes
  // differ), so the wrap that drops `name` survives and the inner
  // emit shows three nested SELECTs.
  const ::googlesql::ResolvedStatement* stmt =
      Analyze("SELECT id AS user_id FROM people");
  ASSERT_NE(stmt, nullptr);
  ASSERT_EQ(stmt->node_kind(), ::googlesql::RESOLVED_QUERY_STMT);
  TestTranspiler t;
  EXPECT_EQ(t.EmitQueryStmt(stmt->GetAs<::googlesql::ResolvedQueryStmt>()),
            "SELECT \"id\" AS \"user_id\" FROM (SELECT \"id\" "
            "FROM (SELECT \"id\", \"name\" FROM \"people\"))");
}

TEST_F(TranspilerTest, EmitQueryStmtFallsBackOnUnloweredProjection) {
  // `BIT_COUNT(id)` is on the `semantic_executor` route in the YAML
  // disposition table; the inner ProjectScan emit returns "" and
  // EmitQueryStmt propagates the empty-string fallback contract
  // instead of stitching an outer SELECT around a missing inner
  // relation.
  const ::googlesql::ResolvedStatement* stmt =
      Analyze("SELECT BIT_COUNT(id) FROM people");
  ASSERT_NE(stmt, nullptr);
  ASSERT_EQ(stmt->node_kind(), ::googlesql::RESOLVED_QUERY_STMT);
  TestTranspiler t;
  EXPECT_EQ(t.EmitQueryStmt(stmt->GetAs<::googlesql::ResolvedQueryStmt>()), "");
}

// --- Parameters ---------------------------------------------------------

}  // namespace transpiler
}  // namespace duckdb
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
