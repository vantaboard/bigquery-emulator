# DuckDB Transpiler Aggregate Completion

## Goal

Finish the aggregate surface beyond simple `GROUP BY` and bare aggregate calls.

## Tracker Rows

- Complete the remaining subset of `ResolvedAggregateScan`.
- `ResolvedGroupingSet*`, `ResolvedRollup`, and `ResolvedCube` from
  `not_started` to `done`.
- `ResolvedAggregateHavingModifier` from `not_started` to `done`.
- Complete remaining `ResolvedAggregateFunctionCall` modifiers.

## Implementation Plan

1. Lower `GROUPING SETS`, `ROLLUP`, and `CUBE` using DuckDB's native grouping
   syntax where it matches BigQuery.
2. Implement `GROUPING()` calls through the resolved grouping-call list and add
   result-column aliases that match the analyzer output.
3. Add support for aggregate `ORDER BY`, `LIMIT`, `HAVING MAX/MIN`,
   `IGNORE/RESPECT NULLS`, aggregate `FILTER` / `where_expr`, and multi-level
   aggregate fields incrementally.
4. Promote deferred aggregate functions in `functions.yaml` that belong with
   aggregate syntax, especially `COUNTIF` as `COUNT(*) FILTER (WHERE ...)`.
5. Keep differential-privacy aggregate variants in their own plan.

## Tests

- Grouping sets, rollup, cube, and `GROUPING()` output.
- `COUNTIF`, `ARRAY_AGG(... ORDER BY ... LIMIT ...)`, `STRING_AGG`, `ANY_VALUE`
  with having modifiers, and aggregate filters.
- Fallback tests for aggregate syntax DuckDB cannot express exactly.

## Done Criteria

- All aggregate rows in this plan are `done` or have a narrower follow-up row
  explicitly split out in `SHAPE_TRACKER.md`.
- Aggregate SQL is covered by execution tests, not only emitter unit tests.
- Existing simple aggregate tests continue to emit unchanged SQL unless a
  deliberate normalization is documented.
