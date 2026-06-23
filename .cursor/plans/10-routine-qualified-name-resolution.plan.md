---
name: Routine qualified-name resolution
overview: Let queries invoke user UDFs with fully-qualified names (`SELECT `proj.ds.fn`(x)`) so the UI and scripts match production BigQuery.
todos:
  - id: repro-qualified-udf
    content: Lock repro in gateway/e2e — CREATE FUNCTION then qualified SELECT returns rows (not 400 Function not found)
    status: pending
  - id: engine-catalog-lookup
    content: Resolve `project.dataset.routine` in analyzer catalog (mirror FindProjectView ordering in googlesql_catalog / udf_registry)
    status: pending
  - id: three-segment-backticks
    content: Support `` `proj`.`ds`.`fn` `` call syntax in addition to dotted single-backtick form
    status: pending
  - id: regression-unqualified
    content: Assert unqualified `SELECT fn(x)` and routines.list/get unchanged
    status: pending
isProject: false
---

# 10 — Routine / UDF qualified-name resolution in queries

- **UI gaps:** #9 (routines), #10 (DDL/DML — CREATE FUNCTION + call)
- **Priority:** **P2** (blocks fully-qualified UDF usage in editor + saved queries)
- **Verified state at HEAD (`60d19b3e`):** **Open**

## Symptom (UI brief repro)

```sql
CREATE OR REPLACE FUNCTION add_one(x INT64) RETURNS INT64 AS (x + 1);
SELECT `local-project.gap-udf.add_one`(5);   -- 400 Function not found
SELECT add_one(5);                             -- OK → 6
```

Three-segment backticks give a parser hint but still fail:

```sql
SELECT `local-project`.`gap-udf`.`add_one`(5);
-- Function not found: `local-project`.`gap-udf`.add_one; … Did you mean (`local-project`.`gap-udf`).add_one(...)?
```

REST `routines.list` / `routines.get` work; only **query-time resolution** is broken.

## Current state at HEAD (grounded)

| Path | State | Anchor |
|------|-------|--------|
| `CREATE FUNCTION` DDL | ✅ registers in engine + `persistRoutineFromDDL` | `local_coordinator_engine.cc` ~248–264 |
| `routines.list/get` | ✅ | `gateway/handlers/routines.go` |
| Unqualified `SELECT fn(x)` | ✅ | engine registers by short name in project catalog |
| Dotted `` `p.d.fn` `` call | ❌ 400 | analyzer cannot resolve qualified path |
| SQL Tools `/complete` for UDF | ⚠️ appears as `kind:function`, not `routine` | plan 09 (separate polish) |

### Root cause

UDFs register in `udf_registry` by **short function name** within a project
(`RegisterProjectFunction`, `udf_registry.cc` ~51–80). The registration catalog
(`udf_registration_catalog.cc`) replays them into `SimpleCatalog` without
dataset-qualified aliases. The analyzer therefore resolves `add_one` in the
default dataset scope but not `` `local-project.gap-udf.add_one` ``.

Views had the same class of bug and were fixed by checking `FindProjectView`
before storage sidecars in `googlesql_catalog.cc` ~306–316. UDFs need an
equivalent lookup keyed by `(project_id, dataset_id, routine_id)` (and/or a
catalog hook that maps dotted paths to registered functions).

## Goal / done-criteria (UI-observable)

1. After `CREATE FUNCTION \`local-project.ds.add_one\` …`, both of these succeed:
   - `SELECT add_one(5)` (default dataset = `ds`)
   - `SELECT \`local-project.ds.add_one\`(5)`
2. Three-segment form `` SELECT `local-project`.`ds`.`add_one`(5) `` succeeds.
3. Unknown qualified names still return a BigQuery-style 400, not an engine abort.
4. `routines.list/get` unchanged; plan 06 CRUD tests still pass.

## Implementation steps

### Step 1 — Catalog lookup for qualified routines

In `backend/catalog/googlesql_catalog.cc` (or a dedicated resolver invoked from
the catalog's function lookup path):

- On `FindFunction` / TVF / procedure resolution, if the name path has three
  parts `project.dataset.routine`, consult `udf_registry` (and procedure/TVF
  registries) keyed by dataset + routine id.
- Register functions under both short and qualified names when replaying into
  `SimpleCatalog`, or add a fallback resolver similar to `FindProjectView`.

Anchor files: `backend/catalog/udf_registry.{h,cc}`,
`backend/catalog/udf_registration_catalog.cc`, `backend/catalog/googlesql_catalog.cc`.

### Step 2 — Persist qualified name at DDL registration

When `RegisterProjectFunction` runs from `CREATE FUNCTION`, capture the
resolved `(project, dataset, routine)` from
`ResolvedCreateFunctionStmt::name_path()` (same source used for
`PersistRoutineDdl`) and index the function by qualified name, not just
`function->Name()`.

### Step 3 — Gateway e2e repro

Add `gateway/e2e/routine_qualified_call_test.go`:

- CREATE FUNCTION via query job
- qualified SELECT returns expected scalar
- unqualified SELECT still works

### Step 4 — Coordinate with SQL Tools (plan 09)

Once engine resolution works, ensure `catalog_names.cc` lists user routines with
`kind:routine` and `fqn=project.dataset.routine` so `/complete` matches query
behavior (optional same PR or follow-up).

## Tests

- `backend/catalog/udf_registry_test.cc` — qualified lookup unit tests.
- `gateway/e2e` — CREATE FUNCTION + qualified SELECT (primary UI repro).
- Regression: unqualified calls, DROP FUNCTION, CREATE OR REPLACE.

## Out of scope

- Routines REST timestamp polish (plan 06).
- JavaScript/Python UDF runtime semantics beyond name resolution.

## Touch list

`backend/catalog/udf_registry.{h,cc}`, `backend/catalog/googlesql_catalog.cc`,
`backend/engine/coordinator/local_coordinator_engine.cc`, optionally
`frontend/handlers/catalog_routines.cc`, `gateway/e2e/routine_qualified_call_test.go`.
