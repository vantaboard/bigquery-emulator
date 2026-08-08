// Analytic numbering / navigation / aggregate-window tests for
// `SemanticExecutor` (R14 NTILE, R15 RANK/DENSE_RANK/LAG/LEAD,
// frame-aware SUM/AVG/MIN/MAX/COUNT). Fixture shared via
// `executor_test_fixture.h`.

#include <cstdint>
#include <string>
#include <utility>
#include <vector>

#include "absl/status/status.h"
#include "backend/engine/semantic/executor.h"
#include "backend/engine/semantic/executor_test_fixture.h"
#include "backend/storage/storage.h"
#include "googlesql/public/analyzer.h"
#include "googlesql/public/analyzer_options.h"
#include "gtest/gtest.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace semantic {
namespace {

// R14: NTILE over aggregate input (RFM shape) — semantic executor path.
TEST_F(SemanticExecutorTest, NtileOverAggregateInputUnevenBuckets) {
  const std::string sql =
      "WITH rfm_raw AS ("
      "  SELECT customer_id, SUM(amount) AS monetary FROM ("
      "    SELECT 1 AS customer_id, 100 AS amount UNION ALL"
      "    SELECT 2, 200 UNION ALL SELECT 3, 300 UNION ALL"
      "    SELECT 4, 400 UNION ALL SELECT 5, 500 UNION ALL"
      "    SELECT 6, 600 UNION ALL SELECT 7, 700"
      "  ) GROUP BY customer_id"
      "), rfm_scored AS ("
      "  SELECT customer_id, monetary,"
      "         NTILE(5) OVER (ORDER BY monetary ASC) AS m_score"
      "  FROM rfm_raw"
      ") "
      "SELECT customer_id, m_score FROM rfm_scored ORDER BY customer_id";
  const auto* stmt = Analyze(sql, MakeAnalyzerOptions());
  ASSERT_NE(stmt, nullptr);
  SemanticExecutor exec;
  auto source = exec.ExecuteQuery(MakeRequest(sql), *stmt, catalog_.get());
  ASSERT_TRUE(source.ok()) << source.status();
  // 7 rows into 5 buckets → sizes 2,2,1,1,1 for customers 1..7.
  const std::vector<int64_t> want_scores = {1, 1, 2, 2, 3, 4, 5};
  for (size_t i = 0; i < want_scores.size(); ++i) {
    storage::Row row;
    auto has = (*source)->Next(&row);
    ASSERT_TRUE(has.ok()) << has.status();
    ASSERT_TRUE(*has) << "missing row " << i;
    ASSERT_EQ(row.cells.size(), 2u);
    EXPECT_EQ(row.cells[0].int64_value(), static_cast<int64_t>(i + 1));
    EXPECT_EQ(row.cells[1].int64_value(), want_scores[i]) << "row " << i;
  }
  storage::Row extra;
  auto has_extra = (*source)->Next(&extra);
  ASSERT_TRUE(has_extra.ok());
  EXPECT_FALSE(*has_extra);
}

TEST_F(SemanticExecutorTest, NtileNonPositiveBucketsRejectedAtAnalyze) {
  // GoogleSQL rejects a constant non-positive NTILE argument before
  // execution; ApplyAnalyticNtile also guards the runtime path for
  // non-constant expressions that slip through.
  const std::string sql =
      "WITH rfm_raw AS ("
      "  SELECT customer_id, SUM(amount) AS monetary FROM ("
      "    SELECT 1 AS customer_id, 100 AS amount UNION ALL SELECT 2, 200"
      "  ) GROUP BY customer_id"
      ") "
      "SELECT NTILE(0) OVER (ORDER BY monetary ASC) AS m_score FROM rfm_raw";
  last_output_.reset();
  absl::Status s = ::googlesql::AnalyzeStatement(sql,
                                                 MakeAnalyzerOptions(),
                                                 catalog_.get(),
                                                 type_factory_.get(),
                                                 &last_output_);
  EXPECT_FALSE(s.ok());
  EXPECT_NE(std::string(s.message()).find("NTILE"), std::string::npos) << s;
}

// R15: RANK / DENSE_RANK over nested aggregate with ties.
TEST_F(SemanticExecutorTest, RankDenseRankOverNestedAggregateTies) {
  const std::string sql =
      "SELECT customer_id,"
      "       RANK() OVER (ORDER BY SUM(amount) DESC) AS rnk,"
      "       DENSE_RANK() OVER (ORDER BY SUM(amount) DESC) AS drnk "
      "FROM ("
      "  SELECT 1 AS customer_id, 100 AS amount UNION ALL"
      "  SELECT 1, 50 UNION ALL"
      "  SELECT 2, 100 UNION ALL"
      "  SELECT 2, 50 UNION ALL"
      "  SELECT 3, 200 UNION ALL"
      "  SELECT 4, 50"
      ") GROUP BY customer_id "
      "ORDER BY customer_id";
  const auto* stmt = Analyze(sql, MakeAnalyzerOptions());
  ASSERT_NE(stmt, nullptr);
  SemanticExecutor exec;
  auto source = exec.ExecuteQuery(MakeRequest(sql), *stmt, catalog_.get());
  ASSERT_TRUE(source.ok()) << source.status();
  // Sums DESC: 200,150,150,50 → RANK 1,2,2,4 and DENSE_RANK 1,2,2,3.
  // Ordered by customer_id: c1=150→2/2, c2=150→2/2, c3=200→1/1, c4=50→4/3.
  const std::vector<std::pair<int64_t, int64_t>> want = {
      {2, 2}, {2, 2}, {1, 1}, {4, 3}};
  for (size_t i = 0; i < want.size(); ++i) {
    storage::Row row;
    auto has = (*source)->Next(&row);
    ASSERT_TRUE(has.ok()) << has.status();
    ASSERT_TRUE(*has) << "missing row " << i;
    ASSERT_EQ(row.cells.size(), 3u);
    EXPECT_EQ(row.cells[0].int64_value(), static_cast<int64_t>(i + 1));
    EXPECT_EQ(row.cells[1].int64_value(), want[i].first) << "rank row " << i;
    EXPECT_EQ(row.cells[2].int64_value(), want[i].second)
        << "dense_rank row " << i;
  }
  storage::Row extra;
  auto has_extra = (*source)->Next(&extra);
  ASSERT_TRUE(has_extra.ok());
  EXPECT_FALSE(*has_extra);
}

// Analytic output is stabilized on PARTITION BY + ORDER BY (no outer
// ORDER BY), matching DuckDB CaptureAnalyticOutputOrder. Pins the
// gateway e2e LEAD/NTILE finishers shape when FORMAT_TIMESTAMP promotes
// the query onto the semantic route.
TEST_F(SemanticExecutorTest, LeadOutputOrderFollowsPartitionAndOrderBy) {
  const std::string sql =
      "WITH finishers AS ("
      "  SELECT 'Sophia' AS name, TIMESTAMP '2016-10-18 02:51:45+00' AS "
      "finish_time, 'F30-34' AS division UNION ALL"
      "  SELECT 'Lisa', TIMESTAMP '2016-10-18 02:54:11+00', 'F35-39' UNION ALL"
      "  SELECT 'Nikki', TIMESTAMP '2016-10-18 02:59:01+00', 'F30-34' UNION ALL"
      "  SELECT 'Carly', TIMESTAMP '2016-10-18 03:08:58+00', 'F25-29'"
      ") "
      "SELECT name, division,"
      "       LEAD(name) OVER (PARTITION BY division ORDER BY finish_time ASC) "
      "AS followed_by "
      "FROM finishers";
  const auto* stmt = Analyze(sql, MakeAnalyzerOptions());
  ASSERT_NE(stmt, nullptr);
  SemanticExecutor exec;
  auto source = exec.ExecuteQuery(MakeRequest(sql), *stmt, catalog_.get());
  ASSERT_TRUE(source.ok()) << source.status();
  struct Want {
    const char* name;
    const char* division;
    bool followed_null;
    const char* followed;
  };
  const std::vector<Want> want = {
      {"Carly", "F25-29", true, nullptr},
      {"Sophia", "F30-34", false, "Nikki"},
      {"Nikki", "F30-34", true, nullptr},
      {"Lisa", "F35-39", true, nullptr},
  };
  for (size_t i = 0; i < want.size(); ++i) {
    storage::Row row;
    auto has = (*source)->Next(&row);
    ASSERT_TRUE(has.ok()) << has.status();
    ASSERT_TRUE(*has) << "missing row " << i;
    ASSERT_EQ(row.cells.size(), 3u);
    EXPECT_EQ(row.cells[0].string_value(), want[i].name) << "row " << i;
    EXPECT_EQ(row.cells[1].string_value(), want[i].division) << "row " << i;
    if (want[i].followed_null) {
      EXPECT_TRUE(row.cells[2].is_null()) << "lead row " << i;
    } else {
      ASSERT_FALSE(row.cells[2].is_null()) << "lead row " << i;
      EXPECT_EQ(row.cells[2].string_value(), want[i].followed) << "row " << i;
    }
  }
  storage::Row extra;
  auto has_extra = (*source)->Next(&extra);
  ASSERT_TRUE(has_extra.ok());
  EXPECT_FALSE(*has_extra);
}

// R15: LAG / LEAD over nested aggregate with offset and default.
TEST_F(SemanticExecutorTest, LagLeadOverNestedAggregate) {
  const std::string sql =
      "SELECT customer_id,"
      "       LAG(SUM(amount), 1) OVER (ORDER BY customer_id) AS prev_m,"
      "       LEAD(SUM(amount), 2, 0) OVER (ORDER BY customer_id) AS next2_m "
      "FROM ("
      "  SELECT 1 AS customer_id, 100 AS amount UNION ALL"
      "  SELECT 2, 200 UNION ALL"
      "  SELECT 3, 300 UNION ALL"
      "  SELECT 4, 400"
      ") GROUP BY customer_id "
      "ORDER BY customer_id";
  const auto* stmt = Analyze(sql, MakeAnalyzerOptions());
  ASSERT_NE(stmt, nullptr);
  SemanticExecutor exec;
  auto source = exec.ExecuteQuery(MakeRequest(sql), *stmt, catalog_.get());
  ASSERT_TRUE(source.ok()) << source.status();
  // monetary: 100,200,300,400 → LAG1: NULL,100,200,300; LEAD2 default0:
  // 300,400,0,0
  struct Want {
    bool prev_null;
    int64_t prev;
    int64_t next2;
  };
  const std::vector<Want> want = {
      {true, 0, 300}, {false, 100, 400}, {false, 200, 0}, {false, 300, 0}};
  for (size_t i = 0; i < want.size(); ++i) {
    storage::Row row;
    auto has = (*source)->Next(&row);
    ASSERT_TRUE(has.ok()) << has.status();
    ASSERT_TRUE(*has) << "missing row " << i;
    ASSERT_EQ(row.cells.size(), 3u);
    EXPECT_EQ(row.cells[0].int64_value(), static_cast<int64_t>(i + 1));
    if (want[i].prev_null) {
      EXPECT_TRUE(row.cells[1].is_null()) << "lag row " << i;
    } else {
      ASSERT_FALSE(row.cells[1].is_null()) << "lag row " << i;
      EXPECT_EQ(row.cells[1].int64_value(), want[i].prev) << "lag row " << i;
    }
    ASSERT_FALSE(row.cells[2].is_null()) << "lead row " << i;
    EXPECT_EQ(row.cells[2].int64_value(), want[i].next2) << "lead row " << i;
  }
  storage::Row extra;
  auto has_extra = (*source)->Next(&extra);
  ASSERT_TRUE(has_extra.ok());
  EXPECT_FALSE(*has_extra);
}

// Whole-partition MAX/MIN/AVG/COUNT(*) over aggregate input.
TEST_F(SemanticExecutorTest, AggregateWindowsWholePartitionOverGroups) {
  const std::string sql =
      "SELECT customer_id,"
      "       SUM(amount) AS monetary,"
      "       MAX(SUM(amount)) OVER () AS max_m,"
      "       MIN(SUM(amount)) OVER () AS min_m,"
      "       AVG(SUM(amount)) OVER () AS avg_m,"
      "       COUNT(*) OVER () AS n_groups "
      "FROM ("
      "  SELECT 1 AS customer_id, 100 AS amount UNION ALL"
      "  SELECT 2, 200 UNION ALL"
      "  SELECT 3, 300"
      ") GROUP BY customer_id "
      "ORDER BY customer_id";
  const auto* stmt = Analyze(sql, MakeAnalyzerOptions());
  ASSERT_NE(stmt, nullptr);
  SemanticExecutor exec;
  auto source = exec.ExecuteQuery(MakeRequest(sql), *stmt, catalog_.get());
  ASSERT_TRUE(source.ok()) << source.status();
  // Group sums: 100,200,300 → max=300, min=100, avg=200, count=3.
  for (int64_t id = 1; id <= 3; ++id) {
    storage::Row row;
    auto has = (*source)->Next(&row);
    ASSERT_TRUE(has.ok()) << has.status();
    ASSERT_TRUE(*has) << "missing row " << id;
    ASSERT_EQ(row.cells.size(), 6u);
    EXPECT_EQ(row.cells[0].int64_value(), id);
    EXPECT_EQ(row.cells[1].int64_value(), id * 100);
    EXPECT_EQ(row.cells[2].int64_value(), 300);
    EXPECT_EQ(row.cells[3].int64_value(), 100);
    EXPECT_DOUBLE_EQ(row.cells[4].float64_value(), 200.0);
    EXPECT_EQ(row.cells[5].int64_value(), 3);
  }
  storage::Row extra;
  auto has_extra = (*source)->Next(&extra);
  ASSERT_TRUE(has_extra.ok());
  EXPECT_FALSE(*has_extra);
}

// Default RANGE frame running SUM with a peer tie.
TEST_F(SemanticExecutorTest, RunningSumOverNestedAggregateWithTies) {
  const std::string sql =
      "SELECT customer_id,"
      "       SUM(amount) AS monetary,"
      "       SUM(SUM(amount)) OVER (ORDER BY SUM(amount)) AS running "
      "FROM ("
      "  SELECT 1 AS customer_id, 100 AS amount UNION ALL"
      "  SELECT 2, 100 UNION ALL"
      "  SELECT 3, 200 UNION ALL"
      "  SELECT 4, 50"
      ") GROUP BY customer_id "
      "ORDER BY customer_id";
  const auto* stmt = Analyze(sql, MakeAnalyzerOptions());
  ASSERT_NE(stmt, nullptr);
  SemanticExecutor exec;
  auto source = exec.ExecuteQuery(MakeRequest(sql), *stmt, catalog_.get());
  ASSERT_TRUE(source.ok()) << source.status();
  // Ordered by monetary ASC: 50(c4), 100(c1), 100(c2), 200(c3).
  // Default RANGE UNBOUNDED PRECEDING .. CURRENT ROW includes peers, so
  // both 100-rows see 50+100+100=250 and the 200-row sees 450.
  // Ordered by customer_id: c1→250, c2→250, c3→450, c4→50.
  const std::vector<int64_t> want_running = {250, 250, 450, 50};
  for (size_t i = 0; i < want_running.size(); ++i) {
    storage::Row row;
    auto has = (*source)->Next(&row);
    ASSERT_TRUE(has.ok()) << has.status();
    ASSERT_TRUE(*has) << "missing row " << i;
    ASSERT_EQ(row.cells.size(), 3u);
    EXPECT_EQ(row.cells[0].int64_value(), static_cast<int64_t>(i + 1));
    EXPECT_EQ(row.cells[2].int64_value(), want_running[i])
        << "running row " << i;
  }
}

// Explicit ROWS frame: preceding + current only (no peer expansion).
TEST_F(SemanticExecutorTest, RowsFrameSumOverNestedAggregate) {
  const std::string sql =
      "SELECT customer_id,"
      "       SUM(SUM(amount)) OVER ("
      "         ORDER BY customer_id"
      "         ROWS BETWEEN 1 PRECEDING AND CURRENT ROW"
      "       ) AS win "
      "FROM ("
      "  SELECT 1 AS customer_id, 100 AS amount UNION ALL"
      "  SELECT 2, 200 UNION ALL"
      "  SELECT 3, 300 UNION ALL"
      "  SELECT 4, 400"
      ") GROUP BY customer_id "
      "ORDER BY customer_id";
  const auto* stmt = Analyze(sql, MakeAnalyzerOptions());
  ASSERT_NE(stmt, nullptr);
  SemanticExecutor exec;
  auto source = exec.ExecuteQuery(MakeRequest(sql), *stmt, catalog_.get());
  ASSERT_TRUE(source.ok()) << source.status();
  // Row sums 100,200,300,400 → windows: 100, 300, 500, 700.
  const std::vector<int64_t> want = {100, 300, 500, 700};
  for (size_t i = 0; i < want.size(); ++i) {
    storage::Row row;
    auto has = (*source)->Next(&row);
    ASSERT_TRUE(has.ok()) << has.status();
    ASSERT_TRUE(*has) << "missing row " << i;
    ASSERT_EQ(row.cells.size(), 2u);
    EXPECT_EQ(row.cells[0].int64_value(), static_cast<int64_t>(i + 1));
    EXPECT_EQ(row.cells[1].int64_value(), want[i]) << "rows frame row " << i;
  }
}

// Numeric RANGE value-offset COUNT (replaces the old CountRange lane;
// BigQuery requires a numeric ORDER BY for RANGE offsets).
TEST_F(SemanticExecutorTest, NumericRangeCountOverAggregateInput) {
  const std::string sql =
      "SELECT customer_id,"
      "       COUNT(*) OVER ("
      "         ORDER BY SUM(amount)"
      "         RANGE BETWEEN 1 PRECEDING AND CURRENT ROW"
      "       ) AS cnt "
      "FROM ("
      "  SELECT 1 AS customer_id, 10 AS amount UNION ALL"
      "  SELECT 2, 11 UNION ALL"
      "  SELECT 3, 12 UNION ALL"
      "  SELECT 4, 20"
      ") GROUP BY customer_id "
      "ORDER BY customer_id";
  const auto* stmt = Analyze(sql, MakeAnalyzerOptions());
  ASSERT_NE(stmt, nullptr);
  SemanticExecutor exec;
  auto source = exec.ExecuteQuery(MakeRequest(sql), *stmt, catalog_.get());
  ASSERT_TRUE(source.ok()) << source.status();
  // Sums 10,11,12,20 → RANGE ±1 preceding..current:
  // 10→{10}, 11→{10,11}, 12→{11,12}, 20→{20}.
  const std::vector<int64_t> want = {1, 2, 2, 1};
  for (size_t i = 0; i < want.size(); ++i) {
    storage::Row row;
    auto has = (*source)->Next(&row);
    ASSERT_TRUE(has.ok()) << has.status();
    ASSERT_TRUE(*has) << "missing row " << i;
    ASSERT_EQ(row.cells.size(), 2u);
    EXPECT_EQ(row.cells[0].int64_value(), static_cast<int64_t>(i + 1));
    EXPECT_EQ(row.cells[1].int64_value(), want[i]) << "range count row " << i;
  }
}

}  // namespace
}  // namespace semantic
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
