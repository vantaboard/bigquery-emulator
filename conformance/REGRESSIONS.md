# Reported bug regression index

Permanent fixtures for defects reported in the v0.3.0 → v0.5.0 email thread.
Each tag must stay pinned so fixes cannot silently regress.

**Policy:** every user-reported fix ships its pinning artifact in the same PR.
See [`.cursor/rules/pin-reported-bugs.mdc`](../.cursor/rules/pin-reported-bugs.mdc).

**Origin plan:** [`.cursor/plans/conformance-hardening/07-reported-bug-regression-fixtures.plan.md`](../.cursor/plans/conformance-hardening/07-reported-bug-regression-fixtures.plan.md)

## Index

| Tag | Symptom | Primary lane | Fix / status |
|-----|---------|--------------|--------------|
| R1 | `CREATE OR REPLACE TABLE \`ds.t\`` parsed as one segment → "no defaultDataset" | core_usage fixture | `control_op_ddl` qualified-name parsing; fixture `5cf0b47d` |
| R2 | View created via client returns 0 rows (worked in UI) | core_usage + session | `3964e2bd`, `4b7abe5d`, `7a279ac1` view persistence |
| R3 | After view fix, `tables.list` on source dataset returns empty | session | `0c8f3408` session harness + dataset list assertion |
| R4 | Engine abort on duplicate catalog name during view replay | session + catalog unit test | `c568e6d6`, `93717eda` crash-safety guards |
| R5 | Views not persisted across container restart (client path) | session (`restart:`) + e2e | plan 08: harness base URL + view rehydrate |
| R6 | Naive TIMESTAMP param `'2026-06-22T10:00:00'` rejected | differential + e2e matrix | `3daff670`, `462a9578` param wire forms |
| R7 | `UNION DISTINCT` → "SetOperationScan op is not UNION ALL" | setops fixture + differential | `6bf52da6` semantic UNION DISTINCT |
| R8 | CTE in subquery → "WithRefScan without active WithScan bindings" | cte_subquery + differential | `a7e968ff` materialized WITH bindings |
| R9 | Anti-join over QUALIFY-deduped views → DuckDB "column id not found" | differential + transpiler test | `e123bec5`, `0f7f054a` join binding fix |
| R10 | Correlated / cross-product UNNEST → DuckDB "column id not found" (only `__bq_input_rn`) | conformance + transpiler test | UNNEST id-alias output mapping fix |

## Paths by tag

### R1 — backtick-qualified DDL target

- `conformance/fixtures/core_usage/qualified_names/create_or_replace_table_backtick_qualified.yaml`
- `third_party/scenarios/python/test_dedup_ctas.py`

### R2 — REST-created view returns rows

- `conformance/fixtures/core_usage/views/view_rest_insert_select_rows.yaml`
- `conformance/sessions/view_select_after_create_via_client_path.yaml`

### R3 — dataset list after view authorization

- `conformance/sessions/dataset_list_after_view_op.yaml`

### R4 — authorize-view repeat must not abort

- `conformance/sessions/authorize_view_repeat.yaml`
- `backend/catalog/catalog_crash_safety_test.cc`

### R5 — client view survives restart

- `conformance/sessions/restart_view_durability.yaml`
- `gateway/e2e/restart_durability_test.go`

### R6 — naive ISO TIMESTAMP parameter

- `conformance/differential/corpus/timestamp_param_naive.yaml`
- `gateway/e2e/query_params_matrix_test.go` (`timestamp_naive_iso` case)
- `third_party/scenarios/python/test_dashboard_params.py`

### R7 — UNION DISTINCT

- `conformance/fixtures/setops/set_op_union_distinct.yaml`
- `conformance/differential/corpus/set_op_union_distinct.yaml`
- `conformance/differential/corpus/union_distinct_cte_rollup.yaml`

### R8 — CTE referenced from scalar subquery

- `conformance/fixtures/cte_subquery/with_scan_scalar_subquery.yaml`
- `conformance/differential/corpus/cte_scalar_subquery.yaml`

### R9 — QUALIFY-dedup anti-join binding

- `conformance/differential/corpus/orphan_orders_antijoin.yaml`
- `backend/engine/duckdb/transpiler/transpiler_emit_composition_test.cc` (`OrphanOrdersQualifyDedupAntiJoinBinds`)
- `third_party/scenarios/python/test_orphan_orders.py`

### R10 — UNNEST correlated / cross-product binding

- `conformance/fixtures/array_struct/cross_join_unnest.yaml`
- `conformance/fixtures/core_usage/everyday_sql/unnest_array.yaml`
- `conformance/fixtures/dml/update_delete_array_offset.yaml`
- `conformance/fixtures/fastpath/scan_array_unnest_cross_join.yaml`
- `conformance/fixtures/fastpath/scan_array_unnest_cross_join_three.yaml`
- `backend/engine/duckdb/transpiler/transpiler_emit_composition_test.cc` (`CorrelatedUnnestFromTableBinds`, `CoreUsageUnnestArrayShapeBinds`, `NestedUnnestCrossProductBinds`)

## Machine-readable index

Parsed by `go test ./conformance/ -run TestRegressionsIndexPathsExist`.

```regressions-index
R1:
  - conformance/fixtures/core_usage/qualified_names/create_or_replace_table_backtick_qualified.yaml
  - third_party/scenarios/python/test_dedup_ctas.py
R2:
  - conformance/fixtures/core_usage/views/view_rest_insert_select_rows.yaml
  - conformance/sessions/view_select_after_create_via_client_path.yaml
R3:
  - conformance/sessions/dataset_list_after_view_op.yaml
R4:
  - conformance/sessions/authorize_view_repeat.yaml
  - backend/catalog/catalog_crash_safety_test.cc
R5:
  - conformance/sessions/restart_view_durability.yaml
  - gateway/e2e/restart_durability_test.go
R6:
  - conformance/differential/corpus/timestamp_param_naive.yaml
  - gateway/e2e/query_params_matrix_test.go
  - third_party/scenarios/python/test_dashboard_params.py
R7:
  - conformance/fixtures/setops/set_op_union_distinct.yaml
  - conformance/differential/corpus/set_op_union_distinct.yaml
  - conformance/differential/corpus/union_distinct_cte_rollup.yaml
R8:
  - conformance/fixtures/cte_subquery/with_scan_scalar_subquery.yaml
  - conformance/differential/corpus/cte_scalar_subquery.yaml
R9:
  - conformance/differential/corpus/orphan_orders_antijoin.yaml
  - backend/engine/duckdb/transpiler/transpiler_emit_composition_test.cc
  - third_party/scenarios/python/test_orphan_orders.py
R10:
  - conformance/fixtures/array_struct/cross_join_unnest.yaml
  - conformance/fixtures/core_usage/everyday_sql/unnest_array.yaml
  - conformance/fixtures/dml/update_delete_array_offset.yaml
  - conformance/fixtures/fastpath/scan_array_unnest_cross_join.yaml
  - conformance/fixtures/fastpath/scan_array_unnest_cross_join_three.yaml
```
