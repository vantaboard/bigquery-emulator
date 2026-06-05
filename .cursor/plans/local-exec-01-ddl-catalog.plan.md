---
name: local-exec-01-ddl-catalog
overview: Fix unqualified DDL table names, dataset bootstrap, and catalog 404s blocking 14 query port tests.
todos:
  - id: gsql-01-ddl-catalog
    content: Qualify DDL names and auto-create datasets for query port tests
    status: pending
isProject: false
---

# query port 01: DDL / Catalog / Gateway SQL

## Goal

Fix unqualified DDL table names, dataset bootstrap, and catalog 404s blocking 14 query port tests.

Baseline: `20260603T035812Z` (12 failing tests in this bucket).

## Root cause

- `query_port_test.go:1945: CREATE TABLE: jobs.query -> 404: {"error":{"code":404,"message":"Not found: Dataset gsql-InformationSchemaColumns:colsds","errors":[{"reason":"notFound","message":"...`
- `query_port_test.go:2108: CREATE TABLE: jobs.query -> 404: {"error":{"code":404,"message":"Not found: Dataset gsql-InformationSchemaColumnsA:infoarr","errors":[{"reason":"notFound","message"...`
- `query_port_test.go:2054: CREATE TABLE: jobs.query -> 404: {"error":{"code":404,"message":"Not found: Dataset gsql-InformationSchemaColumnsO:op_ds","errors":[{"reason":"notFound","message":"...`

## Primary files

- [`gateway/e2e/emulator_sql.go`](../../gateway/e2e/emulator_sql.go)
- [`backend/engine/control/control_op_executor.cc`](../../backend/engine/control/control_op_executor.cc)
- [`backend/catalog/googlesql_catalog.h`](../../backend/catalog/googlesql_catalog.h)

## Implementation steps

1. Teach `emulator_sql.go` to qualify bare `CREATE TABLE t` as `dataset.table` using the test project default dataset.
2. Relax or branch `control_op_executor` DDL name parsing so single-segment names accepted when a default dataset is in scope.
3. Auto-create datasets referenced by query port tests before DDL runs (404 `Not found: Dataset …`).
4. Return BigQuery-shaped success for `CREATE TABLE` / `CREATE TABLE AS` used in test setup.
5. Add focused unit tests in `control_op_executor_test.cc` for one-segment vs three-segment names.

## Verify

```bash
BIGQUERY_EMULATOR_BIN=./bin/emulator_main \
  go test -tags=integration ./gateway/e2e/ \
  -run '^(TestInformationSchemaColumns$|TestInformationSchemaColumnsArrayType$|TestInformationSchemaColumnsOrdinalPosition$|TestInformationSchemaColumnsStructType$|TestInformationSchemaSchemata$|TestInformationSchemaTables$|TestParameterBindingTypes$|TestPivotAndUnpivotExtra$|TestSelectStarReplace$|TestTableSample$|TestUpdateSetWhere$|TestWildcardTableScan$)' \
  -count=1 -parallel 1
```

## Done when

- All 12 tests listed below pass.
- `./gateway/e2e/testresults/run_query_port_tests.sh` fail count drops by ~12.

## Failing tests

- `TestInformationSchemaColumns`
- `TestInformationSchemaColumnsArrayType`
- `TestInformationSchemaColumnsOrdinalPosition`
- `TestInformationSchemaColumnsStructType`
- `TestInformationSchemaSchemata`
- `TestInformationSchemaTables`
- `TestParameterBindingTypes`
- `TestPivotAndUnpivotExtra`
- `TestSelectStarReplace`
- `TestTableSample`
- `TestUpdateSetWhere`
- `TestWildcardTableScan`
