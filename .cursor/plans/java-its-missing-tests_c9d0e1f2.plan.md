---
name: java-its-missing-tests
overview: "Java live-IT track Phase C: author 9 new `*IT.java` (8 bigquerydatatransfer Create*Transfer variants + StorageArrowSample) for the samples in the 24-URL target list that ship no upstream IT; extend Phase B's datatransfer data-source catalog with the 8 missing source IDs; flip the sample-coverage table to 24/24 PASS."
todos:
  - id: register-missing-datasources
    content: "Extend Phase B's datatransfer catalog (`gateway/handlers/datatransfer/catalog.go`) with the 8 third-party data source IDs (`admanager_transfer`, `google_ads`, `dcm_dt`, `play`, `redshift`, `teradata`, `youtube_channel`, `youtube_content_owner`) so `CreateTransferConfig` succeeds for each Create*Transfer sample."
    status: pending
  - id: author-9-its
    content: "Author 9 new `*IT.java` modeled on `CreateAmazonS3TransferIT` / `QuickstartArrowSampleIT`, routing through Phase A's `BqDataTransferOpts` / `BqStorageOpts`, invoking the corresponding driver's runSample method, asserting a minimal post-condition."
    status: pending
  - id: coverage-table-and-validate
    content: "Update `third_party/README.md` 'Sample coverage' table to mark all 24 samples as IT-shipped; rerun `task thirdparty:java-bigquery-tests` and capture a 24/24-PASS verdict table into Done criteria."
    status: pending
isProject: false
---

# Phase C: Author the 9 Missing Sample ITs

## Prerequisites

- [java-its-shallow-emulators_b8c9d0e1.plan.md](java-its-shallow-emulators_b8c9d0e1.plan.md) — Phase B must have landed: `gateway/handlers/datatransfer/` and `gateway/handlers/bqstorage/` must be serving the surfaces these new ITs exercise.

## Scope

Of the 24 BigQuery sample URLs originally targeted, 9 have a vendored snippet driver under `src/main/java/...` but no `*IT.java` in the upstream tree. Phase C authors a minimal smoke IT for each, plus the small `datatransfer` catalog extension required to make the 8 Create*Transfer ITs succeed.

The 9 missing-IT samples:

| Sample ID | Driver class | New IT to author |
|-----------|--------------|------------------|
| `bigquerydatatransfer-create-admanager-transfer` | [`CreateAdManagerTransfer.java`](../../third_party/java-bigquery-tests/java-docs-samples/bigquery/bigquerydatatransfer/snippets/src/main/java/com/example/bigquerydatatransfer/CreateAdManagerTransfer.java) | `CreateAdManagerTransferIT` |
| `bigquerydatatransfer-create-ads-transfer` | `CreateAdsTransfer.java` | `CreateAdsTransferIT` |
| `bigquerydatatransfer-create-campaignmanager-transfer` | `CreateCampaignmanagerTransfer.java` | `CreateCampaignmanagerTransferIT` |
| `bigquerydatatransfer-create-play-transfer` | `CreatePlayTransfer.java` | `CreatePlayTransferIT` |
| `bigquerydatatransfer-create-redshift-transfer` | `CreateRedshiftTransfer.java` | `CreateRedshiftTransferIT` |
| `bigquerydatatransfer-create-teradata-transfer` | `CreateTeradataTransfer.java` | `CreateTeradataTransferIT` |
| `bigquerydatatransfer-create-youtubechannel-transfer` | `CreateYoutubeChannelTransfer.java` | `CreateYoutubeChannelTransferIT` |
| `bigquerydatatransfer-create-youtubecontentowner-transfer` | `CreateYoutubeContentOwnerTransfer.java` | `CreateYoutubeContentOwnerTransferIT` |
| `bigquerystorage-arrow-quickstart` | [`StorageArrowSample.java`](../../third_party/java-bigquery-tests/java-bigquerystorage/samples/snippets/src/main/java/com/example/bigquerystorage/StorageArrowSample.java) | `StorageArrowSampleIT` |

## Work items

### 1. `register-missing-datasources`

Edit `gateway/handlers/datatransfer/catalog.go` (created in Phase B) and append `DataSourceCatalogEntry` rows for each of the 8 third-party transfer source IDs:

