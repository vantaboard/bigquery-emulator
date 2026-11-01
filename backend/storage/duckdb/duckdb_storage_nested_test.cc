#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "absl/types/span.h"
#include "backend/schema/schema.h"
#include "backend/storage/duckdb/duckdb_storage.h"
#include "backend/storage/duckdb/duckdb_storage_internal.h"
#include "backend/storage/duckdb/duckdb_storage_test_fixture.h"
#include "backend/storage/storage.h"
#include "gtest/gtest.h"

namespace bigquery_emulator {
namespace backend {
namespace storage {
namespace duckdb {
namespace {

schema::ColumnSchema Column(
    std::string name,
    schema::ColumnType type,
    schema::ColumnMode mode = schema::ColumnMode::kNullable,
    std::vector<schema::ColumnSchema> fields = {}) {
  schema::ColumnSchema column;
  column.name = std::move(name);
  column.type = type;
  column.mode = mode;
  column.fields = std::move(fields);
  return column;
}

schema::ColumnSchema EventColumn() {
  return Column("events",
                schema::ColumnType::kStruct,
                schema::ColumnMode::kRepeated,
                {Column("kind", schema::ColumnType::kString),
                 Column("tags",
                        schema::ColumnType::kString,
                        schema::ColumnMode::kRepeated)});
}

TEST(ParseDuckDBStructVarchar, ParsesARepeatedField) {
  const schema::ColumnSchema person =
      Column("person",
             schema::ColumnType::kStruct,
             schema::ColumnMode::kNullable,
             {Column("id", schema::ColumnType::kInt64),
              Column("labels",
                     schema::ColumnType::kString,
                     schema::ColumnMode::kRepeated)});

  auto parsed =
      internal::ParseDuckDBStructVarchar("{'id': 1, 'labels': [a, b]}", person);

  ASSERT_TRUE(parsed.ok()) << parsed.status();
  ASSERT_EQ(parsed->kind(), Value::Kind::kStruct);
  const auto& fields = parsed->struct_value();
  ASSERT_EQ(fields.size(), 2u);
  EXPECT_EQ(fields[0].int64_value(), 1);
  ASSERT_EQ(fields[1].kind(), Value::Kind::kArray);
  ASSERT_EQ(fields[1].array_value().size(), 2u);
  EXPECT_EQ(fields[1].array_value()[0].string_value(), "a");
  EXPECT_EQ(fields[1].array_value()[1].string_value(), "b");
}

TEST(ParseDuckDBListVarchar, ParsesStructElements) {
  auto parsed = internal::ParseDuckDBListVarchar(
      "[{'kind': view, 'tags': []}, {'kind': 'a, b', 'tags': NULL}]",
      internal::ListElementSchema(EventColumn()));

  ASSERT_TRUE(parsed.ok()) << parsed.status();
  ASSERT_EQ(parsed->kind(), Value::Kind::kArray);
  const auto& elements = parsed->array_value();
  ASSERT_EQ(elements.size(), 2u);
  ASSERT_EQ(elements[0].kind(), Value::Kind::kStruct);
  EXPECT_EQ(elements[0].struct_value()[0].string_value(), "view");
  EXPECT_EQ(elements[0].struct_value()[1].kind(), Value::Kind::kArray);
  EXPECT_TRUE(elements[0].struct_value()[1].array_value().empty());
  EXPECT_EQ(elements[1].struct_value()[0].string_value(), "a, b");
  EXPECT_TRUE(elements[1].struct_value()[1].is_null());
}

TEST(ParseDuckDBStructVarchar, SkipsFieldsTheSchemaDoesNotHave) {
  const schema::ColumnSchema user =
      Column("user",
             schema::ColumnType::kStruct,
             schema::ColumnMode::kNullable,
             {Column("name", schema::ColumnType::kString)});

  auto parsed = internal::ParseDuckDBStructVarchar(
      "{'extra': {'a': [1, 2]}, 'name': ada}", user);

  ASSERT_TRUE(parsed.ok()) << parsed.status();
  ASSERT_EQ(parsed->struct_value().size(), 1u);
  EXPECT_EQ(parsed->struct_value()[0].string_value(), "ada");
}

TEST_F(DuckDBStorageTest, ScanRowsRoundTripsRepeatedStructsWithRepeatedFields) {
  auto store_or = DuckDBStorage::Open(data_dir_.string());
  ASSERT_TRUE(store_or.ok()) << store_or.status();
  auto& store = **store_or;
  const TableId table{"proj-1", "ds_1", "events"};
  ASSERT_TRUE(store.CreateDataset({"proj-1", "ds_1"}, "US").ok());
  schema::TableSchema schema;
  schema.columns.push_back(EventColumn());
  ASSERT_TRUE(store.CreateTable(table, schema).ok());

  const std::vector<std::string> tag_strings = {
      "a, 'b'", "c", "x]y", "NULL", "", "back\\slash \\'q'"};
  std::vector<Value> tags;
  for (const std::string& tag : tag_strings)
    tags.push_back(Value::String(tag));
  std::vector<Value> event;
  event.push_back(Value::String("view"));
  event.push_back(Value::Array(std::move(tags)));
  std::vector<Value> events;
  events.push_back(Value::Struct(std::move(event)));
  Row row;
  row.cells.push_back(Value::Array(std::move(events)));
  ASSERT_TRUE(store.AppendRows(table, absl::MakeConstSpan({row})).ok());

  auto iter_or = store.ScanRows(table);
  ASSERT_TRUE(iter_or.ok()) << iter_or.status();
  Row scanned;
  auto has = (*iter_or)->Next(&scanned);
  ASSERT_TRUE(has.ok() && *has);
  ASSERT_EQ(scanned.cells[0].kind(), Value::Kind::kArray);
  ASSERT_EQ(scanned.cells[0].array_value().size(), 1u);
  const Value& read = scanned.cells[0].array_value()[0];
  ASSERT_EQ(read.kind(), Value::Kind::kStruct);
  EXPECT_EQ(read.struct_value()[0].string_value(), "view");
  ASSERT_EQ(read.struct_value()[1].kind(), Value::Kind::kArray);
  const std::vector<Value>& read_tags = read.struct_value()[1].array_value();
  ASSERT_EQ(read_tags.size(), tag_strings.size());
  for (size_t i = 0; i < tag_strings.size(); ++i) {
    ASSERT_EQ(read_tags[i].kind(), Value::Kind::kString) << i;
    EXPECT_EQ(read_tags[i].string_value(), tag_strings[i]);
  }
}

}  // namespace
}  // namespace duckdb
}  // namespace storage
}  // namespace backend
}  // namespace bigquery_emulator
