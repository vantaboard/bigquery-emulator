// Crash-safety regression tests for client-reachable catalog paths.
//
// R4: Engine abort on duplicate catalog name during view replay (authorize-view
// repeat). A duplicate bare view name across datasets used to abort the engine
// via SimpleCatalog::AddTable during eager view replay. Views now resolve
// lazily through FindProjectView; these tests assert catalog construction and
// registration never abort on adversarial duplicate names or replay cycles.
//
// Plan:
// .cursor/plans/conformance-hardening/07-reported-bug-regression-fixtures.plan.md
// See also: .cursor/plans/conformance-hardening/03-engine-crash-safety.plan.md

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
#include "backend/catalog/udf_registration_catalog.h"
#include "backend/catalog/udf_registry.h"
#include "backend/catalog/view_registry.h"
#include "backend/schema/schema.h"
#include "backend/storage/duckdb/duckdb_storage.h"
#include "googlesql/public/analyzer.h"
#include "googlesql/public/analyzer_options.h"
#include "googlesql/public/analyzer_output.h"
#include "googlesql/public/function.h"
#include "googlesql/public/function_signature.h"
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

const char* kProject = "proj_catalog_crash_safety";

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

std::unique_ptr<const ::googlesql::Function> MakeScalarFn(
    const std::string& name) {
  ::googlesql::FunctionSignature signature(
      ::googlesql::FunctionArgumentType(::googlesql::types::Int64Type()),
      /*arguments=*/{},
      /*context_id=*/static_cast<int64_t>(0));
  return std::make_unique<::googlesql::Function>(
      std::vector<std::string>{name},
      /*group=*/"External_function",
      ::googlesql::Function::SCALAR,
      std::vector<::googlesql::FunctionSignature>{signature});
}

class CatalogCrashSafetyTest : public ::testing::Test {
 protected:
  void SetUp() override {
    const char* tmpdir_env = std::getenv("TMPDIR");
    const std::string tmpdir = tmpdir_env != nullptr ? tmpdir_env : "/tmp";
    std::random_device rd;
    std::seed_seq seed{rd(), rd()};
    std::mt19937_64 rng(seed);
    data_dir_ =
        fs::path(tmpdir) / absl::StrCat("bqemu-catalog-crash-safety-", rng());
    std::error_code ec;
    fs::remove_all(data_dir_, ec);
    auto opened = storage::duckdb::DuckDBStorage::Open(data_dir_.string());
    ASSERT_TRUE(opened.ok()) << opened.status();
    storage_ = std::move(opened).value();
    ASSERT_TRUE(storage_->CreateDataset({kProject, "ds_base"}, "US").ok());
    schema::TableSchema schema;
    schema.columns.push_back({.name = "id",
                              .type = schema::ColumnType::kInt64,
                              .mode = schema::ColumnMode::kRequired});
    ASSERT_TRUE(
        storage_->CreateTable({kProject, "ds_base", "source"}, schema).ok());
  }

  void TearDown() override {
    storage_.reset();
    std::error_code ec;
    fs::remove_all(data_dir_, ec);
  }

  struct CatalogBundle {
    std::unique_ptr<::googlesql::TypeFactory> type_factory{};
    std::unique_ptr<GoogleSqlCatalog> catalog{};
  };

  CatalogBundle MakeCatalog(absl::string_view default_dataset = "") {
    auto type_factory = std::make_unique<::googlesql::TypeFactory>();
    auto catalog = std::make_unique<GoogleSqlCatalog>(kProject,
                                                      storage_.get(),
                                                      type_factory.get(),
                                                      MakeLanguageOptions(),
                                                      default_dataset);
    return {std::move(type_factory), std::move(catalog)};
  }

  absl::Status RegisterViewFromSql(absl::string_view sql) {
    CatalogBundle bundle = MakeCatalog();
    ::googlesql::TypeFactory analyze_tf;
    std::unique_ptr<const ::googlesql::AnalyzerOutput> output;
    absl::Status analyzed = ::googlesql::AnalyzeStatement(
        sql, MakeAnalyzerOptions(), bundle.catalog.get(), &analyze_tf, &output);
    if (!analyzed.ok()) return analyzed;
    const ::googlesql::ResolvedStatement* stmt = output->resolved_statement();
    if (stmt == nullptr) {
      return absl::InternalError("analyzer returned null statement");
    }
    if (stmt->node_kind() != ::googlesql::RESOLVED_CREATE_VIEW_STMT) {
      return absl::InvalidArgumentError("expected CREATE VIEW statement");
    }
    const auto* create_view =
        stmt->GetAs<::googlesql::ResolvedCreateViewStmt>();
    if (create_view == nullptr) {
      return absl::InternalError("CREATE VIEW has null resolved stmt");
    }
    ::googlesql::TypeFactory* reg_tf = EnsureProjectTypeFactory(kProject);
    return RegisterProjectView(kProject,
                               /*default_dataset_id=*/"",
                               *create_view,
                               std::move(output),
                               reg_tf);
  }