- `admanager_transfer` — Ad Manager.
- `google_ads` — Google Ads.
- `dcm_dt` — Campaign Manager (DoubleClick).
- `play` — Google Play.
- `redshift` — Amazon Redshift.
- `teradata` — Teradata.
- `youtube_channel` — YouTube Channel.
- `youtube_content_owner` — YouTube Content Owner.

Each entry needs only the metadata fields that `CreateTransferConfig` validates: `dataSourceId`, `displayName`, a minimal `parameters` list reflecting what the driver supplies (cross-reference each driver class for the exact `TransferConfig` params it builds). No execution path is required — these are non-runnable catalog stubs (analogous to go-googlesql's `dataSourceAmazonS3` stub in [api/datatransfer/handler.go](../../../go-googlesql/api/datatransfer/handler.go) line 39).

Add a unit test in `gateway/handlers/datatransfer/catalog_test.go` asserting that `DataSources.list` returns all 8 new entries alongside the existing `amazon_s3` + `scheduled_query`.

### 2. `author-9-its`

For each of the 9 ITs, model on the closest existing IT in the same module:

- The 8 transfer ITs follow [`CreateAmazonS3TransferIT.java`](../../third_party/java-bigquery-tests/java-docs-samples/bigquery/bigquerydatatransfer/snippets/src/test/java/com/example/bigquerydatatransfer/CreateAmazonS3TransferIT.java): JUnit 4, `@BeforeClass` creates a project-scoped dataset, the test calls the driver's static `runSample(...)` (or whichever entry point the driver exposes), then asserts `GetTransferConfig` returns the created config with the expected `dataSourceId`. Use Phase A's `BqDataTransferOpts` to build the client.
- `StorageArrowSampleIT` follows [`QuickstartArrowSampleIT.java`](../../third_party/java-bigquery-tests/java-bigquerystorage/samples/snippets/src/test/java/com/example/bigquerystorage/QuickstartArrowSampleIT.java): invokes `StorageArrowSample.main(args)` (after patching the driver in Phase A to route through `BqStorageOpts`), captures stdout, asserts the row-count line is present. Use a small fixture dataset seeded in `@BeforeClass`.

Place all 8 transfer ITs under `third_party/java-bigquery-tests/java-docs-samples/bigquery/bigquerydatatransfer/snippets/src/test/java/com/example/bigquerydatatransfer/` and the storage one under `third_party/java-bigquery-tests/java-bigquerystorage/samples/snippets/src/test/java/com/example/bigquerystorage/`.

Each IT must clean up after itself (`DeleteTransferConfig` in `@After` for transfers, dataset teardown for storage) so the suite is re-runnable.

### 3. `coverage-table-and-validate`

- Update [third_party/README.md](../../third_party/README.md) "Sample coverage" table: change the 9 rows in the user's target list from "src only" → "IT shipped" with the new class names linked.
- Update [java-bigquery/samples/EMULATOR.md](../../third_party/java-bigquery-tests/java-bigquery/samples/EMULATOR.md) and the bigquerydatatransfer / bigquerystorage `EMULATOR.md` siblings (if they exist; if not, create them) to reflect the now-complete 24/24 coverage.
- Re-run `task thirdparty:java-bigquery-tests` and paste the 24-IT verdict table into Done criteria below.

## Verification

```bash
go test ./gateway/handlers/datatransfer/...
task emulator:run-full
task thirdparty:java-bigquery-tests
```

## Done criteria

- `gateway/handlers/datatransfer/catalog.go` lists all 8 new `dataSourceId` rows; `catalog_test.go` asserts them.
- 9 new `*IT.java` files exist and run as part of Failsafe's `**/*IT.java` discovery.
- `task thirdparty:java-bigquery-tests` reports 24/24 PASS; verdict table captured here.
- `third_party/README.md` "Sample coverage" table marks every one of the 24 target samples as IT-shipped.

## Next plan(s)

- None — Phase C closes out the Java live-IT track. Follow-ups (full bqstorage Read path, scheduled_query execution, IAM end-to-end, etc.) belong in the broader [ROADMAP.md](../../ROADMAP.md) lanes.
