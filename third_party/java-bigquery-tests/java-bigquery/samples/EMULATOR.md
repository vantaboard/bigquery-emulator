# BigQuery Java samples and the bigquery-emulator gateway

This file is a bigquery-emulator addition to the upstream `java-bigquery/samples/`
tree (it is not present in `googleapis/google-cloud-java`). It documents how the
slim-path Java lane wires the published `google-cloud-bigquery` client at the
local emulator gateway, and where to find the rest of the contract.

## TL;DR

```bash
task emulator:run-full                # start gateway + engine on :9050 / :9060
task thirdparty:java-bigquery-tests   # mvn -B package -Dmaven.test.skip=true on every JAVA_BQ_SAMPLE_PATHS module
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

Importantly, the bigquery-emulator does **not** implement the
`bigqueryconnection`, `bigquerydatatransfer`, or `bigquerystorage` gRPC
backends, so the latter three trees stay strictly compile-only — they
exist to catch upstream API drift in the libraries-bom-resolved
client artifacts, not to drive live ITs against this emulator.
Promotion of any of them to a live IT lane is a follow-up tracked by the
top-level `third_party/README.md`. The full sample-ID-to-class index lives
in that README's "Sample coverage" sub-section.

## Compile-only by default

The vendored `src/test/java/**/*IT.java` suite expects ADC and a real BigQuery
project, so the task and the GitHub workflow both build with
`-Dmaven.test.skip=true`. The snippets POM also sets
`<maven.test.skip>true</maven.test.skip>` so a raw `mvn package` matches the
Task's behaviour. Promotion to a live emulator IT lane is a follow-up; see
`third_party/README.md` (Java section) for the rationale and the link to the
go-googlesql commits that walked back the heavier vendored-reactor approach.

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
