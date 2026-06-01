// Unit tests for `RouteClassifier::Classify`.
//
// We drive a real `AnalyzeStatement` against a tiny
// `SimpleCatalog` (mirroring `transpiler_test.cc`'s pattern) so the
// `ResolvedStatement` the classifier sees is the same shape the
// engine sees at runtime. The catalog is the analyzer's builtin
// function set plus one toy `people` table; that is enough to cover
// every shape the classifier branches on without dragging the full
// production catalog (`backend/catalog/googlesql_catalog.h`) into
// the test link line.
//
// The test cases line up with the plan's "Tests" section
// (`.cursor/plans/engine-router-foundation.plan.md`):
//
//   * pure `duckdb_native` SELECT -> DuckDB route.
//   * SELECT containing a `semantic_executor` function -> semantic
//     route with the offending function recorded.
//   * DDL root -> control-op route.
//   * `unsupported` function in a SELECT -> unsupported route with
//     the function name in the reason.

#include "backend/engine/coordinator/route_classifier.h"

#include <memory>
#include <string>
#include <vector>

#include "absl/status/status.h"
#include "absl/strings/string_view.h"
#include "backend/engine/disposition.h"
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
namespace coordinator {
namespace {

// Mirrors `duckdb_engine::MakeAnalyzerOptions` plus
// `SetSupportsAllStatementKinds` so the DDL tests can also
// analyze through this fixture. Drifting these two settings from
// the production engine's analyzer breaks function dispatch in
// subtle ways that only surface in the conformance harness.
::googlesql::AnalyzerOptions MakeAnalyzerOptions() {
  ::googlesql::LanguageOptions language;
  language.EnableMaximumLanguageFeatures();
  language.set_product_mode(::googlesql::PRODUCT_EXTERNAL);
  language.set_name_resolution_mode(::googlesql::NAME_RESOLUTION_DEFAULT);
  language.SetSupportsAllStatementKinds();
  ::googlesql::AnalyzerOptions options(language);
  options.set_error_message_mode(::googlesql::ERROR_MESSAGE_ONE_LINE);
  options.CreateDefaultArenasIfNotSet();
  return options;
}

class RouteClassifierTest : public ::testing::Test {
 protected:
  void SetUp() override {
    type_factory_ = std::make_unique<::googlesql::TypeFactory>();
    catalog_ = std::make_unique<::googlesql::SimpleCatalog>(
        "test_catalog", type_factory_.get());
    ::googlesql::LanguageOptions language;
    language.EnableMaximumLanguageFeatures();
    language.set_product_mode(::googlesql::PRODUCT_EXTERNAL);
    ASSERT_TRUE(catalog_
                    ->AddBuiltinFunctionsAndTypes(
                        ::googlesql::BuiltinFunctionOptions(language))
                    .ok());

    auto people = std::make_unique<::googlesql::SimpleTable>(
        "people",
        std::vector<::googlesql::SimpleTable::NameAndType>{
            {"id", type_factory_->get_int64()},
            {"name", type_factory_->get_string()},
        });
    catalog_->AddOwnedTable(std::move(people));
  }

  // Analyze `sql` against the fixture catalog and return the
  // resolved statement. `AnalyzerOutput` lives on `last_output_` so
  // the resolved AST (and the `Function*` / `Type*` pointers it
  // references) stays valid for the duration of the test.
  const ::googlesql::ResolvedStatement* Analyze(absl::string_view sql) {
    last_output_.reset();
    ::googlesql::AnalyzerOptions options = MakeAnalyzerOptions();
    absl::Status s = ::googlesql::AnalyzeStatement(
        sql, options, catalog_.get(), type_factory_.get(), &last_output_);
    EXPECT_TRUE(s.ok()) << s;
    if (!s.ok() || last_output_ == nullptr) return nullptr;
    return last_output_->resolved_statement();
  }

