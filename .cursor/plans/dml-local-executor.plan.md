---
name: ""
overview: ""
todos: []
isProject: false
---

# DML Local Executor

## Goal

Route the DML shapes the DuckDB fast path cannot model cleanly to a
storage-aware local executor. Own `numDmlAffectedRows` correctness
and the BigQuery DML response shape.

## Background

Today:

- `MERGE` lowers through the DuckDB fast path for the easy branches
  and surfaces `UNIMPLEMENTED` for the harder matrix
  (`WHEN NOT MATCHED BY SOURCE`, multi-action sequences).
- `INSERT VALUES`, `INSERT ... SELECT`, `UPDATE`, `DELETE` all
  surface `UNIMPLEMENTED`. Conformance fixtures seed rows with
  `tabledata.insertAll` as a workaround.
- `numDmlAffectedRows` is never populated correctly because the
  fast path does not have a reliable place to count rows after
  DuckDB's optimizer has finished with them.

A semantic-executor-style local DML executor is the right place for
this work because:

- It owns the per-row evaluation loop, so counting affected rows is
  natural.
- It can mutate storage through `DuckDBStorage`'s C++ API instead
  of through DuckDB SQL, sidestepping the SQL-level rewrites that
  the DuckDB-only roadmap would have required.
- It can call into `array-struct-semantic-path.plan.md`'s deep
  STRUCT mutation primitive for `UPDATE t SET s.a.b = ...`.

## Dependencies

- `semantic-executor-core.plan.md` (expression evaluation).
- `array-struct-semantic-path.plan.md` (deep STRUCT mutation).
- `engine-router-foundation.plan.md` (routing).
- `execution-disposition-registry.plan.md` (the disposition rows
  flip from `(planned)` to landed here).

## Scope

Statements that route here:

- `ResolvedInsertStmt` (`INSERT VALUES`, `INSERT ... SELECT`).
- `ResolvedUpdateStmt`.
- `ResolvedDeleteStmt`.
- `ResolvedMergeStmt` (the branches the fast path could not lower).
- `ResolvedPipeInsertScan` (the pipe-operator INSERT form).
- `ResolvedReturningClause` (DML RETURNING).
- `ResolvedUpdateConstructor` (the DML-expr-only construction).

## Implementation Plan

1. Add `backend/engine/semantic/dml/` with `dml_executor.{h,cc}`
   and per-statement files (`insert.cc`, `update.cc`, `delete.cc`,
   `merge.cc`).
2. Implement `INSERT VALUES` first: evaluate the values list with
   the semantic executor's `EvalExpr`, append to
   `DuckDBStorage::AppendRows`, count rows.
3. Implement `INSERT ... SELECT`: stream rows from the SELECT
   (routed by the coordinator — fast path or semantic, depending on
   shape), append to the target table, count rows.
4. Implement `UPDATE`: evaluate the predicate per row, evaluate the
   SET list, mutate via `DuckDBStorage` (deep STRUCT updates via
   `Value::WithField`), count rows.
5. Implement `DELETE`: evaluate the predicate per row, delete via
   `DuckDBStorage`, count rows.
6. Reroute `MERGE` from the DuckDB fast path: simple branches stay
   on `duckdb_rewrite`; the harder matrix runs the source vs.
   target join in the executor and dispatches per matched row to
   the appropriate INSERT / UPDATE / DELETE primitive above.
7. Update `gateway/jobs/` so DML jobs report the right
   `statementType` and `numDmlAffectedRows`.
8. Drop the conformance-harness comment that says "seed rows with
   `tabledata.insertAll` because INSERT is `UNIMPLEMENTED`" once
   INSERT lands.

## Tests

- Per-statement unit tests under `backend/engine/semantic/dml/`.
- Engine-level integration tests in `gateway/e2e/dml/`:
  - `INSERT VALUES` with single + multi-row variants.
  - `INSERT ... SELECT` from a non-trivial query.
  - `UPDATE ... WHERE` predicate.
  - `UPDATE t SET s.a.b = ...` deep STRUCT.
  - `DELETE FROM ... WHERE`.
  - `MERGE` with each WHEN branch (matched / not-matched-target /
    not-matched-source / multi-action).
  - `INSERT ... RETURNING *` shape.
- Conformance fixtures under `conformance/fixtures/dml/` pinning
  the route label to `semantic_executor` (or `duckdb_rewrite` for
  the simple MERGE branches that stay on the fast path).

## Done Criteria

- `INSERT VALUES`, `INSERT ... SELECT`, `UPDATE`, `DELETE` no
  longer return `UNIMPLEMENTED`.
- `MERGE`'s full BigQuery matrix runs locally.
- `numDmlAffectedRows` is populated and matches BigQuery's
  semantics for every supported DML statement.
- Conformance fixtures stop using `tabledata.insertAll` as a
  workaround for seeding; they may still use it as a deliberate
  test of the streaming-insert path.
- ROADMAP.md "DML / DDL" section's DML row flips to `✅`.
