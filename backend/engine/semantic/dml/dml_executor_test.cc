// Unit tests for the local DML executor.
//
// We drive the analyzer against a small in-memory `FakeStorage` +
// `SimpleCatalog` populated with a `StorageTable` so the resolved
// AST the executor sees is the same shape the production
// coordinator produces. Storage round-trip is verified by reading
// back through `FakeStorage`.

#include "backend/engine/semantic/dml/dml_executor.h"

#include <cstddef>
#include <cstdint>
#include <map>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "absl/types/span.h"
#include "backend/catalog/storage_table.h"
#include "backend/engine/engine.h"
#include "backend/engine/semantic/error.h"
#include "backend/schema/schema.h"
#include "backend/storage/storage.h"
#include "googlesql/public/analyzer.h"
#include "googlesql/public/analyzer_options.h"
#include "googlesql/public/analyzer_output.h"
#include "googlesql/public/builtin_function_options.h"
#include "googlesql/public/catalog.h"
#include "googlesql/public/language_options.h"
#include "googlesql/public/options.pb.h"
#include "googlesql/public/simple_catalog.h"
#include "googlesql/public/type.h"
#include "googlesql/public/type.pb.h"
#include "googlesql/public/types/type_factory.h"
#include "googlesql/resolved_ast/resolved_ast.h"
#include "gtest/gtest.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace semantic {
namespace dml {
namespace {

// Minimal in-memory `Storage` impl. The DML executor only needs
// `GetSchema`, `AppendRows`, `OverwriteRows`, and `ScanRows`; the
// dataset/table CRUD, list, and Storage Read API methods stay
// `kUnimplemented` so the test stays focused.
class FakeStorage : public storage::Storage {
 public:
  // Pre-register a table at `id` with `schema`. Subsequent
  // append/overwrite/scan calls operate against the buffer keyed
  // by the canonical "<project>/<dataset>/<table>" string.
  void RegisterTable(const storage::TableId& id, schema::TableSchema schema) {
    schemas_[Key(id)] = std::move(schema);
    rows_[Key(id)] = {};
  }

  const std::vector<storage::Row>& Rows(const storage::TableId& id) const {
    return rows_.at(Key(id));
  }

  // ---- Storage interface ----
  absl::Status CreateDataset(const storage::DatasetId& /*id*/,
                             absl::string_view /*location*/) override {
    return absl::UnimplementedError("FakeStorage::CreateDataset");
  }
  absl::Status DropDataset(const storage::DatasetId& /*id*/,
                           bool /*delete_contents*/,
                           absl::string_view = {}) override {
    return absl::UnimplementedError("FakeStorage::DropDataset");
  }
  absl::Status CreateTable(const storage::TableId& /*id*/,
                           const schema::TableSchema& /*schema*/) override {
    return absl::UnimplementedError("FakeStorage::CreateTable");
  }
  absl::Status DropTable(const storage::TableId& /*id*/) override {
    return absl::UnimplementedError("FakeStorage::DropTable");
  }
  absl::StatusOr<std::vector<storage::DatasetId>> ListDatasets(
      absl::string_view /*project_id*/) const override {
    return absl::UnimplementedError("FakeStorage::ListDatasets");
  }
  absl::StatusOr<std::vector<storage::TableId>> ListTables(
      const storage::DatasetId& /*dataset_id*/) const override {
    return absl::UnimplementedError("FakeStorage::ListTables");
  }
  absl::StatusOr<schema::TableSchema> GetSchema(
      const storage::TableId& id) const override {
    auto it = schemas_.find(Key(id));
    if (it == schemas_.end()) {
      return absl::NotFoundError(
          "FakeStorage::GetSchema: table not registered");
    }
    return it->second;
  }
  absl::Status AppendRows(const storage::TableId& id,
                          absl::Span<const storage::Row> rows) override {
    auto it = rows_.find(Key(id));
    if (it == rows_.end()) {
      return absl::NotFoundError(
          "FakeStorage::AppendRows: table not registered");
    }
    for (const storage::Row& r : rows)
      it->second.push_back(r);
    return absl::OkStatus();
  }
  absl::Status OverwriteRows(const storage::TableId& id,
                             absl::Span<const storage::Row> rows) override {
    auto it = rows_.find(Key(id));
    if (it == rows_.end()) {
      return absl::NotFoundError(
          "FakeStorage::OverwriteRows: table not registered");
    }
    it->second.assign(rows.begin(), rows.end());
    return absl::OkStatus();
  }