  fs::path data_dir_{};
  std::unique_ptr<storage::duckdb::DuckDBStorage> storage_{};
};

TEST_F(CatalogCrashSafetyTest,
       DuplicateViewNameAcrossDatasetsDoesNotAbortCatalogConstruction) {
  ASSERT_TRUE(storage_->CreateDataset({kProject, "ds_a"}, "US").ok());
  ASSERT_TRUE(storage_->CreateDataset({kProject, "ds_b"}, "US").ok());

  ASSERT_TRUE(RegisterViewFromSql(
                  "CREATE VIEW ds_a.profiles AS SELECT id FROM ds_base.source")
                  .ok());
  ASSERT_TRUE(RegisterViewFromSql(
                  "CREATE VIEW ds_b.profiles AS SELECT id FROM ds_base.source")
                  .ok());

  for (int i = 0; i < 5; ++i) {
    CatalogBundle bundle = MakeCatalog();
    const ::googlesql::Table* table_a = nullptr;
    const ::googlesql::Table* table_b = nullptr;
    EXPECT_TRUE(bundle.catalog->FindTable({"ds_a", "profiles"}, &table_a).ok());
    EXPECT_TRUE(bundle.catalog->FindTable({"ds_b", "profiles"}, &table_b).ok());
    ASSERT_NE(table_a, nullptr);
    ASSERT_NE(table_b, nullptr);
    EXPECT_NE(table_a, table_b);
  }
}

TEST_F(CatalogCrashSafetyTest, ReRegisterViewReplacesWithoutAbort) {
  ASSERT_TRUE(storage_->CreateDataset({kProject, "ds_tenant"}, "US").ok());
  const std::string ddl =
      "CREATE VIEW ds_tenant.v AS SELECT id FROM ds_base.source";
  ASSERT_TRUE(RegisterViewFromSql(ddl).ok());
  ASSERT_TRUE(
      RegisterViewFromSql(
          "CREATE OR REPLACE VIEW ds_tenant.v AS SELECT id FROM ds_base.source")
          .ok());

  for (int i = 0; i < 3; ++i) {
    CatalogBundle bundle = MakeCatalog();
    const ::googlesql::Table* view = nullptr;
    EXPECT_TRUE(bundle.catalog->FindTable({"ds_tenant", "v"}, &view).ok());
    ASSERT_NE(view, nullptr);
  }
}

TEST_F(CatalogCrashSafetyTest, ReplayFunctionsTwiceOnSameCatalogDoesNotAbort) {
  const std::string fn_name = "crash_safety_ds.fn";
  ::googlesql::TypeFactory type_factory;
  ::googlesql::SimpleCatalog catalog(kProject, &type_factory);

  ASSERT_TRUE(RegisterProjectFunction(kProject,
                                      /*is_temp=*/false,
                                      /*analyzer_output=*/nullptr,
                                      MakeScalarFn(fn_name))
                  .ok());
  ReplayFunctionsIntoCatalog(kProject, catalog);
  ReplayFunctionsIntoCatalog(kProject, catalog);

  const ::googlesql::Function* fn = nullptr;
  ASSERT_TRUE(catalog.GetFunction(fn_name, &fn).ok());
  ASSERT_NE(fn, nullptr);
}

TEST_F(CatalogCrashSafetyTest,
       RegistrationCatalogSurvivesRepeatedViewAuthorizeCycles) {
  ASSERT_TRUE(storage_->CreateDataset({kProject, "ds_main"}, "US").ok());
  ASSERT_TRUE(storage_->CreateDataset({kProject, "ds_tenant"}, "US").ok());

  ::googlesql::TypeFactory reg_tf;
  const ::googlesql::LanguageOptions language = MakeCatalogLanguageOptions();

  for (int i = 0; i < 5; ++i) {
    ASSERT_TRUE(RegisterViewFromSql(
                    "CREATE OR REPLACE VIEW ds_tenant.v AS SELECT id FROM "
                    "ds_base.source")
                    .ok());
    GoogleSqlCatalog* reg_catalog = GetOrCreateRegistrationCatalog(
        kProject, storage_.get(), &reg_tf, language, "ds_tenant");
    ASSERT_NE(reg_catalog, nullptr);

    CatalogBundle query_catalog = MakeCatalog("ds_tenant");
    const ::googlesql::Table* view = nullptr;
    EXPECT_TRUE(
        query_catalog.catalog->FindTable({"ds_tenant", "v"}, &view).ok());
    ASSERT_NE(view, nullptr);
  }
}

}  // namespace
}  // namespace catalog
}  // namespace backend
}  // namespace bigquery_emulator
