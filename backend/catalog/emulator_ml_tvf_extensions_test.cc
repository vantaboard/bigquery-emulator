#include "backend/catalog/emulator_ml_tvf_extensions.h"

#include <memory>
#include <string>
#include <vector>

#include "absl/status/status.h"
#include "absl/strings/str_join.h"
#include "backend/catalog/emulator_ml_test_catalog.h"
#include "googlesql/public/analyzer.h"
#include "googlesql/public/analyzer_options.h"
#include "googlesql/public/builtin_function_options.h"
#include "googlesql/public/catalog.h"
#include "googlesql/public/language_options.h"
#include "googlesql/public/options.pb.h"
#include "googlesql/public/types/type_factory.h"
#include "gtest/gtest.h"

namespace bigquery_emulator {
namespace backend {
namespace catalog {
namespace {

class EmulatorMlTvfCatalogTest : public ::testing::Test {
 protected:
  void SetUp() override {
    type_factory_ = std::make_unique<::googlesql::TypeFactory>();
    catalog_ = std::make_unique<EmulatorMlTestCatalog>("test_catalog",
                                                       type_factory_.get());
    ::googlesql::LanguageOptions language;
    language.EnableMaximumLanguageFeaturesForDevelopment();
    language.EnableLanguageFeature(::googlesql::FEATURE_REMOTE_MODEL);
    language.set_product_mode(::googlesql::PRODUCT_EXTERNAL);
    ASSERT_TRUE(catalog_
                    ->AddBuiltinFunctionsAndTypes(
                        ::googlesql::BuiltinFunctionOptions(language))
                    .ok());
    RegisterEmulatorMlTvfStubs(*catalog_);
  }

  std::unique_ptr<::googlesql::TypeFactory> type_factory_{};
  std::unique_ptr<EmulatorMlTestCatalog> catalog_{};
};

TEST_F(EmulatorMlTvfCatalogTest, RegistersLookupPathsForMlPredict) {
  const ::googlesql::TableValuedFunction* tvf = nullptr;
  for (const std::vector<std::string> path :
       {std::vector<std::string>{"ML", "PREDICT"},
        std::vector<std::string>{"ml", "predict"}}) {
    SCOPED_TRACE(path[0] + "." + path[1]);
    absl::Status st = catalog_->FindTableValuedFunction(path, &tvf);
    if (st.ok()) {
      ASSERT_NE(tvf, nullptr);
      EXPECT_EQ(tvf->SQLName(), "ML.PREDICT");
      return;
    }
  }
  FAIL() << "FindTableValuedFunction did not resolve ML.PREDICT";
}

TEST_F(EmulatorMlTvfCatalogTest, AnalyzeMlPredictSucceeds) {
  ::googlesql::LanguageOptions language;
  language.EnableMaximumLanguageFeaturesForDevelopment();
  language.EnableLanguageFeature(::googlesql::FEATURE_REMOTE_MODEL);
  language.set_product_mode(::googlesql::PRODUCT_EXTERNAL);
  language.set_name_resolution_mode(::googlesql::NAME_RESOLUTION_DEFAULT);
  language.SetSupportsAllStatementKinds();
  ::googlesql::AnalyzerOptions options(language);
  options.set_error_message_mode(::googlesql::ERROR_MESSAGE_ONE_LINE);
  options.CreateDefaultArenasIfNotSet();

  std::unique_ptr<const ::googlesql::AnalyzerOutput> output;
  absl::Status st = ::googlesql::AnalyzeStatement(
      "SELECT * FROM ML.PREDICT(MODEL `ds.unregistered_model`, "
      "(SELECT 1.0 AS f1))",
      options,
      catalog_.get(),
      type_factory_.get(),
      &output);
  EXPECT_TRUE(st.ok()) << st;
}

}  // namespace
}  // namespace catalog
}  // namespace backend
}  // namespace bigquery_emulator
