---
name: storage-read-proto
overview: "Phase 7a: Storage Read proto and CreateReadSession handler."
todos:
  - id: proto-storage-read
    content: "Add proto/storage_read.proto with CreateReadSession, ReadRows RPCs (simplified BQ storage v1)"
    status: completed
  - id: read-session-handler
    content: "Implement storage_read.{h,cc}: CreateReadSession validates table, returns session + schema + streams"
    status: completed
isProject: false
---

# Phase 7a: Storage Read Proto

## Prerequisites

- [query-select-e2e_b3e4f5a6.plan.md](query-select-e2e_b3e4f5a6.plan.md)

## Verification

```bash
grpcurl -plaintext localhost:9060 list | grep -i read
```

## Done criteria

- CreateReadSession returns valid session for existing table

## Next plan(s)

- [storage-read-rows_j7a8b9c0.plan.md](storage-read-rows_j7a8b9c0.plan.md)
