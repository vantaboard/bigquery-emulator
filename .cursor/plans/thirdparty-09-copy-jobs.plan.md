---
name: copy jobs
overview: "Implement single + multi-source copy jobs by lowering `configuration.copy` to `INSERT INTO dst SELECT * FROM src` against the existing DuckDB tables. No GCS dependency. Smallest of the three job-kind plans on top of `JobInsert`."
todos:
  - id: tp09_storage_primitive
    content: "Add Storage::CopyTable(src []TableRef, dst TableRef, writeDisposition) — single + multi-source — implemented as INSERT INTO dst SELECT * FROM src1 UNION ALL SELECT * FROM src2 ... (with WRITE_TRUNCATE / WRITE_APPEND honoring)."
    status: pending
  - id: tp09_handler
    content: "Wire JobInsert's load-dispatch sibling for configuration.copy (post-plan-08) to call Storage::CopyTable."
    status: pending
  - id: tp09_schema_compat
    content: "Validate schema compatibility across all sources; reject with bigquery-shaped error if columns differ. Respect destinationTable.tableId creation if it does not exist (CREATE TABLE LIKE src1)."
    status: pending
  - id: tp09_tests
    content: "Unit-test Storage::CopyTable; integration-test via gateway/handlers/jobs/copy_test.go; rerun task thirdparty:node-bigquery-tests for `should copy a table` and `copy multiple source tables to a given destination`."
    status: pending
  - id: tp09_index
    content: "Flip thirdparty-00-completion-index.plan.md todo `tp09` to `completed`."
    status: pending
isProject: false
---

# Thirdparty 09 — copy jobs

## Source

- `not-implemented-routes-catalog.plan.md` Group 6 (now deleted).
- Failing node tests:
  - `should copy a table`
  - `copy multiple source tables to a given destination`
- Stub: [`gateway/handlers/jobs.go`](../../gateway/handlers/jobs.go)
  `JobInsert` returns 501 for `configuration.copy`.
- Catalog note: implementable as `INSERT INTO dst SELECT * FROM src`
  against the existing DuckDB tables; no GCS dependency.

## Prerequisites

- [`thirdparty-08-jobs-insert-implementation.plan.md`](./thirdparty-08-jobs-insert-implementation.plan.md)
  — the dispatcher that hands `configuration.copy` off to a storage
  primitive must exist.

## Scope

- A new `Storage::CopyTable(srcs []TableRef, dst TableRef,
  writeDisposition)` primitive.
- The `JobInsert` dispatch branch for `configuration.copy` calling
  into it.
- Schema-compatibility check (all sources must have the same
  column list and types as the destination, or as each other when
  the destination does not yet exist).
- Single-source and multi-source variants.

Out of scope: cross-region copies, snapshot semantics, time-travel
copy (`CLONE` / `SNAPSHOT` job types — file as a follow-up if a
sample exercises them).

## Implementation

### Storage primitive

Add `Storage::CopyTable` next to the existing append / overwrite
primitives. Behavior:

- `WRITE_APPEND` (default): `INSERT INTO dst SELECT * FROM src1
  UNION ALL SELECT * FROM src2 ...`
- `WRITE_TRUNCATE`: `DELETE FROM dst` first, then `INSERT INTO dst ...`.
  Wrap in a single DuckDB transaction.
- `WRITE_EMPTY`: fail with bigquery-shaped `duplicate` error if
  `dst` exists with rows.
- Destination creation: if `dst` table does not exist, derive
  schema from `src1` via `CREATE TABLE dst LIKE src1` (or the
  DuckDB equivalent) before inserting.

### Schema compatibility

For multi-source copies, verify every source has the same column
list (names + types) as the others (or as the destination, when
provided). If not, return a bigquery-shaped `invalid` error from
the dispatcher (the storage primitive can also self-reject).

### Handler dispatch

In the JobInsert dispatcher (from plan 08), the `configuration.copy`
branch decodes `sourceTables[]` and `destinationTable`, then calls
`Storage::CopyTable`. The job lifecycle is the standard async
PENDING -> RUNNING -> DONE.

## Tests

- `backend/storage/duckdb/copy_table_test.cc` — unit tests for
  single + multi-source, WRITE_TRUNCATE, WRITE_APPEND, schema
  rejection, destination autocreate.
- `gateway/handlers/jobs/copy_test.go` — handler-level integration
  test driving JobInsert.
- `task thirdparty:node-bigquery-tests` — both copy rows go green.
- `task thirdparty:python-bigquery-tests` — any copy samples in
  the python suite that share the path pass too.

## Done criteria

- `Storage::CopyTable` implemented and tested.
- The handler dispatch branch is wired.
- Both node copy rows go green.
- `thirdparty-00-completion-index.plan.md` todo `tp09` flipped to
  `completed`.
