# 07 — Dataset & table metadata completeness (+ delete dataset)

- **UI gaps:** #1 (dataset metadata), #2 (table metadata richness), #9 (delete
  dataset — already works) (priority **P7**)
- **UI features:** Dataset Details tab and Table Details tab ("Table info",
  "Storage info", view query section) render every BigQuery-console field, even
  when empty. Missing fields render `—`.
- **Verified state at HEAD (`d390572`):** Partial for #1/#2; #9 works.

## Shared mechanism

REST-only fields persist in the in-memory `MetadataStore`
(`gateway/handlers/metadata_store.go`); engine-owned fields (schema, row counts)
come from the gRPC catalog. GET merges engine schema + overlay +
`MergeSchemaPolicyTags`. Adding a field generally means: (1) add it to the wire
type in `gateway/bqtypes/types.go`, (2) persist it in the overlay
(`stripEngineOwned*Fields` + the merge helpers), or compute it from the engine.

---

## Gap 1 — Dataset metadata (`GET`/`INSERT`/`PATCH`/`PUT /datasets/{d}`)

Handlers: `gateway/handlers/datasets.go` (`DatasetInsert` ~149, `DatasetGet`
~199, `DatasetPatch` ~258, `DatasetUpdate` ~229). Types:
`gateway/bqtypes/types.go` ~68–98. Overlay: `metadata_store.go`
(`stripEngineOwnedDatasetFields` ~196–207, `applyDatasetMetadataOverlay`
~324–350).

| Field | State | Action |
|-------|-------|--------|
| `friendlyName`, `description`, `location` | ✅ works | none |
| `labels` | ✅ works (PATCH delete semantics tested) | none |
| `defaultCollation` | ✅ works (round-trip tested) | none |
| `defaultTableExpirationMs`, `defaultPartitionExpirationMs` | ✅ works | add explicit test for the former |
| `access` | ✅ works | none |
| `creationTime` | ⚠️ synthesized fresh each GET (not persisted) | persist in overlay |
| `lastModifiedTime` | ⚠️ set to now on every response | persist; bump on write only |
| `defaultRoundingMode` | ❌ missing | add field + overlay |
| `maxTimeTravelHours` | ❌ missing | add field + overlay |
| `isCaseInsensitive` | ❌ missing | add field + overlay |
| `tags` | ❌ missing | add field + overlay |
| `replicas[]` | ❌ missing | add field + overlay (static/echo) |

**Steps:**
1. Add `defaultRoundingMode`, `maxTimeTravelHours`, `isCaseInsensitive`, `tags`
   (map), `replicas[]` to `bqtypes.Dataset` with correct JSON tags +
   explicit-set semantics where empty-vs-unset matters (mirror
   `defaultCollation`).
2. Persist them in `stripEngineOwnedDatasetFields` /
   `applyDatasetMetadataOverlay` / `mergeDatasetMetadataOverlay`.
3. Persist `creationTime` once (on insert) and `lastModifiedTime` on each write;
   stop synthesizing them per GET (`datasetResource` ~62–65).
4. Consider the gRPC v2 path `gateway/handlers/bqv2grpc/convert.go` (~44–60)
   which currently drops `defaultCollation` / `defaultTableExpirationMs` /
   `access` — align if non-REST clients matter.

---

## Gap 2 — Table metadata richness (`GET /tables/{id}`)

Handler: `gateway/handlers/tables.go` `TableGet` (~340–398). Types:
`gateway/bqtypes/types.go` ~176–243, 439–481. Tests:
`tables_metadata_get_test.go`.

### Identity / typing

| Field | State | Action |
|-------|-------|--------|
| `type` TABLE/VIEW/MATERIALIZED_VIEW/EXTERNAL | ✅ works (overlay) | none (DDL view typing is plan 01) |
| `view.query` | ✅ works (insert/overlay) | DDL path is plan 01 |
| `view.useLegacySql` | ❌ missing | add to `ViewDefinition` (~439–441) |
| `materializedView.query` | ✅ on insert/patch overlay | none |
| `SNAPSHOT` table type | ❌ no constant | decide: type vs decorator-only (coordinate plan 04) |

