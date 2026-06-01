---
name: ""
overview: ""
todos: []
isProject: false
---

# DuckDB Polyfill UDF Library

## Goal

Implement the `duckdb_udf` route. For BigQuery functions whose
semantics are close enough to DuckDB's that the gap can be closed
with a small wrapper, register a DuckDB UDF (or macro) at engine
startup and route the BigQuery call to it. This keeps the fast path
in DuckDB while keeping BigQuery semantics correct.

## Background

`functions.yaml` currently has a `kFallback` disposition for ~40 BigQuery
functions whose direct DuckDB analog is wrong (mostly
`date_add`/`date_sub`/`date_diff` month-end behavior, `format`,
`regexp_*` flag handling, the `SAFE.*` family, JSON path navigators
with BigQuery-specific path semantics, and a handful of numeric
edge cases like `mod` on negatives). The DuckDB-only roadmap planned
to rewrite each call site as raw DuckDB SQL. That is the wrong
abstraction — the BigQuery semantics belong inside a single named
helper, not duplicated in every emit method that touches the
function.

This plan introduces a per-process UDF library that the
`DuckDbExecutor` registers on its DuckDB connection at startup. Each
UDF owns the BigQuery semantics; the transpiler emits a plain DuckDB
function call.

## Dependencies

- `execution-disposition-registry.plan.md` so `functions.yaml` can
  set `duckdb_udf` as a disposition.
- `engine-router-foundation.plan.md` so we know there is a single
  DuckDB connection lifecycle to hang UDF registration on.

## Scope

- A `backend/engine/duckdb/udf/` package that registers DuckDB UDFs
  / macros at executor startup.
- One UDF (or macro) per BigQuery function whose `functions.yaml`
  row routes `duckdb_udf`.
- Wire the transpiler so `duckdb_udf` rows emit
  `<bq_emulator_udf_name>(<args>)` instead of falling back.

Initial UDF families this plan ships:

- **Date / time arithmetic.** `DATE_ADD` / `DATE_SUB` / `DATETIME_ADD` /
  `DATETIME_SUB` / `TIMESTAMP_ADD` / `TIMESTAMP_SUB` /
  `DATE_DIFF` / `DATETIME_DIFF` / `TIMESTAMP_DIFF` with BigQuery's
  month-end and timezone behavior.
- **Regex.** `REGEXP_CONTAINS`, `REGEXP_EXTRACT`,
  `REGEXP_EXTRACT_ALL`, `REGEXP_REPLACE` using BigQuery's RE2 flag
  semantics (case sensitivity, dotall behavior) on top of DuckDB's
  PCRE.
- **String / numeric.** `FORMAT` (printf-style),
  `MOD` (negative-operand sign), `IEEE_DIVIDE`, `SAFE_DIVIDE`,
  `SAFE_NEGATE`.
- **JSON path navigators.** `JSON_VALUE`, `JSON_QUERY`,
  `JSON_EXTRACT`, `JSON_EXTRACT_SCALAR` with BigQuery's path syntax
  (`$.a.b[0]`) and lenient / strict mode semantics.
- **SAFE family.** Every `SAFE.<fn>(...)` wrapper that needs to swap
  the error path for `NULL` lowers through a generated
  `safe_<fn>(...)` UDF.

Out of scope (other plans):

- BigQuery-exact functions whose semantics cannot be made correct as
  a thin wrapper (`semantic-functions-compliance.plan.md`).
- Specialized families (`specialized-feature-policy.plan.md`).

## Implementation Plan

1. Add `backend/engine/duckdb/udf/registrar.{h,cc}` with a
   `RegisterAll(duckdb::Connection&)` entry point.
2. Define a UDF authoring convention: each UDF lives in its own
   `.cc` file under `backend/engine/duckdb/udf/<family>/`, exports a
   `Register(duckdb::Connection&)` function, and ships with a unit
   test under the same folder.
3. Implement the date/time family first; it unblocks the largest
   `kFallback` chunk in `functions.yaml`.
4. Implement the regex family using DuckDB's PCRE bindings plus a
   small flag translator (`gi` → DuckDB equivalent).
5. Implement the string / numeric family.
6. Implement the JSON path navigators on top of DuckDB's JSON
   functions plus a path parser.
7. Implement the `SAFE.*` family via a macro that catches the inner
   call's error and returns `NULL`.
8. Update `functions.yaml` rows to `duckdb_udf` as each UDF lands.
9. Update the transpiler so `duckdb_udf` rows emit
   `<bq_emulator_udf_name>(<args>)` (the YAML row stores the UDF
   name).

## Tests

- Per-UDF unit tests under `backend/engine/duckdb/udf/`, each
  driving the UDF directly against an in-process DuckDB connection.
- Engine-level integration tests that drive each function through
  the gateway with both common inputs and the BigQuery-specific
  edge case the UDF exists to handle (month-end dates, negative MOD,
  unicode in regex, JSON path that picks a NULL field, ...).
- Conformance fixtures under `conformance/fixtures/functions/` that
  pin the route label to `duckdb_udf`.

## Done Criteria

- Every `functions.yaml` row originally on `kFallback` is either
  `duckdb_udf` (with a registered UDF) or has been moved to
  `semantic_executor` (with a row in
  `semantic-functions-compliance.plan.md`).
- The UDF registrar runs at executor startup and registers every
  UDF in the library; failure to register fails fast.
- Per-UDF unit tests plus per-function conformance fixtures all
  pass in CI.
- The conformance routing matrix
  (`conformance-routing-matrix.plan.md`) shows the expected
  `duckdb_udf` route label for each function.
