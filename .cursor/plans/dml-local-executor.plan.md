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

## Status (post-subagent-9)

Landed (subagent 9, commits `628269d`/`e76b939`/`5651943`):

- **Family 1 — `INSERT INTO t (...) VALUES (...)`**: single +
  multi-row, NULL-pads omitted columns, populates
  `DmlStats.inserted_row_count`.
- **Family 2 — `DELETE FROM t WHERE ...`**: full row scan +
  per-row predicate eval, writes survivors back via
  `Storage::OverwriteRows`. Honors BigQuery's
  "WHERE NULL deletes nothing" semantics.
- **Family 3 — `UPDATE t SET col = expr WHERE ...`**: scalar SET
  only. Multi-target SET works (`SET a = 1, b = 2`); the SET expr
  may reference any source column from the table scan.

Deferred (each row stays `status=planned` in
`backend/engine/duckdb/transpiler/node_dispositions.yaml` and
surfaces a structured `kNotImplemented` so the gateway envelope
matches every other "planned but not landed" route):

- **Family 4 — deep-STRUCT `SET s.a.b = ...` /
  `ResolvedUpdateConstructor` / nested array DML.** The update
  executor explicitly rejects `update_item->target()` shapes that
  are not `RESOLVED_COLUMN_REF`, and rejects any
  `update_item_element_list` / `delete_list` / `update_list` /
  `insert_list` / `element_column` content with a re-point at this
  family. Blocker per plan: depends on a `Value::WithField(path,
  new_value)` immutable-update primitive on
  `backend/engine/semantic/value.{h,cc}`. **No silent
  approximation reason**: writing `SET s.a.b = X` as
  `SET s = STRUCT(X AS b, ...)` would require the executor to
  reconstruct the rest of the struct from the catalog schema,
  which is fundamentally different from BigQuery's "update only
  field b, leave the rest of s alone" contract for
  REPEATED-mode parents and would silently rewrite NULLs in
  unrelated fields.
- **Family 5 — `INSERT ... SELECT`.** The insert executor
  rejects `insert.query() != nullptr` with a re-point at this
  family. The implementation needs to stream rows from the
  SELECT (whichever route the coordinator picks) into
  `Storage::AppendRows` while propagating
  `DmlStats.inserted_row_count` from the streamed row count;
  none of that infrastructure ships with subagent 9. **No silent
  approximation reason**: surfacing `0 inserted` when the SELECT
  does have rows (or `len(SELECT)` without actually appending)
  would corrupt downstream replay (`gateway/handlers/queries.go`
  uses the count for `numDmlAffectedRows` AND the storage state
  must match).
- **Family 6 — MERGE harder matrix.** Simple MERGE branches
  continue to route via `duckdb_rewrite`; the executor returns
  `kNotImplemented` for `RESOLVED_MERGE_STMT` so the harder
  matrix (`WHEN NOT MATCHED BY SOURCE`, multi-action sequences)
  stays explicitly unsupported. **No silent approximation
  reason**: a partial implementation that runs the join in C++
  but only handles a subset of the WHEN branches would mismatch
  BigQuery's per-branch counts in `dmlStats`.
- **Family 7 — DML `RETURNING` clause.** `RejectUnsupportedDmlFeatures`
  in `dml_executor.cc` surfaces a structured `kNotImplemented`
  for every `returning != nullptr` shape across INSERT / UPDATE /
  DELETE. **No silent approximation reason**: dropping the
  RETURNING projection silently would change the response shape
  from "rows" to "stats" and break clients that read RETURNING
  output.
- **Family 8 — `ResolvedPipeInsertScan`.** Untouched; the
  classifier still routes pipe inserts to whatever the existing
  registry says. **No silent approximation reason**: the audit
  needed to confirm whether `duckdb_rewrite` covers it cleanly
  did not happen this subagent.
- **`ASSERT_ROWS_MODIFIED`** and **`array_offset_column`** on
  UPDATE / DELETE (correlated lateral / `WITH OFFSET` shapes,
  cross-cuts plan 8 Family 4) and **GENERATED columns**: each
  surfaces `kNotImplemented` with a structured reason, gated by
  `RejectUnsupportedDmlFeatures` in `dml_executor.cc`.

`numDmlAffectedRows` plumbing is correct end-to-end for the
landed families: `dml_executor.cc` populates `DmlStats` ->
`SemanticExecutor::ExecuteDml` returns it ->
`LocalCoordinatorEngine::ExecuteDml` propagates it ->
`frontend/handlers/query.cc::EmitDmlStats` writes a `dml_stats`
proto message -> `gateway/handlers/queries.go::assembleQueryResponse`
emits the legacy `numDmlAffectedRows` aggregate plus the
structured `dmlStats` envelope (and replay path).

Conformance fixtures pinning the round-trip wire shape live
under `conformance/fixtures/dml/`. The runner doc + the two
existing select-fixture references to "INSERT VALUES is
UNIMPLEMENTED" have been retargeted at this plan.

Subagent-10's `cte-subquery-routing.plan.md` does NOT need to
touch any DML routing — the CTE work is SELECT-only. The DML
families left planned above re-point at this plan and remain
the responsibility of follow-up subagents of this plan.
