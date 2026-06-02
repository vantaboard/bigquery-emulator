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
  // Match the engine: the route classifier sees raw
  // `ResolvedPivotScan` / `ResolvedUnpivotScan` nodes (the
  // `advanced-relational-routing` plan dispositions them as
  // `duckdb_rewrite`), not the analyzer's rewritten output.
  options.disable_rewrite(::googlesql::REWRITE_PIVOT);
  options.disable_rewrite(::googlesql::REWRITE_UNPIVOT);
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

    // Toy table for correlated array-scan shapes
    // (`FROM t, UNNEST(t.arr)`). One INT64 id plus an INT64 ARRAY
    // column the analyzer can resolve `t.arr` against.
    const ::googlesql::Type* int64_array_type = nullptr;
    ASSERT_TRUE(
        type_factory_
            ->MakeArrayType(type_factory_->get_int64(), &int64_array_type)
            .ok());
    auto arr_table = std::make_unique<::googlesql::SimpleTable>(
        "arr_table",
        std::vector<::googlesql::SimpleTable::NameAndType>{
            {"id", type_factory_->get_int64()},
            {"arr", int64_array_type},
        });
    catalog_->AddOwnedTable(std::move(arr_table));
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

TEST_F(RouteClassifierTest, SafeDivideRoutesToSemanticExecutor) {
  // `SAFE_DIVIDE`'s `functions.yaml` row dropped `status=planned`
  // in `edfb141` (the semantic-functions slice). The classifier
  // now promotes a SELECT containing `SAFE_DIVIDE` to the
  // semantic executor so the BigQuery-exact division-by-zero ->
  // NULL contract fires. The call is wrapped in
  // `SELECT ... FROM people` so the scalar-only promotion in
  // `VisitResolvedQueryStmt` does not also fire (we want to pin
  // the function-row contract, not the node-shape contract).
  const auto* stmt = Analyze("SELECT SAFE_DIVIDE(id, 0) FROM people");
  ASSERT_NE(stmt, nullptr);

  RouteDecision d = classifier_.Classify(*stmt);
  EXPECT_EQ(d.disposition, Disposition::kSemanticExecutor);
  EXPECT_EQ(d.offending_node, "function:safe_divide");
}

