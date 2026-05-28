---
name: java-its-task-conversion
overview: "Java live-IT track Phase A: flip `task thirdparty:java-bigquery-tests` from compile-only to live Failsafe ITs against the local emulator, with three BqOpts-style helpers and patched ITs/drivers; capture a per-IT pass/fail baseline."
todos:
  - id: task-and-pom-conversion
    content: "Convert taskfile + four snippet POMs: default goal `verify`, default `JAVA_BQ_SKIP_TESTS=false`, drop `-Dmaven.test.skip=true`, remove POM-level skip properties, bind maven-failsafe-plugin to integration-test/verify with `**/*IT.java`."
    status: pending
  - id: bqopts-siblings-and-snippet-drivers
    content: "Add BqStorageOpts / BqConnectionOpts / BqDataTransferOpts (mirror of BqOpts.java) under each snippets module; patch the bigqueryconnection / bigquerydatatransfer / bigquerystorage `src/main/java` drivers to route service-client construction through them."
    status: pending
  - id: it-patches-and-baseline
    content: "Patch the 15 existing `*IT.java` classes to construct service clients via the new opts; run `task thirdparty:java-bigquery-tests` end-to-end and record the per-IT pass/fail matrix into Done criteria below."
    status: pending
isProject: false
---

# Phase A: Java ITs Task Conversion

## Prerequisites

- None — this is the entry point of the Java live-IT track. The compile-only baseline it replaces is documented in [third_party/README.md](../../third_party/README.md) and [java-bigquery/samples/EMULATOR.md](../../third_party/java-bigquery-tests/java-bigquery/samples/EMULATOR.md).

## Scope

The 24 BigQuery sample IDs targeted by this track break down as: 15 ship a `*IT.java` upstream (covered here), 9 do not (covered in Phase C). Of those 15, only the 6 under `java-bigquery/samples/snippets` exercise surfaces the emulator implements today; the other 9 (5 bigqueryconnection + 3 bigquerydatatransfer + 1 bigquerystorage) will fail with `NOT_IMPLEMENTED`-shaped errors until Phase B lands the shallow backends. Phase A's job is to make those failures real and structured, not to fix them.

## Work items

### 1. `task-and-pom-conversion`

- Edit [taskfiles/thirdparty.yml](../../taskfiles/thirdparty.yml):
  - `JAVA_BQ_MAVEN_GOALS` default `verify` (was `package`).
  - `JAVA_BQ_SKIP_TESTS` default `false` (was `true`).
  - Drop the `mvn_test_flags="-DskipTests=true -Dmaven.test.skip=true"` branch.
  - Add child-env exports `BIGQUERY_EMULATOR_HOST=http://localhost:9050` and `BIGQUERY_STORAGE_GRPC_ENDPOINT=localhost:9060` (mirror the gateway + storage ports declared in [docker-compose.yml](../../docker-compose.yml)).
  - Rewrite the task `desc` and the `# Slim-path Java snippets lane` header comment block to drop "compile-only" framing.
- Patch the four snippet POMs:
  - [`java-bigquery/samples/snippets/pom.xml`](../../third_party/java-bigquery-tests/java-bigquery/samples/snippets/pom.xml)
  - [`java-bigquerystorage/samples/snippets/pom.xml`](../../third_party/java-bigquery-tests/java-bigquerystorage/samples/snippets/pom.xml)
  - [`java-docs-samples/bigquery/bigqueryconnection/snippets/pom.xml`](../../third_party/java-bigquery-tests/java-docs-samples/bigquery/bigqueryconnection/snippets/pom.xml)
  - [`java-docs-samples/bigquery/bigquerydatatransfer/snippets/pom.xml`](../../third_party/java-bigquery-tests/java-docs-samples/bigquery/bigquerydatatransfer/snippets/pom.xml)

  In each: delete the `<maven.test.skip>true</maven.test.skip>` and `<skipTests>true</skipTests>` properties; add `<plugin><artifactId>maven-failsafe-plugin</artifactId>...</plugin>` bound to the `integration-test` + `verify` goals with `<includes><include>**/*IT.java</include></includes>` and `<systemPropertyVariables>` forwarding `BIGQUERY_EMULATOR_HOST` / `BIGQUERY_STORAGE_GRPC_ENDPOINT`.

### 2. `bqopts-siblings-and-snippet-drivers`

Add three helper classes, each mirroring the existing [`BqOpts.java`](../../third_party/java-bigquery-tests/java-bigquery/samples/snippets/src/main/java/com/example/bigquery/BqOpts.java) (env-var-driven, falls through to ADC when unset):

- `com.example.bigquerystorage.BqStorageOpts` at `third_party/java-bigquery-tests/java-bigquerystorage/samples/snippets/src/main/java/com/example/bigquerystorage/BqStorageOpts.java` — overrides `BigQueryReadSettings` / `BigQueryWriteSettings` endpoint + credentials. Reads `BIGQUERY_STORAGE_GRPC_ENDPOINT` first, falls back to `BIGQUERY_EMULATOR_HOST`.
- `com.example.bigqueryconnection.BqConnectionOpts` under the bigqueryconnection snippets — overrides `ConnectionServiceSettings`.
- `com.example.bigquerydatatransfer.BqDataTransferOpts` under the bigquerydatatransfer snippets — overrides `DataTransferServiceSettings`.

