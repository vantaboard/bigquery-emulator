---
name: Expand 09 — Measure functions
overview: Promote MEASURE types and AGGREGATE(<measure>) off unsupported. Implement the measure-type surface (CREATE TABLE / view columns typed as MEASURE, and the AGGREGATE(<measure>) call that materializes them) on the semantic executor over the existing aggregate infrastructure.
est_effort: ~2-3 weeks
isProject: true
todos:
  - id: measure-type
    content: "MEASURE type plumbing: model the MEASURE<T> column type through analysis, storage, and the catalog so a table/view can declare a measure column carrying a deferred aggregate expression. Decide the storable representation (the measure's defining aggregate expression + result type)."
    status: pending
  - id: aggregate-call
    content: "AGGREGATE(<measure>) evaluation: when a query references a measure via AGGREGATE(...), expand it into the underlying aggregate over the current grouping on the semantic executor (reuse aggregate_specialized.cc / the aggregate scan eval). Confirm grouping/window context composes correctly."
    status: pending
  - id: routing
    content: "Route classifier: confirm a query using a measure column / AGGREGATE() promotes to the semantic executor and that a measure used outside a valid aggregation context surfaces a BigQuery-shaped error rather than a wrong answer."
    status: pending
  - id: fixtures-trackers
    content: "Conformance fixtures: a measure-typed view column + AGGREGATE(<measure>) over a GROUP BY. Flip the measure rows off unsupported in the registries + SHAPE_TRACKER; update the ENGINE_POLICY MEASURE row + ROADMAP §Measure functions."
    status: pending
  - id: skip-audit
    content: "Third-party + conformance skip audit (run before declaring done). Sweep the GoogleSQL `.test` corpus (conformance/googlesql-corpus/) and bqutils known_failing/ for MEASURE / AGGREGATE(<measure>) fixtures that now pass and promote them; re-run any third-party subtest touching measure columns. Update third_party/README.md only for rows truly unblocked + note anything still failing."
    status: pending
---

# Expand 09 — Measure functions

## Why

[ROADMAP.md §Measure functions](../../ROADMAP.md) tracks MEASURE types
and `AGGREGATE(<measure>)` as ⏳ planned; ENGINE_POLICY marks them
`unsupported` ("Measure types require BigQuery's analytical layer the
emulator does not model"). Measures are increasingly used in BigQuery's
semantic-layer / looker-style modeling, so closing this lets those
queries run locally.

## The hard part

A MEASURE column carries a **deferred aggregate** that only resolves
when `AGGREGATE(<measure>)` is called in a grouping context. The plan
must plumb the measure type through analysis + storage, then correctly
expand the deferred aggregate over whatever grouping the calling query
establishes — reusing the existing aggregate-scan evaluation rather than
building a parallel path.

## Key files

- [`backend/engine/duckdb/transpiler/types.cc`](../../backend/engine/duckdb/transpiler/types.cc) — MEASURE type lowering
- [`backend/catalog/googlesql_catalog.{h,cc}`](../../backend/catalog/) — measure-column catalog modeling
- [`backend/engine/semantic/functions/aggregate_specialized.cc`](../../backend/engine/semantic/functions/aggregate_specialized.cc) — aggregate eval to reuse
- [`backend/engine/semantic/scan_eval_scan_impl.cc`](../../backend/engine/semantic/) — aggregate scan eval
- [`backend/engine/coordinator/route_classifier_visitor.cc`](../../backend/engine/coordinator/route_classifier_visitor.cc) — measure dispatch
- [`backend/engine/duckdb/transpiler/node_dispositions.yaml`](../../backend/engine/duckdb/transpiler/node_dispositions.yaml) / [`functions.yaml`](../../backend/engine/duckdb/transpiler/functions.yaml) — measure rows
- [`docs/ENGINE_POLICY.md`](../../docs/ENGINE_POLICY.md) — MEASURE row

## Steps

1. MEASURE type plumbing (analysis / storage / catalog).
2. `AGGREGATE(<measure>)` expansion over the grouping context.
3. Routing + invalid-context guard.
4. Fixtures + tracker/posture flips.

## Verify

```bash
task emulator:build-engine:bazel
task conformance:run
task lint:dispositions
task bazel:shutdown && task bazel:status
```

## Third-party / conformance to revisit

When measures land, **audit for newly-passing tests**, not just fresh
fixtures. Re-run to prove it; note anything still failing.

- **GoogleSQL `.test` + bqutils corpus** — sweep
  `conformance/googlesql-corpus/` and
  `conformance/thirdparty-fixtures/bigquery_utils/known_failing/` for
  `MEASURE` / `AGGREGATE(<measure>)` fixtures to promote.
- **client lanes** — re-run any subtest touching measure columns; update
  `third_party/README.md` for rows truly unblocked.

## Out of scope

- BigQuery's full semantic-layer / metrics modeling beyond the
  MEASURE type + `AGGREGATE()` call surfaced in the resolved AST.
- Performance optimization of measure expansion.
