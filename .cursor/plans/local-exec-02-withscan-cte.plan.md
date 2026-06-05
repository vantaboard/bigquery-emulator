---
name: local-exec-02-withscan-cte
overview: Implement DuckDB lowering for `ResolvedWithScan` / `ResolvedWithRefScan` to unblock ~58 query port tests.
todos:
  - id: gsql-02-withscan-cte
    content: Implement EmitWithScan/EmitWithRefScan in DuckDB transpiler
    status: pending
isProject: false
---

# query port 02: WithScan / CTE Emit

## Goal

Implement DuckDB lowering for `ResolvedWithScan` / `ResolvedWithRefScan` to unblock ~58 query port tests.

Baseline: `20260603T035812Z` (51 failing tests in this bucket).

## Dependencies

- [`local-exec-01-ddl-catalog.plan.md`](local-exec-01-ddl-catalog.plan.md)

## Root cause

- `I0000 00:00:1780459094.089375  399173 transpiler.cc:1836] duckdb transpiler: function '$equal' has no disposition; surfacing UNIMPLEMENTED`
- `query_port_test.go:145: rows.Err: jobs.query -> 501: {"error":{"code":501,"message":"duckdb engine: transpiler does not yet cover this query shape (family: node:WithScan, route: duckdb_nati...`
- `I0000 00:00:1780459094.910303  399173 transpiler.cc:1924] duckdb transpiler: aggregate 'array_agg' uses a modifier (HAVING / ORDER BY / LIMIT / GROUP BY / NULL-handling) that has no DuckDB analog y...`

## Primary files

- [`backend/engine/duckdb/transpiler/transpiler.cc`](../../backend/engine/duckdb/transpiler/transpiler.cc)
- [`backend/engine/duckdb/transpiler/transpiler.h`](../../backend/engine/duckdb/transpiler/transpiler.h)
- [`conformance/fixtures/cte_subquery/`](../../conformance/fixtures/cte_subquery/)

## Implementation steps

1. Implement `EmitWithScan` to emit `WITH cte AS (subquery) …` preserving column aliases.
2. Implement `EmitWithRefScan` for CTE references inside the same query.
3. Wire emit paths through existing join / scan recursion; fail closed only when a child shape is unsupported.
4. Add conformance fixture asserting `duckdb_native` route for non-recursive CTE.
5. Re-run plan verify regex; expect `TestJoinVariants` and CTE-heavy `TestQuery` rows to pass.

## Verify

```bash
BIGQUERY_EMULATOR_BIN=./bin/emulator_main \
  go test -tags=integration ./gateway/e2e/ \
  -run '^(TestJoinVariants$|TestQuery/array_agg_with_null_in_order_by$|TestQuery/array_boundary_indexing_test$|TestQuery/array_concat_agg_with_limt$|TestQuery/array_concat_agg_with_null_in_order_by$|TestQuery/array_index_access_operator$|TestQuery/cume_dist$|TestQuery/date_type$|TestQuery/extract_from_timestamp$|TestQuery/farm_fingerprint$|TestQuery/full_join$|TestQuery/group_by_having$|TestQuery/initcap$|TestQuery/initcap_with_delimiters$|TestQuery/inner_join_with_using$|TestQuery/lag_with_option$|TestQuery/lead$|TestQuery/lead_with_default$|TestQuery/lead_with_offset$|TestQuery/left_join$|TestQuery/nested_with$|TestQuery/normalize_and_casefold_with_params$|TestQuery/normalize_with_nfkc$|TestQuery/nth_value$|TestQuery/ntile$|TestQuery/out_of_range_error$|TestQuery/percent_rank$|TestQuery/percentile_cont_non_zero_min_sorted$|TestQuery/percentile_cont_non_zero_min_unsorted$|TestQuery/qualify$|TestQuery/qualify_direct$|TestQuery/qualify_group$|TestQuery/qualify_without_group_by_/_where_/_having$|TestQuery/regexp_extract_all$|TestQuery/regexp_extract_with_capture$|TestQuery/regexp_extract_with_position_and_occurrence$|TestQuery/regexp_instr$|TestQuery/regexp_instr_with_occurrence$|TestQuery/regexp_instr_with_occurrence_position$|TestQuery/regexp_instr_with_position$|TestQuery/regexp_substr$|TestQuery/replace\#02$|TestQuery/right_bytes$|TestQuery/right_join$|TestQuery/right_string$|TestQuery/row_number_nest$|TestQuery/subselect_qualifier$|TestQuery/to_json$|TestQuery/to_json_string$|TestQuery/translate$|TestQuery/window_rank$)' \
  -count=1 -parallel 1
```

## Done when

- All 51 tests listed below pass.
- `./gateway/e2e/testresults/run_query_port_tests.sh` fail count drops by ~51.

## Failing tests

- `TestJoinVariants`
- `TestQuery/array_agg_with_null_in_order_by`
- `TestQuery/array_boundary_indexing_test`
- `TestQuery/array_concat_agg_with_limt`
- `TestQuery/array_concat_agg_with_null_in_order_by`
- `TestQuery/array_index_access_operator`
- `TestQuery/cume_dist`
- `TestQuery/date_type`
- `TestQuery/extract_from_timestamp`
- `TestQuery/farm_fingerprint`
- `TestQuery/full_join`
- `TestQuery/group_by_having`
- `TestQuery/initcap`
- `TestQuery/initcap_with_delimiters`
- `TestQuery/inner_join_with_using`
- `TestQuery/lag_with_option`
- `TestQuery/lead`
- `TestQuery/lead_with_default`
- `TestQuery/lead_with_offset`
- `TestQuery/left_join`
- `TestQuery/nested_with`
- `TestQuery/normalize_and_casefold_with_params`
- `TestQuery/normalize_with_nfkc`
- `TestQuery/nth_value`
- `TestQuery/ntile`
- `TestQuery/out_of_range_error`
- `TestQuery/percent_rank`
- `TestQuery/percentile_cont_non_zero_min_sorted`
- `TestQuery/percentile_cont_non_zero_min_unsorted`
- `TestQuery/qualify`
- `TestQuery/qualify_direct`
- `TestQuery/qualify_group`
- `TestQuery/qualify_without_group_by_/_where_/_having`
- `TestQuery/regexp_extract_all`
- `TestQuery/regexp_extract_with_capture`
- `TestQuery/regexp_extract_with_position_and_occurrence`
- `TestQuery/regexp_instr`
- `TestQuery/regexp_instr_with_occurrence`
- `TestQuery/regexp_instr_with_occurrence_position`
- `TestQuery/regexp_instr_with_position`
- `TestQuery/regexp_substr`
- `TestQuery/replace#02`
- `TestQuery/right_bytes`
- `TestQuery/right_join`
- `TestQuery/right_string`
- `TestQuery/row_number_nest`
- `TestQuery/subselect_qualifier`
- `TestQuery/to_json`
- `TestQuery/to_json_string`
- `TestQuery/translate`
- `TestQuery/window_rank`
