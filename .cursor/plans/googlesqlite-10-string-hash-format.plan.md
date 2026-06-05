---
name: googlesqlite-10-string-hash-format
overview: Implement string, regex, hash, encoding, and `format` functions (~94 tests).
todos:
  - id: gsql-10-string-hash-format
    content: Semantic string, regex, hash, and format functions
    status: pending
isProject: false
---

# googlesqlite 10: Semantic String / Hash / Format

## Goal

Implement string, regex, hash, encoding, and `format` functions (~94 tests).

Baseline: `20260603T035812Z` (83 failing tests in this bucket).

## Dependencies

- [`googlesqlite-09-date-time.plan.md`](googlesqlite-09-date-time.plan.md)

## Root cause

- `googlesqlite_query_test.go:8045: jobs.query -> 501: {"error":{"code":501,"message":"semantic: function 'ascii' is not yet implemented in the semantic executor","errors":[{"reason":"notImplemented",...`
- `googlesqlite_query_test.go:8045: jobs.query -> 501: {"error":{"code":501,"message":"duckdb engine: DuckDB rejected transpiled SQL: Parser Error: syntax error at or near \"AS\"\n\nLINE 1: ...' AS \"...`
- `googlesqlite_query_test.go:8045: jobs.query -> 501: {"error":{"code":501,"message":"semantic: function 'byte_length' is not yet implemented in the semantic executor","errors":[{"reason":"notImpleme...`

## Primary files

- [`backend/engine/semantic/functions/`](../../backend/engine/semantic/functions/)
- [`backend/engine/semantic/eval_expr.cc`](../../backend/engine/semantic/eval_expr.cc)

## Implementation steps

1. Implement core string funcs: `concat`, `substr`, `length`, `lower`/`upper`, `trim`, `split`, `replace`.
2. Implement regex family: `regexp_contains`, `regexp_extract`, `regexp_replace`, `regexp_instr`.
3. Implement hash/encoding: `md5`, `sha1`, `sha256`, `sha512`, `to_base64`, `from_hex`, etc.
4. Implement `format`, `least`/`greatest`, and remaining single-test string functions from failure list.

## Verify

Use `TestQuery$/^(subtest|…)$` for subtests (not `TestQuery/subtest$` — matches `TestQuerySemantic*`). Run top-level `TestSubqueryExpr` and `TestWindowVariousFrames` separately. Omit bare `TestQuery$` and out-of-scope datetime/window/param tests from the gate regex.

Session result (2026-06-03): **55/59 in-scope subtests pass**; 4 fail: `byte_length`, `octet_length`, `reverse` (CTE+bytes → DuckDB transpiler), `parse_bignumeric` (E37 overflow vs googlesql max).

```bash
BIGQUERY_EMULATOR_BIN=./bin/emulator_main \
  go test -tags=integration ./gateway/e2e/ \
  -run '^(TestQuery$|TestQuery/ascii$|TestQuery/byte_length$|TestQuery/byte_length_null$|TestQuery/cast_numeric_and_bignumeric_to_string$|TestQuery/char_length_null$|TestQuery/chr$|TestQuery/code_points_to_bytes$|TestQuery/code_points_to_string$|TestQuery/concat$|TestQuery/concat_string_operator$|TestQuery/datetime$|TestQuery/ends_with$|TestQuery/format_%'d$|TestQuery/format_%d$|TestQuery/format_%f_%E$|TestQuery/format_%s$|TestQuery/format_\+%010d\+$|TestQuery/format_time_with_%E\*S$|TestQuery/format_time_with_%E3S$|TestQuery/format_time_with_%R$|TestQuery/format_time_with_%k_%l$|TestQuery/format_\|%10d\|$|TestQuery/from_base32$|TestQuery/from_base64$|TestQuery/from_hex$|TestQuery/invalid_cast$|TestQuery/least_greatest_between_integer$|TestQuery/least_greatest_between_string$|TestQuery/least_greatest_date$|TestQuery/left_with_bytes_value$|TestQuery/left_with_string_value$|TestQuery/length$|TestQuery/lower$|TestQuery/ltrim$|TestQuery/md5$|TestQuery/multiple_statements_with_named_params$|TestQuery/normalize_null$|TestQuery/not_enough_named_params_given$|TestQuery/not_enough_positional_params_given$|TestQuery/octet_length$|TestQuery/octet_length_null$|TestQuery/parse_bignumeric$|TestQuery/parse_numeric$|TestQuery/regexp_contains_null_pattern$|TestQuery/regexp_extract_all_null$|TestQuery/regexp_extract_null_pattern$|TestQuery/regexp_replace_null$|TestQuery/regexp_replace_quoted$|TestQuery/replace_null$|TestQuery/reverse$|TestQuery/rounding$|TestQuery/rounding_precision$|TestQuery/safe_convert_bytes_to_string$|TestQuery/sha1$|TestQuery/sha256$|TestQuery/sha512$|TestQuery/single_statement_with_named_params$|TestQuery/single_statement_with_positional_params$|TestQuery/split_null_delimiter$|TestQuery/starts_with$|TestQuery/string$|TestQuery/strpos$|TestQuery/substr$|TestQuery/substring$|TestQuery/time$|TestQuery/time_from_datetime$|TestQuery/to_base32$|TestQuery/to_base64$|TestQuery/to_code_points_compare_string_and_bytes$|TestQuery/to_hex$|TestQuery/trim$|TestQuery/unicode$|TestQuery/upper$|TestQuery/use_function$|TestQuery/window_dense_rank_with_group$|TestQuery/window_lag$|TestQuery/window_lag_with_offset$|TestQuery/window_lag_with_offset_and_default_value$|TestQuery/window_order_by$|TestQuery/window_order_by_handles_nil$|TestSubqueryExpr$|TestWindowVariousFrames$)' \
  -count=1 -parallel 1
```

