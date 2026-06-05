#include "backend/engine/duckdb/transpiler/transpiler_test_fixture.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace duckdb {
namespace transpiler {

TEST_F(TranspilerTest, EmitSetOperationScanUnionAll) {
  // BigQuery `<lhs> UNION ALL <rhs>` analyzes to a
  // `ResolvedSetOperationScan` whose `op_type=UNION_ALL`,
  // `column_match_mode=BY_POSITION`, and two
  // `ResolvedSetOperationItem`s. GoogleSQL takes the parent
  // column's name from the leftmost input's column (`id` here), so
  // the LHS item's projection collapses (`"id"` -> `"id"`) and the
  // RHS item renames `order_id` to `id` to land on the parent's
  // column name.
  //
  // The analyzer wraps each `SELECT <col> FROM <table>` arm in a
  // ResolvedProjectScan over the ResolvedTableScan (because the
  // selected columns are a subset of the table's full column list),
  // which is why the per-arm SQL has the extra `(SELECT ... FROM
  // (SELECT ... FROM ...))` nesting -- the outer SELECT is the
  // set-op item's projection, the middle SELECT is the analyzer's
  // ProjectScan, and the inner SELECT is the TableScan.
  const ::googlesql::ResolvedStatement* stmt =
      Analyze("SELECT id FROM people UNION ALL SELECT order_id FROM orders");
  ASSERT_NE(stmt, nullptr);
  const ::googlesql::ResolvedScan* scan = QueryInputScan(stmt);
  ASSERT_NE(scan, nullptr);
  ASSERT_EQ(scan->node_kind(), ::googlesql::RESOLVED_SET_OPERATION_SCAN);
  TestTranspiler t;
  EXPECT_EQ(t.EmitSetOperationScan(
                scan->GetAs<::googlesql::ResolvedSetOperationScan>()),
            "SELECT \"id\" FROM (SELECT *, 1 AS \"__bq_union_ord\" FROM "
            "(SELECT \"id\" FROM (SELECT \"id\" FROM (SELECT \"id\", "
            "\"name\" FROM \"people\"))) UNION ALL SELECT *, 2 AS "
            "\"__bq_union_ord\" FROM (SELECT \"order_id\" AS \"id\" FROM "
            "(SELECT \"order_id\" FROM (SELECT \"order_id\", \"amount\" FROM "
            "\"orders\")))) ORDER BY \"__bq_union_ord\"");
}

TEST_F(TranspilerTest, EmitSetOperationScanUnionDistinct) {
  // `UNION DISTINCT` lowers to DuckDB's bare `UNION` (DuckDB's
  // default duplicate-handling on `UNION` is DISTINCT, matching the
  // BigQuery semantics). The per-item projection shape is the same
  // as UNION ALL because the duplicate-handling is the only
  // difference between the two ops.
  const ::googlesql::ResolvedStatement* stmt = Analyze(
      "SELECT id FROM people UNION DISTINCT SELECT order_id FROM orders");
  ASSERT_NE(stmt, nullptr);
  const ::googlesql::ResolvedScan* scan = QueryInputScan(stmt);
  ASSERT_NE(scan, nullptr);
  ASSERT_EQ(scan->node_kind(), ::googlesql::RESOLVED_SET_OPERATION_SCAN);
  TestTranspiler t;
  EXPECT_EQ(t.EmitSetOperationScan(
                scan->GetAs<::googlesql::ResolvedSetOperationScan>()),
            "SELECT \"id\" FROM (SELECT \"id\" FROM (SELECT \"id\", \"name\" "
            "FROM \"people\"))"
            " UNION "
            "SELECT \"order_id\" AS \"id\" FROM (SELECT \"order_id\" FROM "
            "(SELECT \"order_id\", \"amount\" FROM \"orders\"))");
}

TEST_F(TranspilerTest, EmitSetOperationScanIntersectDistinct) {
  // `INTERSECT DISTINCT` lowers to DuckDB's bare `INTERSECT` (also
  // DISTINCT by default). Same item shape as UNION; only the
  // keyword between items changes. The output column name comes
  // from the leftmost input's column.
  const ::googlesql::ResolvedStatement* stmt = Analyze(
      "SELECT id FROM people INTERSECT DISTINCT SELECT order_id FROM orders");
  ASSERT_NE(stmt, nullptr);
  const ::googlesql::ResolvedScan* scan = QueryInputScan(stmt);
  ASSERT_NE(scan, nullptr);
  ASSERT_EQ(scan->node_kind(), ::googlesql::RESOLVED_SET_OPERATION_SCAN);
  TestTranspiler t;
  EXPECT_EQ(t.EmitSetOperationScan(
                scan->GetAs<::googlesql::ResolvedSetOperationScan>()),
            "SELECT \"id\" FROM (SELECT \"id\" FROM (SELECT \"id\", \"name\" "
            "FROM \"people\"))"
            " INTERSECT "
            "SELECT \"order_id\" AS \"id\" FROM (SELECT \"order_id\" FROM "
            "(SELECT \"order_id\", \"amount\" FROM \"orders\"))");
}

TEST_F(TranspilerTest, EmitSetOperationScanExceptDistinct) {
  // `EXCEPT DISTINCT` lowers to DuckDB's bare `EXCEPT` (DISTINCT by
  // default). BigQuery's EXCEPT DISTINCT semantics (row R in LHS at
  // least once and absent from RHS) match DuckDB's bag-difference
  // followed by DISTINCT.
  const ::googlesql::ResolvedStatement* stmt = Analyze(
      "SELECT id FROM people EXCEPT DISTINCT SELECT order_id FROM orders");
  ASSERT_NE(stmt, nullptr);
  const ::googlesql::ResolvedScan* scan = QueryInputScan(stmt);
  ASSERT_NE(scan, nullptr);
  ASSERT_EQ(scan->node_kind(), ::googlesql::RESOLVED_SET_OPERATION_SCAN);
  TestTranspiler t;
  EXPECT_EQ(t.EmitSetOperationScan(
                scan->GetAs<::googlesql::ResolvedSetOperationScan>()),
            "SELECT * FROM (SELECT \"id\" FROM (SELECT \"id\" FROM "
            "(SELECT \"id\", \"name\" FROM \"people\")) EXCEPT SELECT "
            "\"order_id\" AS \"id\" FROM (SELECT \"order_id\" FROM "
            "(SELECT \"order_id\", \"amount\" FROM \"orders\"))) ORDER BY 1");
}

TEST_F(TranspilerTest, EmitSetOperationScanIdenticalArmsBothCollapse) {
  // When both arms expose the same column name as the parent's
  // output column, the per-item AS aliases collapse on both sides.
  // Identical-arm `SELECT id FROM people UNION ALL SELECT id FROM
  // people` is the smallest input that exercises the both-side
  // collapse path -- both items project `"id"` onto the parent's
  // `id` column, so neither projection needs an AS keyword. This
  // pins the symmetric-collapse path that the per-arm-rename tests
  // above leave only half-covered.
  const ::googlesql::ResolvedStatement* stmt =
      Analyze("SELECT id FROM people UNION ALL SELECT id FROM people");
  ASSERT_NE(stmt, nullptr);
  const ::googlesql::ResolvedScan* scan = QueryInputScan(stmt);
  ASSERT_NE(scan, nullptr);
  ASSERT_EQ(scan->node_kind(), ::googlesql::RESOLVED_SET_OPERATION_SCAN);
  TestTranspiler t;
  EXPECT_EQ(t.EmitSetOperationScan(
                scan->GetAs<::googlesql::ResolvedSetOperationScan>()),
            "SELECT \"id\" FROM (SELECT *, 1 AS \"__bq_union_ord\" FROM "
            "(SELECT \"id\" FROM (SELECT \"id\" FROM (SELECT \"id\", "
            "\"name\" FROM \"people\"))) UNION ALL SELECT *, 2 AS "
            "\"__bq_union_ord\" FROM (SELECT \"id\" FROM (SELECT \"id\" FROM "
            "(SELECT \"id\", \"name\" FROM \"people\")))) ORDER BY "
            "\"__bq_union_ord\"");
}

TEST_F(TranspilerTest, EmitSetOperationScanMultiColumnPreservesOrder) {
  // Two-column UNION ALL. The LHS exposes `id, name`; the RHS
  // (`SELECT order_id, CAST(amount AS STRING) FROM orders`)
  // renames both columns to land on the LHS-named output columns
  // (`id`, `name`). The test pins that the per-item projections
  // honor positional column matching even when each column needs a
  // different rename direction.
  //
  // The analyzer assigns column IDs across the whole query and
  // hands the synthesized computed-column name through them; the
  // CAST in the RHS lands as `$col2` (slot 2 in the overall
  // computed-column ordering), not `$col1`. The set-op item
  // renames both onto the parent's `id` / `name`.
  const ::googlesql::ResolvedStatement* stmt = Analyze(
      "SELECT id, name FROM people UNION ALL "
      "SELECT order_id, CAST(amount AS STRING) FROM orders");
  ASSERT_NE(stmt, nullptr);
  const ::googlesql::ResolvedScan* scan = QueryInputScan(stmt);
  ASSERT_NE(scan, nullptr);
  ASSERT_EQ(scan->node_kind(), ::googlesql::RESOLVED_SET_OPERATION_SCAN);
  TestTranspiler t;
  EXPECT_EQ(
      t.EmitSetOperationScan(
          scan->GetAs<::googlesql::ResolvedSetOperationScan>()),
      "SELECT \"id\", \"name\" FROM (SELECT *, 1 AS \"__bq_union_ord\" FROM "
      "(SELECT \"id\", \"name\" FROM (SELECT \"id\", \"name\" FROM "
      "\"people\")) UNION ALL SELECT *, 2 AS \"__bq_union_ord\" FROM "
      "(SELECT \"order_id\" AS \"id\", \"$col2\" AS \"name\" FROM (SELECT "
      "\"order_id\", CAST(\"amount\" AS VARCHAR) AS \"$col2\" FROM (SELECT "
      "\"order_id\", \"amount\" FROM \"orders\")))) ORDER BY "
      "\"__bq_union_ord\"");
}

TEST_F(TranspilerTest, EmitSetOperationScanThreeArmFlattening) {
  // BigQuery / GoogleSQL flattens same-op chains so a three-way
  // `UNION ALL` lands as a single `ResolvedSetOperationScan` with
  // three items, not a tree. The emit joins all three items with
  // the keyword.
  const ::googlesql::ResolvedStatement* stmt = Analyze(
      "SELECT id FROM people UNION ALL SELECT order_id FROM orders "
      "UNION ALL SELECT amount FROM orders");
  ASSERT_NE(stmt, nullptr);
  const ::googlesql::ResolvedScan* scan = QueryInputScan(stmt);
  ASSERT_NE(scan, nullptr);
  ASSERT_EQ(scan->node_kind(), ::googlesql::RESOLVED_SET_OPERATION_SCAN);
  const auto* set_op = scan->GetAs<::googlesql::ResolvedSetOperationScan>();
  ASSERT_EQ(set_op->input_item_list_size(), 3);
  TestTranspiler t;
  EXPECT_EQ(t.EmitSetOperationScan(set_op),
            "SELECT \"id\" FROM (SELECT *, 1 AS \"__bq_union_ord\" FROM "
            "(SELECT \"id\" FROM (SELECT \"id\" FROM (SELECT \"id\", "
            "\"name\" FROM \"people\"))) UNION ALL SELECT *, 2 AS "
            "\"__bq_union_ord\" FROM (SELECT \"order_id\" AS \"id\" FROM "
            "(SELECT \"order_id\" FROM (SELECT \"order_id\", \"amount\" FROM "
            "\"orders\"))) UNION ALL SELECT *, 3 AS \"__bq_union_ord\" FROM "
            "(SELECT \"amount\" AS \"id\" FROM (SELECT \"amount\" FROM "
            "(SELECT \"order_id\", \"amount\" FROM \"orders\")))) ORDER BY "
            "\"__bq_union_ord\"");
}

TEST_F(TranspilerTest, EmitSetOperationScanNestedDifferentOps) {
  // Mixing operators (UNION ALL outside, INTERSECT DISTINCT inside)
  // forces the analyzer to nest: the outer UNION ALL has one
  // TableScan-y item plus one SetOperationScan item. The emit
  // composes recursively -- each item's child scan goes through
  // `EmitScan`, which dispatches back to `EmitSetOperationScan`
  // for the inner set-op.
  const ::googlesql::ResolvedStatement* stmt = Analyze(
      "SELECT id FROM people UNION ALL "
      "(SELECT order_id FROM orders INTERSECT DISTINCT "
      "SELECT amount FROM orders)");
  ASSERT_NE(stmt, nullptr);
  const ::googlesql::ResolvedScan* scan = QueryInputScan(stmt);
  ASSERT_NE(scan, nullptr);
  ASSERT_EQ(scan->node_kind(), ::googlesql::RESOLVED_SET_OPERATION_SCAN);
  TestTranspiler t;
  // The inner INTERSECT's parent column name is `order_id` (the
  // leftmost input's column), and the outer UNION ALL renames it
  // onto its own parent column `id` for the second arm.
  EXPECT_EQ(t.EmitSetOperationScan(
                scan->GetAs<::googlesql::ResolvedSetOperationScan>()),
            "SELECT \"id\" FROM (SELECT *, 1 AS \"__bq_union_ord\" FROM "
            "(SELECT \"id\" FROM (SELECT \"id\" FROM (SELECT \"id\", "
            "\"name\" FROM \"people\"))) UNION ALL SELECT *, 2 AS "
            "\"__bq_union_ord\" FROM (SELECT \"order_id\" AS \"id\" FROM "
            "(SELECT \"order_id\" FROM (SELECT \"order_id\" FROM (SELECT "
            "\"order_id\", \"amount\" FROM \"orders\")) INTERSECT SELECT "
            "\"amount\" AS \"order_id\" FROM (SELECT \"amount\" FROM (SELECT "
            "\"order_id\", \"amount\" FROM \"orders\"))))) ORDER BY "
            "\"__bq_union_ord\"");
}

TEST_F(TranspilerTest, EmitSetOperationScanFallsBackOnUnloweredChild) {
  // If any child scan returns "" the whole set-op emit must
  // propagate the empty string. `BIT_COUNT` is on the
  // `semantic_executor` route in the YAML disposition table so the
  // right-hand ProjectScan's computed column emit returns "" ->
  // ProjectScan returns "" -> set-op item returns "" -> set-op
  // scan returns "".
  const ::googlesql::ResolvedStatement* stmt = Analyze(
      "SELECT id FROM people UNION ALL SELECT BIT_COUNT(amount) FROM orders");
  ASSERT_NE(stmt, nullptr);
  const ::googlesql::ResolvedScan* scan = QueryInputScan(stmt);
  ASSERT_NE(scan, nullptr);
  ASSERT_EQ(scan->node_kind(), ::googlesql::RESOLVED_SET_OPERATION_SCAN);
  TestTranspiler t;
  EXPECT_EQ(t.EmitSetOperationScan(
                scan->GetAs<::googlesql::ResolvedSetOperationScan>()),
            "");
}

TEST_F(TranspilerTest, EmitSetOperationScanUnionAllDuplicateBehaviorContrast) {
  // Execution-style contrast between UNION ALL and UNION DISTINCT
  // on the *same* input shape. We assert on the SQL strings (this
  // fixture does not have a running DuckDB connection) so a
  // regression in either keyword choice surfaces here. The
  // expected duplicate behavior is documented in
  // `ResolvedSetOperationScan`'s comment block in
  // `resolved_ast.h` (UNION ALL keeps all rows, UNION DISTINCT
  // dedupes); DuckDB's `UNION ALL` and `UNION` (DISTINCT by
  // default) match that contract.
  //
  // We compute each side's SQL fully and discard the analyzer's
  // output before re-`Analyze`-ing for the next side. The fixture
  // `last_output_` slot is single-shot (the second `Analyze` call
  // would otherwise free the first AST out from under us), so the
  // strings are the durable artifact we compare across the two
  // emits.
  std::string sql_all;
  {
    const ::googlesql::ResolvedStatement* stmt =
        Analyze("SELECT id FROM people UNION ALL SELECT id FROM people");
    ASSERT_NE(stmt, nullptr);
    const ::googlesql::ResolvedScan* scan = QueryInputScan(stmt);
    ASSERT_NE(scan, nullptr);
    ASSERT_EQ(scan->node_kind(), ::googlesql::RESOLVED_SET_OPERATION_SCAN);
    TestTranspiler t;
    sql_all = t.EmitSetOperationScan(
        scan->GetAs<::googlesql::ResolvedSetOperationScan>());
  }
  std::string sql_distinct;
  {
    const ::googlesql::ResolvedStatement* stmt =
        Analyze("SELECT id FROM people UNION DISTINCT SELECT id FROM people");
    ASSERT_NE(stmt, nullptr);
    const ::googlesql::ResolvedScan* scan = QueryInputScan(stmt);
    ASSERT_NE(scan, nullptr);
    ASSERT_EQ(scan->node_kind(), ::googlesql::RESOLVED_SET_OPERATION_SCAN);
    TestTranspiler t;
    sql_distinct = t.EmitSetOperationScan(
        scan->GetAs<::googlesql::ResolvedSetOperationScan>());
  }
  EXPECT_NE(sql_all, sql_distinct);
  EXPECT_NE(sql_all.find(" UNION ALL "), std::string::npos);
  EXPECT_EQ(sql_all.find(" UNION DISTINCT "), std::string::npos);
  EXPECT_EQ(sql_distinct.find(" UNION ALL "), std::string::npos);
  // The bare ` UNION ` keyword (with spaces on both sides) needs
  // to land in the distinct emit; we deliberately do NOT match a
  // substring `UNION` because that would also match `UNION ALL`.
  EXPECT_NE(sql_distinct.find(" UNION "), std::string::npos);
}

// Helper: synthesize a `ResolvedSetOperationScan` directly so the
// fallback paths can be exercised without depending on the
// analyzer producing the matching surface SQL. Each "arm" of the
// set operation is a fresh `ResolvedSingleRowScan` so the inner
// emit composes onto `SELECT 1`. The parent's `column_list` is
// left empty so the per-item projection lands on `SELECT *`,
// which keeps the fallback assertions focused on the
// kind/mode-bail behavior of `EmitSetOperationScan` rather than
// the per-item projection logic.
std::unique_ptr<::googlesql::ResolvedSetOperationScan> MakeTestSetOperationScan(
    ::googlesql::ResolvedSetOperationScan::SetOperationType op_type,
    ::googlesql::ResolvedSetOperationScan::SetOperationColumnMatchMode
        match_mode) {
  std::vector<std::unique_ptr<const ::googlesql::ResolvedSetOperationItem>>
      items;
  items.push_back(::googlesql::MakeResolvedSetOperationItem(
      ::googlesql::MakeResolvedSingleRowScan(),
      /*output_column_list=*/{}));
  items.push_back(::googlesql::MakeResolvedSetOperationItem(
      ::googlesql::MakeResolvedSingleRowScan(),
      /*output_column_list=*/{}));
  auto scan = ::googlesql::MakeResolvedSetOperationScan(
      /*column_list=*/{}, op_type, std::move(items));
  scan->set_column_match_mode(match_mode);
  scan->set_column_propagation_mode(
      ::googlesql::ResolvedSetOperationScan::STRICT);
  return scan;
}

TEST_F(TranspilerTest, EmitSetOperationScanCorrespondingFallsBack) {
  // `CORRESPONDING` reshuffles columns by name -- our positional
  // projection in `EmitSetOperationItem` does not implement the
  // reshuffle, so the emit propagates "" and the engine surfaces
  // UNIMPLEMENTED.
  auto scan = MakeTestSetOperationScan(
      ::googlesql::ResolvedSetOperationScan::UNION_ALL,
      ::googlesql::ResolvedSetOperationScan::CORRESPONDING);
  TestTranspiler t;
  EXPECT_EQ(t.EmitSetOperationScan(scan.get()), "");
}

TEST_F(TranspilerTest, EmitSetOperationScanIntersectAllEmitsKeyword) {
  // `INTERSECT_ALL` (DuckDB-native extension; not in BQ surface
  // SQL) emits the matching `INTERSECT ALL` keyword. The standard
  // SQL bag semantics (`min(m, n)`) match the GoogleSQL
  // `INTERSECT_ALL` contract per `resolved_ast.h`, so the lowered
  // SQL preserves the analyzer's intent.
  auto scan = MakeTestSetOperationScan(
      ::googlesql::ResolvedSetOperationScan::INTERSECT_ALL,
      ::googlesql::ResolvedSetOperationScan::BY_POSITION);
  TestTranspiler t;
  EXPECT_EQ(t.EmitSetOperationScan(scan.get()),
            "SELECT * FROM (SELECT 1) INTERSECT ALL SELECT * FROM (SELECT 1)");
}

TEST_F(TranspilerTest, EmitSetOperationScanExceptAllEmitsKeyword) {
  // `EXCEPT_ALL` similarly emits `EXCEPT ALL`. DuckDB has shipped
  // `EXCEPT ALL` with standard SQL bag-difference semantics
  // (`max(m - n, 0)`) since v0.10; the GoogleSQL contract is the
  // same.
  auto scan = MakeTestSetOperationScan(
      ::googlesql::ResolvedSetOperationScan::EXCEPT_ALL,
      ::googlesql::ResolvedSetOperationScan::BY_POSITION);
  TestTranspiler t;
  EXPECT_EQ(t.EmitSetOperationScan(scan.get()),
            "SELECT * FROM (SELECT * FROM (SELECT 1) EXCEPT ALL SELECT * "
            "FROM (SELECT 1)) ORDER BY 1");
}

// --- Sample scan -------------------------------------------------------


}  // namespace transpiler
}  // namespace duckdb
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
