#include "backend/engine/duckdb/transpiler/transpiler_test_fixture.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace duckdb {
namespace transpiler {

// --- Window / Analytic --------------------------------------------------

TEST_F(TranspilerTest, EmitAnalyticScanRowNumber) {
  // `ROW_NUMBER() OVER (ORDER BY id)` lowers to a ResolvedAnalyticScan
  // whose only group has a null partition_by and a single-item
  // order_by; the analytic function list carries one
  // ResolvedAnalyticFunctionCall (`row_number`, no args). The
  // synthesized output column is `$analytic1`.
  const ::googlesql::ResolvedStatement* stmt =
      Analyze("SELECT ROW_NUMBER() OVER (ORDER BY id) FROM people");
  const ::googlesql::ResolvedScan* scan = QueryInputScan(stmt);
  ASSERT_NE(scan, nullptr);
  ASSERT_EQ(scan->node_kind(), ::googlesql::RESOLVED_ANALYTIC_SCAN);
  TestTranspiler t;
  const std::string kPeopleWithRn =
      "SELECT *, row_number() OVER () AS \"__bq_input_rn\" FROM (SELECT "
      "\"id\", \"name\" FROM \"people\")";
  EXPECT_EQ(
      t.EmitAnalyticScan(scan->GetAs<::googlesql::ResolvedAnalyticScan>()),
      "SELECT *, ROW_NUMBER() OVER (ORDER BY \"id\" ASC NULLS FIRST) AS "
      "\"$analytic1\" FROM (" +
          kPeopleWithRn + ")");
}

TEST_F(TranspilerTest, EmitAnalyticScanRankPartitionByOrderBy) {
  // `RANK() OVER (PARTITION BY name ORDER BY id DESC)` exercises both
  // the partition_by and the explicit-direction order_by paths. The
  // partition spec emits one PARTITION BY column and the order spec
  // emits the explicit DESC keyword.
  const ::googlesql::ResolvedStatement* stmt = Analyze(
      "SELECT RANK() OVER (PARTITION BY name ORDER BY id DESC) FROM people");
  const ::googlesql::ResolvedScan* scan = QueryInputScan(stmt);
  ASSERT_NE(scan, nullptr);
  ASSERT_EQ(scan->node_kind(), ::googlesql::RESOLVED_ANALYTIC_SCAN);
  TestTranspiler t;
  const std::string kPeopleWithRn =
      "SELECT *, row_number() OVER () AS \"__bq_input_rn\" FROM (SELECT "
      "\"id\", \"name\" FROM \"people\")";
  EXPECT_EQ(
      t.EmitAnalyticScan(scan->GetAs<::googlesql::ResolvedAnalyticScan>()),
      "SELECT *, RANK() OVER (PARTITION BY \"name\" ORDER BY \"id\" DESC "
      "NULLS LAST) AS \"$analytic1\" FROM (" +
          kPeopleWithRn + ")");
}

TEST_F(TranspilerTest, EmitAnalyticScanDenseRank) {
  // DENSE_RANK is the third ranking analytic the plan calls out; the
  // test mirrors RANK so we get explicit coverage of the disposition
  // row in `functions.yaml` (`dense_rank: DENSE_RANK`).
  const ::googlesql::ResolvedStatement* stmt =
      Analyze("SELECT DENSE_RANK() OVER (ORDER BY id) FROM people");
  const ::googlesql::ResolvedScan* scan = QueryInputScan(stmt);
  ASSERT_NE(scan, nullptr);
  ASSERT_EQ(scan->node_kind(), ::googlesql::RESOLVED_ANALYTIC_SCAN);
  TestTranspiler t;
  const std::string kPeopleWithRn =
      "SELECT *, row_number() OVER () AS \"__bq_input_rn\" FROM (SELECT "
      "\"id\", \"name\" FROM \"people\")";
  EXPECT_EQ(
      t.EmitAnalyticScan(scan->GetAs<::googlesql::ResolvedAnalyticScan>()),
      "SELECT *, DENSE_RANK() OVER (ORDER BY \"id\" ASC NULLS FIRST) AS "
      "\"$analytic1\" FROM (" +
          kPeopleWithRn + ")");
}

TEST_F(TranspilerTest, EmitAnalyticScanSumOverWithFrame) {
  // Aggregate-over-window with an explicit ROWS frame. SUM is a
  // `kDuckdbNative` entry shared with the scalar aggregate emit, so
  // the
  // analytic path renders it the same way (`SUM(<expr>)`) and the
  // OVER clause carries the ROWS BETWEEN bound. UNBOUNDED PRECEDING
  // / CURRENT ROW are both supported boundary types.
  const ::googlesql::ResolvedStatement* stmt = Analyze(
      "SELECT SUM(id) OVER (ORDER BY id ROWS BETWEEN UNBOUNDED PRECEDING "
      "AND CURRENT ROW) FROM people");
  const ::googlesql::ResolvedScan* scan = QueryInputScan(stmt);
  ASSERT_NE(scan, nullptr);
  ASSERT_EQ(scan->node_kind(), ::googlesql::RESOLVED_ANALYTIC_SCAN);
  TestTranspiler t;
  const std::string kPeopleWithRn =
      "SELECT *, row_number() OVER () AS \"__bq_input_rn\" FROM (SELECT "
      "\"id\", \"name\" FROM \"people\")";
  EXPECT_EQ(
      t.EmitAnalyticScan(scan->GetAs<::googlesql::ResolvedAnalyticScan>()),
      "SELECT *, SUM(\"id\") OVER (ORDER BY \"id\" ASC NULLS FIRST ROWS "
      "BETWEEN UNBOUNDED PRECEDING AND CURRENT ROW) AS \"$analytic1\" FROM (" +
          kPeopleWithRn + ")");
}

TEST_F(TranspilerTest, EmitAnalyticScanCountStarOverPartition) {
  // COUNT(*) lowers through the `$count_star` special case both in
  // the aggregate path and in the analytic path -- the analyzer
  // gives us an empty argument_list and the function name
  // `$count_star`. With a PARTITION-BY-only OVER clause the analyzer
  // synthesizes a `ROWS BETWEEN UNBOUNDED PRECEDING AND UNBOUNDED
  // FOLLOWING` frame for aggregate analytic functions, so the emit
  // surfaces that frame even though the user didn't spell it.
  const ::googlesql::ResolvedStatement* stmt =
      Analyze("SELECT COUNT(*) OVER (PARTITION BY name) FROM people");
  const ::googlesql::ResolvedScan* scan = QueryInputScan(stmt);
  ASSERT_NE(scan, nullptr);
  ASSERT_EQ(scan->node_kind(), ::googlesql::RESOLVED_ANALYTIC_SCAN);
  TestTranspiler t;
  const std::string kPeopleWithRn =
      "SELECT *, row_number() OVER () AS \"__bq_input_rn\" FROM (SELECT "
      "\"id\", \"name\" FROM \"people\")";
  EXPECT_EQ(
      t.EmitAnalyticScan(scan->GetAs<::googlesql::ResolvedAnalyticScan>()),
      "SELECT *, COUNT(*) OVER (PARTITION BY \"name\" ROWS BETWEEN "
      "UNBOUNDED PRECEDING AND UNBOUNDED FOLLOWING) AS \"$analytic1\" FROM (" +
          kPeopleWithRn + ")");
}

TEST_F(TranspilerTest, EmitAnalyticScanSafeAggregateFallsBack) {
  // `SAFE.SUM(id) OVER (ORDER BY id)` analyzes cleanly (SAFE is a
  // function-call decoration, not an OVER-time modifier) but sets
  // `error_mode = SAFE_ERROR_MODE`. The per-call SAFE short-circuit
  // returns "" and the analytic emit propagates the empty string,
  // so the engine surfaces UNIMPLEMENTED.
  const ::googlesql::ResolvedStatement* stmt =
      Analyze("SELECT SAFE.SUM(id) OVER (ORDER BY id) FROM people");
  if (stmt == nullptr) {
    GTEST_SKIP() << "analyzer rejected SAFE aggregate OVER -- skip";
  }
  const ::googlesql::ResolvedScan* scan = QueryInputScan(stmt);
  if (scan == nullptr ||
      scan->node_kind() != ::googlesql::RESOLVED_ANALYTIC_SCAN) {
    GTEST_SKIP() << "analyzer produced non-analytic scan -- skip";
  }
  TestTranspiler t;
  EXPECT_EQ(
      t.EmitAnalyticScan(scan->GetAs<::googlesql::ResolvedAnalyticScan>()), "");
}

// --- Top-level SELECT (QueryStmt / ProjectScan / SingleRowScan /
//     OutputColumn / ComputedColumn) -----------------------------------


}  // namespace transpiler
}  // namespace duckdb
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
