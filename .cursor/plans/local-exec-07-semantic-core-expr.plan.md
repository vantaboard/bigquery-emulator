---
name: local-exec-07-semantic-core-expr
overview: Extend semantic executor scan walk and expression kinds for ~22 query port tests.
todos:
  - id: gsql-07-semantic-core-expr
    content: Semantic scan walk + SubqueryExpr/struct/array expr kinds
    status: pending
isProject: false
---

# query port 07: Semantic Core Expressions / Scan Walk

## Goal

Extend semantic executor scan walk and expression kinds for ~22 query port tests.

Baseline: `20260603T035812Z` (27 failing tests in this bucket).

## Dependencies

- [`local-exec-05-arrow-marshaling.plan.md`](local-exec-05-arrow-marshaling.plan.md)
- [`local-exec-06-aggregate-modifiers.plan.md`](local-exec-06-aggregate-modifiers.plan.md)

## Root cause

- `query_port_test.go:1484: rows.Err: jobs.query -> 501: {"error":{"code":501,"message":"semantic: SELECT path expects a ProjectScan; got OrderByScan","errors":[{"reason":"notImplemented","mes...`
- `query_port_test.go:8045: jobs.query -> 501: {"error":{"code":501,"message":"semantic: ResolvedExpr kind SubqueryExpr is not yet implemented","errors":[{"reason":"notImplemented","message":"...`
- `query_port_test.go:8045: jobs.query -> 501: {"error":{"code":501,"message":"semantic: SELECT path expects a ProjectScan; got WithScan","errors":[{"reason":"notImplemented","message":"semant...`

## Primary files

- [`backend/engine/semantic/eval_expr.cc`](../../backend/engine/semantic/eval_expr.cc)
- [`backend/engine/semantic/eval_expr.h`](../../backend/engine/semantic/eval_expr.h)
- [`backend/engine/coordinator/local_coordinator_engine.cc`](../../backend/engine/coordinator/local_coordinator_engine.cc)

## Implementation steps

1. Allow semantic SELECT path to handle `OrderByScan`, `WithScan`, and nested scan trees without requiring top-level `ProjectScan`.
2. Implement `ResolvedSubqueryExpr` evaluation (scalar, EXISTS, IN, ARRAY forms).
3. Implement `MakeStruct`, `GetStructField`, and correlated `ResolvedArrayScan` where listed in failures.
4. Add semantic executor unit tests mirroring failing subtests.

## Verify

```bash
# Top-level tests (run separately — do not use TestQuery/foo$; it matches TestQuerySemantic*).
BIGQUERY_EMULATOR_BIN=./bin/emulator_main \
  go test -tags=integration ./gateway/e2e/ \
  -run '^(TestArrayTransformLambda|TestUnnestWithOffset)$' \
  -count=1 -parallel 1

# TestQuery subtests — anchor parent with TestQuery$/^subtest$
BIGQUERY_EMULATOR_BIN=./bin/emulator_main \
  go test -tags=integration ./gateway/e2e/ \
  -run 'TestQuery$/^(array_function|array_function_with_multiple_array|array_function_with_other_column|array_function_with_struct|array_scan_inner_join|array_scan_left_outer_join|bit_count|exists|instr|json_extract_array_with_integer_cast|json_extract_string_array_with_integer_cast|json_query_array_with_integer_cast|json_value_array_with_integer_cast|not_exists|null_array_scan|regexp_contains|regexp_contains2|soundex|struct_with_bool|subquery_expr_with_array_type|subquery_expr_with_exists_type|subquery_expr_with_scalar_type_at_SELECT|subquery_expr_with_scalar_type_at_function_call|to_json_with_struct|unnest_with_offset)$' \
  -count=1 -parallel 1
```

## Done when

- All 27 tests listed below pass.
- `./gateway/e2e/testresults/run_query_port_tests.sh` fail count drops by ~27.

## Failing tests

- `TestArrayTransformLambda`
- `TestQuery/array_function`
- `TestQuery/array_function_with_multiple_array`
- `TestQuery/array_function_with_other_column`
- `TestQuery/array_function_with_struct`
- `TestQuery/array_scan_inner_join`
- `TestQuery/array_scan_left_outer_join`
- `TestQuery/bit_count`
- `TestQuery/exists`
- `TestQuery/instr`
- `TestQuery/json_extract_array_with_integer_cast`
- `TestQuery/json_extract_string_array_with_integer_cast`
- `TestQuery/json_query_array_with_integer_cast`
- `TestQuery/json_value_array_with_integer_cast`
- `TestQuery/not_exists`
- `TestQuery/null_array_scan`
- `TestQuery/regexp_contains`
- `TestQuery/regexp_contains2`
- `TestQuery/soundex`
- `TestQuery/struct_with_bool`
- `TestQuery/subquery_expr_with_array_type`
- `TestQuery/subquery_expr_with_exists_type`
- `TestQuery/subquery_expr_with_scalar_type_at_SELECT`
- `TestQuery/subquery_expr_with_scalar_type_at_function_call`
- `TestQuery/to_json_with_struct`
- `TestQuery/unnest_with_offset`
- `TestUnnestWithOffset`
