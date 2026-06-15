// Route classifier tests: subqueries, hand-built nodes, and edge cases.

#include "backend/engine/coordinator/route_classifier_test_fixture.h"
#include "backend/engine/disposition.h"
#include "googlesql/public/id_string.h"
#include "googlesql/public/value.h"
#include "googlesql/resolved_ast/resolved_column.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace coordinator {
namespace {
TEST_F(RouteClassifierTest, UncorrelatedSubqueryExprStaysOnFastPath) {
  // `WHERE id IN (SELECT n FROM UNNEST([1, 2, 3]) AS n)` is a
  // non-correlated IN subquery: the inner SELECT does not
  // reference any column from the outer scan. The analyzer marks
  // this by leaving `parameter_list()` empty. The transpiler's
  // `EmitSubqueryExpr` lowers it directly to DuckDB's
  // `(<lhs> IN (<sub>))` shape, so the classifier MUST keep the
  // query on `kDuckdbNative` -- a false promotion would force the
  // semantic executor to run a shape the fast path handles
  // correctly. See `docs/ENGINE_POLICY.md` Family 3.
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
  // `docs/ENGINE_POLICY.md` Family 4 (deferred to a
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
  // `docs/ENGINE_POLICY.md` Family 2. A
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
  // `docs/ENGINE_POLICY.md` Family 3. The engine
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
  // `docs/ENGINE_POLICY.md` Family 4. The disposition
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

TEST_F(RouteClassifierTest, DeferredComputedColumnPromotesToSemanticExecutor) {
  // `docs/ENGINE_POLICY.md` Family 5. A
  // `ResolvedDeferredComputedColumn` is the side-effect-aware
  // form of a computed column the analyzer emits when conditional
  // / pipe-evaluation features capture errors in a companion
  // BYTES `side_effect_column`. DuckDB has no native model for the
  // deferred-error semantic; the disposition table routes any
  // query containing one to the semantic executor.
  //
  // The analyzer only emits this shape under specific feature
  // flags that the engine does not enable today, so we exercise
  // the YAML row by hand-building a `ResolvedAggregateScan` whose
  // `aggregate_list` (typed as `ResolvedComputedColumnBase`) holds
  // a `ResolvedDeferredComputedColumn`. The classifier walks the
  // statement tree and promotes the route via the YAML lookup the
  // moment it sees the deferred node.
  ::googlesql::ResolvedColumn out_col(
      /*column_id=*/300,
      /*table_name=*/::googlesql::IdString::MakeGlobal("$query"),
      /*name=*/::googlesql::IdString::MakeGlobal("v"),
      type_factory_->get_int64());
  ::googlesql::ResolvedColumn side_col(
      /*column_id=*/301,
      /*table_name=*/::googlesql::IdString::MakeGlobal("$query"),
      /*name=*/::googlesql::IdString::MakeGlobal("_se"),
      type_factory_->get_bytes());
  std::vector<std::unique_ptr<const ::googlesql::ResolvedComputedColumnBase>>
      aggregate_list;
  aggregate_list.push_back(::googlesql::MakeResolvedDeferredComputedColumn(
      out_col,
      ::googlesql::MakeResolvedLiteral(::googlesql::Value::Int64(0)),
      side_col));
  auto agg_scan = ::googlesql::MakeResolvedAggregateScan(
      /*column_list=*/{out_col},
      ::googlesql::MakeResolvedSingleRowScan(),
      /*group_by_list=*/{},
      std::move(aggregate_list),
      /*grouping_set_list=*/{},
      /*rollup_column_list=*/{});
  std::vector<std::unique_ptr<const ::googlesql::ResolvedOutputColumn>> outputs;
  outputs.push_back(::googlesql::MakeResolvedOutputColumn("v", out_col));
  auto query_stmt = ::googlesql::MakeResolvedQueryStmt(
      std::move(outputs), /*is_value_table=*/false, std::move(agg_scan));

  RouteDecision d = classifier_.Classify(*query_stmt);
  EXPECT_EQ(d.disposition, Disposition::kSemanticExecutor);
  EXPECT_EQ(d.offending_node, "ResolvedDeferredComputedColumn");
}

TEST_F(RouteClassifierTest, DifferentialPrivacyAggregateScanRoutesToLocalStub) {
  const auto* stmt = Analyze(
      "SELECT WITH DIFFERENTIAL_PRIVACY "
      "OPTIONS(epsilon=10, delta=0.01, privacy_unit_column=id) "
      "name, COUNT(*) AS c FROM people GROUP BY name");
  ASSERT_NE(stmt, nullptr);

  RouteDecision d = classifier_.Classify(*stmt);
  EXPECT_EQ(d.disposition, Disposition::kLocalStub);
  EXPECT_EQ(d.offending_node, "ResolvedDifferentialPrivacyAggregateScan");
  EXPECT_NE(d.reason.find("local-stub"), std::string::npos)
      << "reason should mention the local-stub route; got: " << d.reason;
}

TEST_F(RouteClassifierTest,
       AggregationThresholdAggregateScanRoutesToLocalStub) {
  const auto* stmt = Analyze(
      "SELECT WITH AGGREGATION_THRESHOLD "
      "OPTIONS(threshold=1, privacy_unit_column=id) "
      "name, COUNT(*) AS c FROM people GROUP BY name");
  ASSERT_NE(stmt, nullptr);

  RouteDecision d = classifier_.Classify(*stmt);
  EXPECT_EQ(d.disposition, Disposition::kLocalStub);
  EXPECT_EQ(d.offending_node, "ResolvedAggregationThresholdAggregateScan");
}

TEST_F(RouteClassifierTest, AnonymizedAggregateScanRoutesToLocalStub) {
  const auto* stmt = Analyze(
      "SELECT WITH ANONYMIZATION "
      "OPTIONS(k_threshold=1, epsilon=10) "
      "name, COUNT(*) AS c FROM people GROUP BY name");
  ASSERT_NE(stmt, nullptr);

  RouteDecision d = classifier_.Classify(*stmt);
  EXPECT_EQ(d.disposition, Disposition::kLocalStub);
  EXPECT_EQ(d.offending_node, "ResolvedAnonymizedAggregateScan");
}

TEST_F(RouteClassifierTest, MlPredictRoutesToLocalStub) {
  const auto* stmt = Analyze(
      "SELECT * FROM ML.PREDICT(MODEL `ds.unregistered_model`, "
      "(SELECT 1.0 AS f1))");
  ASSERT_NE(stmt, nullptr);

  RouteDecision d = classifier_.Classify(*stmt);
  EXPECT_EQ(d.disposition, Disposition::kLocalStub);
  EXPECT_EQ(d.offending_node, "function:ml.predict");
  EXPECT_NE(d.reason.find("local-stub"), std::string::npos)
      << "reason should mention the local-stub route; got: " << d.reason;
}

TEST_F(RouteClassifierTest, KeysFunctionRoutesToLocalStub) {
  // `KEYS.NEW_KEYSET('AEAD_AES_GCM_256')` is a `local_stub` row in
  // `functions.yaml` per `docs/ENGINE_POLICY.md`. A
  // SELECT referencing it must promote the route to `kLocalStub`
  // (above `kSemanticExecutor`, below `kUnsupported`) so the
  // coordinator dispatches into the semantic executor's per-family
  // stub handler (`backend/engine/semantic/stubs/keys.cc`). The
  // offending node carries the function name so a future
  // gateway-side error envelope can attribute the stub to the
  // right family.
  const auto* stmt = Analyze("SELECT KEYS.NEW_KEYSET('AEAD_AES_GCM_256')");
  ASSERT_NE(stmt, nullptr);

  RouteDecision d = classifier_.Classify(*stmt);
  EXPECT_EQ(d.disposition, Disposition::kLocalStub);
  EXPECT_EQ(d.offending_node, "function:keys.new_keyset");
  EXPECT_NE(d.reason.find("local-stub"), std::string::npos)
      << "reason should mention the local-stub route; got: " << d.reason;
}

TEST_F(RouteClassifierTest, KeysEncryptRoutesToLocalStub) {
  // `KEYS.ENCRYPT` is registered on the `keys` subcatalog (not stock
  // builtins) and is a `local_stub` row in `functions.yaml`. Analysis
  // must resolve the full `KEYS.ENCRYPT` name path and classify to
  // `kLocalStub` with `function:keys.encrypt` attribution.
  const auto* stmt = Analyze(
      "SELECT KEYS.ENCRYPT(KEYS.NEW_KEYSET('AEAD_AES_GCM_256'), "
      "FROM_BASE64('YWJj'))");
  ASSERT_NE(stmt, nullptr);

  RouteDecision d = classifier_.Classify(*stmt);
  EXPECT_EQ(d.disposition, Disposition::kLocalStub);
  EXPECT_EQ(d.offending_node, "function:keys.encrypt");
  EXPECT_NE(d.reason.find("local-stub"), std::string::npos)
      << "reason should mention the local-stub route; got: " << d.reason;
}

TEST_F(RouteClassifierTest, LocalStubOutranksSemanticExecutorInSameQuery) {
  // When a `local_stub` function (`KEYS.NEW_KEYSET`) and a
  // `semantic_executor` function (`APPROX_QUANTILES`) appear together,
  // the local-stub promotion wins (priority 5 > 4).
  const auto* stmt = Analyze(
      "SELECT APPROX_QUANTILES(id, 4) AS q, "
      "KEYS.NEW_KEYSET('AEAD_AES_GCM_256') AS k FROM people");
  ASSERT_NE(stmt, nullptr);

  RouteDecision d = classifier_.Classify(*stmt);
  EXPECT_EQ(d.disposition, Disposition::kLocalStub);
  EXPECT_EQ(d.offending_node, "function:keys.new_keyset");
}

TEST_F(RouteClassifierTest, MatchRecognizeScanPromotesToSemanticExecutor) {
  const auto* stmt = Analyze(
      "SELECT m FROM people MATCH_RECOGNIZE("
      "ORDER BY id MEASURES COUNT(*) AS m PATTERN (A) DEFINE A AS id > 0)");
  ASSERT_NE(stmt, nullptr);

  RouteDecision d = classifier_.Classify(*stmt);
  EXPECT_EQ(d.disposition, Disposition::kSemanticExecutor);
  EXPECT_EQ(d.offending_node, "ResolvedMatchRecognizeScan");
}

TEST_F(RouteClassifierTest, ExplainStatementRoutesToSemanticExecutor) {
  const auto* stmt = Analyze("EXPLAIN SELECT * FROM people");
  ASSERT_NE(stmt, nullptr);

  RouteDecision d = classifier_.Classify(*stmt);
  EXPECT_EQ(d.disposition, Disposition::kSemanticExecutor);
  EXPECT_EQ(d.offending_node, "ResolvedExplainStmt");
}
}  // namespace
}  // namespace coordinator
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
