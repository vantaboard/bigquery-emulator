# DuckDB Transpiler Window And Order Completion

## Goal

Finish the analytic and ordering subsets that currently fall back because of
collation, parameter, null-handling, or RANGE-frame semantics.

## Tracker Rows

- Complete the remaining subset of `ResolvedAnalyticScan`.
- Complete the remaining subset of `ResolvedAnalyticFunctionCall`.
- Complete the remaining subset of `ResolvedWindowFrame*` and
  `ResolvedAnalyticFunctionGroup`.
- Complete the remaining subset of `ResolvedOrderByItem`.

## Implementation Plan

1. Add collation support for `ORDER BY` and analytic partition/order lists only
   for collations DuckDB can reproduce.
2. Implement analytic `parameter_list` handling after the lateral support plan
   defines the common correlated-parameter representation.
3. Lower `IGNORE NULLS` / `RESPECT NULLS` for `LAG`, `LEAD`, `FIRST_VALUE`,
   `LAST_VALUE`, and `NTH_VALUE` where DuckDB matches BigQuery.
4. Validate `RANGE` frames by input type. Either cast or fall back for
   non-numeric / non-temporal order expressions that DuckDB treats differently.
5. Add negative tests for malformed or semantically mismatched frames.

## Tests

- Analytic functions with `IGNORE NULLS` and `RESPECT NULLS`.
- Collated `ORDER BY` at top level and inside `OVER (...)`.
- RANGE frames over numeric, date/time, and unsupported types.
- Multiple analytic groups sharing partition/order definitions.

## Done Criteria

- The tracker rows above are fully `done`, not `done (subset)`.
- SQL strings and execution results are covered for each promoted feature.
- Collation support is deliberately scoped and documented in
  `SHAPE_TRACKER.md`.
