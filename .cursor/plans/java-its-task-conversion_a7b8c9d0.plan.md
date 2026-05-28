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
- Per-IT verdict table captured in this section showing 6 PASS + 9 expected-FAIL (NOT_IMPLEMENTED-shaped).
- `task thirdparty:java-bigquery-tests` exit status is 0 under the agreed Phase A allowlist (the 9 surface-missing ITs are recorded as known-fail until Phase B); without the allowlist it's non-zero, with a structured diagnostic that lists exactly the 9 expected ITs.

## Next plan(s)

- [java-its-shallow-emulators_b8c9d0e1.plan.md](java-its-shallow-emulators_b8c9d0e1.plan.md)