### Table fields

| Field | State | Action |
|-------|-------|--------|
| `location`, `expirationTime`, `labels`, `defaultCollation` | ✅ works | none |
| `friendlyName`, `description`, partitioning, clustering, encryption, external config | ✅ works (overlay) | none |
| `defaultRoundingMode` | ❌ missing | add field + overlay |
| `caseInsensitive` | ❌ missing | add field + overlay |
| `tags` | ❌ missing | add field + overlay |
| `tableConstraints.primaryKey.columns` | ❌ missing | add struct + overlay |

### Storage block

| Field | State | Action |
|-------|-------|--------|
| `numRows` | ✅ computed (`Catalog.ListRows` total, ~384–396) | none |
| `numBytes` | ❌ field exists, never set | compute/estimate |
| `numLongTermBytes`, `numActiveLogicalBytes`, `numTotalLogicalBytes`, `numCurrentPhysicalBytes`, `numPhysicalBytes`, `numActivePhysicalBytes`, `numLongTermPhysicalBytes`, `numTimeTravelPhysicalBytes` | ❌ missing | add fields; populate from engine or stub explicit `0` |

**Steps:**
1. Extend `bqtypes.Table` / `ViewDefinition` with the missing fields:
   `view.useLegacySql`, `defaultRoundingMode`, `caseInsensitive`, `tags`,
   `tableConstraints`, and the storage-byte fields.
2. Persist REST-only fields (`defaultRoundingMode`, `caseInsensitive`, `tags`,
   `tableConstraints`, `view.useLegacySql`) in the table overlay
   (`stripEngineOwnedTableFields` + merge).
3. Populate storage stats in `TableGet`: at minimum compute `numBytes` from
   stored Parquet size / a sum over column sizes; for logical/physical breakdown,
   either add an engine RPC that returns byte stats or **explicitly stub `0`**
   (so the field is present and the UI shows `0` instead of `—`). Document which
   are real vs stubbed.
4. SNAPSHOT typing: align with plan 04 — if snapshot tables are first-class,
   add the `type` constant; otherwise document the decorator-only model.

---

## Gap 9 — Delete dataset (already works — verify only)

`DELETE /datasets/{d}?deleteContents=true` is implemented: route
`gateway/server.go` ~93, handler `DatasetDelete` (`datasets.go` ~286–316),
`deleteContents` parsed (~300) and forwarded to `Catalog.DropDataset`; metadata
cascade via `Metadata.DeleteDataset` + `DeleteTablesInDataset`. Tested in
`datasets_test.go` (~150–174, 246–268) and `datasets_isolation_test.go`
(~134–161). Without `deleteContents=true`, a non-empty dataset → 400.

**Action:** add an e2e that creates a dataset with a table, `DELETE
?deleteContents=true`, then asserts `tables.list`/`datasets.get` 404 — to lock
behavior. `datasets.undelete` is still 501 (`datasets.go` ~325–327); implement
only if the UI requires it (out of scope otherwise).

## Tests

- Dataset round-trip (mirror `runDatasetMetadataRoundTrip`) for each new field;
  `creationTime` stable across GETs; `lastModifiedTime` bumps on write.
- Table GET: new fields round-trip via PATCH/insert; storage block present
  (real or explicit `0`); `view.useLegacySql` default `false`.
- Delete-dataset e2e (Gap 9).

## Out of scope

- Real cross-region replica behavior (`replicas[]` is echo/static).
- True physical-byte accounting if the engine can't provide it — stub `0` and
  document.

## Touch list

`gateway/bqtypes/types.go`, `gateway/handlers/metadata_store.go`,
`gateway/handlers/datasets.go`, `gateway/handlers/tables.go`,
`gateway/handlers/tables_schema.go`, optionally
`gateway/handlers/bqv2grpc/convert.go` and an engine byte-stats RPC.
