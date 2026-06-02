---
name: not-implemented routes (node thirdparty backlog)
overview: "Catalog the BigQuery REST routes the node-bigquery-tests suite hits that still return a wired 501 NotImplemented today. Each row maps a test class to the handler stub, the engine plumbing the implementation needs, and the existing `.cursor/plans/` entry (if any) that already owns the work. This file is a tracker, not a code-change plan; it exists so the post-fix node failure count is exactly the set listed here."
todos:
  - id: jobs-list-get
    content: "Wire jobs.list / jobs.get / jobs.delete / jobs.cancel against the in-memory jobs.Registry (gateway/handlers/jobs.go)"
    status: pending
  - id: routines-crud
    content: "Implement routines.* CRUD against the engine catalog (UDFs, stored procedures)"
    status: pending
  - id: models-crud
    content: "Implement models.* CRUD (BQML); needs an engine model registry first"
    status: pending
  - id: load-jobs
    content: "Implement load jobs from GCS (CSV/JSON/Avro/ORC/Parquet/Firestore) -> plain / partitioned / clustered tables, append / truncate / add-column / relax-column"
    status: pending
  - id: extract-jobs
    content: "Implement extract jobs to GCS (CSV/JSON/compressed)"
    status: pending
  - id: copy-jobs
    content: "Implement copy jobs (single + multi-source)"
    status: pending
  - id: tabledata-list-impl
    content: "Wire tabledata.list (already a stub) against DuckDBStorage scan paginator (gateway/handlers/tabledata.go)"
    status: pending
  - id: load-local-csv
    content: "Implement upload-form load jobs (local CSV / JSON file -> table) via /upload/bigquery/v2/projects/{p}/jobs"
    status: pending
isProject: false
---

## Why this exists

After items 1-10 of `fix_thirdparty_test_failures_*.plan.md` land, the
**node-bigquery-tests** suite's remaining failures collapse onto the
routes below. Every entry returns the same 501 envelope today
(`handlers.NotImplemented` -> `"This BigQuery emulator route is
registered but not yet implemented. See ROADMAP.md."`); the upstream
samples that trip the failure are tracked alongside each.

The goal is **not** to ship the implementations here. The goal is to:

1. Confirm the node failure count drops to exactly the routes in this
   file after items 1-10 of the thirdparty plan land, so we know we
   are looking at feature work and not regressions.
2. Give each route an explicit owner in the form of an existing
   `.cursor/plans/` entry or a TODO sibling here, so a future plan can
   pick up exactly one row without re-doing the discovery.

Conventions: each row carries the failing node tests, the handler
stub, the engine surface the implementation needs, and the existing
`.cursor/plans/` plan (if any) it should land under. The
`.cursor/plans/` files referenced below all sit in this directory.

## Catalog

### Group 1 — Jobs

| Failing test (node-bigquery-tests) | Handler stub | Engine surface needed | Existing plan |
|---|---|---|---|
| `Jobs > should list jobs` (before-all) | [`JobList`](../../gateway/handlers/jobs.go) | Surface the in-memory `gateway/jobs.Registry` over a list page. No engine changes. | None — track here. |
| (parent of) `Jobs > should retrieve a job` | [`JobGet`](../../gateway/handlers/jobs.go) | Read from `gateway/jobs.Registry` by ID. No engine changes. | None — track here. |
| (parent of) `Jobs > should cancel a job` | [`JobCancel`](../../gateway/handlers/jobs.go) | Flip the `Registry` entry's state. No engine changes. | None — track here. |
| (parent of) `Jobs > should delete a job` | [`JobDelete`](../../gateway/handlers/jobs.go) | Remove from `Registry` (parent cascades to children). | None — track here. |

The synchronous `jobs.query` path already populates `Jobs` from
`QueryRun`, so the registry has live entries to surface. All four
routes are single-handler changes plus an updated `JobInsert` that
records the minted job in the registry instead of relying on the
sync path.

### Group 2 — Routines (UDF / procedure CRUD)

| Failing test | Handler stub | Engine surface needed | Existing plan |
|---|---|---|---|
| `Routines > should create a routine` (before-all) | [`RoutineInsert`](../../gateway/handlers/routines.go) | A persistent routine registry in `Storage`; the executor side already has UDF/macro plumbing under [`udf-tvf-module-routing.plan.md`](./udf-tvf-module-routing.plan.md) and [`duckdb-polyfill-udf-library.plan.md`](./duckdb-polyfill-udf-library.plan.md). | The CRUD surface is **not** covered by those two plans — they target query-time UDF lookup. Track the CRUD half here. |
| (cascades) `RoutineUpdate` | [`RoutineUpdate`](../../gateway/handlers/routines.go) | Same. | Same. |

### Group 3 — Models (BQML CRUD)

