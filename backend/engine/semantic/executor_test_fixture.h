#ifndef BIGQUERY_EMULATOR_BACKEND_ENGINE_SEMANTIC_EXECUTOR_TEST_FIXTURE_H_
#define BIGQUERY_EMULATOR_BACKEND_ENGINE_SEMANTIC_EXECUTOR_TEST_FIXTURE_H_

#include <memory>
#include <string>

#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "backend/engine/engine.h"
#include "backend/engine/semantic/executor.h"
#include "backend/storage/storage.h"
#include "googlesql/public/analyzer.h"
#include "googlesql/public/analyzer_options.h"
#include "googlesql/public/analyzer_output.h"
#include "googlesql/public/builtin_function_options.h"
#include "googlesql/public/catalog.h"
#include "googlesql/public/language_options.h"
#include "googlesql/public/options.pb.h"
#include "googlesql/public/simple_catalog.h"
#include "googlesql/public/types/type_factory.h"
#include "googlesql/resolved_ast/resolved_ast.h"
#include "gtest/gtest.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace semantic {

// NOTE: deliberately NOT in an anonymous namespace. Both
// `executor_test.cc` and `executor_analytic_test.cc` link into the
// same `cc_test` binary and share the `SemanticExecutorTest` suite
// name; an anonymous namespace would give each TU a distinct fixture
// type, which gtest rejects ("must use the same test fixture class").

inline ::googlesql::AnalyzerOptions MakeAnalyzerOptions() {
  ::googlesql::LanguageOptions language;
  language.EnableMaximumLanguageFeatures();
  language.set_product_mode(::googlesql::PRODUCT_EXTERNAL);
  ::googlesql::AnalyzerOptions options(language);
  options.CreateDefaultArenasIfNotSet();
  return options;
}

class SemanticExecutorTest : public ::testing::Test {
 protected:
  void SetUp() override {
    type_factory_ = std::make_unique<::googlesql::TypeFactory>();
    catalog_ = std::make_unique<::googlesql::SimpleCatalog>(
        "exec_catalog", type_factory_.get());
    catalog_->AddBuiltinFunctions(
        ::googlesql::BuiltinFunctionOptions::AllReleasedFunctions());
  }

  const ::googlesql::ResolvedStatement* Analyze(
      absl::string_view sql, const ::googlesql::AnalyzerOptions& options) {
    last_output_.reset();
    absl::Status s = ::googlesql::AnalyzeStatement(
        sql, options, catalog_.get(), type_factory_.get(), &last_output_);
    EXPECT_TRUE(s.ok()) << s;
    if (!s.ok() || last_output_ == nullptr) return nullptr;
    return last_output_->resolved_statement();
  }

  QueryRequest MakeRequest(absl::string_view sql) {
    QueryRequest req;
    req.project_id = "test-project";
    req.sql = std::string(sql);
    return req;
  }

  // Drain a single-row output and return the first cell.
  absl::StatusOr<storage::Value> RunForFirstCell(
      const std::string& sql,
      ::googlesql::AnalyzerOptions options = MakeAnalyzerOptions(),
      QueryRequest req = QueryRequest{}) {
    const auto* stmt = Analyze(sql, options);
    if (stmt == nullptr) return absl::InternalError("analyzer failed");
    if (req.sql.empty()) req = MakeRequest(sql);
    SemanticExecutor exec;
    auto source = exec.ExecuteQuery(req, *stmt, catalog_.get());
    if (!source.ok()) return source.status();
    storage::Row row;
    auto has = (*source)->Next(&row);
    if (!has.ok()) return has.status();
    if (!*has) return absl::InternalError("executor returned no rows");
    if (row.cells.empty()) return absl::InternalError("row has no cells");
    return row.cells[0];
  }

  std::unique_ptr<::googlesql::TypeFactory> type_factory_{};
  std::unique_ptr<::googlesql::SimpleCatalog> catalog_{};
  std::unique_ptr<const ::googlesql::AnalyzerOutput> last_output_{};
};

}  // namespace semantic
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator

#endif  // BIGQUERY_EMULATOR_BACKEND_ENGINE_SEMANTIC_EXECUTOR_TEST_FIXTURE_H_
