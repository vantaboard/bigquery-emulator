#include "backend/storage/duckdb/duckdb_storage_test_fixture.h"

#include <filesystem>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "absl/status/status.h"
#include "absl/strings/str_cat.h"
#include "absl/types/span.h"
#include "backend/schema/schema.h"
#include "backend/storage/duckdb/duckdb_storage.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace bigquery_emulator {
namespace backend {
namespace storage {
namespace duckdb {
namespace {

namespace fs = std::filesystem;

TEST_F(DuckDBStorageTest, RoundTripsHundredRowsAcrossRestart) {
  const DatasetId ds{"proj-1", "ds_1"};
  const TableId table{"proj-1", "ds_1", "people"};

  // ---------------- First process: write 100 rows. -----------------
  {
    auto store_or = DuckDBStorage::Open(data_dir_.string());
    ASSERT_TRUE(store_or.ok()) << store_or.status();
    auto& store = **store_or;

    ASSERT_TRUE(store.CreateDataset(ds, "US").ok());
    ASSERT_TRUE(store.CreateTable(table, PeopleSchema()).ok());

    std::vector<Row> rows;
    rows.reserve(100);
    for (int64_t i = 0; i < 100; ++i) {
      rows.push_back(MakePerson(i, absl::StrCat("person-", i)));
    }
    ASSERT_TRUE(store.AppendRows(table, absl::MakeConstSpan(rows)).ok());

    // Sanity check: in-process scan agrees with what we just wrote.
    auto iter_or = store.ScanRows(table);
    ASSERT_TRUE(iter_or.ok());
    int64_t count = 0;
    Row r;
    while (true) {
      auto has = (*iter_or)->Next(&r);
      ASSERT_TRUE(has.ok());
      if (!*has) break;
      ++count;
    }
    EXPECT_EQ(count, 100);
  }

  // The DuckDBStorage destructor closes the connection and the
  // catalog.duckdb file; reopening below mirrors a process restart.
  EXPECT_TRUE(fs::exists(data_dir_ / "catalog.duckdb"));
  EXPECT_TRUE(fs::exists(data_dir_ / "proj-1" / "ds_1" / "people.parquet"));
  EXPECT_TRUE(fs::exists(data_dir_ / "proj-1" / "ds_1" / "people.meta.json"));

  // ---------------- Second process: read 100 rows back. --------------
  {
    auto store_or = DuckDBStorage::Open(data_dir_.string());
    ASSERT_TRUE(store_or.ok()) << store_or.status();
    auto& store = **store_or;

    auto schema_or = store.GetSchema(table);
    ASSERT_TRUE(schema_or.ok());
    ASSERT_EQ(schema_or->columns.size(), 2u);
    EXPECT_EQ(schema_or->columns[0].name, "id");
    EXPECT_EQ(schema_or->columns[0].type, schema::ColumnType::kInt64);
    EXPECT_EQ(schema_or->columns[1].name, "name");
    EXPECT_EQ(schema_or->columns[1].type, schema::ColumnType::kString);

    auto iter_or = store.ScanRows(table);
    ASSERT_TRUE(iter_or.ok());
    std::vector<Row> scanned;
    Row r;
    while (true) {
      auto has = (*iter_or)->Next(&r);
      ASSERT_TRUE(has.ok());
      if (!*has) break;
      scanned.push_back(r);
    }
    ASSERT_EQ(scanned.size(), 100u);

    // The parquet file does not have a guaranteed order (we did not
    // ORDER BY), so build a set of seen ids and confirm we got
    // exactly 0..99 with the matching name string.
    std::vector<bool> seen(100, false);
    for (const auto& row : scanned) {
      ASSERT_EQ(row.cells.size(), 2u);
      const int64_t id = row.cells[0].int64_value();
      ASSERT_GE(id, 0);
      ASSERT_LT(id, 100);
      EXPECT_FALSE(seen[id]) << "duplicate row for id " << id;
      seen[id] = true;
      EXPECT_EQ(row.cells[1].string_value(), absl::StrCat("person-", id));
    }
    for (size_t i = 0; i < seen.size(); ++i) {
      EXPECT_TRUE(seen[i]) << "missing row for id " << i;
    }
  }
}

TEST_F(DuckDBStorageTest, CreateTableMaterializesEmptyParquet) {
  auto store_or = DuckDBStorage::Open(data_dir_.string());
  ASSERT_TRUE(store_or.ok()) << store_or.status();
  auto& store = **store_or;

  const DatasetId ds{"proj-1", "ds_1"};
  const TableId table{"proj-1", "ds_1", "people"};
  ASSERT_TRUE(store.CreateDataset(ds, "US").ok());
  ASSERT_TRUE(store.CreateTable(table, PeopleSchema()).ok());

  const fs::path parquet = data_dir_ / "proj-1" / "ds_1" / "people.parquet";
  ASSERT_TRUE(fs::exists(parquet));
  EXPECT_GT(fs::file_size(parquet), 0u);

  auto iter_or = store.ScanRows(table);
  ASSERT_TRUE(iter_or.ok());
  Row r;
  auto has = (*iter_or)->Next(&r);
  ASSERT_TRUE(has.ok());
  EXPECT_FALSE(*has);
}

// Regression: thirdparty:node-bigquery-tests view/query/delete-table
// `before all` hooks failed with `CREATE OR REPLACE TEMP TABLE
// main.__bqemu_mkempty (): Parser Error: Table must have at least
// one column!` whenever the gateway registered a view or other
// schema-less table through CreateTable. The fix short-circuits the
// DuckDB scratch path for empty schemas; this test pins it.
TEST_F(DuckDBStorageTest, CreateTableWithEmptySchemaSkipsParquet) {
  auto store_or = DuckDBStorage::Open(data_dir_.string());
  ASSERT_TRUE(store_or.ok()) << store_or.status();
  auto& store = **store_or;

  const DatasetId ds{"proj-1", "ds_1"};
  const TableId table{"proj-1", "ds_1", "the_view"};
  ASSERT_TRUE(store.CreateDataset(ds, "US").ok());

  schema::TableSchema empty;
  ASSERT_TRUE(empty.columns.empty());
  ASSERT_TRUE(store.CreateTable(table, empty).ok());

  const fs::path sidecar = data_dir_ / "proj-1" / "ds_1" / "the_view.meta.json";
  const fs::path parquet = data_dir_ / "proj-1" / "ds_1" / "the_view.parquet";
  EXPECT_TRUE(fs::exists(sidecar));
  EXPECT_FALSE(fs::exists(parquet));

  auto schema_or = store.GetSchema(table);
  ASSERT_TRUE(schema_or.ok()) << schema_or.status();
  EXPECT_TRUE(schema_or->columns.empty());
}

// Regression: CREATE TABLE with a STRUCT column must not crash the
// engine while materializing the empty parquet scratch table.
TEST_F(DuckDBStorageTest, CreateTableWithStructColumnMaterializesParquet) {
  auto store_or = DuckDBStorage::Open(data_dir_.string());
  ASSERT_TRUE(store_or.ok()) << store_or.status();
  auto& store = **store_or;

  const DatasetId ds{"proj-1", "ds_1"};
  const TableId table{"proj-1", "ds_1", "infostruct"};
  ASSERT_TRUE(store.CreateDataset(ds, "US").ok());

  schema::TableSchema schema;
  schema::ColumnSchema k;
  k.name = "k";
  k.type = schema::ColumnType::kInt64;
  schema::ColumnSchema s;
  s.name = "s";
  s.type = schema::ColumnType::kStruct;
  schema::ColumnSchema a;
  a.name = "a";
  a.type = schema::ColumnType::kInt64;
  schema::ColumnSchema b;
  b.name = "b";
  b.type = schema::ColumnType::kString;
  s.fields = {a, b};
  schema.columns = {k, s};

  ASSERT_TRUE(store.CreateTable(table, schema).ok());

  const fs::path parquet = data_dir_ / "proj-1" / "ds_1" / "infostruct.parquet";
  ASSERT_TRUE(fs::exists(parquet));
  EXPECT_GT(fs::file_size(parquet), 0u);

  auto schema_or = store.GetSchema(table);
  ASSERT_TRUE(schema_or.ok()) << schema_or.status();
  ASSERT_EQ(schema_or->columns.size(), 2u);
  EXPECT_EQ(schema_or->columns[1].type, schema::ColumnType::kStruct);
  ASSERT_EQ(schema_or->columns[1].fields.size(), 2u);

  auto iter_or = store.ScanRows(table);
  ASSERT_TRUE(iter_or.ok()) << iter_or.status();
  Row r;
  auto has = (*iter_or)->Next(&r);
  ASSERT_TRUE(has.ok());
  EXPECT_FALSE(*has);
}

TEST_F(DuckDBStorageTest, ListDatasetsReturnsSortedIdsForProject) {
  auto store_or = DuckDBStorage::Open(data_dir_.string());
  ASSERT_TRUE(store_or.ok()) << store_or.status();
  auto& store = **store_or;

  ASSERT_TRUE(store.CreateDataset({"proj-a", "ds_charlie"}, "US").ok());
  ASSERT_TRUE(store.CreateDataset({"proj-a", "ds_alpha"}, "US").ok());
  ASSERT_TRUE(store.CreateDataset({"proj-a", "ds_bravo"}, "US").ok());
  ASSERT_TRUE(store.CreateDataset({"proj-other", "ds_zulu"}, "US").ok());

  auto list_a_or = store.ListDatasets("proj-a");
  ASSERT_TRUE(list_a_or.ok()) << list_a_or.status();
  ASSERT_EQ(list_a_or->size(), 3u);
  EXPECT_EQ((*list_a_or)[0].dataset_id, "ds_alpha");
  EXPECT_EQ((*list_a_or)[1].dataset_id, "ds_bravo");
  EXPECT_EQ((*list_a_or)[2].dataset_id, "ds_charlie");
  for (const auto& id : *list_a_or) {
    EXPECT_EQ(id.project_id, "proj-a");
  }

  auto list_other_or = store.ListDatasets("proj-other");
  ASSERT_TRUE(list_other_or.ok());
  ASSERT_EQ(list_other_or->size(), 1u);
  EXPECT_EQ((*list_other_or)[0].dataset_id, "ds_zulu");

  // Unknown project: empty vector, not NOT_FOUND. The gateway treats
  // "no datasets in this project" and "this project has nothing yet"
  // the same way (live BigQuery returns 200 with an empty `datasets`
  // array for both).
  auto list_unknown_or = store.ListDatasets("proj-missing");
  ASSERT_TRUE(list_unknown_or.ok()) << list_unknown_or.status();
  EXPECT_TRUE(list_unknown_or->empty());
}

TEST_F(DuckDBStorageTest, ListDatasetsRejectsEmptyProjectID) {
  auto store_or = DuckDBStorage::Open(data_dir_.string());
  ASSERT_TRUE(store_or.ok());
  auto& store = **store_or;

  auto list_or = store.ListDatasets("");
  ASSERT_FALSE(list_or.ok());
  EXPECT_EQ(list_or.status().code(), absl::StatusCode::kInvalidArgument);
}

TEST_F(DuckDBStorageTest, ListTablesReturnsSortedIdsForDataset) {
  auto store_or = DuckDBStorage::Open(data_dir_.string());
  ASSERT_TRUE(store_or.ok()) << store_or.status();
  auto& store = **store_or;

  const DatasetId ds{"proj-1", "ds_1"};
  ASSERT_TRUE(store.CreateDataset(ds, "US").ok());
  ASSERT_TRUE(
      store.CreateTable({"proj-1", "ds_1", "charlie"}, PeopleSchema()).ok());
  ASSERT_TRUE(
      store.CreateTable({"proj-1", "ds_1", "alpha"}, PeopleSchema()).ok());
  // Empty-schema entries (views; see the empty-schema regression test)
  // must still surface in ListTables. The sidecar is the canonical
  // existence marker.
  schema::TableSchema empty;
  ASSERT_TRUE(store.CreateTable({"proj-1", "ds_1", "bravo_view"}, empty).ok());

  auto list_or = store.ListTables(ds);
  ASSERT_TRUE(list_or.ok()) << list_or.status();
  ASSERT_EQ(list_or->size(), 3u);
  EXPECT_EQ((*list_or)[0].table_id, "alpha");
  EXPECT_EQ((*list_or)[1].table_id, "bravo_view");
  EXPECT_EQ((*list_or)[2].table_id, "charlie");
  for (const auto& id : *list_or) {
    EXPECT_EQ(id.project_id, "proj-1");
    EXPECT_EQ(id.dataset_id, "ds_1");
  }
}

TEST_F(DuckDBStorageTest, ListTablesOnMissingDatasetIsNotFound) {
  auto store_or = DuckDBStorage::Open(data_dir_.string());
  ASSERT_TRUE(store_or.ok());
  auto& store = **store_or;

  auto list_or = store.ListTables({"proj-x", "ds_missing"});
  ASSERT_FALSE(list_or.ok());
  EXPECT_EQ(list_or.status().code(), absl::StatusCode::kNotFound);
}

TEST_F(DuckDBStorageTest, ListTablesOnEmptyDatasetIsEmpty) {
  auto store_or = DuckDBStorage::Open(data_dir_.string());
  ASSERT_TRUE(store_or.ok());
  auto& store = **store_or;

  const DatasetId ds{"proj-1", "ds_empty"};
  ASSERT_TRUE(store.CreateDataset(ds, "US").ok());

  auto list_or = store.ListTables(ds);
  ASSERT_TRUE(list_or.ok()) << list_or.status();
  EXPECT_TRUE(list_or->empty());
}

TEST_F(DuckDBStorageTest, DropTableRemovesParquetAndSidecar) {
  auto store_or = DuckDBStorage::Open(data_dir_.string());
  ASSERT_TRUE(store_or.ok()) << store_or.status();
  auto& store = **store_or;

  const DatasetId ds{"proj-1", "ds_1"};
  const TableId table{"proj-1", "ds_1", "people"};
  ASSERT_TRUE(store.CreateDataset(ds, "US").ok());
  ASSERT_TRUE(store.CreateTable(table, PeopleSchema()).ok());

  const fs::path parquet = data_dir_ / "proj-1" / "ds_1" / "people.parquet";
  const fs::path sidecar = data_dir_ / "proj-1" / "ds_1" / "people.meta.json";
  ASSERT_TRUE(fs::exists(parquet));
  ASSERT_TRUE(fs::exists(sidecar));

  ASSERT_TRUE(store.DropTable(table).ok());
  EXPECT_FALSE(fs::exists(parquet));
  EXPECT_FALSE(fs::exists(sidecar));
}

TEST_F(DuckDBStorageTest, AppendRowsRejectsMisshapenBatch) {
  auto store_or = DuckDBStorage::Open(data_dir_.string());
  ASSERT_TRUE(store_or.ok()) << store_or.status();
  auto& store = **store_or;

  const DatasetId ds{"proj-1", "ds_1"};
  const TableId table{"proj-1", "ds_1", "people"};
  ASSERT_TRUE(store.CreateDataset(ds, "US").ok());
  ASSERT_TRUE(store.CreateTable(table, PeopleSchema()).ok());

  std::vector<Row> rows;
  rows.push_back(MakePerson(1, "ada"));
  Row malformed;
  malformed.cells = {Value::Int64(2)};
  rows.push_back(std::move(malformed));

  auto status = store.AppendRows(table, absl::MakeConstSpan(rows));
  EXPECT_EQ(status.code(), absl::StatusCode::kInvalidArgument);

  // Misshapen batch must not have leaked the good row at index 0.
  auto iter_or = store.ScanRows(table);
  ASSERT_TRUE(iter_or.ok());
  Row r;
  auto has = (*iter_or)->Next(&r);
  ASSERT_TRUE(has.ok());
  EXPECT_FALSE(*has);
}

TEST_F(DuckDBStorageTest, AppendRowsAppendsAcrossMultipleBatches) {
  auto store_or = DuckDBStorage::Open(data_dir_.string());
  ASSERT_TRUE(store_or.ok()) << store_or.status();
  auto& store = **store_or;

  const DatasetId ds{"proj-1", "ds_1"};
  const TableId table{"proj-1", "ds_1", "people"};
  ASSERT_TRUE(store.CreateDataset(ds, "US").ok());
  ASSERT_TRUE(store.CreateTable(table, PeopleSchema()).ok());

  std::vector<Row> first;
  first.push_back(MakePerson(1, "ada"));
  first.push_back(MakePerson(2, "linus"));
  ASSERT_TRUE(store.AppendRows(table, absl::MakeConstSpan(first)).ok());

  std::vector<Row> second;
  second.push_back(MakePerson(3, "grace"));
  ASSERT_TRUE(store.AppendRows(table, absl::MakeConstSpan(second)).ok());

  auto iter_or = store.ScanRows(table);
  ASSERT_TRUE(iter_or.ok());
  std::vector<Row> scanned;
  Row r;
  while (true) {
    auto has = (*iter_or)->Next(&r);
    ASSERT_TRUE(has.ok());
    if (!*has) break;
    scanned.push_back(r);
  }
  EXPECT_EQ(scanned.size(), 3u);
}

}  // namespace
}  // namespace duckdb
}  // namespace storage
}  // namespace backend
}  // namespace bigquery_emulator
