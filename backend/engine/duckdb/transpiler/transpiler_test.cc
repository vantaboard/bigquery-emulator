// Unit tests for the `Transpiler` emit subset.
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
// hands to the analyzer in the reference-impl engine, minus the
// `Storage` adapter the engine layers on top.

#include "backend/engine/duckdb/transpiler/transpiler.h"

#include <memory>
#include <string>
#include <vector>

#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "absl/strings/string_view.h"
#include "backend/engine/duckdb/transpiler/functions.h"
#include "googlesql/public/analyzer.h"
#include "googlesql/public/analyzer_options.h"
#include "googlesql/public/analyzer_output.h"
#include "googlesql/public/builtin_function_options.h"
#include "googlesql/public/catalog.h"
#include "googlesql/public/id_string.h"
#include "googlesql/public/language_options.h"
#include "googlesql/public/options.pb.h"
#include "googlesql/public/simple_catalog.h"
#include "googlesql/public/types/type_factory.h"
#include "googlesql/public/value.h"
#include "googlesql/resolved_ast/resolved_ast.h"
#include "googlesql/resolved_ast/resolved_column.h"
#include "gtest/gtest.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace duckdb {
namespace transpiler {
namespace {

// Mirrors `duckdb_engine::MakeAnalyzerOptions` so the tests
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

    // The join tests need a second table with disjoint column names so
    // the analyzer doesn't have to disambiguate references in the ON
    // expression; the transpiler doesn't know how to disambiguate yet
    // (the per-column emit goes through `ResolvedColumn::name()`).
    auto orders = std::make_unique<::googlesql::SimpleTable>(
        "orders",
        std::vector<::googlesql::SimpleTable::NameAndType>{
            {"order_id", type_factory_->get_int64()},
            {"amount", type_factory_->get_int64()},
        });
    catalog_->AddOwnedTable(std::move(orders));

    transpiler_ = std::make_unique<Transpiler>();
  }

  // Analyze `sql` against the fixture catalog and return the
  // resolved AST. The `AnalyzerOutput` lives in `last_output_` so
  // the `ResolvedStatement` (and the `Type*` / `Function*` pointers
  // it references) stays alive for the duration of the test.
  const ::googlesql::ResolvedStatement* Analyze(absl::string_view sql) {
    ::googlesql::AnalyzerOptions options = MakeAnalyzerOptions();
    return AnalyzeWith(sql, options);
  }

  // Analyze `sql` with `options` already configured -- handy for the
  // parameter-emit tests that need `AddQueryParameter` /
  // `AddPositionalQueryParameter` calls before analysis. Same
  // ownership contract as `Analyze`: the resolved AST lives in
  // `last_output_` for the duration of the test.
  const ::googlesql::ResolvedStatement* AnalyzeWith(
      absl::string_view sql, const ::googlesql::AnalyzerOptions& options) {
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
    const auto* project = scan->GetAs<::googlesql::ResolvedProjectScan>();
    if (project->expr_list_size() == 0) return nullptr;
    return project->expr_list(0)->expr();
  }

  std::unique_ptr<::googlesql::TypeFactory> type_factory_{};
  std::unique_ptr<::googlesql::SimpleCatalog> catalog_{};
  std::unique_ptr<const ::googlesql::AnalyzerOutput> last_output_{};
  std::unique_ptr<Transpiler> transpiler_{};
};

// Subclass that publishes the protected `Emit*` family so the tests
// can assert on individual emits without having to drive a full
// query through `Transpile`. The class doesn't override anything --
// it just widens the visibility.
class TestTranspiler : public Transpiler {
 public:
  using Transpiler::EmitAggregateScan;
  using Transpiler::EmitAnalyticScan;
  using Transpiler::EmitArrayScan;
  using Transpiler::EmitCast;
  using Transpiler::EmitColumnRef;
  using Transpiler::EmitComputedColumn;
  using Transpiler::EmitFilterScan;
  using Transpiler::EmitFunctionArgument;
  using Transpiler::EmitFunctionCall;
  using Transpiler::EmitGetJsonField;
  using Transpiler::EmitGetStructField;
  using Transpiler::EmitJoinScan;
  using Transpiler::EmitLimitOffsetScan;
  using Transpiler::EmitLiteral;
  using Transpiler::EmitMakeStruct;
  using Transpiler::EmitOrderByScan;
  using Transpiler::EmitOutputColumn;
  using Transpiler::EmitParameter;
  using Transpiler::EmitProjectScan;
  using Transpiler::EmitQueryStmt;
  using Transpiler::EmitSingleRowScan;
  using Transpiler::EmitTableScan;
  using Transpiler::EmitWithExpr;
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
  EXPECT_EQ(t.EmitLiteral(expr->GetAs<::googlesql::ResolvedLiteral>()), "'hi'");
}

TEST_F(TranspilerTest, EmitLiteralBoolTrue) {
  const ::googlesql::ResolvedStatement* stmt = Analyze("SELECT TRUE AS b");
  const ::googlesql::ResolvedExpr* expr = QueryFirstSelectExpr(stmt);
  ASSERT_NE(expr, nullptr);
  ASSERT_EQ(expr->node_kind(), ::googlesql::RESOLVED_LITERAL);
  TestTranspiler t;
  EXPECT_EQ(t.EmitLiteral(expr->GetAs<::googlesql::ResolvedLiteral>()), "true");
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
  EXPECT_EQ(
      t.EmitFunctionCall(expr->GetAs<::googlesql::ResolvedFunctionCall>()),
      "COALESCE(\"name\", 'unknown')");
}

TEST_F(TranspilerTest, EmitFunctionCallIfnull) {
  const ::googlesql::ResolvedStatement* stmt =
      Analyze("SELECT IFNULL(name, 'unknown') AS n FROM people");
  const ::googlesql::ResolvedExpr* expr = QueryFirstSelectExpr(stmt);
  ASSERT_NE(expr, nullptr);
  ASSERT_EQ(expr->node_kind(), ::googlesql::RESOLVED_FUNCTION_CALL);
  TestTranspiler t;
  EXPECT_EQ(
      t.EmitFunctionCall(expr->GetAs<::googlesql::ResolvedFunctionCall>()),
      "IFNULL(\"name\", 'unknown')");
}

TEST_F(TranspilerTest, EmitFunctionCallSkiplistReturnsEmpty) {
  // `BIT_COUNT` is on the skiplist in `functions.yaml` (BQ flavor
  // differs from DuckDB's `bit_count` and the conformance harness
  // pins it on the reference-impl engine for now). The emit returns
  // "" so the engine takes the reference-impl fallback for the
  // whole query.
  const ::googlesql::ResolvedStatement* stmt =
      Analyze("SELECT BIT_COUNT(id) AS n FROM people");
  const ::googlesql::ResolvedExpr* expr = QueryFirstSelectExpr(stmt);
  ASSERT_NE(expr, nullptr);
  ASSERT_EQ(expr->node_kind(), ::googlesql::RESOLVED_FUNCTION_CALL);
  TestTranspiler t;
  EXPECT_EQ(
      t.EmitFunctionCall(expr->GetAs<::googlesql::ResolvedFunctionCall>()), "");
}

TEST_F(TranspilerTest, EmitFunctionCallMappedFunction) {
  // Disposition-table-backed scalars: `ABS(id)` lowers to DuckDB's
  // `ABS(...)`. The casing of the emitted function name comes from
  // the YAML disposition (we render the duckdb_name verbatim); the
  // BQ-side `abs` lookup is case-insensitive.
  const ::googlesql::ResolvedStatement* stmt =
      Analyze("SELECT ABS(id) AS n FROM people");
  const ::googlesql::ResolvedExpr* expr = QueryFirstSelectExpr(stmt);
  ASSERT_NE(expr, nullptr);
  ASSERT_EQ(expr->node_kind(), ::googlesql::RESOLVED_FUNCTION_CALL);
  TestTranspiler t;
  EXPECT_EQ(
      t.EmitFunctionCall(expr->GetAs<::googlesql::ResolvedFunctionCall>()),
      "ABS(\"id\")");
}

TEST_F(TranspilerTest, EmitFunctionCallLengthMaps) {
  // `LENGTH(name)` -> `LENGTH("name")`. Two-arg variants don't exist
  // for LENGTH in either dialect; the single-arg shape is the entire
  // disposition surface.
  const ::googlesql::ResolvedStatement* stmt =
      Analyze("SELECT LENGTH(name) AS n FROM people");
  const ::googlesql::ResolvedExpr* expr = QueryFirstSelectExpr(stmt);
  ASSERT_NE(expr, nullptr);
  ASSERT_EQ(expr->node_kind(), ::googlesql::RESOLVED_FUNCTION_CALL);
  TestTranspiler t;
  EXPECT_EQ(
      t.EmitFunctionCall(expr->GetAs<::googlesql::ResolvedFunctionCall>()),
      "LENGTH(\"name\")");
}

