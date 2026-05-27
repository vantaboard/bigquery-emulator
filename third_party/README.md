# Third-party sources

Vendored client test trees are maintained directly in this repository. Update them manually when adding or refreshing integration coverage; there is no automated sync from external forks.

**`task thirdparty:default`** runs **`golang-bigquery-tests`**, **`python-bigquery-tests`**, **`node-bigquery-tests`**, and **`python-bigquery-dataframes-tests`** in sequence (see **`taskfiles/test/thirdparty.yml`**).

## `golang-bigquery-tests`

In-tree BigQuery Go integration tests at [`golang-bigquery-tests`](golang-bigquery-tests) (`bigquery/` snippets + `bqopts` emulator wiring). After large imports, re-check go-googlesql–specific paths: **`internal/testutil/`**, **`emulator/`**. Then:

```bash
cd third_party/golang-bigquery-tests && go mod tidy && go test ./... -count=1
```

### `go test` (emulator-aware clients)

The module adds `bqopts`: when **`BIGQUERY_EMULATOR_HOST`** is set (host:port, no scheme), **`cloud.google.com/go/bigquery`** `NewClient` spreads **`bqopts.ClientOptions()...`** (`option.WithEndpoint("http://"+host)` and **`option.WithoutAuthentication()`**). For tests that assert **regional** dataset rules while the real HTTP `Host` is loopback, set **`BIGQUERY_EMULATOR_CLIENT_API_REGION`** (e.g. `us-east4`); `bqopts` then sends **`X-BigQuery-Emulator-Api-Region`** on each request, which the emulator uses when deriving “API endpoint location” (see `api/apiregion`). The emulator process may instead set **`BIGQUERY_EMULATOR_ASSUMED_API_REGION`** for manual `curl` without that header. The **`cloud.google.com/go/bigquery/v2`** preview **REST** client (`apiv2_client.NewRESTClient`) uses the same HTTP options in `snippets_preview` tests. For **v2 gRPC** (`NewClient`) against the emulator, set **`BIGQUERY_V2_GRPC_ENDPOINT`** or **`BIGQUERY_STORAGE_GRPC_ENDPOINT`** and spread **`bqopts.BigQueryV2GRPCClientOptions()...`** (the emulator multiplexes BigQuery v2 metadata gRPC on the same gRPC listener as Storage). If **`BIGQUERY_EMULATOR_HOST`** is set but neither gRPC endpoint is set, those gRPC subtests **skip** while REST coverage still runs. With **`BIGQUERY_EMULATOR_HOST`** unset, behavior matches upstream (production and ADC).

Start the emulator (e.g. `task emulator:start`), export **`BIGQUERY_EMULATOR_HOST`** (HTTP BigQuery REST), **`BIGQUERY_MIGRATION_EMULATOR_HOST`** (optional; BigQuery Migration **v2alpha** REST on the same HTTP port—`bqopts.MigrationRESTClientOptions()` falls back to **`BIGQUERY_EMULATOR_HOST`** when unset), **`BIGQUERY_STORAGE_GRPC_ENDPOINT`** (gRPC: BigQuery **v2 metadata** transcoding, Storage read/write, **BigQuery Reservation**, and **Connection API**; same host and port as the emulator’s `-grpc-port`, e.g. **`127.0.0.1:9060`**), **`BIGQUERY_ANALYTICSHUB_GRPC_ENDPOINT`** (same gRPC `host:port` for the Analytics Hub quickstart when testing against this emulator), **`GOLANG_SAMPLES_PROJECT_ID`** (same as the emulator’s project id, default **`dev`**), and optionally **`GOLANG_SAMPLES_E2E_TEST`**, then:

```bash
task thirdparty:golang-bigquery-tests
```

For **`cloud.google.com/go/storage`**, run **`docker compose up -d fake-gcs-server`** from the repo root (see [`docker-compose.yaml`](../docker-compose.yaml)). Compose mounts **[`testdata/fake-gcs-data`](../testdata/fake-gcs-data)** at **`/data`** so fake-gcs preloads objects that mirror public **`gs://`** paths:

