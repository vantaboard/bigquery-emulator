# conformance/fixtures/functions/

Per-function fixtures for the BigQuery polyfill UDF library (rows in
`backend/engine/duckdb/transpiler/functions.yaml` with disposition
`duckdb_udf`). Each fixture drives the function end-to-end through
the gateway and pins both the common path AND the BigQuery-specific
edge case the polyfill macro exists to close.

## Directory layout

```
conformance/fixtures/functions/
  <family>/<function_name>.yaml
```

Families today:

- `numeric/` — `bq_mod`, `bq_div`.

Families landed by follow-up commits within the same plan ship one
fixture per BigQuery function alongside the macro itself (per the
plan's "no silent approximation" rule).

## Route-label assertion (planned)

The fixtures here all resolve to the `duckdb_udf` route once their
matching `functions.yaml` row flips from `status=planned` to ready.
`conformance-routing-matrix.plan.md` (plan 16) wires the
`expected.route` field; until then this directory does not carry a
hard route-label assertion -- the route is exercised implicitly via
the live macro registration in `DuckDbExecutor`.

## How this directory is used

`task conformance:run` (full conformance suite) walks every YAML
under `conformance/fixtures/` so these fixtures are picked up
alongside the seed set. `task conformance:fastpath`
(`conformance/fixtures/fastpath/` only) is unchanged today; once
plan 16 lands its route-label assertion, the fastpath task can
optionally widen its scope to include this directory (since
`duckdb_udf` is also a fast-path-compatible route).
