#ifndef BIGQUERY_EMULATOR_BACKEND_ENGINE_DUCKDB_TRANSPILER_TRANSPILER_TEST_FIXTURE_H_
#define BIGQUERY_EMULATOR_BACKEND_ENGINE_DUCKDB_TRANSPILER_TRANSPILER_TEST_FIXTURE_H_

#include <memory>
#include <string>
#include <vector>

#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/string_view.h"
#include "backend/engine/disposition.h"
#include "backend/engine/duckdb/transpiler/functions.h"
#include "backend/engine/duckdb/transpiler/transpiler.h"
#include "backend/engine/duckdb/udf/registrar.h"
#include "duckdb.h"
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

// Mirrors `duckdb_engine::MakeAnalyzerOptions` so the tests
// resolve names through the same `LanguageOptions` snapshot the
// engine itself uses. Drifting these two breaks function dispatch
// (e.g. `IFNULL` resolves but `COALESCE` does not) in subtle ways
// that only surface in the conformance harness.
inline ::googlesql::AnalyzerOptions MakeAnalyzerOptions() {
  ::googlesql::LanguageOptions language;
  language.EnableMaximumLanguageFeatures();
  language.set_product_mode(::googlesql::PRODUCT_EXTERNAL);
  language.set_name_resolution_mode(::googlesql::NAME_RESOLUTION_DEFAULT);
  ::googlesql::AnalyzerOptions options(language);
  options.set_error_message_mode(::googlesql::ERROR_MESSAGE_ONE_LINE);
  // Match the engine: keep PIVOT / UNPIVOT in their raw resolved-AST
  // forms so the transpiler `EmitPivotScan` / `EmitUnpivotScan`
  // emit paths are exercised. The engine itself disables these
  // rewriters (see `local_coordinator_engine.cc::MakeAnalyzerOptions`)
  // because the disposition table routes the raw nodes through
  // `duckdb_rewrite`.
  options.disable_rewrite(::googlesql::REWRITE_PIVOT);
  options.disable_rewrite(::googlesql::REWRITE_UNPIVOT);
  options.CreateDefaultArenasIfNotSet();
  return options;
}

// Helper: synthesize a `ResolvedWithExpr` directly so tests do not
// depend on the analyzer preserving a `WITH(...)` expression against
// constant-folding / inlining heuristics.
struct TestWithExprBinding {
  std::string name;
  std::unique_ptr<const ::googlesql::ResolvedExpr> expr;
};

