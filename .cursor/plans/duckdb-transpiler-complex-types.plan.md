# DuckDB Transpiler Complex Type Support

## Goal

Promote complex BigQuery type surfaces that need catalog and serialization work
before their resolved AST nodes can lower safely.

## Tracker Rows

- `ResolvedMakeProto` from `skiplist` to `done`.
- `ResolvedGetProtoField` from `skiplist` to `done`.
- `ResolvedGetProtoOneof` from `skiplist` to `done`.
- `ResolvedReplaceField` from `skiplist` to `done`.
- `ResolvedFilterField` and `ResolvedFilterFieldArg` from `skiplist` to `done`.
- `ResolvedGetRowField` from `skiplist` to `done`.
- `ResolvedFlatten` and `ResolvedFlattenedArg` from `skiplist` to `done`.
- Complete JSON-adjacent behavior that depends on type serialization after
  `ResolvedGetJsonField` lands.

## Implementation Plan

1. Decide which BigQuery complex types the emulator will actually store:
   PROTO, MEASURE/ROW, JSON, and nested repeated records each have different
   DuckDB physical representations.
2. Extend schema conversion and Arrow-to-BigQuery serialization before emitting
   SQL for types DuckDB can compute but the gateway cannot return.
3. Implement proto construction and field access either through JSON/STRUCT
   encoding or through a dedicated binary representation with descriptor
   awareness.
4. Implement `REPLACE_FIELDS` and field filters for structs and protos as
   deterministic expression rewrites.
5. Implement legacy `FLATTEN` only if the emulator supports the legacy surface;
   otherwise document the GoogleSQL replacement and keep API rejection explicit.

## Tests

- Proto literal construction and field access if proto storage is enabled.
- Struct/proto `REPLACE_FIELDS` with nested paths.
- Row/measure access if MEASURE type support is enabled.
- Flatten behavior or explicit unsupported-language rejection.

## Done Criteria

- Complex type rows are `done` with documented physical representations.
- Serialization tests prove query results match BigQuery REST row shape.
- Any intentionally unsupported legacy-only behavior is rejected before reaching
  the transpiler.