Then patch every snippet driver under those three modules' `src/main/java/...` to construct its client through the new helper instead of `Foo.create()`. (The `java-bigquery` snippets already use `BqOpts`; the other three trees don't use any helper today.) This is what makes the snippet itself talk to the emulator, not just the test wrapper.

### 3. `it-patches-and-baseline`

Patch the 15 existing IT classes so the service client they construct goes through the new opts:

- `java-bigquery`: `AuthorizeDatasetIT`, `DeleteDatasetAndContentsIT`, `QueryExternalBigtablePermIT`, `QueryExternalBigtableTempIT`, `QueryMaterializedViewIT`, `CreateTableExternalHivePartitionedIT`.
- `bigqueryconnection`: `CreateAwsConnectionIT`, `DeleteConnectionIT`, `GetConnectionIT`, `ShareConnectionIT`, `UpdateConnectionIT`.
- `bigquerydatatransfer`: `CreateAmazonS3TransferIT`, `DisableTransferConfigIT`, `ReEnableTransferConfigIT`.
- `bigquerystorage`: `WriteBufferedStreamIT`.

Then run `task thirdparty:java-bigquery-tests` end-to-end and paste the resulting per-IT verdict table into the Done criteria below (replacing the placeholder). Expected shape: 6 `java-bigquery` ITs PASS, 9 others FAIL with the documented `NOT_IMPLEMENTED` shape.

Final tidy:

- [third_party/README.md](../../third_party/README.md) "Compile-only contract" section rewritten to "Live-IT contract"; list the three new BqOpts variants; document expected fail states per surface.
- [java-bigquery/samples/EMULATOR.md](../../third_party/java-bigquery-tests/java-bigquery/samples/EMULATOR.md) "Compile-only by default" subsection rewritten.
- [.github/workflows/thirdparty-samples.yml](../../.github/workflows/thirdparty-samples.yml) job `java-bigquery-tests-compile` renamed to `java-bigquery-tests-live` and made tolerant of the 9 expected-fail ITs (via `continue-on-error` or a `JAVA_BQ_ALLOW_FAILING_ITS` allowlist).

## Verification

```bash
task emulator:run-full
task thirdparty:java-bigquery-tests
```

Pipe Failsafe output into a per-IT verdict capture, e.g. `task thirdparty:java-bigquery-tests 2>&1 | rg '^\[INFO\] (Running|Tests run:)' > /tmp/javabq-its-baseline.txt`.

## Done criteria

- `task thirdparty:java-bigquery-tests` invokes Failsafe (no `Tests are skipped.` lines).
- All 15 ITs execute (Failsafe reports `Tests run:` for each).
- Per-IT verdict table captured below.
- `task thirdparty:java-bigquery-tests` exit status is non-zero, with a structured diagnostic that lists exactly the failing modules. The Phase A `JAVA_BQ_ALLOW_FAILING_ITS` allowlist is wired into [.github/workflows/thirdparty-samples.yml](../../.github/workflows/thirdparty-samples.yml) (`continue-on-error: true` for the live-IT job), so CI surfaces the verdict but does not block while Phase B is in flight.

### Phase A verdict baseline (captured 2026-05-27)

Emulator: pre-existing root-owned `docker-compose` engine container (REST :9050, storage gRPC :9060). Maven goals: `mvn -B verify -DskipTests=false -Dsurefire.skipTests=true -DskipITs=false` per module, with `BIGQUERY_EMULATOR_HOST=http://localhost:9050`, `BIGQUERY_STORAGE_GRPC_ENDPOINT=localhost:9060`, `GOOGLE_CLOUD_PROJECT=dev`, `BIGQUERY_DATASET_NAME=test_dataset`. Reports under each module's `target/failsafe-reports/`.

