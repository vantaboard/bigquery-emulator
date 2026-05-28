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
names exactly the live-IT-track-targeted classes for this module. After
Phase C
(`.cursor/plans/java-its-missing-tests_c9d0e1f2.plan.md`) the allowlist
covers **11** classes — Phase A patched 3, Phase C authored 8 new
`Create*TransferIT` smoke ITs:

| Sample ID | IT |
|-----------|----|
| `bigquerydatatransfer-create-amazons3-transfer` | `CreateAmazonS3TransferIT` (Phase A) |
| `bigquerydatatransfer-disable-transfer` | `DisableTransferConfigIT` (Phase A) |
| `bigquerydatatransfer-reenable-transfer` | `ReEnableTransferConfigIT` (Phase A) |
| `bigquerydatatransfer-create-admanager-transfer` | `CreateAdManagerTransferIT` (Phase C) |
| `bigquerydatatransfer-create-ads-transfer` | `CreateAdsTransferIT` (Phase C) |
| `bigquerydatatransfer-create-campaignmanager-transfer` | `CreateCampaignmanagerTransferIT` (Phase C) |
| `bigquerydatatransfer-create-play-transfer` | `CreatePlayTransferIT` (Phase C) |
| `bigquerydatatransfer-create-redshift-transfer` | `CreateRedshiftTransferIT` (Phase C) |
| `bigquerydatatransfer-create-teradata-transfer` | `CreateTeradataTransferIT` (Phase C) |
| `bigquerydatatransfer-create-youtubechannel-transfer` | `CreateYoutubeChannelTransferIT` (Phase C) |
| `bigquerydatatransfer-create-youtubecontentowner-transfer` | `CreateYoutubeContentOwnerTransferIT` (Phase C) |

## Expected status

All 11 ITs currently **FAIL** with
`io.grpc.StatusRuntimeException: UNIMPLEMENTED` against the local
emulator — the gapic `DataTransferServiceClient` dials gRPC, not REST,
and Phase B's `gateway/handlers/datatransfer/` package serves the REST
surface only. Phase D will land a DTS gRPC bridge (either by re-porting
go-googlesql's gRPC handlers or by translating gRPC calls back into REST
handler calls). When that lands, the 8 Phase C smokes are expected to
PASS (the emulator's catalog already accepts every connector ID the 8
drivers send — see
[`gateway/handlers/datatransfer/catalog.go`](../../../../../gateway/handlers/datatransfer/catalog.go)).

Two ITs have additional Phase B / Phase C notes:

- `CreateAmazonS3TransferIT` (row 13 of the Phase B verdict table) —
  the IT's `setUp` issues `bigquery.create(DatasetInfo.of(...))` and
  used to land on live BigQuery (401). Root cause: the upstream POM
  pinned `libraries-bom` 26.32.0, which resolves `google-cloud-core`
  to 2.32.0, whose `ServiceOptions.getResolvedApiaryHost()`
  unconditionally returns `https://<service>.googleapis.com/` and
  ignores any custom host set via
  `BigQueryOptions.Builder.setHost()`. Phase C bumped the BOM to
  26.73.0 (`google-cloud-core` 2.62.2 honors the configured host),
  which is the snippet-side BIGQUERY_EMULATOR_HOST honoring fix the
  Phase B deferral list called out. After the bump, `setUp` succeeds
  against the emulator and the test method reaches the gRPC layer.
- `DisableTransferConfigIT` / `ReEnableTransferConfigIT` (rows 14, 15
  of the Phase B verdict table) — `taskfiles/thirdparty.yml` exports
  a default `DTS_TRANSFER_CONFIG_NAME`
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
- Per-IT verdict baselines (Phase A / B / C):
  [`.cursor/plans/java-its-*.plan.md`](../../../../.cursor/plans/).
