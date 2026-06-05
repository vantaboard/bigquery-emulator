// Unit tests for the local `ResolvedArrayScan` evaluator.
//
// We drive a real `AnalyzeStatement` against a small
// `SimpleCatalog` (matching the pattern in
// `route_classifier_test.cc` / `executor_test.cc`) so the
// `ResolvedArrayScan*` the evaluator sees is the same shape the
// engine sees at runtime. The tests cover each shape family the
// classifier promotes to the semantic executor.

#include "backend/engine/semantic/array_struct/array_scan.h"

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "backend/engine/semantic/error.h"
#include "backend/engine/semantic/eval_expr.h"
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
#include "googlesql/resolved_ast/resolved_node_kind.pb.h"
#include "gtest/gtest.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace semantic {
namespace array_struct {
namespace {

::googlesql::AnalyzerOptions MakeAnalyzerOptions() {
  ::googlesql::LanguageOptions language;
  language.EnableMaximumLanguageFeatures();
  language.set_product_mode(::googlesql::PRODUCT_EXTERNAL);
  ::googlesql::AnalyzerOptions options(language);
  options.CreateDefaultArenasIfNotSet();
  return options;
}

class ArrayScanTest : public ::testing::Test {
 protected:
  void SetUp() override {
    type_factory_ = std::make_unique<::googlesql::TypeFactory>();
    catalog_ = std::make_unique<::googlesql::SimpleCatalog>(
        "exec_catalog", type_factory_.get());
    catalog_->AddBuiltinFunctions(
        ::googlesql::BuiltinFunctionOptions::AllReleasedFunctions());
  }

  const ::googlesql::ResolvedStatement* Analyze(absl::string_view sql) {
    last_output_.reset();
    absl::Status s = ::googlesql::AnalyzeStatement(sql,
                                                   MakeAnalyzerOptions(),
                                                   catalog_.get(),
                                                   type_factory_.get(),
                                                   &last_output_);
    EXPECT_TRUE(s.ok()) << s;
    if (!s.ok() || last_output_ == nullptr) return nullptr;
    return last_output_->resolved_statement();
  }

  // Walk the analyzed query down to the inner ResolvedArrayScan
  // (the executor's `ResolvedProjectScan(input_scan=
  // ResolvedArrayScan(...))` shape).
  const ::googlesql::ResolvedArrayScan* AnalyzeArrayScan(
      absl::string_view sql) {
    const auto* stmt = Analyze(sql);
    if (stmt == nullptr ||
        stmt->node_kind() != ::googlesql::RESOLVED_QUERY_STMT) {
      return nullptr;
    }
    const auto& q = *stmt->GetAs<::googlesql::ResolvedQueryStmt>();
    const ::googlesql::ResolvedScan* query = q.query();
    if (query == nullptr ||
        query->node_kind() != ::googlesql::RESOLVED_PROJECT_SCAN) {
      return nullptr;
    }
    const auto& p = *query->GetAs<::googlesql::ResolvedProjectScan>();
    if (p.input_scan() == nullptr ||
        p.input_scan()->node_kind() != ::googlesql::RESOLVED_ARRAY_SCAN) {
      return nullptr;
    }
    return p.input_scan()->GetAs<::googlesql::ResolvedArrayScan>();
  }

