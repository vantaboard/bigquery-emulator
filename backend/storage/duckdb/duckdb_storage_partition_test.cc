#include "backend/schema/schema.h"
#include "backend/storage/duckdb/duckdb_storage.h"
#include "backend/storage/duckdb/duckdb_storage_test_fixture.h"
#include "backend/storage/storage.h"
#include "gtest/gtest.h"

namespace bigquery_emulator {
namespace backend {
namespace storage {
namespace duckdb {
namespace {

TEST_F(DuckDBStorageTest, IngestionPartitionTypeSurvivesRestart) {
  const TableId table{"proj-1", "ds_1", "events"};
  schema::TableSchema schema = PeopleSchema();
  schema.ingestion_partition_type = "HOUR";
  {
    auto store_or = DuckDBStorage::Open(data_dir_.string());
    ASSERT_TRUE(store_or.ok()) << store_or.status();
    ASSERT_TRUE((*store_or)->CreateDataset({"proj-1", "ds_1"}, "US").ok());
    ASSERT_TRUE((*store_or)->CreateTable(table, schema).ok());
  }
  auto store_or = DuckDBStorage::Open(data_dir_.string());
  ASSERT_TRUE(store_or.ok()) << store_or.status();
  auto got = (*store_or)->GetSchema(table);
  ASSERT_TRUE(got.ok()) << got.status();
  EXPECT_EQ(got->ingestion_partition_type, "HOUR");
  EXPECT_EQ(got->columns.size(), schema.columns.size());
}

}  // namespace
}  // namespace duckdb
}  // namespace storage
}  // namespace backend
}  // namespace bigquery_emulator
