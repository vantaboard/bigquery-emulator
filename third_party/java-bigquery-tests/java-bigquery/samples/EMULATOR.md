# BigQuery Java samples and the bigquery-emulator gateway

This file is a bigquery-emulator addition to the upstream `java-bigquery/samples/`
tree (it is not present in `googleapis/google-cloud-java`). It documents how the
slim-path Java lane wires the published `google-cloud-bigquery` client at the
local emulator gateway, and where to find the rest of the contract.

## TL;DR

```bash
task emulator:run-full                # start gateway + engine on :9050 / :9060
task thirdparty:java-bigquery-tests   # mvn -B verify on every JAVA_BQ_SAMPLE_PATHS module (Failsafe ITs against local emulator)
```

The sibling `install-without-bom/` and `snapshot/` modules are vendored verbatim
but are not built by the Task; only `snippets/` is in the loop.

## Companion snippet trees in this directory

`task thirdparty:java-bigquery-tests` no longer builds **only** `java-bigquery/`.
The parent `third_party/java-bigquery-tests/` directory now also vendors three
sibling snippet trees so the cloud.google.com sample IDs in those product areas
are present and stay compile-clean against upstream API drift:

| Tree | Upstream | Built by the task? |
|------|----------|--------------------|
| `java-bigquery/samples/snippets` (this dir) | `googleapis/google-cloud-java` HEAD | yes |
| `java-bigquerystorage/samples/snippets` | `googleapis/google-cloud-java` HEAD | yes |
| `java-docs-samples/bigquery/bigqueryconnection/snippets` | `GoogleCloudPlatform/java-docs-samples` HEAD | yes |
| `java-docs-samples/bigquery/bigquerydatatransfer/snippets` | `GoogleCloudPlatform/java-docs-samples` HEAD | yes |

All four trees now drive a curated subset of their vendored
`src/test/java/**/*IT.java` suite as live Failsafe ITs against this
emulator. The four sibling snippet modules ship per-surface
`Bq{Storage,Connection,DataTransfer}Opts` helpers (`BqOpts` for core
BigQuery here) that route service-client construction at the local
emulator when `BIGQUERY_EMULATOR_HOST` /
`BIGQUERY_STORAGE_GRPC_ENDPOINT` are exported. The bigqueryconnection /
bigquerydatatransfer / bigquerystorage trees currently fail with
`NOT_IMPLEMENTED`-shaped errors until Phase B
(`.cursor/plans/java-its-shallow-emulators_b8c9d0e1.plan.md`) lands the
matching shallow gRPC backends; the full per-IT verdict baseline lives
in `.cursor/plans/java-its-task-conversion_a7b8c9d0.plan.md`. The full
sample-ID-to-class index lives in `third_party/README.md`'s "Sample
coverage" sub-section.

## Live Failsafe by default

The vendored `src/test/java/**/*IT.java` suite no longer ships
`<maven.test.skip>true</maven.test.skip>` / `<skipTests>true</skipTests>`
in the snippets POM. Each POM instead binds
`maven-failsafe-plugin` 3.2.5 to the `integration-test` + `verify`
goals with a `<includes>` allowlist that names exactly the
live-IT-track-targeted classes, plus `<systemPropertyVariables>` that
forwards the emulator + storage gRPC endpoints into the forked test
JVMs. A raw `mvn -B verify` from inside the module runs the same
allowlist; the Task wrapper adds the env-var exports
(`BIGQUERY_EMULATOR_HOST=http://localhost:9050`,
`BIGQUERY_STORAGE_GRPC_ENDPOINT=localhost:9060`) and bring-up of the
docker-compose emulator. Set `JAVA_BQ_SKIP_TESTS=true` to revert to the
legacy compile-only posture.

## Emulator wiring (BqOpts)

The published `google-cloud-bigquery` does **not** auto-read
`BIGQUERY_EMULATOR_HOST`, unlike the Go client. Sample drivers that want to
talk to this emulator must route through the
[`BqOpts`](snippets/src/main/java/com/example/bigquery/BqOpts.java) helper added
under `com.example.bigquery`:

```java
import com.example.bigquery.BqOpts;
import com.google.cloud.bigquery.BigQuery;

BigQuery bq = BqOpts.builder().build().getService();
```

`BqOpts.builder()` reads:

- `BIGQUERY_EMULATOR_HOST` — `host:port` or `http(s)://host:port`. Schemeless
  values get `http://` prefixed, and credentials are forced to
  `NoCredentials.getInstance()`.
- `GOOGLE_CLOUD_PROJECT` / `GCLOUD_PROJECT` / `GOLANG_SAMPLES_PROJECT_ID` —
  first non-empty wins, applied as `setProjectId`.

Without `BIGQUERY_EMULATOR_HOST`, `BqOpts.builder()` falls through to
`BigQueryOptions.newBuilder()` and uses application-default credentials
(live BigQuery).

## Pointers

- Top-level contract, env vars, and skip rules:
  [`third_party/README.md`](../../README.md) (Java section).
- Local Task: [`taskfiles/thirdparty.yml`](../../../../taskfiles/thirdparty.yml)
  (`thirdparty:java-bigquery-tests`).
- CI lane: [`.github/workflows/thirdparty-samples.yml`](../../../../.github/workflows/thirdparty-samples.yml)
  (job `java-bigquery-tests-compile`).
- Helper: [`BqOpts.java`](snippets/src/main/java/com/example/bigquery/BqOpts.java).
