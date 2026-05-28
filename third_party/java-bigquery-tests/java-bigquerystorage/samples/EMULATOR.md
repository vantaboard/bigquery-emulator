# BigQuery Storage Java samples and the bigquery-emulator gateway

This file is a bigquery-emulator addition to the upstream
`java-bigquerystorage/samples/` tree (it is not present in
`googleapis/google-cloud-java`). It documents how the slim-path Java
lane wires the published `google-cloud-bigquerystorage` client at the
local emulator's BigQuery Storage gRPC listener (default: `:9060`).

## TL;DR

```bash
task emulator:run-full                # start gateway + engine on :9050 / :9060
task thirdparty:java-bigquery-tests   # mvn -B verify on every JAVA_BQ_SAMPLE_PATHS module (Failsafe ITs against local emulator)
```

## Live Failsafe by default

`snippets/pom.xml` binds `maven-failsafe-plugin` 3.2.5 to the
`integration-test` + `verify` goals with a `<includes>` allowlist that
names exactly the live-IT-track-targeted classes for this module.
After the missing-tests follow-up
(`.cursor/plans/java-its-missing-tests_c9d0e1f2.plan.md`) the
allowlist covers **2** classes:

| Sample ID | IT |
|-----------|----|
| `bigquerystorage-jsonstreamwriter-buffered` | `WriteBufferedStreamIT` |
| `bigquerystorage-arrow-quickstart` | `StorageArrowSampleIT` |

## Expected status

Both ITs currently **FAIL** with
`io.grpc.StatusRuntimeException: UNIMPLEMENTED`:

- `WriteBufferedStreamIT` (row 7 of the shallow-emulator verdict
  table) — the gzip middleware unblocked the `@Before`
  `POST /datasets` REST call; the test now reaches
  `BigQueryWrite.CreateWriteStream` which the
  `gateway/handlers/bqstorage/` skeleton does not implement.
- `StorageArrowSampleIT` (added by the missing-tests follow-up) —
  the snippet calls `BigQueryRead.CreateReadSession`; the same
  shallow-emulator skeleton package has no Read-path implementation
  (the shallow-emulator port explicitly deferred the Read path; see
  the shallow-emulator "Side-quests deferred" notes).

The gRPC-server follow-up will port the streaming Write API
(`write*.go`) and a minimal Read path (`read*.go`) from
go-googlesql to land the matching gRPC backends. The
shallow-emulator skeleton's `doc.go` carries the per-IT mapping so
the re-port has a one-to-one target.

## Emulator wiring (BqStorageOpts)

The snippet drivers route through
[`BqStorageOpts.newReadClient()`](snippets/src/main/java/com/example/bigquerystorage/BqStorageOpts.java)
/ `BqStorageOpts.newWriteClient()` to construct the gapic clients; the
helper reads `BIGQUERY_STORAGE_GRPC_ENDPOINT` first, falls back to
`BIGQUERY_EMULATOR_HOST` (stripping the scheme), and forces plaintext +
`NoCredentialsProvider` when either is set. `WriteBufferedStreamIT`
additionally uses a `BIGQUERY_EMULATOR_HOST`-aware `BigQuery` client
for the `@Before` dataset/table fixture; `StorageArrowSampleIT` reads
from `bigquery-public-data` and does not need a fixture.

## Pointers

- Top-level contract, env vars, and skip rules:
  [`third_party/README.md`](../../README.md) (Java section).
- Sibling EMULATOR.md (core BigQuery wiring):
  [`java-bigquery/samples/EMULATOR.md`](../../java-bigquery/samples/EMULATOR.md).
- Shallow-emulator `bqstorage` skeleton package (and its per-IT
  mapping):
  [`gateway/handlers/bqstorage/`](../../../../gateway/handlers/bqstorage/).
- Local Task: [`taskfiles/thirdparty.yml`](../../../taskfiles/thirdparty.yml)
  (`thirdparty:java-bigquery-tests`).
- Per-IT verdict baselines (failing-IT inventory / shallow-emulator
  port / missing-tests follow-up):
  [`.cursor/plans/java-its-*.plan.md`](../../../.cursor/plans/).
