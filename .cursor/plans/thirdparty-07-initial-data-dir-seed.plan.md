---
name: bigquery-public-data initial-data-dir seed
overview: "Port the go-googlesql `testdata/bq-emulator/` initial-data-dir pattern into this repo so the emulator boots with pre-seeded `bigquery-public-data.{samples.shakespeare, usa_names.usa_1910_2013, usa_names.usa_1910_current}` tables. Removes the three `bigquery-public-data` deselects from python-bigquery-tests."
todos:
  - id: tp07_seed_build
    content: "Build the gateway once with `-tags=seed_production_live` so the seed orchestrator can pull from production."
    status: pending
  - id: tp07_snapshot
    content: "Run the emulator against a clean --data-dir; POST /api/emulator/seed three times (one per table) or once with a dataset-scope source for samples; copy the working dir to testdata/bq-emulator/; commit the snapshot."
    status: pending
  - id: tp07_refresh_script
    content: "Add scripts/refresh_bq_emulator_golden.sh modeled on go-googlesql/scripts/refresh_bq_emulator_golden_usa_1910_2013.sh — DELETE then POST then poll for each of the three tables."
    status: pending
  - id: tp07_task
    content: "Add a `task testdata:bq-emulator-refresh` wrapper pointing at the script; group it alongside testdata:fake-gcs-sync."
    status: pending
  - id: tp07_compose
    content: "Bind-mount ./testdata/bq-emulator:/var/lib/bigquery-emulator/initial-data:ro and export EMULATOR_INITIAL_DATA_DIR for the bigquery-emulator service in docker-compose.yml."
    status: pending
  - id: tp07_drop_deselect
    content: "Drop the three PYTHON_SAMPLES_PYTEST_ARGS deselects (test_client_query_total_rows, test_query_results_as_dataframe, test_list_rows_as_dataframe) once the template ships."
    status: pending
  - id: tp07_verify
    content: "Run `task thirdparty:python-bigquery-tests` end-to-end against a fresh THIRDPARTY_FRESH_VOLUME=1 invocation; the three previously deselected tests pass."
    status: pending
  - id: tp07_index
    content: "Flip thirdparty-00-completion-index.plan.md todo `tp07` to `completed`."
    status: pending
isProject: false
---

# Thirdparty 07 — seed `bigquery-public-data` via initial-data-dir

## Source

- `fix_thirdparty_failures_cbe91a41.plan.md` Tier 2.2 (now deleted).
- Failing log lines `.logs/thirdparty-20260602-134739.log:2246, 2626,
  2783`:
  - `test_client_query_total_rows` queries
    `bigquery-public-data.usa_names.usa_1910_2013`.
  - `test_query_results_as_dataframe` queries
    `bigquery-public-data.usa_names.usa_1910_current`.
  - `test_list_rows_as_dataframe` queries
    `bigquery-public-data:samples.shakespeare`.
- Reference implementation:
  [`go-googlesql/scripts/refresh_bq_emulator_golden_usa_1910_2013.sh`](/home/brighten-tompkins/Code/go-googlesql/scripts/refresh_bq_emulator_golden_usa_1910_2013.sh)
  + [`go-googlesql/taskfiles/emulator.yml:16,51,71-77`](/home/brighten-tompkins/Code/go-googlesql/taskfiles/emulator.yml).

## Prerequisites

- The seed orchestrator must be able to pull from production. That
  is the `-tags=seed_production_live` build path; see
  [`docs/SEEDING.md §3`](../../docs/SEEDING.md). If the production
  reader is not landing this cycle, ship the deselect interim and
  revisit when it lands. Mark this row as `pending` in the index and
  do not block downstream plans on it.

## Scope

Three tables need to live in the emulator catalog before the three
python snippets pass. They cannot be wired through fake-gcs — they
need pre-seeded rows. This plan ports the
`testdata/bq-emulator/` initial-data-dir pattern from go-googlesql
verbatim. Every primitive already exists in this repo:

- `--initial-data-dir` + `BIGQUERY_EMULATOR_INITIAL_DATA_DIR` /
  `EMULATOR_INITIAL_DATA_DIR` fallbacks:
  [`binaries/gateway_main/cli.go:73-77, 206-210, 264-273`](../../binaries/gateway_main/cli.go).
  Documented in [`docs/SEEDING.md §2`](../../docs/SEEDING.md).
- `POST /api/emulator/seed` LRO surface:
  [`gateway/seed/handler.go`](../../gateway/seed/handler.go) +
  [`gateway/seed/orchestrator.go`](../../gateway/seed/orchestrator.go).
- Token guard + loopback gate via `--enable-seed-api` /
  `--seed-api-allow-remote` / `--seed-api-seed-token` (also reads
  `BIGQUERY_EMULATOR_SEED_TOKEN`).

