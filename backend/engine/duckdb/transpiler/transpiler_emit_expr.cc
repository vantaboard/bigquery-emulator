#include <algorithm>
#include <optional>
#include <string>
#include <vector>

#include "absl/container/flat_hash_map.h"
#include "absl/container/flat_hash_set.h"
#include "absl/log/log.h"
#include "absl/strings/ascii.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/str_join.h"
#include "absl/strings/str_replace.h"
#include "absl/strings/string_view.h"
#include "absl/time/time.h"
#include "backend/engine/disposition.h"
#include "backend/engine/duckdb/transpiler/functions.h"
#include "backend/engine/duckdb/transpiler/transpiler.h"
#include "backend/engine/duckdb/transpiler/transpiler_internal.h"
#include "backend/engine/duckdb/transpiler/types.h"
#include "googlesql/public/catalog.h"
#include "googlesql/public/function.h"
#include "googlesql/public/options.pb.h"
#include "googlesql/public/sql_function.h"
#include "googlesql/public/templated_sql_function.h"
#include "googlesql/public/type.h"
#include "googlesql/public/type.pb.h"
#include "googlesql/public/types/struct_type.h"
#include "googlesql/public/value.h"
#include "googlesql/resolved_ast/resolved_ast.h"
#include "googlesql/resolved_ast/resolved_ast_visitor.h"
#include "googlesql/resolved_ast/resolved_node.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace duckdb {
namespace transpiler {

std::string Transpiler::EmitCast(const ::googlesql::ResolvedCast* node) {
  // BigQuery `CAST(<expr> AS T)` lowers to DuckDB's `CAST(<expr> AS T)`
  // for every `TypeKind` we have a first-class DuckDB analog for
  // (see `IsCastTargetSupported`). `SAFE_CAST(<expr> AS T)` sets
  // `return_null_on_error()` and lowers to DuckDB's `TRY_CAST(...)`,
  // which preserves the BigQuery contract of returning NULL on
  // conversion failure rather than raising.
  //
  // The richer cast surface needs bespoke rewrites we defer to a
  // follow-up plan; for now we propagate "" so the engine surfaces
  // UNIMPLEMENTED whenever any of these are set:
  //
  //   * `format()`        -- BigQuery's FORMAT clause for STRING/BYTES
  //                          cast templates differs from DuckDB's
  //                          (no DuckDB analog ships).
  //   * `time_zone()`     -- AT TIME ZONE on TIMESTAMP cast; needs
  //                          dedicated DuckDB function rewrite.
  //   * `extended_cast()` -- `TYPE_EXTENDED` family; out of scope.
  //   * `type_modifiers()`-- `STRING(2)`, `NUMERIC(38, 9)`,
  //                          collation modifiers; need parameter /
  //                          collation lower passes.
  //   * Unsupported target type -- proto / enum / range / graph /
  //                          measure / tokenlist / GEOGRAPHY all
  //                          fall through `DuckDBSqlTypeName` to
  //                          `VARCHAR`; emitting that silently
  //                          would not match BigQuery semantics.
  if (node == nullptr || node->expr() == nullptr) return "";
  if (node->format() != nullptr || node->time_zone() != nullptr) return "";
  if (node->extended_cast() != nullptr) return "";
  if (!node->type_modifiers().IsEmpty()) return "";
  const ::googlesql::Type* target = node->type();
  if (target == nullptr) return "";
  if (!internal::IsCastTargetSupported(target->kind())) return "";
  std::string inner = EmitExpr(node->expr());
  if (inner.empty()) return "";
  std::string type_sql = ToDuckDBSqlType(*target);
  if (type_sql.empty()) return "";
  if (target->kind() == ::googlesql::TYPE_TIMESTAMP) {
    if (auto lit = internal::TryLiteralString(node->expr())) {
      if (lit->find('+') == std::string::npos &&
          lit->find('Z') == std::string::npos &&
          lit->find("UTC") == std::string::npos &&
          (lit->size() < 6 ||
           lit->compare(lit->size() - 6, 6, "+00:00") != 0)) {
        inner = internal::QuoteString(absl::StrCat(*lit, "+00"));
      }
    }
    type_sql = "TIMESTAMPTZ";
  }
  if (target->kind() == ::googlesql::TYPE_STRUCT) {
    const ::googlesql::Type* source_type = node->expr()->type();
    if (source_type != nullptr && source_type->IsStruct()) {
      const ::googlesql::StructType* target_st = target->AsStruct();
      const ::googlesql::StructType* source_st = source_type->AsStruct();
      if (target_st != nullptr && source_st != nullptr) {
        std::string remapped = internal::EmitStructPositionalCastRemap(
            inner, *source_st, *target_st);
        if (!remapped.empty()) return remapped;
      }
    }
  }
  const char* op = node->return_null_on_error() ? "TRY_CAST" : "CAST";
  return absl::StrCat(op, "(", inner, " AS ", type_sql, ")");
}

std::string Transpiler::EmitMakeStruct(
    const ::googlesql::ResolvedMakeStruct* node) {
  // BigQuery `STRUCT(<expr> [AS <name>], ...)` lowers through the
  // analyzer to a `ResolvedMakeStruct` carrying the resolved
  // `StructType` (the source of truth for field names + order) plus
  // the parallel `field_list` of value expressions. DuckDB's struct
  // literal is `{'<name>': <value>, ...}`, keyed by name -- so the
  // emit walks the two lists in lockstep and stitches them onto the
  // DuckDB key/value syntax.
  //
  // BQ STRUCT semantics quirks we honor here, with the matching
  // SHAPE_TRACKER row documenting the fallback:
  //
  //   * BigQuery STRUCT field order is positional and the analyzer
  //     guarantees `field_list_size() == StructType::num_fields()`;
  //     we double-check defensively because a drift would silently
  //     produce a struct DuckDB rejects.
  //   * DuckDB STRUCTs *require* named fields; BigQuery permits
  //     anonymous ones (`STRUCT(1, 2)`). We synthesize a positional
  //     name (`_0`, `_1`, ...) via `ResolveStructFieldName` so the
  //     emit yields `{'_0': 1, '_1': 2}`; `EmitGetStructField` looks
  //     up the same synthesized name on the access side so positional
  //     field access composes byte-for-byte.
  //   * NULL field values propagate through `EmitExpr` /
  //     `EmitValueLiteral` as the literal `NULL`; DuckDB matches
  //     BigQuery's "absent NULL field" semantics for struct literals.
  if (node == nullptr) return "";
  const ::googlesql::Type* t = node->type();
  if (t == nullptr || !t->IsStruct()) return "";
  const ::googlesql::StructType* st = t->AsStruct();
  if (st == nullptr || st->num_fields() != node->field_list_size()) {
    return "";
  }
  std::vector<std::string> kvs;
  kvs.reserve(node->field_list_size());
  for (int i = 0; i < node->field_list_size(); ++i) {
    std::string v = EmitExpr(node->field_list(i));
    if (v.empty()) return "";
    kvs.push_back(absl::StrCat(
        internal::QuoteString(internal::ResolveStructFieldName(*st, i)),
        ": ",
        v));
  }
  return absl::StrCat("{", absl::StrJoin(kvs, ", "), "}");
}

std::string Transpiler::EmitGetStructField(
    const ::googlesql::ResolvedGetStructField* node) {
  // BigQuery `s.a` (or `s[OFFSET(0)]`) lowers to a
  // `ResolvedGetStructField` carrying the parent expression, the
  // 0-indexed `field_idx`, and `field_expr_is_positional` (which is
  // user-intent only; it does not change semantics). DuckDB's struct
  // field access is `<expr>.<name>` or `<expr>['<name>']`; we pick
  // the dotted form for readability.
  //
  // Named fields use the analyzer-supplied name verbatim. Anonymous
  // fields (empty `StructField::name`) resolve to the synthesized
  // positional name (`_<idx>`) via `ResolveStructFieldName`; that
  // matches the convention `EmitMakeStruct` and `EmitValueLiteral`
  // use on the construction side, so anonymous-field access
  // round-trips through DuckDB.
  if (node == nullptr || node->expr() == nullptr) return "";
  const ::googlesql::Type* base_type = node->expr()->type();
  if (base_type == nullptr || !base_type->IsStruct()) return "";
  const ::googlesql::StructType* st = base_type->AsStruct();
  if (st == nullptr) return "";
  int idx = node->field_idx();
  if (idx < 0 || idx >= st->num_fields()) return "";
  std::string base = EmitExpr(node->expr());
  if (base.empty()) return "";
  // Mark `field_expr_is_positional` accessed even though it carries
  // no semantic weight on the DuckDB side; the validator inside
  // `ResolvedAST::CheckFieldsAccessed` otherwise tears down deep
  // copies through the conformance harness.
  (void)node->field_expr_is_positional();
  return absl::StrCat(
      base,
      ".",
      internal::QuoteIdent(internal::ResolveStructFieldName(*st, idx)));
}

std::string Transpiler::EmitGetJsonField(
    const ::googlesql::ResolvedGetJsonField* node) {
  // BigQuery `<json>.<field>` lowers to a `ResolvedGetJsonField`
  // whose `expr()` carries the parent JSON expression and whose
  // `field_name()` is the unescaped field key. The analyzer also
  // sets `type()` on the node: BigQuery semantics keep the result
  // as JSON for `<json>.<field>` access (use `JSON_VALUE` or the
  // `JSON`-to-scalar cast suite for a scalar pull), so the typical
  // case lands with `type()->IsJson()`.
  //
  // DuckDB has two JSON access surfaces and the choice matters --
  // they differ on the *return type*, not on what they read:
  //
  //   * `<json> -> 'k'`      -> JSON     (json_extract)
  //   * `<json> ->> 'k'`     -> VARCHAR  (json_extract_string)
  //
  // We deliberately key off the resolved return type so the lowered
  // SQL preserves BQ's contract: a JSON-typed result emits `->`
  // (DuckDB returns JSON); a non-JSON-typed result emits `->>`
  // (DuckDB returns the scalar text). Picking the wrong operator
  // would leak DuckDB's auto-stringification into a query the user
  // expected to keep typed as JSON, or vice versa -- a silent
  // semantic drift the conformance harness would have to chase
  // down through serialization.
  //
  // Nested `<json>.a.b` chains compose naturally: the outer call
  // recurses through `EmitExpr` into another `EmitGetJsonField`
  // whose result is already a parenthesized `<json> -> 'a'`, and
  // we wrap our own emit in parentheses so a downstream operator
  // splice (`IS NULL`, `=`, function-arg) stays unambiguous against
  // DuckDB precedence rules.
  //
  // The string-literal field key uses `QuoteString`, which doubles
  // embedded `'` -- that's the same shape every other STRING
  // literal in the transpiler emits, so unicode / quote characters
  // round-trip without bespoke JSON-path escaping.
  if (node == nullptr || node->expr() == nullptr) return "";
  std::string inner = EmitExpr(node->expr());
  if (inner.empty()) return "";
  const ::googlesql::Type* result_type = node->type();
  const char* op =
      (result_type != nullptr && result_type->IsJson()) ? "->" : "->>";
  return absl::StrCat(
      "(", inner, " ", op, " ", internal::QuoteString(node->field_name()), ")");
}

std::string Transpiler::EmitSubqueryExpr(
    const ::googlesql::ResolvedSubqueryExpr* node) {
  // Non-correlated expression subqueries lower directly to DuckDB's
  // native subquery surface; each `ResolvedSubqueryExpr::SubqueryType`
  // has a one-line DuckDB analog whose semantics already match
  // BigQuery for the uncorrelated case:
  //
  //   * SCALAR  -> `(<sql>)`         (DuckDB raises on >1 row,
  //                                   matching BigQuery's runtime
  //                                   error for scalar subqueries
  //                                   that overflow).
  //   * IN      -> `<lhs> IN (<sql>)`
  //   * EXISTS  -> `EXISTS (<sql>)`
  //   * ARRAY   -> `ARRAY(<sql>)`    (DuckDB's `ARRAY(subquery)`
  //                                   builds a LIST whose element
  //                                   order matches the subquery's
  //                                   row order; BigQuery's ARRAY
  //                                   subquery preserves order when
  //                                   the subquery has an ORDER BY
  //                                   and is otherwise unordered --
  //                                   both engines agree on this).
  //
  // LIKE ANY / ALL / NOT LIKE ANY / ALL (BigQuery's
  // `<expr> LIKE ANY (<subquery>)`) are deliberately out of scope
  // for the transpiler; they fall through to the empty-string
  // contract today (surfacing UNIMPLEMENTED).
  //
  // Correlated subqueries (non-empty `parameter_list()`) belong to
  // the semantic executor (`docs/ENGINE_POLICY.md` Family 4,
  // deferred). The route classifier promotes any query containing a
  // correlated `ResolvedSubqueryExpr` to `kSemanticExecutor` before
  // the transpiler is ever asked to lower it -- but we defend in
  // depth and bail to "" if we somehow see a correlated form here.
  // Returning "" lets the engine's empty-string contract surface
  // UNIMPLEMENTED rather than emitting SQL that DuckDB would
  // evaluate against the wrong outer-row context.
  //
  // Hint lists (`hint_list_size > 0`) are user-supplied optimizer
  // hints with no DuckDB analog; we touch the accessor below so
  // `ResolvedAST::CheckFieldsAccessed` does not flag the read, but
  // we ignore the hints' content.
  if (node == nullptr || node->subquery() == nullptr) return "";
  if (node->parameter_list_size() > 0) return "";
  (void)node->hint_list_size();
  (void)node->in_collation();
  const bool saved_has_rn = input_has_rn_column_;
  const bool saved_rn_order = input_rn_ordering_;
  const bool saved_output_rn = output_includes_input_rn_;
  const bool saved_suppress_rn = suppress_rn_in_project_;
  const std::vector<std::string> saved_output_order = output_order_items_;
  input_has_rn_column_ = false;
  input_rn_ordering_ = false;
  output_includes_input_rn_ = false;
  suppress_rn_in_project_ = true;
  output_order_items_.clear();
  std::string inner = EmitScan(node->subquery());
  input_has_rn_column_ = saved_has_rn;
  input_rn_ordering_ = saved_rn_order;
  output_includes_input_rn_ = saved_output_rn;
  suppress_rn_in_project_ = saved_suppress_rn;
  output_order_items_ = saved_output_order;
  if (inner.empty()) return "";
  switch (node->subquery_type()) {
    case ::googlesql::ResolvedSubqueryExpr::SCALAR:
      return absl::StrCat("(", inner, ")");
    case ::googlesql::ResolvedSubqueryExpr::EXISTS:
      return absl::StrCat("EXISTS (", inner, ")");
    case ::googlesql::ResolvedSubqueryExpr::ARRAY:
      return absl::StrCat("ARRAY(", inner, ")");
    case ::googlesql::ResolvedSubqueryExpr::IN: {
      if (node->in_expr() == nullptr) return "";
      std::string lhs = EmitExpr(node->in_expr());
      if (lhs.empty()) return "";
      return absl::StrCat("(", lhs, " IN (", inner, "))");
    }
    default:
      // LIKE_ANY / LIKE_ALL / NOT_LIKE_ANY / NOT_LIKE_ALL:
      // deliberately out of scope for the transpiler; the
      // empty-string contract surfaces UNIMPLEMENTED.
      return "";
  }
}

std::string Transpiler::EmitWithExpr(
    const ::googlesql::ResolvedWithExpr* node) {
  // BigQuery `WITH(name AS <expr>, ...) <body>` is a scalar-context
  // expression that binds each `<expr>` once and then evaluates
  // `<body>` against those bindings. DuckDB has no `WITH ... <body>`
  // expression syntax, but a scalar subquery preserves the
  // once-per-row evaluation contract: we emit the inner SELECT to
  // expose every binding as a named column, then project the body
  // off it. Each `ResolvedColumnRef` inside the body lowers to the
  // bound column name via `EmitColumnRef`, so the names land where
  // the body expects them.
  //
  //   WITH(a AS <e1>, b AS <e2>) <body>
  //     ->
  //   (SELECT <body> FROM (SELECT <e1> AS "a", <e2> AS "b"))
  //
  // Falls back via the empty-string contract when any binding's
  // expression or the body cannot be lowered; an empty assignment
  // list is malformed (the analyzer rejects it upstream) and we
  // guard defensively.
  if (node == nullptr || node->expr() == nullptr) return "";
  if (node->assignment_list_size() == 0) return "";
  std::vector<std::string> assigns;
  assigns.reserve(node->assignment_list_size());
  for (int i = 0; i < node->assignment_list_size(); ++i) {
    const ::googlesql::ResolvedComputedColumn* cc = node->assignment_list(i);
    if (cc == nullptr || cc->expr() == nullptr) return "";
    std::string e = EmitExpr(cc->expr());
    if (e.empty()) return "";
    assigns.push_back(
        absl::StrCat(e,
                     " AS \"",
                     absl::StrReplaceAll(cc->column().name(), {{"\"", "\"\""}}),
                     "\""));
  }
  std::string body = EmitExpr(node->expr());
  if (body.empty()) return "";
  return absl::StrCat(
      "(SELECT ", body, " FROM (SELECT ", absl::StrJoin(assigns, ", "), "))");
}

std::string Transpiler::EmitFunctionArgument(
    const ::googlesql::ResolvedFunctionArgument* node) {
  // `ResolvedFunctionArgument` is the one-of wrapper the analyzer
  // produces for `generic_argument_list` slots (TVFs, lambda-bearing
  // function calls, descriptor / model / connection arguments, ...).
  // Today's emit pass only knows how to lower the `expr()` slot --
  // the wrapped scalar expression flows through `EmitExpr` like any
  // other ResolvedExpr so callers walking a generic argument list
  // can splice the result into their own SQL fragment.
  //
  // Every other slot (scan / model / connection / descriptor /
  // lambda / sequence / graph) needs bespoke lowering work that no
  // caller has proven yet; we propagate "" so the engine surfaces
  // UNIMPLEMENTED for the surrounding function call. The
  // `argument_alias()` (BigQuery's `F(<arg> AS <alias>)` syntax) is
  // similarly user-intent metadata with no DuckDB analog -- we
  // ignore it and keep the unwrapped expression's emit on the
  // expression path.
  if (node == nullptr) return "";
  if (node->expr() == nullptr) return "";
  if (node->scan() != nullptr || node->model() != nullptr ||
      node->connection() != nullptr || node->descriptor_arg() != nullptr ||
      node->inline_lambda() != nullptr || node->sequence() != nullptr ||
      node->graph() != nullptr) {
    return "";
  }
  // Touch the metadata slots the analyzer expects every consumer to
  // observe so `ResolvedAST::CheckFieldsAccessed` does not flag a
  // missed argument when this node round-trips through validation.
  (void)node->argument_column_list_size();
  (void)node->argument_alias();
  return EmitExpr(node->expr());
}

// Column / output shape -----------------------------------------------------

std::string Transpiler::EmitOutputColumn(
    const ::googlesql::ResolvedOutputColumn* node) {
  // Final user-visible alias mapping. The inner query exposes each
  // physical column under its `ResolvedColumn::name()`; the output
  // schema renames it to `node->name()`. We collapse the alias when
  // both are equal so the emitted SQL stays readable for the common
  // case (`SELECT id FROM people` -> the output name and the
  // physical column name both resolve to `"id"`).
  if (node == nullptr) return "";
  std::string col =
      join_output_uses_id_aliases_
          ? internal::JoinColumnIdAlias(node->column().column_id())
          : internal::QuoteIdent(node->column().name());
  if (node->name() == node->column().name()) {
    return col;
  }
  return absl::StrCat(col, " AS ", internal::QuoteIdent(node->name()));
}

std::string Transpiler::EmitComputedColumn(
    const ::googlesql::ResolvedComputedColumn* node) {
  // `column := expr` lowers to `<expr> AS "<column-name>"`. The
  // upstream `EmitProjectScan` calls this once per
  // `ResolvedComputedColumn` in `expr_list` and slots the result into
  // the SELECT list. The empty-string contract: any child expression
  // we cannot lower (a function outside the disposition table, a
  // parameter we have not implemented yet, ...) propagates back here
  // and we hand "" to the caller so the engine surfaces UNIMPLEMENTED
  // instead of emitting partial SQL.
  if (node == nullptr) return "";
  std::string expr = EmitExpr(node->expr());
  if (expr.empty()) return "";
  return absl::StrCat(
      expr, " AS ", internal::QuoteIdent(node->column().name()));
}

}  // namespace transpiler
}  // namespace duckdb
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
