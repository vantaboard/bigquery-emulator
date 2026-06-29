// Deferred control-op shape tests for the executor. Kept in a separate
// file so `control_op_executor_test.cc` stays under the cpp-lint
// file-length cap.

#include <cstdint>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <memory>
#include <random>
#include <string>
#include <system_error>
#include <utility>
#include <vector>

#include "absl/status/status.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/string_view.h"
#include "backend/catalog/googlesql_catalog.h"
#include "backend/engine/control/control_op_executor.h"
#include "backend/engine/control/control_op_internal.h"
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

::googlesql::AnalyzerOptions MakeAnalyzerOptions() {
  ::googlesql::AnalyzerOptions options(MakeLanguageOptions());
  options.set_error_message_mode(::googlesql::ERROR_MESSAGE_ONE_LINE);
  options.set_attach_error_location_payload(true);
  options.CreateDefaultArenasIfNotSet();
  options.mutable_language()->SetSupportsAllStatementKinds();
  return options;
}

class ControlOpExecutorDeferredTest : public ::testing::Test {
 protected:
  void SetUp() override {
    const char* tmpdir_env = std::getenv("TMPDIR");
    const std::string tmpdir = tmpdir_env != nullptr ? tmpdir_env : "/tmp";
    std::random_device rd;
    std::seed_seq seed{rd(), rd()};
    std::mt19937_64 rng(seed);
    data_dir_ = fs::path(tmpdir) /
                absl::StrCat("bqemu-control-op-deferred-test-", rng());
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
          "ControlOpExecutorDeferredTest::RunDdl: analyzer produced no "
          "resolved statement");
    }
    return executor_->ExecuteDdl(
        MakeRequest(sql), *output->resolved_statement(), bundle.catalog.get());
  }

  fs::path data_dir_{};
  std::unique_ptr<storage::duckdb::DuckDBStorage> storage_{};
  std::unique_ptr<ControlOpExecutor> executor_{};
};

TEST_F(ControlOpExecutorDeferredTest, CreateViewRegisteredByCoordinator) {
  absl::Status s = RunDdl("CREATE VIEW ds.v AS SELECT 1 AS id");
  EXPECT_TRUE(s.ok()) << s;
}

TEST_F(ControlOpExecutorDeferredTest, CreateMaterializedViewMaterializesRows) {
  CreatePeopleTable();
  absl::Status s = RunDdl(
      "CREATE MATERIALIZED VIEW ds.mv AS SELECT id, name FROM ds.people");
  ASSERT_TRUE(s.ok()) << s;
  auto scan = storage_->ScanRows({"proj-test", "ds", "mv"});
  ASSERT_TRUE(scan.ok()) << scan.status();
  int rows = 0;
  storage::Row row;
  while (true) {
    auto has = (*scan)->Next(&row);
    ASSERT_TRUE(has.ok()) << has.status();
    if (!*has) break;
    ASSERT_EQ(row.cells.size(), 2u);
    ++rows;
  }
  EXPECT_EQ(rows, 3);
}

TEST_F(ControlOpExecutorDeferredTest, ExportDataWritesLocalCsv) {
  CreatePeopleTable();
  const std::string path = absl::StrCat(std::getenv("TEST_TMPDIR") != nullptr
                                            ? std::getenv("TEST_TMPDIR")
                                            : "/tmp",
                                        "/bqemu_export_test.csv");
  absl::Status s =
      RunDdl(absl::StrCat("EXPORT DATA OPTIONS(uri='file://",
                          path,
                          "', format='CSV') AS ",
                          "SELECT id, name FROM ds.people ORDER BY id"));
  ASSERT_TRUE(s.ok()) << s;
  std::ifstream in(path);
  ASSERT_TRUE(in.good()) << "expected export file at " << path;
  std::string line;
  ASSERT_TRUE(std::getline(in, line));
  EXPECT_EQ(line, "id,name");
}

TEST_F(ControlOpExecutorDeferredTest, TruncateTableClearsRows) {
  CreatePeopleTable();
  CatalogBundle bundle = MakeCatalog();
  ::googlesql::AnalyzerOptions options = MakeAnalyzerOptions();
  ::googlesql::TypeFactory type_factory;
  std::unique_ptr<const ::googlesql::AnalyzerOutput> output;
  absl::Status analyze =
      ::googlesql::AnalyzeStatement("TRUNCATE TABLE ds.people",
                                    options,
                                    bundle.catalog.get(),
                                    &type_factory,
                                    &output);
  ASSERT_TRUE(analyze.ok()) << analyze;
  ASSERT_NE(output, nullptr);
  ASSERT_NE(output->resolved_statement(), nullptr);
  const auto* truncate =
      output->resolved_statement()->GetAs<::googlesql::ResolvedTruncateStmt>();
  ASSERT_NE(truncate, nullptr);
  absl::StatusOr<int64_t> deleted =
      internal::RunTruncateTable(*storage_, truncate);
  ASSERT_TRUE(deleted.ok()) << deleted.status();
  EXPECT_EQ(*deleted, 3);
  absl::StatusOr<std::int64_t> after =
      storage_->CountRows({"proj-test", "ds", "people"});
  ASSERT_TRUE(after.ok()) << after.status();
  EXPECT_EQ(*after, 0);
  auto schema = storage_->GetSchema({"proj-test", "ds", "people"});
  ASSERT_TRUE(schema.ok()) << schema.status();
  EXPECT_EQ(schema->columns.size(), 2u);
}

}  // namespace
}  // namespace control
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
