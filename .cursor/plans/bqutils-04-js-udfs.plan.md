---
name: BQUtils 04 — JavaScript (LANGUAGE js) UDFs
overview: Decide and implement engine handling for CREATE FUNCTION ... LANGUAGE js so the large bigquery-utils JS UDF corpus (community + GCS-library UDFs) can either run or surface a clean, asserted UNIMPLEMENTED. Extend the codegen pipeline to stop dropping JS UDFs once a path exists.
depends_on: [bqutils-02-engine-build-triage]
blocks: [bqutils-09-promote-to-gate]
est_effort: 2+ weeks (full execution) / 2-3 days (clean-rejection-only)
isProject: true
todos:
  - id: decide
    content: "Decide the target: (A) execute JS UDFs via an embedded JS runtime, or (B) make CREATE+CALL surface a clean, documented UNIMPLEMENTED. Reconcile docs/ENGINE_POLICY.md (says UNIMPLEMENTED) with the actual CREATE path that registers External_function metadata."
    status: pending
  - id: policy-reject
    content: "Option B: make LANGUAGE js CREATE FUNCTION (and call) consistently surface UNIMPLEMENTED instead of registering External_function silently; add a conformance error-fixture asserting the shape."
    status: pending
  - id: runtime
    content: "Option A (if chosen): integrate a JS engine for scalar JS UDF bodies; handle OPTIONS(library=...) GCS-hosted bundles (skip or stub). Large; gate behind explicit decision."
    status: pending
  - id: codegen
    content: Extend scripts/bigquery_utils/extract_test_cases.js + generator to stop excluding LANGUAGE js UDFs (emit to known_failing or a dedicated js/ bucket) once a path exists; keep GCS-library UDFs excluded unless runtime+lib stubbing lands.
    status: pending
  - id: docs
    content: Update docs/ENGINE_POLICY.md, ROADMAP.md, SHAPE_TRACKER.md with the chosen JS UDF posture.
    status: pending
---

# BQUtils 04 — JavaScript (LANGUAGE js) UDFs

## Why

A large fraction of `udfs/community/` are `LANGUAGE js` UDFs (e.g. `levenshtein`, many string/JSON helpers), several with `OPTIONS(library="${dataform.projectConfig.vars.gcsBucket}/...js")`. Plan 01 deliberately **excludes** these at codegen. This plan decides what to do with them.

## Current engine reality (verified)

- `docs/ENGINE_POLICY.md:105` states JS UDFs should surface `UNIMPLEMENTED` and not be persisted.
- But `backend/engine/coordinator/local_coordinator_engine.cc` registers the function **before** the control-op executor checks language; non-SQL/non-expression bodies become an `External_function` (`backend/catalog/create_function_util.cc:61-69`).
- Call-time: `route_classifier_visitor.cc` only promotes `SQLFunction`/`TemplatedSQLFunction`; `External_function` falls through to builtin lookup -> unknown-function error.
- Gateway `routines/ddl.go` can parse a quoted JS body but hardcodes `Language: "SQL"`.

So today JS UDFs are inconsistent: CREATE may "succeed" as metadata, CALL fails with a non-deterministic error.

## Decision fork

- **Option A — execute** JS scalar UDF bodies via an embedded JS runtime. High cost; also must handle `OPTIONS(library=...)` (GCS-hosted npm bundles) — likely skip/stub libraries, only run inline-body JS. This unlocks the most fixtures but is a multi-week effort.
- **Option B — clean rejection** (recommended first): make `LANGUAGE js` CREATE+CALL surface a consistent, documented `UNIMPLEMENTED` (matching policy), and add a conformance **error-fixture** asserting the shape. bigquery-utils JS UDFs stay excluded/known_failing, but the emulator behavior is honest and testable.

## Steps (Option B baseline; A behind explicit go-ahead)

1. Pick A or B (record in plan + docs). Reconcile the policy/code mismatch either way.
2. **B**: in the CREATE path, detect `language == "js"` and return `UNIMPLEMENTED` (do not register `External_function`); add `conformance/fixtures/specialized/error_create_function_js.yaml` asserting `expected.error`.
3. **A** (if chosen): integrate runtime; route JS scalar calls; stub/skip library-backed UDFs.
4. **Codegen**: update `scripts/bigquery_utils/extract_test_cases.js` to stop blanket-excluding JS — for B, route them to a `js/` skip report; for A, emit fixtures (inline-body only) to `known_failing/js/` then triage.
5. **Docs**: `docs/ENGINE_POLICY.md`, `ROADMAP.md`, `SHAPE_TRACKER.md`.

## Verify

```bash
task emulator:build-engine:bazel
# Option B:
task conformance:run-fixture FIXTURE=conformance/fixtures/specialized/error_create_function_js.yaml
# Option A:
task conformance:bqutils-sync && task conformance:bqutils
```

## Out of scope

- GCS-hosted JS library resolution (`js_libs.yaml` bundles) unless Option A explicitly funds it.
- ANY TYPE (plan 03), UDAF (plan 05), TVF (plan 06).