| # | IT                                  | Module                              | Verdict   | Failure root cause                                                                                                                                                                                                                                  |
| - | ----------------------------------- | ----------------------------------- | --------- | --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| 1 | AuthorizeDatasetIT                  | java-bigquery                       | FAIL      | NPE at `AuthorizeDataset.java:52` (`new ArrayList<>(dataset.getAcl())`). Emulator returns `null` for an empty ACL list where live BigQuery returns `[]`. Driver bug **or** emulator response-shape bug — needs Phase B follow-up to decide.         |
| 2 | DeleteDatasetAndContentsIT          | java-bigquery                       | FAIL      | `BigQueryException: Could not parse dataset request body as JSON: invalid character '\x1f' looking for beginning of value`. Java client gzips outgoing POST bodies; emulator REST gateway does not decode `Content-Encoding: gzip`. Emulator bug.   |
| 3 | QueryExternalBigtablePermIT         | java-bigquery                       | SKIP      | JUnit `@Assume` short-circuit (Bigtable fixture env vars missing — `BIGTABLE_INSTANCE_ID` / `BIGTABLE_TABLE_ID`). Clean skip, operationally equivalent to PASS for the live-IT lane.                                                                 |
| 4 | QueryExternalBigtableTempIT         | java-bigquery                       | SKIP      | Same as #3 — `@Assume` skip on Bigtable fixture env.                                                                                                                                                                                                |
| 5 | QueryMaterializedViewIT             | java-bigquery                       | FAIL      | Same gzip bug as #2 — fires on `POST /tables` (create base table), `POST /tables` (materialized view), and `POST /queries`. Emulator bug.                                                                                                           |
| 6 | CreateTableExternalHivePartitionedIT | java-bigquery                      | FAIL      | Same gzip bug as #2 on `POST /tables`. Emulator bug.                                                                                                                                                                                                |
| 7 | WriteBufferedStreamIT               | java-bigquerystorage                | FAIL      | Same gzip bug as #2 on `@Before` `POST /datasets` — fails before any Storage gRPC traffic. Emulator bug; once the REST gateway accepts gzip, the test will reach the Storage Write API and is expected to surface the **Phase B**-tracked NOT_IMPLEMENTED. |
| 8 | CreateAwsConnectionIT               | bigqueryconnection                  | FAIL      | `<clinit>:41`: `Environment variable AWS_ACCOUNT_ID is required`. NOT_IMPLEMENTED-shaped — requires an external AWS account; emulator can't satisfy. Phase B will surface NOT_IMPLEMENTED on the gRPC Connection API instead.                       |
| 9 | DeleteConnectionIT                  | bigqueryconnection                  | FAIL      | `<clinit>:42`: `Environment variable MY_SQL_DATABASE is required`. NOT_IMPLEMENTED-shaped — same shape as #8.                                                                                                                                       |
| 10 | GetConnectionIT                    | bigqueryconnection                  | FAIL      | Same shape as #9 — `MY_SQL_DATABASE` required.                                                                                                                                                                                                      |
| 11 | ShareConnectionIT                  | bigqueryconnection                  | FAIL      | Same shape as #9 — `MY_SQL_DATABASE` required.                                                                                                                                                                                                      |
| 12 | UpdateConnectionIT                 | bigqueryconnection                  | FAIL      | Same shape as #9 — `MY_SQL_DATABASE` required.                                                                                                                                                                                                      |
| 13 | CreateAmazonS3TransferIT           | bigquerydatatransfer                | FAIL      | `<clinit>:63`: `Environment variable AWS_ACCESS_KEY_ID is required`. NOT_IMPLEMENTED-shaped — needs external AWS creds.                                                                                                                             |
| 14 | DisableTransferConfigIT            | bigquerydatatransfer                | FAIL      | `<clinit>:42`: `Environment variable DTS_TRANSFER_CONFIG_NAME is required`. NOT_IMPLEMENTED-shaped — needs a pre-existing transfer config.                                                                                                          |
| 15 | ReEnableTransferConfigIT           | bigquerydatatransfer                | FAIL      | Same shape as #14 — `DTS_TRANSFER_CONFIG_NAME` required.                                                                                                                                                                                            |

Tally: **2 SKIP** (clean `@Assume`) + **13 FAIL**, where **8** are NOT_IMPLEMENTED-shaped (rows 8–15) and **5** are emulator-bug-shaped (rows 1, 2, 5, 6, 7 — gzip REST + null ACL).

Deviation from the plan's "6 PASS + 9 NOT_IMPLEMENTED-shaped FAIL" projection:

- **2 of the 6 expected-PASS slots are `@Assume` skips** (QueryExternalBigtable Perm/Temp) — these clear cleanly and are treated as PASS-equivalents for the live-IT lane.
- **4 of the 6 expected-PASS slots (AuthorizeDatasetIT, DeleteDatasetAndContentsIT, QueryMaterializedViewIT, CreateTableExternalHivePartitionedIT) and 1 of the 9 expected-NOT_IMPLEMENTED slots (WriteBufferedStreamIT)** fail because the emulator's REST gateway does not decode gzipped POST bodies (`Content-Encoding: gzip` → `invalid character '\x1f'`). This is a distinct emulator bug that blocks every live-IT path that creates a dataset/table/query via REST. Fix should land separately (gateway-side gzip decode, OR client-side `setDisableGZipContent(true)` shim baked into BqOpts) — flagged for Phase B intake.
- AuthorizeDatasetIT additionally surfaces an NPE inside the driver when the emulator returns `null` for an empty ACL list; once gzip is fixed, this becomes the actual failure mode.
- All **8** NOT_IMPLEMENTED-shaped failures fire at `<clinit>`/`@BeforeClass` because of missing external-resource env vars (AWS/MySQL/DTS), *before* reaching the emulator gRPC surface. Phase B's shallow backends will only see traffic from these ITs once those env vars are stubbed (or the ITs are rewritten to drop the precondition).

## Next plan(s)

- [java-its-shallow-emulators_b8c9d0e1.plan.md](java-its-shallow-emulators_b8c9d0e1.plan.md)
