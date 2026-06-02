---
name: ""
overview: ""
todos:
  - id: todo-1780309600001-subagent-01
    content: "execution-disposition-registry.plan.md: launch background subagent per Subagent Dispatch Workflow."
    status: completed
  - id: todo-1780309600002-subagent-02
    content: "engine-router-foundation.plan.md: launch background subagent per Subagent Dispatch Workflow."
    status: completed
  - id: todo-1780309600003-subagent-03
    content: "duckdb-fast-path-stabilization.plan.md: launch background subagent per Subagent Dispatch Workflow."
    status: completed
  - id: todo-1780309600004-subagent-04
    content: "duckdb-polyfill-udf-library.plan.md: launch background subagent per Subagent Dispatch Workflow."
    status: completed
  - id: todo-1780309600005-subagent-05
    content: "control-op-executor.plan.md: launch background subagent per Subagent Dispatch Workflow."
    status: completed
  - id: todo-1780309600006-subagent-06
    content: "semantic-executor-core.plan.md: launch background subagent per Subagent Dispatch Workflow."
    status: completed
  - id: todo-1780309600007-subagent-07
    content: "semantic-functions-compliance.plan.md: launch background subagent per Subagent Dispatch Workflow."
    status: completed
  - id: todo-1780309600008-subagent-08
    content: "array-struct-semantic-path.plan.md: launch background subagent per Subagent Dispatch Workflow."
    status: completed
  - id: todo-1780309600009-subagent-09
    content: "dml-local-executor.plan.md: launch background subagent per Subagent Dispatch Workflow."
    status: completed
  - id: todo-1780309600010-subagent-10
    content: "cte-subquery-routing.plan.md: launch background subagent per Subagent Dispatch Workflow."
    status: completed
  - id: todo-1780309600011-subagent-11
    content: "advanced-relational-routing.plan.md: launch background subagent per Subagent Dispatch Workflow."
    status: in_progress
  - id: todo-1780309600012-subagent-12
    content: "procedural-scripting-executor.plan.md: launch background subagent per Subagent Dispatch Workflow."
    status: pending
  - id: todo-1780309600013-subagent-13
    content: "udf-tvf-module-routing.plan.md: launch background subagent per Subagent Dispatch Workflow."
    status: pending
  - id: todo-1780309600014-subagent-14
    content: "specialized-feature-policy.plan.md: launch background subagent per Subagent Dispatch Workflow."
    status: pending
  - id: todo-1780309600015-subagent-15
    content: "storage-read-write-api-plan.plan.md: launch background subagent per Subagent Dispatch Workflow."
    status: pending
  - id: todo-1780309600016-subagent-16
    content: "conformance-routing-matrix.plan.md: launch background subagent per Subagent Dispatch Workflow."
    status: pending
  - id: todo-1780309600017-subagent-17
    content: "migration-cleanup-docs.plan.md: launch background subagent per Subagent Dispatch Workflow."
    status: pending
isProject: false
---

# Local Execution Roadmap Index

## Goal

Own the execution order, terminology, and done criteria for the
**local multi-strategy execution coordinator** that lives behind
`backend/engine/engine.h`. Every user-visible resolved-AST shape must
end up with one of six routing dispositions
(`duckdb_native`, `duckdb_rewrite`, `duckdb_udf`, `semantic_executor`,
`control_op`, `unsupported`) plus conformance coverage that pins the
chosen route.

This index is the single plan ROADMAP.md links to. The 17 sibling
plans in this directory each own one route or one shape family; this
file decides which order they land in and what "done" means across
the set.

## Terminology

The routing vocabulary (matching `docs/ENGINE_POLICY.md` and the
SHAPE_TRACKER):

- `duckdb_native` — lowers to DuckDB SQL whose semantics already match
  BigQuery exactly.
- `duckdb_rewrite` — lowers to DuckDB SQL via a deliberate structural
  rewrite (struct/array shape rewrites, JSON operator mapping, ...).
- `duckdb_udf` — lowers to DuckDB SQL that calls a DuckDB UDF/macro
  registered at engine startup, where the UDF body owns the
  BigQuery-specific semantics.
- `semantic_executor` — runs on the local row/value interpreter
  instead of DuckDB SQL evaluation. DuckDB is still the row source.
- `control_op` — DDL / metadata / catalog op routed through the
  storage layer; bypasses query execution entirely.
- `unsupported` — deliberately out of scope locally. Returns a
  BigQuery-shaped `UNIMPLEMENTED`.

## Execution Order

The order minimizes rework: the registry and router land first so the
remaining plans can target an explicit dispatch surface; the
already-strong DuckDB fast path is stabilized before the new routes
are layered on; semantic executor, control ops, and DML follow.

1. `execution-disposition-registry.plan.md`
   - Retire `done | skiplist | not_started` (tracker) and
     `kMap | kFallback | kSkiplist` (functions table) in favor of the
     six-route vocabulary; emit machine-readable disposition metadata
     for node kinds and functions.
