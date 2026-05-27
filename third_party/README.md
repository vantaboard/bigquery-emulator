# Third-party client sample test trees

This directory vendors the four BigQuery client-library sample suites that
[`go-googlesql`](https://github.com/vantaboard/go-googlesql) exposes as its
"third-party" conformance lane. They are kept here as a sibling to the
in-repo SQL-fixture conformance harness:

| Lane | Lives under | Drives | What it asserts |
|------|-------------|--------|-----------------|
| **YAML fixture conformance** | `conformance/` | `task conformance:*` | SQL semantics through this repo's purpose-built runner (`go run ./conformance/cmd/runner`). Both `memory` and `duckdb` profiles run by default; results pinned in YAML. |
| **Third-party client conformance** | `third_party/<lang>-bigquery-tests/` | `task thirdparty:*` | The published Google BigQuery client libraries (Go, Node.js, Python, BigQuery DataFrames) talk to the emulator's REST + gRPC surface end-to-end. Tests come from upstream sample/snippet repos. |

The two lanes are deliberately separate. A SQL-shape regression should fail
the fixture lane; a client-library-compat regression (header parsing, REST
field shapes, gRPC multiplexing, GCS load semantics) should fail the
third-party lane. Neither blocks the other in CI.

The `*-bigquery-tests/` trees are vendored manually — there is no automated
sync from upstream. Refresh them by re-running the same `rsync` recipe used
in the original import (caches under `__pycache__/`, `.nox/`, `.venv/`,
`node_modules/` are excluded by `.gitignore`).

`task thirdparty` (alias `task thirdparty:default`) runs all four suites in
order. See `taskfiles/thirdparty.yml` for the per-suite knobs.

## Bootstrapping the emulator + storage stack

The third-party suites all assume:

1. The BigQuery emulator gateway is reachable at the address pointed to by
   `BIGQUERY_EMULATOR_HOST`. In this repo that is the Go gateway
   (`gateway_main`) with the C++ engine subprocess attached:

   ```bash
   task emulator:build-engine:bazel   # build full GoogleSQL+DuckDB engine
   task emulator:run-full              # gateway + engine on :9050 / :9060
   ```

   `mise.toml` already exports `BIGQUERY_EMULATOR_HOST=localhost:9050` for any
   shell that activated direnv/mise. CI builds the binaries explicitly and
   exports the env vars per workflow step (see
   `.github/workflows/thirdparty-samples.yml`).

2. (Optional but recommended) `fake-gcs-server` is reachable at
   `STORAGE_EMULATOR_HOST` for the suites that load `gs://cloud-samples-data/...`
   objects:

   ```bash
   task testdata:fake-gcs-sync         # one-time mirror into testdata/fake-gcs-data/
   task thirdparty:fake-gcs-up         # `docker compose --profile thirdparty up`
   ```

   Without `STORAGE_EMULATOR_HOST` set, GCS-backed subtests **skip** rather
   than fail (see the per-suite skip matrices below).

`testdata/fake-gcs-data/` is gitignored: it is too large to commit and the
sync script populates it from the public `gs://cloud-samples-data/...`
bucket on demand (gcloud-or-stdlib HTTP fallback; see
`scripts/sync_fake_gcs_public_samples.sh`).

## Environment-variable contract

These match the upstream go-googlesql defaults so the same client wiring
(`bqopts.ClientOptions()`, `setClientEndpoint.js`, `bigframes.pandas.options`)
works against either emulator without code changes.

| Variable | Default for this repo | Purpose |
|----------|-----------------------|---------|
| `BIGQUERY_EMULATOR_HOST` | `localhost:9050` (mise.toml) | HTTP REST listener. `host:port` (no scheme) for the Go client; Node/`http://` is normalized in `lib/bigqueryEmulatorClientOptions.js`. |
| `BIGQUERY_STORAGE_GRPC_ENDPOINT` | unset (set to `127.0.0.1:9060` for gRPC tests) | BigQuery Storage Read/Write API + Reservation + Connection + Analytics Hub multiplex. Without it, Storage/Managed-Writer subtests skip. |
| `BIGQUERY_V2_GRPC_ENDPOINT` | falls back to `BIGQUERY_STORAGE_GRPC_ENDPOINT` | BigQuery v2 metadata gRPC (preview client). |
| `BIGQUERY_ANALYTICSHUB_GRPC_ENDPOINT` | falls back to `BIGQUERY_STORAGE_GRPC_ENDPOINT` | Analytics Hub gRPC. |
| `BIGQUERY_MIGRATION_EMULATOR_HOST` | falls back to `BIGQUERY_EMULATOR_HOST` | Migration v2alpha REST (workflow create/get/list). gRPC is not emulated. |
| `BIGQUERY_EMULATOR_CLIENT_API_REGION` | unset | Sends `X-BigQuery-Emulator-Api-Region` header so regional dataset rules apply when the real Host is loopback. |
| `STORAGE_EMULATOR_HOST` | `127.0.0.1:4443` (when fake-gcs is up) | fake-gcs-server JSON API. Go: `host:port`. Node: `http://...` (the Node task auto-prefixes). |
| `FAKE_GCS_PORT` | `4443` | Compose port mapping for `fake-gcs-server`. |
| `GOLANG_SAMPLES_PROJECT_ID` | `dev` | Project ID seen by the upstream Go samples. |
| `GOLANG_SAMPLES_E2E_TEST` | `1` | Enables system-test branches in the Go suite. |
| `GOOGLE_CLOUD_PROJECT` / `GCLOUD_PROJECT` | `dev` (aligned by Mocha `test/setup.js`) | Node + Python clients. |
| `EMULATOR_PROJECT_ID` | `dev` | Used by Node `lib/sampleProjectEnv.js` when the Google project vars are unset. |

## `golang-bigquery-tests`

In-tree BigQuery Go integration tests at [`golang-bigquery-tests`](golang-bigquery-tests)
(`bigquery/` snippets + `bqopts` emulator wiring). The module's `go.mod`
declares its own dependencies, isolated from this repo's main module.

```bash
cd third_party/golang-bigquery-tests && go test ./... -count=1
```

When `BIGQUERY_EMULATOR_HOST` is set, `cloud.google.com/go/bigquery.NewClient`
spreads `bqopts.ClientOptions()...`
(`option.WithEndpoint("http://"+host)` and
`option.WithoutAuthentication()`). For tests that exercise regional rules
under a loopback Host, set `BIGQUERY_EMULATOR_CLIENT_API_REGION` (e.g.
`us-east4`) and `bqopts` adds `X-BigQuery-Emulator-Api-Region` per request.
The same wiring drives the v2 preview REST/gRPC clients
(`bqopts.BigQueryV2GRPCClientOptions()`).

```bash
task thirdparty:golang-bigquery-tests
```

For `cloud.google.com/go/storage`, run `task thirdparty:fake-gcs-up` from
the repo root. Compose mounts `testdata/fake-gcs-data/` at `/data` so
fake-gcs preloads objects that mirror public `gs://` paths:

- `cloud-samples-data`: `bigquery/us-states/*`,
  `bigquery/sample-transactions/transactions.csv`,
  `vertex-ai/bigframe/df.csv`,
  `bigquery/ml/onnx/pipeline_rf.onnx`, Cymbal pets tutorial assets under
  `bigquery/tutorials/cymbal-pets/`
  (`images/`, `documents/`, `document_chunks/`,
  `tables/products/products_*.avro`).
- `cloud-training-demos`:
  `txtclass/export/exporter/1549825580/` (TensorFlow SavedModel export).
- `ibis-testing-libraries`: `lodash.min.js`.

To refresh fixtures from public buckets, run `task testdata:fake-gcs-sync`
(gcloud preferred, stdlib HTTP fallback). Set
`STORAGE_EMULATOR_HOST=127.0.0.1:4443` (or
`127.0.0.1:${FAKE_GCS_PORT}`). When both `BIGQUERY_EMULATOR_HOST` and
`STORAGE_EMULATOR_HOST` are set, GCS-backed snippet tests expect those
seeded objects (recreate the container after changing the volume:
`docker compose --profile thirdparty up -d --force-recreate fake-gcs-server`).

For local-only checks without fake-gcs, leave `STORAGE_EMULATOR_HOST`
unset so GCS-backed subtests skip cleanly.

### Emulator coverage matrix (skips you should expect)

When `BIGQUERY_EMULATOR_HOST` points at this emulator, tests **skip**
rather than fail when they require unsupported surfaces. The list below is
inherited from go-googlesql; deltas specific to this emulator's Phase 8
state are tracked in `ROADMAP.md` and `docs/REST_API.md`.

| Area | Behavior with `BIGQUERY_EMULATOR_HOST` set |
|------|---------------------------------------------|
| **BigQuery v2 preview gRPC** (`apiv2_client.NewClient`) | Runs when `BIGQUERY_V2_GRPC_ENDPOINT` or `BIGQUERY_STORAGE_GRPC_ENDPOINT` is set; uses `bqopts.BigQueryV2GRPCClientOptions()`. Otherwise skipped; REST subtests remain. |
| **Analytics Hub** | Set `BIGQUERY_ANALYTICSHUB_GRPC_ENDPOINT` to the gRPC `host:port` (same listener as Storage gRPC). Tests run against the in-memory Analytics Hub surface. |
| **BigQuery Connection** | Set `BIGQUERY_STORAGE_GRPC_ENDPOINT`. Metadata-only (no live federated data sources; IAM methods are unimplemented). |
| **BigQuery Migration** (v2alpha REST) | Partial: workflow create/get/list/delete/start on the emulator HTTP port. Use `bqopts.MigrationRESTClientOptions()`. gRPC `NewClient` is not emulated. |
| **Reservation** | When `BIGQUERY_STORAGE_GRPC_ENDPOINT` is set, Reservation gRPC is multiplexed on that listener. If only HTTP is set, the test skips. |
| **ManagedWriter / Storage Read** | Skipped unless `BIGQUERY_STORAGE_GRPC_ENDPOINT` is set; uses `bqopts.StorageGRPCClientOptions()`. |
| **GCS sample loads** (`gs://cloud-samples-data/...`) | Skipped when `BIGQUERY_EMULATOR_HOST` is set and `STORAGE_EMULATOR_HOST` is unset. When both are set, tests expect the compose-mounted fake-gcs objects. |
| **Legacy SQL** | This emulator rejects `useLegacySql=true` with HTTP 400 (see `docs/REST_API.md`); samples that rely on legacy syntax fail rather than skip. Adjust the sample set accordingly. |
| **CREATE MODEL / ML** + **routine DDL** | Skipped or fails (tracked under SQL gaps in `ROADMAP.md`). |
| **Public sample tables** (`bigquery-public-data.samples.shakespeare`, etc.) | The vendored go-googlesql snapshot under `testdata/bq-emulator/` (`-initial-data-dir`) seeds these. This emulator does not yet expose a `--initial-data-dir` flag on `gateway_main`; tests that need pre-seeded public catalog data will need to skip or be re-driven through fixture setup. |

### Emulator stderr (benign vs actionable)

- `dataset location "us-central1" does not match API endpoint location "us-east4"`
  — Expected while `snippets/client` runs: it intentionally creates a dataset
  in `us-central1` against a regional endpoint and asserts the create fails.
- `Not found: Table ...` — Usually a snippet test expecting a table that
  another subtest creates; treat as a follow-up if it reproduces outside
  parallel noise.

Without the env vars above, `go test` mostly **skips** system tests but
still **compiles** packages. CI's first-pass gate exercises that compile
path before opting into emulator-backed runs.

## `python-bigquery-tests`

In-tree Python BigQuery client and integration tests at
[`python-bigquery-tests`](python-bigquery-tests).

**Status: scaffold only.** This vendor mirrors the upstream layout
(`LICENSE`, `pyproject.toml`, `samples/`, `tests/data/`) but matches
go-googlesql's `*.py` ignore policy: the actual `.py` source is *not*
checked in here. `task thirdparty:python-bigquery-tests` therefore fails
fast at the `noxfile.py` precondition and asks you to populate the tree
before it tries to run nox.

When the noxfile + samples are present, the task runs:

```bash
task thirdparty:python-bigquery-tests
# ↑ wraps `nox -s ${PYTHON_SAMPLES_NOX_SESSION:-snippets_full} -p $PYTHON_SAMPLES_PYTHON`
```

The in-tree Python client reads `BIGQUERY_EMULATOR_HOST` directly in
`google.cloud.bigquery._helpers` (prepends `http://` for `host:port`).
Snippet tests under `samples/tests/` and `samples/snippets/` use
`samples/emulator_client.py` (`AnonymousCredentials`) when the emulator
is set.

| Nox session | What runs |
|-------------|-----------|
| `unit` | Mocked `tests/unit` (good first CI gate) |
| `snippets_docs` | `docs/snippets.py` only (fast emulator check) |
| `snippets_full` | `docs/snippets.py` + `samples/` + `samples/snippets/` (default) |
| `unit_noextras` | Minimal extras (many skips) |

`samples/` and `samples/snippets/` invocations use `-n0`: a single emulator
process is shared, so `-n auto` often stalls or deadlocks after parallel
errors.

## `node-bigquery-tests`

In-tree Node BigQuery doc samples and Mocha integration tests at
[`node-bigquery-tests`](node-bigquery-tests).

Helpers in `lib/bigqueryEmulatorClientOptions.js` normalize schemeless
`BIGQUERY_EMULATOR_HOST` to `http://`, optional
`X-BigQuery-Emulator-Api-Region`, and project id via
`lib/sampleProjectEnv.js` + `test/setup.js` (see
[`EMULATOR.md`](node-bigquery-tests/EMULATOR.md)).

```bash
task thirdparty:node-bigquery-tests   # npm install && npm test (full Mocha)
```

With emulator + fake-gcs: `BIGQUERY_EMULATOR_HOST`,
`STORAGE_EMULATOR_HOST` (`http://` for Node Storage v7), gRPC endpoints,
`GOLANG_SAMPLES_PROJECT_ID=dev`. The task auto-prefixes
`STORAGE_EMULATOR_HOST` with `http://` for Node and runs
`scripts/preflight_node_samples_gcs.sh` first so a stale fake-gcs body
(empty or truncated `us-states.csv`) fails fast instead of running the
full Mocha suite.

## `python-bigquery-dataframes-tests`

In-tree BigQuery DataFrames library and snippet tests at
[`python-bigquery-dataframes-tests`](python-bigquery-dataframes-tests).

Only `samples/snippets` (nox session `py`) is kept; the tree includes
`bigframes/` and `third_party/bigframes_vendored/` for editable
`pip install -e`. `samples/snippets/conftest.py` wires anonymous BigQuery
+ Storage clients, gRPC overrides from `BIGQUERY_STORAGE_GRPC_ENDPOINT`,
and an allowlist when `BIGQUERY_EMULATOR_HOST` is set.

```bash
task thirdparty:python-bigquery-dataframes-tests
# Narrow CI gate (single allowlisted snippet):
task thirdparty:python-bigquery-dataframes-snippet-gate
```

Python **3.10–3.13** (vendored bigframes pins do not support 3.14); set
`DATAFRAME_SAMPLES_PYTHON` if your default `python3` is 3.14. The task
falls back to `python3.12 → 3.13 → 3.11 → 3.10` on `PATH` automatically.
When `STORAGE_EMULATOR_HOST` is set, the preflight
`scripts/preflight_dataframe_samples_gcs.sh` confirms fake-gcs is
reachable before the conftest tries to create buckets.

## Inventory

`scripts/list_thirdparty_bigquery_samples.sh` enumerates the snippets
under each tree. Useful when triaging coverage or filing follow-ups.
