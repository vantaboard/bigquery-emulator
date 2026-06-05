---
name: googlesqlite-16-result-fixes
overview: Fix remaining wrong results, empty row sets, and Go driver scan type mismatches (~16 tests).
todos:
  - id: gsql-16-result-fixes
    content: emulator_sql type mapping and empty-result semantic bugs
    status: pending
isProject: false
---

# googlesqlite 16: Result / Driver Fixes

## Goal

Fix remaining wrong results, empty row sets, and Go driver scan type mismatches (~16 tests).

Baseline: `20260603T035812Z` (19 failing tests in this bucket).

## Dependencies

- [`googlesqlite-15-specialized-stubs.plan.md`](googlesqlite-15-specialized-stubs.plan.md)

## Notes

Run last; some tests here may clear earlier once upstream plans land.

## Root cause

- `googlesqlite_query_test.go:1631: Scan: sql: no rows in result set`
- `googlesqlite_query_test.go:350: IN: sql: no rows in result set`
- `googlesqlite_query_test.go:1208: Scan: cannot scan int64 into *sql.NullInt64`

## Primary files

- [`gateway/e2e/emulator_sql.go`](../../gateway/e2e/emulator_sql.go)
- [`backend/engine/coordinator/local_coordinator_engine.cc`](../../backend/engine/coordinator/local_coordinator_engine.cc)

## Implementation steps

1. Fix `sql.NullInt64` scanning: return NULL as sql.NullInt64 valid=false instead of bare int64.
2. Fix LIMIT/OFFSET result delivery (`TestLimitOffset`).
3. Fix EXISTS/IN/subquery tests returning empty result sets when rows expected.
4. Sweep any tests still failing after plans 01–15; re-baseline with `run_googlesqlite_emulator_tests.sh`.

## Verify

Use split patterns (plan’s single `^(TestQuery/…)$` alternation only runs ~6 top-level tests in Go):

```bash
BIGQUERY_EMULATOR_BIN=./bin/emulator_main \
  go test -tags=integration ./gateway/e2e/ \
  -run '^(TestArrayConcatBetweenLiterals|TestExistsAndInSubquery|TestLagLeadFirstLast|TestLimitOffset|TestNumericArithmetic|TestOrderByDescAndNullsFirst)$|TestQuery$/^(avg_with_window|begin-end|cast_string_to_int64_-_leading_zeros|except#01|multiple_statements_with_positional_params|order_by|split|string_agg_with_distinct|window_range)$|TestSubqueryExpr$/^(array|exists|in|scalar)$' \
  -count=1 -parallel 1
```

Parent re-gate (2026-06-04): **10/19 pass** (6 top-level + 4 `TestSubqueryExpr/*`); 9 `TestQuery/*` still fail (row/order/duckdb — upstream plans). Subagent reported 19/19 with legacy regex (under-ran subtests).

## Full suite (after plan 16)

```bash
./gateway/e2e/testresults/run_googlesqlite_emulator_tests.sh
```

Latest: `googlesqlite-emulator-20260604T201957Z-summary.txt` — **pass=197 fail=508 skip=2** (baseline fail=568). Many post-`use_function` failures are connection-refused cascades, not plan-16 regressions.

## Done when

- All 19 tests listed below pass.
- `./gateway/e2e/testresults/run_googlesqlite_emulator_tests.sh` fail count drops by ~19.

## Failing tests

- `TestArrayConcatBetweenLiterals`
- `TestExistsAndInSubquery`
- `TestLagLeadFirstLast`
- `TestLimitOffset`
- `TestNumericArithmetic`
- `TestOrderByDescAndNullsFirst`
- `TestQuery/avg_with_window`
- `TestQuery/begin-end`
- `TestQuery/cast_string_to_int64_-_leading_zeros`
- `TestQuery/except#01`
- `TestQuery/multiple_statements_with_positional_params`
- `TestQuery/order_by`
- `TestQuery/split`
- `TestQuery/string_agg_with_distinct`
- `TestQuery/window_range`
- `TestSubqueryExpr/array`
- `TestSubqueryExpr/exists`
- `TestSubqueryExpr/in`
- `TestSubqueryExpr/scalar`
