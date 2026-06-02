---
name: load jobs (GCS + local)
overview: "Implement load jobs from GCS and local files (CSV / JSON / Parquet first; Avro / ORC / Firestore-backup as follow-on once the rest pass). Adds `Storage::LoadFromURIs` on top of DuckDB's `read_csv_auto` / `read_json` / `read_parquet`. Largest job-kind plan; the load surface is the long tail in the node Tables suite."
todos:
  - id: tp10_storage_primitive
    content: "Add Storage::LoadFromURIs(table TableRef, sourceURIs []string, format, schema?, writeDisposition) — CSV/JSON/Parquet first, with appendrows / overwriterows wiring."
    status: pending
  - id: tp10_gcs_client
    content: "Add a small GCS client in backend/storage/ that rewrites gs:// URIs to the fake-gcs-server endpoint when STORAGE_EMULATOR_HOST is set, and falls back to real GCS otherwise (mirror google-cloud-storage client behavior)."
    status: pending
  - id: tp10_uri_resolution
    content: "URI resolution + globbing: support `gs://bucket/prefix*`, `gs://bucket/a,b,c`, comma-separated and brace expansion; resolve to a deterministic ordered list of object URLs."
    status: pending
  - id: tp10_schema
    content: "Schema handling: explicit schema (from configuration.load.schema), autodetect (configuration.load.autodetect=true), and schema evolution (add-column, relax-column via fieldUpdates / schemaUpdateOptions)."
    status: pending
  - id: tp10_write_dispositions
    content: "WRITE_APPEND / WRITE_TRUNCATE / WRITE_EMPTY honoring; CREATE_IF_NEEDED vs CREATE_NEVER table creation gates."
    status: pending
  - id: tp10_partitioned_clustered
    content: "Partitioned-table + clustered-table destinations: respect timePartitioning + clustering in the destinationTable + JobInsert config."
    status: pending
  - id: tp10_local_upload
    content: "Local-file path: feed the multipart/resumable upload buffer from plan 08 into Storage::LoadFromURIs via a `file://<tempfile>` URI."
    status: pending
  - id: tp10_followon_formats
    content: "Avro / ORC / Firestore-backup converter layer — file as a sibling follow-up since DuckDB does not speak these natively. Track here, do not block the main plan landing."
    status: pending
  - id: tp10_tests
    content: "Unit-test the storage primitive per format; integration-test the full node tables.test.js load-job rows."
    status: pending
  - id: tp10_index
    content: "Flip thirdparty-00-completion-index.plan.md todo `tp10` to `completed` once CSV/JSON/Parquet land (Avro/ORC/Firestore as a sibling row)."
    status: pending
isProject: false
---

# Thirdparty 10 — load jobs

## Source

- `not-implemented-routes-catalog.plan.md` Group 4 (now deleted).
- Failing node tests (all from `tables.test.js`):
  - `should load a local CSV file`
  - `should load a GCS ORC file`
  - `should load a GCS Parquet file`
  - `should load a GCS Avro file`
  - `should load a GCS Firestore backup file`
  - `should load a GCS CSV file with explicit schema`
  - `should load a GCS JSON file with explicit schema`
  - `should load a GCS CSV file to partitioned table`
  - `should load a GCS CSV file to clustered table`
  - `should add a new column via a GCS file load job`
  - `should relax a column via a GCS file load job`
  - `should load a GCS CSV file with autodetected schema`
  - `should load a GCS JSON file with autodetected schema`
  - `should load a GCS CSV file truncate table`
  - `should load a GCS JSON file truncate table`
  - `should load a GCS parquet file truncate table`
  - `should load a GCS ORC file truncate table`
  - `should load a GCS Avro file truncate table`
- python-bigquery-tests: `test_load_table_add_column`,
  `test_load_table_relax_column` (also passes through the upload
  surface from plan 08).
- Stubs: [`gateway/handlers/jobs.go`](../../gateway/handlers/jobs.go)
  `JobInsert` (GCS path) + `JobInsertUpload` (local-file path).

## Prerequisites

- [`thirdparty-02-fake-gcs-readiness.plan.md`](./thirdparty-02-fake-gcs-readiness.plan.md)
  — fake-gcs healthz wait.
- [`thirdparty-08-jobs-insert-implementation.plan.md`](./thirdparty-08-jobs-insert-implementation.plan.md)
  — the dispatcher that hands off `configuration.load` to a storage
  primitive.

## Scope

This is the largest job-kind plan. It lands in three slices:

1. **Core formats**: CSV, JSON, Parquet — DuckDB speaks these
   natively via `read_csv_auto`, `read_json`, `read_parquet`.
2. **Operational shapes**: partitioned + clustered destinations,
   write dispositions, schema evolution (add column / relax column),
   autodetect vs explicit.
3. **Long-tail formats** (Avro, ORC, Firestore backup): need a
   converter layer DuckDB does not have today. File as a sibling
   plan; do not block the main plan landing on these.

