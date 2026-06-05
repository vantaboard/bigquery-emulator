---
name: routines CRUD
overview: "Implement the routines.* CRUD surface (UDFs, stored procedures, table-valued functions) against a new persistent routine registry in `Storage`. Coordinates with the query-time UDF lookup that `local-exec-15-specialized-stubs.plan.md` and `local-exec-03-operator-disposition.plan.md` own."
todos:
  - id: tp12_registry
    content: "Design + implement a persistent Routines registry in backend/storage/ (catalog-side; columns mirror BigQuery's Routine resource: name, type, language, definitionBody, args, returnType, etag, creation/lastModified time)."
    status: pending
  - id: tp12_insert
    content: "RoutineInsert: persist a routine; validate language (SQL / JAVASCRIPT) and routineType (SCALAR_FUNCTION / PROCEDURE / TABLE_VALUED_FUNCTION). Mint etag."
    status: pending
  - id: tp12_get_list
    content: "RoutineGet + RoutineList: read from the registry; paginate by routineId; honor `filter` and `readMask`."
    status: pending
  - id: tp12_update_delete
    content: "RoutineUpdate (PUT) + RoutinePatch (PATCH) + RoutineDelete: standard CRUD against the registry; honor If-Match etag."
    status: pending
  - id: tp12_query_lookup
    content: "Confirm the query-time routine lookup (owned by local-exec-15-specialized-stubs.plan.md) reads from the same registry; if not, add a thin adapter so a CREATE FUNCTION via routines.insert is invokable in the next query (and vice versa)."
    status: pending
  - id: tp12_tests
    content: "Unit-test the registry + handler set; integration-test via the node Routines suite and via a SQL CREATE FUNCTION + SELECT round-trip."
    status: pending
  - id: tp12_index
    content: "Flip thirdparty-00-completion-index.plan.md todo `tp12` to `completed`."
    status: pending
isProject: false
---

# Thirdparty 12 — routines CRUD

## Source

- `not-implemented-routes-catalog.plan.md` Group 2 (now deleted).
- Failing node tests (before-all):
  - `Routines > should create a routine`
  - (cascade) `RoutineUpdate`
- Stubs: [`gateway/handlers/routines.go`](../../gateway/handlers/routines.go).
- Coordination plans (query-time UDF lookup, not CRUD):
  - [`local-exec-15-specialized-stubs.plan.md`](./local-exec-15-specialized-stubs.plan.md)
  - [`local-exec-03-operator-disposition.plan.md`](./local-exec-03-operator-disposition.plan.md)

## Prerequisites

- None hard; coordinates with the two query-time plans above. If
  those plans have not landed yet, this plan still lands the CRUD
  surface and stores the routine bytes; query-time execution stays
  blocked on those plans.

## Scope

A persistent routine registry in `Storage` plus the six standard CRUD
handlers (Insert, Get, List, Update, Patch, Delete). The query-time
half (resolving a routine reference inside a SELECT and dispatching
it) is owned by `local-exec-15-specialized-stubs.plan.md`; this plan
guarantees they share one source of truth.

## Implementation

### Registry shape

Per-row columns mirror the BigQuery `Routine` resource:

- `routineReference = {projectId, datasetId, routineId}` (primary key)
- `routineType` — `SCALAR_FUNCTION` | `PROCEDURE` |
  `TABLE_VALUED_FUNCTION` | `AGGREGATE_FUNCTION`.
- `language` — `SQL` | `JAVASCRIPT`.
- `definitionBody` — text.
- `arguments[]` — name + type spec (mode, dataType, argumentKind).
- `returnType` — for scalar / TVF.
- `returnTableType` — for TVF.
- `importedLibraries[]` — for JS.
- `description` — optional.
- `etag`, `creationTime`, `lastModifiedTime` — metadata.

Persist alongside the dataset rows; cascade-delete with parent
dataset (mirror Tables).

### Handlers

1. **RoutineInsert**: validate routineType + language; mint etag +
   timestamps; persist.
2. **RoutineGet**: by routineReference; 404 envelope.
3. **RoutineList**: paginate by `pageToken`; honor `filter` and
   `readMask` parameters.
4. **RoutineUpdate (PUT)** and **RoutinePatch (PATCH)**: enforce
   If-Match etag; bump lastModifiedTime + etag.
5. **RoutineDelete**: remove from registry.

### Query-time integration

Confirm that
[`local-exec-15-specialized-stubs.plan.md`](./local-exec-15-specialized-stubs.plan.md)
already resolves routine references through the same `Storage`
surface. If it does, no change. If it does not (e.g. it reads from a
separate in-memory map populated by `CREATE FUNCTION` DDL), add a
thin adapter so a routine created via `routines.insert` is
invokable in the next query — and vice versa, a routine created via
`CREATE FUNCTION` is visible via `routines.get`.

## Tests

- Backend unit tests for the registry CRUD operations.
- Handler tests under
  `gateway/handlers/routines_test.go` for each handler.
- Integration test: create a routine via `routines.insert`, invoke
  it from a SELECT in the next query, observe the result.
- `task thirdparty:node-bigquery-tests` — Routines block goes
  green.
- Java + python suites: no regression; routines surface should
  return real rows now.

## Done criteria

- The six CRUD handlers are wired to a real registry.
- Query-time lookup shares the registry with the CRUD surface.
- Node Routines block goes green.
- `thirdparty-00-completion-index.plan.md` todo `tp12` flipped to
  `completed`.
