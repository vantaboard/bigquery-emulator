#include "backend/engine/duckdb/transpiler/transpiler_test_fixture.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace duckdb {
namespace transpiler {

TEST_F(TranspilerTest, TranspileSelectFromWhereGroupByOrderByLimit) {
  // Engine-level smoke check for the plan's "SELECT ... FROM ...
  // WHERE ... GROUP BY ... ORDER BY ... LIMIT" target. We don't
  // round-trip through DuckDB here -- the unit-test fixture has no
  // running DuckDB connection -- but we *do* drive the full
  // `Transpile(stmt)` pipeline so a regression in any one of
  // EmitQueryStmt / EmitLimitOffsetScan / EmitOrderByScan /
  // EmitAggregateScan / EmitFilterScan / EmitTableScan surfaces as
  // a string drift here. The engine-side smoke test (executing on
  // DuckDB) is left to a follow-up plan once the DuckDBEngine
  // integration is updated to dispatch on QueryStmt directly rather
  // than the StripPassThroughProjectScans subset; see
  // docs/ENGINE_POLICY.md for the engine wiring
  // that lands separately.
  const ::googlesql::ResolvedStatement* stmt = Analyze(
      "SELECT id, COUNT(*) AS c FROM people WHERE id > 0 GROUP BY id "
      "ORDER BY id LIMIT 10");
  ASSERT_NE(stmt, nullptr);
  TestTranspiler t;
  // Filter predicate (`>`) and LIMIT 10 over a synthesized aggregate
  // column thread together; the per-piece coverage above keeps each
  // emit honest, while this assertion pins the composition.
  std::string sql = t.Transpile(stmt);
  ASSERT_FALSE(sql.empty());
  EXPECT_NE(sql.find("WHERE (\"id\" > 0)"), std::string::npos);
  EXPECT_NE(sql.find("GROUP BY \"id\""), std::string::npos);
  EXPECT_NE(sql.find("ORDER BY \"id\" ASC"), std::string::npos);
  EXPECT_NE(sql.find("LIMIT 10"), std::string::npos);
}

TEST_F(TranspilerTest, TranspileGroupByAggregateOrderByLimit) {
  // BigFrames `groupby(...).mean().sort_values().head()` lowers to
  // GROUP BY + aggregate + ORDER BY aggregate output + LIMIT. The
  // ORDER BY column is an aggregate output, not a grouping key.
  const ::googlesql::ResolvedStatement* stmt = Analyze(
      "SELECT SUM(amount) AS total FROM orders GROUP BY order_id "
      "ORDER BY total DESC LIMIT 5");
  ASSERT_NE(stmt, nullptr);
  TestTranspiler t;
  std::string sql = t.Transpile(stmt);
  ASSERT_FALSE(sql.empty())
      << "OrderByScan over aggregate output must transpile";
  EXPECT_NE(sql.find("GROUP BY"), std::string::npos);
  EXPECT_NE(sql.find("ORDER BY"), std::string::npos);
  EXPECT_NE(sql.find("LIMIT 5"), std::string::npos);
}

TEST_F(TranspilerTest, TranspileGroupByHiddenKeyOrderByAggregateLimit) {
  // BigFrames groupby-mean keeps only the aggregate in the SELECT list
  // while still grouping by another column; ORDER BY sorts the aggregate
  // output column (often reusing the source column name as alias).
  const ::googlesql::ResolvedStatement* stmt = Analyze(
      "SELECT AVG(amount) AS amount FROM orders GROUP BY order_id "
      "ORDER BY amount DESC LIMIT 5");
  ASSERT_NE(stmt, nullptr);
  TestTranspiler t;
  std::string sql = t.Transpile(stmt);
  ASSERT_FALSE(sql.empty());
}

TEST_F(TranspilerTest, EmitOrderByScanOverAggregateScan) {
  const ::googlesql::ResolvedStatement* stmt = Analyze(
      "SELECT order_id, SUM(amount) AS total FROM orders GROUP BY order_id "
      "ORDER BY total DESC");
  ASSERT_NE(stmt, nullptr);
  const ::googlesql::ResolvedScan* scan = QueryInputScan(stmt);
  ASSERT_NE(scan, nullptr);
  // Peel LimitOffsetScan if present; find OrderByScan in tree.
  while (scan != nullptr &&
         scan->node_kind() == ::googlesql::RESOLVED_LIMIT_OFFSET_SCAN) {
    scan = scan->GetAs<::googlesql::ResolvedLimitOffsetScan>()->input_scan();
  }
  ASSERT_EQ(scan->node_kind(), ::googlesql::RESOLVED_ORDER_BY_SCAN);
  TestTranspiler t;
  std::string sql =
      t.EmitOrderByScan(scan->GetAs<::googlesql::ResolvedOrderByScan>());
  ASSERT_FALSE(sql.empty())
      << "EmitOrderByScan returned empty for aggregate ORDER BY";
  EXPECT_NE(sql.find("ORDER BY"), std::string::npos);
}

TEST_F(TranspilerTest, TranspileGroupByOrderByAggregateExpr) {
  const ::googlesql::ResolvedStatement* stmt = Analyze(
      "SELECT order_id, SUM(amount) AS total FROM orders GROUP BY order_id "
      "ORDER BY SUM(amount) DESC LIMIT 5");
  ASSERT_NE(stmt, nullptr);
  TestTranspiler t;
  std::string sql = t.Transpile(stmt);
  ASSERT_FALSE(sql.empty()) << "ORDER BY aggregate expr must transpile";
}

TEST_F(TranspilerTest, TranspileOrderByScanRootWithoutLimit) {
  const ::googlesql::ResolvedStatement* stmt = Analyze(
      "SELECT order_id, SUM(amount) AS total FROM orders GROUP BY order_id "
      "ORDER BY total DESC");
  ASSERT_NE(stmt, nullptr);
  const ::googlesql::ResolvedScan* scan = QueryInputScan(stmt);
  ASSERT_NE(scan, nullptr);
  ASSERT_EQ(scan->node_kind(), ::googlesql::RESOLVED_ORDER_BY_SCAN);
  TestTranspiler t;
  ASSERT_FALSE(t.Transpile(stmt).empty());
}

TEST_F(TranspilerTest, TranspileBigframesMeanStatsOrderByRowNumber) {
  // bigframes `Series.mean()` materializes multiple aggregates plus a
  // ROW_NUMBER() OVER (ORDER BY NULL) wrapper, then ORDER BY the
  // synthetic bfuid column — see captured SQL from snippet gate.
  const ::googlesql::ResolvedStatement* stmt = Analyze(R"sql(
SELECT `bfuid_col_2`, `mean` FROM (
  SELECT
    ROW_NUMBER() OVER (ORDER BY NULL ASC) - 1 AS `bfuid_col_2`,
    `t2`.`mean`
  FROM (
    SELECT AVG(`amount`) AS `mean` FROM `orders`
  ) AS `t2`
) AS `t`
ORDER BY `bfuid_col_2` ASC NULLS LAST
)sql");
  ASSERT_NE(stmt, nullptr);
  TestTranspiler t;
  std::string sql = t.Transpile(stmt);
  ASSERT_FALSE(sql.empty()) << "bigframes mean stats ORDER BY bfuid_col_2";
}

TEST_F(TranspilerTest, TranspileRowNumberOverFarmFingerprint) {
  // bigframes peek/cache assigns ROW_NUMBER() OVER (ORDER BY keys built
  // from FARM_FINGERPRINT(...)). Route classifier must not promote on the
  // string literal inside FARM_FINGERPRINT (ResolvedConstant).
  const ::googlesql::ResolvedStatement* stmt = Analyze(
      "SELECT ROW_NUMBER() OVER (ORDER BY FARM_FINGERPRINT('x') ASC) "
      "FROM orders");
  ASSERT_NE(stmt, nullptr);
  TestTranspiler t;
  std::string sql = t.Transpile(stmt);
  ASSERT_FALSE(sql.empty())
      << "ROW_NUMBER ORDER BY FARM_FINGERPRINT must transpile";
  EXPECT_NE(sql.find("ROW_NUMBER"), std::string::npos);
}

TEST_F(TranspilerTest, TranspileBigframesCacheJoinCoalesceOn) {
  const ::googlesql::ResolvedStatement* stmt = Analyze(
      "SELECT a.level_0, b.bfuid_col_2 FROM "
      "(SELECT 0 AS level_0, 'John' AS column_0) a "
      "LEFT OUTER JOIN "
      "(SELECT 0 AS bfuid_col_1, 'group_1' AS bfuid_col_2) b "
      "ON COALESCE(a.level_0, 0) = COALESCE(b.bfuid_col_1, 0) "
      "AND COALESCE(a.level_0, 1) = COALESCE(b.bfuid_col_1, 1)");
  ASSERT_NE(stmt, nullptr);
  TestTranspiler t;
  ASSERT_FALSE(t.Transpile(stmt).empty())
      << "bigframes cache COALESCE join ON must transpile";
}

TEST_F(TranspilerTest, TranspileUnnestJoinPreservesInputRn) {
  const ::googlesql::ResolvedStatement* stmt = Analyze(
      "SELECT a.level_0, b.column_0 FROM "
      "(SELECT * FROM UNNEST(ARRAY<STRUCT<level_0 INT64, column_0 STRING>>"
      "[STRUCT(0, 'John')]) AS level_0) a "
      "LEFT OUTER JOIN "
      "(SELECT * FROM UNNEST(ARRAY<STRUCT<level_0 INT64, column_0 STRING>>"
      "[STRUCT(0, 'group_1')]) AS level_0) b "
      "ON COALESCE(a.level_0, 0) = COALESCE(b.level_0, 0)");
  ASSERT_NE(stmt, nullptr);
  TestTranspiler t;
  std::string sql = t.Transpile(stmt);
  ASSERT_FALSE(sql.empty()) << "UNNEST join must transpile";
  EXPECT_NE(sql.find("\"__bq_input_rn\""), std::string::npos)
      << "join must preserve UNNEST ordinality column: " << sql;
}

TEST_F(TranspilerTest, TranspileBigframesMeanStatsFullAggregates) {
  const ::googlesql::ResolvedStatement* stmt = Analyze(R"sql(
SELECT `bfuid_col_2`, `count`, `min`, `max`, `std`, `mean`, `var`, `sum` FROM (
  SELECT
    ROW_NUMBER() OVER (ORDER BY NULL ASC) - 1 AS `bfuid_col_2`,
    `t2`.`count`, `t2`.`min`, `t2`.`max`, `t2`.`std`, `t2`.`mean`, `t2`.`var`, `t2`.`sum`
  FROM (
    SELECT
      COUNT(`amount`) AS `count`,
      MIN(`amount`) AS `min`,
      MAX(`amount`) AS `max`,
      STDDEV_SAMP(`amount`) AS `std`,
      AVG(`amount`) AS `mean`,
      VARIANCE(`amount`) AS `var`,
      COALESCE(SUM(`amount`), 0) AS `sum`
    FROM `orders`
  ) AS `t2`
) AS `t`
ORDER BY `bfuid_col_2` ASC NULLS LAST
)sql");
  ASSERT_NE(stmt, nullptr);
  TestTranspiler t;
  std::string sql = t.Transpile(stmt);
  ASSERT_FALSE(sql.empty()) << "full bigframes stats query must transpile";
}

TEST_F(TranspilerTest, TranspileFullOuterJoinCoalesceOrderBy) {
  const ::googlesql::ResolvedStatement* stmt = Analyze(
      "SELECT COALESCE(CAST(u.id AS STRING), 'no-user') AS user_id, "
      "COALESCE(CAST(e.id AS STRING), 'no-event') AS event_id, "
      "COALESCE(u.name, '<none>') AS name, "
      "COALESCE(e.kind, '<none>') AS kind "
      "FROM (SELECT 1 AS id, 'ada' AS name) AS u "
      "FULL OUTER JOIN (SELECT 10 AS id, 1 AS user_id, 'login' AS kind) AS e "
      "ON u.id = e.user_id "
      "ORDER BY user_id, event_id");
  ASSERT_NE(stmt, nullptr);
  TestTranspiler t;
  std::string sql = t.Transpile(stmt);
  ASSERT_FALSE(sql.empty()) << "FULL OUTER JOIN COALESCE must transpile";
  EXPECT_NE(sql.find("ORDER BY \"user_id\""), std::string::npos) << sql;
  EXPECT_EQ(sql.find("__bq_j_6"), std::string::npos) << sql;
}

TEST_F(TranspilerTest, TranspileClusteredTableSampleQuery) {
  // golang-samples queryClusteredTable: global aggregates + filter +
  // named parameter. Regression for transpiler coverage on the
  // FilterScan -> AggregateScan -> QueryStmt shape used by clustered
  // table docs (COUNT(1), SUM(NUMERIC), COUNT(DISTINCT), TIMESTAMP).
  ::googlesql::AnalyzerOptions options = MakeAnalyzerOptions();
  ASSERT_TRUE(
      options.AddQueryParameter("wallet", type_factory_->get_string()).ok());
  const ::googlesql::ResolvedStatement* stmt = AnalyzeWith(R"sql(
SELECT
  COUNT(1) AS transactions,
  SUM(amount) AS total_paid,
  COUNT(DISTINCT destination) AS distinct_recipients
FROM transactions
WHERE timestamp > TIMESTAMP('2015-01-01')
  AND origin = @wallet
)sql",
                                                           options);
  ASSERT_NE(stmt, nullptr);
  TestTranspiler t;
  std::string sql = t.Transpile(stmt);
  ASSERT_FALSE(sql.empty()) << "queryClusteredTable sample must transpile";
  EXPECT_NE(sql.find("COUNT(1)"), std::string::npos);
  EXPECT_NE(sql.find("SUM(\"amount\")"), std::string::npos);
  EXPECT_NE(sql.find("COUNT(DISTINCT"), std::string::npos);
}

