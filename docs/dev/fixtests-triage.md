# FixTests triage (living doc)

Latest baseline: **2026-06-08** (plan 14 third-party bar). See commit history for prior baselines.

## First-party prerequisites (2026-06-08, pre plan 14)

| Lane | Result |
|------|--------|
| `task conformance:run` | **100/100** pass |
| `task conformance:bqutils` | **111/111** pass (105/111 runner) |
| `GOOGLESQL_SOURCE=prebuilt task lint:cpp:test` | **green** |

## Third-party `task thirdparty` (plan 14, 2026-06-08)

Run: `THIRDPARTY_FRESH_VOLUME=1 THIRDPARTY_SKIP_BUILD=1 BIGQUERY_EMULATOR_HOST=localhost:9050 task thirdparty` after `docker build --build-arg ENGINE_SOURCE=prebuilt -t bigquery-emulator:local .` (host `bin/emulator_main` must match the stabilized engine).

| Suite | Index baseline (2026-06-05) | After plan 14 (2026-06-08) | Delta / owner |
|-------|----------------------------|----------------------------|---------------|
| `golang-bigquery-tests` | OK | pass=23 skip=2 **fail=1** | `TestQueries` / `queryPartitionedTable`: `CAST STRING to TIMESTAMP` for `1800-01-01` → [thirdparty-09-public-data-seed](.cursor/plans/thirdparty-09-public-data-seed.plan.md) |
| `python-bigquery-tests` | **34 failed** | **1 failed**, 65 passed, 11 skipped | `test_query_script`: `SET` not supported in script executor → script-variable lane (not in index; track with conformance script work) |
| `node-bigquery-tests` | **59 failed** | **107 passing** | **Green** — no sub-plan routing |
| `java-bigquery-tests` | panic / storage gRPC | **OK** (allowlisted gRPC ITs only; `QueryMaterializedViewIT` passes) | Shallow backends still on allowlist → [thirdparty-10-storage-grpc](.cursor/plans/thirdparty-10-storage-grpc.plan.md) for non-allowlisted storage IT hardening |
| `python-bigquery-dataframes-snippet-gate` | **33 errors** | **1 failed**, 3 passed (4 allowlisted) | `test_performance_optimizations`: DuckDB STRUCT cast → [thirdparty-11-bigframes-gate](.cursor/plans/thirdparty-11-bigframes-gate.plan.md) |
| `dbt-bigquery-tests` (manual) | not measured | **2 collection errors** (pytest import/collection) | Manual lane; see `third_party/README.md` dbt section |

**Harness note:** A naive `task thirdparty` image rebuild (`ensure_emulator_image_fresh` without `THIRDPARTY_SKIP_BUILD=1`) currently fails in-container `googlesql-source` Bazel (`googlesql/public/simple_catalog.h` missing). CI and local triage should use `ENGINE_SOURCE=prebuilt` (artifact or host `bin/`) plus `THIRDPARTY_SKIP_BUILD=1`. Default docker `ENGINE_SOURCE=bazel` rebuild is a separate infra item.

**CI (plan 14):** `thirdparty-samples.yml` now gates python nox snippets, node Mocha, and bigframes snippet-gate on `workflow_run` from `build-engine`, sharing `.github/actions/setup-thirdparty-docker-emulator` (engine artifact + fake-gcs seed).

## Baseline counts (2026-06-07, post plan 02) — archived

| Lane | Result |
|------|--------|
| `task conformance:fastpath` | **17/20** pass (+2 from 15/20; 3 remaining are plan 06) |
| `task conformance:run` | **89/100** pass (+9 from 80/100) |
| `task conformance:bqutils` | **61/61** pass |
| `GOOGLESQL_SOURCE=prebuilt task lint:cpp:test` | **exit 201** — abseil `@@abseil-cpp~//absl/statusor` analysis failure |

## cc_test (2026-06-07 archive)

Was failing on `@@abseil-cpp~//absl/statusor`; **green as of 2026-06-08** (plan 03 landed).

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