## Done when

- All 83 tests listed below pass.
- `./gateway/e2e/testresults/run_googlesqlite_emulator_tests.sh` fail count drops by ~83.

## Failing tests

- `TestQuery`
- `TestQuery/ascii`
- `TestQuery/byte_length`
- `TestQuery/byte_length_null`
- `TestQuery/cast_numeric_and_bignumeric_to_string`
- `TestQuery/char_length_null`
- `TestQuery/chr`
- `TestQuery/code_points_to_bytes`
- `TestQuery/code_points_to_string`
- `TestQuery/concat`
- `TestQuery/concat_string_operator`
- `TestQuery/datetime`
- `TestQuery/ends_with`
- `TestQuery/format_%'d`
- `TestQuery/format_%d`
- `TestQuery/format_%f_%E`
- `TestQuery/format_%s`
- `TestQuery/format_+%010d+`
- `TestQuery/format_time_with_%E*S`
- `TestQuery/format_time_with_%E3S`
- `TestQuery/format_time_with_%R`
- `TestQuery/format_time_with_%k_%l`
- `TestQuery/format_|%10d|`
- `TestQuery/from_base32`
- `TestQuery/from_base64`
- `TestQuery/from_hex`
- `TestQuery/invalid_cast`
- `TestQuery/least_greatest_between_integer`
- `TestQuery/least_greatest_between_string`
- `TestQuery/least_greatest_date`
- `TestQuery/left_with_bytes_value`
- `TestQuery/left_with_string_value`
- `TestQuery/length`
- `TestQuery/lower`
- `TestQuery/ltrim`
- `TestQuery/md5`
- `TestQuery/multiple_statements_with_named_params`
- `TestQuery/normalize_null`
- `TestQuery/not_enough_named_params_given`
- `TestQuery/not_enough_positional_params_given`
- `TestQuery/octet_length`
- `TestQuery/octet_length_null`
- `TestQuery/parse_bignumeric`
- `TestQuery/parse_numeric`
- `TestQuery/regexp_contains_null_pattern`
- `TestQuery/regexp_extract_all_null`
- `TestQuery/regexp_extract_null_pattern`
- `TestQuery/regexp_replace_null`
- `TestQuery/regexp_replace_quoted`
- `TestQuery/replace_null`
- `TestQuery/reverse`
- `TestQuery/rounding`
- `TestQuery/rounding_precision`
- `TestQuery/safe_convert_bytes_to_string`
- `TestQuery/sha1`
- `TestQuery/sha256`
- `TestQuery/sha512`
- `TestQuery/single_statement_with_named_params`
- `TestQuery/single_statement_with_positional_params`
- `TestQuery/split_null_delimiter`
- `TestQuery/starts_with`
- `TestQuery/string`
- `TestQuery/strpos`
- `TestQuery/substr`
- `TestQuery/substring`
- `TestQuery/time`
- `TestQuery/time_from_datetime`
- `TestQuery/to_base32`
- `TestQuery/to_base64`
- `TestQuery/to_code_points_compare_string_and_bytes`
- `TestQuery/to_hex`
- `TestQuery/trim`
- `TestQuery/unicode`
- `TestQuery/upper`
- `TestQuery/use_function`
- `TestQuery/window_dense_rank_with_group`
- `TestQuery/window_lag`
- `TestQuery/window_lag_with_offset`
- `TestQuery/window_lag_with_offset_and_default_value`
- `TestQuery/window_order_by`
- `TestQuery/window_order_by_handles_nil`
- `TestSubqueryExpr`
- `TestWindowVariousFrames`
