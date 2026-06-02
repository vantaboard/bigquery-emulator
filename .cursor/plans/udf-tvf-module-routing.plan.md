---
name: ""
overview: ""
todos: []
isProject: false
---

# UDF / TVF / Module Routing

## Goal

Add local metadata and execution routes for SQL UDFs, table-valued
functions (TVFs), module-defined constants, and argument references.
Do **not** promise JavaScript UDF execution by default;
`specialized-feature-policy.plan.md` documents the JavaScript posture.

## Background

BigQuery exposes three function-like constructs the emulator must
recognize:

1. **SQL UDFs.** Inline expression-only functions
   (`CREATE FUNCTION f(x INT64) AS (x + 1)`). The body is a
   GoogleSQL expression resolved at registration; calls inline the
   body.
2. **JavaScript UDFs.** External-language UDFs. Running them
   locally requires a sandboxed JS runtime (V8 / QuickJS); see
   `specialized-feature-policy.plan.md` for the deliberate posture.
3. **TVFs.** Functions whose return type is a table
   (`CREATE TABLE FUNCTION f(x INT64) AS (SELECT ...)`). Calls
   substitute the body as a relational scan in the calling query.

Module-defined constants (`ResolvedConstant`) and UDF/TVF arguments
(`ResolvedArgumentRef` / `ResolvedArgumentDef`) live in the same
family.

## Dependencies

- `control-op-executor.plan.md` (the CREATE / DROP side of UDFs and
  TVFs).
- `semantic-executor-core.plan.md` (the call-side execution for SQL
  UDFs and TVFs).
- `engine-router-foundation.plan.md` (routing).

## Scope

Shapes this plan covers:

- `ResolvedCreateFunctionStmt` (SQL UDF body storage; routing of
  invocations is the call side covered here).
- `ResolvedCreateTableFunctionStmt` (TVF body storage).
- `ResolvedTVFScan` (TVF invocation as a relational scan).
- `ResolvedRelationArgumentScan` (TVF table-typed argument).
- `ResolvedArgumentRef` / `ResolvedArgumentDef` (UDF / TVF
  arguments).
- `ResolvedConstant` (module-defined constants).
- `ResolvedFunctionCall` invocation of a SQL UDF.
- Routing for JavaScript UDF calls: route to `unsupported` with a
  clear error, unless the deliberate JS posture from
  `specialized-feature-policy.plan.md` ships local JS execution.

## Implementation Plan

1. Add a UDF / TVF registry in `DuckDBStorage` that persists the
   function body (analyzer-resolved AST plus argument signature) to
   `catalog.duckdb`.
2. SQL UDFs: on invocation, the semantic executor inlines the body
   into the calling expression. Use the existing
   `EvalExpr(ResolvedExpr&, Environment&)` plumbing; bind UDF
   arguments into the local environment.
3. TVFs: on invocation, the semantic executor instantiates a scan
   from the body's analyzed `ResolvedQueryStmt` (with table-typed
   arguments substituted) and threads it through the coordinator
   as a sub-query. Composable shapes (TVF inside a fast-path
   query) route the whole top-level statement to the semantic
   executor.
4. Constants: load module-defined constant values at registration;
   `EvalExpr` resolves a `ResolvedConstant` to its bound value.
5. Argument refs: a `ResolvedArgumentRef` evaluates against the
   semantic executor's current scope (the UDF / TVF invocation
   frame).
6. JS UDFs: the registration is accepted (catalog metadata
   recorded), but invocation routes `unsupported` with an error
   identifying the unsupported language unless the JS sandbox lands
   per `specialized-feature-policy.plan.md`.

## Tests

- Per-shape unit tests under
  `backend/engine/semantic/udf/` and
  `backend/engine/semantic/tvf/`.
- Engine-level integration tests in `gateway/e2e/udf/`:
  - Inline SQL UDF: `CREATE FUNCTION add1(x INT64) AS (x + 1);
     SELECT add1(5);`.
  - SQL UDF referenced from a `WHERE` clause.
  - TVF: `CREATE TABLE FUNCTION top(n INT64) AS (SELECT * FROM t
     LIMIT n); SELECT * FROM top(3);`.
  - TVF with a table-typed argument.
  - Module-defined constant reference.
  - JS UDF rejected with the documented `unsupported` error.
- Conformance fixtures under `conformance/fixtures/udf/` and
  `conformance/fixtures/tvf/` pinning the route label.

## Done Criteria

- SQL UDFs and TVFs (with literal and table-typed arguments) work
  end-to-end through the gateway.
