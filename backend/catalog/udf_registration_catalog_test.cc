// Regression: cached registration catalogs must replay procedures registered
// after the catalog was first constructed (CREATE PROCEDURE in setup, CALL in
// a later script). See udf_registration_catalog.cc reuse branch.

#include "backend/catalog/udf_registration_catalog.h"

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
#include "backend/catalog/procedure_registry.h"
#include "backend/catalog/stored_procedure.h"
#include "backend/catalog/udf_registry.h"
#include "backend/schema/schema.h"
#include "backend/storage/duckdb/duckdb_storage.h"
#include "googlesql/public/analyzer.h"
#include "googlesql/public/analyzer_options.h"
#include "googlesql/public/analyzer_output.h"
#include "googlesql/public/language_options.h"
#include "googlesql/public/options.pb.h"
#include "googlesql/public/simple_catalog.h"
#include "googlesql/public/types/type_factory.h"
#include "googlesql/resolved_ast/resolved_ast.h"
#include "googlesql/resolved_ast/resolved_node_kind.pb.h"
#include "gtest/gtest.h"

namespace bigquery_emulator {
namespace backend {
namespace catalog {
namespace {

namespace fs = std::filesystem;

const char* kProject = "proj_udf_registration_catalog_test";

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
  options.CreateDefaultArenasIfNotSet();
  options.mutable_language()->SetSupportsAllStatementKinds();
  return options;
}

class UdfRegistrationCatalogTest : public ::testing::Test {
 protected:
  void SetUp() override {
    const char* tmpdir_env = std::getenv("TMPDIR");
    const std::string tmpdir = tmpdir_env != nullptr ? tmpdir_env : "/tmp";
    std::random_device rd;
    std::seed_seq seed{rd(), rd()};
    std::mt19937_64 rng(seed);
    data_dir_ =
        fs::path(tmpdir) / absl::StrCat("bqemu-udf-reg-catalog-test-", rng());
    std::error_code ec;
    fs::remove_all(data_dir_, ec);
    auto opened = storage::duckdb::DuckDBStorage::Open(data_dir_.string());
    ASSERT_TRUE(opened.ok()) << opened.status();
    storage_ = std::move(opened).value();
    ASSERT_TRUE(storage_->CreateDataset({kProject, "ds"}, "US").ok());
  }

  void TearDown() override {
    storage_.reset();
    std::error_code ec;
    fs::remove_all(data_dir_, ec);
  }

  absl::Status RegisterProcedureFromSql(absl::string_view sql) {
    ::googlesql::TypeFactory analyze_tf;
    std::unique_ptr<GoogleSqlCatalog> catalog =
        std::make_unique<GoogleSqlCatalog>(
            kProject, storage_.get(), &analyze_tf, MakeLanguageOptions(), "ds");
    std::unique_ptr<const ::googlesql::AnalyzerOutput> output;
    absl::Status analyzed = ::googlesql::AnalyzeStatement(
        sql, MakeAnalyzerOptions(), catalog.get(), &analyze_tf, &output);
    if (!analyzed.ok()) return analyzed;
    const ::googlesql::ResolvedStatement* stmt = output->resolved_statement();
    if (stmt == nullptr) {
      return absl::InternalError("analyzer returned null statement");
    }
    if (stmt->node_kind() != ::googlesql::RESOLVED_CREATE_PROCEDURE_STMT) {
      return absl::InvalidArgumentError("expected CREATE PROCEDURE statement");
    }
    const auto* create_procedure =
        stmt->GetAs<::googlesql::ResolvedCreateProcedureStmt>();
    if (create_procedure == nullptr) {
      return absl::InternalError("CREATE PROCEDURE has null resolved stmt");
    }
    return RegisterProjectProcedure(
        kProject, *create_procedure, std::move(output));
  }

  fs::path data_dir_{};
  std::unique_ptr<storage::duckdb::DuckDBStorage> storage_{};
};

TEST_F(UdfRegistrationCatalogTest,
       ReusedRegistrationCatalogReplaysProceduresRegisteredAfterCreation) {
  ::googlesql::TypeFactory reg_tf;
  const ::googlesql::LanguageOptions language = MakeLanguageOptions();

  GoogleSqlCatalog* first = nullptr;
  first = GetOrCreateRegistrationCatalog(
      kProject, storage_.get(), &reg_tf, language, "ds");
  ASSERT_NE(first, nullptr);

  const ::googlesql::Procedure* before = nullptr;
  EXPECT_FALSE(first->FindProcedure({"ds", "echo"}, &before).ok());

  ASSERT_TRUE(RegisterProcedureFromSql(
                  "CREATE OR REPLACE PROCEDURE ds.echo(OUT arr ARRAY<INT64>) "
                  "BEGIN SET arr = GENERATE_ARRAY(1, 3); END;")
                  .ok());

  const StoredSQLProcedure* in_registry =
      FindProjectProcedure(kProject, "echo");
  ASSERT_NE(in_registry, nullptr)
      << "procedure must be in project registry after RegisterProjectProcedure";

  GoogleSqlCatalog* second = nullptr;
  second = GetOrCreateRegistrationCatalog(
      kProject, storage_.get(), &reg_tf, language, "ds");
  ASSERT_NE(second, nullptr);
  EXPECT_EQ(second, first);

  const ::googlesql::Procedure* after = nullptr;
  ASSERT_TRUE(second->FindProcedure({"ds", "echo"}, &after).ok())
      << "reused registration catalog must replay procedures registered "
         "after first construction";
  ASSERT_NE(after, nullptr);
  EXPECT_EQ(after, in_registry);
  EXPECT_EQ(after->Name(), "echo");
}

}  // namespace
}  // namespace catalog
}  // namespace backend
}  // namespace bigquery_emulator