TEST_F(RouteClassifierTest, ControlOpStatementRoutesToControlOp) {
  // `ResolvedCreateTableStmt` is `disposition=control_op` (no
  // `status=planned`) in `node_dispositions.yaml`. Per
  // `control-op-executor.plan.md` the executor for that disposition
  // shipped, so the classifier promotes the root statement to
  // `kControlOp` and the coordinator dispatches DDL through
  // `backend/engine/control/control_op_executor.cc` -- not through
  // the DuckDB SQL evaluator.
  const auto* stmt = Analyze("CREATE TABLE new_table (a INT64, b STRING)");
  ASSERT_NE(stmt, nullptr);

  RouteDecision d = classifier_.Classify(*stmt);
  EXPECT_EQ(d.disposition, Disposition::kControlOp);
  EXPECT_EQ(d.offending_node, "ResolvedCreateTableStmt");
  EXPECT_NE(d.reason.find("control-op executor"), std::string::npos)
      << "reason should name the control-op executor; got: " << d.reason;
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

TEST_F(RouteClassifierTest, ScalarOnlySelectPromotesToSemanticExecutor) {
  // `SELECT 1` (and any other no-FROM SELECT) resolves to
  // `ResolvedQueryStmt(query=ResolvedProjectScan(input_scan=
  // ResolvedSingleRowScan))`. Per
  // `.cursor/plans/semantic-executor-core.plan.md` the classifier
  // promotes this shape to `kSemanticExecutor` at planning time
  // so the semantic executor's strict NULL / overflow / error
  // contracts handle the evaluation; the DuckDB fast path keeps
  // every shape that has a real FROM clause.
  const auto* stmt = Analyze("SELECT 1 + 2");
  ASSERT_NE(stmt, nullptr);
  RouteDecision d = classifier_.Classify(*stmt);
  EXPECT_EQ(d.disposition, Disposition::kSemanticExecutor);
  EXPECT_EQ(d.offending_node, "ResolvedQueryStmt(scalar-only SELECT)");
}

TEST_F(RouteClassifierTest, SelectWithFromStaysOnFastPath) {
  // The companion to the scalar-only promotion above: a SELECT
  // with a FROM clause keeps its `duckdb_native` route, so the
  // existing fast-path TableScan -> ProjectScan -> QueryStmt
  // surface continues to flow through DuckDB.
  const auto* stmt = Analyze("SELECT id FROM people");
  ASSERT_NE(stmt, nullptr);
  RouteDecision d = classifier_.Classify(*stmt);
  EXPECT_EQ(d.disposition, Disposition::kDuckdbNative);
}

TEST_F(RouteClassifierTest,
       ValueTableQueryStatementPromotesToSemanticExecutor) {
  // `SELECT AS VALUE STRUCT(1, 'a')` is BigQuery's value-table
  // surface: the row collapses to a single anonymous value. DuckDB
  // has no value-table row shape, so the classifier promotes the
  // route to `kSemanticExecutor` via the
  // `VisitResolvedQueryStmt(is_value_table=true)` override in
  // `route_classifier.cc`. Plan-2 owner is the semantic executor
  // (`semantic-executor-core.plan.md`); the stub returns
  // UNIMPLEMENTED today, which is the same end-user-visible
  // outcome as the prior empty-string gate inside `EmitQueryStmt`.
  const auto* stmt = Analyze("SELECT AS VALUE STRUCT(1 AS a, 'b' AS b)");
  ASSERT_NE(stmt, nullptr);

  RouteDecision d = classifier_.Classify(*stmt);
  EXPECT_EQ(d.disposition, Disposition::kSemanticExecutor);
  EXPECT_EQ(d.offending_node, "ResolvedQueryStmt(is_value_table=true)");
  EXPECT_NE(d.reason.find("semantic"), std::string::npos)
      << "reason should mention semantic-executor route; got: " << d.reason;
}

// NOTE on `ResolvedJoinScan(is_lateral=true)` coverage: the
// classifier carries a `VisitResolvedJoinScan` override that
// promotes the lateral join shape to `kSemanticExecutor` (see
// `route_classifier.cc`'s plan-2 additions). Triggering
// `is_lateral=true` from a SimpleCatalog-backed SQL query
// requires the `LATERAL` keyword plus a correlated subquery shape
// that a minimal in-process catalog cannot easily express:
// surface SQL forms like `JOIN UNNEST(<col>)` analyze to a
// `ResolvedArrayScan` (NOT a `ResolvedJoinScan`), so they exercise
// a different route-promotion path. The lateral-promotion route
// will be pinned by the integration suite owned by
// `array-struct-semantic-path.plan.md` once that plan ships its
// catalog. Until then, the override stays in place as
// defense-in-depth -- the only cost of the dead branch is the
// per-Visit dispatch and a `MaybePromote` bookkeeping call.

TEST_F(RouteClassifierTest, UnnestWithOffsetPromotesToSemanticExecutor) {
  // `UNNEST(<arr>) WITH OFFSET <name>` resolves to a
  // `ResolvedArrayScan` carrying a non-null `array_offset_column`.
  // The DuckDB fast path's standalone-UNNEST emit (`EmitArrayScan`)
  // returns "" for any `array_offset_column != nullptr` case, so
  // the classifier promotes the whole query to `kSemanticExecutor`
  // via the `VisitResolvedArrayScan` override. See
  // `.cursor/plans/array-struct-semantic-path.plan.md` Family 1.
  const auto* stmt =
      Analyze("SELECT n, idx FROM UNNEST([1, 2, 3]) AS n WITH OFFSET AS idx");
  ASSERT_NE(stmt, nullptr);
  RouteDecision d = classifier_.Classify(*stmt);
  EXPECT_EQ(d.disposition, Disposition::kSemanticExecutor);
  EXPECT_EQ(d.offending_node, "ResolvedArrayScan(array_offset_column)");
}

TEST_F(RouteClassifierTest, OuterUnnestPromotesToSemanticExecutor) {
  // `LEFT JOIN UNNEST(<arr>)` (the outer form) flips `is_outer` to
  // true; BigQuery emits one all-NULL row when the array is empty,
  // DuckDB drops the row. Classifier promotes the query to the
  // semantic executor so the empty-array NULL-row contract gets
  // honored. See `array-struct-semantic-path.plan.md` Family 2.
  const auto* stmt = Analyze(
      "SELECT id, n FROM arr_table LEFT JOIN UNNEST(arr_table.arr) AS n "
      "ON TRUE");
  ASSERT_NE(stmt, nullptr);
  RouteDecision d = classifier_.Classify(*stmt);
  EXPECT_EQ(d.disposition, Disposition::kSemanticExecutor);
  // The first promotion in the visitor walk wins -- either the
  // outer flag (this test's intent) or the join_expr (the inner
  // ArrayScan's `ON TRUE` clause). Either is correct; both pin
  // the same family.
  EXPECT_TRUE(d.offending_node == "ResolvedArrayScan(is_outer=true)" ||
              d.offending_node == "ResolvedArrayScan(join_expr)")
      << "got: " << d.offending_node;
}

TEST_F(RouteClassifierTest, MultiArrayUnnestZipPromotesToSemanticExecutor) {
  // `UNNEST(<a>, <b>)` is BigQuery's array-zip surface (multiway
  // UNNEST). The analyzer produces a `ResolvedArrayScan` with
  // `array_expr_list_size() > 1` and (when GoogleSQL's
  // FEATURE_MULTIWAY_UNNEST is enabled by
  // `EnableMaximumLanguageFeatures`) attaches an `array_zip_mode`
  // ENUM literal (PAD by default). DuckDB has no native array
  // zip, so the classifier promotes the query to
  // `kSemanticExecutor`. See
  // `array-struct-semantic-path.plan.md` Family 3.
  const auto* stmt = Analyze("SELECT * FROM UNNEST([1, 2, 3], [10, 20, 30])");
  ASSERT_NE(stmt, nullptr);
  RouteDecision d = classifier_.Classify(*stmt);
  EXPECT_EQ(d.disposition, Disposition::kSemanticExecutor);
  EXPECT_EQ(d.offending_node, "ResolvedArrayScan(array_zip_mode)");
}

TEST_F(RouteClassifierTest, CorrelatedArrayScanPromotesToSemanticExecutor) {
  // `FROM t, UNNEST(t.arr)` (the cross-join form) resolves to a
  // `ResolvedArrayScan` whose `input_scan` is the `t` TableScan
  // (not a SingleRowScan). The DuckDB fast path's standalone-UNNEST
  // emit returns "" for this shape; the classifier promotes the
  // query to `kSemanticExecutor`. See Family 4 of
  // `array-struct-semantic-path.plan.md`.
  const auto* stmt =
      Analyze("SELECT id, n FROM arr_table, UNNEST(arr_table.arr) AS n");
  ASSERT_NE(stmt, nullptr);
  RouteDecision d = classifier_.Classify(*stmt);
  EXPECT_EQ(d.disposition, Disposition::kSemanticExecutor);
  EXPECT_EQ(d.offending_node, "ResolvedArrayScan(correlated_input_scan)");
}

TEST_F(RouteClassifierTest, StandaloneUnnestStaysOnFastPath) {
  // Defense-in-depth: `SELECT n FROM UNNEST([1,2,3]) AS n` (no
  // offset / not outer / single array / no FROM-side input) MUST
  // stay on `kDuckdbNative` so the fast path keeps the standalone
  // shape it already handles. This pins that the
  // `VisitResolvedArrayScan` override above does not over-promote.
  const auto* stmt = Analyze("SELECT n FROM UNNEST([1, 2, 3]) AS n");
  ASSERT_NE(stmt, nullptr);
  RouteDecision d = classifier_.Classify(*stmt);
  EXPECT_EQ(d.disposition, Disposition::kDuckdbNative);
}

TEST_F(RouteClassifierTest, UncorrelatedSubqueryExprStaysOnFastPath) {
  // `WHERE id IN (SELECT n FROM UNNEST([1, 2, 3]) AS n)` is a
  // non-correlated IN subquery: the inner SELECT does not
  // reference any column from the outer scan. The analyzer marks
  // this by leaving `parameter_list()` empty. The transpiler's
  // `EmitSubqueryExpr` lowers it directly to DuckDB's
  // `(<lhs> IN (<sub>))` shape, so the classifier MUST keep the
  // query on `kDuckdbNative` -- a false promotion would force the
  // semantic executor to run a shape the fast path handles
  // correctly. See `cte-subquery-routing.plan.md` Family 3.
  const auto* stmt = Analyze(
      "SELECT id FROM people "
      "WHERE id IN (SELECT n FROM UNNEST([1, 2, 3]) AS n)");
  ASSERT_NE(stmt, nullptr);
  RouteDecision d = classifier_.Classify(*stmt);
  EXPECT_EQ(d.disposition, Disposition::kDuckdbNative);
  EXPECT_TRUE(d.offending_node.empty()) << d.offending_node;
}

TEST_F(RouteClassifierTest,
       CorrelatedScalarSubqueryExprPromotesToSemanticExecutor) {
  // `(SELECT COUNT(*) FROM <inner> WHERE <inner>.k = outer.k)` is
  // a correlated scalar subquery: the inner WHERE clause
  // references the outer scan's column. The analyzer marks the
  // referenced outer columns in `ResolvedSubqueryExpr::parameter_list()`,
  // which the classifier inspects via `VisitResolvedSubqueryExpr`.
  // Promotion to `kSemanticExecutor` is mandatory: DuckDB's
  // correlated-subquery decorrelation does not guarantee BigQuery
  // per-outer-row evaluation order for every shape, and the only
  // way to avoid silent approximation is to evaluate the inner
  // subquery once per outer row in the local interpreter.
  //
  // The semantic executor's correlated-subquery evaluator is
  // `cte-subquery-routing.plan.md` Family 4 (deferred to a
  // follow-up subagent); until it lands the gateway surfaces
  // UNIMPLEMENTED via the executor stub. That is the same
  // end-user-visible outcome the fast path's empty-string
  // contract would produce, but going through the classifier
  // means the disposition is a deliberate route choice, not a
  // surprise transpiler bailout.
  const auto* stmt = Analyze(
      "SELECT (SELECT COUNT(*) FROM people AS p WHERE p.id = people.id) AS c "
      "FROM people");
  ASSERT_NE(stmt, nullptr);
  RouteDecision d = classifier_.Classify(*stmt);
  EXPECT_EQ(d.disposition, Disposition::kSemanticExecutor);
  EXPECT_EQ(d.offending_node, "ResolvedSubqueryExpr(correlated)");
}

TEST_F(RouteClassifierTest,
       CorrelatedExistsSubqueryExprPromotesToSemanticExecutor) {
  // `EXISTS (SELECT 1 FROM <inner> WHERE <inner>.k = outer.k)` is
  // the most common correlated subquery shape (semi-join
  // expression). The classifier promotes it for the same reason
  // as the scalar case: per-outer-row evaluation order is
  // BigQuery-defined, not DuckDB's call. Self-join the only
  // available table (`people`) so the test does not depend on a
  // second catalog table. The semantic executor evaluator is
  // Family 4.
  const auto* stmt = Analyze(
      "SELECT id FROM people "
      "WHERE EXISTS (SELECT 1 FROM people AS p WHERE p.id = people.id)");
  ASSERT_NE(stmt, nullptr);
  RouteDecision d = classifier_.Classify(*stmt);
  EXPECT_EQ(d.disposition, Disposition::kSemanticExecutor);
  EXPECT_EQ(d.offending_node, "ResolvedSubqueryExpr(correlated)");
}

TEST_F(RouteClassifierTest, BarrierScanPromotesToSemanticExecutor) {
  // `advanced-relational-routing.plan.md` Family 2. A
  // `ResolvedBarrierScan` is a pipe-operator optimizer marker that
  // blocks fusion across its boundary. DuckDB has no analog
  // contract, so the classifier MUST promote any query containing
  // one to `kSemanticExecutor` (the row's YAML disposition).
  //
  // We exercise the YAML row directly via a hand-built statement
  // because the analyzer does not emit `ResolvedBarrierScan` for
  // surface SQL today (pipe operators are an analyzer feature flag).
  // The hand-built shape mirrors what the analyzer emits for
  // `<expr> |> BARRIER` once the flag flips.
  auto single = ::googlesql::MakeResolvedSingleRowScan();
  auto barrier = ::googlesql::MakeResolvedBarrierScan(
      /*column_list=*/{}, std::move(single));
  ::googlesql::ResolvedColumn out_col(
      /*column_id=*/200,
      /*table_name=*/::googlesql::IdString::MakeGlobal("$query"),
      /*name=*/::googlesql::IdString::MakeGlobal("c"),
      type_factory_->get_int64());
  std::vector<std::unique_ptr<const ::googlesql::ResolvedComputedColumn>> exprs;
  exprs.push_back(::googlesql::MakeResolvedComputedColumn(
      out_col, ::googlesql::MakeResolvedLiteral(::googlesql::Value::Int64(7))));
  auto project = ::googlesql::MakeResolvedProjectScan(
      /*column_list=*/{out_col}, std::move(exprs), std::move(barrier));
  std::vector<std::unique_ptr<const ::googlesql::ResolvedOutputColumn>> outputs;
  outputs.push_back(::googlesql::MakeResolvedOutputColumn("c", out_col));
  auto query_stmt = ::googlesql::MakeResolvedQueryStmt(
      std::move(outputs), /*is_value_table=*/false, std::move(project));

  RouteDecision d = classifier_.Classify(*query_stmt);
  EXPECT_EQ(d.disposition, Disposition::kSemanticExecutor);
  EXPECT_EQ(d.offending_node, "ResolvedBarrierScan");
}

TEST_F(RouteClassifierTest, PivotScanRoutesToDuckdbRewrite) {
  // `advanced-relational-routing.plan.md` Family 3. The engine
  // disables `REWRITE_PIVOT` so the analyzer hands us a raw
  // `ResolvedPivotScan`; the disposition table routes it through
  // `kDuckdbRewrite`, and the transpiler's `EmitPivotScan` lowers
  // it to DuckDB conditional aggregation (FILTER).
  const ::googlesql::ResolvedStatement* stmt =
      Analyze("SELECT * FROM people PIVOT(COUNT(*) FOR name IN ('a', 'b'))");
  ASSERT_NE(stmt, nullptr);
  RouteDecision d = classifier_.Classify(*stmt);
  EXPECT_EQ(d.disposition, Disposition::kDuckdbRewrite);
}

TEST_F(RouteClassifierTest, UnpivotScanRoutesToDuckdbRewrite) {
  // Same as the PIVOT test above but for `ResolvedUnpivotScan`.
  // The engine disables `REWRITE_UNPIVOT`; the disposition table
  // routes it through `kDuckdbRewrite`; the transpiler's
  // `EmitUnpivotScan` lowers it to UNION ALL.
  const ::googlesql::ResolvedStatement* stmt =
      Analyze("SELECT * FROM people UNPIVOT(value FOR label IN (id))");
  ASSERT_NE(stmt, nullptr);
  RouteDecision d = classifier_.Classify(*stmt);
  EXPECT_EQ(d.disposition, Disposition::kDuckdbRewrite);
}

TEST_F(RouteClassifierTest, RecursiveScanRoutesToDuckdbRewrite) {
  // `advanced-relational-routing.plan.md` Family 4. The disposition
  // table routes `ResolvedRecursiveScan` (and its
  // `ResolvedRecursiveRefScan` reference) through `kDuckdbRewrite`;
  // the transpiler's `EmitRecursiveScan` lowers it to DuckDB's
  // `WITH RECURSIVE`.
  const ::googlesql::ResolvedStatement* stmt = Analyze(
      "WITH RECURSIVE r AS ("
      "  SELECT 1 AS n"
      "  UNION ALL"
      "  SELECT n FROM r"
      ")"
      "SELECT n FROM r");
  ASSERT_NE(stmt, nullptr);
  RouteDecision d = classifier_.Classify(*stmt);
  EXPECT_EQ(d.disposition, Disposition::kDuckdbRewrite);
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
