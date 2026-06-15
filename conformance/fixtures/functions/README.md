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
- `conditional/` — `bq_if`.
- `regex/` — `bq_regexp_contains`, `bq_regexp_replace` (re2-vendored
  thin wrappers; `regexp_extract` / `regexp_extract_all` stay on
  `semantic_executor` because their capture-group semantic needs
  pattern introspection).
- `datetime/` — `bq_unix_seconds`, `bq_unix_millis`, `bq_unix_micros`,
  `bq_unix_date` (epoch wrappers). Interval-semantics
  `DATE_ADD` / `*_DIFF` / `*_TRUNC` and extended `FORMAT_*` /
  `PARSE_*` route through `semantic_executor` (googlesql-backed).
- `string/` — `bq_strpos`, `bq_split`; `contains_substr` /
  `instr` / `soundex` route through `semantic_executor`.
- `format/` — `format_date` conformance fixture (semantic executor).
- `aggregate/` — `countif` (routed `duckdb_native` to DuckDB's
  `count_if` aggregate; no UDF wrapper needed because v1.5.3's
  `count_if` already matches BQ COUNTIF on NULL / FALSE handling).

New families ship one fixture per BigQuery function alongside the
macro itself (the "no silent approximation" rule).

## Route-label assertion

The fixtures here all resolve to the `duckdb_udf` route once their
matching `functions.yaml` row flips from `status=planned` to ready.
The runner supports an `expected.route` assertion (see
`docs/ENGINE_POLICY.md`), but this directory intentionally does not
carry a hard route-label assertion -- the route is exercised
implicitly via the live macro registration in `DuckDbExecutor`, and
routing drift is tracked by `task conformance:routing-matrix`.

## How this directory is used

`task conformance:run` (full conformance suite) walks every YAML
under `conformance/fixtures/` so these fixtures are picked up
alongside the seed set. `task conformance:fastpath` (the PR-gating
fast lane) also runs this directory explicitly alongside
`conformance/fixtures/fastpath/` (since `duckdb_udf` is also a
fast-path-compatible route).
