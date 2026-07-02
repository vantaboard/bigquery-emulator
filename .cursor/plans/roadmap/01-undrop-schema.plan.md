---
name: UNDROP SCHEMA
overview: Implement BigQuery's only undrop form locally by extending the existing table-tombstone machinery to dataset scope, wiring RunUndrop and the datasets.undelete REST stub to it.
todos:
  - id: dataset-tombstone
    content: "Extend tombstone machinery to dataset scope: DROP SCHEMA moves all tables + dataset metadata into .tombstones/, restorable by newest timestamp"
    status: pending
  - id: run-undrop
    content: Implement RunUndrop for schema_object_kind() == SCHEMA against the dataset tombstone; keep UNDROP TABLE rejecting (not a BigQuery statement)
    status: pending
  - id: rest-undelete
    content: Wire gateway DatasetUndelete (501 stub today) to the same engine path via a Catalog RPC
    status: pending
  - id: fixtures-registry
    content: Conformance fixtures (drop→undrop round-trip, IF NOT EXISTS, expired/no-tombstone error) + node_dispositions/SHAPE_TRACKER/ENGINE_POLICY/ROADMAP updates
    status: pending
isProject: false
---

# 01 — `UNDROP SCHEMA` (real)

- **Roadmap row:** §Planned work — `UNDROP SCHEMA` | `unsupported` | **real**
- **Policy row:** `docs/ENGINE_POLICY.md` §Unsupported families ~118

## Current state at HEAD (grounded)

- `RunUndrop` (`backend/engine/control/control_op_time_travel.cc` ~262–280)
  returns `UNIMPLEMENTED` for every `ResolvedUndropStmt`. The comment already
  documents that `UNDROP TABLE` is not a BigQuery statement (bq dry-run:
  "Unexpected keyword TABLE") — only `UNDROP SCHEMA` is real.
- Table-scope soft delete already exists:
  `backend/storage/duckdb/duckdb_storage_version_log_tombstone.cc` implements
  `MoveTableToTombstone` / `RestoreTableFromTombstone`
  (`.tombstones/{table_id}/{deleted_ms}/`, newest-wins when `deleted_ms` is 0).
- REST `datasets.undelete` (`POST .../datasets/{id}:undelete`) is routed but
  returns 501 (`gateway/handlers/datasets.go` ~321–331: "engine has no
  undelete RPC yet").
- `ResolvedUndropStmt` is dispatched to `RunUndrop` from
  `control_op_executor.cc` ~228, so routing is done; only the behavior is
  missing.

## Done-criteria

1. `DROP SCHEMA ds` followed by `UNDROP SCHEMA ds` restores the dataset with
   all its tables, schemas, and rows.
2. `UNDROP SCHEMA IF NOT EXISTS` semantics match BigQuery (no error when the
   target already exists).
3. Undropping a dataset that was never dropped (or whose tombstone is past
   the retention window) surfaces a BigQuery-shaped `NOT_FOUND`.
4. REST `datasets.undelete` returns the restored Dataset resource instead
   of 501.
5. `UNDROP TABLE` still rejects (it must NOT start working — bq rejects it).

## Implementation steps

### Step 1 — dataset-scope tombstone

In `backend/storage/duckdb/`, extend the version-log tombstone module with
`MoveDatasetToTombstone` / `RestoreDatasetFromTombstone`. Reuse the per-table
move for each table in the dataset and additionally snapshot the dataset-level
catalog row (dataset metadata) so restore recreates the dataset entry first,
then restores member tables. Store under
`.tombstones/__dataset__/{project.dataset}/{deleted_ms}/`.

Hook the move into the storage-layer dataset drop path (the control-op
`DROP SCHEMA` handler) so that from now on drops are soft at the storage
level. Add a bounded retention sweep (mirror whatever the table tombstones do;
if they have no sweep, defer retention to out-of-scope).

### Step 2 — `RunUndrop` implementation

In `control_op_time_travel.cc`, branch on `stmt->schema_object_kind()`:
`"SCHEMA"` resolves the name path to a project+dataset id and calls
`RestoreDatasetFromTombstone` (newest tombstone). Honor
`stmt->is_if_not_exists()`. Anything else keeps today's `UNIMPLEMENTED`.
Populate `Job.statistics.query.statementType` per the control-op envelope
conventions.

### Step 3 — Catalog RPC + REST wiring

Add an `UndeleteDataset` RPC to the internal `Catalog` service
(`proto/emulator.proto`), implement it in `frontend/handlers/catalog.{h,cc}`
by delegating to the same storage restore, regenerate Go bindings, and replace
the 501 body in `DatasetUndelete` (`gateway/handlers/datasets.go`) with the
RPC call + Dataset resource response. `datasets.undelete` and `UNDROP SCHEMA`
must share one restore implementation.

### Step 4 — registries + docs

- `node_dispositions.yaml`: `ResolvedUndropStmt` → `control_op` (landed).
- `SHAPE_TRACKER.md` row ~71, `docs/ENGINE_POLICY.md` row ~118, `ROADMAP.md`
  Planned-work row + §DML/DDL bullet: flip to landed.

## Tests

- Conformance: `conformance/fixtures/ddl/undrop_schema_round_trip.yaml`
  (create ds + table + rows → drop → undrop → query rows),
  `undrop_schema_not_found.yaml`, `undrop_schema_if_not_exists.yaml`.
- Unit: dataset tombstone move/restore in the storage tests next to the
  existing table-tombstone coverage.
- Gateway e2e: `datasets.undelete` REST round-trip
  (`gateway/e2e/`, mirroring `restart_durability_test.go` patterns).

## Out of scope

- Time-travel-precise undelete (BigQuery lets you undelete "as of" within the
  7-day window; we restore the newest tombstone only, documented).
- Retention sweeps beyond whatever the table tombstones already do.

## Touch list

`backend/storage/duckdb/duckdb_storage_version_log*.{h,cc}`,
`backend/engine/control/control_op_time_travel.cc`, `proto/emulator.proto`,
`frontend/handlers/catalog.{h,cc}`, `gateway/enginepb/`,
`gateway/handlers/datasets.go`, `conformance/fixtures/ddl/`,
`node_dispositions.yaml`, `SHAPE_TRACKER.md`, `docs/ENGINE_POLICY.md`,
`docs/REST_API.md`, `ROADMAP.md`.
