---
name: googlesqlite-08-semantic-operators
overview: Implement `$like`, `$between`, `$in`, bitwise, and IS DISTINCT FROM operators (~30 tests).
todos:
  - id: gsql-08-semantic-operators
    content: Semantic evaluator for $-prefixed comparison/bitwise operators
    status: pending
isProject: false
---

# googlesqlite 08: Semantic Internal Operators

## Goal

Implement `$like`, `$between`, `$in`, bitwise, and IS DISTINCT FROM operators (~30 tests).

Baseline: `20260603T035812Z` (30 failing tests in this bucket).

## Dependencies

- [`googlesqlite-07-semantic-core-expr.plan.md`](googlesqlite-07-semantic-core-expr.plan.md)

## Root cause

- `googlesqlite_query_test.go:8045: jobs.query -> 501: {"error":{"code":501,"message":"semantic: function '$between' is not yet implemented in the semantic executor","errors":[{"reason":"notImplemente...`
- `googlesqlite_query_test.go:8045: jobs.query -> 501: {"error":{"code":501,"message":"semantic: function '$bitwise_and' is not yet implemented in the semantic executor","errors":[{"reason":"notImplem...`
- `googlesqlite_query_test.go:8045: jobs.query -> 501: {"error":{"code":501,"message":"semantic: function '$bitwise_not' is not yet implemented in the semantic executor","errors":[{"reason":"notImplem...`

## Primary files

- [`backend/engine/semantic/eval_expr.cc`](../../backend/engine/semantic/eval_expr.cc)
- [`backend/engine/semantic/functions/`](../../backend/engine/semantic/functions/)
- [`backend/engine/duckdb/transpiler/functions.yaml`](../../backend/engine/duckdb/transpiler/functions.yaml)

## Implementation steps

1. Register internal operators in `functions.yaml` with `semantic_executor` disposition.
2. Implement BigQuery-exact NULL semantics for `$like`, `$between`, `$in`, `$is_true`, `$is_false`.
3. Implement bitwise and `$is_distinct_from` / `$is_not_distinct_from` with BigQuery truth tables.
4. Implement `round` with precision argument in semantic path.
5. Verify all matching `TestQuery/*operator*` subtests in this bucket.

## Verify

```bash
BIGQUERY_EMULATOR_BIN=./bin/emulator_main \
  go test -tags=integration ./gateway/e2e/ \
  -run 'TestQuery$/^(between_operator|bit_and_operator|bit_not_operator|bit_or_operator|bit_xor_operator|in_operator|is_distinct_from_with_1_and_1|is_distinct_from_with_1_and_2|is_distinct_from_with_1_and_null|is_distinct_from_with_null_and_null|is_false_operator|is_not_distinct_from_with_1_and_1|is_not_distinct_from_with_1_and_2|is_not_distinct_from_with_1_and_null|is_not_distinct_from_with_null_and_null|is_not_false_operator|is_not_true_operator|is_true_operator|justify_days|justify_hours|left_shift_operator|like_operator|like_operator2|like_operator3|like_operator4|like_operator_-_special_regex_characters|not_between_operator|not_in_operator|not_like_operator|right_shift_operator)$' \
  -count=1 -parallel 1
```

## Done when

- All 30 tests listed below pass.
- `./gateway/e2e/testresults/run_googlesqlite_emulator_tests.sh` fail count drops by ~30.

## Failing tests

- `TestQuery/between_operator`
- `TestQuery/bit_and_operator`
- `TestQuery/bit_not_operator`
- `TestQuery/bit_or_operator`
- `TestQuery/bit_xor_operator`
- `TestQuery/in_operator`
- `TestQuery/is_distinct_from_with_1_and_1`
- `TestQuery/is_distinct_from_with_1_and_2`
- `TestQuery/is_distinct_from_with_1_and_null`
- `TestQuery/is_distinct_from_with_null_and_null`
- `TestQuery/is_false_operator`
- `TestQuery/is_not_distinct_from_with_1_and_1`
- `TestQuery/is_not_distinct_from_with_1_and_2`
- `TestQuery/is_not_distinct_from_with_1_and_null`
- `TestQuery/is_not_distinct_from_with_null_and_null`
- `TestQuery/is_not_false_operator`
- `TestQuery/is_not_true_operator`
- `TestQuery/is_true_operator`
- `TestQuery/justify_days`
- `TestQuery/justify_hours`
- `TestQuery/left_shift_operator`
- `TestQuery/like_operator`
- `TestQuery/like_operator2`
- `TestQuery/like_operator3`
- `TestQuery/like_operator4`
- `TestQuery/like_operator_-_special_regex_characters`
- `TestQuery/not_between_operator`
- `TestQuery/not_in_operator`
- `TestQuery/not_like_operator`
- `TestQuery/right_shift_operator`
