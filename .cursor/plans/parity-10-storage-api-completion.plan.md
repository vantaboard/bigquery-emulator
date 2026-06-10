---
name: Parity 10 — Storage Read/Write API completion
overview: Close the deferred Storage API surface - multi-stream read sessions + SplitReadStream with deterministic partitioning, analyzer-resolved row_restriction pushdown beyond single-equality, and the Write API PENDING stream type with BatchCommitWriteStreams.
est_effort: ~1-2 weeks
isProject: true
todos:
  - id: multi-stream
    content: "max_stream_count > 1 on CreateReadSession: deterministic row-boundary partitioning over the DuckDB scan (ROADMAP notes parquet-row-boundary design is the open piece); each stream serves a disjoint partition; keep single-stream as the degenerate case."
    status: pending
  - id: split-read-stream
    content: "SplitReadStream RPC: split an existing stream's remaining range into primary + residual per the public contract; reject when fraction is out of range."
    status: pending
  - id: row-restriction
    content: "row_restriction beyond single equality: analyze the restriction with the GoogleSQL analyzer against the table schema (the ENGINE_POLICY-compliant route - no parser-only approximation), then transpile the resolved boolean expr through the existing transpiler expression emit into the DuckDB scan WHERE clause; reject still-unsupported constructs at CreateReadSession with INVALID_ARGUMENT."
    status: pending
  - id: write-pending
    content: "PENDING stream type: CreateWriteStream(PENDING), AppendRows buffering to a per-stream staging area, FinalizeWriteStream, BatchCommitWriteStreams atomically committing finalized streams through DuckDBStorage::AppendRows; update proto/storage_write.proto reserved slots + gateway shim."
    status: pending
  - id: fixtures-suites
    content: "Coverage: Go thirdparty Storage Read multi-stream subtests + Java WritePendingStreamIT-style sample; remove the matching skip-matrix rows; update ROADMAP Storage sections + ENGINE_POLICY Storage gRPC posture table."
    status: pending
---

# Parity 10 — Storage Read/Write API completion

## Why

Client libraries default to the Storage API for bulk reads/writes.
Three deliberate deferrals remain (ROADMAP Storage Read/Write
sections, ENGINE_POLICY Storage gRPC table):

| Surface | Today |
|---------|-------|
| `max_stream_count > 1` | rejected; *"tractable follow-up, pending a deterministic parquet-row-boundary partition design"* |
| `SplitReadStream` | pending |
| `row_restriction` | single `<column> = <literal>` only; multi-clause deferred because parser-only expansion is *"exactly the silent-approximation hazard the execution policy forbids"* |
| Write `PENDING` + `BatchCommitWriteStreams` | `UNIMPLEMENTED` |

## Key files

- [`frontend/handlers/storage_read.{h,cc}`](../../frontend/handlers/), [`frontend/handlers/storage_write.h`](../../frontend/handlers/storage_write.h) + `.cc`
- [`proto/storage_read.proto`](../../proto/storage_read.proto), [`proto/storage_write.proto`](../../proto/storage_write.proto) — internal contract
- [`gateway/handlers/bqstorage/`](../../gateway/handlers/bqstorage/) — public-service shim (Arrow/Avro encoding, proto-descriptor rows)
- [`backend/storage/duckdb/`](../../backend/storage/duckdb/) — scan partitioning + append staging
- `third_party/` Go + Java Storage suites, `third_party/README.md` skip matrices

## Steps

1. `row_restriction` first (most user-visible): reuse the engine's
   analyzer with an expression-only catalog of the table's columns;
   walk the resolved expr through the existing transpiler expression
   emitter; anything the emitter cannot lower rejects up front. This
   honors rule 4 (no silent approximation) while expanding coverage to
   comparisons, connectives, `IN`, `IS NULL`.
2. Multi-stream: partition on stable row ranges from the DuckDB scan
   (e.g. rowid ranges snapshotted at CreateReadSession); document the
   determinism contract; wire stream count negotiation (server may
   return fewer streams).
3. `SplitReadStream` over the partition bookkeeping from step 2.
4. Write `PENDING`: stage appends per stream (storage-backed staging
   table keyed by stream id so finalize/commit survives within the
   session), `BatchCommitWriteStreams` moves staged rows atomically;
   internal proto changes regenerate Go bindings (`buf generate`) +
   C++ (Bazel).
5. Update both shims, the ENGINE_POLICY posture table, ROADMAP rows,
   and the third-party skip matrices.

## Verify

```bash
task emulator:build-engine:bazel
go test ./gateway/handlers/bqstorage/...
task conformance:run
task thirdparty:golang        # Storage Read subtests
task thirdparty:java          # Write pending/buffered ITs
task bazel:shutdown && task bazel:status
```

## Out of scope

- Read-session snapshot isolation guarantees beyond what DuckDB scans give — document, don't promise
- Avro writer-side support (write path is proto rows per public contract)
