

#include "absl/status/status.h"
#include "absl/strings/string_view.h"
#include "backend/engine/semantic/eval_context.h"
#include "googlesql/public/analyzer.h"
#include "googlesql/public/analyzer_options.h"
#include "googlesql/public/analyzer_output.h"
#include "googlesql/public/builtin_function_options.h"
#include "googlesql/public/catalog.h"
#include "googlesql/public/language_options.h"
#include "googlesql/public/options.pb.h"
#include "googlesql/public/types/type_factory.h"
#include "googlesql/resolved_ast/resolved_ast.h"
#include "googlesql/resolved_ast/resolved_node_kind.pb.h"
#include "gtest/gtest.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace semantic {
namespace stubs {
namespace {

void ExpectFloatingPointNear(const Value& value, double expected) {
  ASSERT_FALSE(value.is_null());
  if (value.type()->IsDouble()) {
    EXPECT_DOUBLE_EQ(value.double_value(), expected);
  } else {
    EXPECT_FLOAT_EQ(value.float_value(), static_cast<float>(expected));
  }
}

class TvfScanFinder : public ::googlesql::ResolvedASTVisitor {
 public:
  const ::googlesql::ResolvedTVFScan* found = nullptr;

  absl::Status VisitResolvedTVFScan(
      const ::googlesql::ResolvedTVFScan* node) override {
    found = node;
    return absl::OkStatus();
  }

  absl::Status DefaultVisit(const ::googlesql::ResolvedNode* node) override {
    return ::googlesql::ResolvedASTVisitor::DefaultVisit(node);
  }
};

::googlesql::AnalyzerOptions MakeAnalyzerOptions() {
  ::googlesql::LanguageOptions language;
  language.EnableMaximumLanguageFeaturesForDevelopment();
  language.EnableLanguageFeature(::googlesql::FEATURE_REMOTE_MODEL);
  language.set_product_mode(::googlesql::PRODUCT_EXTERNAL);
  language.set_name_resolution_mode(::googlesql::NAME_RESOLUTION_DEFAULT);
  language.SetSupportsAllStatementKinds();
  ::googlesql::AnalyzerOptions options(language);
  options.set_error_message_mode(::googlesql::ERROR_MESSAGE_ONE_LINE);
  options.disable_rewrite(::googlesql::REWRITE_PIVOT);
  options.disable_rewrite(::googlesql::REWRITE_UNPIVOT);
  options.CreateDefaultArenasIfNotSet();
  return options;
}

class MlStubTest : public ::testing::Test {
 protected:
  void SetUp() override {
    type_factory_ = std::make_unique<::googlesql::TypeFactory>();
    catalog_ = std::make_unique<catalog::EmulatorMlTestCatalog>(
        "test_catalog", type_factory_.get());
    ::googlesql::LanguageOptions language;
    language.EnableMaximumLanguageFeaturesForDevelopment();
    language.EnableLanguageFeature(::googlesql::FEATURE_REMOTE_MODEL);
    language.set_product_mode(::googlesql::PRODUCT_EXTERNAL);
    ASSERT_TRUE(catalog_
                    ->AddBuiltinFunctionsAndTypes(
                        ::googlesql::BuiltinFunctionOptions(language))
                    .ok());
    catalog::RegisterEmulatorMlTvfStubs(*catalog_);
  }

  const ::googlesql::ResolvedTVFScan* AnalyzeTvfScan(absl::string_view sql) {
    last_output_.reset();
    ::googlesql::AnalyzerOptions options = MakeAnalyzerOptions();
    absl::Status s = ::googlesql::AnalyzeStatement(
        sql, options, catalog_.get(), type_factory_.get(), &last_output_);
    EXPECT_TRUE(s.ok()) << s;
    if (!s.ok() || last_output_ == nullptr) {
      return nullptr;
    }
    const ::googlesql::ResolvedStatement* stmt = nullptr;
    stmt = last_output_->resolved_statement();
    if (stmt == nullptr ||
        stmt->node_kind() != ::googlesql::RESOLVED_QUERY_STMT) {
      return nullptr;
    }
    const auto* query = stmt->GetAs<::googlesql::ResolvedQueryStmt>();
    TvfScanFinder finder;
    absl::Status walk = query->query()->Accept(&finder);
    EXPECT_TRUE(walk.ok()) << walk;
    return finder.found;
  }

