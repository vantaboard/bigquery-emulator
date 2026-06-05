// Per-handler unit tests for the control-op executor. Each test
// drives a single `Resolved*Stmt` shape directly through the
// executor's `ExecuteDdl` surface (the same surface the coordinator
// dispatches to for `kControlOp` rows in
// `node_dispositions.yaml`) and asserts the catalog mutation lands
// on the underlying `Storage` backend.
//
// We deliberately avoid the gateway path here -- the gateway's
// statementType envelope is exercised by `gateway/e2e/` integration
// tests. This file is the per-handler "did the storage row change"
// pin; it lets a regression in the handler's name-path resolution,
// schema mapping, or storage-write call surface as a unit-test
// failure first.
//
// Plan ownership: `.cursor/plans/local-exec-01-ddl-catalog.plan.md` Tests
// section.

#include "backend/engine/control/control_op_executor.h"

#include <algorithm>
#include <cstdint>
#include <cstdlib>
#include <filesystem>
#include <memory>
#include <random>
#include <string>
#include <system_error>
#include <utility>
#include <vector>

#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/string_view.h"
#include "backend/catalog/googlesql_catalog.h"
#include "backend/engine/engine.h"
#include "backend/schema/schema.h"
#include "backend/storage/duckdb/duckdb_storage.h"
#include "backend/storage/storage.h"
#include "googlesql/public/analyzer.h"
#include "googlesql/public/analyzer_options.h"
#include "googlesql/public/analyzer_output.h"
#include "googlesql/public/language_options.h"
#include "googlesql/public/options.pb.h"
#include "googlesql/public/types/type_factory.h"
#include "googlesql/resolved_ast/resolved_ast.h"
#include "gtest/gtest.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace control {
namespace {

namespace fs = std::filesystem;

::googlesql::LanguageOptions MakeLanguageOptions() {
  ::googlesql::LanguageOptions language;
  language.EnableMaximumLanguageFeatures();
  language.set_product_mode(::googlesql::PRODUCT_EXTERNAL);
  language.set_name_resolution_mode(::googlesql::NAME_RESOLUTION_DEFAULT);
  return language;
}

// Mirrors `LocalCoordinatorEngine`'s analyzer setup with the
// supports-all-statements allowlist flipped on so DDL parses.
::googlesql::AnalyzerOptions MakeAnalyzerOptions() {
  ::googlesql::AnalyzerOptions options(MakeLanguageOptions());
  options.set_error_message_mode(::googlesql::ERROR_MESSAGE_ONE_LINE);
  options.set_attach_error_location_payload(true);
  options.CreateDefaultArenasIfNotSet();
  options.mutable_language()->SetSupportsAllStatementKinds();
  return options;
}

class ControlOpExecutorTest : public ::testing::Test {
 protected:
  void SetUp() override {
    const char* tmpdir_env = std::getenv("TMPDIR");
    const std::string tmpdir = tmpdir_env != nullptr ? tmpdir_env : "/tmp";
    std::random_device rd;
    std::seed_seq seed{rd(), rd()};
    std::mt19937_64 rng(seed);
    data_dir_ =
        fs::path(tmpdir) / absl::StrCat("bqemu-control-op-test-", rng());
    std::error_code ec;
    fs::remove_all(data_dir_, ec);
    auto opened = storage::duckdb::DuckDBStorage::Open(data_dir_.string());
    ASSERT_TRUE(opened.ok()) << opened.status();
    storage_ = std::move(opened).value();
    executor_ = std::make_unique<ControlOpExecutor>(storage_.get());
    ASSERT_TRUE(storage_->CreateDataset({"proj-test", "ds"}, "US").ok());
  }

  void TearDown() override {
    executor_.reset();
    storage_.reset();
    std::error_code ec;
    fs::remove_all(data_dir_, ec);
  }

  QueryRequest MakeRequest(absl::string_view sql) {
    QueryRequest req;
    req.project_id = "proj-test";
    req.sql = std::string(sql);
    return req;
  }

