---
name: Thirdparty 04 — tp08 LOAD jobs
overview: Implement jobs.insert LOAD and JobInsertUpload — GCS/local file ingest for CSV, JSON, Avro, Parquet, ORC with write disposition and schema-update modes.
depends_on: [thirdparty-03-tp08-jobs-foundation]
blocks: [thirdparty-07-external-tables, thirdparty-11-bigframes-gate]
baseline_log: .logs/thirdparty-20260605-112926.log
est_effort: 1-2 weeks
isProject: true
todos:
  - id: gcs-fetch
    content: Fetch sourceUris from STORAGE_EMULATOR_HOST (fake-gcs) and local file paths
    status: pending
  - id: format-csv-json
    content: CSV + newline-delimited JSON parsers → rows for bulk insert
    status: pending
  - id: format-avro-parquet-orc
    content: Avro, Parquet, ORC readers (Arrow/DuckDB where possible)
    status: pending
  - id: write-disposition
    content: WRITE_TRUNCATE, WRITE_APPEND, schema add/relax via schemaUpdateOptions
    status: pending
  - id: bulk-insert
    content: Persist rows via tabledata.insertAll batching or new engine bulk-load RPC
    status: pending
  - id: resumable-upload
    content: Implement JobInsertUpload multipart + resumable per api-uploads.md
    status: pending
  - id: load-tests
    content: Gateway integration tests + re-run python/node load_table_* suites
    status: pending
---

# Thirdparty 04 — tp08 LOAD jobs

## Goal

Clear **~20 load-job failures** in python and node thirdparty suites.

## Log signature

```
jobs.insert: only configuration.query is implemented; load / copy / extract paths land in thirdparty-08.
```

Also: `This BigQuery emulator route is registered but not yet implemented` for resumable upload route.

## Architecture

```mermaid
sequenceDiagram
  participant Client
  participant Gateway
  participant GCS as fake_gcs_or_local
  participant Engine
  participant Storage
  Client->>Gateway: POST jobs.insert configuration.load
  Gateway->>GCS: GET object bytes for each sourceUri
  Gateway->>Gateway: parse by sourceFormat
  Gateway->>Engine: ensure destination table + schema
  Gateway->>Storage: bulk insert rows
  Gateway->>Client: Job DONE + statistics.load
```

## Python tests unblocked (baseline)

- `test_client_load_partitioned_table`
- `test_load_table_clustered`, `test_load_table_dataframe`, `test_load_table_file`
- `test_load_table_uri_*` (csv, json, avro, parquet, orc, autodetect, truncate variants, cmek)
- Node: `should load a local CSV file`, all GCS load variants (items 34–55)

## Implementation phases

### Phase A — GCS + CSV/JSON (highest ROI)

**Files:**
- [`gateway/handlers/jobs.go`](../../gateway/handlers/jobs.go) — `runSyncLoadInsert`
- New package e.g. [`gateway/load/`](../../gateway/load/) — URI fetch + parse
- Reuse fake-gcs fixtures from `testdata/fake-gcs-data/` (`task testdata:fake-gcs-sync`)

**GCS fetch:** Translate `gs://bucket/object` → `http://127.0.0.1:4443/storage/v1/b/{bucket}/o/{object}?alt=media` (or fake-gcs download URL).

**Local files:** Node `load a local CSV` — support `file://` paths or temp upload dir.

### Phase B — Avro / Parquet / ORC

Use DuckDB `read_parquet` / Arrow libraries in gateway (Go) or delegate to engine via new RPC. Priority order matches failing test count: Parquet, Avro, ORC.

### Phase C — Write disposition + schema updates

- `writeDisposition: WRITE_TRUNCATE` — truncate destination before load
- `schemaUpdateOptions.allowAddColumns` / `allowRelaxColumns` — node items 47–48, python add/relax via **load job** (distinct from query-job path in plan 08)

### Phase D — Resumable upload (`JobInsertUpload`)

[`gateway/handlers/jobs.go`](../../gateway/handlers/jobs.go) `JobInsertUpload` currently `NotImplemented`.

Required for python `test_load_table_file` and docs resumable-upload snippets.

Spec: [`docs/bigquery/docs/reference/api-uploads.md`](../../docs/bigquery/docs/reference/api-uploads.md)

## Engine interaction

Options (pick one, document in plan PR):
1. **Gateway-only:** Parse files in Go, batch `tabledata.insertAll` REST internally
2. **Engine RPC:** Add `BulkLoad` to [`proto/emulator.proto`](../../proto/emulator.proto) — better for large files

Existing engine note: `RESOLVED_AUX_LOAD_DATA_STMT` is UNIMPLEMENTED in [`backend/engine/control/control_op_executor.cc`](../../backend/engine/control/control_op_executor.cc) for SQL `LOAD DATA`; REST load jobs are a **separate path** (gateway orchestration).

## Verification

```bash
# Prerequisites
task testdata:fake-gcs-sync
task thirdparty:fake-gcs-up

# Fast
go test ./gateway/... -count=1 -run Load

# Full
task thirdparty:python-bigquery-tests
task thirdparty:node-bigquery-tests
```

Filter to load families first:
```bash
PYTHON_SAMPLES_PYTEST_ARGS='samples/tests/test_load_table_uri_csv.py -v' task thirdparty:python-bigquery-tests
```

## Out of scope

- COPY / EXTRACT jobs (plan 05)
- External table **queries** (plan 07)
- Cloud Firestore backup format (low priority — skip or 501 with clear message)

## Done when

- [ ] All `test_load_table_uri_*` and node GCS load tests pass against fake-gcs
- [ ] Local CSV load passes
- [ ] Resumable upload route accepts payloads (or documented deferral with test skip removed)
- [ ] `ROADMAP.md` tp08 load section updated
