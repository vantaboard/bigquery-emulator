// Join materialization tests for `SemanticExecutor` (R17 hash
// equi-join). Every test asserts end results only: eligible shapes
// take `MaterializeHashEquiJoinRows`, the rest keep the nested loop,
// and both must produce identical rows. Fixture shared via
// `executor_test_fixture.h`.

#include <cstdint>
#include <string>
#include <utility>
#include <vector>

#include "backend/engine/semantic/executor.h"
#include "backend/engine/semantic/executor_test_fixture.h"
#include "backend/storage/storage.h"
#include "gtest/gtest.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace semantic {
namespace {

class SemanticExecutorJoinTest : public SemanticExecutorTest {
 protected:
  std::vector<storage::Row> Run(const std::string& sql) {
    const auto* stmt = Analyze(sql, MakeAnalyzerOptions());
    EXPECT_NE(stmt, nullptr);
    if (stmt == nullptr) return {};
    SemanticExecutor exec;
    auto source = exec.ExecuteQuery(MakeRequest(sql), *stmt, catalog_.get());
    EXPECT_TRUE(source.ok()) << source.status();
    std::vector<storage::Row> rows;
    if (!source.ok()) return rows;
    while (true) {
      storage::Row row;
      auto has = (*source)->Next(&row);
      EXPECT_TRUE(has.ok()) << has.status();
      if (!has.ok() || !*has) break;
      rows.push_back(std::move(row));
    }
    return rows;
  }
};

// INNER equi-join over STRING keys with duplicates on both sides:
// hash-path eligible; every key pair must appear with full
// multiplicity (2 x 2 = 4 rows for the duplicated key).
TEST_F(SemanticExecutorJoinTest, InnerEquiJoinDuplicateKeysKeepMultiplicity) {
  const std::string sql =
      "SELECT l.k, l.lv, r.rv FROM ("
      "  SELECT 'a' AS k, 1 AS lv UNION ALL SELECT 'a', 2 UNION ALL"
      "  SELECT 'b', 3 UNION ALL SELECT 'z', 9"
      ") l JOIN ("
      "  SELECT 'a' AS k, 10 AS rv UNION ALL SELECT 'a', 20 UNION ALL"
      "  SELECT 'b', 30"
      ") r ON l.k = r.k "
      "ORDER BY l.k, l.lv, r.rv";
  std::vector<storage::Row> rows = Run(sql);
  ASSERT_EQ(rows.size(), 5u);  // 'a': 2x2 = 4, 'b': 1, 'z': unmatched.
  const std::vector<std::vector<int64_t>> want = {
      {1, 10}, {1, 20}, {2, 10}, {2, 20}, {3, 30}};
  for (size_t i = 0; i < want.size(); ++i) {
    EXPECT_EQ(rows[i].cells[1].int64_value(), want[i][0]) << "row " << i;
    EXPECT_EQ(rows[i].cells[2].int64_value(), want[i][1]) << "row " << i;
  }
}

// LEFT equi-join: unmatched left rows null-extend, and NULL join
// keys never match (SQL equality), including NULL-vs-NULL.
TEST_F(SemanticExecutorJoinTest, LeftEquiJoinNullExtendsAndNullKeysMiss) {
  const std::string sql =
      "SELECT l.id, r.rv FROM ("
      "  SELECT 1 AS id, 'a' AS k UNION ALL"
      "  SELECT 2, CAST(NULL AS STRING) UNION ALL"
      "  SELECT 3, 'missing'"
      ") l LEFT JOIN ("
      "  SELECT 'a' AS k, 10 AS rv UNION ALL"
      "  SELECT CAST(NULL AS STRING), 99"
      ") r ON l.k = r.k "
      "ORDER BY l.id";
  std::vector<storage::Row> rows = Run(sql);
  ASSERT_EQ(rows.size(), 3u);
  EXPECT_EQ(rows[0].cells[1].int64_value(), 10);
  // Row 2 has a NULL key: must NOT match the NULL-keyed right row.
  EXPECT_TRUE(rows[1].cells[1].is_null());
  EXPECT_TRUE(rows[2].cells[1].is_null());
}

// Equality keys plus a non-equality residual conjunct: the residual
// must filter candidate pairs after the hash probe, and a left row
// whose candidates all fail the residual must still null-extend.
TEST_F(SemanticExecutorJoinTest, LeftEquiJoinResidualFiltersCandidates) {
  const std::string sql =
      "SELECT l.id, r.rv FROM ("
      "  SELECT 1 AS id, 'a' AS k, 5 AS lo UNION ALL"
      "  SELECT 2, 'a', 50"
      ") l LEFT JOIN ("
      "  SELECT 'a' AS k, 10 AS rv UNION ALL SELECT 'a', 60"
      ") r ON l.k = r.k AND r.rv > l.lo "
      "ORDER BY l.id, r.rv";
  std::vector<storage::Row> rows = Run(sql);
  ASSERT_EQ(rows.size(), 3u);
  // id=1 (lo=5): both rv=10 and rv=60 pass. id=2 (lo=50): only rv=60.
  EXPECT_EQ(rows[0].cells[1].int64_value(), 10);
  EXPECT_EQ(rows[1].cells[1].int64_value(), 60);
  EXPECT_EQ(rows[2].cells[1].int64_value(), 60);
}

// Composite (two-column) hash key: both equalities must hold.
TEST_F(SemanticExecutorJoinTest, InnerEquiJoinCompositeKey) {
  const std::string sql =
      "SELECT l.v, r.w FROM ("
      "  SELECT 't1' AS tenant, 1 AS id, 100 AS v UNION ALL"
      "  SELECT 't1', 2, 200 UNION ALL"
      "  SELECT 't2', 1, 300"
      ") l JOIN ("
      "  SELECT 't1' AS tenant, 1 AS id, 111 AS w UNION ALL"
      "  SELECT 't2', 1, 333"
      ") r ON l.tenant = r.tenant AND l.id = r.id "
      "ORDER BY l.v";
  std::vector<storage::Row> rows = Run(sql);
  ASSERT_EQ(rows.size(), 2u);
  EXPECT_EQ(rows[0].cells[0].int64_value(), 100);
  EXPECT_EQ(rows[0].cells[1].int64_value(), 111);
  EXPECT_EQ(rows[1].cells[0].int64_value(), 300);
  EXPECT_EQ(rows[1].cells[1].int64_value(), 333);
}

// Non-equality join (no hashable equi conjunct) keeps the nested
// loop and still produces correct LEFT OUTER rows.
TEST_F(SemanticExecutorJoinTest, NonEquiLeftJoinUsesNestedLoop) {
  const std::string sql =
      "SELECT l.id, r.rv FROM ("
      "  SELECT 1 AS id, 5 AS k UNION ALL"
      "  SELECT 2, 50"
      ") l LEFT JOIN ("
      "  SELECT 10 AS rv UNION ALL SELECT 60"
      ") r ON r.rv > l.k "
      "ORDER BY l.id, r.rv";
  std::vector<storage::Row> rows = Run(sql);
  ASSERT_EQ(rows.size(), 3u);
  // id=1 (k=5): both 10 and 60. id=2 (k=50): only 60.
  EXPECT_EQ(rows[0].cells[1].int64_value(), 10);
  EXPECT_EQ(rows[1].cells[1].int64_value(), 60);
  EXPECT_EQ(rows[2].cells[1].int64_value(), 60);
}

// Swapped-side equality (`r.k = l.k`) must still hash-plan; result
// parity with the canonical order.
TEST_F(SemanticExecutorJoinTest, InnerEquiJoinSwappedEqualitySides) {
  const std::string sql =
      "SELECT l.lv, r.rv FROM ("
      "  SELECT 1 AS k, 100 AS lv UNION ALL SELECT 2, 200"
      ") l JOIN ("
      "  SELECT 1 AS k, 11 AS rv UNION ALL SELECT 3, 33"
      ") r ON r.k = l.k "
      "ORDER BY l.lv";
  std::vector<storage::Row> rows = Run(sql);
  ASSERT_EQ(rows.size(), 1u);
  EXPECT_EQ(rows[0].cells[0].int64_value(), 100);
  EXPECT_EQ(rows[0].cells[1].int64_value(), 11);
}

}  // namespace
}  // namespace semantic
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
