# DuckDB Transpiler Function Rewrites

## Goal

Promote deferred scalar functions in `functions.yaml` from `fallback` to DuckDB
lowerings when a direct name map is not enough.

## Tracker Rows

- Complete the remaining subset of `ResolvedFunctionCall`.
- Promote matching `functions.yaml` `fallback` entries as rewrites land.

## Implementation Plan

1. Split function dispatch into direct maps and rewrite handlers so simple
   `kMap` entries stay table-driven while semantic rewrites live in C++ helpers.
2. Implement math rewrites: `LOG`, `MOD`, `SQRT` over NUMERIC,
   `SAFE_DIVIDE`, `DIV`, and `SAFE_NEGATE`.
3. Implement conditional rewrites: `IF`, `ISNULL` if analyzer output uses it,
   and any CASE-like internal functions encountered by conformance tests.
4. Implement string/regex/format rewrites only after checking dialect gaps:
   `SPLIT`, `REGEXP_*`, `FORMAT`, `CONTAINS_SUBSTR`, `STRPOS`, `INSTR`, and
   `SOUNDEX`.
5. Implement datetime rewrites: `DATE_*`, `DATETIME_*`, `TIMESTAMP_*`,
   `EXTRACT`, format/parse functions, and Unix conversion functions.
6. Regenerate `functions_table.inc` through the existing genrule path whenever
   YAML dispositions change.

## Tests

- One focused emitter test for every promoted function family.
- Execution tests for boundary semantics: divide-by-zero, signed modulo,
  month-end date arithmetic, regex dialect differences, formatting edge cases,
  and parse failures.
- Fallback tests for any function variant still intentionally deferred.

## Done Criteria

- Promoted functions are no longer `fallback` in `functions.yaml`.
- Rewrites are centralized enough that adding a function does not grow
  `EmitFunctionCall` into a long one-off chain.
- `SHAPE_TRACKER.md` documents the remaining function skiplist separately from
  completed scalar function rewrites.
