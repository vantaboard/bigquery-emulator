# DuckDB Transpiler UDF TVF And Module Support

## Goal

Promote user-defined function, table-valued function, module, constant, and
expression-mode shapes that currently stay on the reference path.

## Tracker Rows

- `ResolvedCreateFunctionStmt` from `skiplist` to `done` if not completed by
  the DDL plan.
- `ResolvedCreateTableFunctionStmt` from `skiplist` to `done` if not completed
  by the DDL plan.
- `ResolvedTVFScan` from `skiplist` to `done`.
- `ResolvedRelationArgumentScan` from `skiplist` to `done`.
- `ResolvedArgumentRef` and `ResolvedArgumentDef` from `skiplist` to `done`.
- `ResolvedConstant` from `skiplist` to `done`.
- `ResolvedExpressionColumn` from `skiplist` to `done`.
- `ResolvedInlineLambda` from `skiplist` to `done` for function families that
  need lambda arguments.

## Implementation Plan

1. Define catalog storage for SQL UDFs, aggregate UDFs if supported, and TVFs.
2. Lower scalar SQL UDF calls either by inlining resolved bodies or registering
   DuckDB macros with equivalent semantics.
3. Lower TVF scans as derived tables, preserving relation argument schemas and
   output-column aliases.
4. Implement argument refs/defs as scoped bindings shared by UDF, TVF, and
   lambda emit paths.
5. Implement constants and expression columns in the analyzer modes used by the
   emulator; keep unused module surfaces unregistered until tests need them.
6. Promote inline lambdas only together with the function families that consume
   them.

## Tests

- Scalar SQL UDF create/use/drop.
- TVF create/use with scalar and relation arguments.
- Constants referenced inside expressions.
- Lambda-consuming functions once enabled.

## Done Criteria

- UDF/TVF/module rows are `done`.
- UDF and TVF definitions survive the catalog lifecycle expected by the gateway.
- Scoped argument binding is shared with subquery/CTE state instead of bespoke
  one-off maps.
