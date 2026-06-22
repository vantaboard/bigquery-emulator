// LocalCoordinatorEngine integration tests for user-reported query fixes.

#include "backend/engine/coordinator/local_coordinator_engine_test_fixture.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace coordinator {
namespace {

TEST_F(LocalCoordinatorEngineTest,
       ExecuteQueryTimestampParameterIsoWithoutTimezone) {
  CatalogBundle bundle = MakeCatalog();
  QueryRequest req = MakeRequest("SELECT @reference_dt");
  QueryParameter p;
  p.name = "reference_dt";
  p.type_kind = "TIMESTAMP";
  p.value_json = "2026-06-22T10:00:00";
  req.parameters.push_back(p);
  auto source = engine_->ExecuteQuery(req, bundle.catalog.get());
  ASSERT_TRUE(source.ok()) << source.status();
  storage::Row row;
  auto has = (*source)->Next(&row);
  ASSERT_TRUE(has.ok()) << has.status();
  ASSERT_TRUE(*has);
  ASSERT_EQ(row.cells.size(), 1u);
  ASSERT_EQ(row.cells[0].kind(), storage::Value::Kind::kString);
  EXPECT_EQ(row.cells[0].string_value(), "2026-06-22 10:00:00+00");
}

TEST_F(LocalCoordinatorEngineTest, ExecuteQueryUnionDistinctInCteRoutesSemantic) {
  CatalogBundle bundle = MakeCatalog();
  const char* sql = R"(
WITH active AS (
  SELECT id FROM UNNEST([1, 1, 2, 3]) AS id
  UNION DISTINCT
  SELECT id FROM UNNEST([3, 4]) AS id
)
SELECT
  (SELECT COUNT(DISTINCT id) FROM active) AS active_count,
  JSON_EXTRACT_SCALAR('{"x": "1"}', '$.x') AS dummy
)";
  auto source = engine_->ExecuteQuery(MakeRequest(sql), bundle.catalog.get());
  ASSERT_TRUE(source.ok()) << source.status();
  storage::Row row;
  auto has = (*source)->Next(&row);
  ASSERT_TRUE(has.ok()) << has.status();
  ASSERT_TRUE(*has);
  ASSERT_EQ(row.cells.size(), 2u);
  EXPECT_EQ(row.cells[0].int64_value(), 4);
  EXPECT_EQ(row.cells[1].string_value(), "1");
}

TEST_F(LocalCoordinatorEngineTest,
       ExecuteQueryCteScalarSubqueryInSelectListRoutesSemantic) {
  CatalogBundle bundle = MakeCatalog();
  const char* sql = R"(
WITH p AS (
  SELECT 1 AS id
  UNION ALL
  SELECT 2 AS id
)
SELECT
  (SELECT COUNT(*) FROM p) AS cnt,
  JSON_EXTRACT_SCALAR('{"ok": true}', '$.ok') AS ok
)";
  auto source = engine_->ExecuteQuery(MakeRequest(sql), bundle.catalog.get());
  ASSERT_TRUE(source.ok()) << source.status();
  storage::Row row;
  auto has = (*source)->Next(&row);
  ASSERT_TRUE(has.ok()) << has.status();
  ASSERT_TRUE(*has);
  ASSERT_EQ(row.cells.size(), 2u);
  EXPECT_EQ(row.cells[0].int64_value(), 2);
  EXPECT_EQ(row.cells[1].string_value(), "true");
}

}  // namespace
}  // namespace coordinator
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