The GCS lane requires the in-tree fake-gcs-server mount (already
provisioned by `docker-compose.yml` under the `thirdparty` profile)
plus a small GCS client in `backend/storage/`.

## Implementation

### 1. Storage primitive

`Storage::LoadFromURIs(table, sourceURIs, format, schema?,
writeDisposition)` lives in
`backend/storage/duckdb/duckdb_storage.cc`. Internally:

- Resolve `sourceURIs` (see "URI resolution" below).
- Build a DuckDB SELECT against the appropriate reader
  (`read_csv_auto`, `read_json`, `read_parquet`).
- Apply `schema` projection if provided; otherwise rely on
  autodetect.
- Wire the resulting row stream into the existing `AppendRows` /
  `OverwriteRows` paths so the rest of the storage layer is
  unchanged.

### 2. GCS client

Add a minimal client in `backend/storage/` (or a sibling
`backend/storage/gcs/`). Behavior:

- When `STORAGE_EMULATOR_HOST` is set, rewrite `gs://bucket/...`
  to `http(s)://${STORAGE_EMULATOR_HOST}/storage/v1/b/bucket/...`.
- Otherwise fall back to real GCS endpoints.
- HTTP-only client; reuse the existing protobuf-free HTTP stack
  in `gateway/` if possible.

### 3. URI resolution

Support:

- Single `gs://bucket/object`.
- Comma-separated lists (`gs://bucket/a,gs://bucket/b`).
- Glob prefixes (`gs://bucket/prefix*`) via the GCS list API.
- Multiple `sourceUris[]` entries combined.

Resolved list must be deterministic + ordered so failures are
reproducible.

### 4. Schema handling

- `configuration.load.schema`: explicit field list — pass as
  projection.
- `configuration.load.autodetect=true`: rely on DuckDB autodetect
  (CSV header sniff, JSON shape inference, Parquet schema).
- `schemaUpdateOptions=[ALLOW_FIELD_ADDITION]`: allow new columns
  in the source; ALTER TABLE ADD COLUMN before insert.
- `schemaUpdateOptions=[ALLOW_FIELD_RELAXATION]`: allow nullable
  promotion (NOT NULL -> nullable).

### 5. Write dispositions + table creation

- `writeDisposition=WRITE_APPEND` (default): INSERT.
- `WRITE_TRUNCATE`: TRUNCATE then INSERT in a transaction.
- `WRITE_EMPTY`: fail with `duplicate` error if rows exist.
- `createDisposition=CREATE_IF_NEEDED` (default): CREATE TABLE
  derived from the load schema.
- `createDisposition=CREATE_NEVER`: fail with `notFound` if the
  table is absent.

### 6. Partitioned + clustered destinations

If the destination table specifies `timePartitioning` or
`clustering`, materialize the table with that DDL on
auto-creation, then route inserts through the partition / cluster
path. DuckDB does not natively cluster by columns — the cluster
list is a hint; storage records it for catalog accuracy but does
not enforce it (document the limitation).

### 7. Local-file path

`JobInsertUpload` from plan 08 buffers the upload to a tempfile;
the dispatch creates a `file://<tempfile>` URI and calls
`Storage::LoadFromURIs`. Tempfile is cleaned up on completion or
expiry.

### 8. Long-tail formats (follow-on)

Avro, ORC, Firestore backup: DuckDB does not have a native reader.
Three options:

- Apache Avro C++ via the existing third-party tree.
- Apache ORC C++ via the same.
- A first-party reader implementation if neither is acceptable.

These cannot be silently approximated. File a follow-on plan once
the CSV/JSON/Parquet rows land and the long-tail rows are the only
ones left red.

## Tests

- `backend/storage/duckdb/load_from_uris_test.cc` — per-format
  unit tests with fake-gcs-server stood up in the test harness.
- `gateway/handlers/jobs/load_test.go` — handler-level dispatch
  per scenario (autodetect / explicit / append / truncate /
  partitioned / clustered / schema evolution).
- `task thirdparty:node-bigquery-tests` — all GCS-load rows in
  tables.test.js go green except Avro/ORC/Firestore (those red
  rows are owned by the follow-on plan).
- `task thirdparty:python-bigquery-tests` —
  `test_load_table_add_column` and `test_load_table_relax_column`
  pass.

## Done criteria

- `Storage::LoadFromURIs` lands for CSV / JSON / Parquet.
- GCS client + URI resolver land.
- Partitioned + clustered destinations honored.
- Local-file upload path works end-to-end via plan 08's
  multipart + resumable upload.
- Avro / ORC / Firestore-backup tracked in a sibling follow-on
  plan; the corresponding node rows stay red until then.
- `thirdparty-00-completion-index.plan.md` todo `tp10` flipped to
  `completed` once CSV/JSON/Parquet land.