- **`cloud-samples-data`**: **`bigquery/us-states/*`**, **`bigquery/sample-transactions/transactions.csv`**, **`vertex-ai/bigframe/df.csv`**, **`bigquery/ml/onnx/pipeline_rf.onnx`**, Cymbal pets tutorial assets under **`bigquery/tutorials/cymbal-pets/`** (`images/`, `documents/`, `document_chunks/`, `tables/products/products_*.avro`).
- **`cloud-training-demos`**: **`txtclass/export/exporter/1549825580/`** (TensorFlow SavedModel export).
- **`ibis-testing-libraries`**: **`lodash.min.js`**.

To refresh fixtures from public buckets, run **`./scripts/sync_fake_gcs_public_samples.sh`** (uses **`gcloud storage cp`** when available, otherwise **`scripts/sync_fake_gcs_public_samples_http.py`**) or **`task testdata:fake-gcs-sync`**. Set **`STORAGE_EMULATOR_HOST=127.0.0.1:4443`** (or **`127.0.0.1:${FAKE_GCS_PORT}`**; [fake-gcs-server](https://github.com/fsouza/fake-gcs-server)). When both **`BIGQUERY_EMULATOR_HOST`** and **`STORAGE_EMULATOR_HOST`** are set, GCS-backed snippet tests expect those seeded objects (recreate the container after changing the volume: **`docker compose up -d --force-recreate fake-gcs-server`**).

After emulator + fake-gcs are up, run **`task thirdparty:golang-bigquery-tests`** (or **`task test:thirdparty:golang-bigquery-tests`**). For local-only checks without fake-gcs, **unset `STORAGE_EMULATOR_HOST`** so load jobs can reach public **`storage.googleapis.com`** instead (direnv sets **`STORAGE_EMULATOR_HOST`** by default—use a shell without that export if you use this path).

### Verification (golang-bigquery-tests + GCS)

1. **Emulator + fake-gcs** — **`docker compose up -d fake-gcs-server`** (volume loads [`testdata/fake-gcs-data`](../testdata/fake-gcs-data)), emulator running, **`BIGQUERY_EMULATOR_HOST`** and **`STORAGE_EMULATOR_HOST`** set as in [`.envrc`](../.envrc): **`task thirdparty:golang-bigquery-tests`** — GCS-backed **`snippets/loadingdata`** subtests and **`snippets/querying`** partitioned/clustered setup run against seeded objects.
2. **Without fake-gcs** — unset **`STORAGE_EMULATOR_HOST`** (or do not start fake-gcs): partitioned/clustered setup and GCS import subtests **skip** when **`BIGQUERY_EMULATOR_HOST`** is set and **`STORAGE_EMULATOR_HOST`** is unset, so loads do not 404 against an empty or missing bucket.

### Emulator coverage matrix (emulator skips)

When **`BIGQUERY_EMULATOR_HOST`** points at **go-googlesql**, tests **skip** (rather than failing) when they require unsupported surfaces:

| Area | Behavior with `BIGQUERY_EMULATOR_HOST` set |
|------|---------------------------------------------|
| **BigQuery v2 preview gRPC** (`apiv2_client.NewClient`) | Runs when **`BIGQUERY_V2_GRPC_ENDPOINT`** or **`BIGQUERY_STORAGE_GRPC_ENDPOINT`** is set (same gRPC port as Storage); uses **`bqopts.BigQueryV2GRPCClientOptions()`**. Otherwise skipped; REST subtests remain. |
| **Analytics Hub** (`emulator/`) | When `BIGQUERY_EMULATOR_HOST` is set, also set **`BIGQUERY_ANALYTICSHUB_GRPC_ENDPOINT`** to the emulator’s gRPC `host:port` (same listener as Storage gRPC). Tests run against the in-memory Analytics Hub surface. |
| **BigQuery Connection** (`emulator/`) | Set **`BIGQUERY_STORAGE_GRPC_ENDPOINT`** to the emulator gRPC `host:port` (same listener as Storage). Uses **`bqopts.ConnectionGRPCClientOptions()`** with **`connection.NewClient`**. Metadata-only (no live federated data sources; **GetIamPolicy** / **SetIamPolicy** / **TestIamPermissions** are unimplemented). |
| **BigQuery Migration** (`migration/apiv2alpha` REST) | Partial: workflow **create / get / list / delete / start** on the emulator HTTP port. Use **`bqopts.MigrationRESTClientOptions()`** (`BIGQUERY_MIGRATION_EMULATOR_HOST` or fallback **`BIGQUERY_EMULATOR_HOST`**). gRPC **`NewClient`** is not emulated. |
| **Reservation** (`emulator/`) | When `BIGQUERY_EMULATOR_HOST` is set, also set **`BIGQUERY_STORAGE_GRPC_ENDPOINT`** to the emulator’s gRPC `host:port` (Reservation gRPC is multiplexed on that listener). Uses **`bqopts.ReservationGRPCClientOptions()`**. If only HTTP is set, the test skips. |
| **ManagedWriter** (`snippets/managedwriter`) | Skipped unless **`BIGQUERY_STORAGE_GRPC_ENDPOINT`** is set to the emulator’s **gRPC** `host:port` (Storage Write/Read is served on gRPC, not the HTTP BigQuery REST port). When set, uses **`bqopts.StorageGRPCClientOptions()`**. |
| **BigQuery Storage read** (`emulator/`) | Same as ManagedWriter: skipped on emulator without **`BIGQUERY_STORAGE_GRPC_ENDPOINT`**; uses the same gRPC options when set. |
| **GCS sample loads** (`gs://cloud-samples-data/...` in **`snippets/querying`** setup) | Skipped when **`BIGQUERY_EMULATOR_HOST`** is set and **`STORAGE_EMULATOR_HOST`** is unset. When **both** are set, tests expect compose-mounted **`testdata/fake-gcs-data`** objects under **`cloud-samples-data`**. |
| **GCS-backed imports** (`snippets/loadingdata` subtests using **`gs://cloud-samples-data/...`**) | Same as querying: when **`BIGQUERY_EMULATOR_HOST`** and **`STORAGE_EMULATOR_HOST`** are set, those subtests run against fake-gcs. Local-file loads (**`importCSVFromFile`**, **`createTableAndWidenLoad`**, **`relaxTableImport`**) and **`importWithHivePartitioning`** (emulator hive stub) still run without GCS. |
| **Legacy SQL** sample queries | Partial: `useLegacySql=true` runs a documented translator subset (see `FEATURE_CHECKLIST.md`, Jobs section); samples that rely on unsupported Legacy-only syntax may still skip or fail with `legacy sql:` errors. |
| **CREATE MODEL / ML** and **routine DDL**-heavy snippets | Skipped (tracked as language/catalog gaps; see root `FEATURE_CHECKLIST.md`). |
| **Public sample tables** | With **`-initial-data-dir`** (default **`./testdata/bq-emulator`** via **`.envrc`** / **Taskfile**), the emulator copies a checked-in Pebble + Parquet snapshot on first use of **`data-dir`** (`storage/catalog_materialize.go`). That includes **`bigquery-public-data.samples.shakespeare`**, **`usa_names`**, **`stackoverflow.posts_questions`**, extended samples (**`utility_us`**, **`github_repos`**, **`google_analytics_sample`** shards, **`ml_datasets.penguins`**, etc.). Use a fresh **`data-dir`** or delete **`meta/`** to re-copy. |

### Emulator stderr / logs (benign vs actionable)

- **`dataset location "us-central1" does not match API endpoint location "us-east4"`** — Expected while **`snippets/client`** [`TestClients`](golang-bigquery-tests/snippets/client/integration_test.go) runs: it intentionally creates a dataset in **`us-central1`** against a regional endpoint **`us-east4`** and expects that create to **fail**. Align **`BIGQUERY_EMULATOR_CLIENT_API_REGION`** with your snippet’s dataset **`Location`** when you need a **successful** regional create (see the `bqopts` paragraph above).
- **`workflow not found`** — Often from **`snippets/migration`** ([`emulator_integration_test.go`](golang-bigquery-tests/snippets/migration/emulator_integration_test.go)) (create/list/get/delete timing); if it persists, run **`go test`** for the failing package with **`-count=1`** and inspect the test name in the emulator log line.
- **`Not found: Table ...`** — Usually a snippet test expecting a table that another subtest creates; treat as a **follow-up** if it reproduces outside parallel noise.

Remaining failures beyond this matrix are usually **SQL** gaps listed as unchecked in **`FEATURE_CHECKLIST.md`** (prefer skips/docs there).

Without the env vars above, `go test` mostly **skips** system tests but still **compiles** packages (as in CI).

## `python-bigquery-tests`

In-tree Python BigQuery client and integration tests at [`python-bigquery-tests`](python-bigquery-tests).

The in-tree client reads **`BIGQUERY_EMULATOR_HOST`** in `google.cloud.bigquery._helpers` (prepends `http://` when the env value is `host:port`). Snippet tests under **`samples/tests/`** and **`samples/snippets/`** use **`samples/emulator_client.py`** (`AnonymousCredentials`) when the emulator is set. **`samples/tests/conftest.py`** only provides fixtures; it does **not** skip whole modules when **`BIGQUERY_EMULATOR_HOST`** is set—public datasets, **`gs://`** URIs, and CMEK-oriented samples are meant to run against go-googlesql like the other third-party trees, using **`.envrc`** (**`STORAGE_EMULATOR_HOST`** / fake-gcs for Cloud Storage, **`BIGQUERY_STORAGE_GRPC_ENDPOINT`** for BigQuery Storage gRPC reads, **`EMULATOR_INITIAL_DATA_DIR`** / **`testdata/bq-emulator`**) as needed. **`Client._ensure_bqstorage_client`** routes Storage API traffic to that gRPC endpoint when the REST emulator env is set.

| Nox session | What runs |
|-------------|-----------|
| `unit` | Mocked **`tests/unit`** (CI **`thirdparty-samples.yml`** calls this directly) |
| `snippets_docs` | **`docs/snippets.py`** only (fast emulator check) |
| `snippets_full` | **`docs/snippets.py`** + top-level **`samples/`** + **`samples/snippets/`** (**default** for **`task thirdparty:python-bigquery-tests`**) |
| `unit_noextras` | Minimal extras (many skips) |

**`samples/`** and **`samples/snippets`** nox pytest invocations use **`-n0`**: a single go-googlesql process is shared, so **`-n auto`** often stalls or deadlocks after parallel errors.
```bash
# Default nox session is snippets_full; override e.g. PYTHON_SAMPLES_NOX_SESSION=unit
task thirdparty:python-bigquery-tests
```

With **`BIGQUERY_EMULATOR_HOST`** set, run **`task emulator:start`** first (preflight via **`scripts/preflight_python_samples_emulator.sh`**).

## `node-bigquery-tests`

In-tree Node BigQuery doc samples and Mocha integration tests at [`node-bigquery-tests`](node-bigquery-tests).

Helpers in **`lib/bigqueryEmulatorClientOptions.js`** normalize schemeless **`BIGQUERY_EMULATOR_HOST`** to **`http://`**, optional **`X-BigQuery-Emulator-Api-Region`**, and project id via **`lib/sampleProjectEnv.js`** + **`test/setup.js`** (see [`EMULATOR.md`](node-bigquery-tests/EMULATOR.md)).

```bash
task thirdparty:node-bigquery-tests   # npm install && npm test (full Mocha)
```

With emulator + fake-gcs (**.envrc** / **`.github/workflows/thirdparty-samples.yml`**): **`BIGQUERY_EMULATOR_HOST`**, **`STORAGE_EMULATOR_HOST`** (`http://` for Node Storage v7), gRPC endpoints, **`GOLANG_SAMPLES_PROJECT_ID=dev`**. Preflight: **`scripts/preflight_node_samples_gcs.sh`**.

## `python-bigquery-dataframes-tests`

In-tree BigQuery DataFrames library and snippet tests at [`python-bigquery-dataframes-tests`](python-bigquery-dataframes-tests).

Only **`samples/snippets`** (nox session **`py`**) is kept; the tree includes **`bigframes/`** and **`third_party/bigframes_vendored/`** for editable **`pip install -e`**. **`samples/snippets/conftest.py`** wires **anonymous** BigQuery / Storage clients, **gRPC** overrides from **`BIGQUERY_STORAGE_GRPC_ENDPOINT`**, and an **allowlist** when **`BIGQUERY_EMULATOR_HOST`** is set (the shared **`testdata/bq-emulator`** snapshot includes **`bigquery-public-data.ml_datasets.penguins`** when the emulator is started with **`-initial-data-dir`**). Override with **`DATAFRAME_SAMPLES_EMULATOR_ALLOWLIST`**; **CI** uses **`set_options_test.py`** only until BigFrames SQL parity grows.

```bash
task thirdparty:python-bigquery-dataframes-tests
```

Python **3.10–3.13** (vendored **bigframes==1.42.0** in **`samples/snippets/requirements.txt`**; not viable on **3.14**); set **`DATAFRAME_SAMPLES_PYTHON`** (see **`.envrc`**, default **3.12**). Fast gate for the eight indexed emulator tests: **`task test:thirdparty:python-bigquery-dataframes-snippet-gate`**. If **`python3`** is **3.14** and the env var is unset, the task tries **`python3.12`**, then **3.13**, **3.11**, **3.10** on **`PATH`**. Preflight: **`scripts/preflight_python_samples_emulator.sh`**, **`scripts/preflight_dataframe_samples_gcs.sh`** when **`STORAGE_EMULATOR_HOST`** is set.

**Emulator:** The go-googlesql emulator exports in [`.envrc`](../.envrc) (`BIGQUERY_EMULATOR_HOST`, gRPC endpoints, `STORAGE_EMULATOR_HOST`, **`EMULATOR_INITIAL_DATA_DIR`**) align with **golang-bigquery-tests** (`bqopts`). The vendored Python client in **`python-bigquery-tests`** reads **`BIGQUERY_EMULATOR_HOST`** for the REST API host (`google.cloud.bigquery._helpers`); **nox** unit tasks do **not** clear that variable (constructor unit tests expect the host from env when set). **`snippets_full`** runs **`docs/snippets.py`**, **`samples/tests`**, and **`samples/snippets`** against the emulator without a samples/tests skip list—failures indicate emulator or env gaps. Use **`task thirdparty:golang-bigquery-tests`** for live HTTP against the emulator from Go. Without **`BIGQUERY_EMULATOR_HOST`**, snippet tests that need ADC still use **`google.auth.default`**. **`python-bigquery-dataframes-tests`** (`samples/snippets`) applies the same REST host plus optional **gRPC** endpoint overrides on **`bigframes.pandas.options`**, expects **`ml_datasets.penguins`** in the emulator snapshot (**`testdata/bq-emulator`** / **`-initial-data-dir`**), and runs a **conftest allowlist** when **`BIGQUERY_EMULATOR_HOST`** is set (other modules stay skipped until BigFrames SQL parity grows). **Node** (**`node-bigquery-tests`**) normalize schemeless **`BIGQUERY_EMULATOR_HOST`** to **`http://`**, optional **`X-BigQuery-Emulator-Api-Region`**, and project id from env—see **`third_party/node-bigquery-tests/EMULATOR.md`**. See **`FEATURE_CHECKLIST.md` → Third-party sample clients**.

**Inventory:** `scripts/list_thirdparty_bigquery_samples.sh`.