  class VectorIterator : public storage::RowIterator {
   public:
    explicit VectorIterator(std::vector<storage::Row> rows)
        : rows_(std::move(rows)) {}
    absl::StatusOr<bool> Next(storage::Row* row) override {
      if (cursor_ >= rows_.size()) return false;
      *row = rows_[cursor_++];
      return true;
    }

   private:
    std::vector<storage::Row> rows_{};
    size_t cursor_ = 0;
  };

  absl::StatusOr<std::unique_ptr<storage::RowIterator>> ScanRows(
      const storage::TableId& id) const override {
    auto it = rows_.find(Key(id));
    if (it == rows_.end()) {
      return absl::NotFoundError("FakeStorage::ScanRows: table not registered");
    }
    return std::unique_ptr<storage::RowIterator>(
        new VectorIterator(it->second));
  }
  absl::StatusOr<std::unique_ptr<storage::RowIterator>> CreateReadStream(
      const storage::TableId& id,
      const storage::ReadFilter& /*filter*/) const override {
    return ScanRows(id);
  }
  absl::StatusOr<std::int64_t> CountRows(
      const storage::TableId& id) const override {
    auto it = rows_.find(Key(id));
    if (it == rows_.end()) {
      return absl::NotFoundError(
          "FakeStorage::CountRows: table not registered");
    }
    return static_cast<std::int64_t>(it->second.size());
  }
  absl::Status UpsertRoutine(
      const storage::RoutineRecord& /*record*/) override {
    return absl::UnimplementedError("FakeStorage::UpsertRoutine");
  }
  absl::Status DeleteRoutine(const storage::RoutineId& /*id*/) override {
    return absl::UnimplementedError("FakeStorage::DeleteRoutine");
  }
  absl::StatusOr<storage::RoutineRecord> GetRoutine(
      const storage::RoutineId& /*id*/) const override {
    return absl::UnimplementedError("FakeStorage::GetRoutine");
  }
  absl::StatusOr<std::vector<storage::RoutineRecord>> ListRoutines(
      const storage::DatasetId& /*dataset_id*/) const override {
    return absl::UnimplementedError("FakeStorage::ListRoutines");
  }
  absl::StatusOr<std::vector<storage::RoutineRecord>> ListAllRoutines()
      const override {
    return absl::UnimplementedError("FakeStorage::ListAllRoutines");
  }
  absl::Status UpsertView(const storage::ViewRecord&) override {
    return absl::UnimplementedError("FakeStorage::UpsertView");
  }
  absl::Status DeleteView(const storage::ViewId&) override {
    return absl::UnimplementedError("FakeStorage::DeleteView");
  }
  absl::StatusOr<std::vector<storage::ViewRecord>> ListAllViews()
      const override {
    return absl::UnimplementedError("FakeStorage::ListAllViews");
  }

 private:
  static std::string Key(const storage::TableId& id) {
    return id.project_id + "/" + id.dataset_id + "/" + id.table_id;
  }

  std::map<std::string, schema::TableSchema> schemas_;
  std::map<std::string, std::vector<storage::Row>> rows_;
};

// Test fixture: builds a `SimpleCatalog` that holds one
// `StorageTable` (named "people") backed by `storage_`. Both the
// columns the analyzer sees and the storage schema are identical:
// `id INT64, name STRING`.
class DmlExecutorTest : public ::testing::Test {
 protected:
  void SetUp() override {
    type_factory_ = std::make_unique<::googlesql::TypeFactory>();
    catalog_ = std::make_unique<::googlesql::SimpleCatalog>(
        "test_catalog", type_factory_.get());
    catalog_->AddBuiltinFunctions(
        ::googlesql::BuiltinFunctionOptions::AllReleasedFunctions());

    storage_ = std::make_unique<FakeStorage>();
    table_id_ = storage::TableId{"test_proj", "test_ds", "people"};
    schema::TableSchema schema;
    schema.columns.push_back({.name = "id",
                              .type = schema::ColumnType::kInt64,
                              .mode = schema::ColumnMode::kRequired});
    schema.columns.push_back({.name = "name",
                              .type = schema::ColumnType::kString,
                              .mode = schema::ColumnMode::kNullable});
    storage_->RegisterTable(table_id_, schema);

    std::vector<::googlesql::SimpleTable::NameAndType> cols = {
        {"id", type_factory_->get_int64()},
        {"name", type_factory_->get_string()},
    };
    auto storage_table = std::make_unique<catalog::StorageTable>(
        "people", "test_ds.people", cols, schema, table_id_, storage_.get());
    // The analyzer resolves multi-segment names by walking nested
    // catalogs. Surface the table under `test_ds.people` by attaching
    // it to a `test_ds` sub-catalog of the test root.
    ::googlesql::SimpleCatalog* dataset_catalog =
        catalog_->MakeOwnedSimpleCatalog("test_ds");
    dataset_catalog_ = dataset_catalog;
    dataset_catalog->AddOwnedTable(std::move(storage_table));
  }

