---
name: Parity 02 — Function polyfills (duckdb_udf lane)
overview: Land the duckdb_udf status=planned rows in functions.yaml - the highest-frequency scalar builtins (IF, ISNULL, COUNTIF, MOD, DIV, two-arg LOG), BigQuery-exact datetime interval arithmetic, and the regex/format/string polyfills - as DuckDB UDFs/macros registered at engine startup, plus the remaining semantic_executor status=planned scalar rows.
est_effort: ~1-2 weeks
isProject: true
todos:
  - id: udf-infra-audit
    content: Audit backend/engine/duckdb/udf/ registration infra - confirm startup-time macro/UDF registration covers scalar + aggregate forms and that functions.yaml duckdb_udf rows can carry the UDF name for the transpiler emit.
    status: completed
  - id: conditional-math
    content: "Land IF, ISNULL, IFNULL-gaps, COUNTIF (aggregate), MOD, DIV, two-arg LOG as macros/UDFs; drop status=planned from each functions.yaml row as it lands."
    status: completed
  - id: datetime-interval
    content: "BigQuery-exact DATE_ADD/DATE_SUB/DATETIME_ADD/TIMESTAMP_ADD/_SUB/_DIFF/_TRUNC interval semantics, including month-end clamping (DATE_ADD('2024-01-31', INTERVAL 1 MONTH) -> 2024-02-29) and part-boundary rules DuckDB's interval math gets wrong."
    status: completed
  - id: regex-format
    content: "RE2-faithful REGEXP_CONTAINS/EXTRACT/EXTRACT_ALL/REPLACE/SUBSTR polyfills and the FORMAT()/FORMAT_DATE/FORMAT_TIMESTAMP/PARSE_* family (BigQuery format elements differ from DuckDB strftime)."
    status: completed
  - id: semantic-scalars
    content: "Clear remaining semantic_executor status=planned scalar rows (e.g. SQRT_NUMERIC) in eval_expr; confirm already-ready rows (BIT_COUNT, IEEE_DIVIDE, SAFE_DIVIDE, SAFE_NEGATE, SOUNDEX, INSTR) have fixtures."
    status: completed
  - id: fixtures-trackers
    content: One conformance fixture per function family under conformance/fixtures/functions/ with expected.route; update functions.yaml + SHAPE_TRACKER coverage summary; verify SAFE.<fn> handling on every new entry.
    status: completed
---

# Parity 02 — Function polyfills (duckdb_udf lane)

## Why

SHAPE_TRACKER's coverage summary is explicit: every `duckdb_udf
(status=planned)` row in `functions.yaml` **returns "" today and the
engine surfaces UNIMPLEMENTED**. That set includes `IF`, `ISNULL`,
`COUNTIF`, `MOD`, `DIV`, two-arg `LOG`, the interval-semantics datetime
arithmetic, and the regex/format/string polyfills — among the most
frequently called functions in real BigQuery SQL. This is the densest
coverage-per-effort plan in the set.

## Key files

- [`backend/engine/duckdb/udf/`](../../backend/engine/duckdb/udf/) — UDF/macro registration at engine startup (UDF body owns BigQuery semantics)
- [`backend/engine/duckdb/transpiler/functions.yaml`](../../backend/engine/duckdb/transpiler/functions.yaml) — source of truth; Bazel genrule -> `functions_table.inc` -> `LookupFunction`
- `backend/engine/duckdb/transpiler/functions.cc` — emit path for `duckdb_udf` rows
- [`backend/engine/semantic/`](../../backend/engine/semantic/) `eval_expr.cc` — the `semantic_executor` scalar rows
- `conformance/fixtures/functions/`

## Steps

1. Enumerate the live gap: `rg 'status=planned' backend/engine/duckdb/transpiler/functions.yaml`
   and bucket rows by family (conditional/math, datetime, regex/format,
   string, semantic-exact). Order buckets by usage: conditional/math →
   datetime → regex → format → the rest.
2. Per row decide the cheapest **correct** lane:
   - DuckDB scalar macro (`CREATE MACRO`) when expressible in DuckDB SQL
     (e.g. `IF`, `ISNULL`, `DIV`, `MOD` sign semantics).
   - Registered C++ UDF when the body needs RE2 / BigQuery format-element
     parsing / month-end clamping logic.
   - `semantic_executor` when error surfaces must be BigQuery-exact
     (overflow messages, SAFE variants) — per ENGINE_POLICY, never
     approximate.
3. Datetime correctness bar: pin month-end clamping, DST-less DATETIME
   vs zoned TIMESTAMP behavior, and `*_DIFF`/`*_TRUNC` part boundaries
   with fixtures derived from documented BigQuery examples.
4. Regex bar: BigQuery uses RE2 syntax; DuckDB uses its own regex engine —
   route through an RE2-backed C++ UDF rather than translating patterns.
5. Drop `status=planned` per landed row in the same commit as the
   implementation + fixture; `task lint:dispositions` keeps the markdown
   mirror honest.

## Verify

```bash
task emulator:build-engine:bazel
task conformance:run                 # includes new functions/ fixtures
task lint:dispositions
task bazel:shutdown && task bazel:status
```

Spot-check `SAFE.<fn>(...)` on at least one new function per bucket
(the SAFE prefix must short-circuit regardless of disposition).

## Out of scope

- Aggregate modifiers (ORDER BY/LIMIT/IGNORE NULLS inside aggregates) — plan 03
- `HLL_COUNT.*` / `NET.*` semantic bodies — plan 12
- `unsupported`-by-design families (`APPROX_QUANTILES`, `ML.*`, `ST_*`, `KEYS.ENCRYPT`) — stay per ENGINE_POLICY

## Blocked / deferred notes

- `sqrt_numeric` remains `semantic_executor status=planned`: the transpiler
  looks up functions by lowercase name only; the analyzer resolves NUMERIC
  `SQRT` to `sqrt`, so this row is unreachable until signature-aware dispatch
  lands (transpiler architecture change, not a polyfill macro).
- `regexp_extract` / `regexp_extract_all` stay on `semantic_executor` (not
  `duckdb_udf`) because capture-group semantics need pattern introspection.
- `CONTAINS_SUBSTR` JSON `json_scope` named argument is not pinned yet; STRING
  / STRUCT / ARRAY recursion is covered by conformance.
