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

- `googlesqlite-14-dml-system.plan.md` (Storage Write API shares the
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

## Notes & deferrals (subagent 15)

Subagent 15 was the largest single plan in the index (gRPC service +
4 stream types + 4 Read API features). Per the parent's pragmatic
posture (precedent: plans 4, 5, 7, 8, 9, 10, 11, 12, 13, 14) the
subagent landed two families and **deferred** the remaining four
to a focused follow-up subagent of this plan. Each deferred row keeps
its `⏳` marker on the matching ROADMAP.md section and the
deferral target re-points at this plan. The "no silent approximation"
rule prohibits the subagent from faking any of the deferred behavior
with a partial implementation that the third-party Go client tests
would not notice.

Read API:

| Family | Shape | Status | Why deferred |
|---|---|---|---|
| 1 | `ReadOptions.selected_fields` projection pushdown | ✅ landed (`storage-read-write` Family 1) | DuckDB backend honors caller-pinned column subset; handler validates names at session-mint time; response schema reflects projection |
| 2 | Multi-clause `ReadOptions.row_restriction` | ⏳ deferred | The current single `<col> = <literal>` parser is the right "no silent approximation" gate; expanding it requires either an analyzer-resolved boolean expression (re-running GoogleSQL on the predicate, integrating with the semantic executor) or a custom multi-clause expression AST + SQL renderer. Doing the AST-only version (the cheap path) silently approximates BigQuery's multi-table-engine `simple_filter` semantics; the follow-up subagent should land the analyzer-resolved version |
| 3 | Avro output format | ⏳ deferred | The plan explicitly forbids adding a new system Avro library; in-tree Avro C++ build is out of scope for one subagent. Worth its own focused subagent that stages an Avro encoder against the existing DuckDB Arrow batches |
| 4 | `SplitReadStream` parallel scans | ⏳ deferred | Requires a deterministic + reproducible parquet-row-boundary partition with a stable handle (`ReadStream` is a stable id, not a fresh cursor). Without that, callers that resume a split stream after a transient failure see drift, which the schema-drift contract refuses to mask. Worth its own focused subagent that pairs the partition design with the cursor-checkpoint plumbing |

Write API:

| Family | Shape | Status | Why deferred |
|---|---|---|---|
| 5 | `BigQueryWrite` gRPC service skeleton + handler registry | ✅ landed (`storage-read-write` Family 5) | `proto/storage_write.proto` + `frontend/handlers/storage_write.{h,cc}` + `Server::Create` registration; full method surface, deferred RPCs return `UNIMPLEMENTED` with explicit messages |
| 6 | `_default` + `COMMITTED` stream types | ✅ landed (`storage-read-write` Family 6) | `CreateWriteStream` + bidi-streaming `AppendRows` + `GetWriteStream` route batches through plan 9's `DuckDBStorage::AppendRows` for immediate commit. Schema-shape mismatches surface on the recoverable `AppendRowsResponse.error_message` envelope |
| 7 | `BUFFERED` stream type + `FlushRows` | ⏳ deferred | Needs a per-stream visibility offset that decouples physical commit from caller-visible reads. The DuckDB parquet snapshot is per-table, not per-stream; the buffer plumbing has to live in the handler (or in a new storage primitive) and round-trip through `FlushRows` advances. Non-trivial. |
| 8 | `PENDING` stream type + `FinalizeWriteStream` | ⏳ deferred | Same buffer plumbing as Family 7, plus a finalization marker that gates a stream's rows from the visibility set until `BatchCommitWriteStreams` makes them live atomically |
| 9 | `BatchCommitWriteStreams` (atomic multi-stream commit) | ⏳ deferred | Atomic multi-stream commit on top of an isolation level the parquet snapshot file does not natively provide. Needs either a write-ahead log + replay or a transaction primitive on `DuckDBStorage` |

Things the follow-up subagent should know:

- Plan 14's `LOAD DATA LOCAL` reader hand-off note suggested a possible
  shared abstraction with the Storage Write API. Subagent 15 did not
  find a natural one inside its scope: the Storage Write API consumes
  rows directly off the gRPC bidi stream as protobuf `DataRow` cells,
  while `LOAD DATA LOCAL` would consume them as parsed CSV/Avro/JSON
  bytes. Both use `DuckDBStorage::AppendRows` as the common sink, but
  the producer-side parsing surface is different enough that the
  natural abstraction (a `RowBatchProducer` interface upstream of
  `AppendRows`) is its own follow-up.
- Plan 13's UDF/TVF storage round-trip: subagent 15 did not touch
  `DuckDBStorage`'s catalog round-trip surface (the Write API only
  consumes the existing `Storage::AppendRows` / `Storage::GetSchema`
  primitives), so plan 13's deferred families 4–8 stay blocked on the
  separate storage round-trip plumbing.
- The Go BigQuery client's `bigqueryreadclient` / `bigquerywriteclient`
  sample tests live under `third_party/golang-bigquery-tests/` and
  need a live emulator process bound to a port. Subagent 15 did not
  run them locally — they were deferred to CI (`task thirdparty:*`)
  per the parent prompt's allowance.