| Failing test | Handler stub | Engine surface needed | Existing plan |
|---|---|---|---|
| `Models > should retrieve a model if it exists` (before-all) | [`ModelGet`](../../gateway/handlers/models.go) | An engine model registry. No current plan covers BQML model state. | None — track here. |
| `Create/Delete Model > should create a model` | (currently `NotImplemented` via the missing `ModelInsert` route, i.e. handled by `NotImplemented` shim) | Same. | None — track here. |
| `Create/Delete Model > should delete a model` | [`ModelDelete`](../../gateway/handlers/models.go) (or unwired) | Same. | None — track here. |
| (cascades) `ModelPatch` | [`ModelPatch`](../../gateway/handlers/models.go) | Same. | None — track here. |

### Group 4 — Load jobs

All from the same family in `tables.test.js`:

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

Wire surface: `POST /upload/bigquery/v2/projects/{p}/jobs` (handled
by [`JobInsertUpload`](../../gateway/handlers/jobs.go)) for the
local-file variant; `POST /bigquery/v2/projects/{p}/jobs` (handled
by [`JobInsert`](../../gateway/handlers/jobs.go)) for the GCS
variant with `configuration.load.sourceUris`. Both currently return
501.

Engine surface needed:

- The DuckDB engine already speaks CSV / JSON / Parquet via the
  fast-path SQL `read_csv_auto` / `read_json` / `read_parquet` (see
  [`duckdb-fast-path-stabilization.plan.md`](./duckdb-fast-path-stabilization.plan.md)).
- A `Storage.LoadFromURIs(table, sourceURIs, format, schema?, writeDisposition)`
  primitive that wires those reads into the existing `AppendRows` /
  `OverwriteRows` paths in `backend/storage/duckdb/duckdb_storage.cc`.
  Avro / ORC / Firestore-backup payloads need a converter layer the
  engine does not have today; track those as a separate sub-row when
  the rest land.
- The GCS lane requires the in-tree `fake-gcs-server` mount
  (`docker-compose.yml` already provisions it under the `thirdparty`
  profile) plus a small client in `backend/storage/`.

Existing plan: none yet. The C++ side of the work touches
storage-not-engine, so a new plan is justified. Recommended name:
`.cursor/plans/load-job-storage.plan.md`.

### Group 5 — Extract jobs

| Failing test | Handler stub | Engine surface needed |
|---|---|---|
| `should extract a table to GCS CSV file` | [`JobInsert`](../../gateway/handlers/jobs.go) | `Storage.ExportToURI(table, destinationURI, format, compression)` going through `DuckDBStorage::ScanRows` plus DuckDB's `COPY (...) TO 'gs://...'` after the fake-gcs URL mapping. |
| `should extract a table to GCS JSON file` | same | same |
| `should extract a table to GCS compressed file` | same | same (gzip path) |

No existing plan. Track under the same new
`.cursor/plans/load-job-storage.plan.md` since both load and extract
share the GCS URI rewriting.

### Group 6 — Copy jobs

| Failing test | Handler stub | Engine surface needed |
|---|---|---|
| `should copy a table` | [`JobInsert`](../../gateway/handlers/jobs.go) | `Storage.CopyTable(src, dst, writeDisposition)` — implementable as `INSERT INTO dst SELECT * FROM src` against the existing DuckDB tables; no GCS dependency. |
| `copy multiple source tables to a given destination` | same | same; iterate `sourceTables[]`. |

No existing plan. The work is small enough to roll into the same
`load-job-storage.plan.md`.

### Group 7 — `tabledata.list` (`browse rows`)

| Failing test | Handler stub | Engine surface needed |
|---|---|---|
| `Tables > should browse table rows` | [`TableDataList`](../../gateway/handlers/tabledata.go) | The handler already calls into the engine for `TableDataInsertAll`; `TableDataList` needs a paginating wrapper around `DuckDBStorage::ScanRows`. |

The roadmap (`ROADMAP.md` line ~313) lists this as `✅` but the
implementation is still missing in handler code (the handler returns
`NotImplemented` when `deps.Catalog == nil` and is otherwise a stub).
Mismatch tracked here so the next plan re-confirms or finishes the
work.

## Process

When the next plan picks up one of these rows:

1. Re-run `task thirdparty:node-bigquery-tests` first to confirm the
   route still fails the same way (the surface may have moved).
2. Add a `.cursor/plans/<group>.plan.md` per the "existing plan"
   column (or new file when none) and check that row off in this
   tracker by setting its `status: completed`.
3. Follow `.cursor/rules/auto-commit.mdc` for the implementation
   commits; do not bundle the catalog update with feature work.

## Out of scope

- IAM custom methods (`tables.{get,set,test}IamPolicy`,
  `rowAccessPolicies.*`): the upstream node samples do not exercise
  them, so they stay 501 with no node-suite signal pressure.
- `migration.workflows.*`: same.
- `datasets.undelete`: same; engine has no soft-delete state.
