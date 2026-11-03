#include <string>
#include <vector>

#include "absl/time/civil_time.h"
#include "absl/time/clock.h"
#include "absl/time/time.h"
#include "absl/types/span.h"
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
}

std::vector<Row> ScanAll(DuckDBStorage& store, const TableId& table) {
  std::vector<Row> rows;
  auto iter_or = store.ScanRows(table);
  EXPECT_TRUE(iter_or.ok()) << iter_or.status();
  if (!iter_or.ok()) return rows;
  Row row;
  while (true) {
    auto has = (*iter_or)->Next(&row);
    EXPECT_TRUE(has.ok()) << has.status();
    if (!has.ok() || !*has) break;
    rows.push_back(row);
  }
  return rows;
}

TEST_F(DuckDBStorageTest, PartitionColumnsAreStoredLastAndHidden) {
  const TableId table{"proj-1", "ds_1", "events"};
  schema::TableSchema schema = PeopleSchema();
  schema.ingestion_partition_type = "DAY";
  auto store_or = DuckDBStorage::Open(data_dir_.string());
  ASSERT_TRUE(store_or.ok()) << store_or.status();
  ASSERT_TRUE((*store_or)->CreateDataset({"proj-1", "ds_1"}, "US").ok());
  ASSERT_TRUE((*store_or)->CreateTable(table, schema).ok());

  auto stored = (*store_or)->GetSchema(table);
  ASSERT_TRUE(stored.ok()) << stored.status();
  ASSERT_EQ(stored->columns.size(), schema.columns.size() + 2);
  const schema::ColumnSchema& time = stored->columns[schema.columns.size()];
  EXPECT_EQ(time.name, schema::kPartitionTimeColumn);
  EXPECT_EQ(time.type, schema::ColumnType::kTimestamp);
  EXPECT_EQ(stored->columns.back().name, schema::kPartitionDateColumn);
  EXPECT_EQ(stored->columns.back().type, schema::ColumnType::kDate);
  EXPECT_EQ(schema::VisibleSchema(*stored).columns.size(),
            schema.columns.size());
}

TEST_F(DuckDBStorageTest, AppendRowsStampsThePartitionDate) {
  const TableId table{"proj-1", "ds_1", "events"};
  schema::TableSchema schema = PeopleSchema();
  schema.ingestion_partition_type = "DAY";
  auto store_or = DuckDBStorage::Open(data_dir_.string());
  ASSERT_TRUE(store_or.ok()) << store_or.status();
  auto& store = **store_or;
  ASSERT_TRUE(store.CreateDataset({"proj-1", "ds_1"}, "US").ok());
  ASSERT_TRUE(store.CreateTable(table, schema).ok());

  Row given = MakePerson(2, "given");
  given.cells.push_back(Value::String("2024-03-04 00:00:00+00"));
  given.cells.push_back(Value::Null());
  const std::vector<Row> rows = {MakePerson(1, "missing"), given};
  ASSERT_TRUE(store.AppendRows(table, absl::MakeConstSpan(rows)).ok());

  const absl::TimeZone utc = absl::UTCTimeZone();
  const absl::Time today = absl::FromCivil(
      absl::CivilDay(absl::ToCivilSecond(absl::Now(), utc)), utc);
  const std::vector<Row> read = ScanAll(store, table);
  ASSERT_EQ(read.size(), 2u);
  for (const Row& row : read)
    ASSERT_EQ(row.cells.size(), 4u);
  EXPECT_EQ(read[0].cells[2].string_value(),
            absl::FormatTime("%Y-%m-%d %H:%M:%S+00", today, utc));
  EXPECT_EQ(read[0].cells[3].string_value(),
            absl::FormatTime("%Y-%m-%d", today, utc));
  EXPECT_EQ(read[1].cells[3].string_value(), "2024-03-04");
}

TEST_F(DuckDBStorageTest, AppendRowsStampsTheTruncatedIngestionTime) {
  const TableId table{"proj-1", "ds_1", "events"};
  schema::TableSchema schema = PeopleSchema();
  schema.ingestion_partition_type = "MONTH";
  auto store_or = DuckDBStorage::Open(data_dir_.string());
  ASSERT_TRUE(store_or.ok()) << store_or.status();
  auto& store = **store_or;
  ASSERT_TRUE(store.CreateDataset({"proj-1", "ds_1"}, "US").ok());
  ASSERT_TRUE(store.CreateTable(table, schema).ok());

  Row given = MakePerson(2, "given");
  given.cells.push_back(Value::String("2024-03-04 00:00:00+00"));
  Row null_cell = MakePerson(3, "null");
  null_cell.cells.push_back(Value::Null());
  const std::vector<Row> rows = {MakePerson(1, "missing"), given, null_cell};
  ASSERT_TRUE(store.AppendRows(table, absl::MakeConstSpan(rows)).ok());

  const absl::TimeZone utc = absl::UTCTimeZone();
  const std::string month_start = absl::FormatTime(
      "%Y-%m-%d %H:%M:%S+00",
      absl::FromCivil(absl::CivilMonth(absl::ToCivilSecond(absl::Now(), utc)),
                      utc),
      utc);
  const std::vector<Row> read = ScanAll(store, table);
  ASSERT_EQ(read.size(), 3u);
  for (const Row& row : read)
    ASSERT_EQ(row.cells.size(), 3u);
  EXPECT_EQ(read[0].cells[2].string_value(), month_start);
  EXPECT_EQ(read[1].cells[2].string_value(), "2024-03-04 00:00:00+00");
  EXPECT_EQ(read[2].cells[2].string_value(), month_start);
}

}  // namespace
}  // namespace duckdb
}  // namespace storage
}  // namespace backend
}  // namespace bigquery_emulator