  std::unique_ptr<::googlesql::TypeFactory> type_factory_{};
  std::unique_ptr<::googlesql::SimpleCatalog> catalog_{};
  std::unique_ptr<const ::googlesql::AnalyzerOutput> last_output_{};
};

TEST_F(ArrayScanTest, StandaloneUnnestEnumeratesElements) {
  // Even without WITH OFFSET, the executor walks
  // `ResolvedArrayScan` for the property-promoted shapes. Pin the
  // baseline: a 3-element literal array emits 3 rows with the
  // element column bound row-at-a-time.
  const auto* scan =
      AnalyzeArrayScan("SELECT n FROM UNNEST([10, 20, 30]) AS n");
  ASSERT_NE(scan, nullptr);
  EvalContext ctx;
  auto rows = EvaluateArrayScan(*scan, ctx);
  ASSERT_TRUE(rows.ok()) << rows.status();
  ASSERT_EQ(rows->size(), 3u);
  const int col_id = scan->element_column_list(0).column_id();
  EXPECT_EQ((*rows)[0].at(col_id).int64_value(), 10);
  EXPECT_EQ((*rows)[1].at(col_id).int64_value(), 20);
  EXPECT_EQ((*rows)[2].at(col_id).int64_value(), 30);
}

TEST_F(ArrayScanTest, UnnestWithOffsetBindsOffsetColumn) {
  // `UNNEST(...) WITH OFFSET AS idx` produces one row per element
  // with two bindings: the element column and the (0-based)
  // offset column. This is Family 1 of
  // `googlesqlite-12-arrays-generators.plan.md`.
  const auto* scan = AnalyzeArrayScan(
      "SELECT n, idx FROM UNNEST(['a', 'b', 'c']) AS n WITH OFFSET AS idx");
  ASSERT_NE(scan, nullptr);
  ASSERT_NE(scan->array_offset_column(), nullptr);
  EvalContext ctx;
  auto rows = EvaluateArrayScan(*scan, ctx);
  ASSERT_TRUE(rows.ok()) << rows.status();
  ASSERT_EQ(rows->size(), 3u);
  const int n_col = scan->element_column_list(0).column_id();
  const int idx_col = scan->array_offset_column()->column().column_id();
  EXPECT_EQ((*rows)[0].at(n_col).string_value(), "a");
  EXPECT_EQ((*rows)[0].at(idx_col).int64_value(), 0);
  EXPECT_EQ((*rows)[1].at(n_col).string_value(), "b");
  EXPECT_EQ((*rows)[1].at(idx_col).int64_value(), 1);
  EXPECT_EQ((*rows)[2].at(n_col).string_value(), "c");
  EXPECT_EQ((*rows)[2].at(idx_col).int64_value(), 2);
}

TEST_F(ArrayScanTest, EmptyArrayInnerProducesZeroRows) {
  // BigQuery contract: `FROM UNNEST([])` (inner UNNEST on empty
  // array) emits zero rows. DuckDB does the same on the standalone
  // shape; the test pins that the semantic executor matches.
  const auto* scan = AnalyzeArrayScan(
      "SELECT n, idx FROM UNNEST(CAST([] AS ARRAY<INT64>)) AS n WITH OFFSET "
      "AS idx");
  ASSERT_NE(scan, nullptr);
  EvalContext ctx;
  auto rows = EvaluateArrayScan(*scan, ctx);
  ASSERT_TRUE(rows.ok()) << rows.status();
  EXPECT_EQ(rows->size(), 0u);
}

// Note on `is_outer` coverage:
//
// `is_outer == true` is set by GoogleSQL only when the
// `ResolvedArrayScan` is the right-hand side of a
// `LEFT JOIN UNNEST(...)` shape, which inherently requires a
// non-trivial `input_scan` (correlated). That shape is gated by
// Family 4 of `googlesqlite-12-arrays-generators.plan.md`: until the
// correlated input-scan evaluator lands, `EvaluateArrayScan`
// surfaces `kNotImplemented`, so a syntactic `is_outer=true`
// fixture cannot exercise the empty-array NULL-row branch yet.
//
// The branch's implementation (`row_count == 0 && scan.is_outer()`
// -> emit single bindings row with NULL element and NULL offset)
// is in `array_scan.cc`; Family 4's tests will pin it via a
// `LEFT JOIN UNNEST(t.arr) WITH OFFSET AS idx` fixture when
// the correlated path ships.

TEST_F(ArrayScanTest, MultiArrayUnnestPadProducesPaddedRows) {
  // Family 3: multi-array zip with PAD (the default). The shorter
  // array pads with NULL element values; the longest determines
  // the row count.
  const auto* scan =
      AnalyzeArrayScan("SELECT * FROM UNNEST([1, 2, 3], [10, 20])");
  ASSERT_NE(scan, nullptr);
  ASSERT_GT(scan->array_expr_list_size(), 1);
  EvalContext ctx;
  auto rows = EvaluateArrayScan(*scan, ctx);
  ASSERT_TRUE(rows.ok()) << rows.status();
  ASSERT_EQ(rows->size(), 3u);
  const int a_col = scan->element_column_list(0).column_id();
  const int b_col = scan->element_column_list(1).column_id();
  EXPECT_EQ((*rows)[0].at(a_col).int64_value(), 1);
  EXPECT_EQ((*rows)[0].at(b_col).int64_value(), 10);
  EXPECT_EQ((*rows)[1].at(a_col).int64_value(), 2);
  EXPECT_EQ((*rows)[1].at(b_col).int64_value(), 20);
  EXPECT_EQ((*rows)[2].at(a_col).int64_value(), 3);
  EXPECT_TRUE((*rows)[2].at(b_col).is_null());
}

TEST_F(ArrayScanTest, MultiArrayUnnestTruncateDropsTail) {
  // The TRUNCATE mode caps row count at the shortest array.
  const auto* scan = AnalyzeArrayScan(
      "SELECT * FROM UNNEST([1, 2, 3], [10, 20], mode => 'TRUNCATE')");
  ASSERT_NE(scan, nullptr);
  EvalContext ctx;
  auto rows = EvaluateArrayScan(*scan, ctx);
  ASSERT_TRUE(rows.ok()) << rows.status();
  EXPECT_EQ(rows->size(), 2u);
}

TEST_F(ArrayScanTest, MultiArrayUnnestStrictRejectsMismatchedLengths) {
  // STRICT requires equal lengths; mismatched -> structured
  // INVALID_ARGUMENT.
  const auto* scan = AnalyzeArrayScan(
      "SELECT * FROM UNNEST([1, 2, 3], [10, 20], mode => 'STRICT')");
  ASSERT_NE(scan, nullptr);
  EvalContext ctx;
  auto rows = EvaluateArrayScan(*scan, ctx);
  ASSERT_FALSE(rows.ok());
  EXPECT_EQ(rows.status().code(), absl::StatusCode::kInvalidArgument);
  EXPECT_EQ(GetSemanticErrorReason(rows.status()),
            SemanticErrorReason::kInvalidArgument);
}

TEST_F(ArrayScanTest, CorrelatedInputScanSurfacesNotImplemented) {
  // Family 4 deferral: the FROM-table cross-join form is not
  // implemented yet. The evaluator surfaces `kNotImplemented`
  // with a pointer at the follow-up subagent.
  const ::googlesql::Type* int64_array = nullptr;
  ASSERT_TRUE(
      type_factory_->MakeArrayType(type_factory_->get_int64(), &int64_array)
          .ok());
  auto arr_tab = std::make_unique<::googlesql::SimpleTable>(
      "arr_tab",
      std::vector<::googlesql::SimpleTable::NameAndType>{
          {"id", type_factory_->get_int64()},
          {"arr", int64_array},
      });
  catalog_->AddOwnedTable(std::move(arr_tab));
  const auto* scan =
      AnalyzeArrayScan("SELECT id, n FROM arr_tab, UNNEST(arr_tab.arr) AS n");
  ASSERT_NE(scan, nullptr);
  EvalContext ctx;
  auto rows = EvaluateArrayScan(*scan, ctx);
  ASSERT_FALSE(rows.ok());
  EXPECT_EQ(rows.status().code(), absl::StatusCode::kUnimplemented);
}

}  // namespace
}  // namespace array_struct
}  // namespace semantic
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