- Module-defined constants and argument refs resolve correctly.
- JS UDF rejection produces a clear, BigQuery-shaped error naming
  the unsupported language (or, if the JS sandbox lands, runs the
  body locally per `specialized-feature-policy.plan.md`).
- ROADMAP.md "DML / DDL" section's UDF / TVF bullet flips to `✅`.

## Notes & deferrals (subagent 13)

`refactor(semantic) dff56ea` extracted the script driver's
`VariableEnvironment` into the shared `semantic::FrameStack`
primitive at `backend/engine/semantic/frame_stack.{h,cc}`. The
script driver now reuses it; the UDF / TVF call-side executor
will reuse the same primitive (already wired into
`EvalContext::arguments`). No duplication.

Family 1 (`ResolvedConstant`) and Family 2 (`ResolvedArgumentRef`)
landed at `158e831` and `4d0950c`. Both rows are off
`status=planned` in `node_dispositions.yaml` / `SHAPE_TRACKER.md`;
both have unit-test coverage in `eval_expr_test.cc`. No
conformance fixture today because the gateway has no SQL surface
for registering module-defined constants or invoking UDFs through
the catalog round-trip — the per-shape executor behavior is pinned
by the unit tests; gateway-side fixtures land alongside the
storage round-trip in the follow-up subagent.

The remaining families (3–8) were **deferred** because they all
depend on a piece of infrastructure this subagent did not build:
a per-engine UDF / TVF / module registry that round-trips through
`DuckDBStorage`. The plan body calls this out explicitly; without
it, registration in one HTTP request cannot persist for invocation
in another. The "no silent approximation" rule prohibits faking
this with an in-memory registry that loses on engine restart.
Each deferred family keeps its `status=planned` row pointing at
this plan; surfaces an `UNIMPLEMENTED` envelope until the
follow-up lands.

| Family | Shape(s) | Disposition (unchanged) | Why deferred |
|---|---|---|---|
| 3 | JS UDF invocation (`ResolvedFunctionCall` on a JS UDF) | `unsupported` (target) | Invocation cannot fire without registration; the storage round-trip (family 4) is the prerequisite. Registration itself rejects with a structured JS-named error envelope; pin that in `specialized-feature-policy.plan.md` (plan 14, the next subagent). |
| 4 | `ResolvedCreateFunctionStmt` (SQL UDF body storage) | `control_op` (planned) | Needs a `DuckDBStorage` functions registry: persisted analyzer-resolved body + argument signature, plus a `GoogleSqlCatalog` adapter that re-publishes registered functions per-request. Per-request `TypeFactory` lifetime means the registry has to re-analyze (or store + replay) the body on each request. Non-trivial design + tests. |
| 5 | SQL UDF invocation (`ResolvedFunctionCall` on a SQL UDF) | `semantic_executor` (planned) | Depends on family 4 storage. The call-side mechanics — pushing a `FrameStack` frame, declaring each argument by name into the frame, evaluating the body's `ResolvedExpr` with `ctx.arguments = &frame`, popping — are already in place via this subagent's `EvalContext::arguments` wiring; what's missing is the `Function*` -> body retrieval. |
| 6 | `ResolvedCreateTableFunctionStmt` (TVF body storage) | `control_op` | Same pattern as family 4 but for a `ResolvedQueryStmt` body + table-typed argument signatures. Same storage round-trip blocker. |
| 7 | `ResolvedTVFScan` (TVF invocation as a relational scan) | `semantic_executor` (planned) | Depends on family 6 storage. Composable shapes (TVF inside a fast-path query) require the route classifier's compositional-fallback contract to promote the whole top-level statement to the semantic executor at planning time — wiring in this subagent's reach but unproductive without the body to invoke. |
| 8 | `ResolvedRelationArgumentScan` (TVF table-typed argument) | `semantic_executor` (planned) | Depends on family 7. Within a TVF body, the table-typed argument expands to the bound caller-side relation; needs the TVF executor's row-source plumbing first. |

For each deferred family the YAML / SHAPE_TRACKER row continues to
read `semantic_executor plan=udf-tvf-module-routing.plan.md
status=planned` (or `control_op plan=...` for the storage-side
rows). The next subagent picking this plan up should land family
4 first, then 5 on top, then 6+7+8 as a TVF triplet.

`ResolvedArgumentDef` (the signature-side definition node, not the
reference-site `ResolvedArgumentRef`) stays `status=planned`
because nothing today walks it — the analyzer carries it on the
`ResolvedCreateFunctionStmt` and the executor will reach it only
once family 4 (storage) lands.
