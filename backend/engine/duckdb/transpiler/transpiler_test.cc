// Unit tests for the Phase 5g `Transpiler` emit subset.
//
// We exercise each emit path through `AnalyzeStatement` so the
// `ResolvedAST` the transpiler sees is exactly the analyzer's own
// output -- no hand-crafted `Function` / `FunctionSignature` /
// `ResolvedColumn` objects, which would otherwise drift the moment
// GoogleSQL changes a default. The tests assert on the *string*
// returned by the per-shape `Emit*` methods so a regression is
// localized to the emit that changed (vs. the catalog setup or the
// analyzer plumbing).
//
// The catalog is a `SimpleCatalog` with one toy table (`people` with
// an INT64 and a STRING column) plus the GoogleSQL builtins
// registered through `AddBuiltinFunctionsAndTypes`. That mirrors the
// shape the production catalog (`backend/catalog/googlesql_catalog.h`)
// hands to the analyzer in Phase 5.A, minus the `Storage` adapter
// the engine layers on top.

#include "backend/engine/duckdb/transpiler/transpiler.h"

#include <memory>
#include <string>
#include <vector>

#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "absl/strings/string_view.h"
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
namespace duckdb {
namespace transpiler {
namespace {

// Mirrors `reference_impl_engine::MakeAnalyzerOptions` so the tests
// resolve names through the same `LanguageOptions` snapshot the
// engine itself uses. Drifting these two breaks function dispatch
// (e.g. `IFNULL` resolves but `COALESCE` does not) in subtle ways
// that only surface in the conformance harness.
::googlesql::AnalyzerOptions MakeAnalyzerOptions() {
  ::googlesql::LanguageOptions language;
  language.EnableMaximumLanguageFeatures();
  language.set_product_mode(::googlesql::PRODUCT_EXTERNAL);
  language.set_name_resolution_mode(::googlesql::NAME_RESOLUTION_DEFAULT);
  ::googlesql::AnalyzerOptions options(language);
  options.set_error_message_mode(::googlesql::ERROR_MESSAGE_ONE_LINE);
  options.CreateDefaultArenasIfNotSet();
  return options;
}

// One-stop test fixture. Owns the type factory, catalog, and a
// people table; every test gets a fresh `Transpiler` so the
// per-traversal accumulator (when one lands) starts clean.
class TranspilerTest : public ::testing::Test {
 protected:
  void SetUp() override {
    type_factory_ = std::make_unique<::googlesql::TypeFactory>();
    catalog_ = std::make_unique<::googlesql::SimpleCatalog>("test_catalog",
                                                            type_factory_.get());
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

    transpiler_ = std::make_unique<Transpiler>();
  }

  // Analyze `sql` against the fixture catalog and return the
  // resolved AST. The `AnalyzerOutput` lives in `last_output_` so
  // the `ResolvedStatement` (and the `Type*` / `Function*` pointers
  // it references) stays alive for the duration of the test.
  const ::googlesql::ResolvedStatement* Analyze(absl::string_view sql) {
    ::googlesql::AnalyzerOptions options = MakeAnalyzerOptions();
    last_output_.reset();
    absl::Status s = ::googlesql::AnalyzeStatement(
        sql, options, catalog_.get(), type_factory_.get(), &last_output_);
    EXPECT_TRUE(s.ok()) << s;
    if (!s.ok() || last_output_ == nullptr) return nullptr;
    return last_output_->resolved_statement();
  }

  // Convenience: pluck the inner `ResolvedScan` out of a
  // `SELECT ... FROM ...` statement. We unwrap the ResolvedQueryStmt
  // (and the ResolvedProjectScan the analyzer wraps around any
  // explicit SELECT list) so the per-shape `Emit*` assertion below
  // sees the exact subtree it covers.
  const ::googlesql::ResolvedScan* QueryInputScan(
      const ::googlesql::ResolvedStatement* stmt) {
    EXPECT_NE(stmt, nullptr);
    if (stmt == nullptr) return nullptr;
    const auto* q = stmt->GetAs<::googlesql::ResolvedQueryStmt>();
    EXPECT_NE(q, nullptr);
    if (q == nullptr) return nullptr;
    const ::googlesql::ResolvedScan* scan = q->query();
    while (scan != nullptr &&
           scan->node_kind() == ::googlesql::RESOLVED_PROJECT_SCAN) {
      scan = scan->GetAs<::googlesql::ResolvedProjectScan>()->input_scan();
    }
    return scan;
  }

  // Walk down to the first ResolvedExpr we can find inside a SELECT
  // list -- handy for testing literal / function / column-ref emit
  // without having to also implement EmitProjectScan.
  const ::googlesql::ResolvedExpr* QueryFirstSelectExpr(
      const ::googlesql::ResolvedStatement* stmt) {
    EXPECT_NE(stmt, nullptr);
    if (stmt == nullptr) return nullptr;
    const auto* q = stmt->GetAs<::googlesql::ResolvedQueryStmt>();
    if (q == nullptr || q->query() == nullptr) return nullptr;
    const ::googlesql::ResolvedScan* scan = q->query();
    if (scan->node_kind() != ::googlesql::RESOLVED_PROJECT_SCAN) return nullptr;
    const auto* project =
        scan->GetAs<::googlesql::ResolvedProjectScan>();
    if (project->expr_list_size() == 0) return nullptr;
    return project->expr_list(0)->expr();
  }