  std::unique_ptr<::googlesql::TypeFactory> type_factory_{};
  std::unique_ptr<catalog::EmulatorMlTestCatalog> catalog_{};
  std::unique_ptr<const ::googlesql::AnalyzerOutput> last_output_{};
};

TEST_F(MlStubTest, PredictPassesThroughInputAndNullPredictedLabel) {
  const ::googlesql::ResolvedTVFScan* tvf = AnalyzeTvfScan(
      "SELECT f1, label, predicted_label FROM ML.PREDICT("
      "MODEL `ds.unregistered_model`, "
      "(SELECT 2.0 AS f1, 3.0 AS label))");
  ASSERT_NE(tvf, nullptr);

  const ::googlesql::ResolvedScan* input_scan = nullptr;
  for (int i = 0; i < tvf->argument_list_size(); ++i) {
    const auto* arg = tvf->argument_list(i);
    if (arg != nullptr && arg->scan() != nullptr) {
      input_scan = arg->scan();
      break;
    }
  }
  ASSERT_NE(input_scan, nullptr);
  ASSERT_GE(input_scan->column_list_size(), 2);

  ColumnBindings input_row;
  const ::googlesql::ResolvedColumn& f1_col = input_scan->column_list(0);
  const ::googlesql::ResolvedColumn& label_col = input_scan->column_list(1);
  input_row.emplace(
      f1_col.column_id(),
      f1_col.type()->IsDouble() ? Value::Double(2.0) : Value::Float(2.0f));
  input_row.emplace(
      label_col.column_id(),
      label_col.type()->IsDouble() ? Value::Double(3.0) : Value::Float(3.0f));

  auto rows = MlPredictStub(*tvf, {input_row}, input_scan);
  ASSERT_TRUE(rows.ok()) << rows.status();
  ASSERT_EQ(rows->size(), 1u);
  const ColumnBindings& out = (*rows)[0];
  ASSERT_EQ(out.size(), 3u);
  bool saw_f1 = false;
  bool saw_label = false;
  bool saw_predicted = false;
  for (int i = 0; i < tvf->column_list_size(); ++i) {
    const ::googlesql::ResolvedColumn& col = tvf->column_list(i);
    const auto it = out.find(col.column_id());
    ASSERT_NE(it, out.end());
    if (col.name() == "f1") {
      saw_f1 = true;
      ExpectFloatingPointNear(it->second, 2.0);
    } else if (col.name() == "label") {
      saw_label = true;
      ExpectFloatingPointNear(it->second, 3.0);
    } else if (col.name() == "predicted_label") {
      saw_predicted = true;
      EXPECT_TRUE(it->second.is_null());
      EXPECT_TRUE(it->second.type()->IsFloatingPoint());
    }
  }
  EXPECT_TRUE(saw_f1);
  EXPECT_TRUE(saw_label);
  EXPECT_TRUE(saw_predicted);
}

TEST_F(MlStubTest, EvaluateReturnsSingleNullMetricsRow) {
  const ::googlesql::ResolvedTVFScan* tvf = AnalyzeTvfScan(
      "SELECT * FROM ML.EVALUATE(MODEL `ds.unregistered_model`)");
  ASSERT_NE(tvf, nullptr);

  auto rows = MlEvaluateStub(*tvf);
  ASSERT_TRUE(rows.ok()) << rows.status();
  ASSERT_EQ(rows->size(), 1u);
  for (int i = 0; i < tvf->column_list_size(); ++i) {
    const ::googlesql::ResolvedColumn& col = tvf->column_list(i);
    const auto it = (*rows)[0].find(col.column_id());
    ASSERT_NE(it, (*rows)[0].end());
    EXPECT_TRUE(it->second.is_null()) << "column " << col.name();
  }
}

TEST_F(MlStubTest, ForecastReturnsSingleNullForecastRow) {
  const ::googlesql::ResolvedTVFScan* tvf = AnalyzeTvfScan(
      "SELECT * FROM ML.FORECAST(MODEL `ds.unregistered_model`, "
      "STRUCT(7 AS horizon))");
  ASSERT_NE(tvf, nullptr);

  auto rows = MlForecastStub(*tvf);
  ASSERT_TRUE(rows.ok()) << rows.status();
  ASSERT_EQ(rows->size(), 1u);
  for (int i = 0; i < tvf->column_list_size(); ++i) {
    const ::googlesql::ResolvedColumn& col = tvf->column_list(i);
    const auto it = (*rows)[0].find(col.column_id());
    ASSERT_NE(it, (*rows)[0].end());
    EXPECT_TRUE(it->second.is_null()) << "column " << col.name();
  }
}

TEST_F(MlStubTest, MaterializeTvfScanPredictEndToEnd) {
  const ::googlesql::ResolvedTVFScan* tvf = AnalyzeTvfScan(
      "SELECT f1, predicted_label FROM ML.PREDICT("
      "MODEL `ds.unregistered_model`, (SELECT 4.0 AS f1))");
  ASSERT_NE(tvf, nullptr);

  EvalContext ctx;
  auto rows = MaterializeTvfScan(*tvf, ctx);
  ASSERT_TRUE(rows.ok()) << rows.status();
  ASSERT_EQ(rows->size(), 1u);
  bool saw_f1 = false;
  bool saw_predicted = false;
  for (int i = 0; i < tvf->column_list_size(); ++i) {
    const ::googlesql::ResolvedColumn& col = tvf->column_list(i);
    const auto it = (*rows)[0].find(col.column_id());
    ASSERT_NE(it, (*rows)[0].end());
    if (col.name() == "f1") {
      saw_f1 = true;
      ExpectFloatingPointNear(it->second, 4.0);
    } else if (col.name() == "predicted_label") {
      saw_predicted = true;
      EXPECT_TRUE(it->second.is_null());
    }
  }
  EXPECT_TRUE(saw_f1);
  EXPECT_TRUE(saw_predicted);
}

}  // namespace
}  // namespace stubs
}  // namespace semantic
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
