---
name: FixTests 05 — Scalar + subquery semantic executor
overview: Close the four scalar/subquery first-party conformance failures that route through the semantic executor - WITH-expr evaluation, CAST(STRING AS BOOL) + float formatting, and the ARRAY/SCALAR/IN subquery-expression paths.
depends_on: [fixtests-02-quickwins]
est_effort: ~1 week
isProject: true
todos:
  - id: with-expr
    content: Implement RESOLVED_WITH_EXPR in the semantic EvalExpr dispatch (or stop promoting scalar-only WITH-expr queries to semantic_executor) so expr_with_expr stops hitting kNotImplemented.
    status: pending
  - id: cast-bool
    content: Implement CAST(STRING AS BOOL) in the semantic executor and fix float formatting (2.0 vs 2) so expr_cast_types passes.
    status: pending
  - id: subquery-array-scalar
    content: Harden EvalSubqueryExpr ARRAY and SCALAR paths (UNNEST materialization + element ordering) so subquery_expr_array and subquery_expr_scalar pass.
    status: pending
  - id: subquery-in
    content: Fix subquery_expr_in (DuckDB-native IN-subquery row shape) - it keeps the fast path since it has a FROM clause.
    status: pending
  - id: verify
    content: All four fixtures pass; re-run task conformance:run and confirm no regressions in the scalar/cte_subquery lanes.
    status: pending
---

# FixTests 05 — Scalar + subquery semantic executor

## Why

Four conformance failures are scalar expressions or subquery expressions routed to the semantic executor where a code path is missing or divergent. They share the same evaluation surface ([`backend/engine/semantic/`](backend/engine/semantic/)), so they batch cleanly.

> Validate each against the observed diff from [fixtests-01-foundation.plan.md](fixtests-01-foundation.plan.md).

## Fixtures and work

| Fixture | Work | Files |
|---------|------|-------|
| `expr_with_expr` | No `RESOLVED_WITH_EXPR` case in `EvalExpr` -> `kNotImplemented`. Implement it, or stop promoting scalar-only WITH-expr to semantic so it stays DuckDB-native. | [`eval_expr.cc`](backend/engine/semantic/eval_expr.cc), [`route_classifier_visitor.cc`](backend/engine/coordinator/route_classifier_visitor.cc) |
| `expr_cast_types` | `CAST('TRUE' AS BOOL)` not implemented (only BOOL->STRING); also `i_to_f` may render `"2"` instead of `"2.0"`. | [`eval_expr.cc`](backend/engine/semantic/eval_expr.cc) (cast eval), wire formatting |
| `subquery_expr_array` | `ARRAY(SELECT n FROM UNNEST(...) ORDER BY n)` — ARRAY subquery element ordering / UNNEST materialization in the semantic path. | [`eval_expr.cc`](backend/engine/semantic/eval_expr.cc) `EvalSubqueryExpr` |
| `subquery_expr_scalar` | `(SELECT MAX(n) FROM UNNEST(...))` — SCALAR subquery path. | same |
| `subquery_expr_in` | `n IN (SELECT ...)` stays DuckDB-native (has FROM); likely a transpiler row-shape fix. | [`backend/engine/duckdb/transpiler/`](backend/engine/duckdb/transpiler/) |

(`subquery_expr_in` is grouped here because it is the third file in [`conformance/fixtures/cte_subquery/`](conformance/fixtures/cte_subquery/); fix it alongside the array/scalar paths.)

## Steps

1. Reproduce each with `task conformance:run-fixture FIXTURE=...` and read the exact diff.
2. Implement `RESOLVED_WITH_EXPR` (preferred) or adjust routing; add the `CAST(STRING AS BOOL)` branch and float formatting.
3. Audit `EvalSubqueryExpr` ARRAY/SCALAR materialization and ordering; fix the IN-subquery transpile shape.
4. If routing dispositions change, update [`SHAPE_TRACKER.md`](backend/engine/duckdb/transpiler/SHAPE_TRACKER.md) + `node_dispositions.yaml` and keep `tools/check_disposition_parity` green.
5. Add/keep a focused first-party fixture per behavior; update `docs/ENGINE_POLICY.md` if a shape's route changes.

## Verify

```bash
task emulator:build-engine:bazel
for f in scalar/expr_with_expr scalar/expr_cast_types \
         cte_subquery/subquery_expr_array cte_subquery/subquery_expr_scalar cte_subquery/subquery_expr_in; do
  task conformance:run-fixture FIXTURE=conformance/fixtures/$f.yaml
done
task conformance:run     # no regressions
```

## Out of scope

- Fastpath scans + nested struct (plan 06); pivot/unpivot/recursive (plan 07).
