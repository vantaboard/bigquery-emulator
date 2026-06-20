# 01 — CREATE VIEW (and DDL) visibility in tables.list / tables.get

- **UI gap:** #10 (priority **P1** — highest)
- **UI features blocked:** "Save view" (CREATE OR REPLACE VIEW), view resource pages,
  sidebar/tree refresh after creating a view.
- **Verified state at HEAD (`d390572`):** Partial — see below.

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

| Surface | Via `tables.insert` + `view` body | Via `CREATE OR REPLACE VIEW` query job |
|---------|-----------------------------------|----------------------------------------|
| Job `statementType` | n/a | ✅ `CREATE_VIEW` (`frontend/handlers/query_internal.cc` ~371) |
| `tables.list` entry present | ✅ | ✅ at HEAD — engine `ListTables` now merges `view_registry` (`frontend/handlers/catalog.cc` ~296–323) |
| `tables.list` `type` | ✅ `VIEW` (overlay) | ❌ defaults to `TABLE` (`gateway/handlers/tables.go` ~130–135) |
| `tables.get` | ✅ `view.query` | ❌ 404 |
| `view.query` served | ✅ | ❌ |
| `SELECT * FROM view` | ✅ | ✅ (engine inlines via `FindProjectView`) |

### Root cause

- DDL query jobs register the view only in the engine's **in-memory**
  `view_registry` (`backend/catalog/view_registry.cc`), via
  `RegisterProjectView` in `backend/engine/coordinator/local_coordinator_engine.cc`
  (~265–304). No view SQL is exposed back through a Catalog RPC.
- The gateway mirrors **routines** and **models** created via DDL into
  `MetadataStore` (`gateway/handlers/queries.go` ~254–261 calls
  `persistRoutineFromDDL` / `persistModelFromDDL`) but has **no**
  `persistViewFromDDL`. So no overlay row exists for DDL views.
- `TableGet` (`gateway/handlers/tables.go` ~340–398) calls
  `Catalog.DescribeTable`, which is storage-only; views have no storage sidecar,
  so it returns gRPC NotFound → HTTP 404. The view-overlay fallback at
  `tables.go` ~355–365 only fires when `overlay.View != nil`, which is never set
  for DDL views.
- `TableList` defaults `type` to `"TABLE"` unless the overlay says otherwise.

Reference: the working `tables.insert` view path is
`gateway/handlers/tables_insert.go` (~22–31) + `tables_metadata_get_test.go`
(~58–107). Mirror that end-state for DDL.

## Goal / done-criteria (UI-observable)

After running the CREATE VIEW DDL above through `POST .../queries` (and through
`POST .../jobs` query config):

1. `GET .../datasets/test-dataset/tables` lists `my_view` with `type: "VIEW"`.
2. `GET .../tables/my_view` returns 200 with `type: "VIEW"` and
   `view.query == "SELECT 1 AS x"` (and `view.useLegacySql` present, default
   `false` — coordinate with Gap 2).
3. `CREATE OR REPLACE VIEW` updates the stored `view.query` (idempotent replace).
4. `DROP VIEW \`...my_view\`` removes it from `tables.list` and makes
   `tables.get` 404 again.
5. No regression to CTAS / `CREATE TABLE` (already listable+queryable, proven by
   `gateway/e2e/ddl_create_drop_test.go` ~39–160) or to `SELECT` from views.

## Implementation steps

### Step 1 — Add a `parseCreateViewDDL` helper (gateway, Go)

Mirror `gateway/routines/ddl.go` (`RegisterFromDDL` / `parseCreateRoutineDDL`).
Add a small parser (new file `gateway/handlers/views_ddl.go` or extend an
existing DDL helper) that extracts from `CREATE [OR REPLACE] [MATERIALIZED] VIEW
\`p.d.v\` AS <query>`:
- target `projectId` / `datasetId` / `tableId` (apply `defaultDataset` when the
  name is 1- or 2-part);
- the AS-query text (everything after the first top-level `AS`);
- view vs materialized-view flag.

Prefer driving this from the engine when possible: if a Catalog RPC can return
the registered view definition (see Step 4), parse only the target name and pull
`view.query` from the engine to avoid SQL-reparsing drift. Parsing the DDL in Go
is the pragmatic first cut.

### Step 2 — `persistViewFromDDL` + call sites

Add `persistViewFromDDL(ctx, deps, projectID, defaultDataset, sql)` next to
`persistRoutineFromDDL` (pattern: `gateway/handlers/routines_catalog.go`
~146–165). It should:
- parse the DDL (Step 1);
- `deps.Metadata.PutTable(projectID, datasetID, viewID, &bqtypes.Table{Type:
  "VIEW", View: &bqtypes.ViewDefinition{Query: asQuery}})` (use
  `MATERIALIZED_VIEW` + `MaterializedView` for the MV form).

Call it from both DDL execution paths when `statementType` indicates a view:
- `gateway/handlers/queries.go` (~253–261): add a branch for
  `statementType == "CREATE_VIEW"` (and `"CREATE_MATERIALIZED_VIEW"`).
- `gateway/handlers/jobs.go` (`runSyncQueryInsert`, ~444–447 region): same branch.

Also handle `DROP VIEW` / `DROP MATERIALIZED VIEW`: when `statementType ==
"DROP_VIEW"` (confirm the engine's emitted string), evict the overlay via a new
`deps.Metadata.DeleteTable(...)` call so the view disappears from list/get.

### Step 3 — Make `TableList` emit `type: "VIEW"`

`gateway/handlers/tables.go` `TableList` currently defaults `type` to `"TABLE"`.
For each listed entry, consult the metadata overlay (and/or an engine-provided
kind) and set `type: "VIEW"` / `"MATERIALIZED_VIEW"` when known. Once Step 2
stores overlays, list typing falls out of the existing overlay merge; verify the
engine `ListTables` merge does not stomp the type.

### Step 4 — `TableGet` resolves DDL views (choose A or B)

- **Option A (gateway-only, lower effort):** rely on the Step 2 overlay. Extend
  the `TableGet` view fallback (`tables.go` ~355–365) so that when
  `DescribeTable` returns NotFound **and** an overlay with `Type==VIEW` exists,
  it synthesizes a 200 view resource from the overlay (schema may be empty or
  derived from a one-row dry-run of `view.query`).
- **Option B (engine-authoritative, more robust):** add a Catalog RPC (e.g.
  `DescribeView` or extend `DescribeTable` to fall back to `FindProjectView` /
  `ListProjectViews` in `frontend/handlers/catalog.cc`) returning the view
  definition + resolved schema. The gateway then serves `view.query` and a real
  schema. Preferred long-term; pairs with Step 5.

Recommendation: ship **Option A** to unblock the UI, file **Option B** as the
durable follow-up.

### Step 5 — (Follow-up) persist views across restart

`RegisterProjectView` is process-local. Routines persist via
`PersistRoutineDdl` (`backend/.../routine_persistence.cc`); add equivalent
storage-backed persistence for views so they survive engine restart and so the
gateway overlay can be rehydrated. Track as a separate change if it expands scope.

### Step 6 — CREATE FUNCTION / routines DDL (already works — verify only)

`CREATE FUNCTION` / `PROCEDURE` / `TABLE FUNCTION` already persist via
`persistRoutineFromDDL` and show in `routines.list` when the catalog is enabled
(see plan 06). Add a test asserting DDL-created routine → `routines.list` (no
such test exists today).

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
