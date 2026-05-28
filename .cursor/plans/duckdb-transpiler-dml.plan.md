# DuckDB Transpiler DML Support

## Goal

Promote mutating data statements and their DML-only helper expressions while
keeping BigQuery catalog/storage state consistent with DuckDB execution.

## Tracker Rows

- `ResolvedInsertStmt` from `skiplist` to `done`.
- `ResolvedUpdateStmt` from `skiplist` to `done`.
- `ResolvedDeleteStmt` from `skiplist` to `done`.
- `ResolvedMergeStmt` from `skiplist` to `done`.
- `ResolvedReturningClause` from `skiplist` to `done`.
- `ResolvedUpdateConstructor` from `skiplist` to `done`.

## Implementation Plan

1. Define whether DML goes through the transpiler's SQL string path or a storage
   mutation API that emits DuckDB statements internally.
2. Implement `INSERT` for values and query inputs, including explicit column
   lists and default values.
3. Implement `UPDATE` and `DELETE` with predicates lowered through existing
   expression emitters.
4. Implement `MERGE` only after enumerating BigQuery's match-action matrix and
   mapping it to DuckDB's supported `MERGE` grammar or a transaction of simpler
   statements.
5. Implement `RETURNING` where DuckDB can return the same row shape; otherwise
   surface a clear unsupported-option error.
6. Add storage-level transaction handling so partially failed DML cannot leave
   metadata and table files out of sync.

## Tests

- Insert values, insert-select, update with predicate, delete with predicate.
- Merge matched, not matched, and multi-action cases.
- Returning clause result shape.
- Error tests for unsupported default expressions or merge actions.

## Done Criteria

- DML rows are `done`.
- Mutating tests assert persisted table contents after execution.
- `MERGE` semantics are covered by targeted conformance cases before the row
  flips from `skiplist`.
