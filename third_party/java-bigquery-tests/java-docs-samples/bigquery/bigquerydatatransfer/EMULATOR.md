# BigQuery Data Transfer Java samples and the bigquery-emulator gateway

This file is a bigquery-emulator addition to the upstream
`java-docs-samples/bigquery/bigquerydatatransfer/` tree (it is not present
in `GoogleCloudPlatform/java-docs-samples`). It documents how the slim-path
Java lane wires the published `google-cloud-bigquerydatatransfer` client at
the local emulator listener, and where to find the rest of the contract.

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
(`ROADMAP.md`) the
allowlist covers **11** classes — the failing-IT inventory patched 3,
the missing-tests follow-up authored 8 new `Create*TransferIT` smoke
ITs:

| Sample ID | IT |
|-----------|----|
| `bigquerydatatransfer-create-amazons3-transfer` | `CreateAmazonS3TransferIT` |
| `bigquerydatatransfer-disable-transfer` | `DisableTransferConfigIT` |
| `bigquerydatatransfer-reenable-transfer` | `ReEnableTransferConfigIT` |
| `bigquerydatatransfer-create-admanager-transfer` | `CreateAdManagerTransferIT` |
| `bigquerydatatransfer-create-ads-transfer` | `CreateAdsTransferIT` |
| `bigquerydatatransfer-create-campaignmanager-transfer` | `CreateCampaignmanagerTransferIT` |
| `bigquerydatatransfer-create-play-transfer` | `CreatePlayTransferIT` |
| `bigquerydatatransfer-create-redshift-transfer` | `CreateRedshiftTransferIT` |
| `bigquerydatatransfer-create-teradata-transfer` | `CreateTeradataTransferIT` |
| `bigquerydatatransfer-create-youtubechannel-transfer` | `CreateYoutubeChannelTransferIT` |
| `bigquerydatatransfer-create-youtubecontentowner-transfer` | `CreateYoutubeContentOwnerTransferIT` |

## Expected status

All 11 ITs currently **FAIL** with
`io.grpc.StatusRuntimeException: UNIMPLEMENTED` against the local
emulator — the gapic `DataTransferServiceClient` dials gRPC, not
REST, and the shallow-emulator `gateway/handlers/datatransfer/`
package serves the REST surface only. The gRPC-server follow-up will
land a DTS gRPC bridge (either by adding gRPC handlers or by
translating gRPC calls back into REST handler calls).
When that lands, the 8 third-party-connector smokes are expected to
PASS (the emulator's catalog already accepts every connector ID the
8 drivers send — see
[`gateway/handlers/datatransfer/catalog.go`](../../../../../gateway/handlers/datatransfer/catalog.go)).

Two ITs have additional shallow-emulator / missing-tests-follow-up
notes:

- `CreateAmazonS3TransferIT` (row 13 of the shallow-emulator verdict
  table) — the IT's `setUp` issues
  `bigquery.create(DatasetInfo.of(...))` and used to land on live
  BigQuery (401). Root cause: the upstream POM pinned
  `libraries-bom` 26.32.0, which resolves `google-cloud-core` to
  2.32.0, whose `ServiceOptions.getResolvedApiaryHost()`
  unconditionally returns `https://<service>.googleapis.com/` and
  ignores any custom host set via
  `BigQueryOptions.Builder.setHost()`. The missing-tests follow-up
  bumped the BOM to 26.73.0 (`google-cloud-core` 2.62.2 honors the
  configured host), which is the snippet-side
  BIGQUERY_EMULATOR_HOST honoring fix the shallow-emulator deferral
  list called out. After the bump, `setUp` succeeds against the
  emulator and the test method reaches the gRPC layer.
- `DisableTransferConfigIT` / `ReEnableTransferConfigIT` (rows 14,
  15 of the shallow-emulator verdict table) —
  `taskfiles/thirdparty.yml` exports a default
  `DTS_TRANSFER_CONFIG_NAME`
  (`projects/${PROJECT}/locations/us/transferConfigs/emulator-fixture`)
  and pre-creates the fixture via curl POST to the new datatransfer
  REST handler. The gapic patch / disable call still dials gRPC and
  fails UNIMPLEMENTED.

## Emulator wiring (BqDataTransferOpts)

The snippet drivers route through
[`BqDataTransferOpts.newClient()`](snippets/src/main/java/com/example/bigquerydatatransfer/BqDataTransferOpts.java)
to construct the gapic client; it reads `BIGQUERY_STORAGE_GRPC_ENDPOINT`
first, falls back to `BIGQUERY_EMULATOR_HOST` (stripping the scheme),
and forces plaintext + `NoCredentialsProvider` when either is set. The
ITs additionally bake in a local `newBigQueryService()` helper that
configures the `google-cloud-bigquery` REST client at
`BIGQUERY_EMULATOR_HOST` so the `@Before` dataset / table fixtures land
in the local emulator.

## Pointers

- Top-level contract, env vars, and skip rules:
  [`third_party/README.md`](../../../README.md) (Java section).
- Sibling EMULATOR.md (core BigQuery wiring):
  [`java-bigquery/samples/EMULATOR.md`](../../../java-bigquery/samples/EMULATOR.md).
- Local Task: [`taskfiles/thirdparty.yml`](../../../../taskfiles/thirdparty.yml)
  (`thirdparty:java-bigquery-tests`).
- Per-IT verdict baselines (failing-IT inventory / shallow-emulator
  port / missing-tests follow-up):
  [`ROADMAP.md`](../../../../../../ROADMAP.md).
