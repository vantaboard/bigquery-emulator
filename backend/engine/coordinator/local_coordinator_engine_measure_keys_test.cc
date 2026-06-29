// Measure-function and KEYS stub integration tests for LocalCoordinatorEngine.

#include <gtest/gtest.h>

#include <cstddef>
#include <cstdint>

#include "backend/engine/coordinator/local_coordinator_engine_test_fixture.h"
#include "backend/schema/schema.h"
#include "backend/storage/storage.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace coordinator {
namespace {

TEST_F(LocalCoordinatorEngineTest, AnalyzeMeasureAggGroupByQueryCompletes) {
  schema::TableSchema bq_schema;
  schema::ColumnSchema store_id;
  store_id.name = "store_id";
  store_id.type = schema::ColumnType::kInt64;
  store_id.mode = schema::ColumnMode::kRequired;
  bq_schema.columns.push_back(store_id);
  schema::ColumnSchema region_id;
  region_id.name = "region_id";
  region_id.type = schema::ColumnType::kInt64;
  region_id.mode = schema::ColumnMode::kRequired;
  bq_schema.columns.push_back(region_id);
  schema::ColumnSchema amount;
  amount.name = "amount";
  amount.type = schema::ColumnType::kInt64;
  amount.mode = schema::ColumnMode::kRequired;
  bq_schema.columns.push_back(amount);
  schema::ColumnSchema total_sales;
  total_sales.name = "total_sales";
  total_sales.type = schema::ColumnType::kInt64;
  total_sales.description = "bqemu_measure:SUM(amount):store_id";
  bq_schema.columns.push_back(total_sales);

  ASSERT_TRUE(storage_->CreateDataset({"proj-test", "ds_measure"}, "US").ok());
  ASSERT_TRUE(
      storage_->CreateTable({"proj-test", "ds_measure", "sales"}, bq_schema)
          .ok());

  CatalogBundle bundle = MakeCatalog();
  auto analyzed = AnalyzeStatementImpl(MakeRequest(R"(
        SELECT region_id, AGG(total_sales) AS region_total
        FROM ds_measure.sales
        GROUP BY region_id
        ORDER BY region_id
      )"),
                                       bundle.catalog.get(),
                                       /*all_statements=*/false);
  ASSERT_TRUE(analyzed.ok()) << analyzed.status();
  ASSERT_NE((*analyzed)->resolved_statement(), nullptr);
}

TEST_F(LocalCoordinatorEngineTest,
       ExecuteQueryMeasureAggGroupByRollsUpViaSemantic) {
  schema::TableSchema bq_schema;
  schema::ColumnSchema store_id;
  store_id.name = "store_id";
  store_id.type = schema::ColumnType::kInt64;
  store_id.mode = schema::ColumnMode::kRequired;
  bq_schema.columns.push_back(store_id);
  schema::ColumnSchema region_id;
  region_id.name = "region_id";
  region_id.type = schema::ColumnType::kInt64;
  region_id.mode = schema::ColumnMode::kRequired;
  bq_schema.columns.push_back(region_id);
  schema::ColumnSchema amount;
  amount.name = "amount";
  amount.type = schema::ColumnType::kInt64;
  amount.mode = schema::ColumnMode::kRequired;
  bq_schema.columns.push_back(amount);
  schema::ColumnSchema total_sales;
  total_sales.name = "total_sales";
  total_sales.type = schema::ColumnType::kInt64;
  total_sales.description = "bqemu_measure:SUM(amount):store_id";
  bq_schema.columns.push_back(total_sales);

  ASSERT_TRUE(storage_->CreateDataset({"proj-test", "ds_measure"}, "US").ok());
  ASSERT_TRUE(
      storage_->CreateTable({"proj-test", "ds_measure", "sales"}, bq_schema)
          .ok());
  auto make_row = [](int64_t store, int64_t region, int64_t amt) {
    storage::Row r;
    r.cells = {storage::Value::Int64(store),
               storage::Value::Int64(region),
               storage::Value::Int64(amt)};
    return r;
  };
  std::vector<storage::Row> rows = {
      make_row(1, 10, 100), make_row(1, 10, 50), make_row(2, 20, 200)};
  ASSERT_TRUE(storage_
                  ->AppendRows({"proj-test", "ds_measure", "sales"},
                               absl::MakeConstSpan(rows))
                  .ok());

  CatalogBundle bundle = MakeCatalog();
  auto source = engine_->ExecuteQuery(MakeRequest(R"(
        SELECT region_id, AGG(total_sales) AS region_total
        FROM ds_measure.sales
        GROUP BY region_id
        ORDER BY region_id
      )"),
                                      bundle.catalog.get());
  ASSERT_TRUE(source.ok()) << source.status();
  ASSERT_EQ((*source)->schema().columns.size(), 2u);

  auto read_int64 = [](storage::Row row, size_t idx) -> int64_t {
    EXPECT_EQ(row.cells[idx].kind(), storage::Value::Kind::kInt64);
    return row.cells[idx].int64_value();
  };

  storage::Row row;
  auto has = (*source)->Next(&row);
  ASSERT_TRUE(has.ok()) << has.status();
  ASSERT_TRUE(*has);
  EXPECT_EQ(read_int64(row, 0), 10);
  EXPECT_EQ(read_int64(row, 1), 150);

  has = (*source)->Next(&row);
  ASSERT_TRUE(has.ok()) << has.status();
  ASSERT_TRUE(*has);
  EXPECT_EQ(read_int64(row, 0), 20);
  EXPECT_EQ(read_int64(row, 1), 200);

  has = (*source)->Next(&row);
  ASSERT_TRUE(has.ok()) << has.status();
  EXPECT_FALSE(*has);
}

TEST_F(LocalCoordinatorEngineTest, KeysNewKeysetStubReturnsKeysetLengthOne) {
  CatalogBundle bundle = MakeCatalog();
  auto source = engine_->ExecuteQuery(
      MakeRequest(
          "SELECT KEYS.KEYSET_LENGTH(KEYS.NEW_KEYSET('AEAD_AES_GCM_256')) AS "
          "n"),
      bundle.catalog.get());
  ASSERT_TRUE(source.ok()) << source.status();
  storage::Row row;
  auto has = (*source)->Next(&row);
  ASSERT_TRUE(has.ok()) << has.status();
  ASSERT_TRUE(*has);
  ASSERT_EQ(row.cells[0].kind(), storage::Value::Kind::kInt64);
  EXPECT_EQ(row.cells[0].int64_value(), 1);
}

}  // namespace
}  // namespace coordinator
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