  std::unique_ptr<::googlesql::TypeFactory> type_factory_{};
  std::unique_ptr<::googlesql::SimpleCatalog> catalog_{};
  std::unique_ptr<const ::googlesql::AnalyzerOutput> last_output_{};
  RouteClassifier classifier_{};
};

TEST_F(RouteClassifierTest, PureDuckDbNativeSelectRoutesToDuckDb) {
  // `SELECT id, name FROM people` analyzes to ResolvedQueryStmt
  // wrapping a ResolvedProjectScan over a ResolvedTableScan. Every
  // node carries `duckdb_native` in `node_dispositions.yaml`, so
  // the classifier picks the DuckDB route with no offending node
  // and an empty `reason` (no promotion happened).
  const auto* stmt = Analyze("SELECT id, name FROM people");
  ASSERT_NE(stmt, nullptr);

  RouteDecision d = classifier_.Classify(*stmt);
  EXPECT_EQ(d.disposition, Disposition::kDuckdbNative);
  EXPECT_TRUE(d.offending_node.empty()) << d.offending_node;
  EXPECT_TRUE(d.reason.empty()) << d.reason;
}

TEST_F(RouteClassifierTest, PlannedSemanticExecutorFunctionDoesNotPromote) {
  // `SAFE_DIVIDE`'s `functions.yaml` row currently carries
  // `status=planned`: the disposition is `semantic_executor` but
  // the executor for that route is not landed yet (owned by
  // `semantic-functions-compliance.plan.md`). The upstream
  // execution-disposition-registry contract is that `planned`
  // rows must NOT be silently promoted -- they stay on the
  // default `duckdb_native` route until the owning plan lands the
  // real executor and drops the `planned` marker from the YAML.
  // The DuckDB transpiler then surfaces UNIMPLEMENTED from inside
  // its emit (per the empty-string contract), which is "no silent
  // approximation": a planned shape does not get re-routed onto a
  // stub executor that would also surface UNIMPLEMENTED but with
  // a different error envelope.
  //
  // When `semantic-functions-compliance.plan.md` lands and drops
  // `status=planned` from this row, this test will fail on the
  // disposition expectation; update it to assert
  // `Disposition::kSemanticExecutor` then.
  const auto* stmt = Analyze("SELECT SAFE_DIVIDE(1, 0)");
  ASSERT_NE(stmt, nullptr);

  RouteDecision d = classifier_.Classify(*stmt);
  EXPECT_EQ(d.disposition, Disposition::kDuckdbNative);
  EXPECT_TRUE(d.offending_node.empty()) << d.offending_node;
}

TEST_F(RouteClassifierTest, PlannedControlOpStatementDoesNotPromote) {
  // `ResolvedCreateTableStmt` has `disposition=control_op
  // status=planned` in `node_dispositions.yaml`. Same contract as
  // the planned function-row case above: planned rows surface
  // their target disposition for documentation but the classifier
  // keeps the actual route at `kDuckdbNative` until the owning
  // plan (`control-op-executor.plan.md`) lands the real executor
  // and drops the `planned` marker. The fast path (DuckDB) already
  // implements DDL via `DuckDbExecutor::ExecuteDdl`; routing CREATE
  // TABLE through the classifier-aware coordinator must preserve
  // the existing `gateway/e2e/ddl_create_drop_test.go` behavior,
  // which this test pins.
  //
  // When `control-op-executor.plan.md` lands and drops
  // `status=planned`, this test will fail on the disposition
  // expectation; update it to assert `Disposition::kControlOp`
  // then.
  const auto* stmt =
      Analyze("CREATE TABLE new_table (a INT64, b STRING)");
  ASSERT_NE(stmt, nullptr);

  RouteDecision d = classifier_.Classify(*stmt);
  EXPECT_EQ(d.disposition, Disposition::kDuckdbNative);
  EXPECT_TRUE(d.offending_node.empty()) << d.offending_node;
}

TEST_F(RouteClassifierTest, ApproxQuantilesFunctionRoutesToUnsupported) {
  // `APPROX_QUANTILES` is on the `unsupported` route per
  // `functions.yaml` (BigQuery's HLL-backed approximate aggregates
  // have no DuckDB analog with matching error surfaces). A SELECT
  // referencing it must route to the unsupported executor, with
  // the offending function name carried in the reason so the
  // gateway error can be useful to operators.
  const auto* stmt = Analyze("SELECT APPROX_QUANTILES(id, 4) FROM people");
  ASSERT_NE(stmt, nullptr);

  RouteDecision d = classifier_.Classify(*stmt);
  EXPECT_EQ(d.disposition, Disposition::kUnsupported);
  EXPECT_EQ(d.offending_node, "function:approx_quantiles");
  EXPECT_NE(d.reason.find("approx_quantiles"), std::string::npos)
      << "reason should name the offending function; got: " << d.reason;
}

TEST_F(RouteClassifierTest, UnsupportedDominatesPlannedSemanticInSameQuery) {
  // When an `unsupported` function and a `planned semantic_executor`
  // function appear together, the unsupported promotion wins.
  // `SAFE_DIVIDE` is currently `planned`, so it does not promote;
  // `APPROX_QUANTILES` is `unsupported` (not planned) and does
  // promote. This pins the priority order: a non-planned
  // `unsupported` row always wins over a planned-but-not-promoted
  // row.
  //
  // `AVG(SAFE_DIVIDE(...))` keeps the SELECT's projection
  // aggregate-only so the analyzer accepts the query without a
  // `GROUP BY`.
  const auto* stmt = Analyze(
      "SELECT APPROX_QUANTILES(id, 4), AVG(SAFE_DIVIDE(id, 1)) FROM people");
  ASSERT_NE(stmt, nullptr);

  RouteDecision d = classifier_.Classify(*stmt);
  EXPECT_EQ(d.disposition, Disposition::kUnsupported);
  EXPECT_EQ(d.offending_node, "function:approx_quantiles");
}

TEST_F(RouteClassifierTest, ExplainStatementRoutesToUnsupported) {
  // `ResolvedExplainStmt` is statement-level `unsupported`. Pin
  // that the classifier returns the unsupported route and records
  // the resolved class name (not a function name) as the
  // offending node.
  const auto* stmt = Analyze("EXPLAIN SELECT * FROM people");
  ASSERT_NE(stmt, nullptr);

  RouteDecision d = classifier_.Classify(*stmt);
  EXPECT_EQ(d.disposition, Disposition::kUnsupported);
  EXPECT_EQ(d.offending_node, "ResolvedExplainStmt");
}

}  // namespace
}  // namespace coordinator
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
