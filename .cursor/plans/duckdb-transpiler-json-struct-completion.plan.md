# DuckDB Transpiler JSON And Struct Completion

## Goal

Promote JSON field access and close the known STRUCT holes so nested data can
round-trip through DuckDB without reference-impl fallback for common shapes.

## Tracker Rows

- `ResolvedGetJsonField` from `not_started` to `done`.
- Complete the remaining subset of `ResolvedMakeStruct`.
- Complete the remaining subset of `ResolvedGetStructField`.
- Complete nested `ResolvedLiteral` STRUCT handling for anonymous fields if the
  chosen representation supports it.

## Implementation Plan

1. Decide the canonical DuckDB representation for anonymous BigQuery STRUCT
   fields. Prefer synthesized stable names (`_0`, `_1`, ...) if it keeps field
   access and row serialization simple.
2. Update `EmitValueLiteral`, `EmitMakeStruct`, and `EmitGetStructField` to use
   the same synthesized-name convention for anonymous fields.
3. Implement `EmitGetJsonField` using DuckDB JSON extraction operators. Keep
   BigQuery scalar-vs-JSON return type semantics explicit rather than relying on
   DuckDB defaults.
4. Add JSON type and literal tests for object field access, nested access,
   missing keys, null values, and string escaping.
5. Update row serialization tests if the emitted DuckDB JSON/STRUCT result needs
   a new Arrow-to-BigQuery conversion path.

## Tests

- `STRUCT(1, "a")` projection and positional field access.
- Named and anonymous struct literals nested in arrays.
- JSON object field access and nested path access.
- Fallback tests for JSON path features DuckDB cannot faithfully match yet.

## Done Criteria

- Tracker notes no longer list anonymous STRUCT fields as fallback.
- `ResolvedGetJsonField` is `done`.
- STRUCT and JSON behavior is covered at both transpiler-string and execution
  levels.
