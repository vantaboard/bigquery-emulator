---
name: local-exec-12-arrays-generators
overview: Implement array generators, UNNEST paths, and array combinators (~20 tests).
todos:
  - id: gsql-12-arrays-generators
    content: Semantic array generators and UNNEST paths
    status: pending
isProject: false
---

# query port 12: Semantic Arrays / Generators

## Goal

Implement array generators, UNNEST paths, and array combinators (~20 tests).

Baseline: `20260603T035812Z` (20 failing tests in this bucket).

## Dependencies

- [`local-exec-11-json.plan.md`](local-exec-11-json.plan.md)

## Root cause

- `query_port_test.go:8071: [0]: (-want +got):`
- `query_port_test.go:8064: unexpected row [[<nil> 1 -2 3 -2 1 <nil>]]. expected row num 0 but got next row`
- `query_port_test.go:8064: unexpected row [2 [1 <nil>]]. expected row num 0 but got next row`

## Primary files

- [`backend/engine/semantic/functions/`](../../backend/engine/semantic/functions/)
- [`backend/engine/semantic/eval_expr.cc`](../../backend/engine/semantic/eval_expr.cc)
- [`backend/engine/coordinator/local_coordinator_engine.cc`](../../backend/engine/coordinator/local_coordinator_engine.cc)

## Implementation steps

1. Implement `generate_array`, `generate_date_array`, `generate_timestamp_array`.
2. Implement `array_concat`, `array_length`, `array_reverse`, and array scan join helpers.
3. Fix semantic UNNEST / `WITH OFFSET` paths (`TestUnnestWithOffset`, `TestArrayTransformLambda`).
4. Verify array literal concat and struct-array tests from failure list.

## Verify

Use `TestQuery$/^(subtest|…)$` — not `^(TestQuery/subtest$|…)`.

```bash
BIGQUERY_EMULATOR_BIN=./bin/emulator_main \
  go test -tags=integration ./gateway/e2e/ \
  -run 'TestQuery$/^(array_agg|array_agg_with_distinct|array_agg_with_nulls|array_agg_with_struct|array_agg_with_window|array_concat_function|array_length_function|array_reverse_function|array_to_string_function_with_null_text|concat_array_operator|generate_array_function|generate_array_function_for_generate_multiple_array|generate_array_function_with_large_step_value|generate_array_function_with_negative_step_value|generate_array_function_with_null|generate_array_function_with_over_step_value|generate_array_function_with_step|lpad_bytes_with_pattern|lpad_bytes_without_pattern|lpad_string_without_pattern)$' \
  -count=1 -parallel 1
```

## Done when

- All 20 tests listed below pass.
- `./gateway/e2e/testresults/run_query_port_tests.sh` fail count drops by ~20.

## Failing tests

- `TestQuery/array_agg`
- `TestQuery/array_agg_with_distinct`
- `TestQuery/array_agg_with_nulls`
- `TestQuery/array_agg_with_struct`
- `TestQuery/array_agg_with_window`
- `TestQuery/array_concat_function`
- `TestQuery/array_length_function`
- `TestQuery/array_reverse_function`
- `TestQuery/array_to_string_function_with_null_text`
- `TestQuery/concat_array_operator`
- `TestQuery/generate_array_function`
- `TestQuery/generate_array_function_for_generate_multiple_array`
- `TestQuery/generate_array_function_with_large_step_value`
- `TestQuery/generate_array_function_with_negative_step_value`
- `TestQuery/generate_array_function_with_null`
- `TestQuery/generate_array_function_with_over_step_value`
- `TestQuery/generate_array_function_with_step`
- `TestQuery/lpad_bytes_with_pattern`
- `TestQuery/lpad_bytes_without_pattern`
- `TestQuery/lpad_string_without_pattern`
