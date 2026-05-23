---
name: storage-read-api
overview: "Phase 7: implement BigQuery Storage Read API (CreateReadSession, ReadRows) backed by Storage ScanRows/Parquet export, with Arrow Avro wire format and row restriction filters for table export workflows."
todos:
  - id: proto-storage-read
    content: "Extend proto/emulator.proto or add proto/storage_read.proto with CreateReadSession, ReadRows RPCs mirroring google.cloud.bigquery.storage.v1 shapes (simplified)"
    status: pending
  - id: read-session-handler
    content: "Implement frontend/handlers/storage_read.{h,cc}: CreateReadSession validates table, returns session name + schema + streams[]"
    status: pending
  - id: scan-backend
    content: "Add Storage::CreateReadStream(table, filter) returning chunked row iterator; memory storage scans in-memory rows; DuckDB storage reads Parquet/attached table"
    status: pending
  - id: read-rows-stream
    content: "Implement ReadRows server-streaming: serialize batches as Avro or Arrow IPC per ReadSession format enum; honor offset/limit"
    status: pending
  - id: go-storage-gateway
    content: "Add optional second listener or route prefix for Storage Read API on gateway (or document direct gRPC-only for v1); map to REST if BQ client expects HTTP — check client libs"
    status: pending
  - id: row-restriction
    content: "Parse simple row restriction strings (column = literal) for filter pushdown to Storage scan"
    status: pending
  - id: storage-read-e2e
    content: "E2E: create table, insertAll, CreateReadSession, ReadRows returns all rows matching schema"
    status: pending
isProject: false
---

# Phase 7: Storage Read API

## Prerequisites

- [reference-impl-execution_9c0d1e2f.plan.md](reference-impl-execution_9c0d1e2f.plan.md) or [duckdb-transpiler-advanced_7e8f9a0b.plan.md](duckdb-transpiler-advanced_7e8f9a0b.plan.md) — row materialization
- [duckdb-persistent-storage_6c7d8e9f.plan.md](duckdb-persistent-storage_6c7d8e9f.plan.md) — Parquet path for DuckDB profile

## Scope

**In:** CreateReadSession + ReadRows for table data export, both storage backends.

**Out:** BigQuery Write API (storage write), bidirectional streaming, full row restriction grammar, column projection pushdown (basic OK).

## Key files

- `proto/storage_read.proto` (new)
- `frontend/handlers/storage_read.{h,cc}`
- `backend/storage/storage.h` — CreateReadStream
- `backend/storage/memory/` and `backend/storage/duckdb/`
- `gateway/` — optional HTTP adapter

## Reference

- ROADMAP Phase 7 — Storage Read vs Write API split
- [docs/bigquery/docs/reference/storage](../../docs/bigquery/docs/reference/storage) (vendored)

## Verification

```bash
# gRPC client test or integration test calling CreateReadSession + ReadRows
go test -tags=integration ./gateway/e2e/... -run StorageRead
```

## Done criteria

- Read session against populated table returns correct row count and schema.
- Memory and DuckDB storage profiles both supported.
- Documented which Storage Read features are stubbed vs implemented.

## Next plan

[conformance-harness_d9e0f1a2.plan.md](conformance-harness_d9e0f1a2.plan.md)
