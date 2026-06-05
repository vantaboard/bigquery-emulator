---
name: local-exec-06-aggregate-modifiers
overview: Support BigQuery `array_agg` / `string_agg` modifiers blocking ~8 query port tests.
todos:
  - id: gsql-06-aggregate-modifiers
    content: Emit array_agg/string_agg with ORDER BY and LIMIT modifiers
    status: pending
isProject: false
---

# query port 06: Aggregate Modifiers (ORDER BY / LIMIT / HAVING)

## Goal

Support BigQuery `array_agg` / `string_agg` modifiers blocking ~8 query port tests.

Baseline: `20260603T035812Z` (8 failing tests in this bucket).

## Dependencies

- [`local-exec-04-scan-emits.plan.md`](local-exec-04-scan-emits.plan.md)

## Root cause

- `I0000 00:00:1780459094.496532  399173 transpiler.cc:1924] duckdb transpiler: aggregate 'string_agg' uses a modifier (HAVING / ORDER BY / LIMIT / GROUP BY / NULL-handling) that has no DuckDB analog ...`
- `query_port_test.go:1781: Scan: sql: no rows in result set`
- `I0000 00:00:1780459094.329759  399173 transpiler.cc:1924] duckdb transpiler: aggregate 'array_agg' uses a modifier (HAVING / ORDER BY / LIMIT / GROUP BY / NULL-handling) that has no DuckDB analog y...`

## Primary files

- [`backend/engine/duckdb/transpiler/transpiler.cc`](../../backend/engine/duckdb/transpiler/transpiler.cc)
- [`backend/engine/duckdb/transpiler/functions.yaml`](../../backend/engine/duckdb/transpiler/functions.yaml)

## Implementation steps

1. Extend aggregate emit to translate ORDER BY / LIMIT / DISTINCT modifiers on `array_agg` and `string_agg`.
2. Route modifier-heavy aggregates to `duckdb_rewrite` when DuckDB syntax differs from BigQuery.
3. Cover dedicated tests: `TestArrayAggLimitOrderBy`, `TestStringAggOrderByLimit`, `TestAggregateModifiers`.
4. Mirror modifier behavior for matching `TestQuery/array_agg_*` and `TestQuery/string_agg_*` subtests.

## Verify

```bash
BIGQUERY_EMULATOR_BIN=./bin/emulator_main \
  go test -tags=integration ./gateway/e2e/ \
  -run '^(TestAggregateModifiers$|TestArrayAggLimitOrderBy$|TestArrayAggOrderByDesc$|TestArrayAggOrderByLimit$|TestStringAggDistinctOrderByLimit$|TestStringAggLimit$|TestStringAggOrderBy$|TestStringAggOrderByLimit$)' \
  -count=1 -parallel 1
```

## Done when

- All 8 tests listed below pass.
- `./gateway/e2e/testresults/run_query_port_tests.sh` fail count drops by ~8.

## Failing tests

- `TestAggregateModifiers`
- `TestArrayAggLimitOrderBy`
- `TestArrayAggOrderByDesc`
- `TestArrayAggOrderByLimit`
- `TestStringAggDistinctOrderByLimit`
- `TestStringAggLimit`
- `TestStringAggOrderBy`
- `TestStringAggOrderByLimit`
