---
name: googlesqlite-03-operator-disposition
overview: Register internal comparison/arithmetic operators (`$equal`, `$less`, `$add`, …) in the disposition table.
todos:
  - id: gsql-03-operator-disposition
    content: Add disposition rows for internal $-prefixed operators
    status: pending
isProject: false
---

# googlesqlite 03: Operator Dispositions

## Goal

Register internal comparison/arithmetic operators (`$equal`, `$less`, `$add`, …) in the disposition table.

Baseline: `20260603T035812Z` (4 failing tests in this bucket).

## Dependencies

- [`googlesqlite-01-ddl-catalog.plan.md`](googlesqlite-01-ddl-catalog.plan.md)

## Root cause

- `I0000 00:00:1780459094.345209  399173 transpiler.cc:1836] duckdb transpiler: function '$add' has no disposition; surfacing UNIMPLEMENTED`
- `googlesqlite_query_test.go:1131: rows.Err: jobs.query -> 501: {"error":{"code":501,"message":"duckdb engine: transpiler does not yet cover this query shape (family: node:OrderByScan, route: duckdb_...`
- `I0000 00:00:1780459094.342006  399173 transpiler.cc:1836] duckdb transpiler: function '$equal' has no disposition; surfacing UNIMPLEMENTED`

## Primary files

- [`backend/engine/duckdb/transpiler/functions.yaml`](../../backend/engine/duckdb/transpiler/functions.yaml)
- [`backend/engine/coordinator/route_classifier.cc`](../../backend/engine/coordinator/route_classifier.cc)
- [`conformance/dispositions/`](../../conformance/dispositions/)

## Implementation steps

1. Add `functions.yaml` rows for `$equal`, `$less`, `$greater`, `$add`, `$subtract`, `$multiply`, `$divide` with `duckdb_native` or `semantic_executor` as appropriate.
2. Ensure transpiler maps internal operator names to DuckDB SQL operators.
3. Regenerate functions table if the repo uses `functions_table_gen.awk`.
4. Verify `TestQualifyClause`, `TestComputedColumnsInProject`, `TestRecursiveCTE` no longer hit `has no disposition`.

## Verify

```bash
BIGQUERY_EMULATOR_BIN=./bin/emulator_main \
  go test -tags=integration ./gateway/e2e/ \
  -run '^(TestComputedColumnsInProject$|TestQualifyClause$|TestRecursiveCTE$|TestWithRecursiveSelfReferential$)' \
  -count=1 -parallel 1
```

## Done when

- All 4 tests listed below pass.
- `./gateway/e2e/testresults/run_googlesqlite_emulator_tests.sh` fail count drops by ~4.

## Failing tests

- `TestComputedColumnsInProject`
- `TestQualifyClause`
- `TestRecursiveCTE`
- `TestWithRecursiveSelfReferential`
