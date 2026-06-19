---
name: M2-01 — Dataset detail page
overview: Build the clickable dataset resource page with an Overview tab (Tables sub-tab live; Graphs/Models/Routines scaffolded), a Details tab rendering full Dataset info, an Unplanned Insights placeholder, and an action toolbar hosting Create Table / Copy / Delete entry points (modals themselves land in M3).
todos:
  - id: page-shell
    content: Add DatasetPage with breadcrumbs, action toolbar, and Overview/Details/Insights tabs
    status: pending
  - id: overview
    content: Overview tab with Tables/Graphs/Models/Routines sub-tabs (Tables + Routines list; Graphs/Models Unplanned)
    status: pending
  - id: details
    content: Details tab rendering full Dataset info via datasetMetadata mapper
    status: pending
  - id: toolbar-stubs
    content: Wire Create Table / Copy / Delete toolbar buttons to open handlers (modals stubbed until M3)
    status: pending
  - id: e2e
    content: Add Playwright coverage for opening a dataset and viewing Details
    status: pending
isProject: false
---

# M2-01: Dataset detail page

## Dependencies

- M1 plans (api/types, primitives, routing/workspace).
- Index: [`00-index.plan.md`](00-index.plan.md)
- Roadmap: [`ROADMAP.md`](../../ROADMAP.md) → Feature areas → Datasets.

## Goal

Clicking a dataset in the sidebar opens a dataset resource tab (route `/project/:p/dataset/:d`) with console-style tabs and action toolbar.

## Scope

### Page shell

- `DatasetPage` rendered for the dataset route, inside the workspace shell.
- `Breadcrumbs`: `{project} / Datasets / {dataset}`.
- `ActionToolbar` with: **Create Table**, **Copy**, **Delete**, **Refresh** (use `ToolbarButton` from m1-02).
- Tab bar (resource tabs): `Overview`, `Details`, `Insights`.

### Overview tab

Sub-tabs:

- **Tables** (live): list tables/views in the dataset using `explorerQueries.tables` (and resource type from metadata where cheap). Each row links to the table page (M2-02). Columns like Table ID, Type, Created.
- **Routines** (live-lite): list via `explorerQueries.routines`; rows link to routine detail (full detail in M5). If endpoint unsupported, show empty/error state and note upstream.
- **Graphs** — `UnplannedTab` placeholder.
- **Models** — `UnplannedTab` placeholder.

### Details tab

`DetailTable` under a "Dataset info" `SectionHeading` using `explorerQueries.datasetMetadata`:

- Dataset ID, Created, Default table expiration, Last modified, Data location, Description, Default collation, Default rounding mode, Time travel window, Case insensitive, Labels (chips), Tags (chips), Replicas (table/section).

Render every field even if the emulator returns null/empty; missing data → `—` and an upstream note (do not hide the row).

### Insights tab

- `UnplannedTab` placeholder.

### Toolbar wiring

- Create Table / Copy / Delete buttons call handlers that (for now) open a stub modal or no-op with a TODO referencing M3. Keep the buttons visible and enabled so M3 only swaps in modal bodies.

## Out of scope

- Create Table / Copy / Delete modal implementations and job submission (M3-01, M3-02).
- Routine detail view and creation (M5).

## Verification

```bash
npm run build
npm run lint
npm run test:e2e
```

Manual: open a dataset from the tree; Overview lists tables; Details shows dataset info; Insights shows placeholder.

## Done criteria

- Dataset route renders Overview/Details/Insights with breadcrumbs and toolbar.
- Tables sub-tab lists and links to tables; Routines lists (or shows graceful empty/error).
- Details shows the full Dataset info field set.
- Graphs/Models/Insights show Unplanned placeholders.