  ::googlesql::AnalyzerOptions MakeOptions() {
    ::googlesql::LanguageOptions lang;
    lang.EnableMaximumLanguageFeatures();
    lang.set_product_mode(::googlesql::PRODUCT_EXTERNAL);
    lang.SetSupportsAllStatementKinds();
    ::googlesql::AnalyzerOptions opts(lang);
    opts.CreateDefaultArenasIfNotSet();
    return opts;
  }

  const ::googlesql::ResolvedStatement* Analyze(absl::string_view sql) {
    last_output_.reset();
    absl::Status s = ::googlesql::AnalyzeStatement(
        sql, MakeOptions(), catalog_.get(), type_factory_.get(), &last_output_);
    EXPECT_TRUE(s.ok()) << s;
    if (!s.ok() || last_output_ == nullptr) return nullptr;
    return last_output_->resolved_statement();
  }

  std::unique_ptr<::googlesql::TypeFactory> type_factory_{};
  std::unique_ptr<::googlesql::SimpleCatalog> catalog_{};
  std::unique_ptr<FakeStorage> storage_{};
  storage::TableId table_id_{};
  ::googlesql::SimpleCatalog* dataset_catalog_ = nullptr;
  std::unique_ptr<const ::googlesql::AnalyzerOutput> last_output_{};
};

// --- INSERT VALUES ---------------------------------------------------------

TEST_F(DmlExecutorTest, InsertValuesAppendsRowsAndCountsThem) {
  const auto* stmt = Analyze(
      "INSERT INTO test_ds.people (id, name) VALUES "
      "(1, 'ada'), (2, 'linus'), (3, 'grace')");
  ASSERT_NE(stmt, nullptr);
  ASSERT_EQ(stmt->node_kind(), ::googlesql::RESOLVED_INSERT_STMT);

  QueryRequest request;
  auto result = ExecuteDml(request, *stmt, catalog_.get(), storage_.get());
  ASSERT_TRUE(result.ok()) << result.status();
  EXPECT_EQ(result->stats.inserted_row_count, 3);
  EXPECT_EQ(result->stats.updated_row_count, 0);
  EXPECT_EQ(result->stats.deleted_row_count, 0);

  const auto& rows = storage_->Rows(table_id_);
  ASSERT_EQ(rows.size(), 3u);
  EXPECT_EQ(rows[0].cells[0].int64_value(), 1);
  EXPECT_EQ(rows[0].cells[1].string_value(), "ada");
  EXPECT_EQ(rows[1].cells[0].int64_value(), 2);
  EXPECT_EQ(rows[1].cells[1].string_value(), "linus");
  EXPECT_EQ(rows[2].cells[0].int64_value(), 3);
  EXPECT_EQ(rows[2].cells[1].string_value(), "grace");
}

TEST_F(DmlExecutorTest, InsertValuesOmittedColumnDefaultsNull) {
  // `INSERT (id) VALUES (1)` leaves `name` unbound; the executor
  // pads with NULL so the storage row matches the table schema.
  const auto* stmt = Analyze("INSERT INTO test_ds.people (id) VALUES (42)");
  ASSERT_NE(stmt, nullptr);

  QueryRequest request;
  auto result = ExecuteDml(request, *stmt, catalog_.get(), storage_.get());
  ASSERT_TRUE(result.ok()) << result.status();
  EXPECT_EQ(result->stats.inserted_row_count, 1);

  const auto& rows = storage_->Rows(table_id_);
  ASSERT_EQ(rows.size(), 1u);
  EXPECT_EQ(rows[0].cells[0].int64_value(), 42);
  EXPECT_TRUE(rows[0].cells[1].is_null());
}

TEST_F(DmlExecutorTest, InsertSelectFromLiteralRows) {
  const auto* stmt = Analyze(
      "INSERT INTO test_ds.people (id, name) "
      "SELECT 1, 'a'");
  ASSERT_NE(stmt, nullptr);

  QueryRequest request;
  auto result = ExecuteDml(request, *stmt, catalog_.get(), storage_.get());
  ASSERT_TRUE(result.ok()) << result.status();
  EXPECT_EQ(result->stats.inserted_row_count, 1);

  const auto& rows = storage_->Rows(table_id_);
  ASSERT_EQ(rows.size(), 1u);
  EXPECT_EQ(rows[0].cells[0].int64_value(), 1);
  EXPECT_EQ(rows[0].cells[1].string_value(), "a");
}

// --- DELETE -----------------------------------------------------------------

TEST_F(DmlExecutorTest, DeleteWherePredicateRemovesMatchingRows) {
  // Seed via INSERT so we exercise the round-trip.
  {
    const auto* seed = Analyze(
        "INSERT INTO test_ds.people (id, name) "
        "VALUES (1, 'ada'), (2, 'linus'), (3, 'grace')");
    ASSERT_NE(seed, nullptr);
    QueryRequest req;
    ASSERT_TRUE(ExecuteDml(req, *seed, catalog_.get(), storage_.get()).ok());
  }
  ASSERT_EQ(storage_->Rows(table_id_).size(), 3u);

  const auto* stmt = Analyze("DELETE FROM test_ds.people WHERE id = 2");
  ASSERT_NE(stmt, nullptr);
  ASSERT_EQ(stmt->node_kind(), ::googlesql::RESOLVED_DELETE_STMT);

  QueryRequest request;
  auto result = ExecuteDml(request, *stmt, catalog_.get(), storage_.get());
  ASSERT_TRUE(result.ok()) << result.status();
  EXPECT_EQ(result->stats.deleted_row_count, 1);

  const auto& rows = storage_->Rows(table_id_);
  ASSERT_EQ(rows.size(), 2u);
  EXPECT_EQ(rows[0].cells[0].int64_value(), 1);
  EXPECT_EQ(rows[1].cells[0].int64_value(), 3);
}

TEST_F(DmlExecutorTest, DeleteWhereTrueClearsTable) {
  {
    const auto* seed = Analyze(
        "INSERT INTO test_ds.people (id, name) "
        "VALUES (1, 'ada'), (2, 'linus')");
    QueryRequest req;
    ASSERT_TRUE(ExecuteDml(req, *seed, catalog_.get(), storage_.get()).ok());
  }
  const auto* stmt = Analyze("DELETE FROM test_ds.people WHERE TRUE");
  ASSERT_NE(stmt, nullptr);
  QueryRequest request;
  auto result = ExecuteDml(request, *stmt, catalog_.get(), storage_.get());
  ASSERT_TRUE(result.ok()) << result.status();
  EXPECT_EQ(result->stats.deleted_row_count, 2);
  EXPECT_TRUE(storage_->Rows(table_id_).empty());
}

// --- UPDATE -----------------------------------------------------------------

TEST_F(DmlExecutorTest, UpdateScalarSetMutatesMatchingRow) {
  {
    const auto* seed = Analyze(
        "INSERT INTO test_ds.people (id, name) "
        "VALUES (1, 'ada'), (2, 'linus'), (3, 'grace')");
    QueryRequest req;
    ASSERT_TRUE(ExecuteDml(req, *seed, catalog_.get(), storage_.get()).ok());
  }
  const auto* stmt =
      Analyze("UPDATE test_ds.people SET name = 'augusta' WHERE id = 1");
  ASSERT_NE(stmt, nullptr);
  ASSERT_EQ(stmt->node_kind(), ::googlesql::RESOLVED_UPDATE_STMT);

  QueryRequest request;
  auto result = ExecuteDml(request, *stmt, catalog_.get(), storage_.get());
  ASSERT_TRUE(result.ok()) << result.status();
  EXPECT_EQ(result->stats.updated_row_count, 1);

  const auto& rows = storage_->Rows(table_id_);
  ASSERT_EQ(rows.size(), 3u);
  EXPECT_EQ(rows[0].cells[1].string_value(), "augusta");
  EXPECT_EQ(rows[1].cells[1].string_value(), "linus");
  EXPECT_EQ(rows[2].cells[1].string_value(), "grace");
}

TEST_F(DmlExecutorTest, UpdateSetExprUsesSourceColumnRef) {
  {
    const auto* seed = Analyze(
        "INSERT INTO test_ds.people (id, name) "
        "VALUES (1, 'ada')");
    QueryRequest req;
    ASSERT_TRUE(ExecuteDml(req, *seed, catalog_.get(), storage_.get()).ok());
  }
  // SET col = col + 10 -- exercises ColumnRef-rebinding inside the
  // SET expression evaluator.
  const auto* stmt =
      Analyze("UPDATE test_ds.people SET id = id + 10 WHERE id = 1");
  ASSERT_NE(stmt, nullptr);

  QueryRequest request;
  auto result = ExecuteDml(request, *stmt, catalog_.get(), storage_.get());
  ASSERT_TRUE(result.ok()) << result.status();
  EXPECT_EQ(result->stats.updated_row_count, 1);
  EXPECT_EQ(storage_->Rows(table_id_)[0].cells[0].int64_value(), 11);
}

TEST_F(DmlExecutorTest, UpdateDeepStructSetMutatesNestedField) {
  storage::TableId struct_id{"test_proj", "test_ds", "items"};
  schema::TableSchema struct_schema;
  struct_schema.columns.push_back({.name = "id",
                                   .type = schema::ColumnType::kInt64,
                                   .mode = schema::ColumnMode::kRequired});
  schema::ColumnSchema nested_field;
  nested_field.name = "nested";
  nested_field.type = schema::ColumnType::kStruct;
  nested_field.mode = schema::ColumnMode::kNullable;
  nested_field.fields.push_back({.name = "val",
                                 .type = schema::ColumnType::kInt64,
                                 .mode = schema::ColumnMode::kNullable});
  nested_field.fields.push_back({.name = "tag",
                                 .type = schema::ColumnType::kString,
                                 .mode = schema::ColumnMode::kNullable});
  schema::ColumnSchema payload_col;
  payload_col.name = "payload";
  payload_col.type = schema::ColumnType::kStruct;
  payload_col.mode = schema::ColumnMode::kNullable;
  payload_col.fields.push_back(std::move(nested_field));
  struct_schema.columns.push_back(std::move(payload_col));
  storage_->RegisterTable(struct_id, struct_schema);

  const ::googlesql::StructType* inner_st = nullptr;
  ASSERT_TRUE(type_factory_
                  ->MakeStructType({{"val", type_factory_->get_int64()},
                                    {"tag", type_factory_->get_string()}},
                                   &inner_st)
                  .ok());
  const ::googlesql::StructType* payload_st = nullptr;
  ASSERT_TRUE(
      type_factory_->MakeStructType({{"nested", inner_st}}, &payload_st).ok());
  std::vector<::googlesql::SimpleTable::NameAndType> item_cols = {
      {"id", type_factory_->get_int64()},
      {"payload", payload_st},
  };
  auto item_table = std::make_unique<catalog::StorageTable>("items",
                                                            "test_ds.items",
                                                            item_cols,
                                                            struct_schema,
                                                            struct_id,
                                                            storage_.get());
  dataset_catalog_->AddOwnedTable(std::move(item_table));

  const auto* seed = Analyze(
      "INSERT INTO test_ds.items (id, payload) VALUES "
      "(1, STRUCT(STRUCT(10 AS val, 'keep' AS tag) AS nested))");
  ASSERT_NE(seed, nullptr);
  QueryRequest req;
  ASSERT_TRUE(ExecuteDml(req, *seed, catalog_.get(), storage_.get()).ok());

  const auto* stmt =
      Analyze("UPDATE test_ds.items SET payload.nested.val = 99 WHERE id = 1");
  ASSERT_NE(stmt, nullptr);
  QueryRequest request;
  auto result = ExecuteDml(request, *stmt, catalog_.get(), storage_.get());
  ASSERT_TRUE(result.ok()) << result.status();
  EXPECT_EQ(result->stats.updated_row_count, 1);
}

TEST_F(DmlExecutorTest, InsertAssertRowsModifiedMismatchRollsBack) {
  const auto* stmt = Analyze(
      "INSERT INTO test_ds.people (id, name) VALUES (1, 'ada') "
      "ASSERT_ROWS_MODIFIED 2");
  ASSERT_NE(stmt, nullptr);

  QueryRequest request;
  auto result = ExecuteDml(request, *stmt, catalog_.get(), storage_.get());
  ASSERT_FALSE(result.ok());
  EXPECT_NE(
      result.status().message().find("1 row(s), but 2 row(s) were expected"),
      std::string::npos);
  EXPECT_TRUE(storage_->Rows(table_id_).empty());
}

}  // namespace
}  // namespace dml
}  // namespace semantic
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
