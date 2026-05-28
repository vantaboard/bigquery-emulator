# DuckDB Transpiler Procedural Support

## Goal

Promote scripting and procedural statement shapes by defining a statement-runner
layer above the single-query transpiler.

## Tracker Rows

- `ResolvedCreateProcedureStmt` from `skiplist` to `done`.
- `ResolvedCallStmt` from `skiplist` to `done`.
- `ResolvedAssignmentStmt` from `skiplist` to `done`.
- `ResolvedExecuteImmediateStmt` from `skiplist` to `done`.
- `ResolvedAssertStmt` from `skiplist` to `done`.
- `ResolvedSystemVariable` from `skiplist` to `done`.

## Implementation Plan

1. Add a scripting execution model outside the current one-shot
   `Transpile(ResolvedNode*) -> string` contract. Procedural statements need
   variables, statement sequencing, dynamic SQL, and error handling.
2. Model script variables and system variables explicitly, including type
   inference from analyzer output and parameter binding into child queries.
3. Implement `ASSERT` as a runtime predicate check with BigQuery-shaped error
   reporting.
4. Implement `EXECUTE IMMEDIATE` by recursively analyzing and executing the
   generated SQL under the same catalog and parameter context.
5. Implement stored procedure DDL and `CALL` after the catalog has somewhere to
   persist procedure bodies.

## Tests

- Multi-statement scripts with variable declaration/assignment and query use.
- `ASSERT` success and failure.
- `EXECUTE IMMEDIATE` with parameters.
- Procedure create/call/drop lifecycle if procedure persistence lands here.

## Done Criteria

- Procedural rows are `done`.
- The single-query transpiler remains focused; scripting orchestration is a
  separate layer that can still call into it for query statements.
- Error behavior is tested through the public query API.
