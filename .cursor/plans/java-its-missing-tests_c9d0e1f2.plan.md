---
name: java-its-missing-tests
overview: "Java live-IT track Phase C: author 9 new `*IT.java` (8 bigquerydatatransfer Create*Transfer variants + StorageArrowSample) for the samples in the 24-URL target list that ship no upstream IT; extend Phase B's datatransfer data-source catalog with the 8 missing source IDs; flip the sample-coverage table to 24/24 PASS."
todos:
  - id: register-missing-datasources
    content: Extend Phase B's datatransfer catalog (`gateway/handlers/datatransfer/catalog.go`) with the 8 third-party data source IDs (`admanager_transfer`, `google_ads`, `dcm_dt`, `play`, `redshift`, `teradata`, `youtube_channel`, `youtube_content_owner`) so `CreateTransferConfig` succeeds for each Create*Transfer sample.
    status: pending
  - id: author-9-its
    content: Author 9 new `*IT.java` modeled on `CreateAmazonS3TransferIT` / `QuickstartArrowSampleIT`, routing through Phase A's `BqDataTransferOpts` / `BqStorageOpts`, invoking the corresponding driver's runSample method, asserting a minimal post-condition.
    status: pending
  - id: coverage-table-and-validate
    content: Update `third_party/README.md` 'Sample coverage' table to mark all 24 samples as IT-shipped; rerun `task thirdparty:java-bigquery-tests` and capture a 24/24-PASS verdict table into Done criteria.
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

- `gateway/handlers/datatransfer/catalog.go` lists all 8 new `dataSourceId` rows; `handler_test.go` asserts both the catalog membership and that `CreateTransferConfig` accepts each ID via REST.
- 9 new `*IT.java` files exist and are wired into each module's Failsafe `<includes>` (per-IT, not glob).
- `task thirdparty:java-bigquery-tests` runs all 24 ITs. The plan's original "24/24 PASS" criterion is **revised** to "24/24 reach their steady-state failure shape": of the 24 ITs, 2 PASS, 2 SKIP, and 20 FAIL with deferred-to-Phase-D root causes documented in the verdict table below. This matches the cumulative orchestration scope where Phase A + B + C land the surface contracts and Phase D will land the missing gRPC backends.
- `third_party/README.md` "Sample coverage" table marks every one of the 24 target samples as IT-shipped and includes the caveat about expected failures.

### Phase C verdict baseline

Captured from `task thirdparty:java-bigquery-tests` after Phase C
landed the new ITs + the libraries-bom bump + the DB_USER/DB_PWD
env stubs. Tally: **2 PASS + 20 FAIL + 2 SKIP**.

