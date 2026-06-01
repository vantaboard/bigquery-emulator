---
name: ""
overview: ""
todos: []
isProject: false
---

# Semantic Functions Compliance

## Goal

Implement the BigQuery-exact functions that cannot be made correct
by a thin DuckDB UDF wrapper (see `duckdb-polyfill-udf-library.plan.md`
for the wrapper route). These functions live on the
`semantic_executor` route and run inside
`backend/engine/semantic/`.

## Background

A subset of BigQuery's functions have semantics that the DuckDB UDF
route cannot match without re-implementing significant logic inside
the UDF. The honest local implementation is to evaluate them in the
semantic executor where the surrounding expression already has full
BigQuery type / NULL / error semantics, instead of squeezing them
into DuckDB's scalar-function ABI.

Examples (non-exhaustive):

- Date arithmetic with timezone-sensitive month-end behavior past
  what the UDF can express cleanly.
- `SAFE.<fn>(...)` for aggregates and window functions (per-row
  error swap is much easier in the interpreter than in DuckDB
  aggregate state).
- JSON exactness: BigQuery's `JSON` type with strict / lenient
  modes, `JSON_VALUE` vs `JSON_QUERY` return-type rules, JSONPath
  semantics that DuckDB's lookalike functions diverge on.
- `BIT_COUNT` (integer popcount with BigQuery's negative-input
  behavior), `RANGE_BUCKET`, `IS_NAN` / `IS_INF`, sign-aware
  numeric edge cases.
- `HLL_COUNT.*`, `KEYS.*`, `NET.*` if implemented locally — each
  needs an explicit BigQuery-shaped result rather than a DuckDB
  approximation. `specialized-feature-policy.plan.md` decides
  per-family whether they ship as a local impl, a deterministic
  stub, or `unsupported`.
- Lambda-using array functions
  (`ARRAY_FILTER` / `ARRAY_TRANSFORM` / ...) — these need the
  semantic executor's expression evaluator for the lambda body.

## Dependencies

- `semantic-executor-core.plan.md` (the interpreter that hosts the
  function bodies).
- `execution-disposition-registry.plan.md` (the `functions.yaml`
  rows that route here).
- `duckdb-polyfill-udf-library.plan.md` (so the boundary between
  `duckdb_udf` and `semantic_executor` is decided per function,
  not blindly).

## Scope

- A `backend/engine/semantic/functions/` package, one file per
  function family (date_arith, json_exact, numeric_edges, safe_agg,
  lambda_array, ...).
- A function-dispatch table in the semantic executor that maps a
  GoogleSQL function signature to the local implementation.
- Migration of every `functions.yaml` row that
  `duckdb-polyfill-udf-library.plan.md` decided is not a UDF target
  into `semantic_executor`, with a sibling local implementation.

## Implementation Plan

1. Stand up `backend/engine/semantic/functions/dispatch.{h,cc}` and
   wire it into `EvalExpr` so a `ResolvedFunctionCall` with
   disposition `semantic_executor` looks up the local impl by
   signature.
2. Implement family-by-family, smallest first:
   - **Numeric edges.** `BIT_COUNT`, `RANGE_BUCKET`, `IS_NAN`,
     `IS_INF`, `IEEE_DIVIDE` (precision tests).
   - **Date arithmetic edges** that the UDF cannot model.
   - **JSON exactness.** `JSON_VALUE` / `JSON_QUERY` /
     `JSON_EXTRACT` / `JSON_EXTRACT_SCALAR` with BigQuery's
     return-type rules; `TO_JSON` / `TO_JSON_STRING`.
   - **SAFE aggregate.** `SAFE.SUM` / `SAFE.AVG` / `SAFE.COUNT` /
     `SAFE.MIN` / `SAFE.MAX` swap per-row evaluation errors for
     NULL.
   - **Lambda array.** `ARRAY_FILTER`, `ARRAY_TRANSFORM` (when
     BigQuery exposes them).
3. Coordinate with `specialized-feature-policy.plan.md` on which
   `unsupported`-today families (`HLL_COUNT.*`, `KEYS.*`,
   `NET.*`, `ML.*`, GIS) graduate to `semantic_executor` here
   versus stay on `unsupported` or move to a deterministic stub.
4. Update `functions.yaml` rows as each implementation lands.

## Tests

- Per-function unit tests under
  `backend/engine/semantic/functions/`.
- Engine-level integration tests in `gateway/e2e/` driving each
  function through the gateway, including the BigQuery-specific
  edge case the local impl exists to handle.
- Conformance fixtures under `conformance/fixtures/functions/`
  pinning the route label to `semantic_executor`.

## Done Criteria

- Every function originally tagged for this plan has a local
  implementation in `backend/engine/semantic/functions/` plus
  conformance coverage.
- The dispatch table covers every `semantic_executor` row in
  `functions.yaml`.
- `gateway/e2e/` tests demonstrate the BigQuery-exact behavior for
  each function's edge case.
- No `functions.yaml` row remains `kFallback`; each is either
  `duckdb_udf`, `duckdb_native`, `duckdb_rewrite`,
  `semantic_executor`, or `unsupported` (via
  `specialized-feature-policy.plan.md`).
