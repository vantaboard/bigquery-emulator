#include <cstddef>
#include <vector>

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

TEST_F(DuckDBStorageTest, AppendRowsStoresANullRepeatedFieldAsAnEmptyArray) {
  const TableId table{"proj-1", "ds_1", "tagged"};
  schema::TableSchema schema;
  schema::ColumnSchema id;
  id.name = "id";
  id.type = schema::ColumnType::kInt64;
  schema::ColumnSchema tags;
  tags.name = "tags";
  tags.type = schema::ColumnType::kString;
  tags.mode = schema::ColumnMode::kRepeated;
  schema.columns = {id, tags};

  auto store_or = DuckDBStorage::Open(data_dir_.string());
  ASSERT_TRUE(store_or.ok()) << store_or.status();
  auto& store = **store_or;
  ASSERT_TRUE(store.CreateDataset({"proj-1", "ds_1"}, "US").ok());
  ASSERT_TRUE(store.CreateTable(table, schema).ok());

  Row null_tags;
  null_tags.cells = {Value::Int64(1), Value::Null()};
  Row one_tag;
  one_tag.cells = {Value::Int64(2), Value::Array({Value::String("a")})};
  const std::vector<Row> rows = {null_tags, one_tag};
  ASSERT_TRUE(store.AppendRows(table, absl::MakeConstSpan(rows)).ok());

  auto iter_or = store.ScanRows(table);
  ASSERT_TRUE(iter_or.ok()) << iter_or.status();
  std::vector<size_t> sizes;
  Row row;
  while (true) {
    auto has = (*iter_or)->Next(&row);
    ASSERT_TRUE(has.ok()) << has.status();
    if (!*has) break;
    ASSERT_EQ(row.cells[1].kind(), Value::Kind::kArray);
    sizes.push_back(row.cells[1].array_value().size());
  }
  EXPECT_EQ(sizes, (std::vector<size_t>{0, 1}));
}

}  // namespace
}  // namespace duckdb
}  // namespace storage
}  // namespace backend
}  // namespace bigquery_emulator
