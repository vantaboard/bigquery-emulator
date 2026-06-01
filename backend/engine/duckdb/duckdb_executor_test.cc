// Unit tests for the route-typed DuckDB executor. These exercise
// the contract the future `LocalCoordinatorEngine` relies on: each
// method takes an already-analyzed `ResolvedStatement` and produces
// the same wire-facing reply the `DuckDBEngine` path used to. We
// rebuild the analyzer up-front (the way the coordinator will) and
// hand the resolved root straight to the executor, so the executor's
// pre-resolution validation, transpiler invocation, DuckDB
// per-query connection, and Arrow result-row path are all on the
// critical path.

#include "backend/engine/duckdb/duckdb_executor.h"

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
namespace duckdb {
namespace {

namespace fs = std::filesystem;

::googlesql::LanguageOptions MakeLanguageOptions() {
  ::googlesql::LanguageOptions language;
  language.EnableMaximumLanguageFeatures();
  language.set_product_mode(::googlesql::PRODUCT_EXTERNAL);
  language.set_name_resolution_mode(::googlesql::NAME_RESOLUTION_DEFAULT);
  return language;
}

// Mirrors `DuckDBEngine`'s internal analyzer setup (with the
// supports-all-statements allowlist flipped on for DML / DDL).
::googlesql::AnalyzerOptions MakeAnalyzerOptions(bool all_statements) {
  ::googlesql::AnalyzerOptions options(MakeLanguageOptions());
  options.set_error_message_mode(::googlesql::ERROR_MESSAGE_ONE_LINE);
  options.set_attach_error_location_payload(true);
  options.CreateDefaultArenasIfNotSet();
  if (all_statements) {
    options.mutable_language()->SetSupportsAllStatementKinds();
  }
  return options;
}

class DuckDbExecutorTest : public ::testing::Test {
 protected:
  void SetUp() override {
    const char* tmpdir_env = std::getenv("TMPDIR");
    const std::string tmpdir = tmpdir_env != nullptr ? tmpdir_env : "/tmp";
    std::random_device rd;
    std::seed_seq seed{rd(), rd()};
    std::mt19937_64 rng(seed);
    data_dir_ =
        fs::path(tmpdir) / absl::StrCat("bqemu-duckdb-executor-test-", rng());
    std::error_code ec;
    fs::remove_all(data_dir_, ec);
    auto opened = storage::duckdb::DuckDBStorage::Open(data_dir_.string());
    ASSERT_TRUE(opened.ok()) << opened.status();
    storage_ = std::move(opened).value();
    executor_ = std::make_unique<DuckDbExecutor>(storage_.get());
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
  // NULLABLE). Matches the canonical fixture used by the engine
  // tests so the executor's wire output stays aligned with the
  // engine path's.
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
    ASSERT_TRUE(storage_->CreateDataset({"proj-test", "ds"}, "US").ok());
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

  // Analyze `sql` against the fixture catalog. The returned
  // `AnalyzerOutput` owns the resolved AST -- callers must keep it
  // alive for the duration of the executor call.
  absl::StatusOr<std::unique_ptr<const ::googlesql::AnalyzerOutput>> Analyze(
      absl::string_view sql,
      ::googlesql::Catalog* catalog,
      bool all_statements) {
    ::googlesql::AnalyzerOptions options = MakeAnalyzerOptions(all_statements);
    ::googlesql::TypeFactory type_factory;
    std::unique_ptr<const ::googlesql::AnalyzerOutput> output;
    absl::Status s = ::googlesql::AnalyzeStatement(
        sql, options, catalog, &type_factory, &output);
    if (!s.ok()) return s;
    return output;
  }

  fs::path data_dir_{};
  std::unique_ptr<storage::duckdb::DuckDBStorage> storage_{};
  std::unique_ptr<DuckDbExecutor> executor_{};
};

TEST_F(DuckDbExecutorTest, ExecuteQuerySelectStarFromTableStreamsAllRows) {
  // The executor's smoke test: hand it a fully-analyzed
  // `SELECT * FROM ds.people` and confirm the wire output matches
  // the canonical three rows we seeded into storage. Covers the
  // analyze-then-execute split that the coordinator will rely on.
  CreatePeopleTable();
  CatalogBundle bundle = MakeCatalog();
  auto analyzed = Analyze(
      "SELECT * FROM ds.people", bundle.catalog.get(), /*all_statements=*/false);
  ASSERT_TRUE(analyzed.ok()) << analyzed.status();
  const ::googlesql::ResolvedStatement* stmt = (*analyzed)->resolved_statement();
  ASSERT_NE(stmt, nullptr);

  absl::StatusOr<std::unique_ptr<RowSource>> source = executor_->ExecuteQuery(
      MakeRequest("SELECT * FROM ds.people"), *stmt, bundle.catalog.get());
  ASSERT_TRUE(source.ok()) << source.status();

  const schema::TableSchema& s = (*source)->schema();
  ASSERT_EQ(s.columns.size(), 2u);
  EXPECT_EQ(s.columns[0].name, "id");
  EXPECT_EQ(s.columns[0].type, schema::ColumnType::kInt64);
  EXPECT_EQ(s.columns[1].name, "name");
  EXPECT_EQ(s.columns[1].type, schema::ColumnType::kString);

  // DuckDB does not promise a stable row order without ORDER BY.
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
  std::vector<std::pair<int64_t, std::string>> want = {
      {1, "ada"}, {2, "linus"}, {3, "grace"}};
  std::sort(seen.begin(), seen.end());
  std::sort(want.begin(), want.end());
  EXPECT_EQ(seen, want);
}

TEST_F(DuckDbExecutorTest, ExecuteQueryRejectsNonQueryStatement) {
  // The coordinator is supposed to dispatch DDL through the control-op
  // route, not through the DuckDB executor; defensively the executor
  // returns INVALID_ARGUMENT (not UNIMPLEMENTED) when fed a non-query
  // statement on its query surface so a routing bug surfaces loudly
  // instead of looking like a transpiler gap.
  CatalogBundle bundle = MakeCatalog();
  auto analyzed = Analyze(
      "CREATE TABLE ds.t (id INT64)",
      bundle.catalog.get(),
      /*all_statements=*/true);
  ASSERT_TRUE(analyzed.ok()) << analyzed.status();
  const ::googlesql::ResolvedStatement* stmt = (*analyzed)->resolved_statement();
  ASSERT_NE(stmt, nullptr);

  auto source = executor_->ExecuteQuery(
      MakeRequest("CREATE TABLE ds.t (id INT64)"), *stmt, bundle.catalog.get());
  ASSERT_FALSE(source.ok());
  EXPECT_EQ(source.status().code(), absl::StatusCode::kInvalidArgument)
      << source.status();
}

TEST_F(DuckDbExecutorTest, ExecuteDdlCreateTableCreatesStorageTable) {
  // Round-trip CREATE TABLE through the executor and confirm the
  // table appears in storage with the requested schema. This pins
  // the contract the coordinator's control-op route will rely on
  // until the dedicated `ControlOpExecutor` is built out.
  ASSERT_TRUE(storage_->CreateDataset({"proj-test", "ds"}, "US").ok());
  CatalogBundle bundle = MakeCatalog();
  auto analyzed = Analyze(
      "CREATE TABLE ds.t (id INT64, name STRING)",
      bundle.catalog.get(),
      /*all_statements=*/true);
  ASSERT_TRUE(analyzed.ok()) << analyzed.status();
  const ::googlesql::ResolvedStatement* stmt = (*analyzed)->resolved_statement();
  ASSERT_NE(stmt, nullptr);

  absl::Status s = executor_->ExecuteDdl(
      MakeRequest("CREATE TABLE ds.t (id INT64, name STRING)"),
      *stmt,
      bundle.catalog.get());
  ASSERT_TRUE(s.ok()) << s;

  auto schema = storage_->GetSchema({"proj-test", "ds", "t"});
  ASSERT_TRUE(schema.ok()) << schema.status();
  ASSERT_EQ(schema->columns.size(), 2u);
  EXPECT_EQ(schema->columns[0].name, "id");
  EXPECT_EQ(schema->columns[0].type, schema::ColumnType::kInt64);
  EXPECT_EQ(schema->columns[1].name, "name");
  EXPECT_EQ(schema->columns[1].type, schema::ColumnType::kString);
}

}  // namespace
}  // namespace duckdb
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
