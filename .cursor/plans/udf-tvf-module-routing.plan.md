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
