---
name: BQUtils 03 — ANY TYPE templated scalar UDFs
overview: Enable CREATE FUNCTION f(v ANY TYPE) AS (...) registration + call-time evaluation in the engine so the large set of bigquery-utils ANY TYPE UDFs (nullifzero, nvl, etc.) move from known_failing to passing.
depends_on: [bqutils-02-engine-build-triage]
blocks: [bqutils-09-promote-to-gate]
est_effort: 3-5 days
isProject: true
todos:
  - id: repro
    content: Confirm which ANY TYPE bigquery-utils fixtures fail today and capture the engine error (CREATE-time vs call-time) from the known_failing set.
    status: pending
  - id: register
    content: Ensure templated SQL UDFs (TemplatedSQLFunction) with ANY TYPE args register correctly via create_function_util.cc / udf_registry; verify gateway routines/ddl.go scanSQLType tolerates ANY TYPE (currently does not parse it).
    status: pending
  - id: evaluate
    content: Verify/repair call-time evaluation of templated UDF bodies (EvalSqlUdfBody path in eval_expr_calls.cc + DuckDB transpiler templated emit) for ANY TYPE arguments across the input types bigquery-utils exercises.
    status: pending
  - id: fixtures
    content: Add focused conformance fixtures under conformance/fixtures/ for ANY TYPE scalar UDFs (smallest repro), then re-sync + re-triage bigquery-utils to migrate newly-passing fixtures.
    status: pending
  - id: docs
    content: Update docs/ENGINE_POLICY.md / ROADMAP.md / SHAPE_TRACKER.md to reflect ANY TYPE scalar UDF support status.
    status: pending
---

# BQUtils 03 — ANY TYPE templated scalar UDFs

## Why

Many bigquery-utils UDFs (migration `nullifzero`, `nvl`, `zeroifnull`, and numerous community ones) declare `ANY TYPE` parameters. The engine has a `TemplatedSQLFunction` code path but **no tests**, so these likely sit in `known_failing/` after plan 02. Closing this gap is the single biggest mover.

## Key files (from prior investigation)

- `backend/catalog/create_function_util.cc` — builds `TemplatedSQLFunction` for SQL-language bodies (templated `ANY TYPE`).
- `backend/catalog/udf_registry.cc` — per-project registration + replay into `GoogleSqlCatalog`.
- `backend/engine/coordinator/local_coordinator_engine.cc` — intercepts `RESOLVED_CREATE_FUNCTION_STMT`, registers function.
- `backend/engine/semantic/eval_expr_calls.cc:311-360` — templated call dispatch -> `EvalSqlUdfBody`.
- `backend/engine/duckdb/transpiler/transpiler_emit_function_call.cc:100-107` — templated UDF emit.
- `backend/engine/coordinator/route_classifier_visitor.cc:179-182` — promotes `SQLFunction`/`TemplatedSQLFunction` to semantic executor.
- `gateway/routines/ddl.go` — `scanSQLType` does **not** parse `ANY TYPE` today (only `ARRAY<...>`, `STRUCT<...>`, simple names).

## Steps

1. **Repro**: from plan 02's `known_failing/`, isolate `ANY TYPE` UDFs and capture whether failure is at CREATE (registration/analysis) or CALL (evaluation). Build the smallest hand-written fixture: `CREATE FUNCTION f(v ANY TYPE) AS (CAST(v AS INT64) + 1); SELECT f(41)`.
2. **Registration**: make `CREATE FUNCTION ... ANY TYPE` register cleanly (engine path) and ensure gateway `routines/ddl.go` parsing tolerates `ANY TYPE` if it must round-trip there.
3. **Evaluation**: verify call-time `EvalSqlUdfBody` resolves templated args against the concrete call types bigquery-utils uses (INT64/FLOAT64/STRING). Fix the semantic + DuckDB transpiler templated paths as needed.
4. **Fixtures**: add 2–3 minimal first-party fixtures under `conformance/fixtures/` (scalar `ANY TYPE`) so this is gated going forward, then re-run `task conformance:bqutils-sync` + triage (plan 02 step 3) to migrate the bigquery-utils `ANY TYPE` set into `passing/`.
5. **Docs**: update `docs/ENGINE_POLICY.md`, `ROADMAP.md`, `backend/engine/duckdb/transpiler/SHAPE_TRACKER.md`.

## Verify

```bash
task emulator:build-engine:bazel        # hygiene per rules
task conformance:run-fixture FIXTURE=conformance/fixtures/<new_any_type>.yaml
task conformance:bqutils-sync && task conformance:bqutils
```

Expect: new `ANY TYPE` first-party fixtures PASS; bigquery-utils `ANY TYPE` UDFs migrate from `known_failing/` to `passing/`.

## Out of scope

- JS UDFs (plan 04), UDAFs (plan 05), TVFs (plan 06).
- Durable cross-request UDF persistence through DuckDB storage (separate ROADMAP item).
