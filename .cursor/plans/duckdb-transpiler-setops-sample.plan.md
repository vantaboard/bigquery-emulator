# DuckDB Transpiler Set Operations And Sampling

## Goal

Lower set operations and table sampling through DuckDB while preserving
BigQuery column matching, duplicate handling, and sample semantics.

## Tracker Rows

- `ResolvedSetOperationScan` from `not_started` to `done`.
- `ResolvedSetOperationItem` from `not_started` to `done`.
- `ResolvedSampleScan` from `not_started` to `done`.

## Implementation Plan

1. Implement `EmitSetOperationItem` as a helper that lowers each input scan and
   projects columns in the exact output order expected by the parent operation.
2. Implement `EmitSetOperationScan` for `UNION ALL`, `UNION DISTINCT`,
   `INTERSECT DISTINCT`, and `EXCEPT DISTINCT`; add `ALL` variants only when
   DuckDB semantics match BigQuery.
3. Respect BigQuery's column-name propagation from the resolved output columns,
   not the child scan's raw aliases.
4. Implement `EmitSampleScan` using DuckDB `USING SAMPLE` only for sampling
   methods and units with matching semantics. Return `""` for unsupported
   repeatable seeds, weights, or method variants.
5. Extend dispatch in `Transpile` and `EmitScan`.

## Tests

- Unit tests for every supported set-op kind, including mixed child projection
  names and nested set operations.
- Execution tests comparing duplicate behavior for `UNION ALL` vs distinct
  operations.
- Sampling tests for percentage and row-count forms, plus fallbacks for
  unsupported sampling options.

## Done Criteria

- The three tracker rows are `done`.
- Unsupported sample methods still fall back.
- Set-op result columns match BigQuery output aliases.
