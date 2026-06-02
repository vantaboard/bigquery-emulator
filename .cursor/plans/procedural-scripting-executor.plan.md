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

## Status (2026-06-01)

Landed family-by-family in the twelfth subagent of the
local-execution-roadmap (precedent: plans 4, 5, 7, 8, 9, 10, 11).
Routing + the smallest evaluator surface land together; everything
else is deferred to focused follow-up subagents of this plan.

| Family | Shapes | Disposition | Commit |
|--------|--------|-------------|--------|
| 1 | Script driver scaffold + variable environment (`backend/engine/semantic/script/script_driver.{h,cc}`) | `semantic_executor` -- foundation only; `VariableEnvironment` is a stack of `BEGIN..END` frames keyed by `(scope, lower(name))` with `Value` payloads. Innermost binding wins; same-frame redeclaration rejected; case-insensitive identifier match. | `5a54dbb` |
| 5 | `ResolvedAssertStmt` (`ASSERT <expr> [AS '<msg>']`) | `semantic_executor` -- disposition row promoted (no `status=planned`). `script::ExecuteAssert` evaluates the BOOL predicate via the existing semantic-executor expression evaluator and surfaces `Assertion failed: <desc>` / `Assertion failed` on FALSE / NULL with `kInvalidArgument` (BigQuery REST `invalidQuery`). The gateway routes the statement through `ExecuteDdl` (`StatementClass::kDdl`) so ASSERT joins every other no-row-stream statement on the same dispatch path. | `b6a8298` |

Deferred to focused follow-up subagents of this plan. Each row's
disposition stays `semantic_executor plan=procedural-scripting-
executor.plan.md status=planned` so the classifier surfaces the
structured `notImplemented` envelope rather than silently
approximating; a future subagent owns the executor work without
re-deriving the route.

- **Family 2: `ResolvedAssignmentStmt` (`DECLARE` / `SET`).** A
  single `AnalyzeStatement` call hands back one `ResolvedAssignmentStmt`,
  but the variable state must persist across statements for the
  scripting contract to be useful. The gateway today calls
  `Engine::Execute*` per-request; cross-request state requires a
  session abstraction the gateway does not yet model. Until that
  lands, a one-shot SET would silently lose the binding on the
  next request -- the plan's "no silent approximation" rule
  forbids it. Tracked by this plan + a future session-scope
  subagent.
- **Family 3: `BEGIN ... END` statement sequencing.** Requires
  `googlesql/scripting/script_executor.h` -- the multi-statement
  script analyzer. The coordinator's per-statement
  `AnalyzeStatement` cannot handle the script form. Tracked by
  this plan; the script driver in Family 1 is the call site.
- **Family 4: `IF` / `ELSEIF` / `ELSE`.** Same blocker as Family
  3 (script-level analyzer not in place).
- **Family 6: `WHILE` / `LOOP` / `BREAK` / `CONTINUE`.** Same
  blocker as Family 3.
- **Family 7: `RAISE` / `EXCEPTION` handlers.** Needs the script
  driver's error-handler stack (the variable environment carries
  values; an error stack carrying `(@@error.message,
  @@error.stack_trace, ...)` is a separate surface). Same script-
  analyzer blocker as Family 3.
- **Family 8: `ResolvedExecuteImmediateStmt` (`EXECUTE IMMEDIATE
  '<sql>' [INTO <vars>] [USING <args>]`).** Re-analyzing the
  literal SQL and routing the resulting `ResolvedAST` through the
  coordinator means injecting a coordinator pointer into the
  semantic executor; that is a circular dependency the plan
  explicitly defers. The `INTO` clause additionally needs the
  cross-request variable state Family 2 is blocked on.
- **Family 9: `ResolvedCreateProcedureStmt`.** The storage layer
  has no procedure-CRUD surface today -- `Storage` (in
  `backend/storage/storage.h`) exposes table CRUD only. Adding
  `Storage::{Create,Get,Drop}Procedure` is a meaningful storage
  primitive; landing it routing-only would leave the body
  unreachable on a process restart, which violates the "no silent
  approximation" rule. The control-op handler skeleton at
  `backend/engine/control/create_procedure.{h,cc}` is part of
  this family and lands once the storage surface exists. Tracked
  by this plan + a follow-up storage-side subagent.
- **Family 10: `ResolvedCallStmt`.** Depends on Family 9 -- the
  procedure body has no registration surface to look up against
  yet.
- **Family 11: `FOR` loop.** Row-iteration form; depends on the
  script-level analyzer (Family 3 blocker) plus the
  `EvalContext::columns` integration plan 8's `9b2bde1` landed
  for per-statement scope (the script-scope variant lives here).
- **Family 12: `ResolvedSystemVariable` (`@@error.message`,
  `@@last_statement_type`, ...).** Reads against per-script error
  state. The state is owned by Family 7's error stack; without
  that the read surface has nothing to read from. Disposition row
  stays `semantic_executor status=planned`.

No silent approximation (the plan's hard rule): every deferred
family's `status=planned` row keeps the classifier surfacing the
structured `notImplemented` envelope until its owning subagent
lands the executor.

## Coordinator wiring posture

The coordinator's `MakeAnalyzerOptions` / `MakeAnalyzerOptionsAllStatements`
were sufficient for ASSERT (`MakeAnalyzerOptionsAllStatements` already
sets `SetSupportsAllStatementKinds`, which covers `RESOLVED_ASSERT_STMT`).
Future scripting families that need additional analyzer rewrites
disabled (e.g. RESOLVED-CALL-STMT support) should follow the
plan-11 `cf05e19` precedent: extend the allowed-output-kinds set
in `MakeAnalyzerOptions` and pre-dispatch the relevant root kinds
inside `LocalCoordinatorEngine::ExecuteQuery` before they hit any
executor that does not understand them. The scaffold for that
wiring is unchanged here because ASSERT goes through `ExecuteDdl`,
which already uses the all-statement-kinds analyzer options.
