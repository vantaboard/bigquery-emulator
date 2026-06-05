---
name: googlesqlite-05-arrow-marshaling
overview: Fix `arrow_to_bq` failures for anonymous/window column names blocking ~19 googlesqlite tests.
todos:
  - id: gsql-05-arrow-marshaling
    content: Fix arrow_to_bq column naming for window and anonymous columns
    status: pending
isProject: false
---

# googlesqlite 05: Arrow → BigQuery Result Marshaling

## Goal

Fix `arrow_to_bq` failures for anonymous/window column names blocking ~19 googlesqlite tests.

Baseline: `20260603T035812Z` (19 failing tests in this bucket).

## Dependencies

- [`googlesqlite-04-scan-emits.plan.md`](googlesqlite-04-scan-emits.plan.md)

## Notes

Some GROUPING SETS tests may still need plan 13 SQL lowering before rows match.

## Root cause

- `googlesqlite_query_test.go:1368: rows.Err: jobs.query -> 501: {"error":{"code":501,"message":"arrow_to_bq: INT64 column 'product_sum' backed by unsupported DuckDB type_id=16","errors":[{"reason":"n...`
- `googlesqlite_query_test.go:8045: jobs.query -> 501: {"error":{"code":501,"message":"arrow_to_bq: INT64 column 'Q1' backed by unsupported DuckDB type_id=16","errors":[{"reason":"notImplemented","mes...`
- `googlesqlite_query_test.go:8045: jobs.query -> 501: {"error":{"code":501,"message":"arrow_to_bq: FLOAT64 column 'total' backed by unsupported DuckDB type_id=19","errors":[{"reason":"notImplemented"...`

## Primary files

- [`backend/engine/duckdb/arrow_to_bq.cc`](../../backend/engine/duckdb/arrow_to_bq.cc)
- [`backend/engine/duckdb/duckdb_executor.cc`](../../backend/engine/duckdb/duckdb_executor.cc)
- [`gateway/e2e/emulator_sql.go`](../../gateway/e2e/emulator_sql.go)

## Implementation steps

1. Map DuckDB anonymous columns (`$col1`, window outputs) to stable BigQuery field names in Arrow→JSON conversion.
2. Support INT64 columns backed by non-standard Arrow physical types used by window functions.
3. Add regression tests for window frame queries that currently fail in `TestWindowVariousFrames`.
4. Re-verify GROUPING SETS tests that fail with `product_sum` column marshaling errors.

## Verify

```bash
BIGQUERY_EMULATOR_BIN=./bin/emulator_main \
  go test -tags=integration ./gateway/e2e/ \
  -run 'TestQuery$/^(PIVOT|group_by|group_by_rollup_with_one_column|group_by_rollup_with_two_columns|sum|sum_with_distinct|sum_with_window|sum_with_window_and_distinct|window_cumulative|window_cumulative_omit_current_row|window_offset|window_subtotal|window_total)$|^(TestGroupingSets|TestRangeFrame|TestWindowOrderedFrame)$|TestWindowVariousFrames$/^(partition_named|rows_current_row|rows_preceding_following)$' \
  -count=1 -parallel 1
```

## Done when

- All 19 tests listed below pass.
- `./gateway/e2e/testresults/run_googlesqlite_emulator_tests.sh` fail count drops by ~19.

## Failing tests

- `TestGroupingSets`
- `TestQuery/PIVOT`
- `TestQuery/group_by`
- `TestQuery/group_by_rollup_with_one_column`
- `TestQuery/group_by_rollup_with_two_columns`
- `TestQuery/sum`
- `TestQuery/sum_with_distinct`
- `TestQuery/sum_with_window`
- `TestQuery/sum_with_window_and_distinct`
- `TestQuery/window_cumulative`
- `TestQuery/window_cumulative_omit_current_row`
- `TestQuery/window_offset`
- `TestQuery/window_subtotal`
- `TestQuery/window_total`
- `TestRangeFrame`
- `TestWindowOrderedFrame`
- `TestWindowVariousFrames/partition_named`
- `TestWindowVariousFrames/rows_current_row`
- `TestWindowVariousFrames/rows_preceding_following`
