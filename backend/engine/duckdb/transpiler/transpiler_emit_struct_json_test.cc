#include <gtest/gtest.h>

#include "backend/engine/duckdb/transpiler/transpiler_test_fixture.h"
#include "googlesql/resolved_ast/resolved_ast.h"
#include "googlesql/resolved_ast/resolved_node_kind.pb.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace duckdb {
namespace transpiler {

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
  // engine surfaces UNIMPLEMENTED for the surrounding function call.
  // This is the named-argument-only / TVF / lambda shape the plan
  // defers to a follow-up.
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
  // surfaces UNIMPLEMENTED rather than emitting partial SQL. The
  // analyzer doesn't produce this shape, but we guard so a future
  // change to the GetJsonField construction surface doesn't silently
  // emit `(<empty> -> ...)`.
  auto get = ::googlesql::MakeResolvedGetJsonField();
  TestTranspiler t;
  EXPECT_EQ(t.EmitGetJsonField(get.get()), "");
}

// --- Set operations ----------------------------------------------------

}  // namespace transpiler
}  // namespace duckdb
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
