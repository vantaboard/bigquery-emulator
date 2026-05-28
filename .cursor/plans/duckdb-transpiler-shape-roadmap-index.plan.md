# DuckDB Transpiler Shape Roadmap Index

Goal: take every `skiplist` and `not_started` row in
`backend/engine/duckdb/transpiler/SHAPE_TRACKER.md` to `done`, preserving the
empty-string fallback contract until each shape has a tested DuckDB lowering.

This index is ordered by dependency. Each child plan should land as one logical
change: emit code, focused tests, conformance labels or skips, and the matching
`SHAPE_TRACKER.md` status update in the same commit.

## Execution Order

1. `duckdb-transpiler-select-core.plan.md`
   - Promotes the top-level SELECT path: `ResolvedQueryStmt`,
     `ResolvedProjectScan`, `ResolvedSingleRowScan`, `ResolvedOutputColumn`,
     and `ResolvedComputedColumn`.
2. `duckdb-transpiler-expression-core.plan.md`
   - Promotes scalar expression plumbing: `ResolvedParameter`, `ResolvedCast`,
     `ResolvedFunctionArgument`, and `ResolvedWithExpr`.
3. `duckdb-transpiler-json-struct-completion.plan.md`
   - Promotes JSON field access and finishes the anonymous struct subset.
4. `duckdb-transpiler-setops-sample.plan.md`
   - Promotes set operations, set-operation items, and `TABLESAMPLE`.
5. `duckdb-transpiler-cte-subquery.plan.md`
   - Promotes non-recursive CTEs, CTE refs, and scalar / `IN` / `EXISTS` /
     `ARRAY` subqueries.
6. `duckdb-transpiler-aggregate-completion.plan.md`
   - Promotes grouping sets, rollup, cube, `GROUPING()`, aggregate modifiers,
     aggregate filters, and deferred aggregate functions.
7. `duckdb-transpiler-array-lateral.plan.md`
   - Promotes lateral `UNNEST`, `WITH OFFSET`, multi-array zip, outer array
     scans, `ResolvedColumnHolder`, and lateral join parameters.
8. `duckdb-transpiler-window-order-completion.plan.md`
   - Promotes the remaining analytic and order-by subsets: collation,
     parameter lists, `IGNORE/RESPECT NULLS`, and stricter RANGE handling.
9. `duckdb-transpiler-function-rewrites.plan.md`
   - Promotes deferred scalar functions in `functions.yaml` that need rewrites:
     math, conditional, regex, format, datetime, and parse/extract families.
10. `duckdb-transpiler-ddl.plan.md`
    - Promotes DDL-shaped statements currently routed outside the transpiler.
11. `duckdb-transpiler-dml.plan.md`
    - Promotes DML statements and DML-only helper shapes.
12. `duckdb-transpiler-procedural.plan.md`
    - Promotes scripting and procedural statements.
13. `duckdb-transpiler-udf-tvf-module.plan.md`
    - Promotes UDF, TVF, module, constant, and expression-mode shapes.
14. `duckdb-transpiler-complex-types.plan.md`
    - Promotes proto, JSON-adjacent, row/measure, and flatten shapes that need
      type-system support before they can lower.
15. `duckdb-transpiler-advanced-relational.plan.md`
    - Promotes recursive CTEs, pivot/unpivot, MATCH_RECOGNIZE, pipe operators,
      graph query scans, and optimizer barriers.
16. `duckdb-transpiler-privacy-specialized.plan.md`
    - Promotes differential privacy, anonymized aggregation, BigQuery ML,
      geography, networking, key-management, HLL, and other specialized
      function families.

## Shared Rules

- A row only flips to `done` when the corresponding `Emit*` path returns valid
  DuckDB SQL and has unit coverage in `transpiler_test.cc`.
- Any user-visible query shape promoted to `done` also needs conformance
  coverage pinned to the DuckDB path.
- Keep unsupported sub-shapes returning `""` until their own plan lands; do not
  silently emit approximations for BigQuery semantics DuckDB does not share.
- Run focused transpiler tests first. Run broader engine or conformance tests
  only after the focused unit surface is green.
