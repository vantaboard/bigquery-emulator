
#include <string>

#include "absl/status/status.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/string_view.h"
#include "googlesql/public/analyzer.h"
#include "googlesql/public/analyzer_options.h"
#include "googlesql/public/analyzer_output.h"
#include "googlesql/public/builtin_function_options.h"
#include "googlesql/public/language_options.h"
#include "googlesql/public/options.pb.h"
#include "googlesql/public/simple_catalog.h"
#include "googlesql/public/type.pb.h"
#include "googlesql/public/types/type_factory.h"
#include "googlesql/public/value.h"
#include "googlesql/resolved_ast/resolved_ast.h"
#include "gtest/gtest.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace semantic {
namespace functions {
namespace {

using ::googlesql::Value;

::googlesql::AnalyzerOptions MakeAnalyzerOptions() {
  ::googlesql::LanguageOptions language;
  language.EnableMaximumLanguageFeatures();
  language.set_product_mode(::googlesql::PRODUCT_EXTERNAL);
  ::googlesql::AnalyzerOptions options(language);
  options.CreateDefaultArenasIfNotSet();
  return options;
}

class RangeFuncsTest : public ::testing::Test {
 protected:
  void SetUp() override {
    type_factory_ = std::make_unique<::googlesql::TypeFactory>();
    catalog_ = std::make_unique<::googlesql::SimpleCatalog>(
        "range_funcs_catalog", type_factory_.get());
    catalog_->AddBuiltinFunctions(
        ::googlesql::BuiltinFunctionOptions::AllReleasedFunctions());
  }

  absl::StatusOr<Value> LiteralValue(absl::string_view literal_sql) {
    last_output_.reset();
    const std::string sql = absl::StrCat("SELECT ", literal_sql);
    absl::Status s = ::googlesql::AnalyzeStatement(sql,
                                                   MakeAnalyzerOptions(),
                                                   catalog_.get(),
                                                   type_factory_.get(),
                                                   &last_output_);
    if (!s.ok()) return s;
    if (last_output_ == nullptr) {
      return absl::InternalError("analyzer returned null output");
    }
    const auto* stmt = last_output_->resolved_statement()
                           ->GetAs<::googlesql::ResolvedQueryStmt>();
    if (stmt == nullptr) {
      return absl::InternalError("expected ResolvedQueryStmt");
    }
    const auto* project =
        stmt->query()->GetAs<::googlesql::ResolvedProjectScan>();
    if (project == nullptr || project->expr_list_size() == 0) {
      return absl::InternalError("expected literal project scan");
    }
    const auto* lit =
        project->expr_list(0)->expr()->GetAs<::googlesql::ResolvedLiteral>();
    if (lit == nullptr) {
      return absl::InternalError("expected ResolvedLiteral");
    }
    return lit->value();
  }

  std::unique_ptr<::googlesql::TypeFactory> type_factory_{};
  std::unique_ptr<::googlesql::SimpleCatalog> catalog_{};
  std::unique_ptr<const ::googlesql::AnalyzerOutput> last_output_{};
};

TEST_F(RangeFuncsTest, RangeOverlapsDetectsOverlap) {
  auto a_or = LiteralValue("RANGE<DATE> '[2001-11-12, 2001-11-14)'");
  auto b_or = LiteralValue("RANGE<DATE> '[2001-11-13, 2001-11-15)'");
  ASSERT_TRUE(a_or.ok()) << a_or.status();
  ASSERT_TRUE(b_or.ok()) << b_or.status();
  Value a = *std::move(a_or);
  Value b = *std::move(b_or);
  auto v = RangeOverlaps({a, b});
  ASSERT_TRUE(v.ok()) << v.status();
  EXPECT_TRUE(v->bool_value());
}

TEST_F(RangeFuncsTest, RangeOverlapsAdjacentRangesDoNotOverlap) {
  auto a_or = LiteralValue("RANGE<DATE> '[2001-11-12, 2001-11-14)'");
  auto b_or = LiteralValue("RANGE<DATE> '[2001-11-14, 2001-11-15)'");
  ASSERT_TRUE(a_or.ok()) << a_or.status();
  ASSERT_TRUE(b_or.ok()) << b_or.status();
  Value a = *std::move(a_or);
  Value b = *std::move(b_or);
  auto v = RangeOverlaps({a, b});
  ASSERT_TRUE(v.ok()) << v.status();
  EXPECT_FALSE(v->bool_value());
}

TEST_F(RangeFuncsTest, RangeStartAndCtorBuildLdiffSlice) {
  auto p1_or = LiteralValue("RANGE<DATE> '[2001-11-12, 2001-11-14)'");
  auto p2_or = LiteralValue("RANGE<DATE> '[2001-11-13, 2001-11-15)'");
  ASSERT_TRUE(p1_or.ok()) << p1_or.status();
  ASSERT_TRUE(p2_or.ok()) << p2_or.status();
  Value p1 = *std::move(p1_or);
  Value p2 = *std::move(p2_or);
  auto start1 = RangeStart({p1});
  auto start2 = RangeStart({p2});
  ASSERT_TRUE(start1.ok()) << start1.status();
  ASSERT_TRUE(start2.ok()) << start2.status();
  auto range = RangeCtor({*start1, *start2}, p1.type());
  ASSERT_TRUE(range.ok()) << range.status();
  EXPECT_EQ(range->type_kind(), ::googlesql::TYPE_RANGE);
  std::string lit = range->GetSQLLiteral();
  for (char& c : lit) {
    if (c == '"') c = '\'';
  }
  EXPECT_EQ(lit, "RANGE<DATE> '[2001-11-12, 2001-11-13)'");
}

}  // namespace
}  // namespace functions
}  // namespace semantic
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
