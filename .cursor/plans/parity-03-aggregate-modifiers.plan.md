---
name: Parity 03 — Aggregate modifiers
overview: Land the deferred ResolvedAggregateFunctionCall modifier matrix - ORDER BY / LIMIT inside ARRAY_AGG / STRING_AGG, IGNORE/RESPECT NULLS, HAVING MAX/MIN, aggregate FILTER, SAFE.<agg>, and multi-level GROUP BY - choosing duckdb_native lowering where DuckDB's modifier semantics already match and semantic_executor where they do not.
depends_on: [parity-02-function-polyfills]
est_effort: ~1 week
isProject: true
todos:
  - id: matrix-probe
    content: "Probe DuckDB's native modifier support per shape (ORDER BY inside aggregate, FILTER clause, arg-level DISTINCT) against BigQuery semantics; record the per-shape route decision before writing code."
    status: completed
  - id: order-by-limit
    content: "ARRAY_AGG(x ORDER BY y [LIMIT n]) and STRING_AGG(x, sep ORDER BY y [LIMIT n]): lower natively where DuckDB's ORDER BY-in-aggregate matches; LIMIT modifier likely needs a rewrite or semantic path."
    status: completed
  - id: ignore-respect-nulls
    content: "IGNORE/RESPECT NULLS on ARRAY_AGG / analytic aggregates: ARRAY_AGG default differs (BigQuery errors on NULL element without IGNORE NULLS in some contexts) - pin exact behavior with fixtures."
    status: completed
  - id: having-max-min
    content: "ResolvedAggregateHavingModifier (HAVING MAX / HAVING MIN inside aggregate): semantic executor implementation; flip the (planned) row."
    status: completed
  - id: safe-agg-filter
    content: "SAFE.<agg>(...) and aggregate filtering: route to semantic executor; reuse the SAFE short-circuit contract from the scalar path."
    status: completed
  - id: analytic-ignore-nulls
    content: "IGNORE/RESPECT NULLS + SAFE on ResolvedAnalyticFunctionCall (LAG/LEAD/FIRST_VALUE/LAST_VALUE/NTH_VALUE) - DuckDB supports IGNORE NULLS natively on several; verify and lower natively where exact."
    status: completed
  - id: fixtures-trackers
    content: Fixtures under conformance/fixtures/aggregate/ per modifier with expected.route; update SHAPE_TRACKER ResolvedAggregateFunctionCall / ResolvedAnalyticFunctionCall / ResolvedAggregateHavingModifier rows + node_dispositions.yaml.
    status: completed
---

# Parity 03 — Aggregate modifiers

## Why

SHAPE_TRACKER `ResolvedAggregateFunctionCall` defers the entire
modifier matrix: *"HAVING MAX/MIN, ORDER BY / LIMIT modifiers,
IGNORE/RESPECT NULLS, multi-level GROUP BY, aggregate filtering,
`SAFE.<agg>(...)` reroute to `semantic_executor` (planned)"*.
`ARRAY_AGG(x ORDER BY y)` and `STRING_AGG(x ORDER BY y)` are ubiquitous
in analytics SQL; today they all surface UNIMPLEMENTED.

## Key files

- [`backend/engine/duckdb/transpiler/`](../../backend/engine/duckdb/transpiler/) — aggregate emit (`transpiler-emit-join-agg` surface)
- [`backend/engine/coordinator/route_classifier_visitor.cc`](../../backend/engine/coordinator/route_classifier_visitor.cc) — currently promotes modifier shapes; relax per shape as native lowering lands
- [`backend/engine/semantic/`](../../backend/engine/semantic/) — `EvalSqlUdafBody` neighborhood for the semantic-side aggregate evaluator
- `conformance/fixtures/aggregate/`

## Steps

### Modifier route matrix (probe outcome)

| Modifier × function | Route | Lowering / notes |
|---------------------|-------|------------------|
| ORDER BY + LIMIT on `ARRAY_AGG` / `STRING_AGG` / `ARRAY_CONCAT_AGG` | `duckdb_native` | `list(... ORDER BY ...)` + optional `list_slice(..., 1, n)`; STRING_AGG via `array_to_string` |
| IGNORE NULLS on `ARRAY_AGG` | `duckdb_native` | `FILTER (WHERE arg IS NOT NULL)` |
| RESPECT NULLS on `ARRAY_AGG` | `duckdb_native` | `WrapArrayAggRespectNulls` error guard |
| Aggregate filtering (`SUM(x WHERE cond)`) | `duckdb_native` | DuckDB `FILTER (WHERE cond)` |
| `SAFE.SUM(...)` | `duckdb_native` | `TRY(SUM(...))` |
| Other `SAFE.<agg>(...)` | `semantic_executor` | Route classifier promotion |
| HAVING MAX / HAVING MIN | `semantic_executor` | `FilterRowsByHavingModifier` then aggregate eval |
| Multi-level GROUP BY inside aggregate | `semantic_executor` | Blocked (planned) |
| IGNORE/RESPECT NULLS on FIRST_VALUE/LAST_VALUE/NTH_VALUE | `duckdb_native` | DuckDB `IGNORE NULLS` / `RESPECT NULLS` keywords |
| IGNORE/RESPECT NULLS on LAG/LEAD | blocked | Googlesql catalog does not expose null-handling on LAG/LEAD |
| IGNORE/RESPECT NULLS on other analytics / `SAFE` analytics | `semantic_executor` | Blocked |

1. Build the decision table first (todo `matrix-probe`): for each
   modifier × function combination, test DuckDB's native form against
   documented BigQuery behavior (NULL handling, stability of ties,
   LIMIT-after-ORDER semantics). Only exact matches stay
   `duckdb_native`; anything else lands on the semantic aggregate
   evaluator. Record decisions in the plan as you go.
2. Land `duckdb_native` lowerings (likely: `ORDER BY` inside
   ARRAY_AGG/STRING_AGG, FILTER for aggregate filtering, IGNORE NULLS
   on window functions) — pure transpiler work, biggest payoff.
3. Build/extend the semantic aggregate evaluator for the rest
   (HAVING MAX/MIN, LIMIT modifier, SAFE.<agg>), reusing the grouping
   machinery `EvalSqlUdafBody` already has.
4. Fixtures: one per modifier, covering NULL edge cases and the
   route label; flip tracker rows in the same commits.

## Verify

```bash
task emulator:build-engine:bazel
task conformance:run
task conformance:routing-matrix      # confirm intended route per new fixture
task lint:dispositions
task bazel:shutdown && task bazel:status
```

## Out of scope

- Skiplisted/unsupported aggregates (`APPROX_QUANTILES`, DP aggregates) — ENGINE_POLICY posture stands
- `WITH GROUP ROWS` (`ResolvedGroupRowsScan`) — plan 12
- Window frame non-literal bounds / RANGE non-numeric — plan 11
