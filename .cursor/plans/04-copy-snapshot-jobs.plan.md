---
name: Copy and snapshot jobs
overview: Copy and snapshot modals work via configuration.copy jobs including SNAPSHOT operationType and destination expiration metadata.
todos:
  - id: e2e-copy-regression
    content: gateway/e2e — basic copy job DONE + destination rows match source
    status: completed
  - id: e2e-snapshot-regression
    content: gateway/e2e — SNAPSHOT operationType, destination type SNAPSHOT, expirationTime on tables.get
    status: completed
  - id: live-table-time-travel
    content: Optional — SNAPSHOT from live table @epoch via FOR SYSTEM_TIME (deleted-table path works)
    status: cancelled
isProject: false
---

# 04 — Copy / snapshot / restore jobs (configuration.copy)

- **UI gaps:** #6 (copy table), #7 (snapshot), #8 (copy dataset orchestration)
- **Priority:** **P4** → **verify-only** for basic copy + snapshot at HEAD
- **Verified state at HEAD (`60d19b3e`):** Copy + SNAPSHOT **pass**; optional live time-travel polish.

## Current state at HEAD (grounded)

Dispatch: `gateway/handlers/jobs.go` (~161–162) routes `cfg.Copy != nil` →
`runSyncCopyInsert` → `gateway/handlers/jobs_upload.go` (~42–61) → `copy.Execute`
(`gateway/copy/executor.go` ~34–89). Wire type `JobConfigurationCopy`
(`gateway/jobs/registry.go` ~232–240). Tests: `jobs_copy_extract_test.go`
(~19–169).

| Feature | State | Anchor |
|---------|-------|--------|
| `configuration.copy` jobs.insert (sync, DONE) | ✅ | `jobs.go` ~161 |
| `sourceTable` / `sourceTables[]` | ✅ | `copy/executor.go` |
| `writeDisposition` (EMPTY/TRUNCATE/APPEND) | ✅ | `executor.go` ~56–63, 337–366 |
| `createDisposition` (CREATE_IF_NEEDED/NEVER) | ✅ | `executor.go` ~66–68, 91–103 |
| `tableId@epochMs` on **deleted** tables | ✅ | `snapshots/store.go` ~72–149; test ~120–169 |
| `tableId@epoch` on **live** tables | ⚠️ | SQL path only (`FOR SYSTEM_TIME AS OF`), no native storage time-travel (`executor.go` ~168–190) |
| `operationType` COPY / SNAPSHOT / RESTORE | ✅ SNAPSHOT verified | `jobs/registry.go` |
| `destinationExpirationTime` | ✅ on job + destination table | verified on `:9050` |
| destination `type: SNAPSHOT` | ✅ | `tables.get` |
| copy entire dataset | UI-only | per-table copy works |

## Goal / done-criteria (UI-observable)

1. Copy table (modal): `POST .../jobs` with `configuration.copy` → poll
   `GET .../jobs/{id}` → `state: "DONE"` → destination has source schema + rows.
   (Works today — add coverage.)
2. Snapshot (modal): `configuration.copy.operationType = "SNAPSHOT"` is accepted,
   creates a destination snapshot of the source, honors
   `destinationExpirationTime`, and supports time-travel source refs
   `{tableId}@{snapshotTimeOffsetMs}`.
3. RESTORE: `operationType = "RESTORE"` recreates a table from a snapshot.
4. Copy dataset: per-table copy jobs succeed; destination dataset auto-created by
   the UI via `POST .../datasets` (verify the dataset-missing path returns the
   error/behavior the UI expects).

## Implementation steps

### Step 1 — Decode `operationType` + `destinationExpirationTime`

`gateway/jobs/registry.go` `JobConfigurationCopy` (~232–240): add
`OperationType string` (`COPY` default, `SNAPSHOT`, `RESTORE`) and
`DestinationExpirationTime string`. Ensure JSON tags match BigQuery.

### Step 2 — Branch on operationType in the executor

`gateway/copy/executor.go` `Execute`:
- `COPY` (default / empty) — current behavior.
- `SNAPSHOT` — copy source rows+schema into destination as a point-in-time
  snapshot; if the source ref carries `@epoch`, resolve via `snapshots.Store`
  (deleted-table path already exists) or `FOR SYSTEM_TIME AS OF` (live tables).
  Mark/track the destination as a snapshot if the table-type model supports it
  (coordinate with Gap 2 SNAPSHOT typing).
- `RESTORE` — create destination from a snapshot source (inverse of SNAPSHOT).

### Step 3 — Apply `destinationExpirationTime`

After a successful copy/snapshot, stash `expirationTime` on the destination via
`Metadata.MergeTable` in `finalizeSuccessfulCopyJob`
(`gateway/handlers/jobs_upload.go`). This surfaces in `tables.get`
(coordinate with Gap 2 `expirationTime`, which already round-trips).

### Step 4 — Time-travel source refs

`snapshots/store.go` already parses `table@123` (absolute ms) and `table@-3600000`
(relative offset) for deleted tables. For SNAPSHOT/live tables, route through the
SQL `FOR SYSTEM_TIME AS OF TIMESTAMP_MILLIS(epoch)` path
(`copy/executor.go` ~168–190). Document the live-vs-deleted distinction.

### Step 5 — Copy dataset (verify, likely no server change)

Confirm per-table copy works and the UI's destination-dataset-create path
behaves. If the UI expects a single dataset-copy job type, that is **not** a
standard BigQuery job — keep it UI-orchestrated and document it.

## Tests

- `gateway/handlers/jobs_copy_extract_test.go`: add `operationType=SNAPSHOT`
  (with `destinationExpirationTime`) and `operationType=RESTORE` cases;
  time-travel ref on a live table.
- `gateway/e2e`: copy table DONE → destination rows; snapshot job → destination
  exists with expiration; multi-table dataset copy.

## Out of scope

- Cross-region copy semantics, encryption-key enforcement beyond accepting the
  field.
- Async/long-running copy job modeling beyond the current sync-DONE model.

## Touch list

`gateway/jobs/registry.go`, `gateway/copy/executor.go`,
`gateway/handlers/jobs_upload.go`, `gateway/snapshots/store.go`.