2. `engine-router-foundation.plan.md`
   - Install a route classifier behind `Engine::Analyze` /
     `Engine::ExecuteQuery`; split the in-process implementation into
     route-aware components without changing the gRPC surface.
3. `duckdb-fast-path-stabilization.plan.md`
   - Preserve and tighten current DuckDB wins; shapes that already
     route `duckdb_native` / `duckdb_rewrite` get conformance
     coverage and stricter typing.
4. `duckdb-polyfill-udf-library.plan.md`
   - Move suitable BigQuery functions from SQL-text rewrites into
     DuckDB UDFs/macros (date/time, regex, string, numeric, JSON,
     SAFE-function behavior).
5. `control-op-executor.plan.md`
   - DDL and metadata/control statements route through storage /
     catalog handlers, not through DuckDB SQL.
6. `semantic-executor-core.plan.md`
   - Local row/array/value interpreter for BigQuery-specific
     expressions: scalar-only SELECT, expression evaluation, parameter
     binding, CASE/IF, strict NULL semantics, exact error behavior.
7. `semantic-functions-compliance.plan.md`
   - BigQuery-exact functions move to the semantic executor: date
     arithmetic edge cases, SAFE semantics, JSON exactness, HLL/NET/
     KEYS stubs or local implementations, precise error surfaces.
8. `array-struct-semantic-path.plan.md`
   - `UNNEST WITH OFFSET`, lateral array scans, multi-array zip,
     outer array scans, anonymous/nested STRUCT edge cases, deep
     STRUCT mutation — wherever DuckDB's LIST/STRUCT model diverges.
9. `dml-local-executor.plan.md`
   - `INSERT`, `UPDATE`, `DELETE`, and the harder `MERGE` branches
     route through storage-aware local execution; owns
     `numDmlAffectedRows` correctness.
10. `cte-subquery-routing.plan.md`
    - Non-recursive CTEs and subqueries route to DuckDB when
      composable; correlated / BigQuery-evaluation-order shapes
      route to the semantic executor.
11. `advanced-relational-routing.plan.md`
    - Recursive CTEs, pivot / unpivot, MATCH_RECOGNIZE, pipe
      operators, graph query scans, optimizer barriers, deferred
      computed columns — classified by route.
12. `procedural-scripting-executor.plan.md`
    - Local script interpreter for BigQuery scripting (variables,
      assignment, CALL, ASSERT, EXECUTE IMMEDIATE, procedures,
      statement sequencing, script-level errors).
13. `udf-tvf-module-routing.plan.md`
    - Local metadata + execution routes for SQL UDFs, TVFs,
      constants, argument refs, and module-like catalog objects.
14. `specialized-feature-policy.plan.md`
    - Local-only posture for BigQuery ML, GIS, differential privacy,
      anonymized aggregation, networking, key-management, HLL, proto,
      measure, graph, and other specialized families: "local
      implementation now," "deterministic stub with BigQuery-shaped
      error," or "unsupported by design."
15. `storage-read-write-api-plan.plan.md`
    - Continue Storage Read API; add Storage Write API; writes share
      the storage-aware path used by DML and `tabledata.insertAll`.
16. `conformance-routing-matrix.plan.md`
    - Fixtures assert which route served a query; route labels in
      fixture output so passing rows do not hide accidental drift.
17. `migration-cleanup-docs.plan.md`
    - Final pass: remove stale "DuckDB-only," "skiplist," "fallback,"
      "all shapes become transpiler work" references from docs and
      plan files.

## Shared Rules

- **Single planned route per shape.** A `ResolvedAST` node kind picks
  exactly one route. The router never silently retries on a different
  route at runtime.
- **Disposition + emit + conformance land together.** A row only
  flips from `unsupported` (or from "(planned)") to a real route when
  the matching implementation lands in the same commit as the
  conformance fixture(s) that pin it.
- **Compositional fallback at planning time, not runtime.** If the
  DuckDB fast path could lower most of a query but a leaf node is
  `semantic_executor`, the surrounding shape gets promoted to the
  semantic executor at planning time. We do not mix strategies
  mid-query.
- **Route labels are observable in tests.** Once
  `conformance-routing-matrix.plan.md` lands, every fixture asserts
  the route it expected to use; route drift fails CI even if rows
  still match.
- **No silent approximation.** Promoting a shape requires a real
  local implementation, not a "close enough" DuckDB rewrite. Shapes
  with no planned local strategy stay on `unsupported` and surface
  a BigQuery-shaped `UNIMPLEMENTED`.
- **`unsupported` is owned by `specialized-feature-policy.plan.md`.**
  That plan documents which specialized families are
  unsupported-by-design vs. stubbed vs. eventually-local.

## Done Criteria

- Every row in `backend/engine/duckdb/transpiler/SHAPE_TRACKER.md`
  has a non-`(planned)` route disposition, or its planned route's
  plan has not landed yet (in which case `(planned)` is acceptable
  and the engine returns `UNIMPLEMENTED`).
