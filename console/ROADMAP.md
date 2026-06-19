# Roadmap

This document charts the evolution of **BigQuery Emulator UI** from its current single-page explorer into a BigQuery web console–style experience. The visual target is the Google Cloud BigQuery console: breadcrumbs, resource detail pages with tabs, action toolbars, a multi-tab query workspace, and modals for create/copy/snapshot flows.

The UI talks to the BigQuery REST API (`/bigquery/v2/*`) served by [bigquery-emulator](https://github.com/vantaboard/bigquery-emulator) (or real BigQuery). Every feature is built to the real BigQuery contract. Where the emulator lacks support today, the gap is tracked as **upstream emulator work** — not a reason to descope UI features.

## Status legend

| Status | Meaning |
|--------|---------|
| **Done** | Shipped in the current codebase |
| **In progress** | Actively being built |
| **Planned** | Scoped and scheduled |
| **Unplanned** | Recognized in the console but not scheduled for this project |

---

## Current state

What ships today (v2.0.0):

- **Single route** `/` → `ExplorerPage` ([src/app/router.tsx](src/app/router.tsx))
- **Monolithic layout**: collapsible resource tree sidebar, one SQL editor, and a 3-tab panel (`Table info` / `Results` / `JSON`) in [src/features/explorer/ExplorerPage.tsx](src/features/explorer/ExplorerPage.tsx)
- **Thin REST client** covering only:
  - `GET /bigquery/v2/projects`
  - `GET /bigquery/v2/projects/{projectId}/datasets`
  - `GET /bigquery/v2/projects/{projectId}/datasets/{datasetId}/tables`
  - `GET /bigquery/v2/projects/{projectId}/datasets/{datasetId}/tables/{tableId}`
  - `POST /bigquery/v2/projects/{projectId}/queries`
  - See [src/features/explorer/api.ts](src/features/explorer/api.ts), [src/lib/bqRest.ts](src/lib/bqRest.ts), [src/lib/api.ts](src/lib/api.ts)
- **Limited types**: `TableMetadata` and `QueryResponse` only ([src/types/api.ts](src/types/api.ts)) — no dataset metadata, view/materialized-view distinction, routines, jobs, or storage stats
- **URL state**: encodes project, dataset, table, results tab, and base64 query ([src/features/explorer/urlState.ts](src/features/explorer/urlState.ts))
- **Sidebar/layout prefs**: persisted to `localStorage` under `bigqueryExplorerUILayout`
- **Query formatting**: BigQuery dialect via `sql-formatter`
- **SQL editor**: CodeMirror with basic SQL highlighting ([src/features/explorer/components/SqlEditor.tsx](src/features/explorer/components/SqlEditor.tsx)) — no parser-based auto-completion yet

What is **not** implemented:

- Breadcrumbs
- Clickable dataset or table/view resource pages (datasets only expand in the tree)
- Console-style tabs (Schema, Details, Preview, etc.)
- Action toolbars (Query, Copy, Snapshot, Delete, Create Table)
- Multi-tab workspace (multiple open queries or resources)
- Browser-persisted workspace session (beyond layout prefs)
- Saved queries / save-as-view
- Routines (UDFs)
- Schema editing, copy-as-table/JSON
- Any mutation (create, copy, delete, snapshot)

---

## Architectural prerequisites

Cross-cutting work that most feature areas depend on.

### Resource routing

**Status: Planned**

Move from a single `/` route to console-style routing, e.g.:

- `/project/:projectId/dataset/:datasetId` — dataset resource page
- `/project/:projectId/dataset/:datasetId/table/:tableId` — table/view resource page

Replace the single-page model in [src/app/router.tsx](src/app/router.tsx) and [src/features/explorer/ExplorerPage.tsx](src/features/explorer/ExplorerPage.tsx) with a tabbed shell that hosts resource pages and query editors.

### Multi-tab workspace surface

**Status: Planned**

A persistent workspace tab bar holds many simultaneously-open tabs:

- Query editor tabs (e.g. `*Untitled query`, named saved queries)
- Resource tabs (datasets, tables, views)

Per-tab actions: open (sidebar click or resource **Query** button), activate, rename, reorder, close; new blank query tab via a **+** control.

Each query tab owns its own SQL text, results/JSON sub-tab selection, and last query results. Switching tabs preserves all state.

### Browser-persisted session state

**Status: Planned**

The full workspace session survives page reload, stored in `localStorage`:

- Set of open tabs and their order
- Active tab
- Per query tab: SQL text, results sub-tab, last results (optional cache)
- Sidebar/layout preferences (extends existing `bigqueryExplorerUILayout` pattern)

Deep-link URL state ([src/features/explorer/urlState.ts](src/features/explorer/urlState.ts)) remains the **shareable subset** for the active tab; the browser store holds the full multi-tab session.

### Expanded API layer and types

**Status: Planned**

Extend [src/features/explorer/api.ts](src/features/explorer/api.ts), [src/lib/bqRest.ts](src/lib/bqRest.ts), and [src/types/api.ts](src/types/api.ts) to cover:

- Dataset metadata (`GET /datasets/{datasetId}`)
- Table vs view vs materialized view vs snapshot typing
- Routines list and detail
- Jobs (copy, snapshot, load)
- Storage byte stats
- Schema update (`PATCH` tables)
- Table data preview (`tabledata.list`)

### Reusable UI primitives

**Status: Planned**

Shared components needed across feature areas:

- Modal / dialog (Create Table, Copy, Snapshot, Edit Schema)
- Tab bar (resource tabs and workspace tabs)
- Key-value detail tables (Details tab sections)
- Action toolbar (Query, Copy, Snapshot, Delete, etc.)
- Copy-to-clipboard helpers
- **Unplanned** tab placeholder component

---

## Feature areas

### Breadcrumbs

**Status: Planned**

Console-style clickable path navigation:

- Dataset page: `{project} / Datasets / {dataset}`
- Table/view page: `{project} / Datasets / {dataset} / Tables / {table}`

Each segment navigates to the corresponding resource. Implemented as a shared `Breadcrumbs` component fed by route params.

| Item | Today | Target | REST |
|------|-------|--------|------|
| Breadcrumb trail | None | Clickable path on every resource page | Route params only |

---

### Datasets (clickable resource page)

**Status: Planned**

Clicking a dataset in the sidebar opens a dataset resource tab (instead of only expanding the tree).

#### Overview tab

**Status: Planned**

Sub-tabs listing dataset contents:

| Sub-tab | Status | Description | REST |
|---------|--------|-------------|------|
| **Tables** | Planned | List tables and views in the dataset | `GET .../datasets/{datasetId}/tables` (exists) |
| **Graphs** | Unplanned | Placeholder — console parity only | — |
| **Models** | Unplanned | Placeholder — console parity only | — |
| **Routines** | Planned | List UDFs and procedures | `GET .../datasets/{datasetId}/routines` |

#### Details tab

**Status: Planned**

**Dataset info** section with fields:

- Dataset ID
- Created
- Default table expiration
- Last modified
- Data location
- Description
- Default collation
- Default rounding mode
- Time travel window
- Case insensitive
- Labels
- Tags
- Replicas

Maps to `GET /bigquery/v2/projects/{projectId}/datasets/{datasetId}`. Build the full panel to the real BigQuery shape; any field the emulator omits is upstream emulator work.

#### Insights tab

**Status: Unplanned**

Placeholder tab — not scheduled.

#### Create Table button

**Status: Planned**

Opens a **Create table** modal.

**Source section** — **Create table from** dropdown:

| Source | Status |
|--------|--------|
| Empty table | Planned |
| Google Cloud Storage | Planned |
| Upload | Planned |
| Drive | Planned |
| Google Bigtable | Planned |
| Amazon S3 | Planned |
| Azure Blob Storage | Planned |
| Existing table/view | Planned |

Build all source options in the UI. Sources the emulator cannot ingest yet are tracked as upstream emulator work.

**Destination section:**

- Project
- Dataset
- Table (with name validation helper text)

**Schema section:**

- Visual schema builder: field name, type, mode, description
- Support RECORD nesting, REPEATED mode, RANGE types, max length (STRING/BYTES)
- **Edit as text** toggle for JSON schema definition
- Validation (required field names, nested fields)

**Partitioning and clustering:**

- Partition type (none, time, ingestion time, integer range)
- Partition field / expiration
- Clustering order (comma-separated fields)

**Tags:**

- Tag scope selector and key/value pairs

**Advanced options:**

- Encryption (Google-managed / Cloud KMS)
- Default collation
- Default rounding mode

**Upload-format options** (when source is Upload or external file):

- File format: CSV, JSONL, Avro, Parquet, ORC
- Format-specific options: write preference, errors allowed, field delimiter, quote character, header rows to skip, quoted newlines, jagged rows, null markers, custom timezone/date format strings

Maps to table insert / load jobs and DDL as appropriate.

#### Copy button

**Status: Planned**

Opens **Copy dataset** modal:

- **Source** (read-only): Project, Dataset, Location
- **Destination**: Dataset name, Overwrite destination tables checkbox

Maps to a BigQuery copy/transfer job.

#### Delete button

**Status: Planned**

Delete dataset with confirmation dialog.

Maps to `DELETE /bigquery/v2/projects/{projectId}/datasets/{datasetId}`.

---

### Tables / Views / Materialized views (clickable resource page)

**Status: Planned**

Clicking a table in the sidebar opens a table/view resource tab. Resource type (TABLE, VIEW, MATERIALIZED_VIEW, SNAPSHOT) drives conditional UI.

#### Action toolbar

| Button | Status | Visibility | Description |
|--------|--------|------------|-------------|
| **Query** | Planned | All | Opens a new query tab with `SELECT * FROM \`project.dataset.table\` LIMIT 1000` |
| **Copy** | Planned | All | Opens Copy table/view modal |
| **Snapshot** | Planned | Tables only | Opens Create table snapshot modal |
| **Delete** | Planned | All | Delete with confirmation |
| **Refresh** | Planned | All | Reload metadata |

#### Schema tab

**Status: Planned**

- Field grid: Field name, Type, Mode, Description, Key, Collation, Default Value, Policy Tags, Data Policies
- Filter bar for field search
- Row selection with bulk actions
- **Copy** dropdown: **Copy as Table**, **Copy as JSON** (selected fields or all)
- **Edit schema** button → modal to change modes (e.g. REQUIRED → NULLABLE), edit descriptions/defaults, add new fields
- **View row access policies** button (disabled placeholder until supported)

Maps to `GET` table metadata; schema edits map to `PATCH .../tables/{tableId}`.

#### Details tab

**Status: Planned**

**Table info / View info** section — fields vary by resource type:

| Field | Tables | Views | Materialized views |
|-------|--------|-------|-------------------|
| Table ID / View ID | Yes | Yes | Yes |
| Created | Yes | Yes | Yes |
| Last modified | Yes | Yes | Yes |
| Table expiration / View expiration | Yes | Yes | Yes |
| Data location | Yes | — | — |
| Default collation | Yes | — | — |
| Default rounding mode | Yes | — | — |
| Case insensitive | Yes | — | — |
| Use Legacy SQL | — | Yes | Yes |
| Description | Yes | Yes | Yes |
| Labels | Yes | Yes | Yes |
| Primary key(s) | Yes | Yes | Yes |
| Tags | Yes | Yes | Yes |

**Storage info** section:

- Empty for views and materialized views (with explanatory note)
- For tables: Number of rows, Total logical bytes, Active logical bytes, Long term logical bytes, Current physical bytes, Total physical bytes, Active physical bytes, Long term physical bytes, Time travel physical bytes

Render all stats; any the emulator does not report is upstream emulator work.

**Query** section (views and materialized views only):

- Display the underlying defining SQL
- **Edit Query** button (Planned)

#### Preview tab

**Status: Planned**

Paginated tabular preview of table data with configurable results per page.

Primary API: `GET .../tables/{tableId}/data` (`tabledata.list`). Fallback: `SELECT * FROM ... LIMIT n` query if `tabledata.list` is unavailable (upstream emulator work).

#### Table Explorer tab

**Status: Unplanned**

Placeholder — not scheduled.

#### Insights tab

**Status: Unplanned**

Placeholder — not scheduled.

#### Lineage tab

**Status: Unplanned**

Placeholder — not scheduled.

#### Data Profile tab

**Status: Unplanned**

Placeholder — not scheduled.

#### Data Quality tab

**Status: Unplanned**

Placeholder — not scheduled.

#### Copy button (modal)

**Status: Planned**

**Copy table** modal:

- **Source** (read-only): Project, Dataset, Table name
- **Destination**: Project, Dataset, Table
- **Advanced options**: Encryption (Google-managed / Cloud KMS)

Maps to copy job API.

#### Snapshot button (modal)

**Status: Planned**

**Create table snapshot** modal (tables only):

- **Source** (read-only): Project, Dataset, Table name
- **Destination**: Project, Dataset, Table (default name with timestamp), Expiration time, Snapshot time

Maps to snapshot/copy job API.

---

### Multi-tab workspace (persisted to the browser)

**Status: Planned**

The central workspace pattern for the console experience.

| Capability | Today | Target |
|------------|-------|--------|
| Open tabs | Single editor | Many query + resource tabs |
| Tab types | Table-bound query only | Query editors, dataset pages, table/view pages |
| Tab actions | — | Open, activate, rename, reorder, close, **+** new query |
| Per-tab state | Global SQL/results | Each query tab owns SQL, sub-tab, results |
| Persistence | Layout prefs only | Full session in `localStorage` |
| Shareable URL | Single table + query | Active tab deep link |

Session storage schema (conceptual):

```json
{
  "tabs": [
    { "id": "...", "type": "query", "title": "Untitled query", "sql": "...", "resultsTab": "results" },
    { "id": "...", "type": "table", "project": "...", "dataset": "...", "table": "...", "activeResourceTab": "schema" }
  ],
  "activeTabId": "...",
  "tabOrder": ["...", "..."]
}
```

Extends [src/features/explorer/urlState.ts](src/features/explorer/urlState.ts) and the `bigqueryExplorerUILayout` pattern in [ExplorerPage.tsx](src/features/explorer/ExplorerPage.tsx).

---

### Query workspace

**Status: Partially done → Planned**

Runs inside the multi-tab workspace.

| Capability | Status | Notes |
|------------|--------|-------|
| Run query | **Done** | `POST .../queries` |
| Format SQL | **Done** | `sql-formatter` BigQuery dialect |
| Syntax highlighting | **Done** | CodeMirror `@codemirror/lang-sql` |
| Parser-based auto-completion | Planned | Projects, datasets, tables, views, columns, UDFs |
| Real-time syntax errors | Planned | Parser diagnostics in editor gutter |
| Results table | **Done** | [ResultsTable.tsx](src/features/explorer/components/ResultsTable.tsx) |
| JSON results | **Done** | [JsonViewer.tsx](src/features/explorer/components/JsonViewer.tsx) |
| Reference panel | Planned | Side panel showing table schema while editing |
| Tools menu | Planned | Toggle parser completion, translation settings |

Auto-completion requires a fetched resource catalog (projects, datasets, tables, views, routines) and column metadata from schema endpoints, wired into CodeMirror completion in [SqlEditor.tsx](src/features/explorer/components/SqlEditor.tsx).

---

### Saving queries

**Status: Planned**

**Save** dropdown on query tabs:

| Option | Description | Storage |
|--------|-------------|---------|
| **Save query** | Versioned saved query (console-style) | Browser `localStorage` initially; upstream emulator work for server-backed saved queries |
| **Save query (Classic)** | Simple saved query without version history | Browser `localStorage` |
| **Save view** | Persist query as a view via `CREATE VIEW` DDL | BigQuery REST / query job |
| **Save as...** | Save query results to a table | Load job / `CREATE TABLE AS SELECT` |

---

### Routines (UDFs)

**Status: Planned**

User-defined functions and stored procedures as **Routines**:

- Listed under dataset **Overview → Routines** sub-tab
- Expandable in sidebar tree under each dataset (future)
- View routine definition (SQL body, arguments, return type)
- Create routine via SQL or modal

Maps to:

- `GET /bigquery/v2/projects/{projectId}/datasets/{datasetId}/routines`
- `GET .../routines/{routineId}`
- `CREATE FUNCTION` / `CREATE PROCEDURE` via query jobs

If unsupported by the emulator, upstream emulator work.

---

## Upstream emulator work

The UI targets the real BigQuery REST contract. Gaps in [bigquery-emulator](https://github.com/vantaboard/bigquery-emulator) are tracked here and addressed upstream — they do **not** descope UI features.

| Area | Expected REST / behavior | UI feature that needs it |
|------|---------------------------|--------------------------|
| Dataset metadata | `GET /datasets/{id}` with full metadata (labels, tags, replicas, collation, etc.) | Dataset Details tab |
| Table metadata | Full storage stats, view query text, materialized view type | Table Details tab |
| Schema update | `PATCH /tables/{id}` | Edit Schema modal |
| Table data | `GET /tables/{id}/data` (`tabledata.list`) | Preview tab |
| Table create | Insert table + load jobs, external sources | Create Table modal |
| Copy jobs | Copy table / copy dataset | Copy modals |
| Snapshot jobs | Table snapshot creation | Snapshot modal |
| Delete | `DELETE` datasets and tables | Delete buttons |
| Routines | List/get/create routines | Routines sub-tab, autocompletion |
| DDL / DML jobs | Query jobs for CREATE VIEW, CREATE FUNCTION, etc. | Save view, routines, schema changes |
| Saved queries | Server-backed saved query objects (optional) | Save query (versioned) |
| Replicas | Cross-region replica metadata | Dataset Details → Replicas |
| External ingestion | GCS, S3, Azure, Drive, Bigtable sources | Create Table modal sources |

When implementing a UI feature, file an issue or PR against the emulator for any missing behavior rather than hiding the UI control.

---

## Phasing / milestones

### M1 — Foundation

- Resource routing (dataset and table/view pages)
- Multi-tab workspace shell with browser persistence
- Breadcrumbs
- Expanded API layer and types
- Reusable UI primitives (modal, tab bar, detail tables, toolbar)

### M2 — Dataset and table detail pages

- Dataset Overview (Tables sub-tab) and Details tab
- Table/view Schema tab (read-only)
- Table/view Details tab (Table info, Storage info, View query section)
- Preview tab
- Query button (opens new persisted query tab)

### M3 — Mutations

- Create Table modal (all source options in UI)
- Copy dataset / Copy table modals
- Delete dataset / Delete table
- Create table snapshot modal
- Edit Schema modal

### M4 — Query workspace upgrades

- Parser-based auto-completion catalog
- Real-time syntax diagnostics
- Save as view / Save query / Save query (Classic)
- Reference side panel

### M5 — Routines and external sources

- Routines sub-tab and sidebar integration
- Routine autocompletion in SQL editor
- External source ingestion flows (GCS, S3, Azure, Drive, Bigtable)

### Upstream emulator work (parallel track)

File issues and PRs against [bigquery-emulator](https://github.com/vantaboard/bigquery-emulator) for any REST behavior the UI needs that the emulator lacks. Run in parallel with UI milestones — UI can stub or show errors gracefully until upstream lands.

---

## Out of scope (Unplanned)

These tabs and sub-tabs appear in the BigQuery console but are **not scheduled** for this project. Each will render an **Unplanned** placeholder when the surrounding page ships.

**Dataset level:**

- Insights tab
- Overview → Graphs sub-tab
- Overview → Models sub-tab

**Table/view level:**

- Table Explorer tab
- Insights tab
- Lineage tab
- Data Profile tab
- Data Quality tab

---

## Related docs

- [README.md](README.md) — setup and current features
- [docs/api-contract.md](docs/api-contract.md) — REST endpoints used today
- [docs/rollout-checklist.md](docs/rollout-checklist.md) — rollout checklist
