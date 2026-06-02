---
name: extract jobs (table -> GCS)
overview: "Implement extract jobs to GCS (CSV / JSON / compressed) via DuckDB's `COPY (...) TO 'gs://...'` after fake-gcs URL rewriting. Shares the GCS-URI rewriting layer with plan 10."
todos:
  - id: tp11_storage_primitive
    content: "Add Storage::ExportToURI(table TableRef, destinationURI string, format, compression) routing through DuckDBStorage::ScanRows plus DuckDB COPY ... TO '...'."
    status: pending
  - id: tp11_uri_rewrite
    content: "Reuse the gs:// -> http(s)://${STORAGE_EMULATOR_HOST}/... URI rewrite layer from plan 10 so COPY ... TO targets fake-gcs when configured."
    status: pending
  - id: tp11_formats
    content: "Format support: CSV (default), NEWLINE_DELIMITED_JSON, plus the GZIP compression option (`destinationFormat` + `compression`)."
    status: pending
  - id: tp11_uri_sharding
    content: "Sharded output: when destinationUris contains an asterisk (e.g. gs://bucket/prefix-*.csv), shard the output across multiple objects per BigQuery's documented sharding semantics."
    status: pending
  - id: tp11_handler
    content: "Wire JobInsert's configuration.extract dispatch branch to Storage::ExportToURI."
    status: pending
  - id: tp11_tests
    content: "Unit-test Storage::ExportToURI per format; integration-test the three node extract rows + the corresponding python `test_extract_table*` rows (also passing through fake-gcs from plan 03)."
    status: pending
  - id: tp11_index
    content: "Flip thirdparty-00-completion-index.plan.md todo `tp11` to `completed`."
    status: pending
isProject: false
---

# Thirdparty 11 — extract jobs

## Source

- `not-implemented-routes-catalog.plan.md` Group 5 (now deleted).
- Failing tests:
  - node: `should extract a table to GCS CSV file`
  - node: `should extract a table to GCS JSON file`
  - node: `should extract a table to GCS compressed file`
  - python (after plan 03): `test_extract_table`,
    `test_extract_table_json`, `test_extract_table_compressed`.
- Stub: [`gateway/handlers/jobs.go`](../../gateway/handlers/jobs.go)
  `JobInsert` for `configuration.extract`.

## Prerequisites

- [`thirdparty-02-fake-gcs-readiness.plan.md`](./thirdparty-02-fake-gcs-readiness.plan.md)
  — fake-gcs healthz wait.
- [`thirdparty-08-jobs-insert-implementation.plan.md`](./thirdparty-08-jobs-insert-implementation.plan.md)
  — JobInsert dispatcher.
- [`thirdparty-10-load-jobs.plan.md`](./thirdparty-10-load-jobs.plan.md)
  — strongly recommended sequencing (the GCS client and URI rewrite
  layer ship there; extract reuses them).

## Scope

- `Storage::ExportToURI(table TableRef, destinationURI string,
  format, compression)` storage primitive.
- Format support: CSV (default), NEWLINE_DELIMITED_JSON.
- Compression: GZIP (BigQuery's documented option; DEFLATE/SNAPPY
  later if a sample asks).
- Sharded output: when `destinationUris[]` contains a single URI
  with an asterisk, BigQuery shards across `prefix-000000000000.csv`,
  `prefix-000000000001.csv`, etc. Reproduce that semantics.
- `JobInsert` dispatch branch for `configuration.extract`.

Out of scope: AVRO + PARQUET destination format (file as follow-on
when a sample exercises them — Parquet specifically is on the
extract surface but no current third-party sample hits it).

## Implementation

### Storage primitive

`Storage::ExportToURI` lives in
`backend/storage/duckdb/duckdb_storage.cc`. Lowering:

- For CSV / JSON: `COPY (SELECT * FROM <table>) TO '<rewritten URI>'
  WITH (FORMAT 'csv'|'json', HEADER, COMPRESSION 'gzip'?)`.
- Multi-URI / sharded: DuckDB's `COPY (...) TO 'path-*'` does
  emit sharded files when `PER_THREAD_OUTPUT` is true; if that
  semantics does not match BigQuery's, fall back to a manual
  scan-then-emit loop that distributes rows across the shard
  count.
- Use `DuckDBStorage::ScanRows` as the underlying read path so
  the column types match what the rest of the storage layer
  produces (the existing schema-mapping code is reused).

### URI rewriting

Reuse the layer from plan 10. Confirm that DuckDB's `COPY TO`
accepts the rewritten `http(s)://` URL via its `httpfs`
extension (already a dependency for parquet remote reads). If
not, add a small temp-buffer pathway: `COPY` to a local tempfile,
then PUT the tempfile to fake-gcs over HTTP.

### Handler dispatch

Wire the `configuration.extract` branch in JobInsert to call
`Storage::ExportToURI`. Honor the same async PENDING -> RUNNING ->
DONE lifecycle from plan 08.

### Test seeding

Tests need fake-gcs primed with empty target buckets. Reuse the
existing `fake-gcs-up` flow (now race-free per plan 02).

## Tests

- `backend/storage/duckdb/export_to_uri_test.cc` per format /
  compression / sharding.
- `gateway/handlers/jobs/extract_test.go` handler integration test.
- `task thirdparty:node-bigquery-tests` — three extract rows go
  green.
- `task thirdparty:python-bigquery-tests` — three
  `test_extract_table*` rows go green (assuming plan 03 has
  landed).

## Done criteria

- `Storage::ExportToURI` lands for CSV + JSON + GZIP.
- Sharded output matches BigQuery's documented semantics.
- All extract rows in node + python suites pass.
- `thirdparty-00-completion-index.plan.md` todo `tp11` flipped to
  `completed`.