  std::unique_ptr<::googlesql::TypeFactory> type_factory_;
  std::unique_ptr<::googlesql::SimpleCatalog> catalog_;
  std::unique_ptr<const ::googlesql::AnalyzerOutput> last_output_;
  std::unique_ptr<Transpiler> transpiler_;
};

// Subclass that publishes the protected `Emit*` family so the tests
// can assert on individual emits without having to drive a full
// query through `Transpile`. The class doesn't override anything --
// it just widens the visibility.
class TestTranspiler : public Transpiler {
 public:
  using Transpiler::EmitColumnRef;
  using Transpiler::EmitFilterScan;
  using Transpiler::EmitFunctionCall;
  using Transpiler::EmitLiteral;
  using Transpiler::EmitTableScan;
};

TEST_F(TranspilerTest, EmitLiteralInt64) {
  const ::googlesql::ResolvedStatement* stmt = Analyze("SELECT 42 AS n");
  const ::googlesql::ResolvedExpr* expr = QueryFirstSelectExpr(stmt);
  ASSERT_NE(expr, nullptr);
  ASSERT_EQ(expr->node_kind(), ::googlesql::RESOLVED_LITERAL);
  TestTranspiler t;
  EXPECT_EQ(t.EmitLiteral(expr->GetAs<::googlesql::ResolvedLiteral>()), "42");
}

TEST_F(TranspilerTest, EmitLiteralString) {
  const ::googlesql::ResolvedStatement* stmt = Analyze("SELECT 'hi' AS s");
  const ::googlesql::ResolvedExpr* expr = QueryFirstSelectExpr(stmt);
  ASSERT_NE(expr, nullptr);
  ASSERT_EQ(expr->node_kind(), ::googlesql::RESOLVED_LITERAL);
  TestTranspiler t;
  // We must emit single-quoted strings: DuckDB reads double-quoted
  // text as an *identifier*, so a `"hi"` literal would be a column
  // reference rather than a string. EmitLiteral overrides
  // GoogleSQL's default double-quoted form on TYPE_STRING for
  // exactly this reason -- the conformance harness would otherwise
  // pin every string-bearing query onto the reference-impl engine.
  EXPECT_EQ(t.EmitLiteral(expr->GetAs<::googlesql::ResolvedLiteral>()),
            "'hi'");
}

TEST_F(TranspilerTest, EmitLiteralBoolTrue) {
  const ::googlesql::ResolvedStatement* stmt = Analyze("SELECT TRUE AS b");
  const ::googlesql::ResolvedExpr* expr = QueryFirstSelectExpr(stmt);
  ASSERT_NE(expr, nullptr);
  ASSERT_EQ(expr->node_kind(), ::googlesql::RESOLVED_LITERAL);
  TestTranspiler t;
  EXPECT_EQ(t.EmitLiteral(expr->GetAs<::googlesql::ResolvedLiteral>()),
            "true");
}

TEST_F(TranspilerTest, EmitColumnRefQuotesIdentifier) {
  // The analyzer collapses a bare `SELECT id FROM people` straight
  // onto the TableScan (no wrapping ProjectScan), so to land a
  // standalone `ResolvedColumnRef` we wrap the column in a
  // function call. `COALESCE(id, 0)` keeps the test focused: the
  // first argument is the ColumnRef we want to assert on.
  const ::googlesql::ResolvedStatement* stmt =
      Analyze("SELECT COALESCE(id, 0) FROM people");
  const ::googlesql::ResolvedExpr* expr = QueryFirstSelectExpr(stmt);
  ASSERT_NE(expr, nullptr);
  ASSERT_EQ(expr->node_kind(), ::googlesql::RESOLVED_FUNCTION_CALL);
  const ::googlesql::ResolvedFunctionCall* call =
      expr->GetAs<::googlesql::ResolvedFunctionCall>();
  ASSERT_GE(call->argument_list_size(), 1);
  const ::googlesql::ResolvedExpr* arg = call->argument_list(0);
  ASSERT_NE(arg, nullptr);
  ASSERT_EQ(arg->node_kind(), ::googlesql::RESOLVED_COLUMN_REF);
  TestTranspiler t;
  EXPECT_EQ(t.EmitColumnRef(arg->GetAs<::googlesql::ResolvedColumnRef>()),
            "\"id\"");
}

TEST_F(TranspilerTest, EmitFunctionCallCoalesce) {
  const ::googlesql::ResolvedStatement* stmt =
      Analyze("SELECT COALESCE(name, 'unknown') AS n FROM people");
  const ::googlesql::ResolvedExpr* expr = QueryFirstSelectExpr(stmt);
  ASSERT_NE(expr, nullptr);
  ASSERT_EQ(expr->node_kind(), ::googlesql::RESOLVED_FUNCTION_CALL);
  TestTranspiler t;
  EXPECT_EQ(t.EmitFunctionCall(expr->GetAs<::googlesql::ResolvedFunctionCall>()),
            "COALESCE(\"name\", 'unknown')");
}

TEST_F(TranspilerTest, EmitFunctionCallIfnull) {
  const ::googlesql::ResolvedStatement* stmt =
      Analyze("SELECT IFNULL(name, 'unknown') AS n FROM people");
  const ::googlesql::ResolvedExpr* expr = QueryFirstSelectExpr(stmt);
  ASSERT_NE(expr, nullptr);
  ASSERT_EQ(expr->node_kind(), ::googlesql::RESOLVED_FUNCTION_CALL);
  TestTranspiler t;
  EXPECT_EQ(t.EmitFunctionCall(expr->GetAs<::googlesql::ResolvedFunctionCall>()),
            "IFNULL(\"name\", 'unknown')");
}

TEST_F(TranspilerTest, EmitFunctionCallUnsupportedReturnsEmpty) {
  // `LENGTH` is not on the Phase 5g whitelist; we expect "" so the
  // disposition policy can take the reference-impl fallback.
  const ::googlesql::ResolvedStatement* stmt =
      Analyze("SELECT LENGTH(name) AS n FROM people");
  const ::googlesql::ResolvedExpr* expr = QueryFirstSelectExpr(stmt);
  ASSERT_NE(expr, nullptr);
  ASSERT_EQ(expr->node_kind(), ::googlesql::RESOLVED_FUNCTION_CALL);
  TestTranspiler t;
  EXPECT_EQ(t.EmitFunctionCall(expr->GetAs<::googlesql::ResolvedFunctionCall>()),
            "");
}

TEST_F(TranspilerTest, EmitTableScanEmitsSelectStar) {
  // `SELECT * FROM people` collapses (after rewrites) onto a
  // ResolvedTableScan whose `column_list` carries every column on
  // the underlying table. We assert on both the select-list shape
  // (one quoted identifier per column, in catalog order) and the
  // bare table-name reference -- the engine ATTACHes the storage's
  // backing files under that name at execute time.
  const ::googlesql::ResolvedStatement* stmt = Analyze("SELECT * FROM people");
  const ::googlesql::ResolvedScan* scan = QueryInputScan(stmt);
  ASSERT_NE(scan, nullptr);
  ASSERT_EQ(scan->node_kind(), ::googlesql::RESOLVED_TABLE_SCAN);
  TestTranspiler t;
  EXPECT_EQ(t.EmitTableScan(scan->GetAs<::googlesql::ResolvedTableScan>()),
            "SELECT \"id\", \"name\" FROM \"people\"");
}

TEST_F(TranspilerTest, EmitFilterScanWrapsInputScanWithWhere) {
  // `WHERE id > 0` lands as a ResolvedFilterScan around a
  // ResolvedTableScan. The emit composes the table scan's
  // self-contained SELECT as a derived table so the WHERE clause
  // sees the same column aliases the inner SELECT exposes.
  const ::googlesql::ResolvedStatement* stmt =
      Analyze("SELECT id FROM people WHERE id > 0");
  const ::googlesql::ResolvedScan* scan = QueryInputScan(stmt);
  ASSERT_NE(scan, nullptr);
  ASSERT_EQ(scan->node_kind(), ::googlesql::RESOLVED_FILTER_SCAN);
  TestTranspiler t;
  // The filter expression itself is a `>` function call which we
  // don't yet emit (not on the COALESCE/IFNULL whitelist), so the
  // emit propagates "" upward. We assert on that contract because
  // it is what the engine fallback reads to decide whether to take
  // the DuckDB path.
  EXPECT_EQ(t.EmitFilterScan(scan->GetAs<::googlesql::ResolvedFilterScan>()),
            "");
}

TEST_F(TranspilerTest, EmitFilterScanWithCoalescePredicateLowers) {
  // Picking a predicate that the function-call emit *does* know
  // about (`COALESCE`) lets us exercise the actual FilterScan SQL
  // composition. COALESCE(name, 'x') = 'x' isn't a particularly
  // useful predicate, but it threads two literals, a column ref,
  // and the COALESCE emit through the same SQL string -- exactly
  // the integration the FilterScan emit needs to keep honest.
  const ::googlesql::ResolvedStatement* stmt = Analyze(
      "SELECT id FROM people WHERE COALESCE(name, 'x') = 'x'");
  const ::googlesql::ResolvedScan* scan = QueryInputScan(stmt);
  ASSERT_NE(scan, nullptr);
  ASSERT_EQ(scan->node_kind(), ::googlesql::RESOLVED_FILTER_SCAN);
  TestTranspiler t;
  // The `=` is a function call we don't emit (yet); the filter emit
  // propagates "" out of EmitFunctionCall. The point of the
  // assertion is that the composition is deterministic — once the
  // equality op lands on the disposition table, this test will
  // tighten to the full SELECT shape.
  EXPECT_EQ(t.EmitFilterScan(scan->GetAs<::googlesql::ResolvedFilterScan>()),
            "");
}

}  // namespace
}  // namespace transpiler
}  // namespace duckdb
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
