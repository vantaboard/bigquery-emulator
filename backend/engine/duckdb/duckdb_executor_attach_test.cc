#include <vector>

#include "backend/engine/duckdb/duckdb_executor_test_fixture.h"
#include "backend/schema/schema.h"
#include "backend/storage/storage.h"
#include "googlesql/resolved_ast/resolved_ast.h"
#include "gtest/gtest.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace duckdb {
namespace {

TEST_F(DuckDbExecutorTest, FloatSumOverParquetAttachedTable) {
  schema::TableSchema bq_schema;
  schema::ColumnSchema id;
  id.name = "id";
  id.type = schema::ColumnType::kInt64;
  id.mode = schema::ColumnMode::kRequired;
  bq_schema.columns.push_back(id);
  schema::ColumnSchema value;
  value.name = "v";
  value.type = schema::ColumnType::kFloat64;
  value.mode = schema::ColumnMode::kRequired;
  bq_schema.columns.push_back(value);
  ASSERT_TRUE(storage_->CreateDataset({"proj-test", "ds"}, "US").ok());
  ASSERT_TRUE(
      storage_->CreateTable({"proj-test", "ds", "floats"}, bq_schema).ok());

  std::vector<storage::Row> rows = {
      storage::Row{{storage::Value::Int64(1), storage::Value::Float64(10.5)}},
      storage::Row{{storage::Value::Int64(2), storage::Value::Float64(2.25)}},
  };
  ASSERT_TRUE(
      storage_
          ->AppendRows({"proj-test", "ds", "floats"}, absl::MakeConstSpan(rows))
          .ok());

  CatalogBundle bundle = MakeCatalog();
  const std::string sql = "SELECT SUM(v) AS total FROM ds.floats";
  auto analyzed = Analyze(sql, bundle.catalog.get(), /*all_statements=*/false);
  ASSERT_TRUE(analyzed.ok()) << analyzed.status();
  const ::googlesql::ResolvedStatement* stmt =
      (*analyzed)->resolved_statement();
  ASSERT_NE(stmt, nullptr);

  absl::StatusOr<std::unique_ptr<RowSource>> source =
      executor_->ExecuteQuery(MakeRequest(sql), *stmt, bundle.catalog.get());
  ASSERT_TRUE(source.ok()) << source.status();

  storage::Row row;
  auto has = (*source)->Next(&row);
  ASSERT_TRUE(has.ok()) << has.status();
  ASSERT_TRUE(*has);
  ASSERT_EQ(row.cells.size(), 1u);
  ASSERT_EQ(row.cells[0].kind(), storage::Value::Kind::kFloat64);
  EXPECT_DOUBLE_EQ(row.cells[0].float64_value(), 12.75);
}

}  // namespace
}  // namespace duckdb
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