TEST_F(TranspilerTest, TranspileDateAddIntervalColumnRef) {
  const ::googlesql::ResolvedStatement* stmt = Analyze(
      "SELECT EXTRACT(YEAR FROM DATE_ADD(DATE '2020-01-01', INTERVAL id "
      "DAY)) AS yr FROM people");
  ASSERT_NE(stmt, nullptr);
  TestTranspiler t;
  std::string sql = t.Transpile(stmt);
  ASSERT_FALSE(sql.empty()) << "transpile failed";
  EXPECT_NE(sql.find("bq_date_add"), std::string::npos) << sql;
  EXPECT_NE(sql.find("bq_extract"), std::string::npos) << sql;
  EXPECT_EQ(sql.find(" DAY"), std::string::npos) << sql;
}

TEST_F(TranspilerTest, TranspileDateFuncsBenchShape) {
  const ::googlesql::ResolvedStatement* stmt = Analyze(
      "SELECT EXTRACT(YEAR FROM DATE_ADD(DATE '2020-01-01', INTERVAL id "
      "DAY)) AS yr, COUNT(*) AS cnt FROM people GROUP BY yr ORDER BY yr");
  ASSERT_NE(stmt, nullptr);
  TestTranspiler t;
  std::string sql = t.Transpile(stmt);
  ASSERT_FALSE(sql.empty()) << "transpile failed";
  EXPECT_NE(sql.find("bq_date_add"), std::string::npos) << sql;
  EXPECT_NE(sql.find("bq_extract"), std::string::npos) << sql;
  EXPECT_EQ(sql.find(" DAY"), std::string::npos) << sql;
}

