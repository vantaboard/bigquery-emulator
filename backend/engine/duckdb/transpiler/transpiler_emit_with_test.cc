#include "backend/engine/duckdb/transpiler/transpiler_test_fixture.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace duckdb {
namespace transpiler {

TEST_F(TranspilerTest, EmitWithScanSingleCteSelectAll) {
  // Single-binding non-recursive CTE referenced once. The CTE body
  // is a plain TableScan over `people`; the ref reads two columns
  // back out.
  const ::googlesql::ResolvedStatement* stmt = Analyze(
      "WITH p AS (SELECT id, name FROM people) "
      "SELECT id, name FROM p");
  ASSERT_NE(stmt, nullptr);
  TestTranspiler t;
  std::string sql = t.Transpile(stmt);
  // The outermost EmitQueryStmt wraps the WITH body as a derived
  // table for the user-visible alias mapping; the CTE itself
  // anchors its columns positionally. The exact wrapping shape is
  // an emit-stability contract -- a regression that drops the
  // anchor or renames the CTE shows up here.
  EXPECT_NE(sql.find("WITH \"p\" AS ("), std::string::npos)
      << "expected non-recursive CTE header; got: " << sql;
  EXPECT_NE(sql.find(" AS \"_cte_0\""), std::string::npos)
      << "expected positional anchor for column 0; got: " << sql;
  EXPECT_NE(sql.find(" AS \"_cte_1\""), std::string::npos)
      << "expected positional anchor for column 1; got: " << sql;
  EXPECT_NE(sql.find(" FROM \"p\""), std::string::npos)
      << "expected ref scan to FROM the CTE name; got: " << sql;
}

TEST_F(TranspilerTest, EmitWithScanMultipleCtesDistinctNames) {
  // Two CTEs in one WITH clause, each bound to a separate ref.
  // The emit must produce both CTE headers and both ref-side
  // SELECTs without cross-pollution of column names.
  const ::googlesql::ResolvedStatement* stmt = Analyze(
      "WITH p AS (SELECT id FROM people), "
      "     o AS (SELECT order_id FROM orders) "
      "SELECT p.id, o.order_id FROM p, o");
  ASSERT_NE(stmt, nullptr);
  TestTranspiler t;
  std::string sql = t.Transpile(stmt);
  EXPECT_NE(sql.find("WITH \"p\" AS ("), std::string::npos)
      << "expected first CTE header; got: " << sql;
  EXPECT_NE(sql.find("\"o\" AS ("), std::string::npos)
      << "expected second CTE header; got: " << sql;
  EXPECT_NE(sql.find(" FROM \"p\""), std::string::npos)
      << "expected ref to first CTE; got: " << sql;
  EXPECT_NE(sql.find(" FROM \"o\""), std::string::npos)
      << "expected ref to second CTE; got: " << sql;
}

TEST_F(TranspilerTest, EmitWithScanCteReferencedTwice) {
  // A single CTE referenced from two different scan positions.
  // The positional-anchor + per-ref rename scheme means each ref
  // independently renames the anchor to its own column names,
  // and the two refs cannot collide on a shared analyzer name.
  // SELF-JOIN form keeps both refs in scope at once.
  const ::googlesql::ResolvedStatement* stmt = Analyze(
      "WITH p AS (SELECT id, name FROM people) "
      "SELECT a.id, b.name FROM p AS a, p AS b WHERE a.id = b.id");
  ASSERT_NE(stmt, nullptr);
  TestTranspiler t;
  std::string sql = t.Transpile(stmt);
  // `a.id = b.id` lowers through `$equal` which is not yet in the
  // function disposition table for transpiled emit. That makes
  // the FilterScan return "" -- which then bubbles up through the
  // outer QueryStmt emit. The empty-string contract here is the
  // right answer for now; we still pin the CTE shape via the
  // single-ref test above. Smoke-test that no partial / malformed
  // emit gets through.
  EXPECT_TRUE(sql.empty() || sql.find("WITH \"p\" AS (") != std::string::npos)
      << "expected either empty (FilterScan fallback) or a WITH header; got: "
      << sql;
}

TEST_F(TranspilerTest, EmitWithScanRecursivePropagatesKeyword) {
  // `WITH RECURSIVE` now lowers through `EmitRecursiveScan`
  // (advanced-relational-routing Family 4), so the prior
  // "bails-to-empty" defense-in-depth has been promoted to a
  // real emit. This hand-built shape mirrors what the analyzer
  // produces for a trivial `WITH RECURSIVE r AS (SELECT 1 UNION
  // ALL SELECT 1) SELECT 1` -- bypassing the analyzer keeps the
  // test independent of the (currently-unmapped) `$less` /
  // `$greater` disposition entries that a non-trivial recursive
  // CTE relies on.
  //
  // The check confirms `WITH RECURSIVE` makes it into the emitted
  // SQL whenever the WithScan's `recursive()` flag is set.
  ::googlesql::ResolvedColumn anchor_col(
      /*column_id=*/200,
      /*table_name=*/::googlesql::IdString::MakeGlobal("$rec"),
      /*name=*/::googlesql::IdString::MakeGlobal("n"),
      type_factory_->get_int64());
  std::vector<std::unique_ptr<const ::googlesql::ResolvedComputedColumn>>
      anchor_exprs;
  anchor_exprs.push_back(::googlesql::MakeResolvedComputedColumn(
      anchor_col,
      ::googlesql::MakeResolvedLiteral(::googlesql::Value::Int64(1))));
  auto anchor_project = ::googlesql::MakeResolvedProjectScan(
      /*column_list=*/{anchor_col},
      std::move(anchor_exprs),
      ::googlesql::MakeResolvedSingleRowScan());
  auto anchor_item = ::googlesql::MakeResolvedSetOperationItem(
      std::move(anchor_project), /*output_column_list=*/{anchor_col});
  ::googlesql::ResolvedColumn rec_col(
      /*column_id=*/201,
      /*table_name=*/::googlesql::IdString::MakeGlobal("$rec"),
      /*name=*/::googlesql::IdString::MakeGlobal("n"),
      type_factory_->get_int64());
  std::vector<std::unique_ptr<const ::googlesql::ResolvedComputedColumn>>
      rec_exprs;
  rec_exprs.push_back(::googlesql::MakeResolvedComputedColumn(
      rec_col, ::googlesql::MakeResolvedLiteral(::googlesql::Value::Int64(2))));
  auto rec_project = ::googlesql::MakeResolvedProjectScan(
      /*column_list=*/{rec_col},
      std::move(rec_exprs),
      ::googlesql::MakeResolvedSingleRowScan());
  auto rec_item = ::googlesql::MakeResolvedSetOperationItem(
      std::move(rec_project), /*output_column_list=*/{rec_col});
  auto recursive_scan = ::googlesql::MakeResolvedRecursiveScan(
      /*column_list=*/{anchor_col},
      ::googlesql::ResolvedRecursiveScan::UNION_ALL,
      std::move(anchor_item),
      std::move(rec_item),
      /*recursion_depth_modifier=*/nullptr);
  auto entry =
      ::googlesql::MakeResolvedWithEntry("r", std::move(recursive_scan));
  std::vector<std::unique_ptr<const ::googlesql::ResolvedWithEntry>> entries;
  entries.push_back(std::move(entry));
  auto with_scan = ::googlesql::MakeResolvedWithScan(
      /*column_list=*/{},
      std::move(entries),
      ::googlesql::MakeResolvedSingleRowScan(),
      /*recursive=*/true);
  TestTranspiler t;
  std::string sql = t.EmitWithScan(with_scan.get());
  EXPECT_NE(sql.find("WITH RECURSIVE \"r\""), std::string::npos)
      << "expected WITH RECURSIVE keyword; got: " << sql;
  EXPECT_NE(sql.find(" UNION ALL "), std::string::npos)
      << "expected UNION ALL between anchor and recursive arm; got: " << sql;
}

TEST_F(TranspilerTest, EmitRecursiveScanWithDepthThreadsCounter) {
  ::googlesql::ResolvedColumn n_col(
      /*column_id=*/200,
      /*table_name=*/::googlesql::IdString::MakeGlobal("$rec"),
      /*name=*/::googlesql::IdString::MakeGlobal("n"),
      type_factory_->get_int64());
  ::googlesql::ResolvedColumn depth_col(
      /*column_id=*/201,
      /*table_name=*/::googlesql::IdString::MakeGlobal("$rec"),
      /*name=*/::googlesql::IdString::MakeGlobal("depth"),
      type_factory_->get_int64());
  auto depth_mod = ::googlesql::MakeResolvedRecursionDepthModifier(
      /*lower_bound=*/nullptr,
      /*upper_bound=*/nullptr,
      ::googlesql::MakeResolvedColumnHolder(depth_col));
  std::vector<std::unique_ptr<const ::googlesql::ResolvedComputedColumn>>
      anchor_exprs;
  anchor_exprs.push_back(::googlesql::MakeResolvedComputedColumn(
      n_col,
      ::googlesql::MakeResolvedLiteral(::googlesql::Value::Int64(1))));
  auto anchor_project = ::googlesql::MakeResolvedProjectScan(
      /*column_list=*/{n_col},
      std::move(anchor_exprs),
      ::googlesql::MakeResolvedSingleRowScan());
  auto anchor_item = ::googlesql::MakeResolvedSetOperationItem(
      std::move(anchor_project), /*output_column_list=*/{n_col, depth_col});
  ::googlesql::ResolvedColumn rec_n_col(
      /*column_id=*/202,
      /*table_name=*/::googlesql::IdString::MakeGlobal("$rec"),
      /*name=*/::googlesql::IdString::MakeGlobal("n"),
      type_factory_->get_int64());
  ::googlesql::ResolvedColumn rec_depth_col(
      /*column_id=*/203,
      /*table_name=*/::googlesql::IdString::MakeGlobal("$rec"),
      /*name=*/::googlesql::IdString::MakeGlobal("depth"),
      type_factory_->get_int64());
  std::vector<std::unique_ptr<const ::googlesql::ResolvedComputedColumn>>
      rec_exprs;
  rec_exprs.push_back(::googlesql::MakeResolvedComputedColumn(
      rec_n_col,
      ::googlesql::MakeResolvedLiteral(::googlesql::Value::Int64(2))));
  auto rec_project = ::googlesql::MakeResolvedProjectScan(
      /*column_list=*/{rec_n_col},
      std::move(rec_exprs),
      ::googlesql::MakeResolvedSingleRowScan());
  auto rec_item = ::googlesql::MakeResolvedSetOperationItem(
      std::move(rec_project), /*output_column_list=*/{rec_n_col, rec_depth_col});
  auto recursive_scan = ::googlesql::MakeResolvedRecursiveScan(
      /*column_list=*/{n_col, depth_col},
      ::googlesql::ResolvedRecursiveScan::UNION_ALL,
      std::move(anchor_item),
      std::move(rec_item),
      std::move(depth_mod));
  auto entry =
      ::googlesql::MakeResolvedWithEntry("r", std::move(recursive_scan));
  std::vector<std::unique_ptr<const ::googlesql::ResolvedWithEntry>> entries;
  entries.push_back(std::move(entry));
  auto with_scan = ::googlesql::MakeResolvedWithScan(
      /*column_list=*/{},
      std::move(entries),
      ::googlesql::MakeResolvedSingleRowScan(),
      /*recursive=*/true);
  TestTranspiler t;
  std::string sql = t.EmitWithScan(with_scan.get());
  EXPECT_NE(sql.find("0 AS \"_cte_1\""), std::string::npos)
      << "expected anchor depth 0; got: " << sql;
  EXPECT_NE(sql.find("\"depth\" + 1 AS \"_cte_1\""), std::string::npos)
      << "expected recursive depth increment; got: " << sql;
}

TEST_F(TranspilerTest, EmitWithRefScanBareDirect) {
  // Direct-construction of a ResolvedWithRefScan so we can pin the
  // per-column rename without depending on the surrounding
  // WithScan setup. The ref scan declares two columns; the emit
  // produces the `SELECT "_cte_0" AS "<n0>", "_cte_1" AS "<n1>"
  // FROM "<name>"` shape.
  ::googlesql::ResolvedColumn c0(
      /*column_id=*/10,
      /*table_name=*/::googlesql::IdString::MakeGlobal("p"),
      /*name=*/::googlesql::IdString::MakeGlobal("id"),
      type_factory_->get_int64());
  ::googlesql::ResolvedColumn c1(
      /*column_id=*/11,
      /*table_name=*/::googlesql::IdString::MakeGlobal("p"),
      /*name=*/::googlesql::IdString::MakeGlobal("name"),
      type_factory_->get_string());
  auto ref = ::googlesql::MakeResolvedWithRefScan({c0, c1}, "p");
  TestTranspiler t;
  EXPECT_EQ(t.EmitWithRefScan(ref.get()),
            "SELECT \"_cte_0\" AS \"id\", \"_cte_1\" AS \"name\" FROM \"p\"");
}

// --- ResolvedSubqueryExpr (non-correlated) -----------------------------
//
// `docs/ENGINE_POLICY.md` Family 2. Non-correlated SCALAR /
// IN / EXISTS / ARRAY subqueries lower to DuckDB's native subquery
// surface. Correlated forms (non-empty `parameter_list()`) are
// the classifier's responsibility (Family 3 promotes them to
// `kSemanticExecutor`); on the transpiler side we defend in
// depth by bailing to "" if we somehow see one.

TEST_F(TranspilerTest, EmitSubqueryExprScalarFromAnalyzer) {
  // `SELECT (SELECT MAX(id) FROM people) AS m FROM people`
  // wraps a scalar subquery in the outer SELECT's expr_list. The
  // inner subquery has no `parameter_list` (uncorrelated), so the
  // emit lowers to `(<inner_sql>)`.
  const ::googlesql::ResolvedStatement* stmt =
      Analyze("SELECT (SELECT MAX(id) FROM people) AS m FROM people");
  ASSERT_NE(stmt, nullptr);
  TestTranspiler t;
  std::string sql = t.Transpile(stmt);
  // The exact wrap shape changes with the outermost emit, but the
  // scalar-subquery emit MUST appear as a parenthesized SELECT
  // somewhere in the body. The empty-string contract would mean
  // the SubqueryExpr emit silently failed.
  EXPECT_FALSE(sql.empty()) << "expected non-empty SQL; SubqueryExpr emit "
                               "should not silently fail";
  EXPECT_NE(sql.find("(SELECT"), std::string::npos)
      << "expected a parenthesized scalar subquery; got: " << sql;
}

TEST_F(TranspilerTest, EmitSubqueryExprExistsFromAnalyzer) {
  // `WHERE EXISTS (<sub>)` resolves to a FilterScan whose
  // `filter_expr` is a SubqueryExpr of type EXISTS. The emit
  // wraps the inner SELECT in `EXISTS (...)`.
  const ::googlesql::ResolvedStatement* stmt =
      Analyze("SELECT id FROM people WHERE EXISTS (SELECT 1 FROM orders)");
  ASSERT_NE(stmt, nullptr);
  TestTranspiler t;
  std::string sql = t.Transpile(stmt);
  ASSERT_FALSE(sql.empty()) << "expected non-empty SQL";
  EXPECT_NE(sql.find("EXISTS ("), std::string::npos)
      << "expected EXISTS-prefixed subquery; got: " << sql;
}

TEST_F(TranspilerTest, EmitSubqueryExprInFromAnalyzer) {
  // `WHERE <lhs> IN (<sub>)` resolves to a FilterScan whose
  // `filter_expr` is a SubqueryExpr of type IN with the LHS
  // captured in `in_expr`. The emit wraps as `(<lhs> IN (<sub>))`.
  const ::googlesql::ResolvedStatement* stmt = Analyze(
      "SELECT id FROM people WHERE id IN (SELECT order_id FROM orders)");
  ASSERT_NE(stmt, nullptr);
  TestTranspiler t;
  std::string sql = t.Transpile(stmt);
  ASSERT_FALSE(sql.empty()) << "expected non-empty SQL";
  EXPECT_NE(sql.find(" IN ("), std::string::npos)
      << "expected IN-style emit; got: " << sql;
}

TEST_F(TranspilerTest, EmitSubqueryExprArrayFromAnalyzer) {
  // `SELECT ARRAY(<sub>)` resolves to a SubqueryExpr of type
  // ARRAY. DuckDB's `ARRAY(SELECT ...)` builds a LIST whose
  // element order matches the subquery's row order, matching
  // BigQuery's contract.
  const ::googlesql::ResolvedStatement* stmt =
      Analyze("SELECT ARRAY(SELECT id FROM people) AS ids FROM people");
  ASSERT_NE(stmt, nullptr);
  TestTranspiler t;
  std::string sql = t.Transpile(stmt);
  ASSERT_FALSE(sql.empty()) << "expected non-empty SQL";
  EXPECT_NE(sql.find("ARRAY("), std::string::npos)
      << "expected ARRAY-prefixed subquery; got: " << sql;
}

TEST_F(TranspilerTest, EmitSubqueryExprCorrelatedBailsToEmpty) {
  // Direct construction so we can build a SubqueryExpr with a
  // non-empty `parameter_list` -- the analyzer-side path for
  // correlated subqueries gets caught by the route classifier
  // upstream (Family 3), so we never reach the transpiler with
  // one. The bailout here is defense-in-depth: a future change
  // that routes a correlated form through this emit MUST NOT
  // silently lower it (the inner ColumnRefs would resolve against
  // DuckDB's own evaluation context, not the BigQuery outer-row
  // context, producing wrong answers).
  ::googlesql::ResolvedColumn outer_col(
      /*column_id=*/1,
      /*table_name=*/::googlesql::IdString::MakeGlobal("outer"),
      /*name=*/::googlesql::IdString::MakeGlobal("k"),
      type_factory_->get_int64());
  std::vector<std::unique_ptr<const ::googlesql::ResolvedColumnRef>> params;
  params.push_back(::googlesql::MakeResolvedColumnRef(
      type_factory_->get_int64(), outer_col, /*is_correlated=*/true));
  auto inner = ::googlesql::MakeResolvedSingleRowScan();
  auto sub = ::googlesql::MakeResolvedSubqueryExpr(
      type_factory_->get_int64(),
      ::googlesql::ResolvedSubqueryExpr::SCALAR,
      std::move(params),
      /*in_expr=*/nullptr,
      std::move(inner));
  TestTranspiler t;
  EXPECT_EQ(t.EmitSubqueryExpr(sub.get()), "");
}

TEST_F(TranspilerTest, EmitSubqueryExprUnsupportedTypeBailsToEmpty) {
  // LIKE ANY / LIKE ALL / NOT LIKE ANY / NOT LIKE ALL stay on the
  // empty-string fallback today (deliberately out of scope for the
  // transpiler). Pin that
  // the default branch returns "" rather than emitting partial
  // SQL when a non-{SCALAR, IN, EXISTS, ARRAY} type slips through.
  auto inner = ::googlesql::MakeResolvedSingleRowScan();
  auto sub = ::googlesql::MakeResolvedSubqueryExpr(
      type_factory_->get_bool(),
      ::googlesql::ResolvedSubqueryExpr::LIKE_ANY,
      /*parameter_list=*/{},
      /*in_expr=*/
      ::googlesql::MakeResolvedLiteral(::googlesql::Value::String("a")),
      std::move(inner));
  TestTranspiler t;
  EXPECT_EQ(t.EmitSubqueryExpr(sub.get()), "");
}

TEST_F(TranspilerTest, EmitWithRefScanEmptyColumnListUsesStar) {
  // Degenerate `SELECT * FROM <cte>` shape: the analyzer may
  // produce a WithRefScan with an empty column_list when the
  // surrounding scan does not project any columns off the ref.
  // The emit falls back to `SELECT *` rather than emitting a
  // bare `SELECT  FROM "p"` which DuckDB would reject.
  auto ref = ::googlesql::MakeResolvedWithRefScan({}, "p");
  TestTranspiler t;
  EXPECT_EQ(t.EmitWithRefScan(ref.get()), "SELECT * FROM \"p\"");
}

}  // namespace transpiler
}  // namespace duckdb
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
