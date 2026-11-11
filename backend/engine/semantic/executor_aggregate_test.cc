// Grouped ordered-aggregate tests for `SemanticExecutor`. Fixture shared
// via `executor_test_fixture.h`.

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

class SemanticExecutorAggregateTest : public SemanticExecutorTest {
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

// Regression: with more than one group, ARRAY_AGG read its ORDER BY keys
// and values by indexing every input row against the group's argument
// values, running past them and aborting the engine (a CHECK failure in
// `Value::Array`, or a segfault). Each group must aggregate only its rows.
TEST_F(SemanticExecutorAggregateTest,
       GroupedArrayAggOrderByLimitPicksPerGroup) {
  const std::string sql =
      "SELECT k, "
      "  ARRAY_AGG(v ORDER BY ts DESC, v DESC LIMIT 1)[SAFE_OFFSET(0)] AS v "
      "FROM ("
      "  SELECT 'a' AS k, 'entered' AS v, 1 AS ts UNION ALL"
      "  SELECT 'b', 'entered', 1 UNION ALL"
      "  SELECT 'a', 'exited', 2 UNION ALL"
      "  SELECT 'b', 'entered', 3 UNION ALL"
      "  SELECT 'b', 'exited', 3 UNION ALL"
      "  SELECT 'c', 'entered', 5"
      ") "
      "GROUP BY k ORDER BY k";
  std::vector<storage::Row> rows = Run(sql);
  ASSERT_EQ(rows.size(), 3u);
  const std::vector<std::pair<std::string, std::string>> want = {
      {"a", "exited"}, {"b", "exited"}, {"c", "entered"}};
  for (size_t i = 0; i < want.size(); ++i) {
    EXPECT_EQ(rows[i].cells[0].string_value(), want[i].first) << "row " << i;
    EXPECT_EQ(rows[i].cells[1].string_value(), want[i].second) << "row " << i;
  }
}

TEST_F(SemanticExecutorAggregateTest,
       GroupedArrayAggIgnoreNullsOrdersPerGroup) {
  const std::string sql =
      "SELECT k, ARRAY_AGG(IF(keep, v, NULL) IGNORE NULLS ORDER BY v) AS vs "
      "FROM ("
      "  SELECT 'a' AS k, 's2' AS v, TRUE AS keep UNION ALL"
      "  SELECT 'b', 's9', FALSE UNION ALL"
      "  SELECT 'a', 's1', TRUE UNION ALL"
      "  SELECT 'b', 's3', TRUE UNION ALL"
      "  SELECT 'a', 's0', FALSE"
      ") "
      "GROUP BY k ORDER BY k";
  std::vector<storage::Row> rows = Run(sql);
  ASSERT_EQ(rows.size(), 2u);
  const auto strings = [](const storage::Value& v) {
    std::vector<std::string> out;
    for (const storage::Value& e : v.array_value()) {
      out.push_back(e.string_value());
    }
    return out;
  };
  EXPECT_EQ(strings(rows[0].cells[1]), (std::vector<std::string>{"s1", "s2"}));
  EXPECT_EQ(strings(rows[1].cells[1]), (std::vector<std::string>{"s3"}));
}

}  // namespace
}  // namespace semantic
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
