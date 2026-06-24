---
name: Dataset and table metadata
overview: Dataset and table Details tabs receive full BigQuery metadata fields including resourceTags, replicas, storage stats, and view/MV typing.
todos:
  - id: table-view-useLegacySql
    content: Add view.useLegacySql to ViewDefinition with default false on GET
    status: completed
  - id: table-resource-tags-constraints
    content: Round-trip table resourceTags and tableConstraints via metadata overlay
    status: completed
  - id: storage-byte-stats
    content: Populate numBytes breakdown (real or explicit 0) on tables.get
    status: completed
  - id: dataset-insert-timestamps
    content: Persist creationTime on dataset insert; bump lastModifiedTime only on writes
    status: completed
  - id: e2e-delete-dataset
    content: e2e DELETE dataset ?deleteContents=true cascade (gap 8 verify)
    status: completed
isProject: false
---

# 07 — Dataset & table metadata completeness (+ delete dataset)

- **UI gaps:** #1 (dataset metadata), #2 (table metadata), #8/#9 (delete — works), #12 (replicas)
- **Priority:** **P7**
- **Verified state at HEAD (`60d19b3e`):** Partial for #1/#2; #8/#12 **pass**.

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
| `labels` | ✅ works | none |
| `defaultCollation` | ✅ works | none |
| `defaultRoundingMode` | ✅ works | none |
| `maxTimeTravelHours` | ✅ works (string wire type) | none |
| `isCaseInsensitive` | ✅ works | none |
| `resourceTags` | ✅ works | UI brief “tags” = wire `resourceTags` |
| `replicas[]` | ✅ echo on PATCH/GET | verify-only (static) |
| `defaultTableExpirationMs`, `defaultPartitionExpirationMs` | ✅ works | none |
| `access` | ✅ works | none |
| `creationTime` | ⚠️ stable on GET; fresh on insert | persist on insert |
| `lastModifiedTime` | ⚠️ bumps on write | verify insert path |

**Steps (remaining):**
1. Persist `creationTime` on dataset insert; ensure `lastModifiedTime` only bumps on writes.
2. Table-level gaps (Gap 2 below) — `view.useLegacySql`, `resourceTags`, `tableConstraints`, byte stats.

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
| `numRows` | ✅ computed | none |
| `numBytes`, `numLongTermBytes` | ✅ present (often `0`) | improve accuracy or document stub |
| `numActiveLogicalBytes`, physical breakdown | ❌ missing | add fields; real or explicit `0` |
| `view.useLegacySql` | ❌ missing | add + overlay |
| `resourceTags`, `tableConstraints` | ❌ missing | add + overlay |
| `defaultRoundingMode`, `caseInsensitive` | ❌ missing | add + overlay |

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
