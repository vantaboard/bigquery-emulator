---
name: Parity 06 — MERGE matrix + RETURNING
overview: Land the harder MERGE branches (WHEN NOT MATCHED BY SOURCE, multi-action sequences) on the semantic executor as ENGINE_POLICY prescribes, plus DML RETURNING (ResolvedReturningClause), keeping numDmlAffectedRows and per-branch action ordering BigQuery-exact.
depends_on: [parity-04-insert-select-dml]
est_effort: ~1 week
isProject: true
todos:
  - id: merge-classifier
    content: "Route classification: detect WHEN NOT MATCHED BY SOURCE and multi-action MERGE shapes and promote them to semantic_executor (today they fall into the duckdb_rewrite path's unsupported edge); simple branches stay duckdb_rewrite."
    status: pending
  - id: merge-semantic
    content: "Semantic MERGE evaluator: full match matrix (MATCHED / NOT MATCHED BY TARGET / NOT MATCHED BY SOURCE), per-branch search conditions, action sequence ordering, BigQuery's multiple-match error; commit through the DML executor's storage primitives."
    status: pending
  - id: merge-stats
    content: "numDmlAffectedRows for semantic MERGE = inserted + updated + deleted per the BigQuery REST contract (legacy aggregate)."
    status: pending
  - id: returning
    content: "ResolvedReturningClause (THEN RETURN on INSERT/UPDATE/DELETE/MERGE): evaluate the returning projection over affected rows; surface rows through the normal RowSource so the gateway serializes them like a query result."
    status: pending
  - id: fixtures-trackers
    content: Fixtures under conformance/fixtures/dml/ (merge_not_matched_by_source, merge_multi_action, dml_returning) with seed/verify round-trips; flip SHAPE_TRACKER ResolvedMergeStmt + ResolvedReturningClause rows; update ROADMAP MERGE hazard bullet.
    status: pending
---

# Parity 06 — MERGE matrix + RETURNING

## Why

ROADMAP's dialect-friction section calls MERGE out by name: DuckDB's
`INSERT ... ON CONFLICT` cannot model the full GoogleSQL matrix, so
*"the harder branches will route through the semantic path so we don't
have to pretend DuckDB SQL can model them."* dbt incremental
materializations and CDC pipelines emit exactly these shapes
(`WHEN NOT MATCHED BY SOURCE THEN DELETE`, multi-action sequences).
`THEN RETURN` batches here because it rides the same affected-row
bookkeeping.

## Key files

- [`backend/engine/coordinator/route_classifier_visitor.cc`](../../backend/engine/coordinator/route_classifier_visitor.cc) — MERGE shape detection
- [`backend/engine/duckdb/transpiler/`](../../backend/engine/duckdb/transpiler/) — existing easy-branch MERGE lowering (stays)
- [`backend/engine/semantic/dml/`](../../backend/engine/semantic/dml/) — new MERGE evaluator next to `dml_executor.cc`
- `conformance/fixtures/dml/`

## Steps

1. Decide the split precisely: enumerate `ResolvedMergeStmt` fields
   that force the semantic route (`WHEN NOT MATCHED BY SOURCE` arm
   present, >1 action of the same kind, action-order sensitivity) and
   encode the test in the classifier with a `RouteDecision` reason
   naming the branch.
2. Semantic evaluator algorithm: materialize source via the
   coordinator; classify each target row (matched / not-matched-by-
   source) and each source row (not-matched-by-target) honoring branch
   search conditions **in declaration order**; collect inserts/updates/
   deletes; apply atomically through storage; raise the documented
   error when a target row matches multiple source rows.
3. `THEN RETURN`: evaluate the projection against the post-action row
   (INSERT) or pre/post per BigQuery docs; reuse the DML executor's
   row plumbing so the gateway response shape needs no new code.
4. Fixtures must pin action ordering (e.g. UPDATE-then-DELETE sequence
   on the same predicate class) — that is exactly the matrix DuckDB
   could not express.

## Verify

```bash
task emulator:build-engine:bazel
task conformance:run
task lint:dispositions
task bazel:shutdown && task bazel:status
```

Re-run any third-party suite rows (dbt feasibility doc, Java DML ITs)
that cite MERGE gaps.

## Out of scope

- `ResolvedPipeInsertScan` — plan 12
- `ASSERT_ROWS_MODIFIED` — plan 04 (verify it composes with semantic MERGE here)
