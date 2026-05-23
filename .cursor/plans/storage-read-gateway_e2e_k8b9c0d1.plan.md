---
name: storage-read-gateway-e2e
overview: "Phase 7c: gateway adapter, row restriction, full Storage Read E2E."
todos:
  - id: go-storage-gateway
    content: "Document or add gateway route for Storage Read API; check BQ client lib expectations"
    status: pending
  - id: row-restriction
    content: "Parse simple row restriction (column = literal) for filter pushdown"
    status: pending
  - id: storage-read-e2e
    content: "E2E: insertAll → CreateReadSession → ReadRows returns all rows"
    status: pending
isProject: false
---

# Phase 7c: Storage Read Gateway E2E

## Prerequisites

- [storage-read-rows_j7a8b9c0.plan.md](storage-read-rows_j7a8b9c0.plan.md)

## Verification

```bash
go test -tags=integration ./gateway/e2e/... -run StorageRead
```

## Done criteria

- Memory and DuckDB profiles both supported

## Next plan(s)

- [conformance-fixtures-runner_l9c0d1e2.plan.md](conformance-fixtures-runner_l9c0d1e2.plan.md)
