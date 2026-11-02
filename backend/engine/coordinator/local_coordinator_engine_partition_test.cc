#include <cstdint>
#include <string>
#include <vector>

#include "absl/strings/str_cat.h"
#include "absl/strings/string_view.h"
#include "absl/types/span.h"
#include "backend/engine/coordinator/local_coordinator_engine_test_fixture.h"
#include "backend/schema/schema.h"
#include "backend/storage/storage.h"
#include "gtest/gtest.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace coordinator {
namespace {

constexpr absl::string_view kToday =
    "TIMESTAMP_TRUNC(CURRENT_TIMESTAMP(), DAY)";

class PartitionTimeTest : public LocalCoordinatorEngineTest {
 protected:
  // ds.events (id INT64), partitioned by ingestion day: id 1 is stamped
  // on insert, id 2 lands in the 2024-03-04 partition.
  void CreateEventsTable() {
    schema::TableSchema bq_schema;
    schema::ColumnSchema id;
    id.name = "id";
    id.type = schema::ColumnType::kInt64;
    bq_schema.columns.push_back(id);
    bq_schema.ingestion_partition_type = "DAY";
    ASSERT_TRUE(storage_->CreateDataset({"proj-test", "ds"}, "US").ok());
    ASSERT_TRUE(
        storage_->CreateTable({"proj-test", "ds", "events"}, bq_schema).ok());
    storage::Row stamped;
    stamped.cells = {storage::Value::Int64(1)};
    storage::Row given;
    given.cells = {storage::Value::Int64(2),
                   storage::Value::String("2024-03-04 00:00:00+00")};
    const std::vector<storage::Row> rows = {stamped, given};
    ASSERT_TRUE(storage_
                    ->AppendRows({"proj-test", "ds", "events"},
                                 absl::MakeConstSpan(rows))
                    .ok());
  }

  std::vector<int64_t> Ids(const std::string& sql) {
    CatalogBundle bundle = MakeCatalog();
    auto source = engine_->ExecuteQuery(MakeRequest(sql), bundle.catalog.get());
    EXPECT_TRUE(source.ok()) << source.status();
    std::vector<int64_t> ids;
    if (!source.ok()) return ids;
    storage::Row row;
    while (true) {
      auto has = (*source)->Next(&row);
      EXPECT_TRUE(has.ok()) << has.status();
      if (!has.ok() || !*has) break;
      ids.push_back(row.cells[0].int64_value());
    }
    return ids;
  }

  void Dml(const std::string& sql) {
    CatalogBundle bundle = MakeCatalog();
    auto result = engine_->ExecuteDml(MakeRequest(sql), bundle.catalog.get());
    ASSERT_TRUE(result.ok()) << sql << ": " << result.status();
  }
};

TEST_F(PartitionTimeTest, SelectStarLeavesThePartitionTimeOut) {
  CreateEventsTable();
  CatalogBundle bundle = MakeCatalog();
  auto source = engine_->ExecuteQuery(MakeRequest("SELECT * FROM ds.events"),
                                      bundle.catalog.get());
  ASSERT_TRUE(source.ok()) << source.status();
  ASSERT_EQ((*source)->schema().columns.size(), 1u);
  EXPECT_EQ((*source)->schema().columns[0].name, "id");
}

TEST_F(PartitionTimeTest, FiltersOnThePartitionTime) {
  CreateEventsTable();
  EXPECT_EQ(Ids("SELECT id FROM ds.events "
                "WHERE _PARTITIONTIME = TIMESTAMP '2024-03-04'"),
            std::vector<int64_t>{2});
  EXPECT_EQ(Ids(absl::StrCat("SELECT id FROM ds.events WHERE _PARTITIONTIME = ",
                             kToday)),
            std::vector<int64_t>{1});
}

TEST_F(PartitionTimeTest, DmlStampsInsertsAndKeepsExistingPartitions) {
  CreateEventsTable();
  Dml("INSERT INTO ds.events (id) VALUES (3)");
  Dml("INSERT INTO ds.events VALUES (4)");
  Dml("UPDATE ds.events SET id = 20 WHERE id = 2");

  EXPECT_EQ(Ids("SELECT id FROM ds.events "
                "WHERE _PARTITIONTIME = TIMESTAMP '2024-03-04'"),
            std::vector<int64_t>{20});
  EXPECT_EQ(Ids(absl::StrCat("SELECT id FROM ds.events WHERE _PARTITIONTIME = ",
                             kToday,
                             " ORDER BY id")),
            (std::vector<int64_t>{1, 3, 4}));
}

}  // namespace
}  // namespace coordinator
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
