#include <memory>
#include <vector>

#include "absl/status/status.h"
#include "absl/types/span.h"
#include "backend/storage/duckdb/duckdb_storage.h"
#include "backend/storage/duckdb/duckdb_storage_test_fixture.h"
#include "backend/storage/storage.h"
#include "gtest/gtest.h"

namespace bigquery_emulator {
namespace backend {
namespace storage {
namespace duckdb {
namespace {

TEST_F(DuckDBStorageTest, DatasetTombstoneRoundTripRestoresTables) {
  auto store_or = DuckDBStorage::Open(data_dir_.string());
  ASSERT_TRUE(store_or.ok()) << store_or.status();
  auto& store = **store_or;

  const DatasetId ds{"proj-1", "ds_undrop"};
  const TableId table{"proj-1", "ds_undrop", "people"};
  ASSERT_TRUE(store.CreateDataset(ds, "EU").ok());
  ASSERT_TRUE(store.CreateTable(table, PeopleSchema()).ok());
  std::vector<Row> rows;
  rows.push_back(MakePerson(1, "ada"));
  ASSERT_TRUE(store.AppendRows(table, absl::MakeConstSpan(rows)).ok());

  ASSERT_TRUE(store.DropDataset(ds, /*delete_contents=*/true).ok());
  auto list_after_drop = store.ListDatasets("proj-1");
  ASSERT_TRUE(list_after_drop.ok());
  EXPECT_TRUE(list_after_drop->empty());

  ASSERT_TRUE(store.RestoreDataset(ds).ok());
  auto list_after_restore = store.ListDatasets("proj-1");
  ASSERT_TRUE(list_after_restore.ok());
  ASSERT_EQ(list_after_restore->size(), 1u);

  auto iter_or = store.ScanRows(table);
  ASSERT_TRUE(iter_or.ok());
  Row r;
  auto has = (*iter_or)->Next(&r);
  ASSERT_TRUE(has.ok());
  ASSERT_TRUE(*has);
  EXPECT_EQ(r.cells[0].int64_value(), 1);
  EXPECT_EQ(r.cells[1].string_value(), "ada");
}

TEST_F(DuckDBStorageTest, RestoreDatasetOnLiveDatasetIsAlreadyExists) {
  auto store_or = DuckDBStorage::Open(data_dir_.string());
  ASSERT_TRUE(store_or.ok()) << store_or.status();
  auto& store = **store_or;

  const DatasetId ds{"proj-1", "ds_live"};
  ASSERT_TRUE(store.CreateDataset(ds, "US").ok());
  absl::Status restored = store.RestoreDataset(ds);
  ASSERT_FALSE(restored.ok());
  EXPECT_EQ(restored.code(), absl::StatusCode::kAlreadyExists);
}

}  // namespace
}  // namespace duckdb
}  // namespace storage
}  // namespace backend
}  // namespace bigquery_emulator
