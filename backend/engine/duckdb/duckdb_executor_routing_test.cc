// Routing-bug defense tests for the DuckDB executor. Kept in a
// separate file so `duckdb_executor_test.cc` stays under the
// cpp-lint file-length cap.

#include <cstdlib>
#include <filesystem>
#include <memory>
#include <random>
#include <string>
#include <system_error>
#include <utility>

#include "absl/status/status.h"
#include "absl/strings/str_cat.h"
#include "backend/catalog/googlesql_catalog.h"
#include "backend/engine/duckdb/duckdb_executor.h"
#include "backend/engine/engine.h"
#include "backend/storage/duckdb/duckdb_storage.h"
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

class DuckDbExecutorRoutingTest : public ::testing::Test {
 protected:
  void SetUp() override {
    const char* tmpdir_env = std::getenv("TMPDIR");
    const std::string tmpdir = tmpdir_env != nullptr ? tmpdir_env : "/tmp";
    std::random_device rd;
    std::seed_seq seed{rd(), rd()};
    std::mt19937_64 rng(seed);
    data_dir_ =
        fs::path(tmpdir) / absl::StrCat("bqemu-duckdb-routing-test-", rng());
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

TEST_F(DuckDbExecutorRoutingTest,
       ExecuteDdlRejectsCreateTableAfterControlOpMigration) {
  CatalogBundle bundle = MakeCatalog();
  auto analyzed = Analyze("CREATE TABLE ds.t (id INT64, name STRING)",
                          bundle.catalog.get(),
                          /*all_statements=*/true);
  ASSERT_TRUE(analyzed.ok()) << analyzed.status();
  const ::googlesql::ResolvedStatement* stmt =
      (*analyzed)->resolved_statement();
  ASSERT_NE(stmt, nullptr);

  absl::Status s = executor_->ExecuteDdl(
      MakeRequest("CREATE TABLE ds.t (id INT64, name STRING)"),
      *stmt,
      bundle.catalog.get());
  ASSERT_FALSE(s.ok());
  EXPECT_EQ(s.code(), absl::StatusCode::kUnimplemented) << s;
  EXPECT_NE(std::string(s.message()).find("ControlOpExecutor"),
            std::string::npos)
      << s.message();
}

}  // namespace
}  // namespace duckdb
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
