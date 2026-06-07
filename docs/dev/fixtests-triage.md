# FixTests triage (living doc)

Latest baseline: **2026-06-07** (plan 02 landed). See commit history for prior baselines.

## Baseline counts (2026-06-07, post plan 02)

| Lane | Result |
|------|--------|
| `task conformance:fastpath` | **17/20** pass (+2 from 15/20; 3 remaining are plan 06) |
| `task conformance:run` | **89/100** pass (+9 from 80/100) |
| `task conformance:bqutils` | **61/61** pass |
| `GOOGLESQL_SOURCE=prebuilt task lint:cpp:test` | **exit 201** — abseil `@@abseil-cpp~//absl/statusor` analysis failure |

## cc_test

Not exit 37. Analysis fails on `//backend/engine/duckdb:duckdb_executor_routing_test` — missing `@@abseil-cpp~//absl/statusor` package. Plan 03.

## Plan 02 resolved (8 fixtures)

| Fixture | Fix |
|---------|-----|
| `specialized_unsupported_engine_policy_link` | `generate_uuid` → `unsupported` in `functions.yaml` |
| `specialized_unsupported_session_user` | `session_user` → `unsupported` |
| `regression_integer_overflow` | `message_contains: "Int64 overflow"` |
| `specialized_keys_new_keyset_stub` | BYTES expected as base64 wire |
| `function_isnull` | Catalog stub routes `ISNULL` → `bq_isnull` duckdb_udf |
| `function_log` | FLOAT64 wire expectations for two-arg LOG |
| `regression_struct_anonymous` | `._N` → `[OFFSET(N)]` SQL preprocess |
| `expr_array_literal` | `mode: REPEATED` + INTEGER↔INT64 schema alias in runner |

## Per-fixture classes (11 failures remaining)

| Fixture | Class | Owner plan |
|---------|-------|------------|
| `pivot`, `unpivot` | route_mismatch (`semantic_executor` vs `duckdb_rewrite`) | 07 |
| `recursive_cte` | transpile_501 | 07 |
| `subquery_expr_array` | row_mismatch | 05 |
| `subquery_expr_in` | transpile_501 | 05 |
| `subquery_expr_scalar` | semantic_501 (max) | 05 |
| `expr_with_expr` | parse_400 | 05 |
| `expr_cast_types` | row_mismatch | 05 |
| `regression_struct_nested_field_order` | parse_400 | 06 |
| `scan_analytic_row_number` | row_mismatch | 06 |
| `scan_join_full` | transpile_501 | 06 |
| `scan_sample_bernoulli` | schema_mismatch (`INTEGER` vs `INT64`) | 06 |
