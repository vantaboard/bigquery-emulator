# BigQuery polyfill UDF library

This package owns the DuckDB UDFs / macros that close the BigQuery vs
DuckDB semantic gap for the function rows in
`backend/engine/duckdb/transpiler/functions.yaml` whose disposition is
`duckdb_udf`.

## How it plugs in

`DuckDbExecutor` opens a fresh in-memory DuckDB database per query
(see `backend/engine/duckdb/duckdb_executor.cc`). On every newly-opened
connection it calls `udf::RegisterAll(conn)` before executing any
transpiled SQL. The registrar installs every UDF / macro the library
ships; the transpiler emits plain `<udf_name>(<args>)` calls and
DuckDB resolves them to the registered UDFs.

Failure to register any UDF is **fail-fast**: `RegisterAll` returns a
non-OK status and `DuckDbExecutor` propagates it up the executor
stack. There is no runtime "missing UDF -> fall back to another route"
path (per `.cursor/plans/duckdb-polyfill-udf-library.plan.md`'s
Done Criterion 2).

## File layout

```
backend/engine/duckdb/udf/
  registrar.{h,cc}              Public RegisterAll entry point.
  registrar_test.cc             Smoke tests for the public contract.
  internal/run_macro.{h,cc}     Shared helper that runs a CREATE
                                MACRO DDL via the DuckDB C-API and
                                folds rejection into absl::Status.
  <family>/<name>.cc            Per-family registrar implementation.
  <family>/<name>_test.cc       Per-family / per-UDF unit tests.
```

Families today:

- `numeric/` — `MOD`, `DIV`, `LOG`, `SQRT` over NUMERIC, etc.
- `conditional/` — `IF`, `ISNULL`.
- `string/` — `STRPOS`, `INSTR`, `CONTAINS_SUBSTR`, `SOUNDEX`, ...

Future families (per the plan, not yet landed in this commit set):

- `datetime/` — `DATE_ADD/SUB/DIFF`, `DATETIME_*`, `TIMESTAMP_*`
  with BigQuery's month-end + timezone behavior.
- `regex/` — `REGEXP_CONTAINS / EXTRACT / REPLACE` with BigQuery RE2
  flag semantics on DuckDB PCRE.
- `json/` — `JSON_VALUE / QUERY / EXTRACT / EXTRACT_SCALAR` with
  BigQuery `$.a.b[0]` path + lenient / strict modes.

## Authoring a new UDF (convention)

1. Pick the family directory (or create a new one + add the source
   to `BUILD.bazel`).
2. Add `Register<NewMacro>` logic to the family's
   `<name>.cc`, using `internal::RunMacroDdl(conn, sql)` to install
   the macro. Macro body lives in plain DuckDB SQL; document the
   BigQuery edge case the wrapper closes in a comment above the
   macro.
3. Add `<name>_test.cc` next to the source. The test drives the
   macro directly against an in-process `::duckdb_connection`. It
   MUST cover both the common path AND the BigQuery-specific edge
   case the wrapper exists to pin (month-end, negative MOD, NULL
   propagation, ...).
4. Flip the matching `functions.yaml` row from
   `status=planned` to ready: replace
   `<bq_name>: duckdb_udf plan=... status=planned` with
   `<bq_name>: duckdb_udf duckdb_name=<macro_name>` and add a
   `notes:` / inline comment explaining the gap the UDF closes.
5. Drop a per-function conformance fixture under
   `conformance/fixtures/functions/<family>/` that exercises the
   function through the gateway end-to-end. Use `match: ordered`
   when the query reads from a table, and include the
   BigQuery-specific edge case row alongside the common path.
6. Land all four artifacts (UDF source, unit test, YAML row flip,
   fixture) in the same commit (per the plan's "no silent
   approximation" rule).

## Why SQL macros (and not C-API scalar UDFs)?

- Macros install via plain `CREATE OR REPLACE MACRO`, which travels
  through the existing `duckdb_query` C-API call.
- Macro bodies are inlined by DuckDB's optimizer, so there is no
  per-call function-pointer overhead.
- We avoid the C-API scalar-function callback boilerplate (variant
  marshalling, thread-safety, lifetime ownership) for the cases
  that can be expressed in SQL.

When a macro cannot express the BigQuery semantic (e.g. BigQuery
regex flag translation needs character-class transforms DuckDB SQL
does not have), the family escalates to a C-API scalar UDF instead;
the same `Register<Family>(conn)` entry point hosts the registration
via `::duckdb_register_scalar_function`. No such UDF has landed yet.
