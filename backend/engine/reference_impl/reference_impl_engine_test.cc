// End-to-end tests for the GoogleSQL reference-impl engine. The tests
// build a small in-memory storage instance, wrap it in a
// `GoogleSqlCatalog`, drive the engine through its `Analyze`,
// `DryRun`, and `ExecuteQuery` methods, and verify the resulting row
// stream + reflected output schema. We deliberately stay one layer
// below the gRPC service boundary: the engine accepts a
// `googlesql::Catalog*` directly so the tests don't have to spin up
// the frontend.

#include "backend/engine/reference_impl/reference_impl_engine.h"

#include <cstdint>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "absl/status/status.h"
#include "absl/strings/string_view.h"
#include "backend/catalog/googlesql_catalog.h"
#include "backend/engine/engine.h"
#include "backend/schema/schema.h"
#include "backend/storage/memory/in_memory_storage.h"
#include "backend/storage/storage.h"
#include "googlesql/public/analyzer_options.h"
#include "googlesql/public/language_options.h"
#include "googlesql/public/options.pb.h"
#include "googlesql/public/types/type_factory.h"
#include "gtest/gtest.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace reference_impl {
namespace {

// Mirrors the AnalyzerOptions the engine itself uses; the test
// reconstructs it so the per-call `GoogleSqlCatalog` registers
// builtins through the same `LanguageOptions` snapshot. If these two
// drift, simple queries like `SELECT 1` keep working but the
// matching error / function-resolution surface gets subtly different
// (e.g. PRODUCT_INTERNAL re-exposes engine-internal scalar names).
::googlesql::LanguageOptions MakeLanguageOptions() {
  ::googlesql::LanguageOptions language;
  language.EnableMaximumLanguageFeatures();
  language.set_product_mode(::googlesql::PRODUCT_EXTERNAL);
  language.set_name_resolution_mode(::googlesql::NAME_RESOLUTION_DEFAULT);
  return language;
}

// Fixture: every test gets a fresh `InMemoryStorage` and a fresh
// engine pointing at it. The catalog is constructed per query
// (mirroring the gateway's per-RPC catalog lifetime), wrapped in a
// helper that also exposes its TypeFactory so the test can keep them
// alive together.
class ReferenceImplEngineTest : public ::testing::Test {
 protected:
  void SetUp() override {
    storage_ = std::make_unique<storage::memory::InMemoryStorage>();
    engine_ = std::make_unique<ReferenceImplEngine>(storage_.get());
  }

  QueryRequest MakeRequest(absl::string_view sql) {
    QueryRequest req;
    req.project_id = "proj-test";
    req.sql = std::string(sql);
    return req;
  }

  // Reusable people table: same shape as the existing
  // `QueryServiceTest::CreatePeopleTable` (id INT64 REQUIRED, name
  // STRING NULLABLE, tags STRING REPEATED).
  void CreatePeopleTable() {
    schema::TableSchema bq_schema;
    schema::ColumnSchema id;
    id.name = "id";
    id.type = schema::ColumnType::kInt64;
    id.mode = schema::ColumnMode::kRequired;
    bq_schema.columns.push_back(id);
    schema::ColumnSchema name;
    name.name = "name";
    name.type = schema::ColumnType::kString;
    name.mode = schema::ColumnMode::kNullable;
    bq_schema.columns.push_back(name);
    schema::ColumnSchema tags;
    tags.name = "tags";
    tags.type = schema::ColumnType::kString;
    tags.mode = schema::ColumnMode::kRepeated;
    bq_schema.columns.push_back(tags);
    ASSERT_TRUE(storage_->CreateDataset({"proj-test", "ds"}, "US").ok());
    ASSERT_TRUE(
        storage_->CreateTable({"proj-test", "ds", "people"}, bq_schema).ok());

    auto make_row = [](int64_t id, std::string name,
                       std::vector<std::string> tags) {
      storage::Row r;
      std::vector<storage::Value> tag_cells;
      tag_cells.reserve(tags.size());
      for (auto& t : tags) {
        tag_cells.push_back(storage::Value::String(std::move(t)));
      }
      r.cells = {
          storage::Value::Int64(id),
          storage::Value::String(std::move(name)),
          storage::Value::Array(std::move(tag_cells)),
      };
      return r;
    };
    std::vector<storage::Row> rows = {
        make_row(1, "ada", {"math", "lace"}),
        make_row(2, "linus", {"kernel"}),
        make_row(3, "grace", {}),
    };
    ASSERT_TRUE(storage_
                    ->AppendRows({"proj-test", "ds", "people"},
                                  absl::MakeConstSpan(rows))
                    .ok());
  }

  // Convenience: build a `GoogleSqlCatalog` pointing at `storage_`
  // for the duration of one call. The test owns the catalog +
  // type_factory so they stay alive long enough for the iterator's
  // type pointers to remain valid.
  struct CatalogBundle {
    std::unique_ptr<::googlesql::TypeFactory> type_factory;
    std::unique_ptr<catalog::GoogleSqlCatalog> catalog;
  };
  CatalogBundle MakeCatalog() {
    auto type_factory = std::make_unique<::googlesql::TypeFactory>();
    auto catalog = std::make_unique<catalog::GoogleSqlCatalog>(
        "proj-test", storage_.get(), type_factory.get(),
        MakeLanguageOptions());
    return {std::move(type_factory), std::move(catalog)};
  }

