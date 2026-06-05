---
name: local-exec-13-advanced-relational
overview: Land GROUPING SETS / ROLLUP / CUBE / PIVOT / recursive CTE SQL (~3+ dedicated tests; overlaps plan 05).
todos:
  - id: gsql-13-advanced-relational
    content: GROUPING SETS, PIVOT, recursive CTE lowering
    status: pending
isProject: false
---

# query port 13: Advanced Relational (GROUPING SETS, PIVOT, …)

## Goal

Land GROUPING SETS / ROLLUP / CUBE / PIVOT / recursive CTE SQL (~3+ dedicated tests; overlaps plan 05).

Baseline: `20260603T035812Z` (3 failing tests in this bucket).

## Dependencies

- [`local-exec-12-arrays-generators.plan.md`](local-exec-12-arrays-generators.plan.md)

## Notes

Re-verify plan 05 failures after this plan lands.

## Root cause

- `query_port_test.go:8071: [1]: (-want +got):`
- `query_port_test.go:8071: [2]: (-want +got):`
- `query_port_test.go:8071: [3]: (-want +got):`

## Primary files

- [`backend/engine/duckdb/transpiler/transpiler.cc`](../../backend/engine/duckdb/transpiler/transpiler.cc)
- [`backend/engine/coordinator/local_coordinator_engine.cc`](../../backend/engine/coordinator/local_coordinator_engine.cc)

## Implementation steps

1. Lower GROUPING SETS / ROLLUP / CUBE through DuckDB-compatible rewrite or semantic executor.
2. Implement PIVOT / UNPIVOT emit (`TestPivotAndUnpivotExtra`, `TestQuery/PIVOT`).
3. Fix recursive CTE evaluation (`TestRecursiveCTE`, `TestWithRecursiveSelfReferential`).
4. Re-run tests that failed on arrow marshaling for grouping aggregates after SQL is correct.

## Verify

```bash
BIGQUERY_EMULATOR_BIN=./bin/emulator_main \
  go test -tags=integration ./gateway/e2e/ \
  -run 'TestQuery$/^UNPIVOT$|^(TestRollupAndCube|TestSelectDistinctAndGroupBy)$' \
  -count=1 -parallel 1
```

## Done when

- All 3 tests listed below pass.
- `./gateway/e2e/testresults/run_query_port_tests.sh` fail count drops by ~3.

## Failing tests

- `TestQuery/UNPIVOT`
- `TestRollupAndCube`
- `TestSelectDistinctAndGroupBy`
