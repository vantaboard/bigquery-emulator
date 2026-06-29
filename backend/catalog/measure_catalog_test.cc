
#include "backend/schema/schema.h"
#include "gtest/gtest.h"

namespace bigquery_emulator {
namespace backend {
namespace catalog {
namespace {

TEST(MeasureCatalogTest, ParseMeasureColumnSpecFromDescription) {
  schema::ColumnSchema column;
  column.name = "total_sales";
  column.type = schema::ColumnType::kInt64;
  column.description = "bqemu_measure:SUM(amount):store_id";
  std::optional<MeasureColumnSpec> spec = ParseMeasureColumnSpec(column);
  ASSERT_TRUE(spec.has_value());
  EXPECT_EQ(spec->name, "total_sales");
  EXPECT_EQ(spec->expression, "SUM(amount)");
  ASSERT_EQ(spec->row_key_names.size(), 1u);
  EXPECT_EQ(spec->row_key_names[0], "store_id");
}

}  // namespace
}  // namespace catalog
}  // namespace backend
}  // namespace bigquery_emulator
