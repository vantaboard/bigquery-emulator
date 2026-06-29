#include <gtest/gtest.h>

#include <string>

#include "backend/engine/duckdb/transpiler/transpiler_test_fixture.h"
#include "googlesql/resolved_ast/resolved_ast.h"
#include "googlesql/resolved_ast/resolved_node_kind.pb.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace duckdb {
namespace transpiler {

TEST_F(TranspilerTest, EmitAggregateScanCountStarNoGroupBy) {
  // `SELECT COUNT(*) FROM people` analyzes to an AggregateScan
  // with an empty group_by_list and a single aggregate. The
  // aggregate column gets a synthesized name (`$agg1`); we assert
  // on it so any drift in the analyzer's naming surfaces here
  // rather than in the engine integration.
  const ::googlesql::ResolvedStatement* stmt =
      Analyze("SELECT COUNT(*) FROM people");
  const ::googlesql::ResolvedScan* scan = QueryInputScan(stmt);
  ASSERT_NE(scan, nullptr);
  ASSERT_EQ(scan->node_kind(), ::googlesql::RESOLVED_AGGREGATE_SCAN);
  TestTranspiler t;
  EXPECT_EQ(
      t.EmitAggregateScan(scan->GetAs<::googlesql::ResolvedAggregateScan>()),
      "SELECT COUNT(*) AS \"$agg1\" FROM (SELECT \"id\", \"name\" FROM "
      "\"people\")");
}

TEST_F(TranspilerTest, EmitAggregateScanSumGroupByColumn) {
  // SUM over a grouped column threads the column-ref through the
  // GROUP BY clause and the SELECT list. The grouping column's
  // ResolvedColumn::name() matches its source name (`id`), so the
  // AS alias collapses to the column reference.
  const ::googlesql::ResolvedStatement* stmt =
      Analyze("SELECT id, SUM(id) FROM people GROUP BY id");
  const ::googlesql::ResolvedScan* scan = QueryInputScan(stmt);
  ASSERT_NE(scan, nullptr);
  ASSERT_EQ(scan->node_kind(), ::googlesql::RESOLVED_AGGREGATE_SCAN);
  TestTranspiler t;
  EXPECT_EQ(
      t.EmitAggregateScan(scan->GetAs<::googlesql::ResolvedAggregateScan>()),
      "SELECT \"id\", SUM(\"id\") AS \"$agg1\" FROM (SELECT \"id\", "
      "\"name\" FROM \"people\") GROUP BY \"id\"");
}

TEST_F(TranspilerTest, EmitAggregateScanAvgMinMaxGroupBy) {
  // All three of AVG / MIN / MAX share the same emit path; one
  // test covers the lot. The output column for each aggregate is
  // again the analyzer-synthesized `$agg<n>` name.
  const ::googlesql::ResolvedStatement* stmt =
      Analyze("SELECT id, AVG(id), MIN(id), MAX(id) FROM people GROUP BY id");
  const ::googlesql::ResolvedScan* scan = QueryInputScan(stmt);
  ASSERT_NE(scan, nullptr);
  ASSERT_EQ(scan->node_kind(), ::googlesql::RESOLVED_AGGREGATE_SCAN);
  TestTranspiler t;
  EXPECT_EQ(
      t.EmitAggregateScan(scan->GetAs<::googlesql::ResolvedAggregateScan>()),
      "SELECT \"id\", AVG(\"id\") AS \"$agg1\", MIN(\"id\") AS \"$agg2\", "
      "MAX(\"id\") AS \"$agg3\" FROM (SELECT \"id\", \"name\" FROM "
      "\"people\") GROUP BY \"id\"");
}

TEST_F(TranspilerTest, EmitAggregateScanArrayAggMapsThroughTable) {
  // `ARRAY_AGG` is in the disposition table (`array_agg: ARRAY_AGG`),
  // so the lower path emits the DuckDB aggregate verbatim. This
  // exercises the table-driven dispatch from inside the AggregateScan
  // emit -- a direct counterpart to `EmitFunctionCallMappedFunction`
  // above for the aggregate code path.
  const ::googlesql::ResolvedStatement* stmt =
      Analyze("SELECT id, ARRAY_AGG(id) FROM people GROUP BY id");
  const ::googlesql::ResolvedScan* scan = QueryInputScan(stmt);
  ASSERT_NE(scan, nullptr);
  ASSERT_EQ(scan->node_kind(), ::googlesql::RESOLVED_AGGREGATE_SCAN);
  TestTranspiler t;
  EXPECT_EQ(
      t.EmitAggregateScan(scan->GetAs<::googlesql::ResolvedAggregateScan>()),
      "SELECT \"id\", if(count(\"id\") < count(*), "
      "error('ARRAY_AGG: input value must be not null'), list(\"id\")) AS "
      "\"$agg1\" FROM (SELECT *, row_number() OVER () AS \"__bq_input_rn\" "
      "FROM (SELECT \"id\", \"name\" FROM \"people\")) GROUP BY \"id\"");
}

TEST_F(TranspilerTest, EmitAggregateScanArrayAggOrderByLimitRewritesToList) {
  const ::googlesql::ResolvedStatement* stmt = Analyze(
      "SELECT ARRAY_AGG(x ORDER BY x LIMIT 2) FROM UNNEST([3, 1, 2, 4]) AS x");
  const ::googlesql::ResolvedScan* scan = QueryInputScan(stmt);
  ASSERT_NE(scan, nullptr);
  TestTranspiler t;
  std::string sql =
      t.EmitAggregateScan(scan->GetAs<::googlesql::ResolvedAggregateScan>());
  EXPECT_NE(sql.find("list_slice(list(\"x\" ORDER BY"), std::string::npos)
      << sql;
  EXPECT_NE(sql.find(", 1, 2)"), std::string::npos) << sql;
}

TEST_F(TranspilerTest, EmitAggregateScanStringAggLimitRewritesToArrayToString) {
  const ::googlesql::ResolvedStatement* stmt = Analyze(
      "SELECT STRING_AGG(fruit, ' & ' LIMIT 2) FROM "
      "UNNEST(['apple', 'pear', 'banana']) AS fruit");
  const ::googlesql::ResolvedScan* scan = QueryInputScan(stmt);
  ASSERT_NE(scan, nullptr);
  TestTranspiler t;
  std::string sql =
      t.EmitAggregateScan(scan->GetAs<::googlesql::ResolvedAggregateScan>());
  EXPECT_NE(sql.find("array_to_string(list_slice(list("), std::string::npos)
      << sql;
  EXPECT_NE(sql.find(", ' & ')"), std::string::npos) << sql;
}

TEST_F(TranspilerTest, EmitAggregateScanStringAggOrderByLimit) {
  const ::googlesql::ResolvedStatement* stmt = Analyze(
      "SELECT STRING_AGG(x, ',' ORDER BY x ASC LIMIT 2) FROM "
      "UNNEST(['c', 'a', 'b']) AS x");
  const ::googlesql::ResolvedScan* scan = QueryInputScan(stmt);
  ASSERT_NE(scan, nullptr);
  TestTranspiler t;
  std::string sql =
      t.EmitAggregateScan(scan->GetAs<::googlesql::ResolvedAggregateScan>());
  EXPECT_NE(sql.find("array_to_string(list_slice(list("), std::string::npos)
      << sql;
  EXPECT_NE(sql.find("ORDER BY \"x\" ASC"), std::string::npos) << sql;
}

TEST_F(TranspilerTest, EmitAggregateScanFallsBackOnUnsupportedAggregate) {
  // `APPROX_QUANTILES` is on the `unsupported` route per
  // `docs/ENGINE_POLICY.md`; the aggregate emit returns
  // "" and the AggregateScan emit propagates the empty string. The
  // engine surfaces UNIMPLEMENTED for the whole query.
  const ::googlesql::ResolvedStatement* stmt =
      Analyze("SELECT id, APPROX_QUANTILES(id, 2) FROM people GROUP BY id");
  const ::googlesql::ResolvedScan* scan = QueryInputScan(stmt);
  ASSERT_NE(scan, nullptr);
  ASSERT_EQ(scan->node_kind(), ::googlesql::RESOLVED_AGGREGATE_SCAN);
  TestTranspiler t;
  EXPECT_EQ(
      t.EmitAggregateScan(scan->GetAs<::googlesql::ResolvedAggregateScan>()),
      "");
}

// --- GROUPING SETS / ROLLUP / CUBE / GROUPING() ------------------------
//
// `docs/ENGINE_POLICY.md` Family 1. Each shape exercises
// the `grouping_set_list` path in `EmitAggregateScan`:
//
//   * Explicit GROUPING SETS  -> `(a, b)`, `(a)`, `()` entries.
//   * ROLLUP                  -> single ROLLUP entry expands to N+1
//                                grouping sets via `ResolvedRollup`.
//   * CUBE                    -> single CUBE entry expands to 2^N
//                                grouping sets via `ResolvedCube`.
//   * GROUPING(<col>)         -> `ResolvedGroupingCall` in
//                                `grouping_call_list`; projects as
//                                `GROUPING(<col>) AS "<output>"`.

TEST_F(TranspilerTest, EmitAggregateScanGroupingSetsExplicit) {
  // `GROUP BY GROUPING SETS ((id, name), (id), ())` analyzes with
  // three `ResolvedGroupingSet` entries in `grouping_set_list`.
  // The emit lands as `GROUP BY GROUPING SETS ((<a>, <b>), (<a>),
  // ())`, with each grouping-set tuple referencing the SELECT-list
  // aliases for `group_by_list` columns. DuckDB resolves the alias
  // inside GROUP BY GROUPING SETS, so the emitted SQL is
  // self-contained.
  const ::googlesql::ResolvedStatement* stmt = Analyze(
      "SELECT id, name, COUNT(*) FROM people "
      "GROUP BY GROUPING SETS ((id, name), (id), ())");
  const ::googlesql::ResolvedScan* scan = QueryInputScan(stmt);
  ASSERT_NE(scan, nullptr);
  ASSERT_EQ(scan->node_kind(), ::googlesql::RESOLVED_AGGREGATE_SCAN);
  TestTranspiler t;
  std::string sql =
      t.EmitAggregateScan(scan->GetAs<::googlesql::ResolvedAggregateScan>());
  EXPECT_NE(sql.find(" GROUP BY GROUPING SETS ("), std::string::npos)
      << "expected GROUP BY GROUPING SETS keyword; got: " << sql;
  EXPECT_NE(sql.find("(\"id\", \"name\")"), std::string::npos)
      << "expected (id, name) tuple; got: " << sql;
  EXPECT_NE(sql.find("(\"id\")"), std::string::npos)
      << "expected (id) tuple; got: " << sql;
  EXPECT_NE(sql.find("()"), std::string::npos)
      << "expected empty () tuple; got: " << sql;
}

TEST_F(TranspilerTest, EmitAggregateScanRollupAnalyzerExpandsToSets) {
  // The analyzer canonicalizes `GROUP BY ROLLUP(id, name)` into
  // three `ResolvedGroupingSet` entries -- `(id, name), (id), ()`
  // -- in `grouping_set_list`, not a single `ResolvedRollup`. Pin
  // the expanded form here so the user-visible BigQuery semantics
  // (one row per ROLLUP step) are exercised end-to-end on the
  // DuckDB fast path. The `ResolvedRollup` direct-construction
  // test below pins the keyword emit code path for shapes that
  // upstream builders could produce.
  const ::googlesql::ResolvedStatement* stmt = Analyze(
      "SELECT id, name, COUNT(*) FROM people GROUP BY ROLLUP(id, name)");
  const ::googlesql::ResolvedScan* scan = QueryInputScan(stmt);
  ASSERT_NE(scan, nullptr);
  ASSERT_EQ(scan->node_kind(), ::googlesql::RESOLVED_AGGREGATE_SCAN);
  TestTranspiler t;
  std::string sql =
      t.EmitAggregateScan(scan->GetAs<::googlesql::ResolvedAggregateScan>());
  EXPECT_NE(sql.find("GROUP BY GROUPING SETS ("), std::string::npos)
      << "expected GROUP BY GROUPING SETS keyword; got: " << sql;
  EXPECT_NE(sql.find("(\"id\", \"name\")"), std::string::npos)
      << "expected (id, name) leaf; got: " << sql;
  EXPECT_NE(sql.find("(\"id\")"), std::string::npos)
      << "expected (id) subtotal; got: " << sql;
  EXPECT_NE(sql.find("()"), std::string::npos)
      << "expected () grand-total tuple; got: " << sql;
}

TEST_F(TranspilerTest, EmitAggregateScanCubeAnalyzerExpandsToSets) {
  // CUBE(id, name) canonicalizes to 2^2 = 4 grouping sets:
  // `(id, name), (id), (name), ()`. Same shape as the ROLLUP test
  // above; here we pin the full fan-out so a partial-emit
  // regression in `EmitGroupingSetEntry` shows up as a missing
  // tuple.
  const ::googlesql::ResolvedStatement* stmt =
      Analyze("SELECT id, name, COUNT(*) FROM people GROUP BY CUBE(id, name)");
  const ::googlesql::ResolvedScan* scan = QueryInputScan(stmt);
  ASSERT_NE(scan, nullptr);
  ASSERT_EQ(scan->node_kind(), ::googlesql::RESOLVED_AGGREGATE_SCAN);
  TestTranspiler t;
  std::string sql =
      t.EmitAggregateScan(scan->GetAs<::googlesql::ResolvedAggregateScan>());
  EXPECT_NE(sql.find("(\"id\", \"name\")"), std::string::npos) << sql;
  EXPECT_NE(sql.find("(\"id\")"), std::string::npos) << sql;
  EXPECT_NE(sql.find("(\"name\")"), std::string::npos) << sql;
  EXPECT_NE(sql.find("()"), std::string::npos) << sql;
}

// --- PIVOT / UNPIVOT -----------------------------------------------------
//
// `EmitPivotScan` lowers a BigQuery `PIVOT(<agg>(...) FOR <expr> IN
// (<vals>))` to DuckDB conditional aggregation using `FILTER`. Each
// `ResolvedPivotColumn` carries the (pivot_expr_index,
// pivot_value_index) tuple plus the analyzer-chosen output column
// name, which the emit aliases onto a `<agg> FILTER (WHERE
// <for_expr> = <pivot_value>)` projection.
//
// `EmitUnpivotScan` lowers `UNPIVOT(<value_cols> FOR <label_col> IN
// (<arg_groups>))` to a UNION ALL of per-arg SELECTs. Each branch
// projects the input columns that pass through unchanged, renames
// the arg's column refs to the value-column names, and adds the
// arg's label literal under `label_column`. When `include_nulls()`
// is false (the BigQuery default), each branch adds a
// `WHERE NOT (val0 IS NULL AND ... AND valN IS NULL)` filter so the
// EXCLUDE NULLS semantics match BigQuery.

TEST_F(TranspilerTest, EmitPivotScanBuildsFilterAggregates) {
  // `SUM(amount) FOR kind IN ('A', 'B')` yields one `ResolvedPivotColumn`
  // per (pivot_expr_index=0, pivot_value_index=v) pair. The emit
  // projects each as `SUM("amount") FILTER (WHERE "kind" = '<v>')`
  // aliased to the analyzer-chosen output column name.
  const ::googlesql::ResolvedStatement* stmt =
      Analyze("SELECT * FROM sales PIVOT(SUM(amount) FOR kind IN ('A', 'B'))");
  const ::googlesql::ResolvedScan* scan = QueryInputScan(stmt);
  ASSERT_NE(scan, nullptr);
  ASSERT_EQ(scan->node_kind(), ::googlesql::RESOLVED_PIVOT_SCAN);
  TestTranspiler t;
  std::string sql =
      t.EmitPivotScan(scan->GetAs<::googlesql::ResolvedPivotScan>());
  EXPECT_NE(sql.find("SUM(\"amount\") FILTER (WHERE \"kind\" = 'A')"),
            std::string::npos)
      << "expected SUM FILTER for 'A'; got: " << sql;
  EXPECT_NE(sql.find("SUM(\"amount\") FILTER (WHERE \"kind\" = 'B')"),
            std::string::npos)
      << "expected SUM FILTER for 'B'; got: " << sql;
  EXPECT_NE(sql.find(" GROUP BY \"region\""), std::string::npos)
      << "expected GROUP BY on the pass-through grouping column; got: " << sql;
}

TEST_F(TranspilerTest, EmitPivotScanMultipleAggregates) {
  // Two pivot expressions x two pivot values = four output columns.
  // The analyzer reuses one pivot_expr_list / pivot_value_list and
  // tags each output column with (expr_index, value_index); the emit
  // pulls the cached aggregate SQL for each index and pairs it with
  // the matching value's filter clause.
  const ::googlesql::ResolvedStatement* stmt = Analyze(
      "SELECT * FROM sales "
      "PIVOT(SUM(amount) AS s, COUNT(*) AS c FOR kind IN ('A', 'B'))");
  const ::googlesql::ResolvedScan* scan = QueryInputScan(stmt);
  ASSERT_NE(scan, nullptr);
  ASSERT_EQ(scan->node_kind(), ::googlesql::RESOLVED_PIVOT_SCAN);
  TestTranspiler t;
  std::string sql =
      t.EmitPivotScan(scan->GetAs<::googlesql::ResolvedPivotScan>());
  EXPECT_NE(sql.find("SUM(\"amount\") FILTER (WHERE \"kind\" = 'A')"),
            std::string::npos)
      << sql;
  EXPECT_NE(sql.find("SUM(\"amount\") FILTER (WHERE \"kind\" = 'B')"),
            std::string::npos)
      << sql;
  EXPECT_NE(sql.find("COUNT(*) FILTER (WHERE \"kind\" = 'A')"),
            std::string::npos)
      << sql;
  EXPECT_NE(sql.find("COUNT(*) FILTER (WHERE \"kind\" = 'B')"),
            std::string::npos)
      << sql;
}

TEST_F(TranspilerTest, EmitUnpivotScanExcludeNullsByDefault) {
  // BigQuery's default UNPIVOT semantic (EXCLUDE NULLS). The emit
  // expands each input row via CROSS JOIN LATERAL (VALUES ...); the
  // outer WHERE filters out rows where every value column is NULL.
  // The label column is the analyzer's string label_list[i].
  const ::googlesql::ResolvedStatement* stmt =
      Analyze("SELECT * FROM wide UNPIVOT(value FOR quarter IN (q1, q2))");
  const ::googlesql::ResolvedScan* scan = QueryInputScan(stmt);
  ASSERT_NE(scan, nullptr);
  ASSERT_EQ(scan->node_kind(), ::googlesql::RESOLVED_UNPIVOT_SCAN);
  TestTranspiler t;
  std::string sql =
      t.EmitUnpivotScan(scan->GetAs<::googlesql::ResolvedUnpivotScan>());
  EXPECT_NE(sql.find(" CROSS JOIN LATERAL (VALUES "), std::string::npos)
      << "expected LATERAL VALUES unpivot expansion; got: " << sql;
  EXPECT_NE(sql.find("'q1'"), std::string::npos)
      << "expected q1 label in VALUES tuple; got: " << sql;
  EXPECT_NE(sql.find("'q2'"), std::string::npos)
      << "expected q2 label in VALUES tuple; got: " << sql;
  EXPECT_NE(sql.find("WHERE NOT (u.\"value\" IS NULL)"), std::string::npos)
      << "expected EXCLUDE NULLS filter; got: " << sql;
}

TEST_F(TranspilerTest, EmitWithScanRecursiveHierarchyTraversal) {
  // Mirrors `conformance/fixtures/advanced_relational/recursive_cte.yaml`:
  // anchor arm projects a literal depth column (`0 AS depth`) whose
  // analyzer name is a synthesized `$colN`; the recursive arm joins
  // the CTE ref back to `org` and increments depth.
  const ::googlesql::ResolvedStatement* stmt = Analyze(
      "WITH RECURSIVE descents AS ("
      "  SELECT employee, 0 AS depth FROM org WHERE manager IS NULL"
      "  UNION ALL"
      "  SELECT org.employee, depth + 1"
      "    FROM descents JOIN org AS org ON org.manager = descents.employee"
      ")"
      "SELECT employee, depth FROM descents ORDER BY depth, employee");
  ASSERT_NE(stmt, nullptr);
  TestTranspiler t;
  std::string sql = t.Transpile(stmt);
  EXPECT_NE(sql.find("WITH RECURSIVE \"descents\""), std::string::npos) << sql;
  EXPECT_NE(sql.find("\"_cte_0\""), std::string::npos) << sql;
  EXPECT_NE(sql.find("\"_cte_1\""), std::string::npos) << sql;
  EXPECT_NE(sql.find(" UNION ALL "), std::string::npos) << sql;
  EXPECT_NE(sql.find("FROM \"descents\""), std::string::npos) << sql;
  EXPECT_NE(sql.find("\"employee\" AS \"_cte_0\""), std::string::npos) << sql;
  EXPECT_NE(sql.find("\"depth\" AS \"_cte_1\""), std::string::npos) << sql;
}

TEST_F(TranspilerTest, EmitWithScanRecursiveLowersToWithRecursive) {
  // `docs/ENGINE_POLICY.md` Family 4. A `WITH
  // RECURSIVE t AS (SELECT 1 AS n UNION ALL SELECT n FROM t) ...`
  // lowers to DuckDB's `WITH RECURSIVE`. The transpiler stages a
  // per-CTE context with stable anchor column names (`_cte_0`),
  // renames each arm's output columns onto those anchors, and
  // unions them; the recursive-arm `ResolvedRecursiveRefScan`
  // projects from the anchor names back to the analyzer's per-ref
  // column names.
  //
  // The recursive arm here is just `SELECT n FROM t` (no
  // `WHERE ... < ...`) so the test does not depend on the `$less`
  // operator's disposition entry, which is owned by a later
  // function-dispatch plan.
  const ::googlesql::ResolvedStatement* stmt = Analyze(
      "WITH RECURSIVE t AS ("
      "  SELECT 1 AS n"
      "  UNION ALL"
      "  SELECT n FROM t"
      ")"
      "SELECT n FROM t");
  ASSERT_NE(stmt, nullptr);
  TestTranspiler t;
  std::string sql = t.Transpile(stmt);
  EXPECT_NE(sql.find("WITH RECURSIVE \"t\""), std::string::npos)
      << "expected WITH RECURSIVE keyword; got: " << sql;
  EXPECT_NE(sql.find("\"_cte_0\""), std::string::npos)
      << "expected anchor column name; got: " << sql;
  EXPECT_NE(sql.find(" UNION ALL "), std::string::npos)
      << "expected UNION ALL between anchor and recursive arm; got: " << sql;
  EXPECT_NE(sql.find("FROM \"t\""), std::string::npos)
      << "expected recursive ref to read FROM the CTE; got: " << sql;
}

TEST_F(TranspilerTest, EmitUnpivotScanIncludeNullsSkipsFilter) {
  // `INCLUDE NULLS` flips `include_nulls()` true; the outer query
  // should not carry the WHERE filter.
  const ::googlesql::ResolvedStatement* stmt = Analyze(
      "SELECT * FROM wide "
      "UNPIVOT INCLUDE NULLS (value FOR quarter IN (q1, q2))");
  const ::googlesql::ResolvedScan* scan = QueryInputScan(stmt);
  ASSERT_NE(scan, nullptr);
  ASSERT_EQ(scan->node_kind(), ::googlesql::RESOLVED_UNPIVOT_SCAN);
  TestTranspiler t;
  std::string sql =
      t.EmitUnpivotScan(scan->GetAs<::googlesql::ResolvedUnpivotScan>());
  EXPECT_EQ(sql.find("WHERE NOT"), std::string::npos)
      << "INCLUDE NULLS should omit the WHERE filter; got: " << sql;
  EXPECT_NE(sql.find(" CROSS JOIN LATERAL (VALUES "), std::string::npos) << sql;
}

TEST_F(TranspilerTest, EmitAggregateScanGroupingCallProjectsBitMask) {
  // `SELECT GROUPING(id), ... FROM ... GROUP BY ROLLUP(id, name)`
  // creates a `ResolvedGroupingCall` in `grouping_call_list`. The
  // emit projects it as `GROUPING("id") AS "<output>"` so the
  // SELECT list exposes the bit at the analyzer-chosen output
  // column name.
  const ::googlesql::ResolvedStatement* stmt = Analyze(
      "SELECT id, name, GROUPING(id), COUNT(*) FROM people "
      "GROUP BY ROLLUP(id, name)");
  const ::googlesql::ResolvedScan* scan = QueryInputScan(stmt);
  ASSERT_NE(scan, nullptr);
  ASSERT_EQ(scan->node_kind(), ::googlesql::RESOLVED_AGGREGATE_SCAN);
  TestTranspiler t;
  std::string sql =
      t.EmitAggregateScan(scan->GetAs<::googlesql::ResolvedAggregateScan>());
  EXPECT_NE(sql.find("GROUPING(\"id\") AS "), std::string::npos)
      << "expected GROUPING projection; got: " << sql;
}

TEST_F(TranspilerTest, EmitAggregateScanSumFilterClause) {
  const ::googlesql::ResolvedStatement* stmt =
      Analyze("SELECT SUM(amount WHERE kind = 'A') FROM sales");
  const ::googlesql::ResolvedScan* scan = QueryInputScan(stmt);
  ASSERT_NE(scan, nullptr);
  TestTranspiler t;
  std::string sql =
      t.EmitAggregateScan(scan->GetAs<::googlesql::ResolvedAggregateScan>());
  EXPECT_NE(sql.find("SUM(\"amount\") FILTER (WHERE"), std::string::npos)
      << sql;
  EXPECT_NE(sql.find("\"kind\" = 'A'"), std::string::npos) << sql;
}

TEST_F(TranspilerTest, EmitAggregateScanArrayAggIgnoreNulls) {
  const ::googlesql::ResolvedStatement* stmt = Analyze(
      "SELECT ARRAY_AGG(x IGNORE NULLS) FROM UNNEST([1, NULL, 2]) AS x");
  const ::googlesql::ResolvedScan* scan = QueryInputScan(stmt);
  ASSERT_NE(scan, nullptr);
  TestTranspiler t;
  std::string sql =
      t.EmitAggregateScan(scan->GetAs<::googlesql::ResolvedAggregateScan>());
  EXPECT_NE(sql.find("FILTER (WHERE \"x\" IS NOT NULL)"), std::string::npos)
      << sql;
}

// --- Order By -----------------------------------------------------------

}  // namespace transpiler
}  // namespace duckdb
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
