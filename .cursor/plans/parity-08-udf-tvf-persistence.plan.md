---
name: Parity 08 — UDF/TVF/procedure durable persistence
overview: Move the in-process UDF / UDAF / TVF / procedure registries onto DuckDBStorage so routines survive restarts and round-trip through the routines REST surface, and land the JS-UDF registration-time posture that is currently blocked on body storage.
depends_on: [parity-05-scripting-control-flow]
est_effort: ~1-2 weeks
isProject: true
todos:
  - id: storage-schema
    content: "Design the routine persistence schema in catalog.duckdb (project, dataset, name, kind UDF/UDAF/TVF/PROCEDURE, language, signature incl. ANY TYPE markers, body SQL, options); add Storage interface methods + DuckDBStorage impl."
    status: pending
  - id: registry-roundtrip
    content: "Rehydrate udf_registry / tvf_registry / procedure_registry from storage at engine startup and on first catalog touch; CREATE [OR REPLACE] / DROP FUNCTION|PROCEDURE write through; keep the replay-into-query-catalog path unchanged."
    status: pending
  - id: drop-function
    content: "ResolvedDropFunctionStmt (and procedure drop) through the control-op executor; flip the (planned) SHAPE_TRACKER row."
    status: pending
  - id: routines-rest
    content: "Gateway routines REST surface: routines.insert/get/list/delete reflect persisted routines (gateway/routines/ddl.go metadata parsing already accepts ANY TYPE); promote the relevant docs/REST_API.md rows from stub to implemented."
    status: pending
  - id: js-udf-posture
    content: "CREATE FUNCTION ... LANGUAGE js: with body storage available, decide the documented posture - register metadata-only (local_stub) so getRoutine round-trips, while call-time evaluation stays UNIMPLEMENTED; update ENGINE_POLICY JS-UDF row + SHAPE_TRACKER ResolvedCreateFunctionStmt note accordingly."
    status: pending
  - id: argument-def
    content: "ResolvedArgumentDef executor-side walk for the storage round-trip (signature reconstruction at rehydrate time); drop its status=planned marker."
    status: pending
  - id: fixtures-trackers
    content: "Fixtures: udf/ + restart-persistence coverage (engine restart inside a gateway e2e test or a two-phase conformance step); flip tracker rows; update ROADMAP scripting/UDF bullets."
    status: pending
---

# Parity 08 — UDF/TVF/procedure durable persistence

## Why

ROADMAP defers a whole cluster on one prerequisite: *"SQL UDF / TVF
body storage + invocation + JS UDF registration-time rejection stay
deferred until the per-engine UDF / TVF registry round-trip through
`DuckDBStorage` lands (the prerequisite for cross-request function
persistence)."* Scalar UDFs, UDAFs, TVFs, and procedures all evaluate
correctly in-process today — the gap is durability and the routines
REST round-trip, which any client that creates routines once and uses
them across sessions depends on.

## Key files

- [`backend/catalog/udf_registry.cc`](../../backend/catalog/) + `tvf_registry` + `procedure_registry`
- [`backend/storage/storage.h`](../../backend/storage/storage.h), [`backend/storage/duckdb/`](../../backend/storage/duckdb/)
- [`backend/engine/control/control_op_executor.{h,cc}`](../../backend/engine/control/) — CREATE/DROP write-through
- [`gateway/routines/ddl.go`](../../gateway/routines/), gateway routines handlers, [`docs/REST_API.md`](../../docs/REST_API.md)
- `conformance/fixtures/udf/`

## Steps

1. Schema design first (todo `storage-schema`); store the original
   body SQL + signature metadata, not a resolved AST — re-analysis at
   rehydrate time is the consistent posture (GoogleSQL is the source
   of truth, per ROADMAP).
2. Write-through on CREATE/DROP via the existing control-op path;
   rehydrate at startup. Watch the shadowing rule: persisted user
   functions must still shadow built-ins on name collision.
3. `DROP FUNCTION` lands here naturally (control-op + storage delete).
4. REST: wire `routines.get/list` to the persisted records so the
   metadata a client wrote comes back (language, definitionBody,
   arguments incl. ANY TYPE).
5. JS UDFs: registration becomes metadata-only `local_stub` (per the
   two-halves stub contract in ENGINE_POLICY — probe-compatible, no
   silent approximation: invocation stays UNIMPLEMENTED).
6. Persistence proof: a test that creates a routine, restarts the
   engine process, and calls it.

## Verify

```bash
task emulator:build-engine:bazel
task conformance:run
go test ./gateway/... -run Routine
task lint:fix && task lint:run
task bazel:shutdown && task bazel:status
```

## Out of scope

- JS UDF call-time execution (embedded JS runtime) — stays `unsupported`
- Python UDFs — stay `unsupported` per ENGINE_POLICY
- `ResolvedCreateConstantStmt` module constants — tracker note only
