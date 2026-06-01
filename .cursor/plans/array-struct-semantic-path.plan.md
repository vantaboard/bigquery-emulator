---
name: ""
overview: ""
todos: []
isProject: false
---

# Array / Struct Semantic Path

## Goal

Route the array and struct shapes where DuckDB's `LIST` / `STRUCT`
model diverges from BigQuery to the semantic executor. This plan
owns: `UNNEST WITH OFFSET`, lateral array scans, multi-array zip,
outer array scans, anonymous / nested struct edge cases, deep STRUCT
mutation, and the BigQuery `FLATTEN` operator.

## Background

The DuckDB fast path lowers standalone `UNNEST(arr) AS col` and
named-field STRUCT literals, plus anonymous-field STRUCT literals
via the `_0` / `_1` synthesized-name rewrite. Everything else in this
family is `(planned)` `semantic_executor`:

- `UNNEST(arr) WITH OFFSET AS idx` — DuckDB's lateral unnest does
  not give us the BigQuery offset semantics cleanly.
- `LEFT JOIN UNNEST(arr) ON ...` and `FROM t, UNNEST(t.arr)` (cross
  join form) — BigQuery's correlated array scan evaluation order
  is row-at-a-time over `t`.
- `UNNEST(a, b, ...)` (`array_zip_mode`) — DuckDB does not have
  multi-array zip natively.
- Outer `UNNEST` (`is_outer`) — BigQuery emits a row with NULL
  array element when the array is empty; DuckDB drops the row.
- `ResolvedFlatten` / `ResolvedFlattenedArg` — BigQuery `FLATTEN`
  has no DuckDB analog.
- Deep STRUCT mutation: `UPDATE t SET s.a.b = ...` where DuckDB's
  `struct_pack` / `struct_extract` cannot express the rewrite
  cleanly.
- `ResolvedColumnHolder` — the UNNEST WITH OFFSET shape's column
  reference.

## Dependencies

- `semantic-executor-core.plan.md` (the interpreter that hosts
  these shapes).
- `engine-router-foundation.plan.md` (so a query that contains any
  of these shapes routes its whole top-level statement to the
  semantic executor).
- `dml-local-executor.plan.md` (the deep-STRUCT mutation case is
  shared with DML).

## Scope

Routing dispositions this plan flips from `(planned)` to landed:

- `ResolvedArrayScan` (the non-`duckdb_native` subset)
- `ResolvedJoinScan` `is_lateral` / `parameter_list` correlated
  cases
- `ResolvedColumnHolder`
- `ResolvedFlatten` / `ResolvedFlattenedArg`
- `ResolvedUpdateConstructor` deep-STRUCT subset
- Anonymous-struct semantics that need exact field-order behavior
  past the current `duckdb_rewrite` synthesized-name rewrite

## Implementation Plan

1. Implement `UNNEST` evaluation in the semantic executor: walk a
   `ResolvedArrayScan` and emit one row per element, carrying the
   BigQuery semantic. Cover the offset variant (`array_offset_column`),
   the outer variant (`is_outer`), and the multi-array zip variant
   (`array_zip_mode`).
2. Implement correlated array scans: when a `ResolvedJoinScan` has
   a lateral input, the semantic executor scans the outer row
   source and for each row evaluates the inner array.
3. Implement `FLATTEN`: walk a chain of GetStructField / ArrayScan
   nodes and produce the flattened sequence per BigQuery's
   documented semantics.
4. Implement deep STRUCT mutation in the semantic executor's value
   space: `Value::WithField(path, new_value)` performs the immutable
   nested update; DML callers
   (`dml-local-executor.plan.md`) consume it.
5. Update `SHAPE_TRACKER.md` rows as each shape lands.

## Tests

- Per-shape unit tests under `backend/engine/semantic/array_struct/`.
- Engine-level integration tests in `gateway/e2e/`:
  - `SELECT * FROM UNNEST([1,2,3]) AS n WITH OFFSET AS idx`.
  - `SELECT t.id, x FROM t, UNNEST(t.arr) AS x`.
  - `SELECT * FROM UNNEST([1,2], [10,20]) AS (a, b)` (zip).
  - Outer-UNNEST returning a NULL row.
  - `SELECT FLATTEN(...)`.
  - DML deep-STRUCT `UPDATE t SET s.a.b = ...` round-trip.
- Conformance fixtures under
  `conformance/fixtures/array_struct/` pinning the route label to
  `semantic_executor`.

## Done Criteria

- Every row in this plan's "Scope" list has a non-`(planned)`
  `semantic_executor` disposition and conformance coverage.
- No array / struct shape that the fast path cannot lower still
  surfaces a `UNIMPLEMENTED` from inside the transpiler — the
  classifier routes it to the semantic executor first.
- Deep STRUCT mutation works in DML (consumed by
  `dml-local-executor.plan.md`).
