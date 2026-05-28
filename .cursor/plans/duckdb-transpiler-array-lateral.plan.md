# DuckDB Transpiler Array And Lateral Support

## Goal

Promote the remaining `UNNEST` and lateral join shapes while preserving
BigQuery's ordinal, zip, and outer-array semantics.

## Tracker Rows

- Complete the remaining subset of `ResolvedArrayScan`.
- Complete the remaining subset of `ResolvedJoinScan` for lateral joins,
  `JOIN USING`, and lateral `parameter_list`.
- `ResolvedColumnHolder` from `not_started` to `done`.

## Implementation Plan

1. Implement lateral `FROM t, UNNEST(t.arr)` and explicit lateral joins using
   DuckDB `CROSS JOIN unnest(...) AS alias(column)` or the closest stable
   derived-table shape.
2. Add `WITH OFFSET` support using `generate_subscripts(...)`, validating
   BigQuery's zero-based offset output against DuckDB's one-based list indices.
3. Implement outer `UNNEST` with the correct null-row behavior for empty arrays.
4. Implement multi-array zip modes only after mapping BigQuery PAD / TRUNCATE /
   STRICT semantics to DuckDB `list_zip` or an explicit generated-index join.
5. Implement `JOIN USING` output-column de-duplication and aliasing rules.

## Tests

- Standalone `UNNEST` remains covered and unchanged.
- `FROM table, UNNEST(table.arr)` with column references from both sides.
- `UNNEST(arr) WITH OFFSET AS pos`.
- Empty-array outer `UNNEST`.
- Multi-array zip for every supported zip mode.
- `JOIN USING` output shape and column-name conflict handling.

## Done Criteria

- Array and lateral tracker notes no longer list the deferred subsets.
- Offset columns and element columns serialize correctly through the engine.
- Unsupported zip or lateral shapes return `""` until implemented.