  // Two-column people table (id INT64 REQUIRED, name STRING
  // NULLABLE). Same fixture the engine tests use so the executor's
  // post-mutation schema stays aligned with the engine path's.
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
    ASSERT_TRUE(
        storage_->CreateTable({"proj-test", "ds", "people"}, bq_schema).ok());

    auto make_row = [](int64_t id_val, std::string name_val) {
      storage::Row r;
      r.cells = {
          storage::Value::Int64(id_val),
          storage::Value::String(std::move(name_val)),
      };
      return r;
    };
    std::vector<storage::Row> rows = {
        make_row(1, "ada"),
        make_row(2, "linus"),
        make_row(3, "grace"),
    };
    ASSERT_TRUE(storage_
                    ->AppendRows({"proj-test", "ds", "people"},
                                 absl::MakeConstSpan(rows))
                    .ok());
  }

  struct CatalogBundle {
    std::unique_ptr<::googlesql::TypeFactory> type_factory{};
    std::unique_ptr<catalog::GoogleSqlCatalog> catalog{};
  };
  CatalogBundle MakeCatalog() {
    auto type_factory = std::make_unique<::googlesql::TypeFactory>();
    auto catalog = std::make_unique<catalog::GoogleSqlCatalog>(
        "proj-test", storage_.get(), type_factory.get(), MakeLanguageOptions());
    return {std::move(type_factory), std::move(catalog)};
  }

  // Analyze `sql` and run it through `ExecuteDdl`, returning the
  // status the executor surfaces. The `AnalyzerOutput` outlives the
  // call by virtue of being held in the local; the executor only
  // touches the resolved AST during the call.
  absl::Status RunDdl(absl::string_view sql) {
    CatalogBundle bundle = MakeCatalog();
    ::googlesql::AnalyzerOptions options = MakeAnalyzerOptions();
    ::googlesql::TypeFactory type_factory;
    std::unique_ptr<const ::googlesql::AnalyzerOutput> output;
    absl::Status analyze = ::googlesql::AnalyzeStatement(
        sql, options, bundle.catalog.get(), &type_factory, &output);
    if (!analyze.ok()) return analyze;
    if (output == nullptr || output->resolved_statement() == nullptr) {
      return absl::InternalError(
          "ControlOpExecutorTest::RunDdl: analyzer produced no resolved "
          "statement");
    }
    return executor_->ExecuteDdl(
        MakeRequest(sql), *output->resolved_statement(), bundle.catalog.get());
  }

  fs::path data_dir_{};
  std::unique_ptr<storage::duckdb::DuckDBStorage> storage_{};
  std::unique_ptr<ControlOpExecutor> executor_{};
};

// --- CREATE TABLE --------------------------------------------------------

TEST_F(ControlOpExecutorTest, CreateTableWritesSchemaToStorage) {
  absl::Status s = RunDdl("CREATE TABLE ds.t (id INT64, name STRING)");
  ASSERT_TRUE(s.ok()) << s;

  auto schema = storage_->GetSchema({"proj-test", "ds", "t"});
  ASSERT_TRUE(schema.ok()) << schema.status();
  ASSERT_EQ(schema->columns.size(), 2u);
  EXPECT_EQ(schema->columns[0].name, "id");
  EXPECT_EQ(schema->columns[0].type, schema::ColumnType::kInt64);
  EXPECT_EQ(schema->columns[1].name, "name");
  EXPECT_EQ(schema->columns[1].type, schema::ColumnType::kString);
}

TEST_F(ControlOpExecutorTest, CreateTableWithNestedStructColumn) {
  absl::Status s =
      RunDdl("CREATE TABLE ds.t (k INT64, s STRUCT<a INT64, b STRING>)");
  ASSERT_TRUE(s.ok()) << s;

  auto schema = storage_->GetSchema({"proj-test", "ds", "t"});
  ASSERT_TRUE(schema.ok()) << schema.status();
  ASSERT_EQ(schema->columns.size(), 2u);
  EXPECT_EQ(schema->columns[0].name, "k");
  EXPECT_EQ(schema->columns[0].type, schema::ColumnType::kInt64);
  ASSERT_EQ(schema->columns[1].name, "s");
  EXPECT_EQ(schema->columns[1].type, schema::ColumnType::kStruct);
  ASSERT_EQ(schema->columns[1].fields.size(), 2u);
  EXPECT_EQ(schema->columns[1].fields[0].name, "a");
  EXPECT_EQ(schema->columns[1].fields[0].type, schema::ColumnType::kInt64);
  EXPECT_EQ(schema->columns[1].fields[1].name, "b");
  EXPECT_EQ(schema->columns[1].fields[1].type, schema::ColumnType::kString);
}

