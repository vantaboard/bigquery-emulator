#include <filesystem>
#include <memory>
#include <string>
#include <vector>

#include "absl/status/status.h"
#include "absl/strings/str_cat.h"
#include "absl/types/span.h"
#include "backend/storage/duckdb/duckdb_storage.h"
#include "backend/storage/duckdb/duckdb_storage_internal.h"
#include "backend/storage/duckdb/duckdb_storage_test_fixture.h"
#include "backend/storage/duckdb/duckdb_storage_version_log.h"
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

TEST_F(DuckDBStorageTest, DatasetTombstoneRoundTripRestoresViewsAndRoutines) {
  auto store_or = DuckDBStorage::Open(data_dir_.string());
  ASSERT_TRUE(store_or.ok()) << store_or.status();
  auto& store = **store_or;

  const DatasetId ds{"proj-1", "ds_registry"};
  ASSERT_TRUE(store.CreateDataset(ds, "US").ok());

  ViewRecord view;
  view.id = ViewId{"proj-1", "ds_registry", "v_items"};
  view.ddl_sql = "CREATE VIEW ds_registry.v_items AS SELECT 1 AS id";
  ASSERT_TRUE(store.UpsertView(view).ok());

  RoutineRecord routine;
  routine.id = RoutineId{"proj-1", "ds_registry", "add_one"};
  routine.kind = RoutineKind::kScalarFunction;
  routine.language = "SQL";
  routine.ddl_sql =
      "CREATE FUNCTION ds_registry.add_one(x INT64) RETURNS INT64 AS (x + 1)";
  ASSERT_TRUE(store.UpsertRoutine(routine).ok());

  ASSERT_TRUE(store.DropDataset(ds, /*delete_contents=*/true).ok());

  auto routines_after_drop = store.ListRoutines(ds);
  ASSERT_TRUE(routines_after_drop.ok());
  EXPECT_TRUE(routines_after_drop->empty());

  ASSERT_TRUE(store.RestoreDataset(ds).ok());

  auto routines_after_restore = store.ListRoutines(ds);
  ASSERT_TRUE(routines_after_restore.ok());
  ASSERT_EQ(routines_after_restore->size(), 1u);
  EXPECT_EQ((*routines_after_restore)[0].id.routine_id, "add_one");

  auto views_after_restore = store.ListAllViews();
  ASSERT_TRUE(views_after_restore.ok());
  bool found_view = false;
  for (const ViewRecord& rec : *views_after_restore) {
    if (rec.id.view_id == "v_items") {
      found_view = true;
      break;
    }
  }
  EXPECT_TRUE(found_view);
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

TEST_F(DuckDBStorageTest, TableTombstonePathIncludesDataset) {
  auto store_or = DuckDBStorage::Open(data_dir_.string());
  ASSERT_TRUE(store_or.ok()) << store_or.status();
  auto& store = **store_or;

  const TableId table{"proj-1", "ds_path", "tbl"};
  const std::string dir =
      internal::TableTombstoneDir(store, table, /*deleted_ms=*/123);
  EXPECT_TRUE(dir.find("proj-1.ds_path") != std::string::npos);
  EXPECT_TRUE(dir.find("/tbl/123") != std::string::npos);
}

TEST_F(DuckDBStorageTest, DropRecreateSameIdUndropIsAlreadyExists) {
  auto store_or = DuckDBStorage::Open(data_dir_.string());
  ASSERT_TRUE(store_or.ok()) << store_or.status();
  auto& store = **store_or;

  const DatasetId ds{"proj-1", "ds_recreate"};
  ASSERT_TRUE(store.CreateDataset(ds, "US").ok());
  ASSERT_TRUE(store.DropDataset(ds, /*delete_contents=*/true).ok());
  ASSERT_TRUE(store.CreateDataset(ds, "US").ok());

  absl::Status restored = store.RestoreDataset(ds);
  ASSERT_FALSE(restored.ok());
  EXPECT_EQ(restored.code(), absl::StatusCode::kAlreadyExists);
}

TEST_F(DuckDBStorageTest, DoubleUndropSecondIsNotFound) {
  auto store_or = DuckDBStorage::Open(data_dir_.string());
  ASSERT_TRUE(store_or.ok()) << store_or.status();
  auto& store = **store_or;

  const DatasetId ds{"proj-1", "ds_double"};
  ASSERT_TRUE(store.CreateDataset(ds, "US").ok());
  ASSERT_TRUE(store.DropDataset(ds, /*delete_contents=*/true).ok());
  ASSERT_TRUE(store.RestoreDataset(ds).ok());

  absl::Status second = store.RestoreDataset(ds);
  ASSERT_FALSE(second.ok());
  EXPECT_EQ(second.code(), absl::StatusCode::kAlreadyExists);
}

TEST_F(DuckDBStorageTest, RestartDurabilityRestoresDatasetAndMetadata) {
  const DatasetId ds{"proj-1", "ds_restart"};
  const TableId table{"proj-1", "ds_restart", "items"};
  const std::string rest_metadata = R"({"labels":{"env":"test"}})";

  {
    auto store_or = DuckDBStorage::Open(data_dir_.string());
    ASSERT_TRUE(store_or.ok()) << store_or.status();
    auto& store = **store_or;
    ASSERT_TRUE(store.CreateDataset(ds, "US").ok());
    ASSERT_TRUE(store.CreateTable(table, PeopleSchema()).ok());
    ASSERT_TRUE(
        store.DropDataset(ds, /*delete_contents=*/true, rest_metadata).ok());
    ASSERT_TRUE(store.RestoreDataset(ds).ok());
    auto meta_or = store.GetDatasetRestMetadataJson(ds);
    ASSERT_TRUE(meta_or.ok());
    EXPECT_NE(meta_or->find("env"), std::string::npos);
  }

  auto reopened_or = DuckDBStorage::Open(data_dir_.string());
  ASSERT_TRUE(reopened_or.ok()) << reopened_or.status();
  auto& reopened = **reopened_or;
  auto datasets_or = reopened.ListDatasets("proj-1");
  ASSERT_TRUE(datasets_or.ok());
  ASSERT_EQ(datasets_or->size(), 1u);
  auto meta_or = reopened.GetDatasetRestMetadataJson(ds);
  ASSERT_TRUE(meta_or.ok());
  EXPECT_NE(meta_or->find("env"), std::string::npos);
}

}  // namespace
}  // namespace duckdb
}  // namespace storage
}  // namespace backend
}  // namespace bigquery_emulator
