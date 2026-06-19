---
name: M5-01 — Routines (UDFs) and external sources
overview: Add user-defined functions and stored procedures as Routines (dataset Routines sub-tab, sidebar integration, detail view, creation) wired into SQL autocompletion, and complete the external-source ingestion flows in the Create Table modal (GCS, S3, Azure, Drive, Bigtable) end to end.
todos:
  - id: routine-list
    content: Promote the Routines sub-tab to full listing and add routines under each dataset in the sidebar tree
    status: pending
  - id: routine-detail
    content: Add routine detail view (definition body, arguments, return type, metadata)
    status: pending
  - id: routine-create
    content: Add routine creation (CREATE FUNCTION / CREATE PROCEDURE) via modal and/or editor
    status: pending
  - id: routine-autocomplete
    content: Feed routines into the M4 completion catalog so UDFs autocomplete in queries
    status: pending
  - id: external-sources
    content: Finish external-source ingestion in Create Table modal (GCS/S3/Azure/Drive/Bigtable) end to end
    status: pending
  - id: e2e
    content: Add Playwright coverage for viewing a routine and creating a UDF
    status: pending
isProject: false
---

# M5-01: Routines (UDFs) and external sources

## Dependencies

- [`m2-01-dataset-detail-page.plan.md`](m2-01-dataset-detail-page.plan.md) (Routines sub-tab scaffold), [`m3-01-create-table-modal.plan.md`](m3-01-create-table-modal.plan.md) (Create Table sources), [`m4-01-query-workspace-upgrades.plan.md`](m4-01-query-workspace-upgrades.plan.md) (completion catalog).
- Index: [`00-index.plan.md`](00-index.plan.md)
- Roadmap: [`ROADMAP.md`](../../ROADMAP.md) → Routines (UDFs) + Create Table external sources.

## Goal

Make routines first-class and finish external ingestion so the Create Table sources built in M3-01 actually load data.

## Scope

### Routines

- **Listing**: full Routines sub-tab on the dataset page (from `explorerQueries.routines`); also surface routines under each dataset in the sidebar tree.
- **Detail view**: routine page/panel showing routine type, language, arguments, return type, and the definition body (read-only code block) via `explorerQueries.routine`.
- **Creation**: create UDFs/procedures through a modal (name, type, args, return type, body) and/or directly via `CREATE FUNCTION` / `CREATE PROCEDURE` in a query tab; submit via `submitJob`/query and refresh.
- **Autocompletion**: register routines in the M4 completion catalog so UDF names suggest in the editor.

### External sources (Create Table)

- Complete end-to-end ingestion for the non-Empty/Existing sources scaffolded in M3-01: Google Cloud Storage, Amazon S3, Azure Blob Storage, Drive, Google Bigtable.
- Build load/external-table job configs and submit via `submitJob` + poll.
- For any source the emulator does not support, keep the UI functional and add/escalate the gap in [`upstream-emulator-work.plan.md`](upstream-emulator-work.plan.md).

## Out of scope

- Unplanned tabs (Insights, Lineage, Data Profile, Data Quality, Graphs, Models).

## Verification

```bash
npm run build
npm run lint
npm run test:e2e
```

Manual: list routines for a dataset; open a routine's definition; create a simple scalar UDF and use it in a query (autocompletes); create a table from a GCS file (or confirm graceful upstream-tracked failure).

## Done criteria

- Routines list in both the dataset page and sidebar; detail view renders; creation works; UDFs autocomplete.
- At least one external source ingests end to end against the emulator; remaining sources attempt the real contract and report errors gracefully with upstream tracking.
