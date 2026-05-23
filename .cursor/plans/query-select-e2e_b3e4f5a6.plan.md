---
name: query-select-e2e
overview: "Phase 5e: QueryGetResults pagination stub and SELECT E2E tests."
todos:
  - id: get-query-results
    content: "Wire QueryGetResults single-page reads from job registry; document pagination limits"
    status: pending
  - id: e2e-select1
    content: "E2E: SELECT 1 one row; SELECT * FROM ds.t after insertAll returns inserted rows"
    status: pending
isProject: false
---

# Phase 5e: Query Select E2E

## Prerequisites

- [jobs-query-handler_a2d3e4f5.plan.md](jobs-query-handler_a2d3e4f5.plan.md)

## Verification

```bash
go test -tags=integration ./gateway/e2e/... -run Query
```

## Done criteria

- SELECT 1 and SELECT * E2E pass on reference_impl+memory

## Next plan(s)

- [dml-insert-e2e_e2b3c4d5.plan.md](dml-insert-e2e_e2b3c4d5.plan.md)
- [storage-read-proto_i6f7a8b9.plan.md](storage-read-proto_i6f7a8b9.plan.md)
- [transpiler-skeleton_c4f5a6b7.plan.md](transpiler-skeleton_c4f5a6b7.plan.md)
