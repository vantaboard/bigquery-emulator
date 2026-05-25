#include "backend/storage/memory/in_memory_storage.h"

#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "backend/schema/schema.h"
#include "backend/storage/storage.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace bigquery_emulator {
namespace backend {
namespace storage {
namespace memory {
namespace {

using ::testing::HasSubstr;

// Three-column toy schema used by the happy-path tests:
//   id   INT64    REQUIRED
//   name STRING   NULLABLE
//   tags ARRAY<STRING>
schema::TableSchema PeopleSchema() {
  schema::TableSchema s;
  schema::ColumnSchema id;
  id.name = "id";
  id.type = schema::ColumnType::kInt64;
  id.mode = schema::ColumnMode::kRequired;
  schema::ColumnSchema name;
  name.name = "name";
  name.type = schema::ColumnType::kString;
  name.mode = schema::ColumnMode::kNullable;
  schema::ColumnSchema tags;
  tags.name = "tags";
  tags.type = schema::ColumnType::kString;
  tags.mode = schema::ColumnMode::kRepeated;
  s.columns = {id, name, tags};
  return s;
}

Row MakePerson(int64_t id, const std::string& name,
               std::vector<std::string> tags) {
  Row r;
  std::vector<Value> tag_values;
  tag_values.reserve(tags.size());
  for (auto& t : tags) {
    tag_values.push_back(Value::String(std::move(t)));
  }
  r.cells = {
      Value::Int64(id),
      Value::String(name),
      Value::Array(std::move(tag_values)),
  };
  return r;
}

TEST(InMemoryStorageTest, CreateAppendScanThreeRows) {
  InMemoryStorage store;
  const DatasetId ds{"proj-1", "ds_1"};
  const TableId table{"proj-1", "ds_1", "people"};

  ASSERT_TRUE(store.CreateDataset(ds, "US").ok());
  ASSERT_TRUE(store.CreateTable(table, PeopleSchema()).ok());

  std::vector<Row> rows = {
      MakePerson(1, "ada", {"math", "lace"}),
      MakePerson(2, "linus", {"kernel"}),
      MakePerson(3, "grace", {}),
  };
  ASSERT_TRUE(store.AppendRows(table, absl::MakeConstSpan(rows)).ok());

  auto schema_or = store.GetSchema(table);
  ASSERT_TRUE(schema_or.ok());
  ASSERT_EQ(schema_or->columns.size(), 3u);
  EXPECT_EQ(schema_or->columns[0].name, "id");
  EXPECT_EQ(schema_or->columns[0].type, schema::ColumnType::kInt64);
  EXPECT_EQ(schema_or->columns[0].mode, schema::ColumnMode::kRequired);
  EXPECT_EQ(schema_or->columns[1].name, "name");
  EXPECT_EQ(schema_or->columns[1].type, schema::ColumnType::kString);
  EXPECT_EQ(schema_or->columns[2].name, "tags");
  EXPECT_EQ(schema_or->columns[2].type, schema::ColumnType::kString);
  EXPECT_EQ(schema_or->columns[2].mode, schema::ColumnMode::kRepeated);

  auto iter_or = store.ScanRows(table);
  ASSERT_TRUE(iter_or.ok());
  std::unique_ptr<RowIterator> iter = std::move(*iter_or);

  std::vector<Row> scanned;
  Row r;
  while (true) {
    auto has = iter->Next(&r);
    ASSERT_TRUE(has.ok());
    if (!*has) break;
    scanned.push_back(r);
  }
  ASSERT_EQ(scanned.size(), 3u);
  EXPECT_EQ(scanned[0].cells[0].int64_value(), 1);
  EXPECT_EQ(scanned[0].cells[1].string_value(), "ada");
  ASSERT_EQ(scanned[0].cells[2].array_value().size(), 2u);
  EXPECT_EQ(scanned[0].cells[2].array_value()[0].string_value(), "math");
  EXPECT_EQ(scanned[1].cells[0].int64_value(), 2);
  EXPECT_EQ(scanned[1].cells[1].string_value(), "linus");
  EXPECT_EQ(scanned[2].cells[0].int64_value(), 3);
  EXPECT_EQ(scanned[2].cells[1].string_value(), "grace");
  EXPECT_TRUE(scanned[2].cells[2].array_value().empty());
}

TEST(InMemoryStorageTest, CreateDatasetRejectsDuplicate) {
  InMemoryStorage store;
  const DatasetId ds{"proj-1", "ds_1"};
  ASSERT_TRUE(store.CreateDataset(ds, "US").ok());
  auto status = store.CreateDataset(ds, "US");
  EXPECT_EQ(status.code(), absl::StatusCode::kAlreadyExists);
}

TEST(InMemoryStorageTest, CreateTableRequiresDataset) {
  InMemoryStorage store;
  const TableId t{"proj-1", "missing_ds", "people"};
  auto status = store.CreateTable(t, PeopleSchema());
  EXPECT_EQ(status.code(), absl::StatusCode::kNotFound);
}

TEST(InMemoryStorageTest, DropTableMissingIsNotFound) {
  InMemoryStorage store;
  const DatasetId ds{"proj-1", "ds_1"};
  ASSERT_TRUE(store.CreateDataset(ds, "US").ok());
  auto status = store.DropTable({"proj-1", "ds_1", "ghost"});
  EXPECT_EQ(status.code(), absl::StatusCode::kNotFound);
}

TEST(InMemoryStorageTest, DropDatasetRefusesNonEmptyWithoutDeleteContents) {
  InMemoryStorage store;
  const DatasetId ds{"proj-1", "ds_1"};
  const TableId t{"proj-1", "ds_1", "people"};
  ASSERT_TRUE(store.CreateDataset(ds, "US").ok());
  ASSERT_TRUE(store.CreateTable(t, PeopleSchema()).ok());

  auto status = store.DropDataset(ds, /*delete_contents=*/false);
  EXPECT_EQ(status.code(), absl::StatusCode::kFailedPrecondition);
  EXPECT_THAT(std::string(status.message()), HasSubstr("not empty"));

  // With delete_contents=true the tables go away with the dataset.
  ASSERT_TRUE(store.DropDataset(ds, /*delete_contents=*/true).ok());
  auto schema_or = store.GetSchema(t);
  EXPECT_EQ(schema_or.status().code(), absl::StatusCode::kNotFound);
}

TEST(InMemoryStorageTest, AppendRowsRejectsMisshapenBatch) {
  InMemoryStorage store;
  const DatasetId ds{"proj-1", "ds_1"};
  const TableId t{"proj-1", "ds_1", "people"};
  ASSERT_TRUE(store.CreateDataset(ds, "US").ok());
  ASSERT_TRUE(store.CreateTable(t, PeopleSchema()).ok());

  std::vector<Row> rows = {
      MakePerson(1, "ada", {}),
      Row{{Value::Int64(2), Value::String("linus")}},  // missing `tags`
  };
  auto status = store.AppendRows(t, absl::MakeConstSpan(rows));
  EXPECT_EQ(status.code(), absl::StatusCode::kInvalidArgument);
  EXPECT_THAT(std::string(status.message()), HasSubstr("row[1]"));

  // The good row at index 0 must not have leaked into the table: the
  // entire batch is rejected.
  auto iter_or = store.ScanRows(t);
  ASSERT_TRUE(iter_or.ok());
  Row r;
  auto has = (*iter_or)->Next(&r);
  ASSERT_TRUE(has.ok());
  EXPECT_FALSE(*has);
}

TEST(InMemoryStorageTest, ScanRowsSnapshotsAtCallTime) {
  InMemoryStorage store;
  const DatasetId ds{"proj-1", "ds_1"};
  const TableId t{"proj-1", "ds_1", "people"};
  ASSERT_TRUE(store.CreateDataset(ds, "US").ok());
  ASSERT_TRUE(store.CreateTable(t, PeopleSchema()).ok());

  std::vector<Row> first = {MakePerson(1, "ada", {})};
  ASSERT_TRUE(store.AppendRows(t, absl::MakeConstSpan(first)).ok());

  auto iter_or = store.ScanRows(t);
  ASSERT_TRUE(iter_or.ok());

  // Append more rows after the iterator is created. The snapshot the
  // iterator owns must not grow.
  std::vector<Row> second = {MakePerson(2, "linus", {})};
  ASSERT_TRUE(store.AppendRows(t, absl::MakeConstSpan(second)).ok());

  Row r;
  auto has1 = (*iter_or)->Next(&r);
  ASSERT_TRUE(has1.ok());
  ASSERT_TRUE(*has1);
  EXPECT_EQ(r.cells[0].int64_value(), 1);

  auto has2 = (*iter_or)->Next(&r);
  ASSERT_TRUE(has2.ok());
  EXPECT_FALSE(*has2);
}

TEST(InMemoryStorageTest, ScanMissingTableIsNotFound) {
  InMemoryStorage store;
  auto iter_or = store.ScanRows({"proj-1", "missing", "ghost"});
  ASSERT_FALSE(iter_or.ok());
  EXPECT_EQ(iter_or.status().code(), absl::StatusCode::kNotFound);
}

TEST(InMemoryStorageTest, OverwriteRowsReplacesTableContents) {
  InMemoryStorage store;
  const DatasetId ds{"proj-1", "ds_1"};
  const TableId t{"proj-1", "ds_1", "people"};
  ASSERT_TRUE(store.CreateDataset(ds, "US").ok());
  ASSERT_TRUE(store.CreateTable(t, PeopleSchema()).ok());

  std::vector<Row> initial = {
      MakePerson(1, "ada", {}),
      MakePerson(2, "linus", {}),
  };
  ASSERT_TRUE(store.AppendRows(t, absl::MakeConstSpan(initial)).ok());

  // OverwriteRows: replace the two-row snapshot with a one-row one.
  std::vector<Row> replacement = {MakePerson(7, "grace", {"compiler"})};
  ASSERT_TRUE(store.OverwriteRows(t, absl::MakeConstSpan(replacement)).ok());

  auto iter_or = store.ScanRows(t);
  ASSERT_TRUE(iter_or.ok());
  std::vector<Row> scanned;
  Row r;
  while (true) {
    auto has = (*iter_or)->Next(&r);
    ASSERT_TRUE(has.ok());
    if (!*has) break;
    scanned.push_back(r);
  }
  ASSERT_EQ(scanned.size(), 1u);
  EXPECT_EQ(scanned[0].cells[0].int64_value(), 7);
  EXPECT_EQ(scanned[0].cells[1].string_value(), "grace");
}

TEST(InMemoryStorageTest, OverwriteRowsEmptyTruncatesTable) {
  InMemoryStorage store;
  const DatasetId ds{"proj-1", "ds_1"};
  const TableId t{"proj-1", "ds_1", "people"};
  ASSERT_TRUE(store.CreateDataset(ds, "US").ok());
  ASSERT_TRUE(store.CreateTable(t, PeopleSchema()).ok());

  std::vector<Row> initial = {MakePerson(1, "ada", {})};
  ASSERT_TRUE(store.AppendRows(t, absl::MakeConstSpan(initial)).ok());

  ASSERT_TRUE(
      store.OverwriteRows(t, absl::Span<const Row>()).ok());

  auto iter_or = store.ScanRows(t);
  ASSERT_TRUE(iter_or.ok());
  Row r;
  auto has = (*iter_or)->Next(&r);
  ASSERT_TRUE(has.ok());
  EXPECT_FALSE(*has);
}

TEST(InMemoryStorageTest, OverwriteRowsRejectsMisshapenBatch) {
  InMemoryStorage store;
  const DatasetId ds{"proj-1", "ds_1"};
  const TableId t{"proj-1", "ds_1", "people"};
  ASSERT_TRUE(store.CreateDataset(ds, "US").ok());
  ASSERT_TRUE(store.CreateTable(t, PeopleSchema()).ok());

  std::vector<Row> initial = {MakePerson(1, "ada", {})};
  ASSERT_TRUE(store.AppendRows(t, absl::MakeConstSpan(initial)).ok());

  std::vector<Row> bad = {Row{{Value::Int64(2)}}};
  auto status = store.OverwriteRows(t, absl::MakeConstSpan(bad));
  EXPECT_EQ(status.code(), absl::StatusCode::kInvalidArgument);

  // The original row is still there: the misshapen batch was rejected
  // before any rewrite happened.
  auto iter_or = store.ScanRows(t);
  ASSERT_TRUE(iter_or.ok());
  Row r;
  auto has = (*iter_or)->Next(&r);
  ASSERT_TRUE(has.ok());
  ASSERT_TRUE(*has);
  EXPECT_EQ(r.cells[0].int64_value(), 1);
}

TEST(InMemoryStorageTest, OverwriteRowsOnMissingTableIsNotFound) {
  InMemoryStorage store;
  const TableId t{"proj-1", "ds_1", "ghost"};
  auto status = store.OverwriteRows(t, absl::Span<const Row>());
  EXPECT_EQ(status.code(), absl::StatusCode::kNotFound);
}

}  // namespace
}  // namespace memory
}  // namespace storage
}  // namespace backend
}  // namespace bigquery_emulator
