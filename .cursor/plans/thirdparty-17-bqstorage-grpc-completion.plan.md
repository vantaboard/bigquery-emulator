---
name: bqstorage gRPC — finish deferred families
overview: "Close out the four families deferred by the storage-read-write-api-plan subagent: Avro output, SplitReadStream parallel scans, multi-clause row_restriction, BUFFERED + PENDING + atomic BatchCommitWriteStreams stream types. Each family has a documented deferral reason in the parent plan that must be honored (no silent approximation)."
todos:
  - id: tp17_avro
    content: "Read API family 3 — Avro output format. Stage an Avro encoder against the existing DuckDB Arrow batches without adding a system Avro library (vendor the C++ encoder if needed)."
    status: pending
  - id: tp17_split_read
    content: "Read API family 4 — SplitReadStream. Deterministic + reproducible parquet-row-boundary partitioning with a stable ReadStream handle that survives reconnects without drift."
    status: pending
  - id: tp17_row_restriction
    content: "Read API family 2 — multi-clause row_restriction via analyzer-resolved boolean expression (re-run GoogleSQL on the predicate, integrate with the semantic executor). Reject unsupported shapes at CreateReadSession (no silent approximation)."
    status: pending
  - id: tp17_buffered
    content: "Write API family 7 — BUFFERED stream type + FlushRows. Per-stream visibility offset that decouples physical commit from caller-visible reads."
    status: pending
  - id: tp17_pending
    content: "Write API family 8 — PENDING stream type + FinalizeWriteStream. Builds on family 7's buffer plumbing; adds a finalization marker that gates rows from the visibility set until BatchCommitWriteStreams flips them in."
    status: pending
  - id: tp17_batch_commit
    content: "Write API family 9 — BatchCommitWriteStreams (atomic multi-stream commit). Needs either a WAL + replay or a transaction primitive on DuckDBStorage."
    status: pending
  - id: tp17_tests
    content: "Per-family unit tests + thirdparty:golang-bigquery-tests for the Go BigQuery client's Storage Read + Write samples; conformance fixtures under conformance/fixtures/storage/."
    status: pending
  - id: tp17_roadmap
    content: "ROADMAP.md: flip the four ⏳ rows in `Storage Read API (gRPC)` and the four ⏳ rows in `Storage Write API (gRPC)` to ✅; link to this plan."
    status: pending
  - id: tp17_index
    content: "Flip thirdparty-00-completion-index.plan.md todo `tp17` to `completed`."
    status: pending
isProject: false
---

# Thirdparty 17 — bqstorage gRPC completion

## Source

- `fix_thirdparty_failures_cbe91a41.plan.md` Tier 4.2 (now deleted).
- Parent plan with the deferral matrix:
  [`storage-read-write-api-plan.plan.md`](./storage-read-write-api-plan.plan.md)
  "Notes & deferrals (subagent 15)".
- Failing Java ITs (UNIMPLEMENTED or partial): `WriteBufferedStreamIT`,
  `StorageArrowSampleIT`, plus the Go BigQuery client's
  `bigqueryreadclient` and `bigquerywriteclient` samples under
  `third_party/golang-bigquery-tests/`.

## Prerequisites

- Read [`storage-read-write-api-plan.plan.md`](./storage-read-write-api-plan.plan.md)
  end-to-end. The four deferred Read API families and four deferred
  Write API families documented there are the scope of this plan
  verbatim — do not re-scope.
- The "no silent approximation" rule from the parent plan applies:
  shipping a partial implementation of any of these families that
  the third-party Go client tests would not notice is explicitly
  prohibited.

## Scope (deferral matrix from the parent plan)

### Read API

