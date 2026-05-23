---
name: storage-read-rows
overview: "Phase 7b: Storage read stream backend and ReadRows streaming."
todos:
  - id: scan-backend
    content: "Add Storage::CreateReadStream(table, filter); memory scans rows; DuckDB reads Parquet"
    status: pending
  - id: read-rows-stream
    content: "ReadRows server-streaming: Avro or Arrow IPC batches; honor offset/limit"
    status: pending
isProject: false
---

# Phase 7b: Storage Read Rows

## Prerequisites

- [storage-read-proto_i6f7a8b9.plan.md](storage-read-proto_i6f7a8b9.plan.md)
- [duckdb-storage-ddl_p1e2f3a4.plan.md](duckdb-storage-ddl_p1e2f3a4.plan.md)

## Verification

```bash
go test -tags=integration ./gateway/e2e/... -run ReadRows
```

## Done criteria

- ReadRows returns correct row count

## Next plan(s)

- [storage-read-gateway_e2e_k8b9c0d1.plan.md](storage-read-gateway_e2e_k8b9c0d1.plan.md)
