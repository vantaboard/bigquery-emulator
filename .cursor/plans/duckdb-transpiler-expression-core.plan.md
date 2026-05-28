# DuckDB Transpiler Expression Core

## Goal

Fill the scalar-expression gaps that block otherwise simple scan plans from
staying on the DuckDB path.

## Tracker Rows

- `ResolvedParameter` from `not_started` to `done`.
- `ResolvedCast` from `not_started` to `done`.
- `ResolvedFunctionArgument` from `not_started` to `done`.
- `ResolvedWithExpr` from `not_started` to `done`.

## Implementation Plan

1. Add parameter state to `Transpiler` so named and positional GoogleSQL
   parameters lower deterministically to DuckDB bind markers (`$1`, `$2`, ...).
   Preserve enough metadata for the engine to bind values in the same order.
2. Implement `EmitCast` using `backend/engine/duckdb/transpiler/types.h`,
   handling explicit casts, safe casts, and unsupported type targets separately.
   Safe casts should lower to DuckDB `TRY_CAST` where semantics match.
3. Route `ResolvedFunctionArgument` through the child expression or argument
   literal it wraps. Keep named-argument-only shapes on fallback until a caller
   proves the target function supports that syntax.
4. Lower `ResolvedWithExpr` as a scalar subquery or CTE-like projection that
   preserves BigQuery evaluation semantics for `WITH name AS (expr)` expression
   aliases.
5. Extend `EmitExpr` dispatch and `transpiler_test.cc` accessors for all new
   expression shapes.

## Tests

- Parameters: named parameter reuse, positional ordering, parameter in `LIMIT`,
  and parameter inside a predicate.
- Casts: scalar casts, safe casts, unsupported target types, and casts nested
  inside project/filter expressions.
- `WithExpr`: single binding, multiple bindings, and binding referenced by a
  child expression.

## Done Criteria

- The tracker rows above are `done`.
- Engine binding order is covered by tests, not only string rendering.
- Unsafe semantic mismatches still fall back by returning `""`.
