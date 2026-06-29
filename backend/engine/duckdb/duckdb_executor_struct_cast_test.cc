// Positional STRUCT cast regression tests for the DuckDB executor.
// Kept in a separate file so `duckdb_executor_test.cc` stays under the
// cpp-lint file-length cap.

#include <cstdlib>
#include <filesystem>
#include <random>
#include <string>
#include <system_error>
#include <utility>

#include "absl/status/status.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/string_view.h"
#include "backend/engine/duckdb/duckdb_executor.h"
#include "backend/engine/engine.h"
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

class DuckDbExecutorStructCastTest : public ::testing::Test {
 protected:
  void SetUp() override {
    const char* tmpdir_env = std::getenv("TMPDIR");
    const std::string tmpdir = tmpdir_env != nullptr ? tmpdir_env : "/tmp";
    std::random_device rd;
    std::seed_seq seed{rd(), rd()};
    std::mt19937_64 rng(seed);
    data_dir_ = fs::path(tmpdir) /
                absl::StrCat("bqemu-duckdb-struct-cast-test-", rng());
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

TEST_F(DuckDbExecutorStructCastTest, ExecuteUnnestArrayStructPositionalCast) {
  static constexpr char kSql[] = R"sql(
SELECT t.x, t.y
  FROM UNNEST(ARRAY<STRUCT<x INT64, y STRING>>[STRUCT(1, 'a'), STRUCT(2, 'b')]) AS t
)sql";
  const std::string sql(kSql);
  CatalogBundle bundle = MakeCatalog();
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
  ASSERT_EQ(row.cells.size(), 2u);
  EXPECT_EQ(row.cells[0].int64_value(), 1);
  EXPECT_EQ(row.cells[1].string_value(), "a");

  has = (*source)->Next(&row);
  ASSERT_TRUE(has.ok()) << has.status();
  ASSERT_TRUE(*has);
  EXPECT_EQ(row.cells[0].int64_value(), 2);
  EXPECT_EQ(row.cells[1].string_value(), "b");

  has = (*source)->Next(&row);
  ASSERT_TRUE(has.ok()) << has.status();
  EXPECT_FALSE(*has);
}

}  // namespace
}  // namespace duckdb
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