inline std::unique_ptr<::googlesql::ResolvedWithExpr> MakeTestWithExpr(
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

    const ::googlesql::Type* int64_array_type = nullptr;
    EXPECT_TRUE(
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

    // A table with a string discriminator + numeric value column so the
    // PIVOT / UNPIVOT tests have something the analyzer accepts for
    // `FOR <expr> IN (<literals>)` (PIVOT) and
    // `UNPIVOT(<value_cols> FOR <label_col> IN (<col_groups>))`
    // (UNPIVOT).
    auto sales = std::make_unique<::googlesql::SimpleTable>(
        "sales",
        std::vector<::googlesql::SimpleTable::NameAndType>{
            {"region", type_factory_->get_string()},
            {"kind", type_factory_->get_string()},
            {"amount", type_factory_->get_int64()},
        });
    catalog_->AddOwnedTable(std::move(sales));

    // Wide table for UNPIVOT: each column is one of the unpivot
    // arguments the analyzer threads through `unpivot_arg_list`.
    auto wide = std::make_unique<::googlesql::SimpleTable>(
        "wide",
        std::vector<::googlesql::SimpleTable::NameAndType>{
            {"region", type_factory_->get_string()},
            {"q1", type_factory_->get_int64()},
            {"q2", type_factory_->get_int64()},
        });
    catalog_->AddOwnedTable(std::move(wide));

    auto org = std::make_unique<::googlesql::SimpleTable>(
        "org",
        std::vector<::googlesql::SimpleTable::NameAndType>{
            {"employee", type_factory_->get_string()},
            {"manager", type_factory_->get_string()},
        });
    catalog_->AddOwnedTable(std::move(org));

    auto transactions = std::make_unique<::googlesql::SimpleTable>(
        "transactions",
        std::vector<::googlesql::SimpleTable::NameAndType>{
            {"timestamp", type_factory_->get_timestamp()},
            {"origin", type_factory_->get_string()},
            {"destination", type_factory_->get_string()},
            {"amount", type_factory_->get_numeric()},
        });
    catalog_->AddOwnedTable(std::move(transactions));

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
  using Transpiler::EmitPivotScan;
  using Transpiler::EmitProjectScan;
  using Transpiler::EmitQueryStmt;
  using Transpiler::EmitRecursiveRefScan;
  using Transpiler::EmitRecursiveScan;
  using Transpiler::EmitSampleScan;
  using Transpiler::EmitSetOperationScan;
  using Transpiler::EmitSingleRowScan;
  using Transpiler::EmitSubqueryExpr;
  using Transpiler::EmitTableScan;
  using Transpiler::EmitUnpivotScan;
  using Transpiler::EmitWithExpr;
  using Transpiler::EmitWithRefScan;
  using Transpiler::EmitWithScan;
};

// DuckDB-backed binding checker for composition / property tests. Opens an
// in-memory connection, registers polyfill UDFs, and asserts transpiled SQL
// binds (via duckdb_query, which runs parse + bind + plan).
class TranspilerBindFixture : public TranspilerTest {
 protected:
  void SetUp() override {
    TranspilerTest::SetUp();
    ASSERT_EQ(::duckdb_open(nullptr, &db_), ::DuckDBSuccess);
    ASSERT_EQ(::duckdb_connect(db_, &conn_), ::DuckDBSuccess);
    absl::Status reg = udf::RegisterAll(conn_);
    ASSERT_TRUE(reg.ok()) << reg;
  }

  void TearDown() override {
    if (conn_ != nullptr) ::duckdb_disconnect(&conn_);
    if (db_ != nullptr) ::duckdb_close(&db_);
    conn_ = nullptr;
    db_ = nullptr;
    TranspilerTest::TearDown();
  }

  void ExecDdl(absl::string_view sql) {
    ::duckdb_result result;
    ASSERT_EQ(::duckdb_query(conn_, std::string(sql).c_str(), &result),
              ::DuckDBSuccess)
        << ::duckdb_result_error(&result);
    ::duckdb_destroy_result(&result);
  }

  void AssertTranspileBinds(const ::googlesql::ResolvedStatement* stmt,
                            absl::string_view source_sql,
                            TestTranspiler* t) {
    ASSERT_NE(stmt, nullptr) << "analyze failed for:\n" << source_sql;
    std::string emitted = t->Transpile(stmt);
    ASSERT_FALSE(emitted.empty()) << "transpiler returned empty SQL for:\n"
                                  << source_sql;
    SCOPED_TRACE(emitted);
    ::duckdb_result result{};
    const auto rc = ::duckdb_query(conn_, emitted.c_str(), &result);
    if (rc != ::DuckDBSuccess) {
      const char* err = ::duckdb_result_error(&result);
      FAIL() << "DuckDB rejected transpiled SQL\n"
             << "source_sql:\n"
             << source_sql << "\n"
             << "emitted_sql:\n"
             << emitted << "\n"
             << "duckdb_error:\n"
             << (err == nullptr ? "(null)" : err);
    }
    ::duckdb_destroy_result(&result);
  }

  void AssertSqlTranspileBinds(absl::string_view sql) {
    const ::googlesql::ResolvedStatement* stmt = Analyze(sql);
    TestTranspiler t;
    AssertTranspileBinds(stmt, sql, &t);
  }

  ::duckdb_database db_ = nullptr;
  ::duckdb_connection conn_ = nullptr;
};

}  // namespace transpiler
}  // namespace duckdb
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator

#endif  // BIGQUERY_EMULATOR_BACKEND_ENGINE_DUCKDB_TRANSPILER_TRANSPILER_TEST_FIXTURE_H_