**Storage layout caveat.** This repo uses DuckDB
(`catalog.duckdb`) for the persistent catalog where go-googlesql
uses Pebble (`meta/`). The on-disk template layout differs from
go-googlesql's exact files — but the workflow (snapshot the live
emulator's `--data-dir` after seeding, check it in, ship via
`--initial-data-dir`) is identical. The "initialized" sentinel that
`--initial-data-dir` checks for is `catalog.duckdb` per
[`docs/SEEDING.md:106`](../../docs/SEEDING.md), so the operator just
runs the emulator once with the seed API, then copies the resulting
`--data-dir` into `testdata/bq-emulator/`.

## Implementation

### 1. Bootstrap the template snapshot (one-time, by hand or new script)

- Build the gateway with `-tags=seed_production_live` so the seed
  API can actually pull from production.
- Run the emulator against a clean `--data-dir`.
- `POST /api/emulator/seed` three times (one per table), or once
  with a dataset-scope source for `bigquery-public-data.samples`.
- Stop the emulator, copy the working dir to
  `testdata/bq-emulator/`, commit (large initial commit —
  go-googlesql's snapshot is in the MB range; usa_1910_2013 alone
  is ~5.5M rows).

### 2. Add `scripts/refresh_bq_emulator_golden.sh`

Model on
[`go-googlesql/scripts/refresh_bq_emulator_golden_usa_1910_2013.sh`](/home/brighten-tompkins/Code/go-googlesql/scripts/refresh_bq_emulator_golden_usa_1910_2013.sh).
Differences from the reference:

- Start `./bin/gateway_main` (with `--engine_binary=./bin/emulator_main`),
  not `go run ./cmd/bq-emulator`.
- Use the same `EMULATOR_SEED_*` port-pinning,
  `BIGQUERY_EMULATOR_SEED_TOKEN` propagation, and LRO polling loop.
- DELETE-then-POST-then-poll for each of the three tables —
  group into one script with a per-table loop, or sibling scripts.

### 3. Add a `task testdata:bq-emulator-refresh` wrapper

Mirror
[`go-googlesql/taskfiles/emulator.yml:71-77`](/home/brighten-tompkins/Code/go-googlesql/taskfiles/emulator.yml)
(`refresh-golden-usa-1910-2013:`). Group alongside the existing
`task testdata:fake-gcs-sync`.

### 4. Wire `--initial-data-dir` into the runtime path

- In [`docker-compose.yml`](../../docker-compose.yml), bind-mount
  `./testdata/bq-emulator:/var/lib/bigquery-emulator/initial-data:ro`
  and export
  `EMULATOR_INITIAL_DATA_DIR=/var/lib/bigquery-emulator/initial-data`
  for the `bigquery-emulator` service.
- The env-var fallback in
  [`binaries/gateway_main/cli.go:264-273`](../../binaries/gateway_main/cli.go)
  already plumbs that to `cfg.InitialDataDir` without code changes.
- `THIRDPARTY_FRESH_VOLUME=1` (already the aggregator default per
  [`taskfiles/thirdparty.yml:16`](../../taskfiles/thirdparty.yml))
  tears down `bq-emulator-data`, so the next boot re-materializes
  the template.

### 5. Drop the deselect

Once the template lands, remove the three
`PYTHON_SAMPLES_PYTEST_ARGS` deselects (or whatever skip mechanism
was added as an interim — see the build-tag fallback below). The
three `bigquery-public-data` snippets pass without test-source
changes.

## Build-tag stub fallback

If the production reader is not yet wired this cycle, keep a
`PYTHON_SAMPLES_PYTEST_ARGS` deselect default in the python task for
those three nodeids, marked `TODO(thirdparty-seed)` so it is easy to
grep when the template lands:

```bash
: "${PYTHON_SAMPLES_PYTEST_ARGS:=--deselect docs/snippets.py::test_client_query_total_rows --deselect docs/snippets.py::test_query_results_as_dataframe --deselect docs/snippets.py::test_list_rows_as_dataframe}"
```

## Tests

- `task thirdparty:python-bigquery-tests` with
  `THIRDPARTY_FRESH_VOLUME=1` runs all three previously-deselected
  tests and they pass.
- Refresh script reproduces the template against a fresh clone
  (idempotent on already-seeded tables).
- Docker-compose smoke spins up with the bind mount and the three
  tables are queryable.

## Done criteria

- `testdata/bq-emulator/` snapshot exists in-tree and is loaded by
  compose.
- Refresh script + task wrapper land alongside.
- The three deselects are gone.
- `thirdparty-00-completion-index.plan.md` todo `tp07` flipped to
  `completed`.
