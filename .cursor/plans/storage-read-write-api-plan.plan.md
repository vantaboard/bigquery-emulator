---
name: ""
overview: ""
todos: []
isProject: false
---

# Storage Read / Write API Plan

## Goal

Finish the BigQuery Storage Read API and add the Storage Write API
on the same gRPC surface (`bigquery_emulator.v1.StorageRead` /
`StorageWrite`). Reads stay on the DuckDB Arrow fast path; writes go
through the storage-aware path the local DML executor already owns.

## Background

The Storage Read API is partially implemented today:

- ✅ `BigQueryRead` gRPC service: `CreateReadSession` and `ReadRows`
  are wired.
- ✅ Arrow output format (native fast path: DuckDB Arrow batches
  streamed straight onto the gRPC wire without REST `f`/`v` round-
  tripping).
- 🟡 `ReadOptions.row_restriction`: only a single
  `<column> = <literal>` equality clause is pushed down.
- ⏳ `ReadOptions.selected_fields`: accepted + echoed but not
  enforced.
- ⏳ `SplitReadStream`: not implemented; single-stream sessions
  only.
- ⏳ Avro output format: not wired.
- ⏳ Storage Write API: not wired at all (no `BigQueryWrite`
  service).

## Dependencies

- `dml-local-executor.plan.md` (Storage Write API shares the
  storage-aware write path with DML and `tabledata.insertAll`).
- `engine-router-foundation.plan.md` (so a Storage Write API
  append goes through the same coordinator that DML does).

## Scope

This plan covers:

- **Read API completion.**
  - `selected_fields` enforcement (project only the requested
    columns in the underlying DuckDB scan).
  - Multi-clause `row_restriction` (boolean connectives, range /
    inequality ops, `IN`, `NULL` checks) with rejection at session
    creation for the still-unsupported subset.
  - `SplitReadStream` for parallel scans.
  - Avro output format alongside Arrow.
- **Write API.**
  - `BigQueryWrite` gRPC service:
    `CreateWriteStream` / `AppendRows` /
    `GetWriteStream` / `FinalizeWriteStream` /
    `BatchCommitWriteStreams` / `FlushRows`.
  - Stream types: `_default`, `BUFFERED`, `COMMITTED`, `PENDING`.
  - Schema-less and schema-aware appends.
  - Reuse the local DML executor's storage append primitive.

## Implementation Plan

1. **`selected_fields` projection pushdown.** Pass the field list
   through the storage-scan emitter so DuckDB only reads the
   requested columns.
2. **Multi-clause `row_restriction`.** Re-parse the restriction
   into an analyzer-resolved boolean expression; reuse the
   transpiler / semantic executor to evaluate. Reject unsupported
   shapes at `CreateReadSession`.
3. **`SplitReadStream`.** Partition the underlying scan range and
   return multiple `ReadStream`s with the same Arrow schema; each
   stream is a DuckDB cursor over a subrange.
4. **Avro output.** Use Avro's C++ writer on top of the
   DuckDB-internal row iterator; reuse the existing schema-mapping
   code that exists for Arrow.
5. **`BigQueryWrite` service.** Wire the gRPC service under
   `frontend/handlers/storage_write.{h,cc}`. Stream rows reach
   `DuckDBStorage::AppendRows` through the local DML executor's
   primitive.
6. **Write API stream types.** `_default` and `COMMITTED` commit
   on every flush; `BUFFERED` requires `FlushRows`; `PENDING`
   requires `FinalizeWriteStream` + `BatchCommitWriteStreams`.
7. **Error envelope.** Map `DuckDBStorage` errors to the
   Storage Write API's `AppendRowsResponse.error` shape.

## Tests

- Per-feature unit tests under `frontend/handlers/`.
- Engine-level integration tests in `gateway/e2e/storage/`:
  - Read API: `selected_fields` projection, multi-clause
    `row_restriction`, `SplitReadStream` parallel scan, Avro
    output.
  - Write API: `_default` stream append, `BUFFERED` stream with
    `FlushRows`, `PENDING` stream with finalize + batch commit,
    schema mismatch rejection.
- Third-party client tests under
  `third_party/golang-bigquery-tests/` (the Go client's Storage
  Read + Write API samples) running green.
- Conformance fixtures under `conformance/fixtures/storage/`.

## Done Criteria

- The Read API completes the four `⏳` rows from ROADMAP.md
  "Storage Read API (gRPC)".
- The Write API gRPC service is implemented and the four BigQuery
  stream types work end-to-end.
- The Go BigQuery client's `bigqueryreadclient` and
  `bigquerywriteclient` samples run green against the emulator.
- ROADMAP.md "Storage Read API (gRPC)" section adds a "Storage
  Write API (gRPC)" twin section with the same level of detail.