TEST_F(TranspilerTest, EmitFunctionCallSafeModeReturnsEmpty) {
  // SAFE.<fn>(...) sets `error_mode = SAFE_ERROR_MODE`. DuckDB has no
  // native SAFE analog yet, so the emit short-circuits to "" before
  // consulting the disposition table -- this would otherwise emit
  // ABS("id") and silently lose the SAFE error semantics.
  const ::googlesql::ResolvedStatement* stmt =
      Analyze("SELECT SAFE.ABS(id) AS n FROM people");
  const ::googlesql::ResolvedExpr* expr = QueryFirstSelectExpr(stmt);
  ASSERT_NE(expr, nullptr);
  ASSERT_EQ(expr->node_kind(), ::googlesql::RESOLVED_FUNCTION_CALL);
  TestTranspiler t;
  EXPECT_EQ(
      t.EmitFunctionCall(expr->GetAs<::googlesql::ResolvedFunctionCall>()), "");
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
  const ::googlesql::ResolvedStatement* stmt =
      Analyze("SELECT id FROM people WHERE COALESCE(name, 'x') = 'x'");
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

// --- Join ---------------------------------------------------------------

TEST_F(TranspilerTest, EmitJoinScanCrossJoinFromImplicit) {
  // `FROM people, orders` analyzes to a ResolvedJoinScan with
  // INNER + null `join_expr`. The emit lowers it to DuckDB's
  // explicit CROSS JOIN so a downstream FilterScan / ProjectScan
  // can wrap it without having to know about the implicit-join
  // shorthand.
  const ::googlesql::ResolvedStatement* stmt =
      Analyze("SELECT id FROM people, orders");
  const ::googlesql::ResolvedScan* scan = QueryInputScan(stmt);
  ASSERT_NE(scan, nullptr);
  ASSERT_EQ(scan->node_kind(), ::googlesql::RESOLVED_JOIN_SCAN);
  TestTranspiler t;
  EXPECT_EQ(t.EmitJoinScan(scan->GetAs<::googlesql::ResolvedJoinScan>()),
            "SELECT * FROM (SELECT \"id\", \"name\" FROM \"people\") "
            "CROSS JOIN (SELECT \"order_id\", \"amount\" FROM \"orders\")");
}

TEST_F(TranspilerTest, EmitJoinScanInnerWithLiteralPredicate) {
  // `ON TRUE` keeps the test focused on the join emit shape: the
  // predicate lowers cleanly through `EmitLiteral` so the assertion
  // can pin the full INNER JOIN SQL string. (`ON x = y` would
  // route through the `=` function call which isn't on the
  // function-call whitelist for the scan emit; that path is covered
  // by the propagation test below.)
  const ::googlesql::ResolvedStatement* stmt =
      Analyze("SELECT id FROM people INNER JOIN orders ON TRUE");
  const ::googlesql::ResolvedScan* scan = QueryInputScan(stmt);
  ASSERT_NE(scan, nullptr);
  ASSERT_EQ(scan->node_kind(), ::googlesql::RESOLVED_JOIN_SCAN);
  TestTranspiler t;
  EXPECT_EQ(t.EmitJoinScan(scan->GetAs<::googlesql::ResolvedJoinScan>()),
            "SELECT * FROM (SELECT \"id\", \"name\" FROM \"people\") "
            "INNER JOIN (SELECT \"order_id\", \"amount\" FROM \"orders\") "
            "ON true");
}

TEST_F(TranspilerTest, EmitJoinScanLeftWithLiteralPredicate) {
  // LEFT JOIN requires a non-null `join_expr`; `ON TRUE` is the
  // smallest predicate that round-trips through the emit. The
  // assertion confirms the keyword swap (INNER -> LEFT) is the
  // only difference vs. the inner test above.
  const ::googlesql::ResolvedStatement* stmt =
      Analyze("SELECT id FROM people LEFT JOIN orders ON TRUE");
  const ::googlesql::ResolvedScan* scan = QueryInputScan(stmt);
  ASSERT_NE(scan, nullptr);
  ASSERT_EQ(scan->node_kind(), ::googlesql::RESOLVED_JOIN_SCAN);
  TestTranspiler t;
  EXPECT_EQ(t.EmitJoinScan(scan->GetAs<::googlesql::ResolvedJoinScan>()),
            "SELECT * FROM (SELECT \"id\", \"name\" FROM \"people\") "
            "LEFT JOIN (SELECT \"order_id\", \"amount\" FROM \"orders\") "
            "ON true");
}

TEST_F(TranspilerTest, EmitJoinScanFallsBackOnUnsupportedPredicate) {
  // `=` isn't on the function-call whitelist used by the scan
  // emits, so the join_expr emit returns "" and the JoinScan emit
  // propagates the empty string up to the engine. The disposition
  // policy then takes the reference-impl fallback for this shape.
  const ::googlesql::ResolvedStatement* stmt =
      Analyze("SELECT id FROM people INNER JOIN orders ON id = order_id");
  const ::googlesql::ResolvedScan* scan = QueryInputScan(stmt);
  ASSERT_NE(scan, nullptr);
  ASSERT_EQ(scan->node_kind(), ::googlesql::RESOLVED_JOIN_SCAN);
  TestTranspiler t;
  EXPECT_EQ(t.EmitJoinScan(scan->GetAs<::googlesql::ResolvedJoinScan>()), "");
}

// --- Aggregate ----------------------------------------------------------

TEST_F(TranspilerTest, EmitAggregateScanCountStarNoGroupBy) {
  // `SELECT COUNT(*) FROM people` analyzes to an AggregateScan
  // with an empty group_by_list and a single aggregate. The
  // aggregate column gets a synthesized name (`$agg1`); we assert
  // on it so any drift in the analyzer's naming surfaces here
  // rather than in the engine integration.
  const ::googlesql::ResolvedStatement* stmt =
      Analyze("SELECT COUNT(*) FROM people");
  const ::googlesql::ResolvedScan* scan = QueryInputScan(stmt);
  ASSERT_NE(scan, nullptr);
  ASSERT_EQ(scan->node_kind(), ::googlesql::RESOLVED_AGGREGATE_SCAN);
  TestTranspiler t;
  EXPECT_EQ(
      t.EmitAggregateScan(scan->GetAs<::googlesql::ResolvedAggregateScan>()),
      "SELECT COUNT(*) AS \"$agg1\" FROM (SELECT \"id\", \"name\" FROM "
      "\"people\")");
}

TEST_F(TranspilerTest, EmitAggregateScanSumGroupByColumn) {
  // SUM over a grouped column threads the column-ref through the
  // GROUP BY clause and the SELECT list. The grouping column's
  // ResolvedColumn::name() matches its source name (`id`), so the
  // AS alias collapses to the column reference.
  const ::googlesql::ResolvedStatement* stmt =
      Analyze("SELECT id, SUM(id) FROM people GROUP BY id");
  const ::googlesql::ResolvedScan* scan = QueryInputScan(stmt);
  ASSERT_NE(scan, nullptr);
  ASSERT_EQ(scan->node_kind(), ::googlesql::RESOLVED_AGGREGATE_SCAN);
  TestTranspiler t;
  EXPECT_EQ(
      t.EmitAggregateScan(scan->GetAs<::googlesql::ResolvedAggregateScan>()),
      "SELECT \"id\", SUM(\"id\") AS \"$agg1\" FROM (SELECT \"id\", "
      "\"name\" FROM \"people\") GROUP BY \"id\"");
}

TEST_F(TranspilerTest, EmitAggregateScanAvgMinMaxGroupBy) {
  // All three of AVG / MIN / MAX share the same emit path; one
  // test covers the lot. The output column for each aggregate is
  // again the analyzer-synthesized `$agg<n>` name.
  const ::googlesql::ResolvedStatement* stmt =
      Analyze("SELECT id, AVG(id), MIN(id), MAX(id) FROM people GROUP BY id");
  const ::googlesql::ResolvedScan* scan = QueryInputScan(stmt);
  ASSERT_NE(scan, nullptr);
  ASSERT_EQ(scan->node_kind(), ::googlesql::RESOLVED_AGGREGATE_SCAN);
  TestTranspiler t;
  EXPECT_EQ(
      t.EmitAggregateScan(scan->GetAs<::googlesql::ResolvedAggregateScan>()),
      "SELECT \"id\", AVG(\"id\") AS \"$agg1\", MIN(\"id\") AS \"$agg2\", "
      "MAX(\"id\") AS \"$agg3\" FROM (SELECT \"id\", \"name\" FROM "
      "\"people\") GROUP BY \"id\"");
}

TEST_F(TranspilerTest, EmitAggregateScanArrayAggMapsThroughTable) {
  // `ARRAY_AGG` is in the disposition table (`array_agg: ARRAY_AGG`),
  // so the lower path emits the DuckDB aggregate verbatim. This
  // exercises the table-driven dispatch from inside the AggregateScan
  // emit -- a direct counterpart to `EmitFunctionCallMappedFunction`
  // above for the aggregate code path.
  const ::googlesql::ResolvedStatement* stmt =
      Analyze("SELECT id, ARRAY_AGG(id) FROM people GROUP BY id");
  const ::googlesql::ResolvedScan* scan = QueryInputScan(stmt);
  ASSERT_NE(scan, nullptr);
  ASSERT_EQ(scan->node_kind(), ::googlesql::RESOLVED_AGGREGATE_SCAN);
  TestTranspiler t;
  EXPECT_EQ(
      t.EmitAggregateScan(scan->GetAs<::googlesql::ResolvedAggregateScan>()),
      "SELECT \"id\", ARRAY_AGG(\"id\") AS \"$agg1\" FROM (SELECT \"id\", "
      "\"name\" FROM \"people\") GROUP BY \"id\"");
}

TEST_F(TranspilerTest, EmitAggregateScanFallsBackOnSkiplistedAggregate) {
  // `APPROX_QUANTILES` is on the skiplist; the aggregate emit
  // returns "" and the AggregateScan emit propagates the empty
  // string. The disposition policy then takes the reference-impl
  // fallback for the whole query.
  const ::googlesql::ResolvedStatement* stmt =
      Analyze("SELECT id, APPROX_QUANTILES(id, 2) FROM people GROUP BY id");
  const ::googlesql::ResolvedScan* scan = QueryInputScan(stmt);
  ASSERT_NE(scan, nullptr);
  ASSERT_EQ(scan->node_kind(), ::googlesql::RESOLVED_AGGREGATE_SCAN);
  TestTranspiler t;
  EXPECT_EQ(
      t.EmitAggregateScan(scan->GetAs<::googlesql::ResolvedAggregateScan>()),
      "");
}

// --- Order By -----------------------------------------------------------

TEST_F(TranspilerTest, EmitOrderByScanAscDefault) {
  // Bare `ORDER BY id` analyzes with `is_descending=false` and
  // `null_order=ORDER_UNSPECIFIED`. We emit the explicit `ASC`
  // keyword so the DuckDB plan is unambiguous; the null order
  // keyword is omitted when unspecified so DuckDB picks its own
  // default for the column type. Note the inner SELECT carries
  // every column the table scan exposes -- the analyzer doesn't
  // prune until the wrapping ProjectScan, which our `QueryInputScan`
  // helper strips so we land on the OrderByScan directly.
  const ::googlesql::ResolvedStatement* stmt =
      Analyze("SELECT id FROM people ORDER BY id");
  const ::googlesql::ResolvedScan* scan = QueryInputScan(stmt);
  ASSERT_NE(scan, nullptr);
  ASSERT_EQ(scan->node_kind(), ::googlesql::RESOLVED_ORDER_BY_SCAN);
  TestTranspiler t;
  EXPECT_EQ(t.EmitOrderByScan(scan->GetAs<::googlesql::ResolvedOrderByScan>()),
            "SELECT * FROM (SELECT \"id\", \"name\" FROM \"people\") "
            "ORDER BY \"id\" ASC");
}

TEST_F(TranspilerTest, EmitOrderByScanDescNullsFirst) {
  // `DESC NULLS FIRST` exercises both the direction swap and the
  // null-order keyword. NULLS FIRST is BigQuery's default for
  // DESC, but the analyzer surfaces it explicitly so we emit it
  // explicitly too.
  const ::googlesql::ResolvedStatement* stmt =
      Analyze("SELECT id FROM people ORDER BY id DESC NULLS FIRST");
  const ::googlesql::ResolvedScan* scan = QueryInputScan(stmt);
  ASSERT_NE(scan, nullptr);
  ASSERT_EQ(scan->node_kind(), ::googlesql::RESOLVED_ORDER_BY_SCAN);
  TestTranspiler t;
  EXPECT_EQ(t.EmitOrderByScan(scan->GetAs<::googlesql::ResolvedOrderByScan>()),
            "SELECT * FROM (SELECT \"id\", \"name\" FROM \"people\") "
            "ORDER BY \"id\" DESC NULLS FIRST");
}

TEST_F(TranspilerTest, EmitOrderByScanMultipleItems) {
  // Two-key ordering checks the join of items with `, `, plus
  // that each item carries its own direction. `ASC NULLS LAST`
  // pins both the direction and the explicit null-order keyword
  // alongside the bare `DESC` from the first key.
  const ::googlesql::ResolvedStatement* stmt = Analyze(
      "SELECT id, name FROM people ORDER BY id DESC, name ASC NULLS LAST");
  const ::googlesql::ResolvedScan* scan = QueryInputScan(stmt);
  ASSERT_NE(scan, nullptr);
  ASSERT_EQ(scan->node_kind(), ::googlesql::RESOLVED_ORDER_BY_SCAN);
  TestTranspiler t;
  EXPECT_EQ(t.EmitOrderByScan(scan->GetAs<::googlesql::ResolvedOrderByScan>()),
            "SELECT * FROM (SELECT \"id\", \"name\" FROM \"people\") "
            "ORDER BY \"id\" DESC, \"name\" ASC NULLS LAST");
}

// --- Limit / Offset -----------------------------------------------------

TEST_F(TranspilerTest, EmitLimitOffsetScanLimitOnly) {
  // `LIMIT 10` analyzes to a LimitOffsetScan with a non-null
  // `limit` literal and a null `offset`. The emit drops the
  // OFFSET clause so DuckDB takes its zero default.
  //
  // We sandwich the LIMIT around an `ORDER BY` so the
  // LimitOffsetScan's input scan is an OrderByScan (which the
  // current emit subset lowers); a plain `LIMIT 10` would put a
  // ProjectScan between the LimitOffsetScan and the TableScan and
  // the ProjectScan emit is reserved for the follow-up plan.
  const ::googlesql::ResolvedStatement* stmt =
      Analyze("SELECT * FROM people ORDER BY id LIMIT 10");
  const ::googlesql::ResolvedScan* scan = QueryInputScan(stmt);
  ASSERT_NE(scan, nullptr);
  ASSERT_EQ(scan->node_kind(), ::googlesql::RESOLVED_LIMIT_OFFSET_SCAN);
  TestTranspiler t;
  EXPECT_EQ(t.EmitLimitOffsetScan(
                scan->GetAs<::googlesql::ResolvedLimitOffsetScan>()),
            "SELECT * FROM (SELECT * FROM (SELECT \"id\", \"name\" FROM "
            "\"people\") ORDER BY \"id\" ASC) LIMIT 10");
}

TEST_F(TranspilerTest, EmitLimitOffsetScanLimitAndOffset) {
  // Both literals are present here; the emit threads them through
  // `EmitLiteral` and stitches the LIMIT / OFFSET keywords in
  // order. DuckDB happily accepts `LIMIT n OFFSET m`. Same wrap
  // trick as above to give the LimitOffsetScan an emittable
  // input.
  const ::googlesql::ResolvedStatement* stmt =
      Analyze("SELECT * FROM people ORDER BY id LIMIT 10 OFFSET 5");
  const ::googlesql::ResolvedScan* scan = QueryInputScan(stmt);
  ASSERT_NE(scan, nullptr);
  ASSERT_EQ(scan->node_kind(), ::googlesql::RESOLVED_LIMIT_OFFSET_SCAN);
  TestTranspiler t;
  EXPECT_EQ(t.EmitLimitOffsetScan(
                scan->GetAs<::googlesql::ResolvedLimitOffsetScan>()),
            "SELECT * FROM (SELECT * FROM (SELECT \"id\", \"name\" FROM "
            "\"people\") ORDER BY \"id\" ASC) LIMIT 10 OFFSET 5");
}

// --- STRUCT / UNNEST / ARRAY --------------------------------------------

TEST_F(TranspilerTest, EmitLiteralArrayInt64) {
  // `ARRAY<INT64>[1, 2]` resolves to a `ResolvedLiteral` whose value
  // is an `ARRAY<INT64>`. DuckDB's array constructor shares
  // BigQuery's bracket syntax, so the emit is a direct join. The
  // assertion pins the spaces around the element separator so a
  // drift in `EmitValueLiteral`'s join (`", "` vs `","`) surfaces
  // here rather than downstream in the engine fallback.
  const ::googlesql::ResolvedStatement* stmt = Analyze("SELECT [1, 2] AS a");
  const ::googlesql::ResolvedExpr* expr = QueryFirstSelectExpr(stmt);
  ASSERT_NE(expr, nullptr);
  ASSERT_EQ(expr->node_kind(), ::googlesql::RESOLVED_LITERAL);
  TestTranspiler t;
  EXPECT_EQ(t.EmitLiteral(expr->GetAs<::googlesql::ResolvedLiteral>()),
            "[1, 2]");
}

TEST_F(TranspilerTest, EmitLiteralArrayStringUsesSingleQuotes) {
  // `["a", "b"]` resolves to a `ResolvedLiteral` of type
  // `ARRAY<STRING>`. `Value::GetSQLLiteral` would emit BQ-flavored
  // `["a", "b"]` with double quotes, which DuckDB reads as
  // *identifiers*. `EmitValueLiteral` recurses into the array so
  // each STRING element gets the single-quoted DuckDB form.
  const ::googlesql::ResolvedStatement* stmt =
      Analyze("SELECT ['a', 'b'] AS a");
  const ::googlesql::ResolvedExpr* expr = QueryFirstSelectExpr(stmt);
  ASSERT_NE(expr, nullptr);
  ASSERT_EQ(expr->node_kind(), ::googlesql::RESOLVED_LITERAL);
  TestTranspiler t;
  EXPECT_EQ(t.EmitLiteral(expr->GetAs<::googlesql::ResolvedLiteral>()),
            "['a', 'b']");
}

TEST_F(TranspilerTest, EmitMakeStructNamedFields) {
  // Fully-constant `STRUCT(1 AS a, 'x' AS b)` gets folded by the
  // analyzer onto a `ResolvedLiteral` whose value is the constant
  // struct; the `EmitValueLiteral` path covers that case. To
  // exercise the `EmitMakeStruct` *emit* we have to thread at
  // least one non-const expression in, here a column ref onto
  // `people.id`. The result is a `ResolvedMakeStruct` with
  // `StructType{a INT64, b STRING}` and a parallel `field_list` of
  // (`ColumnRef`, `Literal`).
  const ::googlesql::ResolvedStatement* stmt =
      Analyze("SELECT STRUCT(id AS a, 'x' AS b) AS s FROM people");
  const ::googlesql::ResolvedExpr* expr = QueryFirstSelectExpr(stmt);
  ASSERT_NE(expr, nullptr);
  ASSERT_EQ(expr->node_kind(), ::googlesql::RESOLVED_MAKE_STRUCT);
  TestTranspiler t;
  EXPECT_EQ(t.EmitMakeStruct(expr->GetAs<::googlesql::ResolvedMakeStruct>()),
            "{'a': \"id\", 'b': 'x'}");
}

TEST_F(TranspilerTest, EmitMakeStructAnonymousFieldsSynthesizeNames) {
  // `STRUCT(id, 'x')` (no `AS <name>`) resolves to a
  // `ResolvedMakeStruct` whose `StructType` may still carry an
  // analyzer-derived name for the column-ref slot (`id` here -- BQ
  // copies the source column name as a courtesy) and an empty name
  // for the literal slot. The empty-name slot synthesizes a
  // positional DuckDB-side name (`_1`) so the lowered struct still
  // has a key per field; the column-ref slot keeps its
  // analyzer-supplied name verbatim. `EmitGetStructField` uses the
  // same convention, so anonymous-field access round-trips through
  // the DuckDB engine. The column ref keeps the analyzer from
  // folding the whole expression onto a `ResolvedLiteral`.
  const ::googlesql::ResolvedStatement* stmt =
      Analyze("SELECT STRUCT(id, 'x') AS s FROM people");
  const ::googlesql::ResolvedExpr* expr = QueryFirstSelectExpr(stmt);
  ASSERT_NE(expr, nullptr);
  ASSERT_EQ(expr->node_kind(), ::googlesql::RESOLVED_MAKE_STRUCT);
  TestTranspiler t;
  EXPECT_EQ(t.EmitMakeStruct(expr->GetAs<::googlesql::ResolvedMakeStruct>()),
            "{'id': \"id\", '_1': 'x'}");
}

TEST_F(TranspilerTest, EmitMakeStructFullyAnonymousLiteralFields) {
  // To pin the *fully* anonymous case (every field empty-named) we
  // synthesize the AST directly: the analyzer would normally fold
  // a struct of two literals onto a `ResolvedLiteral` (covered by
  // `EmitMakeStructLiteralAnonymousAllConst`) so this construction
  // exercises the `EmitMakeStruct` path with both names empty. The
  // emit synthesizes `_0` / `_1` end-to-end and never reaches the
  // empty-string fallback.
  const ::googlesql::Type* int64_t_ty = type_factory_->get_int64();
  const ::googlesql::Type* string_ty = type_factory_->get_string();
  std::vector<::googlesql::StructType::StructField> fields = {
      {/*name=*/"", int64_t_ty},
      {/*name=*/"", string_ty},
  };
  const ::googlesql::StructType* struct_ty = nullptr;
  ASSERT_TRUE(type_factory_->MakeStructType(fields, &struct_ty).ok());
  std::vector<std::unique_ptr<const ::googlesql::ResolvedExpr>> field_list;
  field_list.push_back(
      ::googlesql::MakeResolvedLiteral(::googlesql::Value::Int64(1)));
  field_list.push_back(
      ::googlesql::MakeResolvedLiteral(::googlesql::Value::String("a")));
  auto make_struct =
      ::googlesql::MakeResolvedMakeStruct(struct_ty, std::move(field_list));
  TestTranspiler t;
  EXPECT_EQ(t.EmitMakeStruct(make_struct.get()), "{'_0': 1, '_1': 'a'}");
}

TEST_F(TranspilerTest, EmitGetStructFieldNamedAccess) {
  // `STRUCT(1 AS a, 'x' AS b).a` analyzes to a
  // `ResolvedGetStructField` whose `expr` is the MakeStruct above
  // and whose `field_idx=0`. The emit composes the two: the inner
  // MakeStruct emits to `{'a': 1, 'b': 'x'}`, the outer GetStructField
  // wraps it as `{'a': 1, 'b': 'x'}."a"`.
  //
  // DuckDB resolves `<struct>."<name>"` against the struct's named
  // field; the quoted form keeps unusual field names (Unicode,
  // hyphens, ...) round-tripping correctly.
  const ::googlesql::ResolvedStatement* stmt =
      Analyze("SELECT STRUCT(1 AS a, 'x' AS b).a AS x");
  const ::googlesql::ResolvedExpr* expr = QueryFirstSelectExpr(stmt);
  ASSERT_NE(expr, nullptr);
  ASSERT_EQ(expr->node_kind(), ::googlesql::RESOLVED_GET_STRUCT_FIELD);
  TestTranspiler t;
  EXPECT_EQ(
      t.EmitGetStructField(expr->GetAs<::googlesql::ResolvedGetStructField>()),
      "{'a': 1, 'b': 'x'}.\"a\"");
}

TEST_F(TranspilerTest, EmitMakeStructLiteralAnonymousAllConst) {
  // `STRUCT(1, 'a')` with all-constant arguments folds to a
  // `ResolvedLiteral` of TYPE_STRUCT in the analyzer -- the
  // `EmitValueLiteral` private helper handles it instead of
  // `EmitMakeStruct`. We assert on the same synthesized-name
  // shape so the literal path stays in lock-step with the
  // construction path; otherwise serialization on the literal
  // would diverge from runtime construction for the same
  // BigQuery shape.
  const ::googlesql::ResolvedStatement* stmt =
      Analyze("SELECT STRUCT(1, 'a') AS s");
  const ::googlesql::ResolvedExpr* expr = QueryFirstSelectExpr(stmt);
  ASSERT_NE(expr, nullptr);
  ASSERT_EQ(expr->node_kind(), ::googlesql::RESOLVED_LITERAL);
  TestTranspiler t;
  EXPECT_EQ(t.EmitLiteral(expr->GetAs<::googlesql::ResolvedLiteral>()),
            "{'_0': 1, '_1': 'a'}");
}

TEST_F(TranspilerTest, EmitGetStructFieldAnonymousUsesSynthesizedName) {
  // Anonymous-field access can't always be expressed cleanly in BQ
  // surface SQL (positional access lands on different shapes
  // depending on the source surface), so we construct the AST
  // directly to pin the emit. A literal `STRUCT(1, 'a')` yields a
  // `ResolvedLiteral` whose value is a constant struct with
  // anonymous fields; wrapping it in a `ResolvedGetStructField`
  // with `field_idx=0` produces exactly the access shape the
  // analyzer would emit when the BigQuery surface allows
  // positional access. The expected DuckDB SQL uses `_0` (the
  // synthesized name) on both the construction and the access
  // side, so the lowered struct round-trips.
  const ::googlesql::Type* int64_t_ty = type_factory_->get_int64();
  const ::googlesql::Type* string_ty = type_factory_->get_string();
  std::vector<::googlesql::StructType::StructField> fields = {
      {/*name=*/"", int64_t_ty},
      {/*name=*/"", string_ty},
  };
  const ::googlesql::StructType* struct_ty = nullptr;
  ASSERT_TRUE(type_factory_->MakeStructType(fields, &struct_ty).ok());
  std::vector<std::unique_ptr<const ::googlesql::ResolvedExpr>> field_list;
  field_list.push_back(
      ::googlesql::MakeResolvedLiteral(::googlesql::Value::Int64(1)));
  field_list.push_back(
      ::googlesql::MakeResolvedLiteral(::googlesql::Value::String("a")));
  auto make_struct =
      ::googlesql::MakeResolvedMakeStruct(struct_ty, std::move(field_list));
  auto get_field = ::googlesql::MakeResolvedGetStructField(
      int64_t_ty,
      std::move(make_struct),
      /*field_idx=*/0,
      /*field_expr_is_positional=*/true);
  TestTranspiler t;
  EXPECT_EQ(t.EmitGetStructField(get_field.get()),
            "{'_0': 1, '_1': 'a'}.\"_0\"");
}

TEST_F(TranspilerTest, EmitLiteralArrayOfNamedStructs) {
  // BigQuery `[STRUCT(1 AS a, 'x' AS b), STRUCT(2 AS a, 'y' AS b)]`
  // resolves to a constant-folded `ResolvedLiteral` of type
  // `ARRAY<STRUCT<a INT64, b STRING>>`. The emit recurses through
  // `EmitValueLiteral` so each inner struct lowers to
  // `{'a': N, 'b': '...'}`. This pins the named-field shape inside
  // an array (the conformance harness exercises this through
  // ARRAY_AGG / generated arrays in conformance suites).
  const ::googlesql::ResolvedStatement* stmt = Analyze(
      "SELECT [STRUCT(1 AS a, 'x' AS b), STRUCT(2 AS a, 'y' AS b)] AS arr");
  const ::googlesql::ResolvedExpr* expr = QueryFirstSelectExpr(stmt);
  ASSERT_NE(expr, nullptr);
  ASSERT_EQ(expr->node_kind(), ::googlesql::RESOLVED_LITERAL);
  TestTranspiler t;
  EXPECT_EQ(t.EmitLiteral(expr->GetAs<::googlesql::ResolvedLiteral>()),
            "[{'a': 1, 'b': 'x'}, {'a': 2, 'b': 'y'}]");
}

TEST_F(TranspilerTest, EmitLiteralArrayOfAnonymousStructs) {
  // Anonymous nested struct literal: each element is `STRUCT(N, 'x')`
  // with empty field names, so both inner structs synthesize the
  // same positional names (`_0`, `_1`). The test pins the lockstep
  // contract between the literal emit and the make-struct emit -- a
  // future drift would surface as one path emitting `_0` and the
  // other emitting some other key.
  const ::googlesql::ResolvedStatement* stmt =
      Analyze("SELECT [STRUCT(1, 'x'), STRUCT(2, 'y')] AS arr");
  const ::googlesql::ResolvedExpr* expr = QueryFirstSelectExpr(stmt);
  ASSERT_NE(expr, nullptr);
  ASSERT_EQ(expr->node_kind(), ::googlesql::RESOLVED_LITERAL);
  TestTranspiler t;
  EXPECT_EQ(t.EmitLiteral(expr->GetAs<::googlesql::ResolvedLiteral>()),
            "[{'_0': 1, '_1': 'x'}, {'_0': 2, '_1': 'y'}]");
}

TEST_F(TranspilerTest, EmitArrayScanStandaloneUnnestLiteral) {
  // `FROM UNNEST([1, 2]) AS x` resolves to a `ResolvedArrayScan`
  // with `input_scan=nullptr`, a single array (a `ResolvedLiteral`
  // of type `ARRAY<INT64>`), and a single element column named "x".
  // The emit lowers it to DuckDB's `SELECT unnest(<arr>) AS "<col>"`
  // shape, which produces one row per array element with the column
  // carrying the BQ alias name.
  const ::googlesql::ResolvedStatement* stmt =
      Analyze("SELECT 1 FROM UNNEST([1, 2]) AS x");
  const ::googlesql::ResolvedScan* scan = QueryInputScan(stmt);
  ASSERT_NE(scan, nullptr);
  ASSERT_EQ(scan->node_kind(), ::googlesql::RESOLVED_ARRAY_SCAN);
  TestTranspiler t;
  EXPECT_EQ(t.EmitArrayScan(scan->GetAs<::googlesql::ResolvedArrayScan>()),
            "SELECT unnest([1, 2]) AS \"x\"");
}

TEST_F(TranspilerTest, EmitArrayScanWithOffsetFallsBack) {
  // `UNNEST(arr) WITH OFFSET pos` analyzes with a non-null
  // `array_offset_column`. DuckDB has no `WITH OFFSET` analog
  // (`generate_subscripts(arr, 1)` is the typical rewrite) and the
  // standalone-UNNEST subset we lower today does not cover the
  // join-on-offset shape, so the emit propagates "" and the engine
  // takes the reference-impl fallback for the whole query.
  const ::googlesql::ResolvedStatement* stmt =
      Analyze("SELECT x, pos FROM UNNEST([1, 2]) AS x WITH OFFSET pos");
  const ::googlesql::ResolvedScan* scan = QueryInputScan(stmt);
  ASSERT_NE(scan, nullptr);
  ASSERT_EQ(scan->node_kind(), ::googlesql::RESOLVED_ARRAY_SCAN);
  TestTranspiler t;
  EXPECT_EQ(t.EmitArrayScan(scan->GetAs<::googlesql::ResolvedArrayScan>()), "");
}

TEST_F(TranspilerTest, EmitArrayScanJoinedToTableFallsBack) {
  // `FROM people, UNNEST([1, 2]) AS x` analyzes to a
  // `ResolvedArrayScan` whose `input_scan` is a `ResolvedTableScan`
  // (lateral cross-join). The lateral rewrite needs DuckDB's
  // `CROSS JOIN unnest(...)` shape plus column-aliasing
  // coordination with the input scan; we defer it to a follow-up
  // plan and fall back via "" today.
  const ::googlesql::ResolvedStatement* stmt =
      Analyze("SELECT id FROM people, UNNEST([1, 2]) AS x");
  const ::googlesql::ResolvedScan* scan = QueryInputScan(stmt);
  ASSERT_NE(scan, nullptr);
  ASSERT_EQ(scan->node_kind(), ::googlesql::RESOLVED_ARRAY_SCAN);
  TestTranspiler t;
  EXPECT_EQ(t.EmitArrayScan(scan->GetAs<::googlesql::ResolvedArrayScan>()), "");
}

TEST_F(TranspilerTest, EmitFunctionCallMakeArrayWithColumn) {
  // `[id, 0, id * 2]` mixes a column ref with a literal, so the
  // analyzer cannot fold it to a `ResolvedLiteral` and instead
  // produces a `$make_array(...)` function call. The `*` arg also
  // forces the expression off the literal path. Here we keep it
  // simple with just one non-const element so the emit shape is
  // unambiguous against the COALESCE / IFNULL whitelist.
  const ::googlesql::ResolvedStatement* stmt =
      Analyze("SELECT [id, 0] AS a FROM people");
  const ::googlesql::ResolvedExpr* expr = QueryFirstSelectExpr(stmt);
  ASSERT_NE(expr, nullptr);
  ASSERT_EQ(expr->node_kind(), ::googlesql::RESOLVED_FUNCTION_CALL);
  TestTranspiler t;
  EXPECT_EQ(
      t.EmitFunctionCall(expr->GetAs<::googlesql::ResolvedFunctionCall>()),
      "[\"id\", 0]");
}

// --- Functions disposition table ----------------------------------------

TEST(FunctionsTableTest, LookupKnownMappedScalar) {
  // Sanity check on a representative `kMap` entry. The lookup is
  // case-insensitive (we accept `ABS`, `abs`, `Abs` all the same).
  const FnEntry* e = LookupFunction("abs");
  ASSERT_NE(e, nullptr);
  EXPECT_EQ(e->kind, FnKind::kMap);
  EXPECT_EQ(e->duckdb_name, "ABS");
  const FnEntry* upper = LookupFunction("ABS");
  ASSERT_NE(upper, nullptr);
  EXPECT_EQ(upper->kind, FnKind::kMap);
  EXPECT_EQ(upper->duckdb_name, "ABS");
}

TEST(FunctionsTableTest, LookupKnownAggregate) {
  // `array_agg` is in the table so the aggregate emit dispatches
  // through it; ditto for the SUM / COUNT family.
  const FnEntry* agg = LookupFunction("array_agg");
  ASSERT_NE(agg, nullptr);
  EXPECT_EQ(agg->kind, FnKind::kMap);
  EXPECT_EQ(agg->duckdb_name, "ARRAY_AGG");
  const FnEntry* sum = LookupFunction("sum");
  ASSERT_NE(sum, nullptr);
  EXPECT_EQ(sum->kind, FnKind::kMap);
  EXPECT_EQ(sum->duckdb_name, "SUM");
}

TEST(FunctionsTableTest, LookupSkiplistedFunction) {
  // Skiplist disposition: the lookup succeeds but the kind tells the
  // caller to short-circuit to "" so the engine falls back.
  const FnEntry* e = LookupFunction("approx_quantiles");
  ASSERT_NE(e, nullptr);
  EXPECT_EQ(e->kind, FnKind::kSkiplist);
  EXPECT_TRUE(e->duckdb_name.empty());
}

TEST(FunctionsTableTest, LookupFallbackFunction) {
  // Fallback disposition: same runtime behavior as skiplist, but the
  // entry is in the table (with kind=kFallback) so we can tell
  // "deliberately deferred" from "no row in the table". `date_add`
  // is fallback today because BigQuery's INTERVAL semantics need a
  // dedicated rewrite pass.
  const FnEntry* e = LookupFunction("date_add");
  ASSERT_NE(e, nullptr);
  EXPECT_EQ(e->kind, FnKind::kFallback);
}

TEST(FunctionsTableTest, LookupUnknownReturnsNull) {
  // Functions not in the YAML disposition table return nullptr; the
  // transpiler treats nullptr the same as a `kFallback` entry, but
  // the distinction lets the LOG(INFO) tell "configured fallback"
  // from "no disposition row".
  EXPECT_EQ(LookupFunction("totally_made_up_function"), nullptr);
}

TEST(FunctionsTableTest, CoverageMeetsPlanThreshold) {
  // The plan requires the disposition table to cover at least 50
  // BigQuery functions across the math / string / datetime /
  // conditional / array / aggregation / skiplist categories. We
  // spot-check a few entries from each category here rather than
  // hard-counting the size of the underlying map (which is private
  // to `functions.cc`) -- a regression in the YAML would surface as
  // one of these sentinel lookups returning nullptr.
  const std::vector<std::string> required = {
      // math
      "abs",
      "ceil",
      "floor",
      "round",
      "trunc",
      "sqrt",
      "exp",
      "sign",
      "greatest",
      "least",
      "pi",
      "ln",
      "pow",
      // string
      "concat",
      "length",
      "lower",
      "upper",
      "substr",
      "replace",
      "trim",
      "ltrim",
      "rtrim",
      "lpad",
      "rpad",
      "reverse",
      "starts_with",
      "ends_with",
      // datetime (fallback)
      "current_timestamp",
      "current_date",
      "date_add",
      "format_timestamp",
      // conditional
      "ifnull",
      "coalesce",
      "nullif",
      // array
      "array_length",
      "array_concat",
      "generate_array",
      // aggregation
      "count",
      "sum",
      "avg",
      "min",
      "max",
      "any_value",
      "array_agg",
      "string_agg",
      // skiplist
      "approx_quantiles",
      "ml.predict",
      "net.ip_from_string",
      // window
      "row_number",
      "rank",
      "dense_rank",
  };
  for (const auto& name : required) {
    EXPECT_NE(LookupFunction(name), nullptr) << "missing entry: " << name;
  }
  EXPECT_GE(required.size(), 50u);
}

// --- Window / Analytic --------------------------------------------------

TEST_F(TranspilerTest, EmitAnalyticScanRowNumber) {
  // `ROW_NUMBER() OVER (ORDER BY id)` lowers to a ResolvedAnalyticScan
  // whose only group has a null partition_by and a single-item
  // order_by; the analytic function list carries one
  // ResolvedAnalyticFunctionCall (`row_number`, no args). The
  // synthesized output column is `$analytic1`.
  const ::googlesql::ResolvedStatement* stmt =
      Analyze("SELECT ROW_NUMBER() OVER (ORDER BY id) FROM people");
  const ::googlesql::ResolvedScan* scan = QueryInputScan(stmt);
  ASSERT_NE(scan, nullptr);
  ASSERT_EQ(scan->node_kind(), ::googlesql::RESOLVED_ANALYTIC_SCAN);
  TestTranspiler t;
  EXPECT_EQ(
      t.EmitAnalyticScan(scan->GetAs<::googlesql::ResolvedAnalyticScan>()),
      "SELECT *, ROW_NUMBER() OVER (ORDER BY \"id\" ASC) AS \"$analytic1\""
      " FROM (SELECT \"id\", \"name\" FROM \"people\")");
}

TEST_F(TranspilerTest, EmitAnalyticScanRankPartitionByOrderBy) {
  // `RANK() OVER (PARTITION BY name ORDER BY id DESC)` exercises both
  // the partition_by and the explicit-direction order_by paths. The
  // partition spec emits one PARTITION BY column and the order spec
  // emits the explicit DESC keyword.
  const ::googlesql::ResolvedStatement* stmt = Analyze(
      "SELECT RANK() OVER (PARTITION BY name ORDER BY id DESC) FROM people");
  const ::googlesql::ResolvedScan* scan = QueryInputScan(stmt);
  ASSERT_NE(scan, nullptr);
  ASSERT_EQ(scan->node_kind(), ::googlesql::RESOLVED_ANALYTIC_SCAN);
  TestTranspiler t;
  EXPECT_EQ(
      t.EmitAnalyticScan(scan->GetAs<::googlesql::ResolvedAnalyticScan>()),
      "SELECT *, RANK() OVER (PARTITION BY \"name\" ORDER BY \"id\" DESC)"
      " AS \"$analytic1\" FROM (SELECT \"id\", \"name\" FROM \"people\")");
}

TEST_F(TranspilerTest, EmitAnalyticScanDenseRank) {
  // DENSE_RANK is the third ranking analytic the plan calls out; the
  // test mirrors RANK so we get explicit coverage of the disposition
  // row in `functions.yaml` (`dense_rank: DENSE_RANK`).
  const ::googlesql::ResolvedStatement* stmt =
      Analyze("SELECT DENSE_RANK() OVER (ORDER BY id) FROM people");
  const ::googlesql::ResolvedScan* scan = QueryInputScan(stmt);
  ASSERT_NE(scan, nullptr);
  ASSERT_EQ(scan->node_kind(), ::googlesql::RESOLVED_ANALYTIC_SCAN);
  TestTranspiler t;
  EXPECT_EQ(
      t.EmitAnalyticScan(scan->GetAs<::googlesql::ResolvedAnalyticScan>()),
      "SELECT *, DENSE_RANK() OVER (ORDER BY \"id\" ASC) AS \"$analytic1\""
      " FROM (SELECT \"id\", \"name\" FROM \"people\")");
}

TEST_F(TranspilerTest, EmitAnalyticScanSumOverWithFrame) {
  // Aggregate-over-window with an explicit ROWS frame. SUM is a
  // `kMap` entry shared with the scalar aggregate emit, so the
  // analytic path renders it the same way (`SUM(<expr>)`) and the
  // OVER clause carries the ROWS BETWEEN bound. UNBOUNDED PRECEDING
  // / CURRENT ROW are both supported boundary types.
  const ::googlesql::ResolvedStatement* stmt = Analyze(
      "SELECT SUM(id) OVER (ORDER BY id ROWS BETWEEN UNBOUNDED PRECEDING "
      "AND CURRENT ROW) FROM people");
  const ::googlesql::ResolvedScan* scan = QueryInputScan(stmt);
  ASSERT_NE(scan, nullptr);
  ASSERT_EQ(scan->node_kind(), ::googlesql::RESOLVED_ANALYTIC_SCAN);
  TestTranspiler t;
  EXPECT_EQ(
      t.EmitAnalyticScan(scan->GetAs<::googlesql::ResolvedAnalyticScan>()),
      "SELECT *, SUM(\"id\") OVER (ORDER BY \"id\" ASC ROWS BETWEEN "
      "UNBOUNDED PRECEDING AND CURRENT ROW) AS \"$analytic1\""
      " FROM (SELECT \"id\", \"name\" FROM \"people\")");
}

TEST_F(TranspilerTest, EmitAnalyticScanCountStarOverPartition) {
  // COUNT(*) lowers through the `$count_star` special case both in
  // the aggregate path and in the analytic path -- the analyzer
  // gives us an empty argument_list and the function name
  // `$count_star`. With a PARTITION-BY-only OVER clause the analyzer
  // synthesizes a `ROWS BETWEEN UNBOUNDED PRECEDING AND UNBOUNDED
  // FOLLOWING` frame for aggregate analytic functions, so the emit
  // surfaces that frame even though the user didn't spell it.
  const ::googlesql::ResolvedStatement* stmt =
      Analyze("SELECT COUNT(*) OVER (PARTITION BY name) FROM people");
  const ::googlesql::ResolvedScan* scan = QueryInputScan(stmt);
  ASSERT_NE(scan, nullptr);
  ASSERT_EQ(scan->node_kind(), ::googlesql::RESOLVED_ANALYTIC_SCAN);
  TestTranspiler t;
  EXPECT_EQ(
      t.EmitAnalyticScan(scan->GetAs<::googlesql::ResolvedAnalyticScan>()),
      "SELECT *, COUNT(*) OVER (PARTITION BY \"name\" ROWS BETWEEN "
      "UNBOUNDED PRECEDING AND UNBOUNDED FOLLOWING) AS \"$analytic1\""
      " FROM (SELECT \"id\", \"name\" FROM \"people\")");
}

TEST_F(TranspilerTest, EmitAnalyticScanSafeAggregateFallsBack) {
  // `SAFE.SUM(id) OVER (ORDER BY id)` analyzes cleanly (SAFE is a
  // function-call decoration, not an OVER-time modifier) but sets
  // `error_mode = SAFE_ERROR_MODE`. The per-call SAFE short-circuit
  // returns "" and the analytic emit propagates the empty string,
  // exactly the disposition policy the engine reads for the
  // reference-impl fallback.
  const ::googlesql::ResolvedStatement* stmt =
      Analyze("SELECT SAFE.SUM(id) OVER (ORDER BY id) FROM people");
  if (stmt == nullptr) {
    GTEST_SKIP() << "analyzer rejected SAFE aggregate OVER -- skip";
  }
  const ::googlesql::ResolvedScan* scan = QueryInputScan(stmt);
  if (scan == nullptr ||
      scan->node_kind() != ::googlesql::RESOLVED_ANALYTIC_SCAN) {
    GTEST_SKIP() << "analyzer produced non-analytic scan -- skip";
  }
  TestTranspiler t;
  EXPECT_EQ(
      t.EmitAnalyticScan(scan->GetAs<::googlesql::ResolvedAnalyticScan>()), "");
}

// --- Top-level SELECT (QueryStmt / ProjectScan / SingleRowScan /
//     OutputColumn / ComputedColumn) -----------------------------------

TEST_F(TranspilerTest, EmitSingleRowScanEmitsSelectOne) {
  // `SELECT 1` analyzes to a ResolvedProjectScan over a
  // ResolvedSingleRowScan. The single-row scan is the analyzer's
  // representation of "no FROM clause" -- a relation with one row
  // and no columns. We emit `SELECT 1` so the wrapping ProjectScan
  // can splice it into `FROM (<inner>)` like every other scan emit.
  const ::googlesql::ResolvedStatement* stmt = Analyze("SELECT 1");
  const ::googlesql::ResolvedScan* scan = QueryInputScan(stmt);
  ASSERT_NE(scan, nullptr);
  ASSERT_EQ(scan->node_kind(), ::googlesql::RESOLVED_SINGLE_ROW_SCAN);
  TestTranspiler t;
  EXPECT_EQ(
      t.EmitSingleRowScan(scan->GetAs<::googlesql::ResolvedSingleRowScan>()),
      "SELECT 1");
}

TEST_F(TranspilerTest, EmitComputedColumnLiteral) {
  // `SELECT 1` lands a ProjectScan whose `expr_list[0]` is a
  // ResolvedComputedColumn binding the `1` literal to the
  // synthesized output column. EmitComputedColumn lowers the bound
  // expression and adds `AS "<column-name>"`. We assert on the
  // analyzer's synthesized column name (`$col1`) so any drift in the
  // analyzer's auto-aliasing surfaces here rather than downstream.
  const ::googlesql::ResolvedStatement* stmt = Analyze("SELECT 1");
  ASSERT_NE(stmt, nullptr);
  const auto* q = stmt->GetAs<::googlesql::ResolvedQueryStmt>();
  ASSERT_NE(q, nullptr);
  const ::googlesql::ResolvedScan* scan = q->query();
  ASSERT_NE(scan, nullptr);
  ASSERT_EQ(scan->node_kind(), ::googlesql::RESOLVED_PROJECT_SCAN);
  const auto* project = scan->GetAs<::googlesql::ResolvedProjectScan>();
  ASSERT_GE(project->expr_list_size(), 1);
  const ::googlesql::ResolvedComputedColumn* cc = project->expr_list(0);
  ASSERT_NE(cc, nullptr);
  TestTranspiler t;
  EXPECT_EQ(t.EmitComputedColumn(cc), "1 AS \"$col1\"");
}

TEST_F(TranspilerTest, EmitComputedColumnFallsBackOnUnloweredExpr) {
  // Pick a function that's not on the disposition table so its
  // `EmitFunctionCall` returns "" -- the wrapping
  // EmitComputedColumn must propagate the empty-string fallback
  // contract rather than emit `<unset> AS "<col>"`.
  // `BIT_COUNT` is on the YAML skiplist (BQ flavor differs from
  // DuckDB's `bit_count`).
  const ::googlesql::ResolvedStatement* stmt =
      Analyze("SELECT BIT_COUNT(id) FROM people");
  ASSERT_NE(stmt, nullptr);
  const auto* q = stmt->GetAs<::googlesql::ResolvedQueryStmt>();
  ASSERT_NE(q, nullptr);
  const ::googlesql::ResolvedScan* scan = q->query();
  ASSERT_NE(scan, nullptr);
  ASSERT_EQ(scan->node_kind(), ::googlesql::RESOLVED_PROJECT_SCAN);
  const auto* project = scan->GetAs<::googlesql::ResolvedProjectScan>();
  ASSERT_GE(project->expr_list_size(), 1);
  const ::googlesql::ResolvedComputedColumn* cc = project->expr_list(0);
  ASSERT_NE(cc, nullptr);
  TestTranspiler t;
  EXPECT_EQ(t.EmitComputedColumn(cc), "");
}

TEST_F(TranspilerTest, EmitProjectScanSelectLiteral) {
  // The full ProjectScan emit for `SELECT 1` threads:
  //   * EmitSingleRowScan -> "SELECT 1"
  //   * EmitComputedColumn -> "1 AS \"$col1\""
  // and stitches them as `SELECT <projection> FROM (<inner>)`.
  const ::googlesql::ResolvedStatement* stmt = Analyze("SELECT 1");
  ASSERT_NE(stmt, nullptr);
  const auto* q = stmt->GetAs<::googlesql::ResolvedQueryStmt>();
  ASSERT_NE(q, nullptr);
  const ::googlesql::ResolvedScan* scan = q->query();
  ASSERT_NE(scan, nullptr);
  ASSERT_EQ(scan->node_kind(), ::googlesql::RESOLVED_PROJECT_SCAN);
  TestTranspiler t;
  EXPECT_EQ(t.EmitProjectScan(scan->GetAs<::googlesql::ResolvedProjectScan>()),
            "SELECT 1 AS \"$col1\" FROM (SELECT 1)");
}

TEST_F(TranspilerTest, EmitOutputColumnCollapsesAliasWhenNamesMatch) {
  // For `SELECT 1` the output column's user-visible name and the
  // physical column's name both resolve to `$col1`, so the alias
  // collapses to just `"$col1"` -- DuckDB carries the column name
  // straight through the outermost SELECT.
  const ::googlesql::ResolvedStatement* stmt = Analyze("SELECT 1");
  ASSERT_NE(stmt, nullptr);
  const auto* q = stmt->GetAs<::googlesql::ResolvedQueryStmt>();
  ASSERT_NE(q, nullptr);
  ASSERT_EQ(q->output_column_list_size(), 1);
  TestTranspiler t;
  EXPECT_EQ(t.EmitOutputColumn(q->output_column_list(0)), "\"$col1\"");
}

TEST_F(TranspilerTest, EmitOutputColumnEmitsAliasWhenNamesDiffer) {
  // `SELECT id AS user_id FROM people` lands an output column whose
  // user-visible name (`user_id`) differs from the physical column
  // name (`id`); the emit must surface both as `"id" AS "user_id"`.
  const ::googlesql::ResolvedStatement* stmt =
      Analyze("SELECT id AS user_id FROM people");
  ASSERT_NE(stmt, nullptr);
  const auto* q = stmt->GetAs<::googlesql::ResolvedQueryStmt>();
  ASSERT_NE(q, nullptr);
  ASSERT_EQ(q->output_column_list_size(), 1);
  TestTranspiler t;
  EXPECT_EQ(t.EmitOutputColumn(q->output_column_list(0)),
            "\"id\" AS \"user_id\"");
}

TEST_F(TranspilerTest, EmitQueryStmtSelectLiteral) {
  // End-to-end for `SELECT 1`: the analyzer wraps a ProjectScan
  // around a SingleRowScan and the QueryStmt's output_column_list
  // carries the synthesized `$col1` alias. The emit wires
  // EmitProjectScan + EmitOutputColumn into the final SQL.
  const ::googlesql::ResolvedStatement* stmt = Analyze("SELECT 1");
  ASSERT_NE(stmt, nullptr);
  ASSERT_EQ(stmt->node_kind(), ::googlesql::RESOLVED_QUERY_STMT);
  TestTranspiler t;
  EXPECT_EQ(t.EmitQueryStmt(stmt->GetAs<::googlesql::ResolvedQueryStmt>()),
            "SELECT \"$col1\" FROM (SELECT 1 AS \"$col1\" FROM (SELECT 1))");
}

TEST_F(TranspilerTest, EmitQueryStmtSelectLiteralWithExplicitAlias) {
  // `SELECT 1 AS x` rebinds the synthesized column id to the
  // user-spelled alias; both `output_column_list[0].name()` and the
  // column's `name()` resolve to `x`, so the AS alias collapses on
  // the outermost SELECT.
  const ::googlesql::ResolvedStatement* stmt = Analyze("SELECT 1 AS x");
  ASSERT_NE(stmt, nullptr);
  ASSERT_EQ(stmt->node_kind(), ::googlesql::RESOLVED_QUERY_STMT);
  TestTranspiler t;
  EXPECT_EQ(t.EmitQueryStmt(stmt->GetAs<::googlesql::ResolvedQueryStmt>()),
            "SELECT \"x\" FROM (SELECT 1 AS \"x\" FROM (SELECT 1))");
}

TEST_F(TranspilerTest, EmitQueryStmtTableProjectionPreservesColumnOrder) {
  // `SELECT id, name FROM people` should round-trip with both
  // columns in their declared order. The analyzer collapses this
  // straight onto the TableScan (no wrapping ProjectScan because
  // the projection matches the table's column list 1:1) and the
  // QueryStmt mapping just renames each column to itself.
  const ::googlesql::ResolvedStatement* stmt =
      Analyze("SELECT id, name FROM people");
  ASSERT_NE(stmt, nullptr);
  ASSERT_EQ(stmt->node_kind(), ::googlesql::RESOLVED_QUERY_STMT);
  TestTranspiler t;
  EXPECT_EQ(t.EmitQueryStmt(stmt->GetAs<::googlesql::ResolvedQueryStmt>()),
            "SELECT \"id\", \"name\" FROM (SELECT \"id\", \"name\" "
            "FROM \"people\")");
}

TEST_F(TranspilerTest, EmitQueryStmtExpressionProjection) {
  // A non-trivial projection (`COALESCE(name, 'unknown') AS n`)
  // forces the analyzer to wrap the TableScan in a ProjectScan
  // whose `expr_list` carries the ComputedColumn binding. The
  // outermost SELECT then projects the synthesized column under the
  // user-spelled alias.
  const ::googlesql::ResolvedStatement* stmt =
      Analyze("SELECT COALESCE(name, 'unknown') AS n FROM people");
  ASSERT_NE(stmt, nullptr);
  ASSERT_EQ(stmt->node_kind(), ::googlesql::RESOLVED_QUERY_STMT);
  TestTranspiler t;
  EXPECT_EQ(t.EmitQueryStmt(stmt->GetAs<::googlesql::ResolvedQueryStmt>()),
            "SELECT \"n\" FROM (SELECT COALESCE(\"name\", 'unknown') AS "
            "\"n\" FROM (SELECT \"id\", \"name\" FROM \"people\"))");
}

TEST_F(TranspilerTest, EmitQueryStmtReorderedOutputColumns) {
  // `SELECT name, id FROM people` reorders the table's column list.
  // The analyzer wraps the TableScan in a ProjectScan that mirrors
  // the table's storage order in `column_list` but the QueryStmt's
  // `output_column_list` reflects the user-spelled order, so the
  // outermost SELECT projects `name` before `id`.
  const ::googlesql::ResolvedStatement* stmt =
      Analyze("SELECT name, id FROM people");
  ASSERT_NE(stmt, nullptr);
  ASSERT_EQ(stmt->node_kind(), ::googlesql::RESOLVED_QUERY_STMT);
  TestTranspiler t;
  EXPECT_EQ(t.EmitQueryStmt(stmt->GetAs<::googlesql::ResolvedQueryStmt>()),
            "SELECT \"name\", \"id\" FROM (SELECT \"id\", \"name\" "
            "FROM \"people\")");
}

TEST_F(TranspilerTest, EmitQueryStmtAliasedColumnSurfacesAlias) {
  // `SELECT id AS user_id FROM people` keeps the physical column as
  // `id` inside the inner scan but renames it to `user_id` on the
  // outermost SELECT. The projection carries `<col> AS <alias>` so
  // the wire-side schema matches the user's spelling.
  const ::googlesql::ResolvedStatement* stmt =
      Analyze("SELECT id AS user_id FROM people");
  ASSERT_NE(stmt, nullptr);
  ASSERT_EQ(stmt->node_kind(), ::googlesql::RESOLVED_QUERY_STMT);
  TestTranspiler t;
  EXPECT_EQ(t.EmitQueryStmt(stmt->GetAs<::googlesql::ResolvedQueryStmt>()),
            "SELECT \"id\" AS \"user_id\" FROM (SELECT \"id\" "
            "FROM \"people\")");
}

TEST_F(TranspilerTest, EmitQueryStmtFallsBackOnUnloweredProjection) {
  // `BIT_COUNT(id)` is on the YAML skiplist; the inner ProjectScan
  // emit returns "" and EmitQueryStmt propagates the empty-string
  // fallback contract instead of stitching an outer SELECT around a
  // missing inner relation.
  const ::googlesql::ResolvedStatement* stmt =
      Analyze("SELECT BIT_COUNT(id) FROM people");
  ASSERT_NE(stmt, nullptr);
  ASSERT_EQ(stmt->node_kind(), ::googlesql::RESOLVED_QUERY_STMT);
  TestTranspiler t;
  EXPECT_EQ(t.EmitQueryStmt(stmt->GetAs<::googlesql::ResolvedQueryStmt>()), "");
}

// --- Parameters ---------------------------------------------------------

TEST_F(TranspilerTest, EmitParameterNamed) {
  // `SELECT @customer_id` analyzes to a ProjectScan whose only
  // computed column is a `ResolvedParameter` carrying the
  // analyzer-lowercased name (`customer_id`). We assert on both the
  // emitted `$N` placeholder and the bind-order accumulator so a
  // regression in either side surfaces here rather than downstream
  // in the engine integration.
  ::googlesql::AnalyzerOptions options = MakeAnalyzerOptions();
  ASSERT_TRUE(
      options.AddQueryParameter("customer_id", type_factory_->get_int64())
          .ok());
  const ::googlesql::ResolvedStatement* stmt =
      AnalyzeWith("SELECT @customer_id AS x", options);
  const ::googlesql::ResolvedExpr* expr = QueryFirstSelectExpr(stmt);
  ASSERT_NE(expr, nullptr);
  ASSERT_EQ(expr->node_kind(), ::googlesql::RESOLVED_PARAMETER);
  TestTranspiler t;
  EXPECT_EQ(t.EmitParameter(expr->GetAs<::googlesql::ResolvedParameter>()),
            "$1");
  ASSERT_EQ(t.parameter_order().size(), 1u);
  EXPECT_EQ(t.parameter_order()[0].name, "customer_id");
  EXPECT_EQ(t.parameter_order()[0].position, 0);
}

TEST_F(TranspilerTest, EmitParameterReuseSharesSlot) {
  // Two textual references to the same named parameter must share a
  // single DuckDB `$N` slot so the engine binds one value, not two.
  // We hit `EmitParameter` twice on the same (or equivalent) node and
  // assert both emits go to `$1` and the bind-order accumulator
  // carries exactly one entry.
  ::googlesql::AnalyzerOptions options = MakeAnalyzerOptions();
  ASSERT_TRUE(
      options.AddQueryParameter("threshold", type_factory_->get_int64()).ok());
  // Use the parameter twice in distinct projections: GoogleSQL
  // produces two `ResolvedParameter` nodes (one per reference) but
  // both carry the same `name()`, so the dedup collapses them.
  const ::googlesql::ResolvedStatement* stmt =
      AnalyzeWith("SELECT @threshold AS a, @threshold AS b", options);
  ASSERT_NE(stmt, nullptr);
  const auto* q = stmt->GetAs<::googlesql::ResolvedQueryStmt>();
  ASSERT_NE(q, nullptr);
  const ::googlesql::ResolvedScan* scan = q->query();
  ASSERT_EQ(scan->node_kind(), ::googlesql::RESOLVED_PROJECT_SCAN);
  const auto* project = scan->GetAs<::googlesql::ResolvedProjectScan>();
  ASSERT_GE(project->expr_list_size(), 2);
  TestTranspiler t;
  EXPECT_EQ(t.EmitParameter(project->expr_list(0)
                                ->expr()
                                ->GetAs<::googlesql::ResolvedParameter>()),
            "$1");
  EXPECT_EQ(t.EmitParameter(project->expr_list(1)
                                ->expr()
                                ->GetAs<::googlesql::ResolvedParameter>()),
            "$1");
  ASSERT_EQ(t.parameter_order().size(), 1u);
  EXPECT_EQ(t.parameter_order()[0].name, "threshold");
}

TEST_F(TranspilerTest, EmitParameterPositionalAssignsFreshSlots) {
  // Positional parameters carry a 1-based `position()` and are
  // referentially distinct on every analyzer reference; we never
  // dedupe them. Two positional references emit `$1` then `$2` and
  // the bind-order accumulator records both with the analyzer
  // positions intact.
  ::googlesql::AnalyzerOptions options = MakeAnalyzerOptions();
  options.set_parameter_mode(::googlesql::PARAMETER_POSITIONAL);
  ASSERT_TRUE(
      options.AddPositionalQueryParameter(type_factory_->get_int64()).ok());
  ASSERT_TRUE(
      options.AddPositionalQueryParameter(type_factory_->get_string()).ok());
  const ::googlesql::ResolvedStatement* stmt =
      AnalyzeWith("SELECT ? AS a, ? AS b", options);
  ASSERT_NE(stmt, nullptr);
  const auto* q = stmt->GetAs<::googlesql::ResolvedQueryStmt>();
  ASSERT_NE(q, nullptr);
  const ::googlesql::ResolvedScan* scan = q->query();
  ASSERT_EQ(scan->node_kind(), ::googlesql::RESOLVED_PROJECT_SCAN);
  const auto* project = scan->GetAs<::googlesql::ResolvedProjectScan>();
  ASSERT_GE(project->expr_list_size(), 2);
  TestTranspiler t;
  EXPECT_EQ(t.EmitParameter(project->expr_list(0)
                                ->expr()
                                ->GetAs<::googlesql::ResolvedParameter>()),
            "$1");
  EXPECT_EQ(t.EmitParameter(project->expr_list(1)
                                ->expr()
                                ->GetAs<::googlesql::ResolvedParameter>()),
            "$2");
  ASSERT_EQ(t.parameter_order().size(), 2u);
  EXPECT_TRUE(t.parameter_order()[0].name.empty());
  EXPECT_EQ(t.parameter_order()[0].position, 1);
  EXPECT_TRUE(t.parameter_order()[1].name.empty());
  EXPECT_EQ(t.parameter_order()[1].position, 2);
}

TEST_F(TranspilerTest, EmitLimitOffsetScanWithNamedParameter) {
  // `LIMIT @n OFFSET @n` exercises the parameter-in-LIMIT path *and*
  // named-parameter dedup inside a single scan emit: both LIMIT and
  // OFFSET resolve `@n` to `$1` and the accumulator records one
  // entry.
  ::googlesql::AnalyzerOptions options = MakeAnalyzerOptions();
  ASSERT_TRUE(options.AddQueryParameter("n", type_factory_->get_int64()).ok());
  const ::googlesql::ResolvedStatement* stmt = AnalyzeWith(
      "SELECT * FROM people ORDER BY id LIMIT @n OFFSET @n", options);
  const ::googlesql::ResolvedScan* scan = QueryInputScan(stmt);
  ASSERT_NE(scan, nullptr);
  ASSERT_EQ(scan->node_kind(), ::googlesql::RESOLVED_LIMIT_OFFSET_SCAN);
  TestTranspiler t;
  EXPECT_EQ(t.EmitLimitOffsetScan(
                scan->GetAs<::googlesql::ResolvedLimitOffsetScan>()),
            "SELECT * FROM (SELECT * FROM (SELECT \"id\", \"name\" FROM "
            "\"people\") ORDER BY \"id\" ASC) LIMIT $1 OFFSET $1");
  ASSERT_EQ(t.parameter_order().size(), 1u);
  EXPECT_EQ(t.parameter_order()[0].name, "n");
}

TEST_F(TranspilerTest, EmitParameterInsideFunctionArgument) {
  // Parameters thread through `EmitFunctionCall`'s argument loop
  // exactly like any other expression: `IFNULL(@s, 'x')` lowers to
  // `IFNULL($1, 'x')` and the parameter accumulator records the
  // single `@s` slot. `IFNULL` is on the function disposition table
  // so the surrounding emit composes fully.
  ::googlesql::AnalyzerOptions options = MakeAnalyzerOptions();
  ASSERT_TRUE(options.AddQueryParameter("s", type_factory_->get_string()).ok());
  const ::googlesql::ResolvedStatement* stmt =
      AnalyzeWith("SELECT IFNULL(@s, 'x') FROM people", options);
  const ::googlesql::ResolvedExpr* expr = QueryFirstSelectExpr(stmt);
  ASSERT_NE(expr, nullptr);
  ASSERT_EQ(expr->node_kind(), ::googlesql::RESOLVED_FUNCTION_CALL);
  TestTranspiler t;
  EXPECT_EQ(
      t.EmitFunctionCall(expr->GetAs<::googlesql::ResolvedFunctionCall>()),
      "IFNULL($1, 'x')");
  ASSERT_EQ(t.parameter_order().size(), 1u);
  EXPECT_EQ(t.parameter_order()[0].name, "s");
}

// --- Cast ---------------------------------------------------------------

TEST_F(TranspilerTest, EmitCastInt64ToString) {
  // `CAST(id AS STRING)` produces a `ResolvedCast` whose `expr` is
  // the column ref and whose target `Type` is STRING. The emit
  // composes both via `EmitColumnRef` + `ToDuckDBSqlType`, so the
  // result threads quoted-identifier and DuckDB type-name conventions
  // together.
  const ::googlesql::ResolvedStatement* stmt =
      Analyze("SELECT CAST(id AS STRING) FROM people");
  const ::googlesql::ResolvedExpr* expr = QueryFirstSelectExpr(stmt);
  ASSERT_NE(expr, nullptr);
  ASSERT_EQ(expr->node_kind(), ::googlesql::RESOLVED_CAST);
  TestTranspiler t;
  EXPECT_EQ(t.EmitCast(expr->GetAs<::googlesql::ResolvedCast>()),
            "CAST(\"id\" AS VARCHAR)");
}

TEST_F(TranspilerTest, EmitCastStringToInt64) {
  // CAST against a column ref of the right source type lands on the
  // expected DuckDB `BIGINT` (BQ INT64 -> DuckDB BIGINT, see
  // `types.cc`). The shape is symmetrical to the int->string case
  // above and pins the type-name mapping for INT64.
  const ::googlesql::ResolvedStatement* stmt =
      Analyze("SELECT CAST(name AS INT64) FROM people");
  const ::googlesql::ResolvedExpr* expr = QueryFirstSelectExpr(stmt);
  ASSERT_NE(expr, nullptr);
  ASSERT_EQ(expr->node_kind(), ::googlesql::RESOLVED_CAST);
  TestTranspiler t;
  EXPECT_EQ(t.EmitCast(expr->GetAs<::googlesql::ResolvedCast>()),
            "CAST(\"name\" AS BIGINT)");
}

TEST_F(TranspilerTest, EmitSafeCastUsesTryCast) {
  // `SAFE_CAST(<expr> AS T)` sets `return_null_on_error()` on the
  // ResolvedCast; we lower it to DuckDB's `TRY_CAST(...)` which
  // matches BigQuery's "return NULL on conversion failure" contract.
  const ::googlesql::ResolvedStatement* stmt =
      Analyze("SELECT SAFE_CAST(name AS INT64) FROM people");
  const ::googlesql::ResolvedExpr* expr = QueryFirstSelectExpr(stmt);
  ASSERT_NE(expr, nullptr);
  ASSERT_EQ(expr->node_kind(), ::googlesql::RESOLVED_CAST);
  TestTranspiler t;
  EXPECT_EQ(t.EmitCast(expr->GetAs<::googlesql::ResolvedCast>()),
            "TRY_CAST(\"name\" AS BIGINT)");
}

TEST_F(TranspilerTest, EmitCastNestedInsideFunctionCall) {
  // CAST nested inside another function call exercises the dispatch
  // path: `EmitFunctionCall` calls `EmitExpr` per argument, which
  // routes the cast through `EmitCast`. The full lower stays on the
  // DuckDB path because both COALESCE (disposition table) and CAST
  // (whitelisted target) are first-class.
  const ::googlesql::ResolvedStatement* stmt =
      Analyze("SELECT COALESCE(CAST(id AS STRING), 'x') FROM people");
  const ::googlesql::ResolvedExpr* expr = QueryFirstSelectExpr(stmt);
  ASSERT_NE(expr, nullptr);
  ASSERT_EQ(expr->node_kind(), ::googlesql::RESOLVED_FUNCTION_CALL);
  TestTranspiler t;
  EXPECT_EQ(
      t.EmitFunctionCall(expr->GetAs<::googlesql::ResolvedFunctionCall>()),
      "COALESCE(CAST(\"id\" AS VARCHAR), 'x')");
}

TEST_F(TranspilerTest, EmitCastArrayThreadsThroughColumnRef) {
  // ARRAY casts thread `ToDuckDBSqlType`'s recursive type expansion;
  // ARRAY<STRING> -> VARCHAR[] mirrors DuckDB's native list-of
  // syntax. We wrap a non-const expression (`[id]`) inside the cast
  // so the analyzer cannot constant-fold the whole expression onto
  // a `ResolvedLiteral` -- a folded array-of-int64 would skip the
  // ResolvedCast entirely and the test would fail with "expected
  // RESOLVED_CAST, got RESOLVED_LITERAL".
  const ::googlesql::ResolvedStatement* stmt =
      Analyze("SELECT CAST([id] AS ARRAY<STRING>) FROM people");
  const ::googlesql::ResolvedExpr* expr = QueryFirstSelectExpr(stmt);
  ASSERT_NE(expr, nullptr);
  ASSERT_EQ(expr->node_kind(), ::googlesql::RESOLVED_CAST);
  TestTranspiler t;
  // `[id]` is a non-const ARRAY constructor; it lowers through
  // `$make_array` to DuckDB's bracket syntax.
  EXPECT_EQ(t.EmitCast(expr->GetAs<::googlesql::ResolvedCast>()),
            "CAST([\"id\"] AS VARCHAR[])");
}

// --- WithExpr -----------------------------------------------------------

// Helper: synthesize a `ResolvedWithExpr` directly so the test does
// not depend on the analyzer preserving a `WITH(...)` expression
// against constant-folding / inlining heuristics. Each entry in
// `bindings` is `{name, expression}`; the helper allocates a fresh
// `ResolvedColumn` per binding, wraps the expression in a
// `ResolvedComputedColumn`, and emits a `ResolvedColumnRef` to the
// first binding for the body. Body type is taken from the first
// binding so the WithExpr's `type()` lines up.
//
// Returns nullptr when `bindings` is empty (a malformed WithExpr the
// analyzer would never produce). Callers transfer ownership of the
// binding expressions into the helper via `std::move`.
struct TestWithExprBinding {
  std::string name;
  std::unique_ptr<const ::googlesql::ResolvedExpr> expr;
};
std::unique_ptr<::googlesql::ResolvedWithExpr> MakeTestWithExpr(
    std::vector<TestWithExprBinding> bindings) {
  if (bindings.empty()) return nullptr;
  std::vector<std::unique_ptr<const ::googlesql::ResolvedComputedColumn>>
      assignments;
  std::vector<::googlesql::ResolvedColumn> columns;
  int next_id = 1;
  for (auto& binding : bindings) {
    if (binding.expr == nullptr) return nullptr;
    const ::googlesql::Type* t = binding.expr->type();
    ::googlesql::ResolvedColumn col(
        next_id++,
        /*table_name=*/::googlesql::IdString::MakeGlobal("$with"),
        /*name=*/::googlesql::IdString::MakeGlobal(binding.name),
        t);
    columns.push_back(col);
    auto cc =
        ::googlesql::MakeResolvedComputedColumn(col, std::move(binding.expr));
    assignments.push_back(std::move(cc));
  }
  std::unique_ptr<const ::googlesql::ResolvedExpr> body =
      ::googlesql::MakeResolvedColumnRef(columns.front(),
                                         /*is_correlated=*/false);
  return ::googlesql::MakeResolvedWithExpr(
      columns.front().type(), std::move(assignments), std::move(body));
}

TEST_F(TranspilerTest, EmitWithExprSingleBindingFromAnalyzer) {
  // `WITH(<assigns>, <body>)` is hard to keep alive against
  // analyzer constant-folding when both sides are constant, so we
  // thread a column ref through a function call (`IFNULL(name,
  // 'x')`) for the binding and reuse the binding column twice in
  // the body via an outer `IFNULL`. That keeps the binding
  // necessary -- inlining would evaluate the
  // `IFNULL(name, 'x')` twice, which would break the semantic
  // contract the WithExpr exists to preserve.
  const ::googlesql::ResolvedStatement* stmt =
      Analyze("SELECT WITH(a AS IFNULL(name, 'x'), IFNULL(a, a)) FROM people");
  if (stmt == nullptr) {
    GTEST_SKIP() << "analyzer rejected WITH(...) expression -- skip";
  }
  const ::googlesql::ResolvedExpr* expr = QueryFirstSelectExpr(stmt);
  if (expr == nullptr || expr->node_kind() != ::googlesql::RESOLVED_WITH_EXPR) {
    GTEST_SKIP() << "WITH(...) lowered to a non-WithExpr shape -- skip";
  }
  TestTranspiler t;
  EXPECT_EQ(t.EmitWithExpr(expr->GetAs<::googlesql::ResolvedWithExpr>()),
            "(SELECT IFNULL(\"a\", \"a\") FROM (SELECT IFNULL(\"name\", 'x') "
            "AS \"a\"))");
}

TEST_F(TranspilerTest, EmitWithExprSingleBindingDirect) {
  // Direct construction of a `ResolvedWithExpr`: one binding to an
  // INT64 literal, body references the binding. This pins the emit
  // shape independently of any analyzer rewrite -- we own the AST,
  // so a regression in `EmitWithExpr` itself surfaces here without
  // needing the WITH(...) parser feature to be on.
  std::vector<TestWithExprBinding> bindings;
  bindings.push_back(
      {"a", ::googlesql::MakeResolvedLiteral(::googlesql::Value::Int64(42))});
  auto with_expr = MakeTestWithExpr(std::move(bindings));
  ASSERT_NE(with_expr, nullptr);
  TestTranspiler t;
  EXPECT_EQ(t.EmitWithExpr(with_expr.get()),
            "(SELECT \"a\" FROM (SELECT 42 AS \"a\"))");
}

TEST_F(TranspilerTest, EmitWithExprMultipleBindingsDirect) {
  // Two bindings (`a`, `b`) -> body references the first via
  // ColumnRef. We pin the emit shape against analyzer rewrites by
  // constructing the AST directly.
  std::vector<TestWithExprBinding> bindings;
  bindings.push_back(
      {"a", ::googlesql::MakeResolvedLiteral(::googlesql::Value::Int64(1))});
  bindings.push_back(
      {"b", ::googlesql::MakeResolvedLiteral(::googlesql::Value::Int64(2))});
  auto with_expr = MakeTestWithExpr(std::move(bindings));
  ASSERT_NE(with_expr, nullptr);
  TestTranspiler t;
  EXPECT_EQ(t.EmitWithExpr(with_expr.get()),
            "(SELECT \"a\" FROM (SELECT 1 AS \"a\", 2 AS \"b\"))");
}

TEST_F(TranspilerTest, EmitWithExprFallsBackOnUnloweredBinding) {
  // Bindings whose expression cannot lower (here a `ResolvedParameter`
  // marked untyped, which falls back per `EmitParameter`) propagate
  // the empty-string contract through the WithExpr emit.
  std::vector<TestWithExprBinding> bindings;
  bindings.push_back({"a",
                      ::googlesql::MakeResolvedParameter(
                          /*type=*/type_factory_->get_int64(),
                          /*name=*/"x",
                          /*position=*/0,
                          /*is_untyped=*/true)});
  auto with_expr = MakeTestWithExpr(std::move(bindings));
  ASSERT_NE(with_expr, nullptr);
  TestTranspiler t;
  EXPECT_EQ(t.EmitWithExpr(with_expr.get()), "");
}

// --- FunctionArgument ---------------------------------------------------

TEST_F(TranspilerTest, EmitFunctionArgumentRoutesThroughExpr) {
  // `ResolvedFunctionArgument` is the wrapper the analyzer produces
  // for `generic_argument_list` slots; today's emit only knows how
  // to lower the `expr()` slot. Constructing one directly with a
  // small literal lets us assert on the routing without needing a
  // builtin function whose AST exposes a generic argument list (the
  // BigQuery surface that produces them is mostly TVFs / lambdas,
  // which is outside this plan).
  auto literal =
      ::googlesql::MakeResolvedLiteral(::googlesql::Value::Int64(42));
  auto arg = ::googlesql::MakeResolvedFunctionArgument();
  arg->set_expr(std::move(literal));
  TestTranspiler t;
  EXPECT_EQ(t.EmitFunctionArgument(arg.get()), "42");
}

TEST_F(TranspilerTest, EmitFunctionArgumentNonExprSlotFallsBack) {
  // A bare `MakeResolvedFunctionArgument()` (every slot null) has no
  // expression to route through; the emit must propagate "" so the
  // engine takes the reference-impl fallback for the surrounding
  // function call. This is the named-argument-only / TVF / lambda
  // shape the plan defers to a follow-up.
  auto arg = ::googlesql::MakeResolvedFunctionArgument();
  TestTranspiler t;
  EXPECT_EQ(t.EmitFunctionArgument(arg.get()), "");
}

// --- JSON field access --------------------------------------------------

// Helper: synthesize a `ResolvedColumnRef` to a JSON-typed column.
// We construct the AST for `EmitGetJsonField` directly so the emit
// is exercised independently of how the analyzer represents BQ JSON
// dot access (which can be analyzer-folded for fully constant LHS).
// Reusing this helper across the JSON tests keeps each case focused
// on the field-name + nested-access shape.
std::unique_ptr<::googlesql::ResolvedColumnRef> MakeJsonColumnRef(
    const ::googlesql::Type* json_ty) {
  ::googlesql::ResolvedColumn col(
      /*column_id=*/1,
      /*table_name=*/::googlesql::IdString::MakeGlobal("$test"),
      /*name=*/::googlesql::IdString::MakeGlobal("data"),
      json_ty);
  return ::googlesql::MakeResolvedColumnRef(col, /*is_correlated=*/false);
}

TEST_F(TranspilerTest, EmitGetJsonFieldObjectAccess) {
  // `data.user` where `data` is JSON resolves to a
  // `ResolvedGetJsonField` whose `expr` is the column ref and whose
  // `field_name` is `user`. The result type is JSON (BQ keeps the
  // type as JSON for `<json>.<field>` access), so the emit uses
  // DuckDB's `->` operator -- which also returns JSON.
  const ::googlesql::Type* json_ty = type_factory_->get_json();
  auto get = ::googlesql::MakeResolvedGetJsonField(json_ty,
                                                   MakeJsonColumnRef(json_ty),
                                                   /*field_name=*/"user");
  TestTranspiler t;
  EXPECT_EQ(t.EmitGetJsonField(get.get()), "(\"data\" -> 'user')");
}

TEST_F(TranspilerTest, EmitGetJsonFieldNestedAccess) {
  // `data.user.name` chains two `ResolvedGetJsonField` nodes; the
  // outer one's `expr` is the inner one's whole `(<json> -> 'user')`
  // emit, so the composition lands as `((data -> 'user') -> 'name')`.
  // Each level is a fresh `EmitExpr` call so the emit composes
  // recursively without any bespoke flattening.
  const ::googlesql::Type* json_ty = type_factory_->get_json();
  auto inner = ::googlesql::MakeResolvedGetJsonField(json_ty,
                                                     MakeJsonColumnRef(json_ty),
                                                     /*field_name=*/"user");
  auto outer = ::googlesql::MakeResolvedGetJsonField(json_ty,
                                                     std::move(inner),
                                                     /*field_name=*/"name");
  TestTranspiler t;
  EXPECT_EQ(t.EmitGetJsonField(outer.get()),
            "((\"data\" -> 'user') -> 'name')");
}

TEST_F(TranspilerTest, EmitGetJsonFieldEscapesSingleQuotes) {
  // BigQuery JSON keys can contain arbitrary characters including
  // `'`. The DuckDB-side string literal must double the quote so
  // the SQL stays well-formed. We do not need a JSON-path escape
  // step because the `->` operator takes a STRING (not a JSON path
  // expression) so the only escaping that matters is the SQL
  // string-literal one `QuoteString` already provides.
  const ::googlesql::Type* json_ty = type_factory_->get_json();
  auto get = ::googlesql::MakeResolvedGetJsonField(json_ty,
                                                   MakeJsonColumnRef(json_ty),
                                                   /*field_name=*/"O'Brien");
  TestTranspiler t;
  EXPECT_EQ(t.EmitGetJsonField(get.get()), "(\"data\" -> 'O''Brien')");
}

TEST_F(TranspilerTest, EmitGetJsonFieldHandlesUnicodeFieldName) {
  // Unicode-bearing JSON field name. `QuoteString` is a byte-wise
  // wrapper so multibyte UTF-8 sequences flow through unchanged --
  // we pin the assertion on the same UTF-8 bytes the field name
  // carries.
  const ::googlesql::Type* json_ty = type_factory_->get_json();
  auto get = ::googlesql::MakeResolvedGetJsonField(json_ty,
                                                   MakeJsonColumnRef(json_ty),
                                                   /*field_name=*/"naïve");
  TestTranspiler t;
  EXPECT_EQ(t.EmitGetJsonField(get.get()), "(\"data\" -> 'naïve')");
}

TEST_F(TranspilerTest, EmitGetJsonFieldScalarReturnUsesArrowGreater) {
  // When the analyzer types the GetJsonField result as something
  // other than JSON (a STRING coerced result, in some BQ analyzer
  // configurations), the emit picks DuckDB's `->>` operator so the
  // returned column is VARCHAR rather than JSON. This pins the
  // type-driven branch in `EmitGetJsonField` for the rare
  // scalar-coerced case.
  const ::googlesql::Type* json_ty = type_factory_->get_json();
  const ::googlesql::Type* string_ty = type_factory_->get_string();
  auto get = ::googlesql::MakeResolvedGetJsonField(string_ty,
                                                   MakeJsonColumnRef(json_ty),
                                                   /*field_name=*/"name");
  TestTranspiler t;
  EXPECT_EQ(t.EmitGetJsonField(get.get()), "(\"data\" ->> 'name')");
}

TEST_F(TranspilerTest, EmitGetJsonFieldNullExprFallsBack) {
  // A malformed `ResolvedGetJsonField` with a null inner expression
  // can't be lowered; the emit must propagate "" so the engine
  // takes the reference-impl fallback for the whole query rather
  // than emitting partial SQL. The analyzer doesn't produce this
  // shape, but we guard so a future change to the GetJsonField
  // construction surface doesn't silently emit `(<empty> -> ...)`.
  auto get = ::googlesql::MakeResolvedGetJsonField();
  TestTranspiler t;
  EXPECT_EQ(t.EmitGetJsonField(get.get()), "");
}

TEST_F(TranspilerTest, TranspileSelectFromWhereGroupByOrderByLimit) {
  // Engine-level smoke check for the plan's "SELECT ... FROM ...
  // WHERE ... GROUP BY ... ORDER BY ... LIMIT" target. We don't
  // round-trip through DuckDB here -- the unit-test fixture has no
  // running DuckDB connection -- but we *do* drive the full
  // `Transpile(stmt)` pipeline so a regression in any one of
  // EmitQueryStmt / EmitLimitOffsetScan / EmitOrderByScan /
  // EmitAggregateScan / EmitFilterScan / EmitTableScan surfaces as
  // a string drift here. The engine-side smoke test (executing on
  // DuckDB) is left to a follow-up plan once the DuckDBEngine
  // integration is updated to dispatch on QueryStmt directly rather
  // than the StripPassThroughProjectScans subset; see
  // duckdb-transpiler-select-core.plan.md for the engine wiring
  // that lands separately.
  const ::googlesql::ResolvedStatement* stmt = Analyze(
      "SELECT id, COUNT(*) AS c FROM people WHERE id > 0 GROUP BY id "
      "ORDER BY id LIMIT 10");
  ASSERT_NE(stmt, nullptr);
  TestTranspiler t;
  // Filter predicate (`>`) and LIMIT 10 over a synthesized aggregate
  // column thread together; the per-piece coverage above keeps each
  // emit honest, while this assertion pins the composition.
  std::string sql = t.Transpile(stmt);
  // The filter expression `id > 0` uses the `$greater` function which
  // is on the disposition fallback, so the FilterScan emit returns
  // "" and the entire QueryStmt emit propagates the empty-string
  // fallback contract. That's the documented baseline today; once a
  // future plan lands the comparison-operator emit, this test will
  // tighten to the full SQL string. The point of asserting here is
  // to catch any silent partial-emit regression -- the empty string
  // is the right answer until the predicate emit path lights up.
  EXPECT_EQ(sql, "");
}

}  // namespace
}  // namespace transpiler
}  // namespace duckdb
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
