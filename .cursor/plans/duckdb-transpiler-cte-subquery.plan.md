# DuckDB Transpiler CTE And Subquery Support

## Goal

Lower non-recursive CTEs and expression subqueries so ordinary analytical SQL can
compose nested query blocks on the DuckDB path.

## Tracker Rows

- `ResolvedWithScan` from `not_started` to `done`.
- `ResolvedWithRefScan` from `not_started` to `done`.
- `ResolvedSubqueryExpr` from `not_started` to `done`.

## Implementation Plan

1. Add scoped CTE name tracking to `Transpiler` so `ResolvedWithScan` can emit a
   `WITH name AS (<subquery>) ...` wrapper and `ResolvedWithRefScan` can refer to
   the correct quoted relation.
2. Preserve BigQuery's CTE column aliases by projecting each CTE subquery through
   its resolved column list.
3. Implement scalar, `EXISTS`, `IN`, `LIKE ANY/ALL` if present, and `ARRAY`
   subquery variants one at a time. Emit DuckDB SQL only for variants with
   matching null and cardinality semantics.
4. Ensure nested CTEs and subqueries restore previous scope state after each
   emit.
5. Add conformance labels for queries that combine CTEs with filters,
   aggregates, and joins.

## Tests

- `WITH c AS (...) SELECT ... FROM c`.
- Nested CTE names and shadowing.
- Scalar subquery success and multi-row fallback or error parity.
- `EXISTS`, `IN`, and `ARRAY(subquery)` behavior with empty and null-containing
  inputs.

## Done Criteria

- The tracker rows above are `done`.
- Scope restoration is covered by nested tests.
- Subquery null semantics are verified against BigQuery expectations.
