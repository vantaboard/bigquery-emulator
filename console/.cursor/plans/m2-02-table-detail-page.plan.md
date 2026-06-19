---
name: M2-02 — Table / view / materialized view detail page
overview: Build the clickable table/view resource page with a read-only Schema tab (field grid + Copy as Table/JSON), a Details tab (Table/View info + Storage info + View query), a paginated Preview tab, an action toolbar (Query/Copy/Snapshot/Delete), and Unplanned placeholders for Table Explorer/Insights/Lineage/Data Profile/Data Quality. Resource type drives conditional UI.
todos:
  - id: page-shell
    content: Add TablePage with breadcrumbs, action toolbar, and the full tab set; branch on resourceType
    status: pending
  - id: schema-tab
    content: Schema tab field grid with filter, selection, and Copy as Table / Copy as JSON (read-only; edit in M3)
    status: pending
  - id: details-tab
    content: Details tab with conditional Table/View info, Storage info (empty for views/MV), and View query section
    status: pending
  - id: preview-tab
    content: Preview tab using tableData pagination with SELECT-LIMIT fallback
    status: pending
  - id: query-button
    content: Query button opens a new persisted query tab with SELECT * FROM fqn LIMIT 1000
    status: pending
  - id: unplanned
    content: Add Unplanned placeholders for Table Explorer/Insights/Lineage/Data Profile/Data Quality
    status: pending
  - id: e2e
    content: Add Playwright coverage for schema view, details, preview, and Query button
    status: pending
isProject: false
---

# M2-02: Table / view / materialized view detail page

## Dependencies

- M1 plans (api/types, primitives, routing/workspace).
- Index: [`00-index.plan.md`](00-index.plan.md)
- Roadmap: [`ROADMAP.md`](../../ROADMAP.md) → Feature areas → Tables / Views / Materialized views.

## Goal

Clicking a table in the sidebar opens a resource tab (route `/project/:p/dataset/:d/table/:t`) whose UI adapts to `resourceType` (TABLE / VIEW / MATERIALIZED_VIEW / SNAPSHOT).

## Scope

### Page shell

- `TablePage` with `Breadcrumbs` (`{project} / Datasets / {dataset} / Tables / {table}`).
- `ActionToolbar`: **Query** (all), **Copy** (all), **Snapshot** (tables only), **Delete** (all), **Refresh**.
- Tab bar: `Schema`, `Details`, `Preview`, `Table Explorer`, `Insights`, `Lineage`, `Data Profile`, `Data Quality`.

### Schema tab (read-only here)

- Field grid: Field name, Type, Mode, Description, Key, Collation, Default Value, Policy Tags, Data Policies (columns present even when empty).
- Filter bar for field name/value.
- Row checkboxes with a **Copy** dropdown: **Copy as Table** (TSV/grid) and **Copy as JSON** (BQ schema JSON) for selected or all fields (use `CopyButton`).
- **Edit schema** and **View row access policies** buttons present; Edit schema is wired in M3-02 (here it can be disabled/stub).
- Source: `explorerQueries.tableSchema` (extended in M1-01).

### Details tab

- "Table info" / "View info" `DetailTable`, conditional on `resourceType`:
  - Common: Table ID/View ID, Created, Last modified, Table/View expiration, Description, Labels, Primary key(s), Tags.
  - Tables only: Data location, Default collation, Default rounding mode, Case insensitive.
  - Views/MV only: Use Legacy SQL.
- "Storage info" `DetailTable`: numRows + logical/physical/time-travel byte stats. Empty (with note) for views and materialized views.
- "Query" section (views/MV only): render the underlying defining SQL (read-only code block; Edit Query button stubbed for M4).

### Preview tab

- Paginated table using `explorerQueries.tableData` (`tabledata.list`) with results-per-page control and page tokens.
- Fallback: if `tabledata.list` is unsupported, run `SELECT * FROM \`fqn\` LIMIT n` and note upstream work. Reuse `ResultsTable` rendering.

### Query button

- Opens a new persisted query tab (m1-03 `openQueryTab`) pre-filled with `SELECT * FROM \`project.dataset.table\` LIMIT 1000` (reuse/relocate `defaultSql` from [ExplorerPage.tsx](../../src/features/explorer/ExplorerPage.tsx)).

### Unplanned tabs

- Table Explorer, Insights, Lineage, Data Profile, Data Quality → `UnplannedTab` placeholders.

## Out of scope

- Schema editing, Copy/Snapshot/Delete modal bodies + jobs (M3).
- Edit Query / save-as-view (M4).

## Verification

```bash
npm run build
npm run lint
npm run test:e2e
```

Manual: open a table → Schema lists fields, Copy as JSON works; Details shows info + storage; Preview paginates; Query opens a new tab. Open a view → no Storage info, View query shown, no Snapshot button.

## Done criteria

- Table/view route renders all listed tabs with correct conditional fields by resource type.
- Schema copy-as-table/JSON works; Preview paginates (or falls back); Query opens a new persisted tab.
- Unplanned tabs render placeholders.
