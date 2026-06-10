---
name: Parity 05 — Scripting control flow
overview: Make stored-procedure bodies and multi-statement scripts upstream-faithful - IF/WHILE/LOOP/REPEAT/FOR, SET assignment, EXECUTE IMMEDIATE, BEGIN...EXCEPTION...END with @@error.message, and RAISE - on top of the existing script_executor + FrameStack foundation.
est_effort: ~2 weeks
isProject: true
todos:
  - id: assignment
    content: "ResolvedAssignmentStmt (SET var = expr, SET (a, b) = (...)): evaluate on the semantic executor against the FrameStack; flip the (planned) row."
    status: completed
  - id: control-flow
    content: "IF/ELSEIF/ELSE, WHILE, LOOP/BREAK/CONTINUE, REPEAT, FOR...IN (query-driven iteration): structured control flow in script_executor.cc over the statement list GoogleSQL's script analyzer produces."
    status: completed
  - id: execute-immediate
    content: "ResolvedExecuteImmediateStmt: analyze + execute the dynamic SQL string through the coordinator at script runtime; support USING parameter binding and INTO variable capture."
    status: completed
  - id: exception-handling
    content: "BEGIN ... EXCEPTION WHEN ERROR THEN ... END: catch statement errors, populate @@error.message / @@error.statement_text (ResolvedSystemVariable on the semantic executor), execute the handler block; RAISE / RAISE USING MESSAGE re-raise semantics."
    status: completed
  - id: gateway-script-loop
    content: "Verify gateway script runner (gateway/handlers/script_runner.go + script_runner_engine.go) streams multi-statement child-job results per BigQuery's script job contract for the new statement kinds (each statement a child job; final SELECT result surfaced)."
    status: completed
  - id: fixtures-trackers
    content: Fixtures under conformance/fixtures/scripting/ (loop accumulation, dynamic DDL via EXECUTE IMMEDIATE, exception recovery with @@error.message); promote remaining bqutils stored_procedures goldens; flip SHAPE_TRACKER rows (ResolvedAssignmentStmt, ResolvedExecuteImmediateStmt, ResolvedSystemVariable) and update ENGINE_POLICY scripting row.
    status: completed
---

# Parity 05 — Scripting control flow

## Why

ROADMAP's scripting bullet is explicit about what is missing for
upstream-faithful procedure bodies: *"`EXECUTE IMMEDIATE`,
`WHILE`/`IF`/`LOOP`/`FOR`/`RAISE`/`EXCEPTION`, and `@@error.message`
remain open"*. Real-world stored procedures and migration scripts
(dbt hooks, bqutils procedures) lean on these constantly. The
foundation already exists: `DECLARE`/`SET`-via-`CREATE CONSTANT`/`CALL`
/`BEGIN...END` sequencing, `FrameStack` scoping, `script_executor.cc`
dispatch, and the gateway engine-first script path.

## Key files

- [`backend/engine/coordinator/script_executor.cc`](../../backend/engine/coordinator/script_executor.cc) + the new `script_executor_set.{h,cc}` — statement dispatch loop
- [`backend/engine/semantic/script/`](../../backend/engine/semantic/script/) — `assert_stmt.cc`, `declare_stmt.cc`, FrameStack home
- [`gateway/handlers/script_runner.go`](../../gateway/handlers/script_runner.go), `script_runner_engine.go` — gateway-side script job orchestration
- [`backend/engine/coordinator/local_coordinator_engine.cc`](../../backend/engine/coordinator/local_coordinator_engine.cc) — `ExecuteScriptViaAnalyzeNext`
- `conformance/fixtures/scripting/`

## Steps

1. Confirm how GoogleSQL's script surface reaches the engine today
   (`ExecuteScriptViaAnalyzeNext` + `ResolvedMultiStmt`): control-flow
   statements are parser-level constructs — determine whether the
   AnalyzeNext loop yields them as script statements needing a local
   script interpreter (likely) and build the interpreter loop around
   the existing per-statement dispatch.
2. Land in dependency order: `SET` assignment → `IF` → `WHILE`/`LOOP`/
   `BREAK`/`CONTINUE` → `FOR...IN` (needs query-result iteration; reuse
   the coordinator row stream) → `EXECUTE IMMEDIATE` → `EXCEPTION` +
   `@@error.*` → `RAISE`.
3. Scope rules: each block pushes a `FrameStack` frame; `DECLARE` only
   at block start (analyzer enforces); loop variables per BigQuery
   scoping docs.
4. `EXECUTE IMMEDIATE` re-enters the coordinator with the dynamic
   string — make sure route classification and the UNIMPLEMENTED
   envelope behave identically to top-level queries.
5. Gateway: ensure child-job statistics and the final result selection
   stay BigQuery-shaped for loops (many child statements).
6. Fixtures + tracker flips per construct; promote bqutils
   `known_failing/` procedure goldens that these unblock.

## Verify

```bash
task emulator:build-engine:bazel
task conformance:run
task conformance:bqutils            # stored-procedure goldens
go test ./gateway/handlers/ -run Script
task lint:fix && task lint:run
task bazel:shutdown && task bazel:status
```

## Out of scope

- Durable procedure/UDF persistence across restarts — plan 08
- `ResolvedCreateConstantStmt` catalog-backed module constants (non-script) — note in SHAPE_TRACKER, defer
- Transactions (`BEGIN TRANSACTION`) — unscoped; surface UNIMPLEMENTED with a named family if encountered
