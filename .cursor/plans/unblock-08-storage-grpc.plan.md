---
name: Unblock 08 — Storage gRPC full
overview: Complete BigQuery Storage Write (buffered) and Read (Arrow) gRPC so Java bigquerystorage ITs pass without JAVA_BQ_ALLOW_FAILING_ITS escapes.
depends_on: [unblock-01-gcs-networking, unblock-07-hive-external]
blocks: [unblock-10-final-aggregator]
baseline_log: .logs/thirdparty-20260605-134407.log
est_effort: 2+ weeks
isProject: true
todos:
  - id: audit-partial
    content: Audit fb36866 BUFFERED flush path in frontend/handlers/storage_write_buffered.cc; list remaining UNIMPLEMENTED RPCs
    status: pending
  - id: create-write-stream
    content: CreateWriteStream gRPC — accept stream name, return write stream resource
    status: pending
  - id: append-finalize
    content: AppendRows + FinalizeWriteStream + BatchCommitWriteStreams commit rows to catalog
    status: pending
  - id: storage-read-arrow
    content: CreateReadSession + Arrow ReadRows for StorageArrowSampleIT
    status: pending
  - id: bazel-handler-tests
    content: Scoped //frontend/handlers/...storage* tests
    status: pending
  - id: shrink-allowlist
    content: Remove WriteBufferedStreamIT, StorageArrowSampleIT from JAVA_BQ_ALLOW_FAILING_ITS; update ENGINE_POLICY.md
    status: pending
  - id: gate-java-storage
    content: JAVA_BQ_SAMPLE_PATHS='java-bigquerystorage/samples/snippets' then full task thirdparty:java-bigquery-tests exit 0
    status: pending
  - id: status-commit
    content: Update orchestration-status.md; lint + commit (C++ heavy)
    status: pending
---

# Unblock 08 — Storage gRPC full

## Goal

Implement storage gRPC surfaces so Java thirdparty ITs pass **without** relying on `JAVA_BQ_ALLOW_FAILING_ITS`. User explicitly chose **full implementation** over allowlist expansion.

Consolidates and extends [thirdparty-10-storage-grpc.plan.md](thirdparty-10-storage-grpc.plan.md) in the unblock lane context.

## Baseline

| IT | Module | Error |
|----|--------|-------|
| `WriteBufferedStreamIT` | `java-bigquerystorage/samples/snippets` | `UNIMPLEMENTED` (partial BUFFERED at `fb36866`) |
| `StorageArrowSampleIT` | same | `UNIMPLEMENTED` |

Other Java modules use allowlist for transfer/connection snippets — keep honest allowlist for **non-storage** UNIMPLEMENTED RPCs.

## Partial work (do not redo)

- [`frontend/handlers/storage_write_buffered.cc`](../../frontend/handlers/storage_write_buffered.cc) — BUFFERED flush engine path
- [`docs/ENGINE_POLICY.md`](../../docs/ENGINE_POLICY.md) — storage posture section

## Architecture

```mermaid
sequenceDiagram
  participant Java as Java_ManagedWriter
  participant GRPC as emulator_9060
  participant FE as frontend_handlers
  participant Engine as emulator_main
  Java->>GRPC: CreateWriteStream
  GRPC->>FE: dispatch
  FE->>Engine: AppendRows / flush
  Java->>GRPC: FinalizeWriteStream
  FE->>Engine: commit to catalog
```

## Phase A — Write API

**Protos / entrypoints:**

- [`proto/`](../../proto/) storage write definitions
- [`frontend/handlers/`](../../frontend/handlers/) gRPC service impls
- [`gateway/gateway.go`](../../gateway/gateway.go) or engine mux on `:9060`

**RPCs:**

1. `CreateWriteStream` — stream resource + schema
2. `AppendRows` — protobuf rows or Arrow IPC per client
3. `FinalizeWriteStream` — commit to destination table
4. `BatchCommitWriteStreams` — if required by IT

Honor `PENDING` vs `COMMITTED` per API contract.

## Phase B — Read API

1. `CreateReadSession` (audit existing partial impl)
2. Arrow-encoded `ReadRows` stream
3. Schema compatibility with table metadata for `StorageArrowSampleIT`

## Phase C — Allowlist shrink

[`taskfiles/thirdparty.yml`](../../taskfiles/thirdparty.yml) `JAVA_BQ_ALLOW_FAILING_ITS`:

- Remove `WriteBufferedStreamIT`, `StorageArrowSampleIT` once green
- Keep transfer/connection ITs until those products are out of scope

## Testing strategy

```bash
# Fast (per iteration — ONE bazel invocation)
task bazel:test -- //frontend/handlers/...storage*

# Scoped Java
JAVA_BQ_SAMPLE_PATHS='java-bigquerystorage/samples/snippets' task thirdparty:java-bigquery-tests

# Full Java (plan 10 verifies)
task thirdparty:java-bigquery-tests
```

## Hygiene (mandatory)

Per [`.cursor/rules/bazel-process-hygiene.mdc`](../rules/bazel-process-hygiene.mdc):

- One `bazel build/test` at a time
- Pre-spawn `task bazel:status`
- End-of-plan `task bazel:shutdown` + kill-strays

Do **not** start this plan until plans **01** and **07** are stable — debugging GCS + gRPC simultaneously is costly.

## Out of scope

- BigQuery Read SQL API (REST) changes unrelated to storage ITs
- ManagedWriter client library changes in `third_party/`

## Done criteria

- [ ] `java-bigquerystorage/samples/snippets` mvn verify exit 0 without allowlist for write/read ITs
- [ ] `ENGINE_POLICY.md` reflects implemented RPCs
- [ ] No new unexpected failing ITs in other Java modules
