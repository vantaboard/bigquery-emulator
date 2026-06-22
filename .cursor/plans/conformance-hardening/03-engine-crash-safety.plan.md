# 03 — Crash-safety as a tested invariant (CHECK→Status audit + fuzz/soak)

- **Series:** conformance-hardening (plans 01–08). Run sequentially.
- **Sequencing:** third. Uses the `expect_alive` primitive from plan 02 for the
  soak assertion. Do plan 02 first.
- **Priority:** P1 (a single bad request taking down the engine for all clients
  is the worst failure class observed).

## Why this exists (origin)

The reported abort:

```
F0000 ... map_util.h:362] Check failed: InsertIfNotPresent(m, value) duplicate value: profiles
*** Check failure stack trace: ***
  ... googlesql_base::InsertOrDie<>()
  ... googlesql::SimpleCatalog::AddTable()
  ... bigquery_emulator::backend::catalog::ReplayViewsIntoCatalog()
  ... bigquery_emulator::backend::catalog::GoogleSqlCatalog::GoogleSqlCatalog()
  ... bigquery_emulator::frontend::QueryService::DryRun()
time=... level=WARN msg="engine subprocess exited" err="signal: aborted"
```

A user-reachable catalog state (a duplicate view/table name during catalog
replay, reached via an ordinary `DryRun`) hit an absl `CHECK`/`InsertOrDie` and
**aborted the whole engine subprocess** — killing every in-flight and future
request, not just the offending one. `CHECK` is correct for genuine "cannot
happen" internal invariants; it is **never** correct for state a client can
drive. This plan (a) audits request-reachable fatal checks and converts them to
`absl::Status`, and (b) makes "the engine never aborts on any input" a tested,
CI-enforced invariant.

## Current state (grounded)

- Fatal-on-duplicate insertion helpers are used across the catalog layer; grep
  shows `InsertOrDie` / `InsertIfNotPresent` / `AddTable` call sites in
  `backend/catalog/view_registry.cc`, `backend/catalog/googlesql_catalog.cc`,
  `backend/catalog/udf_registration_catalog.cc`,
  `backend/catalog/tvf_registry.cc`, `backend/catalog/udf_registry.cc`,
  `backend/catalog/emulator_ml_tvf_extensions.cc`.
- The exact `ReplayViewsIntoCatalog` symbol from the crash stack may have been
  renamed/moved since the reported version — **grep the current tree** for the
  catalog view-replay path (`rg -n "ReplayViews|AddTable|InsertOrDie" backend/catalog`)
  and treat the email stack as the shape, not a literal line number.
- There is no test asserting the engine survives adversarial catalog input;
  crashes surface only as a gateway "engine subprocess exited" warning.

## Goal / done-criteria

1. **Audit:** every `LOG(FATAL)` / `CHECK*` / `ZETASQL_CHECK*` / `InsertOrDie` /
   `InsertIfNotPresent`-as-die reachable from a request handler
   (`QueryService::DryRun`/`Query`, catalog construction, view/UDF/TVF
   registration) is either proven unreachable-by-client or converted to a
   returned `absl::Status` (→ gRPC `InvalidArgument`/`AlreadyExists`/`Internal`).
2. The specific duplicate-name-during-replay path returns a structured error
   (e.g. `AlreadyExists: catalog object "profiles" already registered`) instead
   of aborting.
3. A **fuzz/soak test** that throws adversarial DDL/catalog ops at a persistent
   engine and asserts: process stays alive, every response is a structured gRPC
   error, never `signal: aborted`.
4. A guideline doc/rule: "no `CHECK`/`*OrDie` on client-controlled state in
   request paths."

## Implementation steps

### Step 1 — Enumerate request-reachable fatal checks
- `rg -n "ZETASQL_CHECK|\\bCHECK\\b|CHECK_|LOG\\(FATAL\\)|InsertOrDie|InsertIfNotPresent|QCHECK|DIE" backend/ frontend/`
- For each hit, classify: (a) unreachable internal invariant (leave, add a
  comment), or (b) reachable from a handler (convert). Start with the catalog
  construction + view/UDF/TVF registration paths named in the crash stack.

### Step 2 — Convert reachable dies to Status
- Replace fatal inserts with checked inserts that return
  `absl::Status`/`absl::StatusOr`, propagate up through
  `GoogleSqlCatalog` construction and `QueryService::DryRun`/`Query`, and map to
  the gateway's existing gRPC→HTTP error envelope (so the client sees a 400/409,
  matching BigQuery's "Already Exists"/"Invalid" shape).
- Idempotency: a `CREATE OR REPLACE VIEW` / re-authorize that re-registers an
  existing name should **replace**, not error or abort — verify the replay path
  dedups by name rather than blindly `AddTable`.

### Step 3 — Fuzz / soak test (C++ unit + harness)
- C++ unit (`backend/catalog/..._crash_safety_test.cc`): drive
  `GoogleSqlCatalog` construction + registration with duplicate names, malformed
  qualified names, empty/oversized identifiers; assert it returns `!ok()` and
  never aborts (death-test style: assert **no** death).
- Cross-layer soak: a `conformance/sessions/` fixture (plan 02) that issues a
  long randomized stream of CREATE/REPLACE/DROP VIEW + dataset ACL PATCH +
  DryRun, with `expect_alive: true` after each step. This is the end-to-end
  guard.
- Optionally add a libFuzzer/`go test -fuzz` target seeding malformed DDL
  strings into the query path if the build supports it.

### Step 4 — Guardrail
- Add `.cursor/rules/no-fatal-on-client-state.mdc` (or a section in an existing
  C++ rule): request-path code must return `absl::Status`, not `CHECK`/`*OrDie`,
  on any client-controllable condition. Reference this plan.

## Tests
- `backend/catalog/..._crash_safety_test.cc` (no-death + Status assertions).
- `conformance/sessions/catalog_soak.yaml` with `expect_alive`.
- Regression: the duplicate-view sequence from plan 02's
  `authorize_view_repeat` flips from red (abort) to green (structured error or
  clean idempotent replace).

## Process hygiene (repo rules)
This plan **builds the C++ engine** (`task emulator:build-engine:bazel`) and runs
scoped bazel tests. Strictly follow `.cursor/rules/bazel-process-hygiene.mdc`
(single invocation, throttled jobs, warm daemon within the plan) and
`.cursor/rules/process-hygiene.mdc` (pre-spawn audit, mid-loop poll if a build
> 5 min, cleanup block + `task bazel:status` == `(clean)` at the end).

## Out of scope
- Broad refactor of every internal invariant — only client-reachable paths.
- Restart durability (plan 08) and the session primitive itself (plan 02).

## Touch list
`backend/catalog/view_registry.cc`, `backend/catalog/googlesql_catalog.cc`,
`backend/catalog/udf_registration_catalog.cc`, `backend/catalog/tvf_registry.cc`,
`backend/catalog/udf_registry.cc`, the view-replay path,
`frontend/handlers/` (Status→gRPC mapping if needed),
new `backend/catalog/*_crash_safety_test.cc` + its `BUILD.bazel` entry,
`conformance/sessions/catalog_soak.yaml`, new rule doc.