TEST_F(ControlOpExecutorTest, CreateTableHonoursNotNullAnnotation) {
  absl::Status s = RunDdl("CREATE TABLE ds.t (id INT64 NOT NULL, name STRING)");
  ASSERT_TRUE(s.ok()) << s;

  auto schema = storage_->GetSchema({"proj-test", "ds", "t"});
  ASSERT_TRUE(schema.ok()) << schema.status();
  ASSERT_EQ(schema->columns.size(), 2u);
  EXPECT_EQ(schema->columns[0].mode, schema::ColumnMode::kRequired);
  EXPECT_EQ(schema->columns[1].mode, schema::ColumnMode::kNullable);
}

TEST_F(ControlOpExecutorTest, CreateTableDuplicateSurfacesAlreadyExists) {
  ASSERT_TRUE(RunDdl("CREATE TABLE ds.t (id INT64)").ok());
  absl::Status second = RunDdl("CREATE TABLE ds.t (id INT64)");
  ASSERT_FALSE(second.ok());
  EXPECT_EQ(second.code(), absl::StatusCode::kAlreadyExists) << second;
}

TEST_F(ControlOpExecutorTest, CreateTableIfNotExistsSwallowsExisting) {
  ASSERT_TRUE(RunDdl("CREATE TABLE ds.t (id INT64)").ok());
  absl::Status second = RunDdl("CREATE TABLE IF NOT EXISTS ds.t (id INT64)");
  EXPECT_TRUE(second.ok()) << second;
}

TEST_F(ControlOpExecutorTest, CreateTableAutoCreatesMissingDataset) {
  absl::Status s = RunDdl("CREATE TABLE fresh_ds.t (id INT64)");
  ASSERT_TRUE(s.ok()) << s;
  auto schema = storage_->GetSchema({"proj-test", "fresh_ds", "t"});
  ASSERT_TRUE(schema.ok()) << schema.status();
}

TEST_F(ControlOpExecutorTest, CreateTableOneSegmentUsesDefaultDataset) {
  CatalogBundle bundle = MakeCatalog();
  ::googlesql::AnalyzerOptions options = MakeAnalyzerOptions();
  ::googlesql::TypeFactory type_factory;
  std::unique_ptr<const ::googlesql::AnalyzerOutput> output;
  const std::string sql = "CREATE TABLE typed (i INT64)";
  ASSERT_TRUE(::googlesql::AnalyzeStatement(
                  sql, options, bundle.catalog.get(), &type_factory, &output)
                  .ok());
  QueryRequest req = MakeRequest(sql);
  req.default_dataset_id = "_default";
  absl::Status s = executor_->ExecuteDdl(
      req, *output->resolved_statement(), bundle.catalog.get());
  ASSERT_TRUE(s.ok()) << s;
  auto schema = storage_->GetSchema({"proj-test", "_default", "typed"});
  ASSERT_TRUE(schema.ok()) << schema.status();
}

TEST_F(ControlOpExecutorTest, CreateOrReplaceTableReplacesExisting) {
  ASSERT_TRUE(RunDdl("CREATE TABLE ds.t (id INT64)").ok());
  absl::Status second =
      RunDdl("CREATE OR REPLACE TABLE ds.t (id INT64, label STRING)");
  ASSERT_TRUE(second.ok()) << second;

  auto schema = storage_->GetSchema({"proj-test", "ds", "t"});
  ASSERT_TRUE(schema.ok()) << schema.status();
  ASSERT_EQ(schema->columns.size(), 2u);
  EXPECT_EQ(schema->columns[1].name, "label");
}

