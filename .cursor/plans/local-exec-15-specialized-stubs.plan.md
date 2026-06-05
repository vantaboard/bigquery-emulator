---
name: local-exec-15-specialized-stubs
overview: Resolve specialized-feature 501s and NET.* / UDF tests (~24 tests) with local impl or deterministic stubs.
todos:
  - id: gsql-15-specialized-stubs
    content: Specialized/NET/UDF policy and minimal implementations
    status: pending
isProject: false
---

# query port 15: Specialized / NET / UDF Stubs

## Goal

Resolve specialized-feature 501s and NET.* / UDF tests (~24 tests) with local impl or deterministic stubs.

Baseline: `20260603T035812Z` (24 failing tests in this bucket).

## Dependencies

- [`local-exec-14-dml-system.plan.md`](local-exec-14-dml-system.plan.md)

## Root cause

- `query_port_test.go:8045: jobs.query -> 501: {"error":{"code":501,"message":"local coordinator: ExecuteQuery on route 'unsupported' is not implemented (statement kind: QueryStmt, family: fun...`
- `query_port_test.go:8045: jobs.query -> 501: {"error":{"code":501,"message":"control op executor: CREATE FUNCTION registration is not implemented yet; needs a per-engine functions registry. ...`
- `query_port_test.go:8045: jobs.query -> 400: {"error":{"code":400,"message":"3:1: Syntax error: Expected end of input but got keyword SELECT [at 3:1]","errors":[{"reason":"invalidQuery","mes...`

## Primary files

- [`backend/engine/semantic/stubs/`](../../backend/engine/semantic/stubs/)
- [`backend/engine/coordinator/route_classifier.cc`](../../backend/engine/coordinator/route_classifier.cc)
- [`backend/engine/duckdb/transpiler/functions.yaml`](../../backend/engine/duckdb/transpiler/functions.yaml)

## Implementation steps

1. Replace legacy specialized-feature plan references in 501 messages with this plan.
2. For each failing specialized family, choose local_impl vs local_stub vs unsupported per test expectation.
3. Implement or stub `net.*` functions referenced by `TestQuery/net_*` subtests.
4. Wire CREATE FUNCTION / temp function tests to udf routing or explicit unsupported errors matching query port expectations.

## Verify

Use `TestQuery$/^(subtest|…)$` — not `^(TestQuery/subtest$|…)` (matches zero tests in Go).

Session result (2026-06-03): **15/24 pass**; 9 fail: `approx_quantiles_2` ($agg1 binding), `approx_quantiles_with_null` / `_respect_nulls`, all `approx_top_*` (struct field types).

```bash
BIGQUERY_EMULATOR_BIN=./bin/emulator_main \
  go test -tags=integration ./gateway/e2e/ \
  -run 'TestQuery$/^(approx_count_distinct|approx_quantiles|approx_quantiles_2|approx_quantiles_with_distinct|approx_quantiles_with_null|approx_quantiles_with_respect_nulls|approx_top_count|approx_top_count_with_null|approx_top_sum|approx_top_sum_with_null|approx_top_sum_with_null_2|approx_top_sum_with_null_3|case_with_case_that_causes_errors|create_function|create_temp_function|error|generate_uuid|if_with_case_that_causes_errors|ifnull_with_case_that_causes_errors|net_ip_net_mask_with_invalid_output_bytes|net_ip_net_mask_with_invalid_prefix_length|net_ip_trunc_with_invalid_ip_address|net_ip_trunc_with_invalid_length|session_user)$' \
  -count=1 -parallel 1
```

## Done when

- All 24 tests listed below pass.
- `./gateway/e2e/testresults/run_query_port_tests.sh` fail count drops by ~24.

## Failing tests

- `TestQuery/approx_count_distinct`
- `TestQuery/approx_quantiles`
- `TestQuery/approx_quantiles_2`
- `TestQuery/approx_quantiles_with_distinct`
- `TestQuery/approx_quantiles_with_null`
- `TestQuery/approx_quantiles_with_respect_nulls`
- `TestQuery/approx_top_count`
- `TestQuery/approx_top_count_with_null`
- `TestQuery/approx_top_sum`
- `TestQuery/approx_top_sum_with_null`
- `TestQuery/approx_top_sum_with_null_2`
- `TestQuery/approx_top_sum_with_null_3`
- `TestQuery/case_with_case_that_causes_errors`
- `TestQuery/create_function`
- `TestQuery/create_temp_function`
- `TestQuery/error`
- `TestQuery/generate_uuid`
- `TestQuery/if_with_case_that_causes_errors`
- `TestQuery/ifnull_with_case_that_causes_errors`
- `TestQuery/net_ip_net_mask_with_invalid_output_bytes`
- `TestQuery/net_ip_net_mask_with_invalid_prefix_length`
- `TestQuery/net_ip_trunc_with_invalid_ip_address`
- `TestQuery/net_ip_trunc_with_invalid_length`
- `TestQuery/session_user`
