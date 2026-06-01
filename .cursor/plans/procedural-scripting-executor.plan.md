---
name: ""
overview: ""
todos: []
isProject: false
---

# Procedural Scripting Executor

## Goal

Implement a local script interpreter for BigQuery's scripting
constructs. Variables, assignment, `CALL`, `ASSERT`, `EXECUTE
IMMEDIATE`, procedure bodies, statement sequencing, and script-level
error handling all run on the semantic executor.

## Background

BigQuery scripting (`DECLARE`, `SET`, `BEGIN ... END`, `IF`,
`WHILE`, `LOOP`, `FOR`, `BREAK`, `CONTINUE`, `RAISE`,
`CALL`, `ASSERT`, `EXECUTE IMMEDIATE`, `CREATE PROCEDURE`) is
analyzer-resolved but currently surfaces `UNIMPLEMENTED` on every
node. The DuckDB-only roadmap planned to lower scripting into
DuckDB SQL where possible. That is the wrong abstraction —
scripting is sequential statement execution with mutable variable
state, and that does not belong in a SQL transpiler.

The semantic executor is the right host because it already evaluates
expressions and has a per-statement evaluation loop. This plan adds
a script-level driver on top.

## Dependencies

- `semantic-executor-core.plan.md` (expression evaluation, type
  system, error surfaces).
- `engine-router-foundation.plan.md` (routing).
- `dml-local-executor.plan.md` (script statements that mutate
  storage call into the DML executor).
- `control-op-executor.plan.md` (script statements that touch
  metadata call into the control-op executor).

## Scope

Shapes this plan covers:

- `ResolvedCallStmt`
- `ResolvedAssignmentStmt`
- `ResolvedExecuteImmediateStmt`
- `ResolvedAssertStmt`
- `ResolvedCreateProcedureStmt` (procedure body storage; invocation
  runs through `CALL`)
- `ResolvedSystemVariable` (`@@error.message`,
  `@@last_statement_type`, ...)
- The script-level driver that sequences statements and threads
  variable / error state.

## Implementation Plan

1. Add `backend/engine/semantic/script/` with `script_driver.{h,cc}`
   and per-statement files.
2. Implement the variable-state environment: a stack of frames
   keyed by `(scope, name)` with `Value` payloads.
3. Implement statement sequencing: `BEGIN ... END` blocks, `IF` /
   `WHILE` / `LOOP` / `FOR` control flow, `BREAK` / `CONTINUE`,
   `RAISE` / nested `EXCEPTION` handlers.
4. Implement `DECLARE` / `SET` (assignment) as updates to the
   variable environment.
5. Implement `EXECUTE IMMEDIATE` by re-analyzing the literal SQL
   string and routing the resulting `ResolvedAST` through the
   coordinator like any other statement (the result feeds back into
   the script's variable environment if `INTO` is present).
6. Implement `CALL` by looking up the registered procedure body and
   running it as a nested script-driver invocation with bound
   arguments.
7. Implement `ASSERT` as a boolean evaluation that raises a
   BigQuery-shaped `assertionFailed` error on false.
8. Implement `CREATE PROCEDURE` as a control-op that stores the
   procedure body for later `CALL`.
9. Implement `ResolvedSystemVariable` reads against the executor's
   per-script state.

## Tests

- Per-statement unit tests under
  `backend/engine/semantic/script/`.
- Engine-level integration tests in `gateway/e2e/scripting/`:
  - `DECLARE x INT64 DEFAULT 0; SET x = x + 1; SELECT x`.
  - `BEGIN ... END` with nested `IF` / `WHILE`.
  - `EXECUTE IMMEDIATE 'SELECT @p' USING 1 AS p`.
  - `CREATE PROCEDURE p() BEGIN INSERT INTO t VALUES (1); END;
     CALL p();`.
  - `ASSERT x > 0`.
  - `RAISE USING MESSAGE = '...'` then `EXCEPTION WHEN ERROR THEN`.
- Conformance fixtures under `conformance/fixtures/scripting/`
  pinning the route label to `semantic_executor`.

## Done Criteria

- Every shape in this plan's "Scope" list runs locally with the
  documented BigQuery semantics.
- Procedure storage round-trips through `DuckDBStorage` (via the
  control-op executor for the registration side).
- ROADMAP.md "DML / DDL" section's scripting bullet flips to `✅`.
