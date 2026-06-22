# 06 — DuckDB transpiler column-binding property / round-trip tests

- **Series:** conformance-hardening (plans 01–08). Run sequentially.
- **Sequencing:** sixth. Independent build-wise; pairs with plan 01 (oracle) for
  result validation and plan 07 (pins the specific repro).
- **Priority:** P2.

## Why this exists (origin)

A `LEFT JOIN ... WHERE p.id IS NULL` orphan-orders query over **QUALIFY-deduped
views** failed inside DuckDB:

```
Unimplemented ... duckdb engine: DuckDB rejected transpiled SQL:
Binder Error: Referenced column "id" not found in FROM clause!
Candidate bindings: "__bq_input_rn"
(sql=SELECT "order_id", ... FROM (SELECT "__bq_j_1" AS "order_id", ...
 ... ROW_NUMBER() OVER (...) AS "$analytic1" ... row_number() OVER () AS "__bq_input_rn" ...))
```

The transpiler's column-projection scheme (the `__bq_j_N` positional aliases,
`__bq_input_rn` row-number tag, `$analytic1` window alias) **lost the `id`
binding** when composing nested subqueries (view dedup → join → anti-filter →
outer projection). Each layer transpiled fine in isolation, but the binding did
not survive composition. This is exactly the failure class hand-written
transpiler unit tests miss: they test shapes individually, not deeply composed.

## Current state (grounded)

- Transpiler lives in `backend/engine/duckdb/transpiler/` with per-construct
  emitters: `transpiler_emit_with.cc` (CTEs), `transpiler_emit_setops.cc`
  (UNION), `transpiler_emit_join.cc`, `transpiler_emit_analytic.cc`
  (window/QUALIFY), `transpiler_emit_query.cc`, `transpiler_scan_tree.cc`.
- Shape/column tracking is already a concept:
  `backend/engine/duckdb/transpiler/SHAPE_TRACKER.md` and `types.h`/`types.cc`.
- Tests are per-emitter and composition-light: `transpiler_emit_*_test.cc`,
  `transpiler_integration_test.cc` (currently open in the editor). They assert
  emitted SQL for specific shapes, not binding survival under arbitrary nesting.

## Goal / done-criteria

1. A **binding-preservation property**: for any transpilable query, every column
   named in the BigQuery output projection resolves in the emitted DuckDB SQL
   (no "Referenced column X not found"), and the emitted SQL **binds**
   (DuckDB-prepares) without error.
2. A **composition generator** that nests the known-tricky constructs (views,
   QUALIFY/ROW_NUMBER dedup, JOIN, anti-join `WHERE x IS NULL`, CTEs, set ops,
   `SELECT * EXCEPT`) to depth ≥3 and feeds them through the transpiler +
   DuckDB binder.
3. The specific orphan-orders shape transpiles, binds, and (via plan 01 oracle)
   returns BigQuery-matching rows.
4. Round-trip schema check: the DuckDB result schema column set/names match the
   BigQuery projection for each generated shape.

## Implementation steps

### Step 1 — Binding assertion helper
- In the transpiler test fixture (`transpiler_test_fixture.h`), add a helper that
  takes a query, transpiles it, then asks DuckDB to **prepare** (bind) the
  emitted SQL and asserts no binder error; on failure, dump the analyzer output,
  the emitted SQL, and the shape-tracker state for triage.

### Step 2 — Composition cases (deterministic first)
- Add `transpiler_emit_composition_test.cc` with explicit nested shapes that
  reproduce the report:
  - view = `SELECT ... QUALIFY ROW_NUMBER() OVER (PARTITION BY id ORDER BY ts DESC)=1`
  - query = `SELECT o.id ... FROM orders_view o LEFT JOIN profiles_view p ON
    o.profile_id=p.id WHERE o.profile_id IS NOT NULL AND p.id IS NULL`
  - assert: transpiles, binds, and the `id`/`order_id` columns resolve.

### Step 3 — Generator (property-style)
- A bounded generator (table of base relations × a set of composable wrappers)
  that emits N nested queries; for each, assert Step 1's binding property. Keep
  it deterministic/seeded so CI failures reproduce. This is the part that finds
  the *next* composition bug, not just this one.

### Step 4 — Fix the binding loss
- Trace where `id` is dropped: likely the projection/alias rewrite in
  `transpiler_emit_query.cc` / `transpiler_scan_tree.cc` not threading the
  source column through the `__bq_j_N` remap when an analytic (`$analytic1`) +
  `__bq_input_rn` layer is interposed under a join. Fix so required columns
  remain in scope for outer references. Use `SHAPE_TRACKER.md` as the model for
  what each layer must preserve; update it if the invariant changes.

### Step 5 — Result validation
- Add the orphan-orders + dedup shapes to plan 01's differential corpus so rows
  (not just binding) match BigQuery.

## Tests
- `transpiler_emit_composition_test.cc` (deterministic repros, incl. orphan
  orders).
- Generator-driven binding property test (seeded).
- Differential corpus rows match BigQuery for the composed shapes.
- No regression in existing `transpiler_emit_*_test.cc`.

## Process hygiene (repo rules)
Heavy C++ build + bazel tests. Follow `.cursor/rules/bazel-process-hygiene.mdc`
and `.cursor/rules/process-hygiene.mdc` (single bazel invocation, throttled jobs,
warm daemon within the plan, cleanup + `(clean)` status at the end).

## Out of scope
- New SQL feature support beyond making existing transpilable shapes bind
  correctly (UNION DISTINCT / WithRefScan feature gaps are tracked via plans
  01/07 and the engine roadmap).
- Performance of the emitted SQL.

## Touch list
`backend/engine/duckdb/transpiler/transpiler_emit_query.cc`,
`backend/engine/duckdb/transpiler/transpiler_scan_tree.cc`,
`backend/engine/duckdb/transpiler/transpiler_emit_join.cc` /
`transpiler_emit_analytic.cc` (as the trace dictates),
`backend/engine/duckdb/transpiler/transpiler_test_fixture.h`,
new `transpiler_emit_composition_test.cc` (+ `BUILD.bazel`),
`backend/engine/duckdb/transpiler/SHAPE_TRACKER.md`,
`conformance/differential/corpus/*`.