- Every entry in `backend/engine/duckdb/transpiler/functions.yaml`
  has a non-`kFallback` disposition (after
  `execution-disposition-registry.plan.md` lands, this is the
  same constraint as the tracker rule above).
- Conformance fixtures assert route labels on every passing query
  shape (after `conformance-routing-matrix.plan.md` lands).
- ROADMAP.md and `docs/ENGINE_POLICY.md` link only to this index
  from their "Execution plans" / "Routes" sections.
- No remaining docs or plan files reference the retired DuckDB-only
  transpiler plan set (validated by
  `migration-cleanup-docs.plan.md`).

## Subagent Dispatch Workflow

Each `todos:` entry above expands to the five-phase workflow below
against the plan file named in the entry. Run the phases strictly in
order; do not advance to the next todo until the current todo has
cleared **Phase 5 (Validate)**. If any phase fails, leave the todo
`in_progress` and resolve the failure before moving on — never
silently skip ahead.

### Phase 1 — Pre-flight (parent agent)
- `rtk git status --short` — confirm a clean working tree (anything
  uncommitted from a prior subagent must be reconciled first).
- `rtk free -h | head -2` and `rtk task bazel:status` — refuse to
  dispatch if MemAvailable < 4 GiB or any non-`emulator_main`
  bazel/clang processes are alive (the persistent
  `emulator_main` PID is the user's; leave it untouched).
- Read the target plan file and confirm its done-criteria are still
  current.

### Phase 2 — Launch (parent agent)
- Dispatch a single subagent via `Task` with
  `run_in_background: true`. Plan content is the source of truth;
  the prompt summarises non-obvious context (the optimization
  posture below, the per-plan "Notes & deferrals" inherited from
  earlier subagents). Do not dispatch a second subagent for the
  same plan in parallel.

### Phase 3 — Monitor (parent agent)
- Wait for the subagent's completion notification. Do not poll
  reflexively; multitask on tangential work or end the turn and let
  the notification surface.

### Phase 4 — Cleanup (parent agent, MANDATORY after every subagent)
- Run the full cleanup block from
  `.cursor/rules/process-hygiene.mdc` (catalog audit, bazel
  shutdown, emulator/gateway/runner kill, docker compose down if
  applicable). Confirm `(clean)` for every catalog before moving on.
- This is the boundary at which the warm-daemon-per-plan posture
  ends. Inside the subagent the daemon stays warm; here we release
  the multi-GB JVM heap so the next subagent starts in a known
  state.

### Phase 5 — Validate (parent agent)
- Read the subagent's commit list (`git log`), spot-check the
  done-criteria from the target plan, and decide:
  - **Pass:** mark the todo `completed`, advance to the next.
  - **Partial-but-acceptable:** document the deferral in the next
    subagent's prompt, mark completed, advance. Use this when the
    "no silent approximation" rule forced the subagent to defer
    scope (precedent: plans 4, 5, 7).
  - **Fail:** leave the todo `in_progress`, dispatch a focused
    follow-up subagent that targets only the gap.

### Build optimization posture (post-`1c7cf99` + `0d11cfb`)

The shared `task bazel:build` / `task bazel:test` / `task bazel:coverage`
wrappers now resolve `GOOGLESQL_SOURCE` to `--config=googlesql-prebuilt`
by default — sharing the disk-cache ABI lane with
`task emulator:build-engine:bazel` so cold scoped tests skip the
~8K-TU GoogleSQL recompile. The wrappers also trap signals only
(`INT TERM`, not `EXIT`), so the Bazel daemon stays warm between
back-to-back wrapper calls inside one subagent (the analysis cache
covers ~7700 targets; reusing it saves 5–10 s per invocation).
`.bazelrc` quiets per-action progress to once / 15 s.

**Every dispatched subagent prompt MUST tell the subagent to:**

- Use `task bazel:test TARGETS=...` (and `bazel:build` / `bazel:coverage`)
  directly. Do NOT pass `--config=googlesql-source` unless there is
  a specific reason — default `prebuilt` is correct and is what the
  engine build uses.
- Do NOT call `task bazel:shutdown` between scoped test invocations
  within your own work. Only run it ONCE at end-of-work as part of
  the cleanup block.
- Expect the FIRST scoped test after the optimization landed to pay
  a one-time disk-cache rebuild (the wrapper ABI lane switched from
  source to prebuilt); subsequent calls within the same subagent
  are fast.
- Continue to defer `task conformance:fastpath` end-to-end live-run
  when the local `bin/emulator_main` predates the subagent's
  changes — CI exercises it. (Unchanged from prior subagents.)
- Follow `.cursor/rules/process-hygiene.mdc` for the per-spawn
  audit, mid-loop poll, and cleanup block; the warm-daemon posture
  is documented in `.cursor/rules/bazel-process-hygiene.mdc`.

End-of-plan shutdown still happens via the parent's **Phase 4**
cleanup block; warm-daemon-per-plan applies only inside a subagent's
own work, never across subagent boundaries.
