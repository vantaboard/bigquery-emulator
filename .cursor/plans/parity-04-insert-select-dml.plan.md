---
name: Parity 04 — INSERT ... SELECT + DML completion
overview: Complete the local DML executor's deferred matrix - INSERT ... SELECT, UPDATE ... FROM, deep-STRUCT SET (s.a.b = ...), DELETE with array_offset_column, and ASSERT_ROWS_MODIFIED - keeping numDmlAffectedRows correct for every new shape.
est_effort: ~1-2 weeks
isProject: true
todos:
  - id: insert-select
    content: "INSERT ... SELECT: run the SELECT through the coordinator (whatever route the query shape picks), stream resulting rows into DuckDBStorage::AppendRows via the DML executor; honor column-list reordering + implicit coercions; populate numDmlAffectedRows."
    status: completed
  - id: update-from
    content: "UPDATE ... FROM ...: join-driven update on the semantic DML executor (per-row match against the FROM relation, BigQuery's multiple-match error surface)."
    status: completed
  - id: deep-struct-set
    content: "Deep-STRUCT SET (UPDATE t SET s.a.b = ...): implement nested-field rewrite in the DML executor's value layer instead of DuckDB struct update; remove the empty-string fallback noted in ROADMAP STRUCT handling."
    status: completed
  - id: delete-offset
    content: "DELETE with array_offset_column (lateral UNNEST in target): depends on parity-01's outer-row primitive; land after or behind it."
    status: pending
  - id: assert-rows-modified
    content: "ASSERT_ROWS_MODIFIED on INSERT/UPDATE/DELETE/MERGE: compare affected count post-execution, surface BigQuery's documented error envelope on mismatch, roll back the statement's effects."
    status: completed
  - id: fixtures-trackers
    content: Fixtures under conformance/fixtures/dml/ (round-trip seed -> mutate -> SELECT verify, plus numDmlAffectedRows assertions); flip SHAPE_TRACKER ResolvedInsertStmt/ResolvedUpdateStmt/ResolvedDeleteStmt subset notes; update ROADMAP DML bullet + ENGINE_POLICY practice section.
    status: completed
---

# Parity 04 — INSERT ... SELECT + DML completion

## Why

`INSERT ... SELECT` is the most common write shape in ETL workloads
and currently *"is owned by (planned) and surfaces a clean
notImplemented"* (SHAPE_TRACKER `ResolvedInsertStmt`). The rest of the
deferred DML matrix (`UPDATE ... FROM`, deep-STRUCT `SET`, DELETE with
offset, `ASSERT_ROWS_MODIFIED`) lives on the same executor and batches
naturally. ENGINE_POLICY §What-this-means #2 names exactly this list.

## Current state

| Shape | Today | Target |
|-------|-------|--------|
| `INSERT VALUES` (single/multi-row) | landed (`dml_executor.cc`) | — |
| `INSERT ... SELECT` | notImplemented | `semantic_executor` (DML executor + coordinator-served SELECT) |
| scalar-`SET` `UPDATE` | landed | — |
| `UPDATE ... FROM` | deferred | `semantic_executor` |
| deep-STRUCT `SET s.a.b` | deferred (empty-string fallback) | `semantic_executor` |
| `DELETE` (incl. no-WHERE) | landed | — |
| `DELETE` with `array_offset_column` | deferred | `semantic_executor` (needs parity-01 primitive) |
| `ASSERT_ROWS_MODIFIED` | UNIMPLEMENTED | `semantic_executor` |
| `numDmlAffectedRows` | landed shapes only | every shape above |

## Key files

- [`backend/engine/semantic/dml/`](../../backend/engine/semantic/dml/) — `dml_executor.cc`
- [`backend/engine/coordinator/local_coordinator_engine.cc`](../../backend/engine/coordinator/local_coordinator_engine.cc) — SELECT-side execution the INSERT...SELECT path reuses
- [`backend/storage/duckdb/`](../../backend/storage/duckdb/) — `AppendRows` commit primitive
- `gateway/jobs/` — `numDmlAffectedRows` surfacing
- `conformance/fixtures/dml/`

## Steps

1. `INSERT ... SELECT` first: the SELECT half must go through the
   coordinator's normal routing (do NOT re-implement query execution
   inside the DML executor); the DML half maps output columns onto the
   target schema with BigQuery's implicit-coercion rules, then commits
   through `AppendRows`. Self-insert (`INSERT INTO t SELECT * FROM t`)
   must read a stable snapshot before writing.
2. `UPDATE ... FROM`: materialize the FROM relation via the
   coordinator, match per target row, apply BigQuery's "UPDATE/MERGE
   must match at most one source row" error.
3. Deep-STRUCT `SET`: implement nested value substitution on the
   executor's value representation; this also closes the ROADMAP
   "deep STRUCT mutations fall back via the empty-string contract" note.
4. `ASSERT_ROWS_MODIFIED`: wrap executor dispatch; on mismatch surface
   the documented error and ensure no partial commit (single
   transaction around the statement).
5. `DELETE` + offset: gate on parity-01's primitive; if 01 has not
   landed, leave the row deferred and say so in the commit.
6. Fixtures + tracker/ROADMAP/POLICY updates per landed shape.

## Verify

```bash
task emulator:build-engine:bazel
task conformance:run
task lint:dispositions
task bazel:shutdown && task bazel:status
```

Check `numDmlAffectedRows` assertions in the new fixtures and re-run
third-party suites whose skip rows cite `INSERT ... SELECT`.

## Out of scope

- MERGE harder matrix + RETURNING — plan 06
- `ResolvedPipeInsertScan` — plan 12
- `ResolvedUpdateConstructor` lands here only if a fixture needs it; otherwise note it for plan 06
