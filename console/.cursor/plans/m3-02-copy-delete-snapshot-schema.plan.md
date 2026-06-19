---
name: M3-02 — Copy, Delete, Snapshot, and Edit Schema
overview: Implement the remaining mutations behind the dataset and table action toolbars — Copy dataset, Copy table/view, Delete dataset, Delete table, Create table snapshot, and Edit Schema (mode changes + add fields) — each as a modal/dialog wired to the appropriate job or PATCH/DELETE call.
todos:
  - id: copy-dataset
    content: Copy dataset modal (destination dataset + overwrite) submitting a copy job
    status: pending
  - id: copy-table
    content: Copy table/view modal (destination project/dataset/table + encryption) submitting a copy job
    status: pending
  - id: snapshot
    content: Create table snapshot modal (tables only) with destination + expiration + snapshot time
    status: pending
  - id: delete
    content: Delete dataset and Delete table confirmation dialogs calling DELETE endpoints
    status: pending
  - id: edit-schema
    content: Edit Schema modal — change modes (REQUIRED->NULLABLE), edit descriptions/defaults, add fields; PATCH table
    status: pending
  - id: e2e
    content: Add Playwright coverage for delete and edit-schema flows
    status: pending
isProject: false
---

# M3-02: Copy, Delete, Snapshot, Edit Schema

## Dependencies

- M1 plans; [`m2-01-dataset-detail-page.plan.md`](m2-01-dataset-detail-page.plan.md) and [`m2-02-table-detail-page.plan.md`](m2-02-table-detail-page.plan.md) (toolbar entry points + Schema tab).
- Index: [`00-index.plan.md`](00-index.plan.md)
- Roadmap: [`ROADMAP.md`](../../ROADMAP.md) → Datasets (Copy, Delete) and Tables/Views (Schema edit, Copy, Snapshot).

## Goal

Complete the mutation surface so every action button on the dataset and table pages does real work, built on `Modal` / `ConfirmDialog` and `explorerQueries` job/patch/delete methods (M1-01).

## Scope

### Copy dataset (dataset page)

- Modal: read-only Source (project/dataset/location) + Destination dataset name + "Overwrite destination tables" checkbox.
- Submit a copy job via `submitJob`; poll `getJob`; refresh on success.

### Copy table / view (table page)

- Modal: read-only Source; Destination project/dataset/table; Advanced → encryption (Google-managed / Cloud KMS).
- Submit copy job; refresh destination dataset and sidebar.

### Create table snapshot (table page, tables only)

- Modal: read-only Source; Destination project/dataset/table (default `name-<timestamp>`); Expiration time; Snapshot time (point-in-time within travel window).
- Submit snapshot/copy job; refresh.

### Delete (dataset + table)

- `ConfirmDialog` requiring confirmation (and for datasets, optional "delete contents").
- Call `deleteDataset` / `deleteTable`; on success remove from tree, close the resource tab(s), refresh.

### Edit Schema (table Schema tab)

- Modal opened from the Schema tab **Edit schema** button.
- Editable grid: change Mode (e.g. REQUIRED → NULLABLE), edit Description and Default Value; add new fields (reuse the schema-builder pieces from M3-01 where practical).
- Validate allowed transitions; submit via `patchTableSchema` (`PATCH .../tables/{t}`); refresh the Schema tab on success.

## Out of scope

- Create Table modal (M3-01).
- Save-as-view and routine creation (M4/M5).

## Verification

```bash
npm run build
npm run lint
npm run test:e2e
```

Manual: copy a table to a new name; snapshot a table; delete a table and a dataset; change a field's mode and add a field via Edit Schema.

## Done criteria

- Copy dataset, Copy table, Snapshot, Delete (dataset + table), and Edit Schema all function against the emulator (or surface upstream-tracked errors gracefully).
- Tree, dataset Overview, and Schema tab reflect changes without manual refresh.
- Destructive actions require explicit confirmation.
