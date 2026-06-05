---
name: local-exec-04-scan-emits
overview: Close missing DuckDB emit coverage for core scan nodes blocking ~56 query port tests.
todos:
  - id: gsql-04-scan-emits
    content: Implement missing DuckDB scan emit methods
    status: pending
isProject: false
---

# query port 04: Scan Emits (ProjectScan, OrderByScan, …)

## Goal

Close missing DuckDB emit coverage for core scan nodes blocking ~56 query port tests.

Baseline: `20260603T035812Z` (62 failing tests in this bucket).

## Dependencies

- [`local-exec-02-withscan-cte.plan.md`](local-exec-02-withscan-cte.plan.md)
- [`local-exec-03-operator-disposition.plan.md`](local-exec-03-operator-disposition.plan.md)

## Root cause

- `I0000 00:00:1780459094.934444  399173 transpiler.cc:1924] duckdb transpiler: aggregate 'array_agg' uses a modifier (HAVING / ORDER BY / LIMIT / GROUP BY / NULL-handling) that has no DuckDB analog y...`
- `query_port_test.go:8045: jobs.query -> 501: {"error":{"code":501,"message":"duckdb engine: transpiler does not yet cover this query shape (family: node:ProjectScan, route: duckdb_native); s...`
- `I0000 00:00:1780459094.928458  399173 transpiler.cc:1924] duckdb transpiler: aggregate 'array_agg' uses a modifier (HAVING / ORDER BY / LIMIT / GROUP BY / NULL-handling) that has no DuckDB analog y...`

## Primary files

- [`backend/engine/duckdb/transpiler/transpiler.cc`](../../backend/engine/duckdb/transpiler/transpiler.cc)
- [`backend/engine/duckdb/duckdb_executor.cc`](../../backend/engine/duckdb/duckdb_executor.cc)
- [`backend/engine/duckdb/transpiler/SHAPE_TRACKER.md`](../../backend/engine/duckdb/transpiler/SHAPE_TRACKER.md)

## Implementation steps

1. Implement or complete `EmitProjectScan` for SELECT-list / computed-column projections.
2. Implement `EmitOrderByScan`, `EmitLimitOffsetScan`, and `EmitSetOperationScan` where still returning empty SQL.
3. Update 501 error strings to cite `local-exec-04-scan-emits.plan.md`.
4. Add one conformance fixture per newly supported scan kind.
5. Run plan verify regex against `TestQuery` ProjectScan failures.

## Verify

```bash
BIGQUERY_EMULATOR_BIN=./bin/emulator_main \
  go test -tags=integration ./gateway/e2e/ \
  -run '^TestQuery/(array_agg_with_abs|array_agg_with_ignore_nulls|array_agg_with_ignore_nulls_and_struct|array_agg_with_limit|array_concat_agg|array_concat_agg_with_format|case\-when|case\-when_with_compare|count_with_if|countif|countif_with_null|countif_with_window|extract_date|extract_from_interval|generate_date_array_function_with_variable|generate_timestamp_array_function_with_variable|having_with_union_all|hll_count\.extract|hll_count\.init|hll_count\.merge|hll_count\.merge_partial|interval_operator|join_nested_with|json_extract_for_format|json_extract_for_name|json_extract_with_array|json_extract_with_escape|json_query_for_format|json_query_for_name|json_query_with_array|json_query_with_escape|json_type|json_value_subscript_operator|make_array|net_host|net_ip_from_string|net_ip_net_mask|net_ip_to_string|net_ip_trunc|net_ipv4_from_int64|net_ipv4_to_int64|net_safe_if_from_string|normalize|normalize_and_casefold|percentile_cont|percentile_disc|percentile_disc_with_respect_nulls|regexp_extract|repeat|rpad_bytes|rpad_bytes_with_pattern|rpad_string|rpad_string_with_pattern|safe_sum|string_agg_with_distinct_and_order_by_and_limit|string_agg_with_limit|string_agg_with_order_by|subquery_expr_with_scalar_type_at_HAVING|subquery_expr_with_scalar_type_at_WHERE|timestamp_trunc_with_week|to_code_points_with_bytes_value|to_code_points_with_string_value)$' \
  -count=1 -parallel 1
```

## Done when

- All 62 tests listed below pass.
- `./gateway/e2e/testresults/run_query_port_tests.sh` fail count drops by ~62.

## Failing tests

- `TestQuery/array_agg_with_abs`
- `TestQuery/array_agg_with_ignore_nulls`
- `TestQuery/array_agg_with_ignore_nulls_and_struct`
- `TestQuery/array_agg_with_limit`
- `TestQuery/array_concat_agg`
- `TestQuery/array_concat_agg_with_format`
- `TestQuery/case-when`
- `TestQuery/case-when_with_compare`
- `TestQuery/count_with_if`
- `TestQuery/countif`
- `TestQuery/countif_with_null`
- `TestQuery/countif_with_window`
- `TestQuery/extract_date`
- `TestQuery/extract_from_interval`
- `TestQuery/generate_date_array_function_with_variable`
- `TestQuery/generate_timestamp_array_function_with_variable`
- `TestQuery/having_with_union_all`
- `TestQuery/hll_count.extract`
- `TestQuery/hll_count.init`
- `TestQuery/hll_count.merge`
- `TestQuery/hll_count.merge_partial`
- `TestQuery/interval_operator`
- `TestQuery/join_nested_with`
- `TestQuery/json_extract_for_format`
- `TestQuery/json_extract_for_name`
- `TestQuery/json_extract_with_array`
- `TestQuery/json_extract_with_escape`
- `TestQuery/json_query_for_format`
- `TestQuery/json_query_for_name`
- `TestQuery/json_query_with_array`
- `TestQuery/json_query_with_escape`
- `TestQuery/json_type`
- `TestQuery/json_value_subscript_operator`
- `TestQuery/make_array`
- `TestQuery/net_host`
- `TestQuery/net_ip_from_string`
- `TestQuery/net_ip_net_mask`
- `TestQuery/net_ip_to_string`
- `TestQuery/net_ip_trunc`
- `TestQuery/net_ipv4_from_int64`
- `TestQuery/net_ipv4_to_int64`
- `TestQuery/net_safe_if_from_string`
- `TestQuery/normalize`
- `TestQuery/normalize_and_casefold`
- `TestQuery/percentile_cont`
- `TestQuery/percentile_disc`
- `TestQuery/percentile_disc_with_respect_nulls`
- `TestQuery/regexp_extract`
- `TestQuery/repeat`
- `TestQuery/rpad_bytes`
- `TestQuery/rpad_bytes_with_pattern`
- `TestQuery/rpad_string`
- `TestQuery/rpad_string_with_pattern`
- `TestQuery/safe_sum`
- `TestQuery/string_agg_with_distinct_and_order_by_and_limit`
- `TestQuery/string_agg_with_limit`
- `TestQuery/string_agg_with_order_by`
- `TestQuery/subquery_expr_with_scalar_type_at_HAVING`
- `TestQuery/subquery_expr_with_scalar_type_at_WHERE`
- `TestQuery/timestamp_trunc_with_week`
- `TestQuery/to_code_points_with_bytes_value`
- `TestQuery/to_code_points_with_string_value`
