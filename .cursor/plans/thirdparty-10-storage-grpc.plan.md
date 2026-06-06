---
name: Thirdparty 10 — Storage gRPC write & read
overview: Implement BigQuery Storage Write API (buffered stream) and Arrow read path for Java thirdparty ITs and ManagedWriter clients.
depends_on: [thirdparty-01-harness]
blocks: []
baseline_log: .logs/thirdparty-20260605-112926.log
est_effort: 2+ weeks
isProject: true
todos:
  - id: create-write-stream
    content: Implement CreateWriteStream gRPC — replace UNIMPLEMENTED
    status: pending
  - id: append-rows
    content: AppendRows / FlushRows buffered write semantics
    status: pending
  - id: finalize-commit
    content: FinalizeWriteStream and BatchCommitWriteStreams
    status: pending
  - id: storage-read-arrow
    content: Storage Read Arrow stream for StorageArrowSampleIT
    status: pending
  - id: storage-e2e
    content: Java bigquerystorage snippets ITs pass without allowlist
    status: pending
  - id: policy-docs
    content: Update ENGINE_POLICY.md and ROADMAP.md for storage write/read posture
    status: pending
---

# Thirdparty 10 — Storage gRPC write & read

## Goal

Implement storage gRPC surfaces so Java `bigquerystorage` thirdparty ITs pass **without** relying on `JAVA_BQ_ALLOW_FAILING_ITS`.

## Baseline failures

After plan 01 (harness), Maven reports:

| IT | Error |
|----|-------|
| `WriteBufferedStreamIT` | `io.grpc.StatusRuntimeException: UNIMPLEMENTED` |
| `StorageArrowSampleIT` | `io.grpc.StatusRuntimeException: UNIMPLEMENTED` |

Module: `third_party/java-bigquery-tests/java-bigquerystorage/samples/snippets`

## Current policy

[`docs/ENGINE_POLICY.md`](../../docs/ENGINE_POLICY.md):

- ManagedWriter / Storage Read skipped unless `BIGQUERY_STORAGE_GRPC_ENDPOINT` set
- `CreateWriteStream` returns `UNIMPLEMENTED` for buffered streams
- `FinalizeWriteStream` / `BatchCommitWriteStreams` deferred

[`ROADMAP.md`](../../ROADMAP.md) conformance section documents shallow storage backends.

## Proto & handlers

**Read existing:**
- [`proto/storage_read.proto`](../../proto/storage_read.proto)
- Storage write protos (if separate) under [`proto/`](../../proto/)
- Frontend gRPC service implementations under [`frontend/`](../../frontend/) or [`backend/`](../../backend/)
- [`gateway/gateway.go`](../../gateway/gateway.go) — gRPC mux on `:9060`

## Implementation phases

### Phase A — WriteBufferedStreamIT

1. `CreateWriteStream` — accept stream name, return write stream resource
2. `AppendRows` — accept serialized rows (protobuf rows or Arrow IPC per client)
3. `FinalizeWriteStream` — commit rows to destination table in catalog/storage
4. Honor `PENDING` vs `COMMITTED` stream types per API contract

### Phase B — StorageArrowSampleIT

1. Storage Read `CreateReadSession` (may partially exist)
2. Arrow-encoded `ReadRows` stream
3. Schema compatibility with table metadata

## Testing strategy

```bash
# Scoped Java (after gateway+engine rebuild)
task thirdparty:java-bigquery-tests
# JAVA_BQ_SAMPLE_PATHS='java-bigquerystorage/samples/snippets' ...

# Engine/gRPC unit tests
task bazel:test -- //frontend/... //backend/...storage...
```

Prefer **grpc-level tests** before full Maven Failsafe (faster iteration).

## Allowlist posture

Plan 01 fixes allowlist **reporting**. This plan **removes the need** for allowlisting storage ITs:

- Update default `JAVA_BQ_ALLOW_FAILING_ITS` in [`taskfiles/thirdparty.yml`](../../taskfiles/thirdparty.yml) to drop `WriteBufferedStreamIT,StorageArrowSampleIT` once green
- Update [`.github/workflows/thirdparty-samples.yml`](../../.github/workflows/thirdparty-samples.yml) similarly

Connection (`CreateAwsConnectionIT`, etc.) and DataTransfer ITs remain allowlisted until their dedicated plans land.

## Out of scope

- BigQuery Connection gRPC ([`thirdparty-16-bqconnection-grpc`](./thirdparty-16-bqconnection-grpc.plan.md) if created later)
- Data Transfer gRPC
- Exactly-once / stream resumption edge cases (implement minimal happy path first)

## Done when

- [ ] `WriteBufferedStreamIT` and `StorageArrowSampleIT` pass
- [ ] `task thirdparty:java-bigquery-tests` green for bigquerystorage module without allowlist
- [ ] ENGINE_POLICY storage section reflects implemented semantics
