// Routing-bug defense tests for the control-op executor. Kept in a
// separate file so `control_op_executor_test.cc` stays under the
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
#include "backend/engine/engine.h"
#include "googlesql/public/analyzer.h"
#include "googlesql/public/analyzer_options.h"
#include "googlesql/public/analyzer_output.h"
#include "googlesql/public/language_options.h"
#include "googlesql/public/options.pb.h"
#include "googlesql/public/types/type_factory.h"
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

class ControlOpRoutingTest : public ::testing::Test {
 protected:
  void SetUp() override {
    const char* tmpdir_env = std::getenv("TMPDIR");
    const std::string tmpdir = tmpdir_env != nullptr ? tmpdir_env : "/tmp";
    std::random_device rd;
    std::seed_seq seed{rd(), rd()};
    std::mt19937_64 rng(seed);
    data_dir_ = fs::path(tmpdir) /
                absl::StrCat("bqemu-control-op-routing-test-", rng());
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

  fs::path data_dir_{};
  std::unique_ptr<storage::duckdb::DuckDBStorage> storage_{};
  std::unique_ptr<ControlOpExecutor> executor_{};
};

TEST_F(ControlOpRoutingTest, ExecuteQueryRejectsControlOpStatement) {
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

TEST_F(ControlOpRoutingTest, ExecuteDmlRejectsControlOpStatement) {
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