// --- DROP TABLE ----------------------------------------------------------

TEST_F(ControlOpExecutorTest, DropTableRemovesStorageTable) {
  CreatePeopleTable();
  absl::Status s = RunDdl("DROP TABLE ds.people");
  ASSERT_TRUE(s.ok()) << s;
  auto schema = storage_->GetSchema({"proj-test", "ds", "people"});
  ASSERT_FALSE(schema.ok());
  EXPECT_EQ(schema.status().code(), absl::StatusCode::kNotFound)
      << schema.status();
}

TEST_F(ControlOpExecutorTest, DropTableIfExistsSwallowsMissingTable) {
  absl::Status s = RunDdl("DROP TABLE IF EXISTS ds.absent");
  EXPECT_TRUE(s.ok()) << s;
}

TEST_F(ControlOpExecutorTest, DropTableMissingSurfacesNotFound) {
  absl::Status s = RunDdl("DROP TABLE ds.absent");
  ASSERT_FALSE(s.ok());
  EXPECT_EQ(s.code(), absl::StatusCode::kNotFound) << s;
}

TEST_F(ControlOpExecutorTest, DropViewNotImplementedYet) {
  // `CREATE VIEW` and friends are deferred (the storage layer has
  // no view-CRUD surface today). The executor advertises that
  // gap with UNIMPLEMENTED + a message that names this plan so a
  // regression here makes it obvious the view-CRUD work has not
  // landed yet.
  absl::Status s = RunDdl("DROP VIEW ds.absent_view");
  ASSERT_FALSE(s.ok());
  EXPECT_EQ(s.code(), absl::StatusCode::kUnimplemented) << s;
  EXPECT_NE(std::string(s.message()).find("DROP VIEW"), std::string::npos)
      << s.message();
}

// --- CREATE TABLE AS SELECT ----------------------------------------------

TEST_F(ControlOpExecutorTest, CreateTableAsSelectMaterializesSourceRows) {
  CreatePeopleTable();
  absl::Status s =
      RunDdl("CREATE TABLE ds.people_copy AS SELECT id, name FROM ds.people");
  ASSERT_TRUE(s.ok()) << s;

  auto schema = storage_->GetSchema({"proj-test", "ds", "people_copy"});
  ASSERT_TRUE(schema.ok()) << schema.status();
  ASSERT_EQ(schema->columns.size(), 2u);

  auto scan = storage_->ScanRows({"proj-test", "ds", "people_copy"});
  ASSERT_TRUE(scan.ok()) << scan.status();
  std::vector<std::pair<int64_t, std::string>> seen;
  storage::Row row;
  while (true) {
    auto has = (*scan)->Next(&row);
    ASSERT_TRUE(has.ok()) << has.status();
    if (!*has) break;
    ASSERT_EQ(row.cells.size(), 2u);
    seen.emplace_back(row.cells[0].int64_value(), row.cells[1].string_value());
  }
  std::sort(seen.begin(), seen.end());
  std::vector<std::pair<int64_t, std::string>> want = {
      {1, "ada"}, {2, "linus"}, {3, "grace"}};
  EXPECT_EQ(seen, want);
}

// --- ANALYZE -------------------------------------------------------------

TEST_F(ControlOpExecutorTest, AnalyzeKnownTableIsNoOpSuccess) {
  CreatePeopleTable();
  // GoogleSQL's `ANALYZE` form takes a comma-separated table list
  // (no `TABLE` keyword). Today the emulator's storage layer does
  // not keep optimizer statistics, so the handler verifies the
  // table exists and returns OK -- that is what this test pins.
  absl::Status s = RunDdl("ANALYZE ds.people");
  EXPECT_TRUE(s.ok()) << s;
}

TEST_F(ControlOpExecutorTest, AnalyzeWithoutTablesSucceeds) {
  // `ANALYZE` with no target list is a global statistics refresh
  // hint. The emulator does not keep optimizer statistics, so the
  // statement is a no-op success regardless of which dataset is in
  // scope.
  absl::Status s = RunDdl("ANALYZE");
  EXPECT_TRUE(s.ok()) << s;
}