  std::unique_ptr<storage::memory::InMemoryStorage> storage_;
  std::unique_ptr<ReferenceImplEngine> engine_;
};

TEST_F(ReferenceImplEngineTest, AnalyzeSelect1ReturnsInt64Column) {
  CatalogBundle bundle = MakeCatalog();
  auto analyzed = engine_->Analyze(MakeRequest("SELECT 1"), bundle.catalog.get());
  ASSERT_TRUE(analyzed.ok()) << analyzed.status();
  const schema::TableSchema& s = (*analyzed)->output_schema();
  ASSERT_EQ(s.columns.size(), 1u);
  EXPECT_EQ(s.columns[0].type, schema::ColumnType::kInt64);
}

TEST_F(ReferenceImplEngineTest, DryRunSelect1ReturnsInt64Column) {
  CatalogBundle bundle = MakeCatalog();
  auto dry_run = engine_->DryRun(MakeRequest("SELECT 1"), bundle.catalog.get());
  ASSERT_TRUE(dry_run.ok()) << dry_run.status();
  ASSERT_EQ(dry_run->schema.columns.size(), 1u);
  EXPECT_EQ(dry_run->schema.columns[0].type, schema::ColumnType::kInt64);
  EXPECT_EQ(dry_run->estimated_bytes_processed, 0);
}

TEST_F(ReferenceImplEngineTest, ExecuteQuerySelect1StreamsOneRow) {
  CatalogBundle bundle = MakeCatalog();
  auto source = engine_->ExecuteQuery(MakeRequest("SELECT 1 AS one"),
                                       bundle.catalog.get());
  ASSERT_TRUE(source.ok()) << source.status();
  const schema::TableSchema& s = (*source)->schema();
  ASSERT_EQ(s.columns.size(), 1u);
  EXPECT_EQ(s.columns[0].name, "one");
  EXPECT_EQ(s.columns[0].type, schema::ColumnType::kInt64);

  storage::Row row;
  auto has = (*source)->Next(&row);
  ASSERT_TRUE(has.ok()) << has.status();
  ASSERT_TRUE(*has);
  ASSERT_EQ(row.cells.size(), 1u);
  EXPECT_EQ(row.cells[0].kind(), storage::Value::Kind::kInt64);
  EXPECT_EQ(row.cells[0].int64_value(), 1);

  has = (*source)->Next(&row);
  ASSERT_TRUE(has.ok()) << has.status();
  EXPECT_FALSE(*has);
}

TEST_F(ReferenceImplEngineTest, ExecuteQueryFromTableStreamsAllRows) {
  CreatePeopleTable();
  CatalogBundle bundle = MakeCatalog();
  auto source = engine_->ExecuteQuery(
      MakeRequest("SELECT id, name FROM ds.people ORDER BY id"),
      bundle.catalog.get());
  ASSERT_TRUE(source.ok()) << source.status();
  const schema::TableSchema& s = (*source)->schema();
  ASSERT_EQ(s.columns.size(), 2u);
  EXPECT_EQ(s.columns[0].name, "id");
  EXPECT_EQ(s.columns[0].type, schema::ColumnType::kInt64);
  EXPECT_EQ(s.columns[1].name, "name");
  EXPECT_EQ(s.columns[1].type, schema::ColumnType::kString);

  std::vector<std::pair<int64_t, std::string>> seen;
  storage::Row row;
  while (true) {
    auto has = (*source)->Next(&row);
    ASSERT_TRUE(has.ok()) << has.status();
    if (!*has) break;
    ASSERT_EQ(row.cells.size(), 2u);
    ASSERT_EQ(row.cells[0].kind(), storage::Value::Kind::kInt64);
    ASSERT_EQ(row.cells[1].kind(), storage::Value::Kind::kString);
    seen.emplace_back(row.cells[0].int64_value(), row.cells[1].string_value());
  }
  ASSERT_EQ(seen.size(), 3u);
  EXPECT_EQ(seen[0].first, 1);
  EXPECT_EQ(seen[0].second, "ada");
  EXPECT_EQ(seen[1].first, 2);
  EXPECT_EQ(seen[1].second, "linus");
  EXPECT_EQ(seen[2].first, 3);
  EXPECT_EQ(seen[2].second, "grace");
}

TEST_F(ReferenceImplEngineTest, ExecuteQuerySyntaxErrorIsInvalidArgument) {
  CatalogBundle bundle = MakeCatalog();
  auto source = engine_->ExecuteQuery(MakeRequest("SELECT FROM"),
                                       bundle.catalog.get());
  ASSERT_FALSE(source.ok());
  EXPECT_EQ(source.status().code(), absl::StatusCode::kInvalidArgument)
      << source.status();
}

TEST_F(ReferenceImplEngineTest, ExecuteQueryRejectsLegacySql) {
  CatalogBundle bundle = MakeCatalog();
  QueryRequest req = MakeRequest("SELECT 1");
  req.use_legacy_sql = true;
  auto source = engine_->ExecuteQuery(req, bundle.catalog.get());
  ASSERT_FALSE(source.ok());
  EXPECT_EQ(source.status().code(), absl::StatusCode::kInvalidArgument);
}

TEST_F(ReferenceImplEngineTest, ExecuteQueryRejectsNullCatalog) {
  auto source = engine_->ExecuteQuery(MakeRequest("SELECT 1"), nullptr);
  ASSERT_FALSE(source.ok());
  EXPECT_EQ(source.status().code(), absl::StatusCode::kFailedPrecondition);
}

}  // namespace
}  // namespace reference_impl
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
