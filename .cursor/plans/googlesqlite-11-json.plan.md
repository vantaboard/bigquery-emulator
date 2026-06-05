---
name: googlesqlite-11-json
overview: Implement JSON extract/query/value/array functions (~42 tests).
todos:
  - id: gsql-11-json
    content: Semantic JSON function family
    status: pending
isProject: false
---

# googlesqlite 11: Semantic JSON Functions

## Goal

Implement JSON extract/query/value/array functions (~42 tests).

Baseline: `20260603T035812Z` (42 failing tests in this bucket).

## Dependencies

- [`googlesqlite-10-string-hash-format.plan.md`](googlesqlite-10-string-hash-format.plan.md)

## Root cause

- `googlesqlite_query_test.go:8045: jobs.query -> 501: {"error":{"code":501,"message":"semantic: function 'bool' is not yet implemented in the semantic executor","errors":[{"reason":"notImplemented","...`
- `googlesqlite_query_test.go:8045: jobs.query -> 501: {"error":{"code":501,"message":"semantic: function 'json_extract' is not yet implemented in the semantic executor","errors":[{"reason":"notImplem...`
- `googlesqlite_query_test.go:8045: jobs.query -> 501: {"error":{"code":501,"message":"semantic: function 'json_extract_array' is not yet implemented in the semantic executor","errors":[{"reason":"not...`

## Primary files

- [`backend/engine/semantic/functions/`](../../backend/engine/semantic/functions/)
- [`backend/engine/semantic/eval_expr.cc`](../../backend/engine/semantic/eval_expr.cc)

## Implementation steps

1. Implement `json_extract`, `json_query`, `json_value`, `json_extract_scalar` with escape/format args.
2. Implement array variants: `json_extract_array`, `json_query_array`, `json_value_array`, string array forms.
3. Implement `parse_json`, `to_json`, `to_json_string`, and JSON type helpers (`json_bool`, `json_int64`, …).
4. Match BigQuery NULL and empty-array edge cases from `TestQuery/json_*` subtests.

## Verify

Use `TestQuery$/^(subtest|…)$` — not `^(TestQuery/subtest$|…)` (matches `TestQuerySemantic*`).

```bash
BIGQUERY_EMULATOR_BIN=./bin/emulator_main \
  go test -tags=integration ./gateway/e2e/ \
  -run 'TestQuery$/^(json_bool|json_extract|json_extract_and_null|json_extract_array|json_extract_array_filter|json_extract_array_format|json_extract_array_with_empty_array|json_extract_array_with_escape|json_extract_array_with_integer|json_extract_array_with_null|json_extract_scalar_with_array|json_extract_scalar_with_escape|json_extract_scalar_with_number|json_extract_scalar_with_string|json_extract_string_array|json_extract_string_array_with_empty_array|json_extract_string_array_with_escape|json_extract_string_array_with_null|json_extract_string_array_with_root_only|json_float64|json_int64|json_query|json_query_and_null|json_query_array|json_query_array_filter|json_query_array_format|json_query_array_with_empty_array|json_query_array_with_escape|json_query_array_with_integer|json_query_array_with_null|json_string|json_value_array|json_value_array_with_empty_array|json_value_array_with_escape|json_value_array_with_null|json_value_array_with_root_only|json_value_with_array|json_value_with_escape|json_value_with_null|json_value_with_number|json_value_with_string|parse_json)$' \
  -count=1 -parallel 1
```

## Done when

- All 42 tests listed below pass.
- `./gateway/e2e/testresults/run_googlesqlite_emulator_tests.sh` fail count drops by ~42.

## Failing tests

- `TestQuery/json_bool`
- `TestQuery/json_extract`
- `TestQuery/json_extract_and_null`
- `TestQuery/json_extract_array`
- `TestQuery/json_extract_array_filter`
- `TestQuery/json_extract_array_format`
- `TestQuery/json_extract_array_with_empty_array`
- `TestQuery/json_extract_array_with_escape`
- `TestQuery/json_extract_array_with_integer`
- `TestQuery/json_extract_array_with_null`
- `TestQuery/json_extract_scalar_with_array`
- `TestQuery/json_extract_scalar_with_escape`
- `TestQuery/json_extract_scalar_with_number`
- `TestQuery/json_extract_scalar_with_string`
- `TestQuery/json_extract_string_array`
- `TestQuery/json_extract_string_array_with_empty_array`
- `TestQuery/json_extract_string_array_with_escape`
- `TestQuery/json_extract_string_array_with_null`
- `TestQuery/json_extract_string_array_with_root_only`
- `TestQuery/json_float64`
- `TestQuery/json_int64`
- `TestQuery/json_query`
- `TestQuery/json_query_and_null`
- `TestQuery/json_query_array`
- `TestQuery/json_query_array_filter`
- `TestQuery/json_query_array_format`
- `TestQuery/json_query_array_with_empty_array`
- `TestQuery/json_query_array_with_escape`
- `TestQuery/json_query_array_with_integer`
- `TestQuery/json_query_array_with_null`
- `TestQuery/json_string`
- `TestQuery/json_value_array`
- `TestQuery/json_value_array_with_empty_array`
- `TestQuery/json_value_array_with_escape`
- `TestQuery/json_value_array_with_null`
- `TestQuery/json_value_array_with_root_only`
- `TestQuery/json_value_with_array`
- `TestQuery/json_value_with_escape`
- `TestQuery/json_value_with_null`
- `TestQuery/json_value_with_number`
- `TestQuery/json_value_with_string`
- `TestQuery/parse_json`
