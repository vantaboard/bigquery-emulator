#include "backend/sqltools/catalog_names.h"

#include <map>
#include <string>
#include <vector>

#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "absl/strings/str_cat.h"
#include "absl/types/span.h"
#include "backend/schema/schema.h"
#include "backend/storage/storage.h"
#include "gtest/gtest.h"

namespace bigquery_emulator {
namespace backend {
namespace sqltools {
namespace {

class FakeCatalogStorage : public storage::Storage {
 public:
  void AddDataset(absl::string_view project_id, absl::string_view dataset_id) {
    datasets_[std::string(project_id)].push_back(
        storage::DatasetId{std::string(project_id), std::string(dataset_id)});
  }

  void AddTable(const storage::TableId& id, schema::TableSchema schema) {
    tables_[id.dataset_id].push_back(id);
    schemas_[TableKey(id)] = std::move(schema);
  }

  void AddRoutine(const storage::DatasetId& dataset,
                  storage::RoutineRecord routine) {
    routines_[dataset.dataset_id].push_back(std::move(routine));
  }

  absl::Status CreateDataset(const storage::DatasetId&,
                             absl::string_view) override {
    return absl::UnimplementedError("FakeCatalogStorage::CreateDataset");
  }
  absl::Status DropDataset(const storage::DatasetId&, bool) override {
    return absl::UnimplementedError("FakeCatalogStorage::DropDataset");
  }
  absl::Status CreateTable(const storage::TableId&,
                           const schema::TableSchema&) override {
    return absl::UnimplementedError("FakeCatalogStorage::CreateTable");
  }
  absl::Status DropTable(const storage::TableId&) override {
    return absl::UnimplementedError("FakeCatalogStorage::DropTable");
  }
  absl::StatusOr<std::vector<storage::DatasetId>> ListDatasets(
      absl::string_view project_id) const override {
    auto it = datasets_.find(std::string(project_id));
    if (it == datasets_.end()) {
      return std::vector<storage::DatasetId>{};
    }
    return it->second;
  }
  absl::StatusOr<std::vector<storage::TableId>> ListTables(
      const storage::DatasetId& dataset) const override {
    auto it = tables_.find(dataset.dataset_id);
    if (it == tables_.end()) {
      return std::vector<storage::TableId>{};
    }
    return it->second;
  }
  absl::StatusOr<schema::TableSchema> GetSchema(
      const storage::TableId& id) const override {
    auto it = schemas_.find(TableKey(id));
    if (it == schemas_.end()) {
      return absl::NotFoundError("schema not found");
    }
    return it->second;
  }
  absl::Status AppendRows(const storage::TableId&,
                          absl::Span<const storage::Row>) override {
    return absl::UnimplementedError("FakeCatalogStorage::AppendRows");
  }
  absl::Status OverwriteRows(const storage::TableId&,
                             absl::Span<const storage::Row>) override {
    return absl::UnimplementedError("FakeCatalogStorage::OverwriteRows");
  }
  absl::StatusOr<std::unique_ptr<storage::RowIterator>> ScanRows(
      const storage::TableId&) const override {
    return absl::UnimplementedError("FakeCatalogStorage::ScanRows");
  }
  absl::StatusOr<std::unique_ptr<storage::RowIterator>> CreateReadStream(
      const storage::TableId&, const storage::ReadFilter&) const override {
    return absl::UnimplementedError("FakeCatalogStorage::CreateReadStream");
  }
  absl::StatusOr<std::int64_t> CountRows(
      const storage::TableId&) const override {
    return absl::UnimplementedError("FakeCatalogStorage::CountRows");
  }
  absl::Status UpsertRoutine(const storage::RoutineRecord&) override {
    return absl::UnimplementedError("FakeCatalogStorage::UpsertRoutine");
  }
  absl::Status DeleteRoutine(const storage::RoutineId&) override {
    return absl::UnimplementedError("FakeCatalogStorage::DeleteRoutine");
  }
  absl::StatusOr<storage::RoutineRecord> GetRoutine(
      const storage::RoutineId&) const override {
    return absl::UnimplementedError("FakeCatalogStorage::GetRoutine");
  }
  absl::StatusOr<std::vector<storage::RoutineRecord>> ListRoutines(
      const storage::DatasetId& dataset) const override {
    auto it = routines_.find(dataset.dataset_id);
    if (it == routines_.end()) {
      return std::vector<storage::RoutineRecord>{};
    }
    return it->second;
  }
  absl::StatusOr<std::vector<storage::RoutineRecord>> ListAllRoutines()
      const override {
    return absl::UnimplementedError("FakeCatalogStorage::ListAllRoutines");
  }
  absl::Status UpsertView(const storage::ViewRecord&) override {
    return absl::UnimplementedError("FakeCatalogStorage::UpsertView");
  }
  absl::Status DeleteView(const storage::ViewId&) override {
    return absl::UnimplementedError("FakeCatalogStorage::DeleteView");
  }
  absl::StatusOr<std::vector<storage::ViewRecord>> ListAllViews()
      const override {
    return absl::UnimplementedError("FakeCatalogStorage::ListAllViews");
  }

