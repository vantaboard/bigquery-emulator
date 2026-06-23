---
name: Routines API completeness
overview: Routines list/get/create/update/delete return complete metadata; qualified-name query invocation is plan 10.
todos:
  - id: unify-list-sources
    content: Union catalog + in-memory store in RoutineList so DDL routines always appear
    status: pending
  - id: catalog-timestamps
    content: Non-zero creationTime/lastModifiedTime on catalog-backed GET (if still divergent)
    status: pending
  - id: catalog-aware-update
    content: RoutineUpdate existence check against catalog when enabled
    status: pending
  - id: e2e-ddl-routine-list
    content: gateway/e2e — CREATE FUNCTION → routines.list + routines.get
    status: pending
isProject: false
---

# 06 — Routines API completeness

- **UI gap:** #9 REST surface (priority **P6** for polish; **P2** for qualified calls → plan 10)
- **Verified state at HEAD (`60d19b3e`):** REST CRUD **works**; query invocation gap → [`10-routine-qualified-name-resolution.plan.md`](10-routine-qualified-name-resolution.plan.md).

## Current state at HEAD (grounded)

Routes (all real handlers, no 501) at `gateway/server.go` ~166–170
(`mountModelsAndRoutines`); discovery `gateway/handlers/discovery_methods.go`
~302–344. Handlers in `gateway/handlers/routines.go`:

| Method | Path | Handler | State |
|--------|------|---------|-------|
| GET (list) | `.../routines` | `RoutineList` | works (partial fields) |
| POST | `.../routines` | `RoutineInsert` | works |
| GET | `.../routines/{id}` | `RoutineGet` | works (partial timestamps) |
| PUT | `.../routines/{id}` | `RoutineUpdate` | works (store-only existence check) |
| DELETE | `.../routines/{id}` | `RoutineDelete` | works |

Dual storage: in-memory `gateway/routines/store.go` (used when `Catalog == nil`
and as mirror) + durable DuckDB catalog (`frontend/handlers/catalog_routines.cc`,
`RoutineRecord` has `ddl_sql` / `language` / `signature_json`, **no timestamps**).
DDL `CREATE FUNCTION/PROCEDURE/TABLE FUNCTION` → `persistRoutineFromDDL`
(`gateway/handlers/queries.go` ~254–257, `jobs.go` ~444–447 →
`routines_catalog.go` ~146–165).

### Known divergences

1. **List source split.** When the catalog is enabled, `RoutineList`
   (`routines.go` ~90–115) reads **only** the catalog and does not union the
   in-memory store; when disabled, it reads only the store. A routine present in
   one but not the other can be invisible to list while visible to get.
2. **Missing timestamps on catalog-backed GET.** `routineFromDescriptor`
   (`routines_catalog.go` ~24–48) can't fill `creationTime` / `lastModifiedTime`
   (engine record has none).
3. **Update existence check is store-only** (`routines.go` ~192), not catalog.
4. **List trims body/args/returnType** (`routineListEntry` ~74–85) — matches BQ's
   typical list shape, but confirm the UI doesn't depend on full bodies in list.
5. **gRPC v2 path** (`gateway/handlers/bqv2grpc/routine.go`) uses the in-memory
   store only — separate from REST but relevant for non-REST clients.

## Goal / done-criteria (UI-observable)

1. A routine created by any path (REST insert **or** `CREATE FUNCTION` DDL)
   appears in `routines.list` whether or not the catalog is enabled.
2. `routines.get` returns `routineType`, `language`, `definitionBody`,
   `arguments`, `returnType`, and **non-zero** `creationTime` /
   `lastModifiedTime`.
3. PUT update succeeds for routines that exist in the catalog (not just the
   store).
4. No regression to the existing CRUD tests.

## Implementation steps

### Step 1 — Unify list sources

In `RoutineList` (`routines.go` ~90–115): when the catalog is enabled, union
catalog results with in-memory store entries (dedupe by
`project:dataset.routine`), or make every write authoritative to the catalog so
the store is a pure cache. Add a test: DDL-created routine appears in list under
a live catalog.

### Step 2 — Persist + serve timestamps

Give catalog-backed routines real timestamps. Options:
- extend the engine `RoutineRecord` (`backend/storage/storage.h` ~85–92) +
  catalog upsert to store `creation_time` / `last_modified_time`; or
- track timestamps in the gateway store mirror and merge them into
  `routineFromDescriptor` on GET.
The gateway-mirror approach avoids a proto/storage change and is the smaller cut.

### Step 3 — Catalog-aware update existence check

`RoutineUpdate` (`routines.go` ~189–215): check existence against the catalog
(when enabled) before 404, then sync both catalog and store.

### Step 4 — Verify list field shape vs UI

Confirm the UI's routines list view only needs the trimmed fields
(`routineType`, `language`, timestamps, reference). If it needs bodies, add them
to `routineListEntry` or have the UI call get per row. Default: keep trimmed.

### Step 5 — (Optional) align gRPC v2 path

Point `gateway/handlers/bqv2grpc/routine.go` at the catalog when enabled for
parity with REST. Lower priority (non-REST clients).

### Step 6 — Doc hygiene

Update the stale "wired stubs" comment at `gateway/server.go` ~156–159; sync
`docs/REST_API.md` routines rows to `done`.

## Tests

- `gateway/handlers/routines_test.go`: DDL → list (currently only DDL → get is
  covered); catalog+store union; timestamps non-zero on catalog GET; PUT on a
  catalog-only routine.
- `gateway/e2e`: `CREATE FUNCTION` via query → `routines.list` + `routines.get`.

## Out of scope

- New routine languages / execution semantics beyond what the engine supports.

## Touch list

`gateway/handlers/routines.go`, `gateway/handlers/routines_catalog.go`,
`gateway/routines/store.go`, optionally `backend/storage/storage.h` +
`frontend/handlers/catalog_routines.cc` (timestamps), `gateway/server.go`
(comment), `docs/REST_API.md`.
