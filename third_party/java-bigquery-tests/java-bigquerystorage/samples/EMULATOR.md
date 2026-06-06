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
names exactly the live-IT-track-targeted classes for this module:

| Sample ID | IT |
|-----------|----|
| `bigquerystorage-jsonstreamwriter-buffered` | `WriteBufferedStreamIT` |
| `bigquerystorage-arrow-quickstart` | `StorageArrowSampleIT` |

## Expected status

Both ITs **PASS** against the local emulator when
`BIGQUERY_EMULATOR_HOST` and `BIGQUERY_STORAGE_GRPC_ENDPOINT` are set
(as `task thirdparty:java-bigquery-tests` does). The gateway shim in
`gateway/handlers/bqstorage/` registers public `BigQueryRead` /
`BigQueryWrite` on `:9060` and adapts to the engine handlers.

## Emulator wiring (BqStorageOpts)

The snippet drivers route through
[`BqStorageOpts.newReadClient()`](snippets/src/main/java/com/example/bigquerystorage/BqStorageOpts.java)
/ `BqStorageOpts.newWriteClient()` to construct the gapic clients; the
helper reads `BIGQUERY_STORAGE_GRPC_ENDPOINT` first, falls back to
`BIGQUERY_EMULATOR_HOST` (stripping the scheme), and forces plaintext +
`NoCredentialsProvider` when either is set.

`WriteBufferedStream` passes the same `BigQueryWriteClient` into
`JsonStreamWriter.newBuilder(..., client)` so append batches stay on the
emulator endpoint. `WriteBufferedStreamIT` additionally uses a
`BIGQUERY_EMULATOR_HOST`-aware `BigQuery` client for the `@Before`
dataset/table fixture. `StorageArrowSampleIT` reads
`bigquery-public-data.usa_names.usa_1910_current` (seeded in
`testdata/public-data/bigquery-public-data.yaml`, including the Zayvion
WA row and enough WA rows for the >1KiB Arrow TSV assertion).

Failsafe sets `--add-opens=java.base/java.nio=ALL-UNNAMED` for the
Apache Arrow read path on Java 17+.

## Pointers

- Top-level contract, env vars, and skip rules:
  [`third_party/README.md`](../../README.md) (Java section).
- Sibling EMULATOR.md (core BigQuery wiring):
  [`java-bigquery/samples/EMULATOR.md`](../../java-bigquery/samples/EMULATOR.md).
- Gateway shim package:
  [`gateway/handlers/bqstorage/`](../../../../gateway/handlers/bqstorage/).
- Local Task: [`taskfiles/thirdparty.yml`](../../../taskfiles/thirdparty.yml)
  (`thirdparty:java-bigquery-tests`).
