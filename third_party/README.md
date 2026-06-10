# Third-party client sample test trees

This directory vendors the five BigQuery client-library sample suites used
for the third-party client conformance lane in bigquery-emulator. They are
kept here as a sibling to the in-repo SQL-fixture conformance harness:

| Lane | Lives under | Drives | What it asserts |
|------|-------------|--------|-----------------|
| **YAML fixture conformance** | `conformance/` | `task conformance:*` | SQL semantics through this repo's purpose-built runner (`go run ./conformance/cmd/runner`). Both `memory` and `duckdb` profiles run by default; results pinned in YAML. |
| **Third-party client conformance** | `third_party/<lang>-bigquery-tests/` | `task thirdparty:*` | The published Google BigQuery client libraries (Go, Node.js, Python, Java, BigQuery DataFrames) and **dbt-bigquery** functional tests talk to the emulator's REST + gRPC surface end-to-end. Tests come from upstream sample/snippet repos (and dbt-labs/dbt-adapters for the dbt lane). The Java lane runs a curated 15-IT Failsafe set against the local emulator (6 java-bigquery ITs exercising surfaces this emulator implements; 9 across bigqueryconnection/bigquerydatatransfer/bigquerystorage that currently fail with `NOT_IMPLEMENTED`-shaped errors until shallow backends land — see `ROADMAP.md` and `taskfiles/thirdparty.yml`). The other lanes run live when `BIGQUERY_EMULATOR_HOST` is exported. |

The two lanes are deliberately separate. A SQL-shape regression should fail
the fixture lane; a client-library-compat regression (header parsing, REST
field shapes, gRPC multiplexing, GCS load semantics) should fail the
third-party lane. Neither blocks the other in CI.

The `*-bigquery-tests/` trees are vendored manually — there is no automated
sync from upstream. Refresh them by re-running the same `rsync` recipe used
in the original import (caches under `__pycache__/`, `.nox/`, `.venv/`,
`node_modules/` are excluded by `.gitignore`).

`task thirdparty` (alias `task thirdparty:default`) runs all five suites in
order. See `taskfiles/thirdparty.yml` for the per-suite knobs.

## Bootstrapping the emulator + storage stack

The third-party suites all assume:

1. The BigQuery emulator gateway is reachable at the address pointed to by
   `BIGQUERY_EMULATOR_HOST`. In this repo that is the Go gateway
   (`gateway_main`) with the C++ engine subprocess attached:

   ```bash
   task emulator:build-engine:bazel   # build engine (GoogleSQL analyzer
                                      # + local execution coordinator
                                      # + DuckDB storage)
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

These match the documented defaults so client wiring
(`bqopts.ClientOptions()`, `setClientEndpoint.js`, `bigframes.pandas.options`)
works against the emulator without code changes.

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
| `PUBSUB_EMULATOR_HOST` | `127.0.0.1:8085` (when `task compose:pubsub` is up) | Official `gcloud beta emulators pubsub`. Opt-in via the `thirdparty` compose profile; default `docker compose up` does not start it. |
| `PUBSUB_EMULATOR_PORT` | `8085` | Compose port mapping for `pubsub-emulator`. |
| `PUBSUB_PROJECT_ID` | `dev` | Project the Pub/Sub emulator initializes (matches `BIGQUERY_EMULATOR_PROJECT`). |
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
documented below; deltas specific to this emulator's conformance-harness
state are tracked in `ROADMAP.md` and `docs/REST_API.md`.

| Area | Behavior with `BIGQUERY_EMULATOR_HOST` set |
|------|---------------------------------------------|
| **BigQuery v2 preview gRPC** (`apiv2_client.NewClient`) | Runs when `BIGQUERY_V2_GRPC_ENDPOINT` or `BIGQUERY_STORAGE_GRPC_ENDPOINT` is set; uses `bqopts.BigQueryV2GRPCClientOptions()`. Otherwise skipped; REST subtests remain. |
| **Analytics Hub** | Set `BIGQUERY_ANALYTICSHUB_GRPC_ENDPOINT` to the gRPC `host:port` (same listener as Storage gRPC). Tests run against the in-memory Analytics Hub surface. |
| **BigQuery Connection** | Set `BIGQUERY_STORAGE_GRPC_ENDPOINT`. Metadata-only (no live federated data sources; IAM methods are unimplemented). |
| **BigQuery Migration** (v2alpha REST) | Partial: workflow create/get/list/delete/start on the emulator HTTP port. Use `bqopts.MigrationRESTClientOptions()`. gRPC `NewClient` is not emulated. |
| **Reservation** | When `BIGQUERY_STORAGE_GRPC_ENDPOINT` is set, Reservation gRPC is multiplexed on that listener. If only HTTP is set, the test skips. |
| **ManagedWriter / Storage Read** | Skipped unless `BIGQUERY_STORAGE_GRPC_ENDPOINT` is set; uses `bqopts.StorageGRPCClientOptions()`. `PendingStream` and `DefaultStream` subtests skip on the emulator (PENDING streams and full proto-type matrix not fully implemented). |
| **GCS sample loads** (`gs://cloud-samples-data/...`) | Skipped when `BIGQUERY_EMULATOR_HOST` is set and `STORAGE_EMULATOR_HOST` is unset. When both are set, tests expect the compose-mounted fake-gcs objects (including `bigquery/hive-partitioning-samples/customlayout/*`). |
| **Legacy SQL** | This emulator rejects `useLegacySql=true` with HTTP 400 (see `docs/REST_API.md`); samples that rely on legacy syntax fail rather than skip. Adjust the sample set accordingly. |
| **CREATE MODEL / ML** + **routine DDL** | Skipped via `bqtestutil.SkipEmulatorBQML()` in Go model/export samples (mirrors Python `emulator_pytest_skip.py` / Node `models.test.js` skip). |
| **Public sample tables** (`bigquery-public-data.samples.shakespeare`, etc.) | The Docker image and `gateway_main --seed-data-file` load minimal fixtures from [`testdata/public-data/bigquery-public-data.yaml`](../testdata/public-data/bigquery-public-data.yaml) (`usa_names`, `samples.shakespeare`, `utility_us.country_code_iso`, `stackoverflow.posts_questions`, etc.). Python skips samples that reference other public tables (`emulator_pytest_skip.py`); Node runs all Mocha files except `models.test.js` (BQML). |

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

