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

## Subagent landing log

### Subagent 1 (commits `fcb5fae`, `9b2bde1`, `af3d9e4`, `0706a5c`, `6ee6dc9`)

**Landed (Families 1 – 3):**

- **Family 1 — `UNNEST WITH OFFSET` + `ResolvedColumnHolder`.**
  Classifier (`route_classifier.cc::VisitResolvedArrayScan`)
  property-promotes any `ResolvedArrayScan` carrying
  `array_offset_column`. The semantic executor binds the offset
  column via `EvalContext::columns` (commit `9b2bde1`) and emits
  one row per element through
  `backend/engine/semantic/array_struct/array_scan.cc::EvaluateArrayScan`.
  `ResolvedColumnHolder` is now ready in the YAML +
  SHAPE_TRACKER (commit `6ee6dc9`).
- **Family 2 — outer `UNNEST` (`is_outer`).** `EvaluateArrayScan`
  detects `is_outer && elements.empty()` and emits a single row
  whose element + offset are both NULL.
- **Family 3 — multi-array zip (`array_zip_mode`).** All three
  modes implemented in `EvaluateArrayScan`: PAD (default; NULL-
  pads the shorter array), TRUNCATE (length = min), STRICT
  (rejects mismatched lengths with INVALID_ARGUMENT). Unit-tested
  in `array_scan_test.cc`.

**Deferred (Families 4 – 7):** see the "Deferrals" section below.

## Deferrals

### Family 4 — correlated `ResolvedJoinScan` / non-`SingleRowScan` input_scan

Status: **deferred to a follow-up subagent of this plan.**

The classifier (`fcb5fae`) does property-promote
`ResolvedArrayScan` with a non-`SingleRowScan` `input_scan` or
non-null `join_expr` to the semantic executor. The executor wiring
in `executor.cc` accepts the shape via `AcceptKnownShape`. What is
**not** wired:

- `EvaluateArrayScan` returns a structured `kNotImplemented`
  (`unimplemented: ResolvedArrayScan correlated/joined input_scan
  is owned by ... Family 4`) for both shapes.
- The semantic executor's `ExecuteQuery` does not yet walk an
  outer row source row-at-a-time and re-evaluate the inner array
  per outer row. The `RowSource` adapter (plan 6 / commit
  `f84a1fe`) is the input; what's missing is the cross-product
  iteration loop that re-points `EvalContext::columns` to the
  combined (outer-row ⊕ inner-element) bindings before calling
  `ProjectOneRow`.

**Why not silently approximated:** A row-at-a-time inner-array
evaluation is a meaningful semantic shape (not a simple
materialization). Faking it as a flat array scan would silently
collapse the correlation, producing wrong rows when the outer
column appears inside the inner array expression
(`FROM t, UNNEST(t.arr)`). Better to surface NOT_IMPLEMENTED.

**Re-point:** Row keeps `plan=array-struct-semantic-path.plan.md`
status pinned via the executor's NOT_IMPLEMENTED. The follow-up
subagent picks it up.

### Family 5 — `ResolvedFlatten` / `ResolvedFlattenedArg`

Status: **deferred; row remains at `status=planned` against this
plan.**

`FLATTEN((x.a.b.c))` walks a chain of
`GetStructField` / `ArrayScan` nodes; the BigQuery semantic
(documented at <https://cloud.google.com/bigquery/docs/reference/standard-sql/array_functions#flatten>)
flattens arrays of structs through field paths. Implementation
budget exceeded the per-family cap; per the plan's pragmatic
posture, deferred to a focused future subagent.

**Why not silently approximated:** The flatten chain interleaves
ARRAY and STRUCT shapes; mapping it to a sequence of UNNEST + dot
access would change the output cardinality on NULL paths
(BigQuery emits one NULL row per missing field; a naive rewrite
emits zero).

### Family 6 — `ResolvedUpdateConstructor` deep-STRUCT mutation

Status: **already re-pointed to `dml-local-executor.plan.md`.**

The YAML row for `ResolvedUpdateConstructor` already carries
`plan=dml-local-executor.plan.md status=planned`. This subagent
did NOT land it because:

- The DML infrastructure (`numDmlAffectedRows`, storage-aware
  execution path) lives in plan 9 (`dml-local-executor.plan.md`).
- Touching the deep-STRUCT mutation surface here would force
  premature commitments on the DML wire shape.

**Why not silently approximated:** A `Value::WithField(path,
new_value)` operation that DML callers depend on must round-trip
through the storage layer; the storage layer's struct
representation is owned by plan 9.

### Family 7 — anonymous-struct field-order edges

Status: **audit complete; no re-point needed.**

The `duckdb_rewrite` synthesized-name path (`_0`, `_1`, ...) is
exercised end-to-end by
`conformance/fixtures/fastpath/regression_struct_anonymous.yaml`
and `regression_struct_nested_field_order.yaml`. Both fixtures
pass on `duckdb_rewrite` today and pin the exact positional
behavior BigQuery requires.

**Verdict:** The `(planned) semantic_executor` row for
anonymous-struct in the original plan goal turned out to be
already-covered by the existing fast-path rewrite. Leaving
`ResolvedMakeStruct` + `ResolvedGetStructField` at `duckdb_rewrite`.
If a future conformance fixture surfaces a case the rewrite cannot
serve, that fixture's owning plan can re-point the row.
