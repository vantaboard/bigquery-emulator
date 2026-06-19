---
name: M4-01 — Query workspace upgrades
overview: Upgrade the query editor with parser-based auto-completion across projects/datasets/tables/views/columns/UDFs, real-time syntax diagnostics, a reference side panel, and saving queries (Save query, Save query Classic, Save view via DDL, Save as table). Builds on the multi-tab workspace from M1-03.
todos:
  - id: catalog
    content: Build a resource catalog provider (projects/datasets/tables/views/columns/routines) for completion
    status: pending
  - id: autocomplete
    content: Wire @codemirror/lang-sql schema completion + custom completion source into SqlEditor
    status: pending
  - id: diagnostics
    content: Add real-time syntax diagnostics (lint gutter) to the editor
    status: pending
  - id: reference-panel
    content: Add a reference side panel showing the active table/view schema while editing
    status: pending
  - id: saving
    content: Add Save dropdown — Save query, Save query (Classic), Save view (DDL), Save as table
    status: pending
  - id: e2e
    content: Add Playwright coverage for autocompletion suggestions and saving a view
    status: pending
isProject: false
---

# M4-01: Query workspace upgrades

## Dependencies

- M1 plans (especially [`m1-03-routing-workspace-breadcrumbs.plan.md`](m1-03-routing-workspace-breadcrumbs.plan.md) for multi-tab query tabs and [`m1-01-api-and-types.plan.md`](m1-01-api-and-types.plan.md) for catalog/job APIs).
- Index: [`00-index.plan.md`](00-index.plan.md)
- Roadmap: [`ROADMAP.md`](../../ROADMAP.md) → Query workspace + Saving queries.

## Goal

Make the editor console-grade: smart completion, inline errors, a schema reference panel, and the Save menu — all per query tab.

## Current state

- [SqlEditor.tsx](../../src/features/explorer/components/SqlEditor.tsx): CodeMirror with `sql()` extension, no schema completion, no diagnostics.
- Run + Format already implemented (carried into per-tab `QueryTab` by M1-03).
- `@codemirror/lang-sql` and `sql-formatter` already in [package.json](../../package.json).

## Scope

### Resource catalog

- Provider that fetches and caches projects → datasets → tables/views → columns, plus routines (UDFs), keyed for completion. Reuse `explorerQueries` list/schema/routines methods; lazy-load per project/dataset to limit calls.

### Auto-completion

- Configure `@codemirror/lang-sql` with a `schema` object (tables → columns) and a custom completion source for fully-qualified names `project.dataset.table` and UDF names.
- Trigger on `.` and identifier typing; include columns of referenced tables.

### Diagnostics

- Parser-based syntax error highlighting in the gutter/inline (CodeMirror lint). Mirror the "Syntax error" feedback seen in the console; non-blocking.

### Reference side panel

- Toggleable panel showing the schema (fields/types) of the table/view referenced by the active query tab, like the console's Reference pane.

### Saving queries — Save dropdown (per query tab)

- **Save query** — versioned saved query stored in `localStorage` (extends the M1-03 workspace store); upstream entry for server-backed saved queries.
- **Save query (Classic)** — simple saved query, no version history.
- **Save view** — `CREATE VIEW`/`CREATE OR REPLACE VIEW` DDL via `submitJob`/query; refresh tree.
- **Save as...** — save results to a table via `CREATE TABLE AS SELECT` / load job.

## Out of scope

- Routine creation UI (M5) — completion only references routines here.
- External source ingestion (M5).

## Verification

```bash
npm run build
npm run lint
npm run test:e2e
```

Manual: type a query and get table/column suggestions; introduce a syntax error and see a diagnostic; save a query (both kinds) and reload to confirm persistence; save a view and see it in the tree.

## Done criteria

- Completion suggests projects/datasets/tables/views/columns/UDFs.
- Syntax diagnostics show inline; reference panel shows active resource schema.
- Save query / Save query (Classic) persist to the browser and restore on reload; Save view creates a view; Save as creates a table.
