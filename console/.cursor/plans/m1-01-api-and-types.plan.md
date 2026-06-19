---
name: M1-01 — Expanded API layer and types
overview: Extend the thin REST client and type definitions to cover dataset metadata, resource typing (table/view/materialized view/snapshot), storage stats, routines, jobs (copy/snapshot/load), schema patch, and table-data preview, so later phases have typed data access. Foundation phase, no UI yet.
todos:
  - id: types
    content: Add DatasetMetadata, ResourceType, RoutineMetadata, StorageStats, JobRef, and view/MV fields to src/types/api.ts
    status: pending
  - id: bqrest-mappers
    content: Add BQ->UI mappers in src/lib/bqRest.ts for dataset metadata, resource type detection, storage stats, routines, view query text
    status: pending
  - id: api-queries
    content: Add explorerQueries methods for datasetMetadata, routines, tableData preview, and job submission (copy/snapshot/delete) in src/features/explorer/api.ts
    status: pending
  - id: unit-tests
    content: Add/extend unit tests for new mappers (mirroring urlState.test.ts location/style)
    status: pending
isProject: false
---

# M1-01: Expanded API layer and types

## Dependencies

- None. First foundation plan.
- Index: [`00-index.plan.md`](00-index.plan.md)
- Roadmap: [`ROADMAP.md`](../../ROADMAP.md) → Architectural prerequisites → "Expanded API layer and types".

## Goal

Give every later phase typed, tested access to the BigQuery REST surface beyond today's list/metadata/query. No UI changes in this plan.

## Current state

- [src/types/api.ts](../../src/types/api.ts): only `ExplorerConfig`, `TableSchemaField`, `TableMetadata`, `QueryResponse`.
- [src/lib/bqRest.ts](../../src/lib/bqRest.ts): mappers for project/dataset/table lists, `tableMetadataFromBq`, `queryResponseFromBq`.
- [src/features/explorer/api.ts](../../src/features/explorer/api.ts): `explorerQueries` with `projects`, `datasets`, `tables`, `tableSchema`, `runQuery`.
- [src/lib/api.ts](../../src/lib/api.ts): `ApiClient` with `get`/`post`; add `patch`/`delete` here.

## Scope

### 1. Types — [src/types/api.ts](../../src/types/api.ts)

- `ResourceType = 'TABLE' | 'VIEW' | 'MATERIALIZED_VIEW' | 'SNAPSHOT' | 'EXTERNAL'`.
- `DatasetMetadata`: id, friendlyName, description, location, creationTime, lastModifiedTime, defaultTableExpirationMs, defaultCollation, defaultRoundingMode, maxTimeTravelHours, isCaseInsensitive, labels (record), tags, replicas (array; may be empty).
- Extend `TableMetadata`: `resourceType: ResourceType`, `expirationTime`, `defaultCollation`, `defaultRoundingMode`, `caseInsensitive`, `useLegacySql` (views), `viewQuery` (views/MV), `labels`, `primaryKeys`, `tags`, and a `storage: StorageStats` block.
- `StorageStats`: numRows, totalLogicalBytes, activeLogicalBytes, longTermLogicalBytes, currentPhysicalBytes, totalPhysicalBytes, activePhysicalBytes, longTermPhysicalBytes, timeTravelPhysicalBytes.
- `RoutineMetadata`: id, routineType, language, definitionBody, arguments, returnType, creationTime, lastModifiedTime.
- `JobRef`: jobId, projectId, state, errorResult.
- `TableDataPage`: rows (reuse `QueryResponse`-style shape), pageToken, totalRows.

### 2. Mappers — [src/lib/bqRest.ts](../../src/lib/bqRest.ts)

- `datasetMetadataFromBq(projectId, datasetId, raw)`.
- `resourceTypeFromBq(raw)` from BQ `type` field (`TABLE`/`VIEW`/`MATERIALIZED_VIEW`/`SNAPSHOT`/`EXTERNAL`).
- Extend `tableMetadataFromBq` to populate new fields, including `view.query`, `materializedView.query`, `view.useLegacySql`, `tableConstraints.primaryKey.columns`, `labels`, and storage byte fields (`numLongTermBytes`, etc.).
- `routineFromBq` / `routineIdsFromList`.
- `tableDataFromBq(raw, schema)` for `tabledata.list` rows (reuse `parseBqValue`).
- Keep all functions pure and exported for unit tests.

### 3. API client — [src/lib/api.ts](../../src/lib/api.ts)

- Add `patch<T>(path, body)` and `del<T>(path)` to `ApiClient` (mirror existing `post`).

### 4. Query layer — [src/features/explorer/api.ts](../../src/features/explorer/api.ts)

Add to `explorerQueries`:

- `datasetMetadata(projectId, datasetId)` → `GET /bigquery/v2/projects/{p}/datasets/{d}`.
- `routines(projectId, datasetId)` and `routine(projectId, datasetId, routineId)` → `.../routines`.
- `tableData(projectId, datasetId, tableId, { maxResults, pageToken })` → `.../tables/{t}/data`.
- `patchTableSchema(projectId, datasetId, tableId, fields)` → `PATCH .../tables/{t}`.
- `deleteTable` / `deleteDataset` → `DELETE`.
- `submitJob(projectId, jobConfig)` → `POST .../jobs` and `getJob(projectId, jobId)` for copy/snapshot/load polling.

Each new method maps raw BQ JSON through the mappers above. Where the emulator may not support an endpoint, surface the error to the caller (UI handles gracefully) and log an entry in [`upstream-emulator-work.plan.md`](upstream-emulator-work.plan.md).

## Out of scope

- Any React components or routes (handled in m1-02/m1-03 and later).
- Actually wiring buttons/modals (M2/M3).

## Verification

```bash
npm run build   # tsc --noEmit + vite build
npm run lint
npm run test    # vitest, including new mapper tests
```

## Done criteria

- New types compile and are exported from `src/types/api.ts`.
- All new mappers have unit tests and pass.
- `explorerQueries` exposes dataset metadata, routines, table data, schema patch, delete, and job submit/poll.
- Existing flows (list, schema, runQuery) unchanged.
