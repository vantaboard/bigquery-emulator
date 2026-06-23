---
name: CREATE VIEW DDL visibility
overview: DDL-created views appear in tables.list/get with type VIEW and view.query so Save view and view resource pages work without UI changes.
todos:
  - id: dotted-view-list-id
    content: Fix tables.list id for dotted `p.d.v` CREATE VIEW (currently registers as tableId `p.d.v` in engine list merge)
    status: pending
  - id: list-view-query
    content: Populate view.query (or materializedView.query) on tables.list entries, not only tables.get
    status: pending
  - id: drop-view-overlay
    content: Verify DROP VIEW evicts overlay + engine registry (handleViewDDLAfterQuery)
    status: pending
  - id: e2e-view-regression
    content: gateway/e2e — three-segment and dotted CREATE VIEW → list + get + SELECT
    status: pending
isProject: false
---

# 01 — CREATE VIEW (and DDL) visibility in tables.list / tables.get

- **UI gap:** #10 (priority **P1**)
- **UI features blocked:** "Save view", view resource pages, sidebar refresh after DDL.
- **Verified state at HEAD (`60d19b3e`):** **Mostly fixed** — remaining list polish; see below.

## Symptom (UI brief)

1. UI runs, via `POST /bigquery/v2/projects/{p}/queries`:
   ```sql
   CREATE OR REPLACE VIEW `local-project.test-dataset.my_view` AS SELECT 1 AS x
   ```
2. Job reports success (`jobComplete`, `statistics.query.statementType =
   "CREATE_VIEW"`).
3. **Bug:** the view does **not** appear (correctly typed) in
   `GET .../datasets/{d}/tables`, `GET .../tables/my_view` returns **404**, and
   `view.query` is never served. Sidebar refresh shows nothing new.

## Current state at HEAD (grounded)

Verified 2026-06-23 on `:9050` (`60d19b3e`).

| Surface | Via `tables.insert` + `view` body | Via `CREATE OR REPLACE VIEW` query job |
|---------|-----------------------------------|----------------------------------------|
| Job `statementType` | n/a | ✅ `CREATE_VIEW` |
| `tables.list` entry present | ✅ | ✅ (engine `ListTables` merges `view_registry`) |
| `tables.list` `type` | ✅ `VIEW` | ✅ `VIEW` (overlay via `persistViewFromDDL`) |
| `tables.list` `view.query` | ✅ | ❌ null in list (only on GET) |
| `tables.get` + `view.query` | ✅ | ✅ three-segment `` `p`.`d`.`v` `` |
| Dotted `` `p.d.v` `` list `tableId` | n/a | ❌ wrong id `p.d.v` as single segment |
| `SELECT * FROM view` | ✅ | ✅ |

### Remaining root causes

1. **Dotted qualified names:** engine `RegisterProjectView` / list merge treats
   `` `local-project.gap-v3.vdot` `` as a single table id (`local-project.gap-v3.vdot`)
   instead of splitting into project/dataset/table. Three-segment backticks work.
   Gateway `splitViewTableName` (`views_ddl.go` ~132–141) splits correctly for
   overlay; engine registry does not.
2. **`view.query` omitted from list:** `TableList` does not merge overlay `view`
   into list entries (GET path does via overlay / engine).

`persistViewFromDDL` (`gateway/handlers/views_catalog.go`) and
`handleViewDDLAfterQuery` are **landed** — the original “no overlay” root cause
is closed.

## Goal / done-criteria (UI-observable)

After `CREATE OR REPLACE VIEW` via query job:

1. ✅ `tables.get` → 200, `type: VIEW`, `view.query` matches AS clause.
2. ✅ `tables.list` includes the view with `type: VIEW`.
3. ❌ **Dotted** `` `p.d.v` `` form lists `tableId: v` (not `p.d.v` as id).
4. ❌ `tables.list` entries include `view.query` (UI may call get today, but BQ
   list often omits query — confirm UI need; at minimum id must be correct).
5. `CREATE OR REPLACE VIEW` updates `view.query`; `DROP VIEW` removes from list/get.
6. No regression to CTAS / `SELECT` from views.

## Implementation steps

### Step 1 — Fix dotted view name in engine registry

When `RegisterProjectView` / `ListProjectViews` records a view whose name is
`p.d.v` inside one backtick segment, split into `(project, dataset, table)` before
indexing — mirror `splitViewTableName` in `views_ddl.go`. Verify list merge in
`frontend/handlers/catalog.cc` emits `tableReference.tableId = v`, not `p.d.v`.

### Step 2 — Merge `view.query` into `TableList` (if required by UI)

In `gateway/handlers/tables.go` `TableList`, when overlay or engine marks
`type: VIEW`, attach `view.query` from metadata overlay (same source as GET).

### Step 3 — DROP VIEW + CREATE OR REPLACE regression

Confirm `evictViewFromDDL` / `handleViewDDLAfterQuery` evicts overlay and engine
registry stays consistent. Add e2e for replace + drop.

### Step 4 — (Follow-up) restart durability

Views persist via `PersistViewDdl` + sidecars; verify gateway rehydrates overlay
on cold start (coordinate with engine restart tests).

## Tests

- `gateway/handlers` (unit, fake catalog): CREATE VIEW via `runQueryExecute` →
  overlay populated → `TableGet` returns `type=VIEW` + `view.query`; `TableList`
  shows `type=VIEW`; CREATE OR REPLACE updates query; DROP VIEW removes it.
- `gateway/e2e` (real engine): mirror `ddl_create_drop_test.go` for views —
  CREATE OR REPLACE VIEW → `tables.list` typed VIEW → `tables.get` serves
  `view.query` → `SELECT` from the view → DROP VIEW → 404.
- Regression: CTAS and `CREATE TABLE` still listable/queryable.
- Materialized view variant if `materializedView.query` is in scope.

## Out of scope

- Full MV refresh/incremental semantics (only typing + `materializedView.query`).
- View column-level schema fidelity if Option A ships first (empty/derived schema
  acceptable until Option B).

## Touch list

`gateway/handlers/queries.go`, `gateway/handlers/jobs.go`,
`gateway/handlers/tables.go`, `gateway/handlers/metadata_store.go` (add
`DeleteTable` if missing), new `gateway/handlers/views_ddl.go`,
optionally `frontend/handlers/catalog.cc` + `backend/catalog/view_registry.cc`
(Option B / Step 5). Keep `docs/REST_API.md` view rows in sync.