 private:
  static std::string TableKey(const storage::TableId& id) {
    return absl::StrCat(id.project_id, "/", id.dataset_id, "/", id.table_id);
  }

  std::map<std::string, std::vector<storage::DatasetId>> datasets_{};
  std::map<std::string, std::vector<storage::TableId>> tables_{};
  std::map<std::string, schema::TableSchema> schemas_{};
  std::map<std::string, std::vector<storage::RoutineRecord>> routines_{};
};

TEST(CatalogNamesTest, PopulateFromStorageIncludesTablesColumnsAndRoutines) {
  FakeCatalogStorage storage;
  storage.AddDataset("proj", "ds");
  storage.AddDataset("proj", "other");

  const storage::TableId table_id{"proj", "ds", "events"};
  schema::TableSchema schema;
  schema.columns = {
      schema::ColumnSchema{.name = "id", .type = schema::ColumnType::kInt64},
      schema::ColumnSchema{.name = "name", .type = schema::ColumnType::kString},
  };
  storage.AddTable(table_id, schema);

  storage::RoutineRecord routine;
  routine.id = storage::RoutineId{"proj", "ds", "my_fn"};
  routine.language = "SQL";
  storage.AddRoutine(storage::DatasetId{"proj", "ds"}, routine);

  CatalogNames names;
  ASSERT_TRUE(
      PopulateCatalogNamesFromStorage("proj", "ds", &storage, &names).ok());

  ASSERT_EQ(names.datasets,
            (std::vector<std::string>{"ds", "proj.ds", "other", "proj.other"}));

  ASSERT_EQ(names.tables.size(), 3u);
  EXPECT_EQ(names.tables[0].label, "ds.events");
  EXPECT_EQ(names.tables[0].fqn, "proj.ds.events");
  EXPECT_EQ(names.tables[0].kind, "table");
  EXPECT_EQ(names.tables[1].label, "proj.ds.events");
  EXPECT_EQ(names.tables[1].fqn, "proj.ds.events");
  EXPECT_EQ(names.tables[2].label, "events");
  EXPECT_EQ(names.tables[2].fqn, "proj.ds.events");

  ASSERT_EQ(names.routines.size(), 3u);
  EXPECT_EQ(names.routines[0].label, "ds.my_fn");
  EXPECT_EQ(names.routines[0].detail, "SQL scalar function");
  EXPECT_EQ(names.routines[0].kind, "routine");
  EXPECT_EQ(names.routines[1].label, "proj.ds.my_fn");
  EXPECT_EQ(names.routines[2].label, "my_fn");

  ASSERT_EQ(names.columns.size(), 2u);
  EXPECT_EQ(names.columns[0].name, "id");
  EXPECT_EQ(names.columns[0].type, "INT64");
  EXPECT_EQ(names.columns[1].name, "name");
  EXPECT_EQ(names.columns[1].type, "STRING");

  const auto qualified = names.columns_by_table.find("ds.events");
  ASSERT_NE(qualified, names.columns_by_table.end());
  EXPECT_EQ(qualified->second.size(), 2u);

  const auto unqualified = names.columns_by_table.find("events");
  ASSERT_NE(unqualified, names.columns_by_table.end());
  EXPECT_EQ(unqualified->second.size(), 2u);
}

TEST(CatalogNamesTest, PopulateFromStorageIncludesProjectQualifiedNames) {
  FakeCatalogStorage storage;
  storage.AddDataset("proj", "ds");

  const storage::TableId table_id{"proj", "ds", "events"};
  schema::TableSchema schema;
  schema.columns = {
      schema::ColumnSchema{.name = "id", .type = schema::ColumnType::kInt64},
  };
  storage.AddTable(table_id, schema);

  CatalogNames names;
  ASSERT_TRUE(
      PopulateCatalogNamesFromStorage("proj", "", &storage, &names).ok());

  bool found_fqn_table = false;
  bool found_fqn_dataset = false;
  for (const CatalogTableEntry& table : names.tables) {
    if (table.label == "proj.ds.events") {
      found_fqn_table = true;
    }
  }
  for (const std::string& dataset : names.datasets) {
    if (dataset == "proj.ds") {
      found_fqn_dataset = true;
    }
  }
  EXPECT_TRUE(found_fqn_table);
  EXPECT_TRUE(found_fqn_dataset);
}

TEST(CatalogNamesTest, RejectsNullStorage) {
  CatalogNames names;
  EXPECT_FALSE(
      PopulateCatalogNamesFromStorage("proj", "ds", nullptr, &names).ok());
}

TEST(CatalogNamesTest, RejectsNullNames) {
  FakeCatalogStorage storage;
  storage.AddDataset("proj", "ds");
  EXPECT_FALSE(
      PopulateCatalogNamesFromStorage("proj", "ds", &storage, nullptr).ok());
}

}  // namespace
}  // namespace sqltools
}  // namespace backend
}  // namespace bigquery_emulator