**Status: scaffold + on-demand sync.** The committed tree mirrors the
upstream layout (`LICENSE`, `pyproject.toml`, `setup.cfg`, `uv.lock`,
`samples/`, `samples/snippets/`, `samples/tests/`, `tests/data/`,
`docs/`) but the actual `.py` source — including `noxfile.py` — is
*not* checked in per the ignore policy in this README. `task
thirdparty:python-bigquery-tests` fails fast at the `noxfile.py`
precondition and asks you to populate the tree first.

To populate (or refresh) the `.py` source, run the sync script from the
repo root:

```bash
./scripts/sync_python_bigquery_tests.sh                       # ref=main (default)
PYTHON_BIGQUERY_REF=v3.40.1 ./scripts/sync_python_bigquery_tests.sh   # pin to a tag
PYTHON_BIGQUERY_REF=e8184fa3856 ./scripts/sync_python_bigquery_tests.sh # pin to a SHA
./scripts/sync_python_bigquery_tests.sh --dry-run             # preview only
```

The script clones [`googleapis/python-bigquery`](https://github.com/googleapis/python-bigquery)
into a temp dir at `${PYTHON_BIGQUERY_REF:-main}`, rsyncs only `*.py`
files under `google/`, `samples/`, `tests/`, `docs/`, plus root-level
`noxfile.py` / `noxfile_config.py` / `conftest.py`, then prints the
resolved upstream commit SHA. It never overwrites the curated scaffold
(`pyproject.toml`, `setup.cfg`, `uv.lock`, `LICENSE`, `tests/data/`
fixtures, `samples/snippets/*.json` schemas), so refreshes stay
diff-bounded.

Once `noxfile.py` is present, the task runs:

```bash
task thirdparty:python-bigquery-tests
# ↑ wraps `nox -s ${PYTHON_SAMPLES_NOX_SESSION:-snippets} -p $PYTHON_SAMPLES_PYTHON \
#         -- -n0 -v --durations=10 $PYTHON_SAMPLES_PYTEST_ARGS`
# (the `-- -n0` posarg is appended automatically for the emulator-driven
# `snippets` session; it overrides upstream's hard-coded `-n=auto` so a
# single shared emulator process doesn't deadlock against pytest-xdist
# workers — pytest-xdist honors the last `-n` flag.)
#
# `-v --durations=10` is appended for every pytest-driven session
# (snippets / system / unit / unit_noextras / prerelease_deps) so each
# test's nodeid prints (and flushes, via PYTHONUNBUFFERED=1) *before*
# the test runs — a hung test then surfaces its nodeid in the live
# output instead of stalling on a single `.ss…` progress line. Append
# extra pytest flags via `PYTHON_SAMPLES_PYTEST_ARGS`; they go last so
# pytest's last-flag-wins rules let you override (e.g. `-q` silences
# per-test progress, `-vv` adds even more detail).
```

The in-tree Python client reads `BIGQUERY_EMULATOR_HOST` directly in
`google.cloud.bigquery._helpers._get_bigquery_host()`, which returns the
env var **verbatim** — it does not prepend `http://`. The Task wrapper
normalizes a bare `host:port` value to `http://host:port` before
launching nox (mirroring what `node-bigquery-tests` does for
`STORAGE_EMULATOR_HOST`), so the env mutation only affects the child
nox process and the surrounding `task thirdparty:default` chain still
sees the bare form their Go/Node clients expect. Snippet tests under
`samples/tests/` use `AnonymousCredentials` when the emulator host is
set.

When `BIGQUERY_EMULATOR_HOST` is set, the task also loads the
repo-owned pytest plugin
[`emulator_pytest_skip.py`](python-bigquery-tests/emulator_pytest_skip.py)
(`-p emulator_pytest_skip`) which skips BQML, geography, legacy SQL,
and public-data samples whose tables are not in the bundled seed file.
Override or narrow via `PYTHON_SAMPLES_PYTEST_ARGS` (appended last).

The default session is `snippets` (upstream-defined; emulator-driven).
Override via the Task CLI: `task thirdparty:python-bigquery-tests
PYTHON_SAMPLES_NOX_SESSION=unit`. Sessions defined by upstream's
`noxfile.py`:

| Nox session | What runs | Hits emulator? |
|-------------|-----------|----------------|
| `snippets` | `docs/snippets.py` + top-level `samples/` (excludes per-subdir noxfile dirs: `samples/snippets`, `samples/desktopapp`, `samples/magics`, `samples/geography`, `samples/notebooks`). **Default.** | Yes |
| `system` | `tests/system/` integration suite | Yes |
| `prerelease_deps` | Same coverage as `snippets`/`system` against pre-release dependency pins | Yes |
| `unit` | Mocked `tests/unit/` with all extras (`pytest -n=8`) | No |
| `unit_noextras` | Mocked `tests/unit/`, minimal extras (many subtests skip) | No |
| `mypy` / `mypy_samples` / `pytype` | Type-check the library and samples | No |
| `cover` | Coverage report (run after `unit`) | No |
| `lint` / `lint_setup_py` / `blacken` | Style/lint gates | No |
| `docs` / `docfx` | Build sphinx + docfx outputs (Python 3.10) | No |

The per-subdir noxfiles under `samples/<subdir>/noxfile.py` (`snippets`,
`desktopapp`, `magics`, `geography`, `notebooks`) define their own `py`
session and are intentionally **not** invoked by the top-level
`snippets` session. Run them explicitly when you need that coverage:

```bash
cd third_party/python-bigquery-tests/samples/snippets && \
    NOX_DEFAULT_VENV_BACKEND=virtualenv uvx --from 'nox[pbs]' nox -s py -- -n0
```

**Python version:** the `snippets` / `system` / `prerelease_deps`
sessions only accept Python **3.9 / 3.11-3.13** (upstream's
`SYSTEM_TEST_PYTHON_VERSIONS`); the project's `mise.toml` pins
`python = "3.14.x"`. The task auto-falls back: it first looks for a
`python3.13 → 3.12 → 3.11 → 3.9` binary on `PATH`, then if none is
present picks `3.13` and lets `uvx --from 'nox[pbs]' nox -p 3.13` fetch
the interpreter via python-build-standalone (the project's pinned `uv`
in `mise.toml` makes this work without any extra setup). Set
`PYTHON_SAMPLES_PYTHON=3.12` (or similar) to pin a specific
interpreter and skip the fallback. The `unit` / `unit_noextras`
sessions accept 3.9-3.14 directly, so 3.14 needs no fallback.

**Backend:** the task defaults `NOX_DEFAULT_VENV_BACKEND=virtualenv`
because upstream's `noxfile.py` invokes `python -m pip` in every
session and uv-created venvs ship without pip. Set
`NOX_DEFAULT_VENV_BACKEND='uv|virtualenv'` to opt back into uv when you
have a custom noxfile that doesn't need pip.

**Known env interaction (unit/unit_noextras only):** the upstream
`unit` session does not clear `BIGQUERY_EMULATOR_HOST`; two constructor
tests (`test_ctor_defaults`, `test_ctor_w_empty_client_options`) assert
the default endpoint is `https://bigquery.googleapis.com` and fail when
that env var is set. `mise.toml` and `.envrc` export it for the
emulator-driven sessions, so the failures are expected when you run
`unit` from an env that has it set; unset it (`env -u
BIGQUERY_EMULATOR_HOST task thirdparty:python-bigquery-tests
PYTHON_SAMPLES_NOX_SESSION=unit`) to get a clean unit run.

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

## `java-bigquery-tests`

Vendored from **two** upstreams under `third_party/java-bigquery-tests/`:

| Subtree | Upstream | Snippet module | Sample IDs covered |
|---------|----------|----------------|--------------------|
| `java-bigquery/samples/` | [`googleapis/google-cloud-java`](https://github.com/googleapis/google-cloud-java) HEAD `java-bigquery/samples/` | `java-bigquery/samples/snippets` | `bigquery-*` (core BigQuery REST samples) |
| `java-bigquerystorage/samples/` | [`googleapis/google-cloud-java`](https://github.com/googleapis/google-cloud-java) HEAD `java-bigquerystorage/samples/` | `java-bigquerystorage/samples/snippets` | `bigquerystorage-*` (Storage Read/Write API + Arrow) |
| `java-docs-samples/bigquery/bigqueryconnection/` | [`GoogleCloudPlatform/java-docs-samples`](https://github.com/GoogleCloudPlatform/java-docs-samples) HEAD `bigquery/bigqueryconnection/snippets/` | `java-docs-samples/bigquery/bigqueryconnection/snippets` | `bigqueryconnection-*` |
| `java-docs-samples/bigquery/bigquerydatatransfer/` | [`GoogleCloudPlatform/java-docs-samples`](https://github.com/GoogleCloudPlatform/java-docs-samples) HEAD `bigquery/bigquerydatatransfer/snippets/` | `java-docs-samples/bigquery/bigquerydatatransfer/snippets` | `bigquerydatatransfer-*` |

**Slim path:** every sample resolves its `google-cloud-*` dependency from
[`libraries-bom`](https://github.com/GoogleCloudPlatform/cloud-opensource-java/wiki/The-Google-Cloud-Platform-Libraries-BOM)
on Maven Central (BOM version pinned per upstream — currently 26.73.0 for
`java-bigquery/samples/snippets`, 26.70.0 for the storage tree, and
26.32.0 for the two `java-docs-samples` modules). This tree does **not**
vendor any Java client jars or patched sources — that heavier path was
explored and dropped once the maintenance cost became clear.

```bash
task thirdparty:java-bigquery-tests   # mvn -B verify on every snippets/ POM (Failsafe ITs against local emulator)
```

The task fans out across `JAVA_BQ_SAMPLE_PATHS`, `cd`-ing into each
listed snippets module (no parent reactor, no `-am`). Sibling modules
shipped under the upstream `samples/` parents (`install-without-bom`,
`snapshot`) are kept verbatim but not built — the `snapshot` POMs still
pin a deliberate `*-SNAPSHOT` BigQuery client version for upstream's
pre-release smoke and would only resolve from upstream's internal
artifact host.

### Live-IT contract

Each upstream snippets module ships a full `src/test/java/**/*IT.java`
suite alongside the sample sources (≈190 Failsafe ITs across all four
modules), every one of which expects ADC + a real backend by default.
This repo curates a 15-IT subset — the ones whose underlying sample IDs
the Java live-IT track targets — and binds maven-failsafe-plugin to the
`integration-test` + `verify` goals with a `<includes>` allowlist of
exactly those classes. Per-module BqOpts variants route the snippet
drivers + IT-side BigQuery client construction at the local emulator:

- `com.example.bigquery.BqOpts` — `google-cloud-bigquery` (REST gateway
  on `BIGQUERY_EMULATOR_HOST`).
- [`com.example.bigquerystorage.BqStorageOpts`](java-bigquery-tests/java-bigquerystorage/samples/snippets/src/main/java/com/example/bigquerystorage/BqStorageOpts.java)
  — `BigQueryReadSettings` / `BigQueryWriteSettings` (storage gRPC on
  `BIGQUERY_STORAGE_GRPC_ENDPOINT`, falling back to
  `BIGQUERY_EMULATOR_HOST`).
- [`com.example.bigqueryconnection.BqConnectionOpts`](java-bigquery-tests/java-docs-samples/bigquery/bigqueryconnection/snippets/src/main/java/com/example/bigqueryconnection/BqConnectionOpts.java)
  — `ConnectionServiceSettings` (same gRPC endpoint).
- [`com.example.bigquerydatatransfer.BqDataTransferOpts`](java-bigquery-tests/java-docs-samples/bigquery/bigquerydatatransfer/snippets/src/main/java/com/example/bigquerydatatransfer/BqDataTransferOpts.java)
  — `DataTransferServiceSettings` (same gRPC endpoint).

Expected fail states per surface (initial baseline; see `ROADMAP.md`):

| Surface | Expectation |
|---------|-------------|
| `java-bigquery` (6 ITs) | PASS — REST gateway implements these surfaces today (with the caveat that some ITs require additional env vars / pre-seeded datasets; see the per-IT verdict table below). |
| `bigqueryconnection` (5 ITs) | FAIL with `NOT_IMPLEMENTED`-shaped errors until shallow ConnectionService backend lands (see `ROADMAP.md`). |
| `bigquerydatatransfer` (11 ITs) | FAIL with `NOT_IMPLEMENTED`-shaped errors until the shallow-emulators work lands the shallow DataTransferService backend. |
| `bigquerystorage` (2 ITs: `WriteBufferedStreamIT`, `StorageArrowSampleIT`) | FAIL with `NOT_IMPLEMENTED`-shaped errors until the shallow-emulators work lands BigQuery Write/Read API parity. |

Setting `JAVA_BQ_SKIP_TESTS=true` reverts to the legacy compile-only
posture (`-DskipTests=true -Dmaven.test.skip=true`); useful when you
just want the libraries-bom drift check.

The [`thirdparty-samples`](../.github/workflows/thirdparty-samples.yml)
workflow (triggered by `build-engine` via `workflow_run`) now gates:

| Job | Task | Notes |
|-----|------|-------|
| `java-bigquery-tests-live` | `task thirdparty:java-bigquery-tests` | Temurin 17; honest gate via `scripts/java_bq_ci_gate.sh` |
| `python-bigquery-tests-live` | `task thirdparty:python-bigquery-tests` | nox `snippets`; fake-gcs seeded in CI |
| `node-bigquery-tests-live` | `task thirdparty:node-bigquery-tests` | Mocha; fake-gcs seeded in CI |
| `golang-bigquery-tests-live` | `task thirdparty:golang-bigquery-tests` | `go test ./...`; fake-gcs seeded in CI |
| `python-bigquery-dataframes-snippet-gate-live` | `task thirdparty:python-bigquery-dataframes-snippet-gate` | 4 allowlisted smokes |

Shared bring-up lives in
[`.github/actions/setup-thirdparty-docker-emulator`](../.github/actions/setup-thirdparty-docker-emulator/action.yml)
(engine artifact + `ENGINE_SOURCE=prebuilt` image + `task testdata:fake-gcs-sync`).

Until shallow gRPC backends land, the Java job tolerates expected-fail ITs via
`JAVA_BQ_ALLOW_FAILING_ITS` (comma-separated Failsafe class names). The same
allowlist is wired in [`taskfiles/thirdparty.yml`](../taskfiles/thirdparty.yml).
`QueryMaterializedViewIT` is **not** allowlisted and passes against the
stabilized engine (2026-06-08).

**Local bar (2026-06-08):** node **green** (107 passing); python **1** failure
(`test_query_script` / `SET`); golang **1** failure (public-data timestamp
cast); bigframes gate **3/4**; java **OK** with allowlist.

### Emulator wiring

The published `google-cloud-bigquery` does **not** auto-read
`BIGQUERY_EMULATOR_HOST` (unlike the Go client, which the Go suite drives
through `bqopts`). Sample drivers that want to talk to this emulator
must route through the helper at
[`BqOpts.java`](java-bigquery-tests/java-bigquery/samples/snippets/src/main/java/com/example/bigquery/BqOpts.java):

```java
import com.example.bigquery.BqOpts;
import com.google.cloud.bigquery.BigQuery;

BigQuery bq = BqOpts.builder().build().getService();
```

`BqOpts.builder()` reads `BIGQUERY_EMULATOR_HOST` (normalises schemeless
`host:port` to `http://`), forces `NoCredentials` when an emulator host
is present, and resolves the project id from
`GOOGLE_CLOUD_PROJECT` / `GCLOUD_PROJECT` /
`GOLANG_SAMPLES_PROJECT_ID`. Without those env vars it falls through to
`BigQueryOptions.newBuilder()` which uses ADC (live BigQuery).

### Toolchain

JDK 17+ (Temurin 17 in CI). The task prefers `JAVA_HOME` and falls back
to `mise x -- mvn` when unset (run `mise install` to activate the
project's Java per `mise.toml`). Maven artifacts under
`third_party/java-bigquery-tests/**/target/` are gitignored; running
`task thirdparty:java-bigquery-tests` populates a local `~/.m2/repository`
mirror of the libraries-bom on first run.

| Knob | Default | Purpose |
|------|---------|---------|
| `JAVA_BQ_MAVEN_GOALS` | `package` | Maven goals run on every snippets module in the fan-out. Override to `compile`, `verify`, etc. |
| `JAVA_BQ_SKIP_TESTS` | `false` | When `true`, reverts to compile-only (`-DskipTests=true -Dmaven.test.skip=true`). |
| `JAVA_BQ_ALLOW_FAILING_ITS` | see `taskfiles/thirdparty.yml` | Comma-separated Failsafe IT simple class names whose failure is expected until gRPC shallow backends land. Empty/unset disables allowlisting (local runs fail on any IT failure). |
| `JAVA_BQ_SAMPLE_PATHS` | the four paths in the upstream table above (whitespace-separated) | Snippet module paths relative to `third_party/java-bigquery-tests/`. Override to narrow the fan-out (e.g. `JAVA_BQ_SAMPLE_PATHS=java-bigquery/samples/snippets` while iterating on a single tree). |

### Sample coverage

Quick lookup for `https://docs.cloud.google.com/bigquery/docs/samples/<id>`
sample IDs against the vendored class and its IT (when an IT is shipped).
Use this when triaging "is sample X available?" or "is sample X exercised
end-to-end?"; rerun the table when adding more.

After the missing-tests follow-up of the Java live-IT track (see `ROADMAP.md`)
all 24 target samples ship a Failsafe IT. The current verdict is
**2 PASS + 20 FAIL + 2 SKIP**: the 2 PASS rows
(`DeleteDatasetAndContentsIT`, `CreateTableExternalHivePartitionedIT`)
round-trip cleanly against the emulator, the 2 SKIP rows
(`QueryExternalBigtable{Perm,Temp}IT`) `@Assume` Bigtable env vars
that the emulator does not provide, and the 20 FAIL rows surface
deferred-to-gRPC-server-follow-ups root causes:

- 18 hit gRPC `UNIMPLEMENTED` because the bqconnection /
  `BigQueryRead` / `BigQueryWrite` / `DataTransferService` gRPC
  servers are not yet wired (the shallow-emulators work landed REST
  handlers for datatransfer, but the Java gapic clients use gRPC).
- `AuthorizeDatasetIT` fails on the bare `POST /datasets/{id}` legacy
  PATCH path the emulator currently rejects.
- `QueryMaterializedViewIT` previously failed when REST `tables.insert` omitted
  schema on materialized views (gateway dropped `materializedView` and registered
  zero columns, so `SELECT *` expanded to nothing). The gateway now dry-runs the
  MV query to infer schema on insert; rebuild the compose image to pick up the fix.

See the Java IT verdict tables below and [`docs/REST_API.md`](../docs/REST_API.md)
for the per-surface gRPC/REST status and queued follow-ups.

| Sample ID | Vendored class | IT shipped |
|-----------|----------------|------------|
| `bigquery-authorized-dataset` | [`AuthorizeDataset.java`](java-bigquery-tests/java-bigquery/samples/snippets/src/main/java/com/example/bigquery/AuthorizeDataset.java) | [`AuthorizeDatasetIT.java`](java-bigquery-tests/java-bigquery/samples/snippets/src/test/java/com/example/bigquery/AuthorizeDatasetIT.java) |
| `bigquery-delete-dataset-and-contents` | [`DeleteDatasetAndContents.java`](java-bigquery-tests/java-bigquery/samples/snippets/src/main/java/com/example/bigquery/DeleteDatasetAndContents.java) | [`DeleteDatasetAndContentsIT.java`](java-bigquery-tests/java-bigquery/samples/snippets/src/test/java/com/example/bigquery/DeleteDatasetAndContentsIT.java) |
| `bigquery-query-external-bigtable-perm` | [`QueryExternalBigtablePerm.java`](java-bigquery-tests/java-bigquery/samples/snippets/src/main/java/com/example/bigquery/QueryExternalBigtablePerm.java) | [`QueryExternalBigtablePermIT.java`](java-bigquery-tests/java-bigquery/samples/snippets/src/test/java/com/example/bigquery/QueryExternalBigtablePermIT.java) |
| `bigquery-query-external-bigtable-temp` | [`QueryExternalBigtableTemp.java`](java-bigquery-tests/java-bigquery/samples/snippets/src/main/java/com/example/bigquery/QueryExternalBigtableTemp.java) | [`QueryExternalBigtableTempIT.java`](java-bigquery-tests/java-bigquery/samples/snippets/src/test/java/com/example/bigquery/QueryExternalBigtableTempIT.java) |
| `bigquery-query-materialized-view` | [`QueryMaterializedView.java`](java-bigquery-tests/java-bigquery/samples/snippets/src/main/java/com/example/bigquery/QueryMaterializedView.java) | [`QueryMaterializedViewIT.java`](java-bigquery-tests/java-bigquery/samples/snippets/src/test/java/com/example/bigquery/QueryMaterializedViewIT.java) |
| `bigquery-set-hivepartitioningoptions` | `[START bigquery_set_hivepartitioningoptions]` region inside [`CreateTableExternalHivePartitioned.java`](java-bigquery-tests/java-bigquery/samples/snippets/src/main/java/com/example/bigquery/CreateTableExternalHivePartitioned.java) | [`CreateTableExternalHivePartitionedIT.java`](java-bigquery-tests/java-bigquery/samples/snippets/src/test/java/com/example/bigquery/CreateTableExternalHivePartitionedIT.java) |
| `bigqueryconnection-create-aws-connection` | [`CreateAwsConnection.java`](java-bigquery-tests/java-docs-samples/bigquery/bigqueryconnection/snippets/src/main/java/com/example/bigqueryconnection/CreateAwsConnection.java) | [`CreateAwsConnectionIT.java`](java-bigquery-tests/java-docs-samples/bigquery/bigqueryconnection/snippets/src/test/java/com/example/bigqueryconnection/CreateAwsConnectionIT.java) |
| `bigqueryconnection-delete-connection` | [`DeleteConnection.java`](java-bigquery-tests/java-docs-samples/bigquery/bigqueryconnection/snippets/src/main/java/com/example/bigqueryconnection/DeleteConnection.java) | [`DeleteConnectionIT.java`](java-bigquery-tests/java-docs-samples/bigquery/bigqueryconnection/snippets/src/test/java/com/example/bigqueryconnection/DeleteConnectionIT.java) |
| `bigqueryconnection-get-connection` | [`GetConnection.java`](java-bigquery-tests/java-docs-samples/bigquery/bigqueryconnection/snippets/src/main/java/com/example/bigqueryconnection/GetConnection.java) | [`GetConnectionIT.java`](java-bigquery-tests/java-docs-samples/bigquery/bigqueryconnection/snippets/src/test/java/com/example/bigqueryconnection/GetConnectionIT.java) |
| `bigqueryconnection-share-connection` | [`ShareConnection.java`](java-bigquery-tests/java-docs-samples/bigquery/bigqueryconnection/snippets/src/main/java/com/example/bigqueryconnection/ShareConnection.java) | [`ShareConnectionIT.java`](java-bigquery-tests/java-docs-samples/bigquery/bigqueryconnection/snippets/src/test/java/com/example/bigqueryconnection/ShareConnectionIT.java) |
| `bigqueryconnection-update-connection` | [`UpdateConnection.java`](java-bigquery-tests/java-docs-samples/bigquery/bigqueryconnection/snippets/src/main/java/com/example/bigqueryconnection/UpdateConnection.java) | [`UpdateConnectionIT.java`](java-bigquery-tests/java-docs-samples/bigquery/bigqueryconnection/snippets/src/test/java/com/example/bigqueryconnection/UpdateConnectionIT.java) |
| `bigquerydatatransfer-create-admanager-transfer` | [`CreateAdManagerTransfer.java`](java-bigquery-tests/java-docs-samples/bigquery/bigquerydatatransfer/snippets/src/main/java/com/example/bigquerydatatransfer/CreateAdManagerTransfer.java) | [`CreateAdManagerTransferIT.java`](java-bigquery-tests/java-docs-samples/bigquery/bigquerydatatransfer/snippets/src/test/java/com/example/bigquerydatatransfer/CreateAdManagerTransferIT.java) |
| `bigquerydatatransfer-create-ads-transfer` | [`CreateAdsTransfer.java`](java-bigquery-tests/java-docs-samples/bigquery/bigquerydatatransfer/snippets/src/main/java/com/example/bigquerydatatransfer/CreateAdsTransfer.java) | [`CreateAdsTransferIT.java`](java-bigquery-tests/java-docs-samples/bigquery/bigquerydatatransfer/snippets/src/test/java/com/example/bigquerydatatransfer/CreateAdsTransferIT.java) |
| `bigquerydatatransfer-create-amazons3-transfer` | [`CreateAmazonS3Transfer.java`](java-bigquery-tests/java-docs-samples/bigquery/bigquerydatatransfer/snippets/src/main/java/com/example/bigquerydatatransfer/CreateAmazonS3Transfer.java) | [`CreateAmazonS3TransferIT.java`](java-bigquery-tests/java-docs-samples/bigquery/bigquerydatatransfer/snippets/src/test/java/com/example/bigquerydatatransfer/CreateAmazonS3TransferIT.java) |
| `bigquerydatatransfer-create-campaignmanager-transfer` | [`CreateCampaignmanagerTransfer.java`](java-bigquery-tests/java-docs-samples/bigquery/bigquerydatatransfer/snippets/src/main/java/com/example/bigquerydatatransfer/CreateCampaignmanagerTransfer.java) | [`CreateCampaignmanagerTransferIT.java`](java-bigquery-tests/java-docs-samples/bigquery/bigquerydatatransfer/snippets/src/test/java/com/example/bigquerydatatransfer/CreateCampaignmanagerTransferIT.java) |
| `bigquerydatatransfer-create-play-transfer` | [`CreatePlayTransfer.java`](java-bigquery-tests/java-docs-samples/bigquery/bigquerydatatransfer/snippets/src/main/java/com/example/bigquerydatatransfer/CreatePlayTransfer.java) | [`CreatePlayTransferIT.java`](java-bigquery-tests/java-docs-samples/bigquery/bigquerydatatransfer/snippets/src/test/java/com/example/bigquerydatatransfer/CreatePlayTransferIT.java) |
| `bigquerydatatransfer-create-redshift-transfer` | [`CreateRedshiftTransfer.java`](java-bigquery-tests/java-docs-samples/bigquery/bigquerydatatransfer/snippets/src/main/java/com/example/bigquerydatatransfer/CreateRedshiftTransfer.java) | [`CreateRedshiftTransferIT.java`](java-bigquery-tests/java-docs-samples/bigquery/bigquerydatatransfer/snippets/src/test/java/com/example/bigquerydatatransfer/CreateRedshiftTransferIT.java) |
| `bigquerydatatransfer-create-teradata-transfer` | [`CreateTeradataTransfer.java`](java-bigquery-tests/java-docs-samples/bigquery/bigquerydatatransfer/snippets/src/main/java/com/example/bigquerydatatransfer/CreateTeradataTransfer.java) | [`CreateTeradataTransferIT.java`](java-bigquery-tests/java-docs-samples/bigquery/bigquerydatatransfer/snippets/src/test/java/com/example/bigquerydatatransfer/CreateTeradataTransferIT.java) |
| `bigquerydatatransfer-create-youtubechannel-transfer` | [`CreateYoutubeChannelTransfer.java`](java-bigquery-tests/java-docs-samples/bigquery/bigquerydatatransfer/snippets/src/main/java/com/example/bigquerydatatransfer/CreateYoutubeChannelTransfer.java) | [`CreateYoutubeChannelTransferIT.java`](java-bigquery-tests/java-docs-samples/bigquery/bigquerydatatransfer/snippets/src/test/java/com/example/bigquerydatatransfer/CreateYoutubeChannelTransferIT.java) |
| `bigquerydatatransfer-create-youtubecontentowner-transfer` | [`CreateYoutubeContentOwnerTransfer.java`](java-bigquery-tests/java-docs-samples/bigquery/bigquerydatatransfer/snippets/src/main/java/com/example/bigquerydatatransfer/CreateYoutubeContentOwnerTransfer.java) | [`CreateYoutubeContentOwnerTransferIT.java`](java-bigquery-tests/java-docs-samples/bigquery/bigquerydatatransfer/snippets/src/test/java/com/example/bigquerydatatransfer/CreateYoutubeContentOwnerTransferIT.java) |
| `bigquerydatatransfer-disable-transfer` | [`DisableTransferConfig.java`](java-bigquery-tests/java-docs-samples/bigquery/bigquerydatatransfer/snippets/src/main/java/com/example/bigquerydatatransfer/DisableTransferConfig.java) | [`DisableTransferConfigIT.java`](java-bigquery-tests/java-docs-samples/bigquery/bigquerydatatransfer/snippets/src/test/java/com/example/bigquerydatatransfer/DisableTransferConfigIT.java) |
| `bigquerydatatransfer-reenable-transfer` | [`ReEnableTransferConfig.java`](java-bigquery-tests/java-docs-samples/bigquery/bigquerydatatransfer/snippets/src/main/java/com/example/bigquerydatatransfer/ReEnableTransferConfig.java) | [`ReEnableTransferConfigIT.java`](java-bigquery-tests/java-docs-samples/bigquery/bigquerydatatransfer/snippets/src/test/java/com/example/bigquerydatatransfer/ReEnableTransferConfigIT.java) |
| `bigquerystorage-arrow-quickstart` | [`StorageArrowSample.java`](java-bigquery-tests/java-bigquerystorage/samples/snippets/src/main/java/com/example/bigquerystorage/StorageArrowSample.java) | [`StorageArrowSampleIT.java`](java-bigquery-tests/java-bigquerystorage/samples/snippets/src/test/java/com/example/bigquerystorage/StorageArrowSampleIT.java) |
| `bigquerystorage-jsonstreamwriter-buffered` | [`WriteBufferedStream.java`](java-bigquery-tests/java-bigquerystorage/samples/snippets/src/main/java/com/example/bigquerystorage/WriteBufferedStream.java) | [`WriteBufferedStreamIT.java`](java-bigquery-tests/java-bigquerystorage/samples/snippets/src/test/java/com/example/bigquerystorage/WriteBufferedStreamIT.java) |

## `python-bigquery-dataframes-tests`

In-tree BigQuery DataFrames library and snippet tests at
[`python-bigquery-dataframes-tests`](python-bigquery-dataframes-tests).

**Status: scaffold + on-demand sync.** The committed tree mirrors the
upstream layout (`LICENSE`, `README.rst`, stub `pyproject.toml`,
`samples/snippets/{noxfile.py,noxfile_config.py,requirements.txt}`,
`bigframes/` directory shells, `third_party/bigframes_vendored/`
license/metadata stubs) but the actual `.py` source — including
`samples/snippets/*_test.py` and the `bigframes/` package itself — is
*not* checked in, matching the sibling `python-bigquery-tests` policy.
`task thirdparty:python-bigquery-dataframes-tests` fails fast at the
`samples/snippets/*_test.py` precondition and asks you to populate the
tree first; without the sync, the noxfile's discovery glob would
silently print "No tests found, skipping directory." and exit 0.

To populate (or refresh) the `.py` source, run the sync script from the
repo root:

```bash
./scripts/sync_python_bigquery_dataframes_tests.sh                          # ref=main (default)
DATAFRAME_BIGQUERY_REF=v1.42.0 ./scripts/sync_python_bigquery_dataframes_tests.sh   # pin to a tag
DATAFRAME_BIGQUERY_REF=abcd1234 ./scripts/sync_python_bigquery_dataframes_tests.sh  # pin to a SHA
./scripts/sync_python_bigquery_dataframes_tests.sh --dry-run                # preview only
```

The script clones [`googleapis/python-bigquery-dataframes`](https://github.com/googleapis/python-bigquery-dataframes)
into a temp dir at `${DATAFRAME_BIGQUERY_REF:-main}`, rsyncs only `*.py`
files under `bigframes/`, `samples/`, `tests/`, and
`third_party/bigframes_vendored/`, plus root-level `noxfile.py` /
`noxfile_config.py` / `conftest.py` / `setup.py`, then prints the
resolved upstream commit SHA. It also pulls `samples/*/requirements*.txt`
and `testing/constraints-*.txt` (the latter only when present). It
never overwrites the curated scaffold (stub `pyproject.toml`, `LICENSE`,
`README.rst`, vendored license/metadata under
`third_party/bigframes_vendored/`), so refreshes stay diff-bounded.
Synced `*.py` files are committed to the repo (same policy as the
sibling `python-bigquery-tests` tree); `.gitignore` only covers
`__pycache__/`, `.nox/`, `.venv/`, and similar regenerable caches.

`setup.py` is part of the top-level allowlist because the committed
scaffold ships only a 3-line stub `pyproject.toml`. With upstream's
`setup.py` present, `INSTALL_LIBRARY_FROM_SOURCE=True` (set by the
Task target) makes the noxfile's `_get_repo_root()` walker stop at the
scaffold root and `pip install -e .` resolves to bigframes-from-source
instead of walking up to `bigquery-emulator/.git`.

Once the snippets are present, the task runs:

```bash
task thirdparty:python-bigquery-dataframes-tests          # full 32-snippet tree
task thirdparty:python-bigquery-dataframes-snippet-gate   # default thirdparty gate (4 non-ML smokes)
```

`task thirdparty` / `task thirdparty:default` invokes
**`python-bigquery-dataframes-snippet-gate`**, not the full suite. The
gate runs only:

- `test_bigquery_dataframes_set_options`
- `test_bigquery_dataframes_load_data_from_bigquery`
- `test_bigquery_dataframes_pandas_methods`
- `test_performance_optimizations`

**Skipped by default (BQML / Gemini / out of scope):** after sync, the
tree ships 15+ ML-oriented snippet tests (`*_model_test.py`, `bqml_*`,
`gemini_*`, `imported_*`, `multimodal_*`, forecasting, clustering, etc.)
that require `CREATE MODEL`, remote ONNX/TensorFlow imports, or Gemini
APIs this emulator does not implement (`docs/ENGINE_POLICY.md`). Run the
full `python-bigquery-dataframes-tests` task locally when triaging ML
parity; do not expect those snippets to pass against the emulator.

Only `samples/snippets` (nox session `py`) is invoked; the post-sync
tree includes `bigframes/` and `third_party/bigframes_vendored/` for
editable `pip install -e`. `samples/snippets/conftest.py` wires
anonymous BigQuery + Storage clients, gRPC overrides from
`BIGQUERY_STORAGE_GRPC_ENDPOINT`, and an allowlist when
`BIGQUERY_EMULATOR_HOST` is set.

Python **3.10–3.13** (vendored bigframes pins do not support 3.14); set
`DATAFRAME_SAMPLES_PYTHON` if your default `python3` is 3.14. The task
falls back to `python3.12 → 3.13 → 3.11 → 3.10` on `PATH` automatically.
When `STORAGE_EMULATOR_HOST` is set, the preflight
`scripts/preflight_dataframe_samples_gcs.sh` confirms fake-gcs is
reachable before the conftest tries to create buckets.

## `dbt-bigquery-tests`

In-tree **dbt-bigquery functional pytest** lane at
[`dbt-bigquery-tests`](dbt-bigquery-tests).

**Status: scaffold + on-demand sync; manual lane only (not in `task thirdparty`
aggregator or CI).** The committed tree carries emulator wiring
(`conftest.py`, `emulator_bootstrap.py`, `emulator_pytest_skip.py`,
`profiles/emulator/profiles.yml`, `requirements-test.txt`, `upstream_ref.txt`,
`FEASIBILITY.md`) and functional tests after sync.
`task thirdparty:dbt-bigquery-tests` fails fast when
`tests/functional/adapter/test_basic.py` is missing.

**Pass bar (2026-06-08):** `task thirdparty:dbt-bigquery-tests`
currently exits during pytest **collection** (2 errors in
`test_generic_catalog.py` / `test_relation_type_change.py` import paths).
Narrow triage: `DBT_BIGQUERY_RUN_TRIAGE=1` with a single-class
`DBT_BIGQUERY_PYTEST_ARGS` selector (see below). Full functional green
remains open — track with dbt skip-matrix refinement and engine
materialization parity.

Populate (or refresh) upstream functional tests:

```bash
./scripts/sync_dbt_bigquery_tests.sh
# or from a local clone:
DBT_ADAPTERS_LOCAL=/path/to/dbt-adapters ./scripts/sync_dbt_bigquery_tests.sh
DBT_ADAPTERS_REF=a82c766c ./scripts/sync_dbt_bigquery_tests.sh --dry-run
```

The sync script rsyncs `dbt-bigquery/tests/**/*.py` (never overwrites curated
`conftest.py`) plus small fixtures (`*.csv`, `*.ndjson`, `*.yml`, …) and
prints the resolved upstream SHA. Runtime packages install from
`requirements-test.txt` (git pin at `upstream_ref.txt`).

Run against the emulator:

```bash
task emulator:run-full                  # or task thirdparty:emulator-up
task thirdparty:dbt-bigquery-tests
```

Feasibility / first-wave triage (attempts `TestSimpleMaterializationsBigQuery`
despite the default skip sketch):

```bash
DBT_BIGQUERY_RUN_TRIAGE=1 \
  DBT_BIGQUERY_PYTEST_ARGS='tests/functional/adapter/test_basic.py::TestSimpleMaterializationsBigQuery::test_base -n0' \
  task thirdparty:dbt-bigquery-tests
```

### Emulator wiring

| Piece | Role |
|-------|------|
| `api_endpoint` | Set from `BIGQUERY_EMULATOR_HOST` (normalized to `http://host:port`) in `conftest.py` `--profile=emulator` |
| `emulator_bootstrap.py` | Patches `create_google_credentials` → `AnonymousCredentials` when the emulator host is set |
| `emulator_pytest_skip.py` | Initial skip matrix for Dataproc/python models, GCS uploads, MV/catalog tests, etc. |

### Skip matrix (initial sketch — refine at triage)

| Area | Emulator posture |
|------|-------------------|
| **Dataproc / python models** | Skipped (`python_model`, `dataproc` paths) |
| **GCS `upload_file`** | Skipped unless `STORAGE_EMULATOR_HOST` + fake-gcs wiring lands in this lane |
| **INFORMATION_SCHEMA / docs generate** | Skipped (`catalog`, `TestDocsGenerateBigQuery`) |
| **Materialized views** | Skipped (`simple_bigquery_view`, `materialized` paths) |
| **JS / SQL UDAF functions** | Skipped (engine gaps; JS UDFs and external-language UDFs are unsupported per `docs/ENGINE_POLICY.md`) |
| **Policy tags / grants / change history** | Skipped (deferred REST surfaces) |
| **BaseSimpleMaterializations** | Skipped by default; set `DBT_BIGQUERY_RUN_TRIAGE=1` for feasibility |

See [`FEASIBILITY.md`](dbt-bigquery-tests/FEASIBILITY.md) for the client-level
`api_endpoint` probe results (list/create dataset, query job — all OK on
`:9050`).

## bigquery-utils UDF conformance (non-gating)

Battle-tested BigQuery SQL UDF tests from
[GoogleCloudPlatform/bigquery-utils](https://github.com/GoogleCloudPlatform/bigquery-utils)
(Apache-2.0), converted to native YAML conformance fixtures. This is a
**third conformance lane** alongside the in-repo `conformance/fixtures/`
gate and the client-library `task thirdparty:*` suites.

| Piece | Location / command |
|-------|-------------------|
| Generated fixtures | `conformance/thirdparty-fixtures/bigquery_utils/{passing,known_failing}/` |
| Sync from upstream | `task conformance:bqutils-sync` (or `./scripts/sync_bigquery_utils_udfs.sh`) |
| Run passing set | `task conformance:bqutils` (requires `./bin/emulator_main`) |
| Triage after engine fixes | `./scripts/triage_bqutils_fixtures.sh` |

**Codegen contract:** `scripts/sync_bigquery_utils_udfs.sh` clones or
reads `BIGQUERY_UTILS_LOCAL`, runs `scripts/bigquery_utils/extract_test_cases.js`
(pure-SQL scalar UDFs only; skips JS, UDAF, Dataform-templated bodies),
and `go run ./conformance/cmd/genbqutils` to emit one YAML per UDF under
`known_failing/`. Use `BIGQUERY_UTILS_REF` to pin upstream (default
`master`). The sync prints the resolved upstream SHA.

**Gating policy:** `task conformance:bqutils` runs in CI after
`build-engine` succeeds (see `.github/workflows/conformance.yml`,
job `bqutils`). It is separate from the default `task conformance:run`
matrix so engine gaps in `known_failing/` do not block unrelated PRs.
`known_failing/` holds fixtures the engine does not pass yet.

**Pinned upstream ref:** `0754ad891dea3afe769f9fd5e537a7ea3c8b3a3b`
(set `BIGQUERY_UTILS_REF` for `task conformance:bqutils-sync`). Refresh:
sync at new ref → `./scripts/triage_bqutils_fixtures.sh` →
`task conformance:bqutils`.

**Current state (upstream `0754ad891dea`):** codegen emits pure-SQL
scalar UDF fixtures only (JS, Dataform-templated, and UDAF bodies are
skipped at extraction). After engine triage and promotion, **116
fixtures live in `passing/`** — spanning scalar UDFs, ANY TYPE
templated UDFs, SQL UDAFs, RANGE<>, BIGNUMERIC/interval,
regexp/string, bytes/bitwise families, plus hand-authored
`passing/views/` and `passing/stored_procedures/` README goldens —
and **1 in `known_failing/`**: `community/cw_xml_extract.yaml`
(**LANGUAGE python**, the documented external-language gap; same
disposition as JS UDFs in `docs/ENGINE_POLICY.md`). Run
`task conformance:bqutils` for the live pass/fail bar. First-party
`tvf_simple.yaml` and `call_with_declare_out.yaml` cover TVF and
gateway→engine scripting separately.

**Stored procedures:** gateway routes `DECLARE`/`CALL`/`BEGIN` scripts in
one engine round-trip; `linear_regression` / `bh_multiple_tests` use
simplified procedure bodies that still match README outputs (upstream uses
`EXECUTE IMMEDIATE` / full BH SQL). `chi_square` is an inline stats query
with nested aggregate+window routed to `semantic_executor`.

## Inventory

`scripts/list_thirdparty_bigquery_samples.sh` enumerates the snippets
under each tree. Useful when triaging coverage or filing follow-ups.
