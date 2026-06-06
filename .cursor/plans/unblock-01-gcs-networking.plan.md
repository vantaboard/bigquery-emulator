---
name: Unblock 01 — Container GCS networking
overview: Wire STORAGE_EMULATOR_HOST into the bigquery-emulator container so gs:// fetches reach fake-gcs-server on the compose network (not 127.0.0.1).
depends_on: []
blocks: [unblock-02-public-data-seed, unblock-06-load-avro-orc, unblock-07-hive-external, unblock-08-storage-grpc]
baseline_log: .logs/thirdparty-20260605-134407.log
est_effort: ~1 day
isProject: true
todos:
  - id: compose-env
    content: Add STORAGE_EMULATOR_HOST passthrough on bigquery-emulator service in docker-compose.yml
    status: pending
  - id: thirdparty-bringup
    content: Export STORAGE_EMULATOR_HOST=http://fake-gcs-server:4443 in taskfiles/thirdparty.yml and recreate emulator when fake-gcs is up
    status: pending
  - id: fetch-test
    content: Add gateway/load fetch test or documented in-container wget check for cloud-samples-data bucket
    status: pending
  - id: gate-load-csv
    content: "Fast gate: PYTHON_SAMPLES_PYTEST_ARGS='samples/tests/test_load_table_uri_csv.py -v' task thirdparty:python-bigquery-tests"
    status: pending
  - id: status-commit
    content: Update orchestration-status.md row; lint + commit
    status: pending
---

# Unblock 01 — Container GCS networking

## Goal

Fix the **highest-ROI** infrastructure blocker: inside the `bigquery-emulator` Docker container, `gateway/load/fetch.go` defaults to `http://127.0.0.1:4443`, but `fake-gcs-server` is a **separate** container. Every `gs://` load/extract/external fetch fails with `connection refused`.

## Log signature

```
fetch gs://cloud-samples-data/...: Get "http://127.0.0.1:4443/...": dial tcp 127.0.0.1:4443: connect: connection refused
```

Affects ~22 node Jobs tests, Java `CreateTableExternalHivePartitionedIT`, and 2 node external-table tests.

## Root cause

[`gateway/load/fetch.go`](../../gateway/load/fetch.go) `storageEmulatorBase()`:

- Uses `STORAGE_EMULATOR_HOST` when set
- Otherwise falls back to `http://127.0.0.1:${FAKE_GCS_PORT}` — correct for **host-side** tests, wrong **in-container**

[`docker-compose.yml`](../../docker-compose.yml) does not pass `STORAGE_EMULATOR_HOST` into `bigquery-emulator`.

## Implementation

### 1. docker-compose.yml

Add to `bigquery-emulator.environment`:

```yaml
STORAGE_EMULATOR_HOST: ${STORAGE_EMULATOR_HOST:-}
```

Empty default preserves `task docker:smoke` / quickstart without fake-gcs.

### 2. taskfiles/thirdparty.yml

When bringing up thirdparty lane (`fake-gcs-up` and suites that need GCS):

1. `export STORAGE_EMULATOR_HOST=http://fake-gcs-server:4443`
2. Ensure `fake-gcs-server` is up **before** recreating `bigquery-emulator`
3. `docker compose up -d --force-recreate bigquery-emulator` with env forwarded (compose reads host env for substitution)

Document in plan comments: host-side client tests still use `localhost:4443`; only the **container** needs the service hostname.

### 3. Verification helper

Either:

- Unit test in `gateway/load/fetch_test.go` with `t.Setenv("STORAGE_EMULATOR_HOST", httptest.Server...)`, or
- Shell check in `task thirdparty:fake-gcs-up` postflight:

```bash
docker compose exec bigquery-emulator wget -qO- "${STORAGE_EMULATOR_HOST}/storage/v1/b/cloud-samples-data" | head -c 200
```

### 4. Prerequisite data

```bash
task testdata:fake-gcs-sync   # if testdata/fake-gcs-data/ empty
task thirdparty:fake-gcs-up
```

## Fast gate

```bash
go test ./gateway/load/... -count=1 -run Fetch
PYTHON_SAMPLES_PYTEST_ARGS='samples/tests/test_load_table_uri_csv.py -v' task thirdparty:python-bigquery-tests
```

## Slow gate (optional, parent verify)

```bash
# Spot-check node load GCS family after docker rebuild
THIRDPARTY_REBUILD=1 task thirdparty:node-bigquery-tests 2>&1 | tail -30
```

## Expected delta

- Node Jobs bucket: large reduction (was 22/45)
- Java hive IT: fetch succeeds (may still fail until plan 07 hive options)
- External GCS node tests: connectivity fixed

## Out of scope

- AVRO/ORC parsers (plan 06)
- Hive partition URI expansion (plan 07)
- Changing fake-gcs fixture contents (only sync + reachability)

## Files

| File | Change |
|------|--------|
| [`docker-compose.yml`](../../docker-compose.yml) | `STORAGE_EMULATOR_HOST` env passthrough |
| [`taskfiles/thirdparty.yml`](../../taskfiles/thirdparty.yml) | Export host env; recreate emulator with GCS hostname |
| [`gateway/load/fetch.go`](../../gateway/load/fetch.go) | Only if hostname normalization needs docker DNS tweak |
| [`gateway/load/fetch_test.go`](../../gateway/load/fetch_test.go) | Optional regression test |

## Done criteria

- [ ] In-container `wget` to `fake-gcs-server:4443` succeeds
- [ ] `test_load_table_uri_csv` passes against docker emulator
- [ ] No regression on `task docker:smoke` (STORAGE_EMULATOR_HOST unset)
