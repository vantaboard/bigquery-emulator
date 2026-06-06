---
name: Thirdparty 06 — Routines CRUD
overview: Implement BigQuery routines REST API (insert/get/list/update/delete) with catalog persistence and DDL integration.
depends_on: [thirdparty-02-gateway-query-metadata]
blocks: []
baseline_log: .logs/thirdparty-20260605-112926.log
est_effort: ~1 week
isProject: true
todos:
  - id: routine-registry
    content: In-memory then persistent routine registry keyed by project.dataset.routineId
    status: pending
  - id: routine-insert
    content: Implement RoutineInsert — replace 501 in gateway/handlers/routines.go
    status: pending
  - id: routine-get-list
    content: RoutineGet returns stored routine; RoutineList paginates registry
    status: pending
  - id: routine-update-delete
    content: RoutineUpdate and RoutineDelete with correct 404 semantics
    status: pending
  - id: ddl-integration
    content: DDL CREATE FUNCTION/PROCEDURE from query jobs registers routines for REST CRUD
    status: pending
  - id: routine-tests
    content: Extend gateway/handlers/routines_test.go; green python test_routine_samples* and node Routines
    status: pending
---

# Thirdparty 06 — Routines CRUD

## Goal

Clear routine-related failures in python and node thirdparty suites.

## Baseline failures

| Suite | Symptom |
|-------|---------|
| Python | `test_routine_samples`, `test_create_routine`, `test_create_routine_ddl`, `test_list_routines`, `test_get_routine`, `test_delete_routine`, `test_update_routine` — 501 on `routines.insert` |
| Node | Routines `before all` hook — 501 on routine create |

**Note:** Node `should create a routine using DDL` (Queries suite) **already passes** — DDL path works partially; REST CRUD does not.

## Current stubs

[`gateway/handlers/routines.go`](../../gateway/handlers/routines.go):

| Method | Today |
|--------|-------|
| `RoutineList` | Empty page (200) |
| `RoutineGet` | 404 always |
| `RoutineInsert` | **501** |
| `RoutineUpdate` | **501** |
| `RoutineDelete` | 404 always |

## Implementation

### 1. Routine registry

New package e.g. [`gateway/routines/`](../../gateway/routines/) or extend catalog seed store:

```go
type Routine struct {
    RoutineReference bqtypes.RoutineReference
    RoutineType      string // SCALAR_FUNCTION | PROCEDURE | ...
    Language         string
    DefinitionBody   string
    Arguments        []RoutineArgument
    // timestamps, etag, etc.
}
```

Persist alongside datasets/tables in gateway catalog ([`gateway/seed/`](../../gateway/seed/) or engine catalog RPC).

### 2. REST handlers

Implement per [BigQuery routines REST reference](../../docs/bigquery/docs/reference/rest/v2/routines.md):

- `POST .../datasets/{datasetId}/routines` — insert
- `GET .../routines/{routineId}` — get
- `GET .../routines` — list with pagination
- `PUT .../routines/{routineId}` — update
- `DELETE .../routines/{routineId}` — delete

Return BigQuery-shaped `Routine` resources with correct `etag` / `creationTime`.

### 3. DDL integration

When a query job runs `CREATE FUNCTION` / `CREATE PROCEDURE` / `CREATE TABLE FUNCTION`:
- Control-op or DDL path should register routine in same registry
- REST `routines.get` must find DDL-created routines (node DDL test already creates one)

Investigate engine path for DDL routines in [`backend/engine/control/`](../../backend/engine/control/) — may need new handler or extend catalog mutation.

### 4. UDF execution (defer if needed)

Samples may only test CRUD, not **calling** the UDF in SQL. If call-path tests fail, document as follow-up engine work; prioritize REST CRUD parity first.

## Verification

```bash
go test ./gateway/handlers/... -count=1 -run Routine

task thirdparty:python-bigquery-tests
# Filter:
PYTHON_SAMPLES_PYTEST_ARGS='samples/tests/test_routine_samples.py -v' task thirdparty:python-bigquery-tests

task thirdparty:node-bigquery-tests
```

## Out of scope

- Full UDF evaluation in SQL queries (unless required by samples)
- Remote function / BigFrames UDF (separate)

## Done when

- [ ] All `test_routine_samples*` pass
- [ ] Node Routines suite `before all` succeeds
- [ ] `gateway/handlers/routines_test.go` covers CRUD round-trip
- [ ] `docs/REST_API.md` routines section upgraded from stub
