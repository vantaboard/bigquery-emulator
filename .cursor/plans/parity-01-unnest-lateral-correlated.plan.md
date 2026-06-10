---
name: Parity 01 — UNNEST lateral / correlated subqueries
overview: Land the highest-frequency missing relational shapes - cross-join UNNEST (FROM t, UNNEST(t.arr)), LEFT JOIN UNNEST, multi-array zip, JOIN USING, lateral joins, and correlated subqueries - by building the semantic executor's outer-row iteration primitive they all share.
est_effort: ~1-2 weeks
isProject: true
todos:
  - id: outer-row-primitive
    content: Build the outer-row iteration primitive in backend/engine/semantic/ - evaluate an inner scan/expr once per outer row with the outer row's columns bound into the eval frame (generalize FrameStack binding from the UDF call path).
    status: pending
  - id: cross-join-unnest
    content: "FROM t, UNNEST(t.arr) [WITH OFFSET]: extend array_scan.cc to drive the primitive when ResolvedArrayScan has a non-SingleRowScan input_scan; preserve BigQuery element order and offset semantics."
    status: pending
  - id: left-join-unnest
    content: "LEFT JOIN UNNEST / is_outer ArrayScan: emit the NULL-padded outer row when the array is NULL/empty (BigQuery keeps the row; DuckDB unnest drops it)."
    status: pending
  - id: multi-array-zip
    content: "Multi-array UNNEST(a, b, ...) honoring array_zip_mode (PAD vs STRICT) on the semantic executor."
    status: pending
  - id: join-using
    content: "JOIN USING(...): evaluate whether DuckDB's native USING matches GoogleSQL's resolved shape (has_using is metadata-only on an already-resolved join_expr) - if so land as duckdb_native in the transpiler instead of semantic; otherwise route through the primitive."
    status: pending
  - id: correlated-subqueries
    content: "Correlated ResolvedSubqueryExpr (scalar/IN/EXISTS/ARRAY with non-empty parameter_list): replace the structured kNotImplemented in the semantic executor with per-outer-row evaluation via the primitive."
    status: pending
  - id: fixtures-trackers
    content: Add conformance fixtures (array_struct/ + cte_subquery/) with expected.route labels; flip SHAPE_TRACKER.md + node_dispositions.yaml rows; update ENGINE_POLICY.md Family 1 wording; drop matching third-party skip rows.
    status: pending
---

# Parity 01 — UNNEST lateral / correlated subqueries

## Why first

`FROM t, UNNEST(t.arr)` is arguably the single most common BigQuery
idiom that this emulator still rejects, and `JOIN USING` plus
correlated subqueries are close behind. All of these shapes were
deliberately deferred onto `semantic_executor` because their BigQuery
evaluation order does not map cleanly onto DuckDB's `LATERAL` /
`unnest(...)` model (SHAPE_TRACKER `ResolvedArrayScan` /
`ResolvedJoinScan` / `ResolvedSubqueryExpr` rows). They share one
missing primitive: **evaluate an inner relation/expression once per
outer row with the outer row's columns in scope**.

## Current state

| Shape | Today | Target route |
|-------|-------|--------------|
| `UNNEST(arr)` standalone | `duckdb_native` (landed) | — |
| `UNNEST ... WITH OFFSET` (standalone) | `semantic_executor` (landed, `array_scan.cc`) | — |
| `FROM t, UNNEST(t.arr)` (non-SingleRowScan `input_scan`) | UNIMPLEMENTED | `semantic_executor` |
| `LEFT JOIN UNNEST` (`is_outer` / `join_expr`) | UNIMPLEMENTED | `semantic_executor` |
| Multi-array zip (`array_zip_mode`) | UNIMPLEMENTED | `semantic_executor` |
| Lateral joins (`is_lateral`) / `JOIN USING` (`has_using`) | UNIMPLEMENTED | `semantic_executor` (USING: try `duckdb_native` first) |
| Correlated `ResolvedSubqueryExpr` (`parameter_list()` non-empty) | promoted to semantic, then structured `kNotImplemented` | `semantic_executor` |

## Key files

- [`backend/engine/semantic/array_struct/array_scan.cc`](../../backend/engine/semantic/array_struct/array_scan.cc) — existing UNNEST WITH OFFSET evaluator to extend
- [`backend/engine/semantic/`](../../backend/engine/semantic/) — `FrameStack`, `EvalExpr` / `EvalSubqueryExpr`
- [`backend/engine/coordinator/route_classifier_visitor.cc`](../../backend/engine/coordinator/route_classifier_visitor.cc) — already promotes these shapes; verify reasons stay accurate
- [`backend/engine/duckdb/transpiler/transpiler_emit_expr.cc`](../../backend/engine/duckdb/transpiler/) + join emit — only if `JOIN USING` lands `duckdb_native`
- `conformance/fixtures/array_struct/`, `conformance/fixtures/cte_subquery/`

## Steps

1. Design the outer-row iteration primitive: outer scan rows stream
   from DuckDB (the row source per ENGINE_POLICY); for each row, push a
   frame binding the outer `ResolvedColumn`s, evaluate the inner
   scan/expr, splice results. Mind NULL/empty-array padding for outer
   joins and `array_zip_mode` PAD/STRICT errors.
2. Land cross-join UNNEST first (largest payoff), then LEFT JOIN
   UNNEST, then zip mode.
3. Probe `JOIN USING` against DuckDB native lowering before defaulting
   to the semantic route — the analyzer resolves USING into an
   ordinary `join_expr`, so the fast path may already be semantically
   exact; if so this is a transpiler-only change (drop the `has_using`
   bail in the classifier).
4. Wire correlated `EvalSubqueryExpr` through the same primitive
   (scalar: exactly-one-row check; IN/EXISTS: short-circuit; ARRAY:
   ordered materialization).
5. Fixtures per shape with `expected.route`; flip tracker rows; run
   the verification matrix.

## Verify

```bash
task emulator:build-engine:bazel
task conformance:run-fixture FIXTURE=conformance/fixtures/array_struct/<new>.yaml   # each new fixture
task conformance:run          # no regressions
task lint:dispositions
task bazel:shutdown && task bazel:status
```

Third-party: re-run `task thirdparty:golang` / `:python` lanes whose
skip rows cite UNNEST/correlated gaps and remove the rows that now pass.

## Out of scope

- `ResolvedFlatten` / `ResolvedFlattenedArg` and inline lambdas (plan 12)
- LIKE ANY/ALL subqueries (plan 11)
- Recursive CTE `WITH DEPTH` (plan 11)
