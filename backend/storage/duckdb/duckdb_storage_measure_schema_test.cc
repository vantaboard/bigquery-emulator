#include "absl/status/statusor.h"
#include "backend/schema/schema.h"
#include "backend/storage/duckdb/duckdb_storage.h"
#include "backend/storage/duckdb/duckdb_storage_test_fixture.h"
#include "gtest/gtest.h"

namespace bigquery_emulator {
namespace backend {
namespace storage {
namespace duckdb {
namespace {

TEST_F(DuckDBStorageTest, RoundTripsColumnDescription) {
  auto opened = DuckDBStorage::Open(data_dir_.string());
  ASSERT_TRUE(opened.ok()) << opened.status();
  auto& store = **opened;

  ASSERT_TRUE(store.CreateDataset({"proj", "ds"}, "US").ok());
  schema::TableSchema schema;
  schema::ColumnSchema measure;
  measure.name = "total_sales";
  measure.type = schema::ColumnType::kInt64;
  measure.description = "bqemu_measure:SUM(amount):store_id";
  schema.columns.push_back(measure);

  ASSERT_TRUE(store.CreateTable({"proj", "ds", "sales"}, schema).ok());
  absl::StatusOr<schema::TableSchema> roundtrip =
      store.GetSchema({"proj", "ds", "sales"});
  ASSERT_TRUE(roundtrip.ok()) << roundtrip.status();
  ASSERT_EQ(roundtrip->columns.size(), 1u);
  EXPECT_EQ(roundtrip->columns[0].description,
            "bqemu_measure:SUM(amount):store_id");
}

}  // namespace
}  // namespace duckdb
}  // namespace storage
}  // namespace backend
}  // namespace bigquery_emulator