| Family | Shape | Deferral reason |
|---|---|---|
| 2 | Multi-clause `ReadOptions.row_restriction` | The current single `<col> = <literal>` parser is the right gate; expanding it requires either an analyzer-resolved boolean expression or a custom multi-clause expression AST. AST-only silently approximates BigQuery's `simple_filter` semantics; ship the analyzer-resolved version. |
| 3 | Avro output format | The plan explicitly forbids adding a new system Avro library; in-tree Avro C++ build is out of scope for one subagent. Stage an Avro encoder against the existing DuckDB Arrow batches. |
| 4 | `SplitReadStream` parallel scans | Requires a deterministic + reproducible parquet-row-boundary partition with a stable handle (ReadStream is a stable id, not a fresh cursor). Without that, callers that resume a split stream after a transient failure see drift, which the schema-drift contract refuses to mask. |

### Write API

| Family | Shape | Deferral reason |
|---|---|---|
| 7 | `BUFFERED` stream type + `FlushRows` | Needs a per-stream visibility offset that decouples physical commit from caller-visible reads. The DuckDB parquet snapshot is per-table, not per-stream; the buffer plumbing has to live in the handler (or in a new storage primitive) and round-trip through FlushRows advances. |
| 8 | `PENDING` stream type + `FinalizeWriteStream` | Same buffer plumbing as family 7, plus a finalization marker that gates a stream's rows from the visibility set until BatchCommitWriteStreams makes them live atomically. |
| 9 | `BatchCommitWriteStreams` (atomic multi-stream commit) | Atomic multi-stream commit on top of an isolation level the parquet snapshot file does not natively provide. Needs either a WAL + replay or a transaction primitive on DuckDBStorage. |

## Implementation

Order the work as: 7 -> 8 -> 9 (the Write API trio shares
buffer / finalization plumbing) and 2 -> 3 -> 4 in parallel
(the Read API trio is more independent).

### Read API

1. **Family 2 (row_restriction).** Re-parse the restriction through
   GoogleSQL into a resolved boolean expression. Evaluate via the
   transpiler / semantic executor. Reject still-unsupported shapes at
   `CreateReadSession` with a documented `UNIMPLEMENTED`.
2. **Family 3 (Avro).** Vendor or build an Avro C++ encoder against
   the existing DuckDB Arrow batches. Reuse the existing
   schema-mapping code that the Arrow output already has.
3. **Family 4 (SplitReadStream).** Partition the underlying scan
   range and return multiple ReadStreams with the same Arrow
   schema. Each stream is a DuckDB cursor over a subrange; the
   partition design must produce reproducible boundaries so a
   resumed stream lands on the same rows.

### Write API

5. **Family 7 (BUFFERED + FlushRows).** Per-stream visibility
   offset in the handler (or a new `DuckDBStorage::AppendBuffered`
   primitive that the handler advances on FlushRows).
6. **Family 8 (PENDING + FinalizeWriteStream).** Same buffer
   plumbing as family 7, plus a finalization marker on the buffer.
7. **Family 9 (BatchCommitWriteStreams).** Atomic multi-stream
   commit. Choose either WAL + replay or a transaction primitive
   on DuckDBStorage; document the choice + tradeoffs in the
   commit.

## Tests

- Per-family unit tests under `frontend/handlers/storage_write*` /
  `storage_read*` (mirroring what subagent 15 already landed).
- Engine-level integration tests in `gateway/e2e/storage/`:
  - Read API: multi-clause `row_restriction`, `SplitReadStream`
    parallel scan, Avro output.
  - Write API: BUFFERED stream with FlushRows; PENDING stream
    with finalize + batch commit; schema mismatch rejection.
- `task thirdparty:golang-bigquery-tests` —
  `bigqueryreadclient` and `bigquerywriteclient` samples run
  green against the emulator.
- Conformance fixtures under `conformance/fixtures/storage/`.

## ROADMAP

Flip the four `⏳` rows in the `Storage Read API (gRPC)` ROADMAP
section and the four `⏳` rows in the `Storage Write API (gRPC)`
section to `✅`; cross-reference back to this plan.

## Done criteria

- All six deferred families are implemented (no silent
  approximation; every family has a real implementation that the
  Go BigQuery client tests notice).
- ROADMAP shows the eight rows flipped to ✅.
- Conformance fixtures pin every new family.
- `thirdparty-00-completion-index.plan.md` todo `tp17` flipped to
  `completed`.