TEST_F(TranspilerTest, TranspileFloatSumCastsAggregateToDouble) {
  const ::googlesql::ResolvedStatement* stmt =
      Analyze("SELECT SUM(CAST(id AS FLOAT64)) AS total FROM people");
  ASSERT_NE(stmt, nullptr);
  TestTranspiler t;
  std::string sql = t.Transpile(stmt);
  ASSERT_FALSE(sql.empty()) << "transpile failed";
  EXPECT_NE(sql.find("SUM("), std::string::npos) << sql;
}

TEST_F(TranspilerTest, TranspileUnnestArrayBenchShape) {
  const ::googlesql::ResolvedStatement* stmt = Analyze(
      "SELECT COUNT(*) AS cnt, SUM(x) AS total FROM arr_table, "
      "UNNEST(arr_table.arr) AS x");
  ASSERT_NE(stmt, nullptr);
  TestTranspiler t;
  std::string sql = t.Transpile(stmt);
  ASSERT_FALSE(sql.empty()) << "transpile failed";
  EXPECT_NE(sql.find("unnest("), std::string::npos) << sql;
  EXPECT_NE(sql.find("__bq_l"), std::string::npos) << sql;
  EXPECT_NE(sql.find("\"cnt\""), std::string::npos) << sql;
}

}  // namespace transpiler
}  // namespace duckdb
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
