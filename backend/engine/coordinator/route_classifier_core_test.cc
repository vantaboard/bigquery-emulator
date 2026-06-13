// Route classifier tests: core routing and array-scan promotion.

#include "backend/engine/coordinator/route_classifier_test_fixture.h"
#include "backend/engine/disposition.h"
#include "googlesql/public/value.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace coordinator {
namespace {
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
  // `docs/ENGINE_POLICY.md` the executor for that disposition
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

TEST_F(RouteClassifierTest, ApproxQuantilesFunctionRoutesToSemanticExecutor) {
  // `APPROX_QUANTILES` is on the `semantic_executor` route per
  // `functions.yaml` and `docs/ENGINE_POLICY.md`.
  const auto* stmt = Analyze("SELECT APPROX_QUANTILES(id, 4) FROM people");
  ASSERT_NE(stmt, nullptr);

  RouteDecision d = classifier_.Classify(*stmt);
  EXPECT_EQ(d.disposition, Disposition::kSemanticExecutor);
  EXPECT_EQ(d.offending_node, "function:approx_quantiles");
  EXPECT_NE(d.reason.find("approx_quantiles"), std::string::npos)
      << "reason should name the offending function; got: " << d.reason;
}

TEST_F(RouteClassifierTest,
       SemanticExecutorDominatesWhenApproxQuantilesPresent) {
  // `APPROX_QUANTILES` and `SAFE_DIVIDE` are both `semantic_executor`
  // rows in `functions.yaml`. The SELECT promotes
  // to the semantic route; `APPROX_QUANTILES` is recorded as the
  // offending node because it is the higher-priority promotion among
  // the semantic functions in this query shape.
  const auto* stmt = Analyze(
      "SELECT APPROX_QUANTILES(id, 4), AVG(SAFE_DIVIDE(id, 1)) FROM people");
  ASSERT_NE(stmt, nullptr);

  RouteDecision d = classifier_.Classify(*stmt);
  EXPECT_EQ(d.disposition, Disposition::kSemanticExecutor);
  EXPECT_EQ(d.offending_node, "function:approx_quantiles");
}

TEST_F(RouteClassifierTest, ScalarOnlySelectPromotesToSemanticExecutor) {
  // `SELECT 1` (and any other no-FROM SELECT) resolves to
  // `ResolvedQueryStmt(query=ResolvedProjectScan(input_scan=
  // ResolvedSingleRowScan))`. Per
  // `docs/ENGINE_POLICY.md` the classifier
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
  // `route_classifier.cc`. The semantic executor owns this surface
  // (`docs/ENGINE_POLICY.md`); the stub returns
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
// `route_classifier.cc`). Triggering
// `is_lateral=true` from a SimpleCatalog-backed SQL query
// requires the `LATERAL` keyword plus a correlated subquery shape
// that a minimal in-process catalog cannot easily express:
// surface SQL forms like `JOIN UNNEST(<col>)` analyze to a
// `ResolvedArrayScan` (NOT a `ResolvedJoinScan`), so they exercise
// a different route-promotion path. The lateral-promotion route
// will be pinned by the integration suite owned by
// `docs/ENGINE_POLICY.md` once that plan ships its
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
  // `docs/ENGINE_POLICY.md` Family 1.
  const auto* stmt =
      Analyze("SELECT n, idx FROM UNNEST([1, 2, 3]) AS n WITH OFFSET AS idx");
  ASSERT_NE(stmt, nullptr);
  RouteDecision d = classifier_.Classify(*stmt);
  EXPECT_EQ(d.disposition, Disposition::kSemanticExecutor);
  EXPECT_EQ(d.offending_node, "ResolvedArrayScan(array_offset_column)");
}

TEST_F(RouteClassifierTest, UnnestNamedConstantPromotesToSemanticExecutor) {
  const ::googlesql::Type* string_array_type = nullptr;
  ASSERT_TRUE(
      type_factory_
          ->MakeArrayType(type_factory_->get_string(), &string_array_type)
          .ok());
  std::unique_ptr<::googlesql::SimpleConstant> top_names;
  ASSERT_TRUE(::googlesql::SimpleConstant::Create(
                  {"top_names"},
                  ::googlesql::Value::EmptyArray(string_array_type->AsArray()),
                  &top_names)
                  .ok());
  catalog_->AddOwnedConstant(top_names.release());

  const auto* stmt = Analyze("SELECT name FROM UNNEST(top_names) AS name");
  ASSERT_NE(stmt, nullptr);
  RouteDecision d = classifier_.Classify(*stmt);
  EXPECT_EQ(d.disposition, Disposition::kSemanticExecutor);
  EXPECT_EQ(d.offending_node, "ResolvedArrayScan(script_constant)");
}

TEST_F(RouteClassifierTest, UnnestInlineArrayStaysOnDuckDbFastPath) {
  const auto* stmt = Analyze("SELECT n FROM UNNEST([1, 2, 3]) AS n");
  ASSERT_NE(stmt, nullptr);
  RouteDecision d = classifier_.Classify(*stmt);
  EXPECT_EQ(d.disposition, Disposition::kDuckdbNative);
}

TEST_F(RouteClassifierTest, OuterUnnestPromotesToSemanticExecutor) {
  // `LEFT JOIN UNNEST(<arr>)` (the outer form) flips `is_outer` to
  // true; BigQuery emits one all-NULL row when the array is empty,
  // DuckDB drops the row. Classifier promotes the query to the
  // semantic executor so the empty-array NULL-row contract gets
  // honored. See `docs/ENGINE_POLICY.md` Family 2.
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
  // `docs/ENGINE_POLICY.md` Family 3.
  const auto* stmt = Analyze("SELECT * FROM UNNEST([1, 2, 3], [10, 20, 30])");
  ASSERT_NE(stmt, nullptr);
  RouteDecision d = classifier_.Classify(*stmt);
  EXPECT_EQ(d.disposition, Disposition::kSemanticExecutor);
  EXPECT_EQ(d.offending_node, "ResolvedArrayScan(array_zip_mode)");
}

TEST_F(RouteClassifierTest, CorrelatedColumnUnnestPromotesToSemanticExecutor) {
  // `FROM t, UNNEST(t.arr)` still routes to the semantic executor until
  // the DuckDB transpiler covers aggregate/project shapes over lateral
  // UNNEST.
  const auto* stmt =
      Analyze("SELECT id, n FROM arr_table, UNNEST(arr_table.arr) AS n");
  ASSERT_NE(stmt, nullptr);
  RouteDecision d = classifier_.Classify(*stmt);
  EXPECT_EQ(d.disposition, Disposition::kSemanticExecutor);
  EXPECT_EQ(d.offending_node, "ResolvedArrayScan(correlated_input_scan)");
}

TEST_F(RouteClassifierTest, RowNumberOverFarmFingerprintStaysOnDuckDb) {
  // Nested ResolvedConstant nodes (FARM_FINGERPRINT('x')) must not promote
  // the whole query to semantic_executor.
  const auto* stmt = Analyze(
      "SELECT ROW_NUMBER() OVER (ORDER BY FARM_FINGERPRINT('x') ASC) "
      "FROM people");
  ASSERT_NE(stmt, nullptr);
  RouteDecision d = classifier_.Classify(*stmt);
  EXPECT_EQ(d.disposition, Disposition::kDuckdbNative) << d.offending_node;
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

TEST_F(RouteClassifierTest, DateFuncsBenchShapeRoutesToDuckDbNative) {
  const auto* stmt = Analyze(
      "SELECT EXTRACT(YEAR FROM DATE_ADD(DATE '2020-01-01', INTERVAL id "
      "DAY)) AS yr, COUNT(*) AS cnt FROM arr_table GROUP BY yr ORDER BY yr");
  ASSERT_NE(stmt, nullptr);
  RouteDecision d = classifier_.Classify(*stmt);
  EXPECT_TRUE(d.disposition == Disposition::kDuckdbNative ||
              d.disposition == Disposition::kDuckdbUdf)
      << d.offending_node;
}

TEST_F(RouteClassifierTest, InsertSelectQualifyRoutesToDuckdbNative) {
  const auto* stmt = Analyze(
      "INSERT INTO people (id, name) "
      "SELECT * FROM people "
      "QUALIFY ROW_NUMBER() OVER (PARTITION BY id ORDER BY name) = 1");
  ASSERT_NE(stmt, nullptr);
  RouteDecision d = classifier_.Classify(*stmt);
  EXPECT_EQ(d.disposition, Disposition::kDuckdbNative);
}

TEST_F(RouteClassifierTest, InsertValuesStaysOnSemanticExecutor) {
  const auto* stmt = Analyze("INSERT INTO people (id, name) VALUES (1, 'a')");
  ASSERT_NE(stmt, nullptr);
  RouteDecision d = classifier_.Classify(*stmt);
  EXPECT_EQ(d.disposition, Disposition::kSemanticExecutor);
  EXPECT_EQ(d.offending_node, "ResolvedInsertStmt");
}
}  // namespace
}  // namespace coordinator
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
