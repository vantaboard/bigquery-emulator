---
name: Thirdparty 08 — Advanced query params & schema jobs
overview: Engine-level TIMESTAMP/STRUCT query parameters and query-job schema add/relax (ALTER TABLE via jobs.insert query configuration).
depends_on: [thirdparty-02-gateway-query-metadata]
blocks: [thirdparty-11-bigframes-gate]
baseline_log: .logs/thirdparty-20260605-112926.log
est_effort: ~1 week
isProject: true
todos:
  - id: timestamp-params
    content: Bind TIMESTAMP query parameters through gateway → engine → DuckDB/semantic executor
    status: pending
  - id: struct-params
    content: Bind STRUCT query parameters (named and positional)
    status: pending
  - id: positional-params
    content: Wire positional query parameters end-to-end (gateway currently drops them)
    status: pending
  - id: schema-add-relax
    content: Query jobs that ALTER TABLE add/relax columns update catalog schema
    status: pending
  - id: advanced-param-tests
    content: Green python timestamp/struct tests and node add/relax column query job tests
    status: pending
---

# Thirdparty 08 — Advanced query params & schema jobs

## Goal

Fix failures that require **engine binding**, not just gateway JSON unmarshaling (plan 02).

## Baseline failures

| Suite | Tests | Error |
|-------|-------|-------|
| Python | `test_client_query_w_timestamp_params` | `parameter type 'TIMESTAMP' is not yet supported` |
| Python | `test_client_query_w_struct_params` | struct binding failure |
| Python | `test_client_query_add_column`, `test_client_query_relax_column` | Schema unchanged after query job |
| Node | `should add a new column via a query job`, `should relax columns via a query job` (items 25–26) | Same |

## 1. TIMESTAMP / STRUCT parameters

### Gateway

Plan 02 fixes numeric `parameterValue.value` JSON. This plan extends [`gateway/handlers/queries.go`](../../gateway/handlers/queries.go) `parametersToEngineMap` to serialize complex types:

- `parameterType.type = TIMESTAMP` → RFC3339 string in `value_json`
- `STRUCT` → nested `structValues` JSON encoding

### Engine

- [`proto/emulator.proto`](../../proto/emulator.proto) — verify `QueryParameter` supports complex `value_json`
- [`backend/engine/duckdb/`](../../backend/engine/duckdb/) — parameter substitution in transpiler
- [`backend/engine/semantic/`](../../backend/engine/semantic/) — fallback for unsupported DuckDB param types

Reference: [`docs/ENGINE_POLICY.md`](../../docs/ENGINE_POLICY.md) deliberately-deferred list.

## 2. Positional parameters

[`gateway/handlers/queries.go`](../../gateway/handlers/queries.go) comment (lines 242–250): positional params are **dropped** because engine proto uses a name-keyed map.

**Options:**
1. Extend proto with `repeated QueryParameter ordered_parameters`
2. Synthetic keys `@positional_0`, `@positional_1` with engine-side decoding

Pick one; add conformance fixture.

## 3. Schema add/relax via query job

Samples run SQL like:

```sql
ALTER TABLE dataset.table ADD COLUMN ...
ALTER TABLE dataset.table ALTER COLUMN ... SET OPTIONS (description=...)
```

Route through [`backend/engine/control/`](../../backend/engine/control/) DDL handlers:

- Ensure `jobs.insert` with `configuration.query` executes DDL
- After job completes, `tables.get` reflects new schema

**Investigate:** Whether failures are engine DDL gaps or gateway not refreshing catalog metadata post-job.

## Verification

```bash
# Engine unit tests
task bazel:test -- //backend/engine/control:control_op_executor_test
task bazel:test -- //backend/engine/semantic:...

# Gateway
go test ./gateway/handlers/... -run 'Parameter|Timestamp|Struct'

# Thirdparty
PYTHON_SAMPLES_PYTEST_ARGS='samples/tests/test_client_query_w_timestamp_params.py samples/tests/test_client_query_add_column.py -v' \
  task thirdparty:python-bigquery-tests
task thirdparty:node-bigquery-tests
```

## Out of scope

- ARRAY parameters (unless surfaced by same tests)
- Geography types

## Done when

- [ ] TIMESTAMP and STRUCT python tests pass
- [ ] Node query-job add/relax column tests pass
- [ ] Positional params work for at least node named-params parity tests
- [ ] Conformance fixture for TIMESTAMP param query