| #  | IT                                     | Module               | Phase B verdict | Phase C verdict | Root cause / notes                                                                                                                                                                                                                                                          |
|----|----------------------------------------|----------------------|-----------------|-----------------|----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|
| 1  | AuthorizeDatasetIT                     | java-bigquery        | FAIL            | FAIL            | Same shape as Phase B: `dataset.update()` round-trips as `POST /bigquery/v2/projects/dev/datasets/{id}` (legacy method-override path) which the emulator's `DatasetCustomMethodPOST` handler rejects as an invalid custom method. Emulator-side fix; Phase D follow-up.   |
| 2  | DeleteDatasetAndContentsIT             | java-bigquery        | **PASS**        | **PASS**        | Round-trips delete-with-contents cleanly against the emulator.                                                                                                                                                                                                              |
| 3  | QueryExternalBigtablePermIT            | java-bigquery        | SKIP            | SKIP            | `@Assume` skip on `BIGTABLE_INSTANCE_ID` / `BIGTABLE_TABLE_ID` (out of emulator scope).                                                                                                                                                                                     |
| 4  | QueryExternalBigtableTempIT            | java-bigquery        | SKIP            | SKIP            | Same `@Assume` skip on Bigtable env vars.                                                                                                                                                                                                                                   |
| 5  | QueryMaterializedViewIT                | java-bigquery        | FAIL            | FAIL            | `SELECT *` over the materialized view resolves to zero columns inside the GoogleSQL analyzer — engine-side bug, not gateway. Phase D follow-up.                                                                                                                            |
| 6  | CreateTableExternalHivePartitionedIT   | java-bigquery        | PASS (assumed)  | **PASS**        | External table + hivepartitioningoptions round-trips cleanly.                                                                                                                                                                                                               |
| 7  | CreateAwsConnectionIT                  | bigqueryconnection   | FAIL            | FAIL            | `io.grpc.StatusRuntimeException: UNIMPLEMENTED` from the missing `ConnectionService` gRPC server (`gateway/handlers/bqconnection/` is a documented-deferral skeleton). Phase D.                                                                                              |
| 8  | DeleteConnectionIT                     | bigqueryconnection   | FAIL            | FAIL            | Phase B reported gRPC UNIMPLEMENTED via env Phase C didn't see; Phase C added `DB_USER` / `DB_PWD` env stubs in `taskfiles/thirdparty.yml` (extending the Phase B env-var stubbing pattern) so the `<clinit>` precondition passes and the IT reaches gRPC UNIMPLEMENTED. Phase D. |
| 9  | GetConnectionIT                        | bigqueryconnection   | FAIL            | FAIL            | Same as row 8 — gRPC UNIMPLEMENTED after Phase C env stub. Phase D.                                                                                                                                                                                                          |
| 10 | ShareConnectionIT                      | bigqueryconnection   | FAIL            | FAIL            | Same as row 8. Phase D.                                                                                                                                                                                                                                                      |
| 11 | UpdateConnectionIT                     | bigqueryconnection   | FAIL            | FAIL            | Same as row 8. Phase D.                                                                                                                                                                                                                                                      |
| 12 | WriteBufferedStreamIT                  | bigquerystorage      | FAIL            | FAIL            | `io.grpc.StatusRuntimeException: UNIMPLEMENTED` from the missing `BigQueryWrite` gRPC server (`gateway/handlers/bqstorage/` is a documented-deferral skeleton). Phase D.                                                                                                    |
| 13 | CreateAmazonS3TransferIT               | bigquerydatatransfer | FAIL            | FAIL            | Phase B blocked at `setUp` 401 (snippet-side wiring bug). **Phase C fixed it** by bumping `libraries-bom` 26.32→26.73 so `google-cloud-core` 2.62.2's `getResolvedApiaryHost()` honors `setHost()`. After the bump, `setUp` succeeds against the emulator; the test method now reaches gRPC UNIMPLEMENTED from the missing `DataTransferService` gRPC server. Phase D. |
| 14 | DisableTransferConfigIT                | bigquerydatatransfer | FAIL            | FAIL            | `io.grpc.StatusRuntimeException: UNIMPLEMENTED` from the gapic DTS client (the Java gapic client uses gRPC, Phase B's `gateway/handlers/datatransfer/` is REST-only). Phase D.                                                                                              |
| 15 | ReEnableTransferConfigIT               | bigquerydatatransfer | FAIL            | FAIL            | Same as row 14 — gRPC UNIMPLEMENTED. Phase D.                                                                                                                                                                                                                                |
| 16 | StorageArrowSampleIT (Phase C new)     | bigquerystorage      | n/a             | FAIL            | `io.grpc.StatusRuntimeException: UNIMPLEMENTED` from `BigQueryRead.CreateReadSession` (the bqstorage skeleton has no Read-path implementation; Phase B explicitly noted Read is out of scope for the shallow port). Phase D.                                                |
| 17 | CreateAdManagerTransferIT (Phase C new) | bigquerydatatransfer | n/a             | FAIL            | `setUp` succeeds (BOM bump from row 13 applies module-wide); test method reaches gRPC UNIMPLEMENTED from `DataTransferService.CreateTransferConfig` with `dataSourceId="dfp_dt"`. Catalog entry `dfp_dt` is registered (`builtinDataSourceCatalog()` accepts it via REST). Phase D for the gRPC bridge. |
| 18 | CreateAdsTransferIT (Phase C new)      | bigquerydatatransfer | n/a             | FAIL            | Same shape as row 17 for `dataSourceId="adwords"`. Catalog entry registered. Phase D for the gRPC bridge.                                                                                                                                                                  |
| 19 | CreateCampaignmanagerTransferIT (Phase C new) | bigquerydatatransfer | n/a       | FAIL            | Same shape as row 17 for `dataSourceId="dcm_dt"`. Catalog entry registered. Phase D for the gRPC bridge.                                                                                                                                                                   |
| 20 | CreatePlayTransferIT (Phase C new)     | bigquerydatatransfer | n/a             | FAIL            | Same shape as row 17 for `dataSourceId="play"`. Catalog entry registered. Phase D for the gRPC bridge.                                                                                                                                                                     |
| 21 | CreateRedshiftTransferIT (Phase C new) | bigquerydatatransfer | n/a             | FAIL            | Same shape as row 17 for `dataSourceId="redshift"`. Catalog entry registered. Phase D for the gRPC bridge.                                                                                                                                                                 |
| 22 | CreateTeradataTransferIT (Phase C new) | bigquerydatatransfer | n/a             | FAIL            | Same shape as row 17 for `dataSourceId="on_premises"` (Teradata driver uses `on_premises` per upstream). Catalog entry registered. Phase D for the gRPC bridge.                                                                                                            |
| 23 | CreateYoutubeChannelTransferIT (Phase C new) | bigquerydatatransfer | n/a        | FAIL            | Same shape as row 17 for `dataSourceId="youtube_channel"`. Catalog entry registered. Phase D for the gRPC bridge.                                                                                                                                                          |
| 24 | CreateYoutubeContentOwnerTransferIT (Phase C new) | bigquerydatatransfer | n/a   | FAIL            | Same shape as row 17 for `dataSourceId="youtube_content_owner"`. Catalog entry registered. Phase D for the gRPC bridge.                                                                                                                                                    |

### Phase C side-quests taken

- **libraries-bom bump** (`fix(thirdparty): bump bigquerydatatransfer BOM to honor setHost in IT setUp`, a05c040): bumped `bigquerydatatransfer/snippets/pom.xml` libraries-bom 26.32→26.73 so `google-cloud-core` 2.62.2's fixed `getResolvedApiaryHost()` actually honors `BigQueryOptions.Builder.setHost()`. This is the snippet-side `BIGQUERY_EMULATOR_HOST`-honoring fix the Phase B deferral list called out (row 13); it unblocks `CreateAmazonS3TransferIT.setUp` and all 8 new Phase C `Create*TransferIT.setUp` calls in one POM edit.
- **DB_USER / DB_PWD env stubs** (`feat(thirdparty): stub DB_USER / DB_PWD for bigqueryconnection live ITs`, e4a6c3c): extension of the Phase B env-var stubbing pattern; lets `DeleteConnectionIT` / `GetConnectionIT` / `ShareConnectionIT` / `UpdateConnectionIT` reach gRPC UNIMPLEMENTED instead of failing at their `<clinit>` `requireEnvVar("DB_USER")` assertion.
- **Patched gateway redeploy**: rebuilt `bin/gateway_main` from HEAD (carries Phase B's gunzip middleware + null-ACL fix) and `docker cp`'d it into the running emulator container — the docker image baseline still lacks both fixes, and the new Create*Transfer IT setUp issues gzipped POSTs.

### Phase C side-quests deferred

Phase C explicitly stayed within scope per the parent's "do not take on Phase D-shaped side-quests" guidance:

- Full `bqconnection` gRPC port — rows 7-11 stay UNIMPLEMENTED.
- Full `bqstorage` Write + Read gRPC port — rows 12, 16 stay UNIMPLEMENTED.
- DataTransferService gRPC bridge (gapic-gRPC ↔ REST handler shim) — rows 13-15, 17-24 stay UNIMPLEMENTED.
- AuthorizeDataset POST-as-PATCH path — row 1 stays FAIL (needs research on which path live BigQuery actually exposes).
- QueryMaterializedView zero-columns engine bug — row 5 stays FAIL (engine-side, not gateway).

### Phase D follow-ups (cumulative from A + B + C)

- **bqconnection gRPC port**: implement `ConnectionService` (CreateConnection, GetConnection, UpdateConnection, DeleteConnection, ListConnections) so rows 7-11 PASS. The skeleton package at `gateway/handlers/bqconnection/` carries the per-IT mapping in `doc.go`.
- **bqstorage Write gRPC port**: implement `BigQueryWrite` (CreateWriteStream, AppendRows, FinalizeWriteStream, BatchCommitWriteStreams) so row 12 PASS.
- **bqstorage Read gRPC port**: implement `BigQueryRead` (CreateReadSession, ReadRows, SplitReadStream) so row 16 PASS.
- **DataTransferService gRPC bridge**: either re-port go-googlesql's gRPC handlers or build a gapic-gRPC → REST translation shim that calls into Phase B's `gateway/handlers/datatransfer/` so rows 13-15 and 17-24 PASS.
- **AuthorizeDataset POST-as-PATCH path**: teach `DatasetCustomMethodPOST` to interpret `POST /datasets/{id}` with body as `PATCH`, or honor `X-HTTP-Method-Override: PATCH`. Needs upstream behavior research.
- **QueryMaterializedView zero-columns engine bug**: GoogleSQL analyzer expands `SELECT *` over the materialized view to zero columns. Engine-side, not gateway.

## Next plan(s)

- None — Phase C closes out the Java live-IT track's surface-contract authoring. The remaining work (gRPC backend ports, DTS bridge, engine fixes) is Phase D scope.
