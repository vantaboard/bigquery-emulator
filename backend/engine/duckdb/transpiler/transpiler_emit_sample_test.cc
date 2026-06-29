#include <gtest/gtest.h>

#include <string>

#include "backend/engine/duckdb/transpiler/transpiler_test_fixture.h"
#include "googlesql/resolved_ast/resolved_ast.h"
#include "googlesql/resolved_ast/resolved_node_kind.pb.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace duckdb {
namespace transpiler {

TEST_F(TranspilerTest, EmitSampleScanSystemPercentFromSurface) {
  // BigQuery `TABLESAMPLE SYSTEM (10 PERCENT)` lowers to a
  // `ResolvedSampleScan` whose `method=SYSTEM`, `unit=PERCENT`,
  // and `size=10`. DuckDB's `USING SAMPLE 10 PERCENT (system)`
  // matches the BQ semantics (block-level sampling at the chosen
  // percent).
  const ::googlesql::ResolvedStatement* stmt =
      Analyze("SELECT * FROM people TABLESAMPLE SYSTEM (10 PERCENT)");
  if (stmt == nullptr) {
    GTEST_SKIP() << "analyzer rejected TABLESAMPLE SYSTEM -- skip";
  }
  const ::googlesql::ResolvedScan* scan = QueryInputScan(stmt);
  if (scan == nullptr ||
      scan->node_kind() != ::googlesql::RESOLVED_SAMPLE_SCAN) {
    GTEST_SKIP() << "analyzer did not produce ResolvedSampleScan -- skip";
  }
  TestTranspiler t;
  EXPECT_EQ(t.EmitSampleScan(scan->GetAs<::googlesql::ResolvedSampleScan>()),
            "SELECT * FROM (SELECT \"id\", \"name\" FROM \"people\") "
            "USING SAMPLE 10 PERCENT (system)");
}

// Helper: synthesize a `ResolvedSampleScan` directly so each
// emit-shape (method + unit + optional repeatable/weight/stratify)
// can be exercised without driving the analyzer through the
// (sometimes BQ-only) surface SQL forms. The input is a fresh
// `ResolvedSingleRowScan` so the wrapped child scan always emits
// `SELECT 1`. Callers transfer ownership of the size / repeatable
// / weight / partition_by expressions through `std::move`. Returns
// nullptr only when the size expression is missing (a malformed
// SampleScan the analyzer would never produce).
struct TestSampleScanArgs {
  std::string method;
  ::googlesql::ResolvedSampleScan::SampleUnit unit =
      ::googlesql::ResolvedSampleScan::PERCENT;
  std::unique_ptr<const ::googlesql::ResolvedExpr> size{};
  std::unique_ptr<const ::googlesql::ResolvedExpr> repeatable{};
  std::unique_ptr<const ::googlesql::ResolvedColumnHolder> weight{};
  std::vector<std::unique_ptr<const ::googlesql::ResolvedExpr>> partition_by{};
};
std::unique_ptr<::googlesql::ResolvedSampleScan> MakeTestSampleScan(
    TestSampleScanArgs args) {
  if (args.size == nullptr) return nullptr;
  return ::googlesql::MakeResolvedSampleScan(
      /*column_list=*/{},
      ::googlesql::MakeResolvedSingleRowScan(),
      args.method,
      std::move(args.size),
      args.unit,
      std::move(args.repeatable),
      std::move(args.weight),
      std::move(args.partition_by));
}

TEST_F(TranspilerTest, EmitSampleScanBernoulliPercentDirect) {
  // BERNOULLI sampling over PERCENT is the second DuckDB
  // method/unit combination the plan calls out. Direct
  // construction sidesteps any analyzer-surface variability around
  // method names other than SYSTEM. The expected SQL pins the
  // DuckDB shape `USING SAMPLE <n> PERCENT (bernoulli)`.
  TestSampleScanArgs args;
  args.method = "BERNOULLI";
  args.unit = ::googlesql::ResolvedSampleScan::PERCENT;
  args.size = ::googlesql::MakeResolvedLiteral(::googlesql::Value::Int64(25));
  auto sample = MakeTestSampleScan(std::move(args));
  ASSERT_NE(sample, nullptr);
  TestTranspiler t;
  EXPECT_EQ(t.EmitSampleScan(sample.get()),
            "SELECT * FROM (SELECT 1) USING SAMPLE 25 PERCENT (bernoulli)");
}

TEST_F(TranspilerTest, EmitSampleScanReservoirRowsDirect) {
  // RESERVOIR over ROWS. DuckDB picks reservoir sampling to hit an
  // exact row count, matching the BQ `RESERVOIR` semantics for
  // ROWS-shape sampling. We construct directly so the assertion
  // does not depend on the BQ surface accepting the `RESERVOIR
  // (50 ROWS)` form.
  TestSampleScanArgs args;
  args.method = "RESERVOIR";
  args.unit = ::googlesql::ResolvedSampleScan::ROWS;
  args.size = ::googlesql::MakeResolvedLiteral(::googlesql::Value::Int64(50));
  auto sample = MakeTestSampleScan(std::move(args));
  ASSERT_NE(sample, nullptr);
  TestTranspiler t;
  EXPECT_EQ(t.EmitSampleScan(sample.get()),
            "SELECT * FROM (SELECT 1) USING SAMPLE 50 ROWS (reservoir)");
}

TEST_F(TranspilerTest, EmitSampleScanSystemPercentDirect) {
  // SYSTEM over PERCENT through direct construction, mirroring the
  // surface-driven SYSTEM test so a future analyzer-side rewrite
  // of TABLESAMPLE leaves the direct-construction assertion as a
  // stable contract.
  TestSampleScanArgs args;
  args.method = "SYSTEM";
  args.unit = ::googlesql::ResolvedSampleScan::PERCENT;
  args.size = ::googlesql::MakeResolvedLiteral(::googlesql::Value::Int64(5));
  auto sample = MakeTestSampleScan(std::move(args));
  ASSERT_NE(sample, nullptr);
  TestTranspiler t;
  EXPECT_EQ(t.EmitSampleScan(sample.get()),
            "SELECT * FROM (SELECT 1) USING SAMPLE 5 PERCENT (system)");
}

TEST_F(TranspilerTest, EmitSampleScanReservoirPercentMismatchFallsBack) {
  // RESERVOIR with PERCENT does not have a clean DuckDB analog --
  // reservoir sampling targets a specific row count -- so we fall
  // back rather than emit `USING SAMPLE N PERCENT (reservoir)`,
  // which DuckDB rejects at parse time.
  TestSampleScanArgs args;
  args.method = "RESERVOIR";
  args.unit = ::googlesql::ResolvedSampleScan::PERCENT;
  args.size = ::googlesql::MakeResolvedLiteral(::googlesql::Value::Int64(10));
  auto sample = MakeTestSampleScan(std::move(args));
  ASSERT_NE(sample, nullptr);
  TestTranspiler t;
  EXPECT_EQ(t.EmitSampleScan(sample.get()), "");
}

TEST_F(TranspilerTest, EmitSampleScanSystemRowsMismatchFallsBack) {
  // SYSTEM with ROWS has no DuckDB equivalent (system sampling is
  // a percent-form block sampler). Bail so the engine surfaces
  // UNIMPLEMENTED for the whole query.
  TestSampleScanArgs args;
  args.method = "SYSTEM";
  args.unit = ::googlesql::ResolvedSampleScan::ROWS;
  args.size = ::googlesql::MakeResolvedLiteral(::googlesql::Value::Int64(100));
  auto sample = MakeTestSampleScan(std::move(args));
  ASSERT_NE(sample, nullptr);
  TestTranspiler t;
  EXPECT_EQ(t.EmitSampleScan(sample.get()), "");
}

TEST_F(TranspilerTest, EmitSampleScanUnknownMethodFallsBack) {
  // Methods outside the {SYSTEM, BERNOULLI, RESERVOIR} matrix do
  // not have a DuckDB analog. The emit falls back rather than
  // emitting `USING SAMPLE ... (other)`, which DuckDB rejects.
  TestSampleScanArgs args;
  args.method = "OTHER";
  args.unit = ::googlesql::ResolvedSampleScan::PERCENT;
  args.size = ::googlesql::MakeResolvedLiteral(::googlesql::Value::Int64(10));
  auto sample = MakeTestSampleScan(std::move(args));
  ASSERT_NE(sample, nullptr);
  TestTranspiler t;
  EXPECT_EQ(t.EmitSampleScan(sample.get()), "");
}

TEST_F(TranspilerTest, EmitSampleScanWithRepeatableSeed) {
  // DuckDB's `REPEATABLE (<seed>)` clause pins the PRNG for
  // deterministic sampling; the transpiler forwards the seed
  // expression verbatim.
  TestSampleScanArgs args;
  args.method = "SYSTEM";
  args.unit = ::googlesql::ResolvedSampleScan::PERCENT;
  args.size = ::googlesql::MakeResolvedLiteral(::googlesql::Value::Int64(10));
  args.repeatable =
      ::googlesql::MakeResolvedLiteral(::googlesql::Value::Int64(42));
  auto sample = MakeTestSampleScan(std::move(args));
  ASSERT_NE(sample, nullptr);
  TestTranspiler t;
  EXPECT_EQ(t.EmitSampleScan(sample.get()),
            "SELECT * FROM (SELECT 1) USING SAMPLE 10 PERCENT (system, 42)");
}

TEST_F(TranspilerTest, EmitSampleScanWithWeightColumnFallsBack) {
  // BigQuery `WITH WEIGHT <col>` lowers to a `weight_column` on
  // the SampleScan. DuckDB has no native weighted-sampling
  // keyword on `USING SAMPLE`, so we fall back. The test uses a
  // synthetic ResolvedColumn for the weight column so the
  // assertion does not depend on a particular surface that exposes
  // weighted sampling.
  TestSampleScanArgs args;
  args.method = "SYSTEM";
  args.unit = ::googlesql::ResolvedSampleScan::PERCENT;
  args.size = ::googlesql::MakeResolvedLiteral(::googlesql::Value::Int64(10));
  ::googlesql::ResolvedColumn weight_col(
      /*column_id=*/1,
      /*table_name=*/::googlesql::IdString::MakeGlobal("$sample"),
      /*name=*/::googlesql::IdString::MakeGlobal("w"),
      type_factory_->get_double());
  args.weight = ::googlesql::MakeResolvedColumnHolder(weight_col);
  auto sample = MakeTestSampleScan(std::move(args));
  ASSERT_NE(sample, nullptr);
  TestTranspiler t;
  EXPECT_EQ(t.EmitSampleScan(sample.get()), "");
}

TEST_F(TranspilerTest, EmitSampleScanWithStratifyFallsBack) {
  // BigQuery STRATIFY-BY surface populates `partition_by_list`.
  // DuckDB's `USING SAMPLE` has no per-partition sampling clause,
  // so we fall back. We push one stratify expression onto the
  // list (a literal so the fallback assertion is about the list
  // being non-empty, not about a sub-expression failure).
  TestSampleScanArgs args;
  args.method = "SYSTEM";
  args.unit = ::googlesql::ResolvedSampleScan::PERCENT;
  args.size = ::googlesql::MakeResolvedLiteral(::googlesql::Value::Int64(10));
  args.partition_by.push_back(
      ::googlesql::MakeResolvedLiteral(::googlesql::Value::Int64(1)));
  auto sample = MakeTestSampleScan(std::move(args));
  ASSERT_NE(sample, nullptr);
  TestTranspiler t;
  EXPECT_EQ(t.EmitSampleScan(sample.get()), "");
}

TEST_F(TranspilerTest, EmitSampleScanUnloweredSizeFallsBack) {
  // A size expression we cannot lower (an untyped parameter)
  // propagates "" through `EmitExpr`; the SampleScan emit must
  // then return "" rather than emit `USING SAMPLE  PERCENT (...)`.
  TestSampleScanArgs args;
  args.method = "SYSTEM";
  args.unit = ::googlesql::ResolvedSampleScan::PERCENT;
  args.size = ::googlesql::MakeResolvedParameter(type_factory_->get_int64(),
                                                 /*name=*/"n",
                                                 /*position=*/0,
                                                 /*is_untyped=*/true);
  auto sample = MakeTestSampleScan(std::move(args));
  ASSERT_NE(sample, nullptr);
  TestTranspiler t;
  EXPECT_EQ(t.EmitSampleScan(sample.get()), "");
}

TEST_F(TranspilerTest, EmitSampleScanPercentVsRowsContrast) {
  // Execution-style contrast: PERCENT and ROWS produce different
  // DuckDB shapes for the same numeric value. We assert on the
  // surface forms so a regression in the unit selector surfaces
  // here. Both methods are direct-construction so we can pin the
  // exact emit shape regardless of analyzer-side rewrites.
  TestSampleScanArgs percent_args;
  percent_args.method = "BERNOULLI";
  percent_args.unit = ::googlesql::ResolvedSampleScan::PERCENT;
  percent_args.size =
      ::googlesql::MakeResolvedLiteral(::googlesql::Value::Int64(10));
  auto percent_sample = MakeTestSampleScan(std::move(percent_args));
  ASSERT_NE(percent_sample, nullptr);
  TestSampleScanArgs rows_args;
  rows_args.method = "RESERVOIR";
  rows_args.unit = ::googlesql::ResolvedSampleScan::ROWS;
  rows_args.size =
      ::googlesql::MakeResolvedLiteral(::googlesql::Value::Int64(10));
  auto rows_sample = MakeTestSampleScan(std::move(rows_args));
  ASSERT_NE(rows_sample, nullptr);
  TestTranspiler t_percent;
  TestTranspiler t_rows;
  std::string percent_sql = t_percent.EmitSampleScan(percent_sample.get());
  std::string rows_sql = t_rows.EmitSampleScan(rows_sample.get());
  EXPECT_NE(percent_sql, rows_sql);
  EXPECT_NE(percent_sql.find(" PERCENT "), std::string::npos);
  EXPECT_NE(rows_sql.find(" ROWS "), std::string::npos);
}

// --- ResolvedWithScan / ResolvedWithRefScan ----------------------------
//
// `docs/ENGINE_POLICY.md` Family 1. These tests pin the
// CTE emit shape end-to-end (`Transpile(stmt)` from a real
// `AnalyzeStatement` output) so a regression that changes the
// CTE-side anchor naming or the ref-scan-side rename surfaces as a
// string diff here. The CTE body projects each column to a
// positional anchor (`_cte_<idx>`) so per-reference name
// collisions across multiple `ResolvedWithRefScan`s cannot leak;
// `EmitWithRefScan` renames the anchor back to the analyzer's
// per-reference column names.

}  // namespace transpiler
}  // namespace duckdb
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
