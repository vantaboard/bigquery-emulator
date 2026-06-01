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

- `numeric/` — `bq_mod`, `bq_div`, `bq_log`.
- `regex/` — `bq_regexp_contains`, `bq_regexp_replace` (re2-vendored
  thin wrappers; `regexp_extract` / `regexp_extract_all` stay at
  `status=planned` because their capture-group semantic needs
  pattern introspection).
- `datetime/` — `bq_unix_seconds`, `bq_unix_millis`, `bq_unix_micros`,
  `bq_unix_date` (epoch wrappers; the richer date / time
  arithmetic + format / parse family stays at `status=planned`
  because of BigQuery's month-end snap, ISO-year discrimination,
  and timezone-aware truncation semantics).
- `conditional/` — `bq_if`, `bq_isnull`.
- `string/` — `bq_strpos`, `bq_split`.
- `aggregate/` — `countif` (routed `duckdb_native` to DuckDB's
  `count_if` aggregate; no UDF wrapper needed because v1.5.3's
  `count_if` already matches BQ COUNTIF on NULL / FALSE handling).

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
