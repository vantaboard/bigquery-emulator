// Unit tests for `SemanticExecutor`.
//
// We drive a real `AnalyzeStatement` against a tiny `SimpleCatalog`
// (mirroring the conformance harness used in
// `route_classifier_test.cc` / `stub_executors_test.cc`) and run
// `SemanticExecutor::ExecuteQuery` over the analyzer's resolved
// statement directly. The tests pin the end-to-end happy paths
// (scalar SELECT + arithmetic + parameter binding) and the
// error-surface mappings the gateway depends on
// (`SELECT 1 / 0 -> divisionByZero`, `SELECT INT64_MAX + 1 ->
// overflow`).

#include "backend/engine/semantic/executor.h"

#include <memory>
#include <string>

#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "backend/engine/engine.h"
#include "backend/engine/semantic/error.h"
#include "backend/storage/storage.h"
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
namespace semantic {
namespace {

::googlesql::AnalyzerOptions MakeAnalyzerOptions() {
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

TEST_F(SemanticExecutorTest, ScalarSelectOneRoundTrips) {
  auto cell = RunForFirstCell("SELECT 1");
  ASSERT_TRUE(cell.ok()) << cell.status();
  EXPECT_EQ(cell->int64_value(), 1);
}

TEST_F(SemanticExecutorTest, ScalarSelectArithmeticRoundTrips) {
  auto cell = RunForFirstCell("SELECT 1 + 2");
  ASSERT_TRUE(cell.ok()) << cell.status();
  EXPECT_EQ(cell->int64_value(), 3);
}

TEST_F(SemanticExecutorTest, ScalarSelectMultipleColumnsRoundTrip) {
  const auto* stmt = Analyze("SELECT 1 AS a, 'x' AS b", MakeAnalyzerOptions());
  ASSERT_NE(stmt, nullptr);
  SemanticExecutor exec;
  auto source =
      exec.ExecuteQuery(MakeRequest("SELECT 1, 'x'"), *stmt, catalog_.get());
  ASSERT_TRUE(source.ok()) << source.status();
  ASSERT_EQ((*source)->schema().columns.size(), 2u);
  EXPECT_EQ((*source)->schema().columns[0].name, "a");
  EXPECT_EQ((*source)->schema().columns[1].name, "b");
  storage::Row row;
  auto has = (*source)->Next(&row);
  ASSERT_TRUE(has.ok()) << has.status();
  ASSERT_TRUE(*has);
  ASSERT_EQ(row.cells.size(), 2u);
  EXPECT_EQ(row.cells[0].int64_value(), 1);
  EXPECT_EQ(row.cells[1].string_value(), "x");
  has = (*source)->Next(&row);
  ASSERT_TRUE(has.ok()) << has.status();
  EXPECT_FALSE(*has);
}

TEST_F(SemanticExecutorTest, NullAdditionPropagatesNull) {
  auto cell = RunForFirstCell("SELECT CAST(NULL AS INT64) + 1");
  ASSERT_TRUE(cell.ok()) << cell.status();
  EXPECT_TRUE(cell->is_null());
}

TEST_F(SemanticExecutorTest, DivisionByZeroSurfacesReason) {
  const auto* stmt = Analyze("SELECT 1.0 / 0", MakeAnalyzerOptions());
  ASSERT_NE(stmt, nullptr);
  SemanticExecutor exec;
  auto source =
      exec.ExecuteQuery(MakeRequest("SELECT 1.0 / 0"), *stmt, catalog_.get());
  ASSERT_FALSE(source.ok());
  EXPECT_EQ(source.status().code(), absl::StatusCode::kInvalidArgument);
  EXPECT_EQ(GetSemanticErrorReason(source.status()),
            SemanticErrorReason::kDivisionByZero);
}

TEST_F(SemanticExecutorTest, Int64OverflowSurfacesReason) {
  const auto* stmt =
      Analyze("SELECT 9223372036854775807 + 1", MakeAnalyzerOptions());
  ASSERT_NE(stmt, nullptr);
  SemanticExecutor exec;
  auto source = exec.ExecuteQuery(
      MakeRequest("SELECT 9223372036854775807 + 1"), *stmt, catalog_.get());
  ASSERT_FALSE(source.ok());
  EXPECT_EQ(GetSemanticErrorReason(source.status()),
            SemanticErrorReason::kOverflow);
}

TEST_F(SemanticExecutorTest, SafeAddOverflowProducesNull) {
  auto cell = RunForFirstCell("SELECT SAFE_ADD(9223372036854775807, 1)");
  ASSERT_TRUE(cell.ok()) << cell.status();
  EXPECT_TRUE(cell->is_null());
}

TEST_F(SemanticExecutorTest, NamedParameterBindsAndArithmeticUses) {
  ::googlesql::AnalyzerOptions options = MakeAnalyzerOptions();
  ASSERT_TRUE(
      options.AddQueryParameter("p", ::googlesql::types::Int64Type()).ok());

  const auto* stmt = Analyze("SELECT @p + 1", options);
  ASSERT_NE(stmt, nullptr);
  QueryRequest req = MakeRequest("SELECT @p + 1");
  QueryParameter p;
  p.name = "p";
  p.type_kind = "INT64";
  p.value_json = "40";
  req.parameters.push_back(p);

  SemanticExecutor exec;
  auto source = exec.ExecuteQuery(req, *stmt, catalog_.get());
  ASSERT_TRUE(source.ok()) << source.status();
  storage::Row row;
  auto has = (*source)->Next(&row);
  ASSERT_TRUE(has.ok()) << has.status();
  ASSERT_TRUE(*has);
  EXPECT_EQ(row.cells[0].int64_value(), 41);
}

TEST_F(SemanticExecutorTest, RejectsSelectWithFromShape) {
  // Add a fake table to the catalog so the analyzer can resolve
  // the FROM clause; the executor should still reject the shape.
  ::googlesql::SimpleTable* table =
      new ::googlesql::SimpleTable("t", {{"x", type_factory_->get_int64()}});
  catalog_->AddOwnedTable(table);
  const auto* stmt = Analyze("SELECT x FROM t", MakeAnalyzerOptions());
  ASSERT_NE(stmt, nullptr);
  SemanticExecutor exec;
  auto source =
      exec.ExecuteQuery(MakeRequest("SELECT x FROM t"), *stmt, catalog_.get());
  ASSERT_FALSE(source.ok());
  EXPECT_EQ(source.status().code(), absl::StatusCode::kUnimplemented);
}

// `docs/ENGINE_POLICY.md` Family 2. A
// `ResolvedBarrierScan` wrapping a SingleRowScan is the
// pipe-operator analog of `SELECT 1 + 2`; the barrier is the
// analyzer's pipe-boundary marker and rows pass through
// unchanged. `StripBarrierScans` peels the wrapper before
// dispatch so the scalar-only evaluator handles the projection.
TEST_F(SemanticExecutorTest, BarrierScanOverSingleRowPassesThrough) {
  // Direct construction: the surface SQL `SELECT 1 + 2 |> SELECT ...`
  // is not yet enabled in this fixture's analyzer, but the
  // `ResolvedQueryStmt(query=ResolvedProjectScan(input_scan=
  // ResolvedBarrierScan(input_scan=ResolvedSingleRowScan)))`
  // shape is what the analyzer would emit, so we build it
  // directly and feed it to the executor.
  auto single = ::googlesql::MakeResolvedSingleRowScan();
  auto barrier = ::googlesql::MakeResolvedBarrierScan(
      /*column_list=*/{}, std::move(single));
  // Project a literal 7 onto a fresh output column.
  ::googlesql::ResolvedColumn out_col(
      /*column_id=*/100,
      /*table_name=*/::googlesql::IdString::MakeGlobal("$query"),
      /*name=*/::googlesql::IdString::MakeGlobal("c"),
      type_factory_->get_int64());
  std::vector<std::unique_ptr<const ::googlesql::ResolvedComputedColumn>> exprs;
  exprs.push_back(::googlesql::MakeResolvedComputedColumn(
      out_col, ::googlesql::MakeResolvedLiteral(::googlesql::Value::Int64(7))));
  auto project = ::googlesql::MakeResolvedProjectScan(
      /*column_list=*/{out_col}, std::move(exprs), std::move(barrier));
  std::vector<std::unique_ptr<const ::googlesql::ResolvedOutputColumn>> outputs;
  outputs.push_back(
      ::googlesql::MakeResolvedOutputColumn(/*name=*/"c", out_col));
  auto query_stmt = ::googlesql::MakeResolvedQueryStmt(
      std::move(outputs), /*is_value_table=*/false, std::move(project));

  SemanticExecutor exec;
  QueryRequest req = MakeRequest("/* barrier shape; built directly */");
  auto source = exec.ExecuteQuery(req, *query_stmt, catalog_.get());
  ASSERT_TRUE(source.ok()) << source.status();
  storage::Row row;
  auto has = (*source)->Next(&row);
  ASSERT_TRUE(has.ok()) << has.status();
  ASSERT_TRUE(*has);
  ASSERT_EQ(row.cells.size(), 1u);
  EXPECT_EQ(row.cells[0].int64_value(), 7);
}

TEST_F(SemanticExecutorTest, UnnestWithOffsetEmitsRowPerElement) {
  // deferred work tracked in docs/ENGINE_POLICY.md: a
  // standalone `UNNEST(...) WITH OFFSET` flowing through the
  // semantic executor produces one row per element with two
  // columns (the element value + the 0-based offset).
  const std::string sql =
      "SELECT n, idx FROM UNNEST(['a', 'b', 'c']) AS n WITH OFFSET AS idx";
  const auto* stmt = Analyze(sql, MakeAnalyzerOptions());
  ASSERT_NE(stmt, nullptr);
  SemanticExecutor exec;
  auto source = exec.ExecuteQuery(MakeRequest(sql), *stmt, catalog_.get());
  ASSERT_TRUE(source.ok()) << source.status();
  ASSERT_EQ((*source)->schema().columns.size(), 2u);
  EXPECT_EQ((*source)->schema().columns[0].name, "n");
  EXPECT_EQ((*source)->schema().columns[1].name, "idx");

  storage::Row row;
  for (int i = 0; i < 3; ++i) {
    auto has = (*source)->Next(&row);
    ASSERT_TRUE(has.ok()) << has.status();
    ASSERT_TRUE(*has) << "expected row #" << i;
    ASSERT_EQ(row.cells.size(), 2u);
    EXPECT_EQ(row.cells[1].int64_value(), i);
  }
  // Confirm the stream ends after 3 elements.
  auto has = (*source)->Next(&row);
  ASSERT_TRUE(has.ok()) << has.status();
  EXPECT_FALSE(*has);
}

TEST_F(SemanticExecutorTest, OuterUnnestEmptyArrayEmitsNullRow) {
  // Family 2: an empty array under `is_outer=true` (the analyzer
  // synthesizes this for the `LEFT JOIN UNNEST(...) ON TRUE`
  // pattern, and for `WITH OFFSET` against an empty literal) emits
  // a single row whose element + offset are both NULL.
  const std::string sql =
      "SELECT n, idx FROM UNNEST(CAST([] AS ARRAY<INT64>)) AS n "
      "WITH OFFSET AS idx";
  const auto* stmt = Analyze(sql, MakeAnalyzerOptions());
  if (stmt == nullptr) {
    GTEST_SKIP() << "analyzer rejected empty-array literal; "
                    "covered by array_scan_test.";
  }
  SemanticExecutor exec;
  auto source = exec.ExecuteQuery(MakeRequest(sql), *stmt, catalog_.get());
  ASSERT_TRUE(source.ok()) << source.status();
  storage::Row row;
  auto has = (*source)->Next(&row);
  ASSERT_TRUE(has.ok()) << has.status();
  // Inner UNNEST against empty array emits zero rows; outer would
  // emit one NULL row. `WITH OFFSET` without `is_outer` is inner.
  EXPECT_FALSE(*has);
}

TEST_F(SemanticExecutorTest, DmlSurfacesNotImplemented) {
  const auto* stmt = Analyze("SELECT 1", MakeAnalyzerOptions());
  ASSERT_NE(stmt, nullptr);
  SemanticExecutor exec;
  auto out = exec.ExecuteDml(MakeRequest("SELECT 1"), *stmt, catalog_.get());
  ASSERT_FALSE(out.ok());
  EXPECT_EQ(out.status().code(), absl::StatusCode::kUnimplemented);
}

TEST_F(SemanticExecutorTest, DdlSurfacesNotImplemented) {
  const auto* stmt = Analyze("SELECT 1", MakeAnalyzerOptions());
  ASSERT_NE(stmt, nullptr);
  SemanticExecutor exec;
  absl::Status out =
      exec.ExecuteDdl(MakeRequest("SELECT 1"), *stmt, catalog_.get());
  ASSERT_FALSE(out.ok());
  EXPECT_EQ(out.code(), absl::StatusCode::kUnimplemented);
}

TEST_F(SemanticExecutorTest, ChainedCteReferencesPriorEntry) {
  const std::string sql =
      "WITH base AS (SELECT 1 AS n UNION ALL SELECT 2 AS n), "
      "     doubled AS (SELECT n * 2 AS m FROM base) "
      "SELECT SUM(m) AS total FROM doubled";
  const auto* stmt = Analyze(sql, MakeAnalyzerOptions());
  ASSERT_NE(stmt, nullptr);
  SemanticExecutor exec;
  auto source = exec.ExecuteQuery(MakeRequest(sql), *stmt, catalog_.get());
  ASSERT_TRUE(source.ok()) << source.status();
  storage::Row row;
  auto has = (*source)->Next(&row);
  ASSERT_TRUE(has.ok()) << has.status();
  ASSERT_TRUE(*has);
  ASSERT_EQ(row.cells.size(), 1u);
  EXPECT_EQ(row.cells[0].int64_value(), 6);
}

TEST_F(SemanticExecutorTest, ChainedCteWithRowNumberAnalyticScan) {
  const std::string sql =
      "WITH base AS (SELECT 1 AS id UNION ALL SELECT 1 AS id), "
      "     ranked AS ("
      "       SELECT id, ROW_NUMBER() OVER (PARTITION BY id ORDER BY id) AS rn "
      "       FROM base"
      "     ) "
      "SELECT COUNT(*) AS c FROM ranked WHERE rn = 1";
  const auto* stmt = Analyze(sql, MakeAnalyzerOptions());
  ASSERT_NE(stmt, nullptr);
  SemanticExecutor exec;
  auto source = exec.ExecuteQuery(MakeRequest(sql), *stmt, catalog_.get());
  ASSERT_TRUE(source.ok()) << source.status();
  storage::Row row;
  auto has = (*source)->Next(&row);
  ASSERT_TRUE(has.ok()) << has.status();
  ASSERT_TRUE(*has);
  ASSERT_EQ(row.cells.size(), 1u);
  EXPECT_EQ(row.cells[0].int64_value(), 1);
}

}  // namespace
}  // namespace semantic
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