// --- Deferred control-op shapes -----------------------------------------

TEST_F(ControlOpExecutorTest, CreateViewSurfacesUnimplemented) {
  absl::Status s = RunDdl("CREATE VIEW ds.v AS SELECT 1 AS id");
  ASSERT_FALSE(s.ok());
  EXPECT_EQ(s.code(), absl::StatusCode::kUnimplemented) << s;
  EXPECT_NE(std::string(s.message()).find("local-exec-01-ddl-catalog.plan.md"),
            std::string::npos)
      << s.message();
}

TEST_F(ControlOpExecutorTest, CreateMaterializedViewSurfacesUnimplemented) {
  absl::Status s = RunDdl("CREATE MATERIALIZED VIEW ds.mv AS SELECT 1 AS id");
  ASSERT_FALSE(s.ok());
  EXPECT_EQ(s.code(), absl::StatusCode::kUnimplemented) << s;
  EXPECT_NE(std::string(s.message()).find("local-exec-15-specialized-stubs"),
            std::string::npos)
      << s.message();
}

TEST_F(ControlOpExecutorTest, ExportDataSurfacesUnimplemented) {
  CreatePeopleTable();
  absl::Status s = RunDdl(
      "EXPORT DATA OPTIONS(uri='file:///tmp/out.csv', format='CSV') AS "
      "SELECT id, name FROM ds.people");
  ASSERT_FALSE(s.ok());
  EXPECT_EQ(s.code(), absl::StatusCode::kUnimplemented) << s;
  EXPECT_NE(std::string(s.message()).find("EXPORT DATA"), std::string::npos)
      << s.message();
}

// --- Routing-bug defenses ------------------------------------------------

TEST_F(ControlOpExecutorTest, ExecuteQueryRejectsControlOpStatement) {
  // The coordinator never dispatches control-op statements through
  // the row-stream surface. If it did, the executor must surface
  // INVALID_ARGUMENT (not UNIMPLEMENTED) so the routing bug shows
  // up loudly in tests.
  CatalogBundle bundle = MakeCatalog();
  ::googlesql::AnalyzerOptions options = MakeAnalyzerOptions();
  ::googlesql::TypeFactory type_factory;
  std::unique_ptr<const ::googlesql::AnalyzerOutput> output;
  ASSERT_TRUE(::googlesql::AnalyzeStatement("CREATE TABLE ds.t (id INT64)",
                                            options,
                                            bundle.catalog.get(),
                                            &type_factory,
                                            &output)
                  .ok());
  ASSERT_NE(output, nullptr);
  auto source =
      executor_->ExecuteQuery(MakeRequest("CREATE TABLE ds.t (id INT64)"),
                              *output->resolved_statement(),
                              bundle.catalog.get());
  ASSERT_FALSE(source.ok());
  EXPECT_EQ(source.status().code(), absl::StatusCode::kInvalidArgument)
      << source.status();
}

TEST_F(ControlOpExecutorTest, ExecuteDmlRejectsControlOpStatement) {
  CatalogBundle bundle = MakeCatalog();
  ::googlesql::AnalyzerOptions options = MakeAnalyzerOptions();
  ::googlesql::TypeFactory type_factory;
  std::unique_ptr<const ::googlesql::AnalyzerOutput> output;
  ASSERT_TRUE(::googlesql::AnalyzeStatement("CREATE TABLE ds.t (id INT64)",
                                            options,
                                            bundle.catalog.get(),
                                            &type_factory,
                                            &output)
                  .ok());
  ASSERT_NE(output, nullptr);
  auto stats =
      executor_->ExecuteDml(MakeRequest("CREATE TABLE ds.t (id INT64)"),
                            *output->resolved_statement(),
                            bundle.catalog.get());
  ASSERT_FALSE(stats.ok());
  EXPECT_EQ(stats.status().code(), absl::StatusCode::kInvalidArgument)
      << stats.status();
}

}  // namespace
}  // namespace control
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
