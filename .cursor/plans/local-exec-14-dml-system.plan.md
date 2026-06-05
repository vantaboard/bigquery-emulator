---
name: local-exec-14-dml-system
overview: Support DML setup statements, `@@` system variables, and information_schema queries.
todos:
  - id: gsql-14-dml-system
    content: DML executor, system variables, information_schema
    status: pending
isProject: false
---

# query port 14: DML / System Variables / information_schema

## Goal

Support DML setup statements, `@@` system variables, and information_schema queries.

Baseline: `20260603T035812Z` (6 failing tests in this bucket).

## Dependencies

- [`local-exec-13-advanced-relational.plan.md`](local-exec-13-advanced-relational.plan.md)

## Root cause

- `query_port_test.go:1672: CREATE/INSERT: jobs.query -> 400: {"error":{"code":400,"message":"2:18: Syntax error: Expected end of input but got keyword INSERT [at 2:18]","errors":[{"reason":"i...`
- `query_port_test.go:1650: CREATE/INSERT: jobs.query -> 400: {"error":{"code":400,"message":"2:18: Syntax error: Expected end of input but got keyword CREATE [at 2:18]","errors":[{"reason":"i...`
- `query_port_test.go:545: CREATE: jobs.query -> 400: {"error":{"code":400,"message":"2:43: Syntax error: Expected end of input but got keyword CREATE [at 2:43]","errors":[{"reason":"invalidQu...`

## Primary files

- [`backend/engine/semantic/dml/dml_executor.h`](../../backend/engine/semantic/dml/dml_executor.h)
- [`backend/engine/control/control_op_executor.cc`](../../backend/engine/control/control_op_executor.cc)
- [`backend/catalog/googlesql_catalog.h`](../../backend/catalog/googlesql_catalog.h)
- [`gateway/e2e/emulator_sql.go`](../../gateway/e2e/emulator_sql.go)

## Implementation steps

1. Support multi-statement scripts or sequential exec in `emulator_sql.go` for CREATE+INSERT patterns.
2. Implement UPDATE/DELETE/INSERT paths used by dedicated DML tests.
3. Recognize `@@time_zone` and related system variables (`TestSystemVariableSet`, `TestSystemVariableTimeZone`).
4. Expose information_schema tables for query port catalog introspection tests.

## Verify

```bash
BIGQUERY_EMULATOR_BIN=./bin/emulator_main \
  go test -tags=integration ./gateway/e2e/ \
  -run '^(TestDeleteWithSubquery|TestInsertWithSelect|TestJoinVariantsExtra|TestSystemVariableSet|TestSystemVariableTimeZone)$|TestQuery$/^create_table_as_select_with_column_list$' \
  -count=1 -parallel 1
```

## Done when

- All 6 tests listed below pass.
- `./gateway/e2e/testresults/run_query_port_tests.sh` fail count drops by ~6.

## Failing tests

- `TestDeleteWithSubquery`
- `TestInsertWithSelect`
- `TestJoinVariantsExtra`
- `TestQuery/create_table_as_select_with_column_list`
- `TestSystemVariableSet`
- `TestSystemVariableTimeZone`
