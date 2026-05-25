---
name: duckdb-parity-e2e
overview: "Phase 5m: reference_impl vs duckdb parity tests and analytics E2E."
todos:
  - id: parity-tests
    content: "Compare reference_impl vs duckdb results on identical seed data (FLOAT tolerance)"
    status: completed
  - id: e2e-analytics
    content: "E2E: UNNEST array column, STRUCT field access, window function on DuckDB engine"
    status: completed
isProject: false
---

# Phase 5m: Duckdb Parity E2E

## Prerequisites

- [duckdb-arrow-pipeline_c0f1a2b3.plan.md](duckdb-arrow-pipeline_c0f1a2b3.plan.md)

## Verification

```bash
go test -tags=integration ./gateway/e2e/... -run DuckDBParity
```

## Done criteria

- Parity test suite passes or documents known diffs

## Next plan(s)

- [dml-insert-e2e_e2b3c4d5.plan.md](dml-insert-e2e_e2b3c4d5.plan.md)
- [storage-read-proto_i6f7a8b9.plan.md](storage-read-proto_i6f7a8b9.plan.md)
