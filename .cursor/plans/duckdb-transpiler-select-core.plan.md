# DuckDB Transpiler Select Core

## Goal

Lower complete ordinary `SELECT` statements through DuckDB, not just scan
subtrees. This is the first dependency for most remaining `not_started` rows.

## Tracker Rows

- `ResolvedQueryStmt` from `not_started` to `done`.
- `ResolvedProjectScan` from `not_started` to `done`.
- `ResolvedSingleRowScan` from `not_started` to `done`.
- `ResolvedOutputColumn` from `not_started` to `done`.
- `ResolvedComputedColumn` from `not_started` to `done`.

## Implementation Plan

1. Teach `Transpiler::EmitSingleRowScan` to emit a one-row relation that composes
   as a derived table, starting with `SELECT 1`.
2. Teach `EmitComputedColumn` to lower `column := expr` as
   `<expr> AS "<resolved-column-name>"`, propagating `""` when the child
   expression cannot lower.
3. Teach `EmitProjectScan` to wrap `input_scan` and project the computed-column
   list, preserving column aliases from the resolved columns.
4. Teach `EmitOutputColumn` to render the final top-level output alias mapping,
   including aliases that differ from the resolved column name.
5. Teach `EmitQueryStmt` to lower `query()` and then apply the
   `output_column_list()` mapping as the final SELECT list.
6. Update dispatcher comments in `transpiler.h` so they describe the current
   implemented baseline instead of the original all-empty skeleton.

## Tests

- Add unit tests for `SELECT 1`, `SELECT 1 AS x`, table projection, expression
  projection, reordered output columns, and aliased output columns.
- Add an engine-level smoke test once unit SQL strings are stable: a simple
  `SELECT ... FROM ... WHERE ... GROUP BY ... ORDER BY ... LIMIT` should execute
  on DuckDB without fallback.

## Done Criteria

- The five tracker rows above are `done`.
- Focused transpiler tests prove emitted SQL for literal-only selects and
  table-backed projections.
- Unsupported child expressions still return `""` instead of emitting partial
  SQL.
