---
name: FixTests 06 — Fastpath relational scans + nested struct
overview: Close four fastpath-lane conformance failures in the DuckDB transpiler - ROW_NUMBER analytic emit, FULL OUTER JOIN null padding, TABLESAMPLE BERNOULLI, and nested STRUCT field-order projection.
depends_on: [fixtests-05-scalar-subquery]
est_effort: ~1 week
isProject: true
todos:
  - id: row-number
    content: Fix scan_analytic_row_number - ROW_NUMBER() OVER (PARTITION BY ... ORDER BY ...) partition-reset / off-by-one in the analytic emit.
    status: pending
  - id: full-join
    content: Fix scan_join_full - FULL OUTER JOIN unmatched-side NULL padding + COALESCE (match unordered).
    status: pending
  - id: bernoulli
    content: Fix scan_sample_bernoulli - TABLESAMPLE BERNOULLI emit; ensure schema_only match returns BQ type names (INT64 not INTEGER).
    status: pending
  - id: nested-struct
    content: Fix regression_struct_nested_field_order - nested STRUCT field projection / JSON wire field order in the duckdb_rewrite struct emit.
    status: pending
  - id: verify
    content: All four pass; task conformance:fastpath stays 20/20 and task conformance:run advances.
    status: pending
---

# FixTests 06 — Fastpath relational scans + nested struct

## Why

These four are in the **PR-gating fastpath lane** ([`conformance/fixtures/fastpath/`](conformance/fixtures/fastpath/)) and are transpiler/emit issues rather than semantic-executor gaps. The first three are `duckdb_native` scans; the fourth is a `duckdb_rewrite` struct shape.

> Validate each against the observed diff from [fixtests-01-foundation.plan.md](fixtests-01-foundation.plan.md). The `scan_sample_bernoulli` schema mismatch (`INTEGER` vs `INT64`) was observed directly during baseline.

## Fixtures and work

| Fixture | Shape / route | Work | Files |
|---------|---------------|------|-------|
| `scan_analytic_row_number` | `ResolvedAnalyticScan` -> `duckdb_native` | Partition reset / numbering off-by-one for `ROW_NUMBER() OVER (PARTITION BY kind ORDER BY id)` | analytic scan emit in [`backend/engine/duckdb/transpiler/`](backend/engine/duckdb/transpiler/) |
| `scan_join_full` | `ResolvedJoinScan` -> `duckdb_native` | `FULL OUTER JOIN` unmatched-side NULL padding + COALESCE (`match: unordered`) | join emit in transpiler |
| `scan_sample_bernoulli` | `ResolvedSampleScan` -> `duckdb_native` | `TABLESAMPLE BERNOULLI(50 PERCENT)`; if `EmitSampleScan` bails the query errors instead of `schema_only` pass. Also map DuckDB `INTEGER` -> BQ `INT64` in the schema response | sample-scan emit + [`gateway/bqtypes/wire.go`](gateway/bqtypes/wire.go) type mapping |
| `regression_struct_nested_field_order` | struct rewrite family -> `duckdb_rewrite` | Nested `STRUCT<..., c STRUCT<d,e>>` field projection / JSON wire field order | struct emit in transpiler |

## Note on the schema_only type-name mismatch

`scan_sample_bernoulli` baseline showed `id:INTEGER` where the fixture expects `id:INT64`. If this is a general wire-type leak (DuckDB native type names surfacing in the REST schema), check whether it also affects other `duckdb_native` fixtures and fix at the wire/type-mapping layer rather than per-fixture.

## Steps

1. Reproduce each; read the diff (rows vs schema vs transpile error).
2. Fix the emit/type-mapping; keep `tools/check_disposition_parity` green if any disposition changes.
3. Update [`SHAPE_TRACKER.md`](backend/engine/duckdb/transpiler/SHAPE_TRACKER.md) entries for the touched shapes.

## Verify

```bash
task emulator:build-engine:bazel
task conformance:fastpath        # stays 20/20 (must not regress)
task conformance:run             # advances toward 100/100
```

## Out of scope

- pivot/unpivot/recursive_cte (plan 07); scalar/subquery (plan 05).
