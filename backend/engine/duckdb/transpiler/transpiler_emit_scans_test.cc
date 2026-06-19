#include "backend/engine/duckdb/transpiler/transpiler_test_fixture.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace duckdb {
namespace transpiler {

TEST_F(TranspilerTest, EmitTableScanEmitsSelectStar) {
  // `SELECT * FROM people` collapses (after rewrites) onto a
  // ResolvedTableScan whose `column_list` carries every column on
  // the underlying table. We assert on both the select-list shape
  // (one quoted identifier per column, in catalog order) and the
  // bare table-name reference -- the engine ATTACHes the storage's
  // backing files under that name at execute time.
  const ::googlesql::ResolvedStatement* stmt = Analyze("SELECT * FROM people");
  const ::googlesql::ResolvedScan* scan = QueryInputScan(stmt);
  ASSERT_NE(scan, nullptr);
  ASSERT_EQ(scan->node_kind(), ::googlesql::RESOLVED_TABLE_SCAN);
  TestTranspiler t;
  EXPECT_EQ(t.EmitTableScan(scan->GetAs<::googlesql::ResolvedTableScan>()),
            "SELECT \"id\", \"name\" FROM \"people\"");
}

TEST_F(TranspilerTest, EmitFilterScanWrapsInputScanWithWhere) {
  // `WHERE id > 0` lands as a ResolvedFilterScan around a
  // ResolvedTableScan. The emit composes the table scan's
  // self-contained SELECT as a derived table so the WHERE clause
  // sees the same column aliases the inner SELECT exposes.
  const ::googlesql::ResolvedStatement* stmt =
      Analyze("SELECT id FROM people WHERE id > 0");
  const ::googlesql::ResolvedScan* scan = QueryInputScan(stmt);
  ASSERT_NE(scan, nullptr);
  ASSERT_EQ(scan->node_kind(), ::googlesql::RESOLVED_FILTER_SCAN);
  TestTranspiler t;
  EXPECT_EQ(t.EmitFilterScan(scan->GetAs<::googlesql::ResolvedFilterScan>()),
            "SELECT * FROM (SELECT \"id\", \"name\" FROM \"people\") WHERE "
            "(\"id\" > 0)");
}

TEST_F(TranspilerTest, EmitFilterScanWithCoalescePredicateLowers) {
  // Picking a predicate that the function-call emit *does* know
  // about (`COALESCE`) lets us exercise the actual FilterScan SQL
  // composition. COALESCE(name, 'x') = 'x' isn't a particularly
  // useful predicate, but it threads two literals, a column ref,
  // and the COALESCE emit through the same SQL string -- exactly
  // the integration the FilterScan emit needs to keep honest.
  const ::googlesql::ResolvedStatement* stmt =
      Analyze("SELECT id FROM people WHERE COALESCE(name, 'x') = 'x'");
  const ::googlesql::ResolvedScan* scan = QueryInputScan(stmt);
  ASSERT_NE(scan, nullptr);
  ASSERT_EQ(scan->node_kind(), ::googlesql::RESOLVED_FILTER_SCAN);
  TestTranspiler t;
  EXPECT_EQ(t.EmitFilterScan(scan->GetAs<::googlesql::ResolvedFilterScan>()),
            "SELECT * FROM (SELECT \"id\", \"name\" FROM \"people\") WHERE "
            "(COALESCE(\"name\", 'x') = 'x')");
}

// --- Join ---------------------------------------------------------------

TEST_F(TranspilerTest, EmitJoinScanCrossJoinTwoUnnests) {
  const ::googlesql::ResolvedStatement* stmt = Analyze(
      "SELECT n, m FROM UNNEST(GENERATE_ARRAY(1, 10)) AS n "
      "CROSS JOIN UNNEST(GENERATE_ARRAY(1, 2)) AS m");
  ASSERT_NE(stmt, nullptr);
  const ::googlesql::ResolvedScan* scan = QueryInputScan(stmt);
  ASSERT_NE(scan, nullptr);
  // Explicit CROSS JOIN of standalone UNNESTs chains into nested
  // single-array ResolvedArrayScan nodes (outer over inner).
  ASSERT_EQ(scan->node_kind(), ::googlesql::RESOLVED_ARRAY_SCAN);
  const auto* arr = scan->GetAs<::googlesql::ResolvedArrayScan>();
  ASSERT_NE(arr, nullptr);
  ASSERT_EQ(arr->array_expr_list_size(), 1);
  ASSERT_NE(arr->input_scan(), nullptr);
  ASSERT_EQ(arr->input_scan()->node_kind(), ::googlesql::RESOLVED_ARRAY_SCAN);
  TestTranspiler t;
  std::string sql = t.EmitArrayScan(arr);
  ASSERT_FALSE(sql.empty()) << "multi-array cross unnest must emit";
  EXPECT_NE(sql.find("CROSS JOIN"), std::string::npos) << sql;
  EXPECT_NE(sql.find("unnest("), std::string::npos) << sql;
}

TEST_F(TranspilerTest, EmitJoinScanCrossJoinFromImplicit) {
  // `FROM people, orders` analyzes to a ResolvedJoinScan with
  // INNER + null `join_expr`. The emit lowers it to DuckDB's
  // explicit CROSS JOIN so a downstream FilterScan / ProjectScan
  // can wrap it without having to know about the implicit-join
  // shorthand.
  const ::googlesql::ResolvedStatement* stmt =
      Analyze("SELECT id FROM people, orders");
  const ::googlesql::ResolvedScan* scan = QueryInputScan(stmt);
  ASSERT_NE(scan, nullptr);
  ASSERT_EQ(scan->node_kind(), ::googlesql::RESOLVED_JOIN_SCAN);
  TestTranspiler t;
  std::string sql =
      t.EmitJoinScan(scan->GetAs<::googlesql::ResolvedJoinScan>());
  EXPECT_NE(sql.find("CROSS JOIN"), std::string::npos) << sql;
  EXPECT_NE(sql.find("__bq_j_"), std::string::npos) << sql;
}

TEST_F(TranspilerTest, EmitJoinScanInnerWithLiteralPredicate) {
  // `ON TRUE` keeps the test focused on the join emit shape: the
  // predicate lowers cleanly through `EmitLiteral` so the assertion
  // can pin the full INNER JOIN SQL string. (`ON x = y` would
  // route through the `=` function call which isn't on the
  // function-call whitelist for the scan emit; that path is covered
  // by the propagation test below.)
  const ::googlesql::ResolvedStatement* stmt =
      Analyze("SELECT id FROM people INNER JOIN orders ON TRUE");
  const ::googlesql::ResolvedScan* scan = QueryInputScan(stmt);
  ASSERT_NE(scan, nullptr);
  ASSERT_EQ(scan->node_kind(), ::googlesql::RESOLVED_JOIN_SCAN);
  TestTranspiler t;
  std::string sql =
      t.EmitJoinScan(scan->GetAs<::googlesql::ResolvedJoinScan>());
  EXPECT_NE(sql.find("INNER JOIN"), std::string::npos) << sql;
  EXPECT_NE(sql.find("__bq_j_"), std::string::npos) << sql;
  EXPECT_NE(sql.find(" ON true"), std::string::npos) << sql;
}

TEST_F(TranspilerTest, EmitJoinScanLeftWithLiteralPredicate) {
  // LEFT JOIN requires a non-null `join_expr`; `ON TRUE` is the
  // smallest predicate that round-trips through the emit. The
  // assertion confirms the keyword swap (INNER -> LEFT) is the
  // only difference vs. the inner test above.
  const ::googlesql::ResolvedStatement* stmt =
      Analyze("SELECT id FROM people LEFT JOIN orders ON TRUE");
  const ::googlesql::ResolvedScan* scan = QueryInputScan(stmt);
  ASSERT_NE(scan, nullptr);
  ASSERT_EQ(scan->node_kind(), ::googlesql::RESOLVED_JOIN_SCAN);
  TestTranspiler t;
  std::string sql =
      t.EmitJoinScan(scan->GetAs<::googlesql::ResolvedJoinScan>());
  EXPECT_NE(sql.find("LEFT JOIN"), std::string::npos) << sql;
  EXPECT_NE(sql.find("__bq_j_"), std::string::npos) << sql;
}

TEST_F(TranspilerTest, EmitJoinScanInnerOnEqualPredicate) {
  const ::googlesql::ResolvedStatement* stmt =
      Analyze("SELECT id FROM people INNER JOIN orders ON id = order_id");
  const ::googlesql::ResolvedScan* scan = QueryInputScan(stmt);
  ASSERT_NE(scan, nullptr);
  ASSERT_EQ(scan->node_kind(), ::googlesql::RESOLVED_JOIN_SCAN);
  TestTranspiler t;
  std::string sql =
      t.EmitJoinScan(scan->GetAs<::googlesql::ResolvedJoinScan>());
  EXPECT_NE(sql.find("INNER JOIN"), std::string::npos) << sql;
  EXPECT_NE(sql.find("__bq_j_"), std::string::npos) << sql;
  EXPECT_NE(sql.find("__bq_l.\"id\" = __bq_r.\"order_id\""), std::string::npos)
      << sql;
}

TEST_F(TranspilerTest, EmitJoinScanLeftOnCoalesceEquality) {
  // bigframes cache joins use COALESCE-sentinel equality predicates in
  // LEFT OUTER JOIN ON clauses for row-ordering keys.
  const ::googlesql::ResolvedStatement* stmt = Analyze(
      "SELECT a.level_0, b.bfuid_col_2 FROM "
      "(SELECT 0 AS level_0, 'John' AS column_0) a "
      "LEFT OUTER JOIN "
      "(SELECT 0 AS bfuid_col_1, 'group_1' AS bfuid_col_2) b "
      "ON COALESCE(a.level_0, 0) = COALESCE(b.bfuid_col_1, 0) "
      "AND COALESCE(a.level_0, 1) = COALESCE(b.bfuid_col_1, 1)");
  ASSERT_NE(stmt, nullptr);
  TestTranspiler t;
  std::string sql = t.Transpile(stmt);
  ASSERT_FALSE(sql.empty()) << "COALESCE join ON must transpile";
  EXPECT_NE(sql.find("COALESCE"), std::string::npos) << sql;
  EXPECT_NE(sql.find("LEFT JOIN"), std::string::npos) << sql;
}

TEST_F(TranspilerTest, EmitJoinScanInnerUsingSingleColumn) {
  // `USING (id)` canonicalizes to `$equal` in `join_expr`, but the
  // emit peels the column name back out and emits DuckDB's native
  // `USING (...)` so we do not need `$equal` on the disposition table.
  const ::googlesql::ResolvedStatement* stmt =
      Analyze("SELECT p.id FROM people p INNER JOIN people p2 USING (id)");
  const ::googlesql::ResolvedScan* scan = QueryInputScan(stmt);
  ASSERT_NE(scan, nullptr);
  ASSERT_EQ(scan->node_kind(), ::googlesql::RESOLVED_JOIN_SCAN);
  const auto* join = scan->GetAs<::googlesql::ResolvedJoinScan>();
  ASSERT_TRUE(join->has_using());
  TestTranspiler t;
  std::string sql = t.EmitJoinScan(join);
  EXPECT_NE(sql.find("INNER JOIN"), std::string::npos) << sql;
  EXPECT_NE(sql.find("USING (\"id\")"), std::string::npos) << sql;
  EXPECT_EQ(sql.find(" ON "), std::string::npos) << sql;
}

// --- Aggregate ----------------------------------------------------------

}  // namespace transpiler
}  // namespace duckdb
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
