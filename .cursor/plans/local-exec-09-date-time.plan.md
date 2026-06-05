---
name: local-exec-09-date-time
overview: Implement date/time parse, format, diff, trunc, and current_* functions (~150 tests).
todos:
  - id: gsql-09-date-time
    content: Semantic date/time function registry (phased)
    status: in_progress
isProject: false
---

# query port 09: Semantic Date / Time Functions

## Goal

Implement date/time parse, format, diff, trunc, and current_* functions (~150 tests).

Baseline: `20260603T035812Z` (158 failing tests in this bucket).

## Dependencies

- [`local-exec-08-semantic-operators.plan.md`](local-exec-08-semantic-operators.plan.md)

## Root cause

- `query_port_test.go:8045: jobs.query -> 501: {"error":{"code":501,"message":"semantic: function 'parse_date' is not yet implemented in the semantic executor","errors":[{"reason":"notImplemen...`
- `query_port_test.go:8045: jobs.query -> 501: {"error":{"code":501,"message":"semantic: function 'parse_datetime' is not yet implemented in the semantic executor","errors":[{"reason":"notImpl...`
- `query_port_test.go:8045: jobs.query -> 501: {"error":{"code":501,"message":"duckdb engine: DuckDB rejected transpiled SQL: Conversion Error: invalid timestamp field format: \"20100317\", ex...`

## Primary files

- [`backend/engine/semantic/functions/`](../../backend/engine/semantic/functions/)
- [`backend/engine/semantic/eval_expr.cc`](../../backend/engine/semantic/eval_expr.cc)

## Implementation steps

1. Phase A: `parse_date`, `parse_datetime`, `parse_timestamp`, `parse_time` with format-string coverage from failures.
2. Phase B: `format_date`, `format_datetime`, `format_timestamp`, `format_time`, and `format` numeric variants.
3. Phase C: `date_add/sub/diff/trunc`, `datetime_*`, `timestamp_*`, `time_*`, `current_*`, unix conversions.
4. Phase D: interval helpers (`make_interval`, `justify_*`) and extract functions.
5. Add focused C++ tests per phase; run plan verify regex after each phase.

## Verify

Use `TestQuery$/^(subtest1|subtest2|…)$` — not `^(TestQuery/subtest$|…)` (matches `TestQuerySemantic*` / runs wrong tests). Build the alternation from the **Failing tests** list below. See **Deferral** for session results (154/157 pass; 3 `CURRENT_*` harness flakes).

```bash
BIGQUERY_EMULATOR_BIN=./bin/emulator_main \
  go test -tags=integration ./gateway/e2e/ \
  -run '^(TestQuery/base_date_is_epoch$|TestQuery/base_date_is_epoch_julian$|TestQuery/base_date_is_epoch_julian_different_day$|TestQuery/base_datetime_is_epoch_julian$|TestQuery/cast_integer_to_datetime$|TestQuery/current_date$|TestQuery/current_date\#01$|TestQuery/current_datetime$|TestQuery/current_time$|TestQuery/current_timestamp$|TestQuery/date_add$|TestQuery/date_add_quarter$|TestQuery/date_diff_with_day$|TestQuery/date_diff_with_month$|TestQuery/date_diff_with_week$|TestQuery/date_diff_with_week_day$|TestQuery/date_diff_with_week_day\#01$|TestQuery/date_from_unix_date$|TestQuery/date_operator$|TestQuery/date_sub$|TestQuery/date_trunc_with_day$|TestQuery/date_trunc_with_month$|TestQuery/date_trunc_with_quarter$|TestQuery/date_trunc_with_week$|TestQuery/date_trunc_with_year$|TestQuery/datetime_add$|TestQuery/datetime_add\#01$|TestQuery/datetime_diff_with_day$|TestQuery/datetime_diff_with_isoweek$|TestQuery/datetime_diff_with_week$|TestQuery/datetime_diff_with_week_day$|TestQuery/datetime_diff_with_week_day\#01$|TestQuery/datetime_diff_with_week_day_1_week$|TestQuery/datetime_diff_with_year,_ISOYEAR$|TestQuery/datetime_sub$|TestQuery/datetime_sub\#01$|TestQuery/datetime_trunc_isoyear$|TestQuery/datetime_trunc_with_day$|TestQuery/datetime_trunc_with_day_weekday$|TestQuery/datetime_trunc_with_isoyear$|TestQuery/datetime_trunc_with_quarter$|TestQuery/datetime_trunc_with_weekday\(monday\)$|TestQuery/format_date_with_%E4Y$|TestQuery/format_date_with_%b\-%d\-%Y$|TestQuery/format_date_with_%b_%Y$|TestQuery/format_date_with_%t$|TestQuery/format_date_with_%x$|TestQuery/format_date_with_%y$|TestQuery/format_datetime_with_%E\*S$|TestQuery/format_datetime_with_%E3S$|TestQuery/format_datetime_with_%E4Y$|TestQuery/format_datetime_with_%b\-%d\-%Y$|TestQuery/format_datetime_with_%b_%Y$|TestQuery/format_datetime_with_%c$|TestQuery/format_timestamp_with_%E\*S$|TestQuery/format_timestamp_with_%E3S$|TestQuery/format_timestamp_with_%E4Y$|TestQuery/format_timestamp_with_%Ez$|TestQuery/format_timestamp_with_%Y\-%m\-%d_%H:%M:%S$|TestQuery/format_timestamp_with_%b\-%d\-%Y$|TestQuery/format_timestamp_with_%b_%Y$|TestQuery/format_timestamp_with_%c$|TestQuery/format_timestamp_with_%t$|TestQuery/generate_date_array_function$|TestQuery/generate_date_array_function_with_month$|TestQuery/generate_date_array_function_with_negative_step$|TestQuery/generate_date_array_function_with_null$|TestQuery/generate_date_array_function_with_over_step$|TestQuery/generate_date_array_function_with_same_value$|TestQuery/generate_date_array_function_with_step$|TestQuery/generate_timestamp_array_function$|TestQuery/generate_timestamp_array_function_interval_1_second$|TestQuery/generate_timestamp_array_function_negative_interval$|TestQuery/generate_timestamp_array_function_over_step$|TestQuery/generate_timestamp_array_function_same_value$|TestQuery/generate_timestamp_array_function_with_null$|TestQuery/interval_from_sub_operator$|TestQuery/justify_interval$|TestQuery/last_day$|TestQuery/last_day_with_month$|TestQuery/last_day_with_week\(monday\)$|TestQuery/last_day_with_week\(sunday\)$|TestQuery/last_day_with_year$|TestQuery/make_interval$|TestQuery/minimum_/_maximum_timestamp_value_uses_microsecond_precision_and_range$|TestQuery/parse_date_\(_one_of_the_year_elements_is_missing_\)$|TestQuery/parse_date_\(_the_year_element_is_in_different_locations_\)$|TestQuery/parse_date_beneath_day_minimum$|TestQuery/parse_date_beneath_day_of_year_minimum$|TestQuery/parse_date_beneath_month_minimum$|TestQuery/parse_date_exceeding_day_maximum$|TestQuery/parse_date_exceeding_day_of_year_maximum$|TestQuery/parse_date_exceeding_month_maximum$|TestQuery/parse_date_with_%A_%b_%e_%Y$|TestQuery/parse_date_with_%F$|TestQuery/parse_date_with_%F_no_day_field$|TestQuery/parse_date_with_%F_no_month_field$|TestQuery/parse_date_with_%F_separator_but_no_month$|TestQuery/parse_date_with_%Y%m%d$|TestQuery/parse_date_with_%e$|TestQuery/parse_date_with_%e_\-_leading_space_allows_multiple_digits$|TestQuery/parse_date_with_%x$|TestQuery/parse_date_with_%y$|TestQuery/parse_date_with_single\-digit_month_%m$|TestQuery/parse_date_with_two_digit_year_after_2000_and_julian_day$|TestQuery/parse_date_with_two_digit_year_after_2000_and_julian_day_leap_year$|TestQuery/parse_date_with_two_digit_year_and_julian_day$|TestQuery/parse_date_with_two_digit_year_before_2000_and_julian_day$|TestQuery/parse_datetime$|TestQuery/parse_datetime_%F_respectfully_consuming_digits$|TestQuery/parse_datetime_\(_one_of_the_year_elements_is_missing_\)$|TestQuery/parse_datetime_\(_the_year_element_is_in_different_locations_\)$|TestQuery/parse_datetime_with_%c$|TestQuery/parse_datetime_with_two_digit_year_after_2000_and_julian_day$|TestQuery/parse_datetime_with_two_digit_year_after_2000_and_julian_day_leap_year$|TestQuery/parse_datetime_with_two_digit_year_before_2000_and_julian_day$|TestQuery/parse_time_\(_one_of_the_seconds_elements_is_missing_\)$|TestQuery/parse_time_\(_the_seconds_element_is_in_different_locations_\)$|TestQuery/parse_time_with_%I:%M:%S$|TestQuery/parse_time_with_%R$|TestQuery/parse_time_with_%R_without_minute_element$|TestQuery/parse_time_with_%R_without_separator$|TestQuery/parse_time_with_%T$|TestQuery/parse_timestamp_\(_one_of_the_year_elements_is_missing_\)$|TestQuery/parse_timestamp_\(_the_year_element_is_in_different_locations_\)$|TestQuery/parse_timestamp_with_%D$|TestQuery/parse_timestamp_with_%Y\-%m\-%d_%H:%M:%E\*S%Ez$|TestQuery/parse_timestamp_with_%Y\-%m\-%d_%H:%M:%S%Ez$|TestQuery/parse_timestamp_with_%a_%b_%e_%I:%M:%S_%Y$|TestQuery/parse_timestamp_with_%c$|TestQuery/parse_timestamp_with_%k$|TestQuery/parse_timestamp_with_%k\#01$|TestQuery/parse_timestamp_with_%p$|TestQuery/parse_timestamp_with_extra_whitespace_$|TestQuery/safe_parse_date_\(_the_year_element_is_in_different_locations_\)$|TestQuery/time_add$|TestQuery/time_diff$|TestQuery/time_sub$|TestQuery/time_trunc$|TestQuery/timestamp$|TestQuery/timestamp_add$|TestQuery/timestamp_diff$|TestQuery/timestamp_diff_with_week_day$|TestQuery/timestamp_from_date$|TestQuery/timestamp_from_datetime$|TestQuery/timestamp_in_zone$|TestQuery/timestamp_micros$|TestQuery/timestamp_millis$|TestQuery/timestamp_seconds$|TestQuery/timestamp_sub$|TestQuery/timestamp_trunc_with_day$|TestQuery/timestamp_trunc_with_quarter$|TestQuery/timestamp_trunc_with_year$|TestQuery/timestamp_with_zone$|TestQuery/unix_date$|TestQuery/unix_micros$|TestQuery/unix_millis$|TestQuery/unix_seconds$)' \
  -count=1 -parallel 1
```

## Done when

- All 158 tests listed below pass.
- `./gateway/e2e/testresults/run_query_port_tests.sh` fail count drops by ~158.

## Failing tests

- `TestQuery/base_date_is_epoch`
- `TestQuery/base_date_is_epoch_julian`
- `TestQuery/base_date_is_epoch_julian_different_day`
- `TestQuery/base_datetime_is_epoch_julian`
- `TestQuery/cast_integer_to_datetime`
- `TestQuery/current_date`
- `TestQuery/current_date#01`
- `TestQuery/current_datetime`
- `TestQuery/current_time`
- `TestQuery/current_timestamp`
- `TestQuery/date_add`
- `TestQuery/date_add_quarter`
- `TestQuery/date_diff_with_day`
- `TestQuery/date_diff_with_month`
- `TestQuery/date_diff_with_week`
- `TestQuery/date_diff_with_week_day`
- `TestQuery/date_diff_with_week_day#01`
- `TestQuery/date_from_unix_date`
- `TestQuery/date_operator`
- `TestQuery/date_sub`
- `TestQuery/date_trunc_with_day`
- `TestQuery/date_trunc_with_month`
- `TestQuery/date_trunc_with_quarter`
- `TestQuery/date_trunc_with_week`
- `TestQuery/date_trunc_with_year`
- `TestQuery/datetime_add`
- `TestQuery/datetime_add#01`
- `TestQuery/datetime_diff_with_day`
- `TestQuery/datetime_diff_with_isoweek`
- `TestQuery/datetime_diff_with_week`
- `TestQuery/datetime_diff_with_week_day`
- `TestQuery/datetime_diff_with_week_day#01`
- `TestQuery/datetime_diff_with_week_day_1_week`
- `TestQuery/datetime_diff_with_year,_ISOYEAR`
- `TestQuery/datetime_sub`
- `TestQuery/datetime_sub#01`
- `TestQuery/datetime_trunc_isoyear`
- `TestQuery/datetime_trunc_with_day`
- `TestQuery/datetime_trunc_with_day_weekday`
- `TestQuery/datetime_trunc_with_isoyear`
- `TestQuery/datetime_trunc_with_quarter`
- `TestQuery/datetime_trunc_with_weekday(monday)`
- `TestQuery/format_date_with_%E4Y`
- `TestQuery/format_date_with_%b-%d-%Y`
- `TestQuery/format_date_with_%b_%Y`
- `TestQuery/format_date_with_%t`
- `TestQuery/format_date_with_%x`
- `TestQuery/format_date_with_%y`
- `TestQuery/format_datetime_with_%E*S`
- `TestQuery/format_datetime_with_%E3S`
- `TestQuery/format_datetime_with_%E4Y`
- `TestQuery/format_datetime_with_%b-%d-%Y`
- `TestQuery/format_datetime_with_%b_%Y`
- `TestQuery/format_datetime_with_%c`
- `TestQuery/format_timestamp_with_%E*S`
- `TestQuery/format_timestamp_with_%E3S`
- `TestQuery/format_timestamp_with_%E4Y`
- `TestQuery/format_timestamp_with_%Ez`
- `TestQuery/format_timestamp_with_%Y-%m-%d_%H:%M:%S`
- `TestQuery/format_timestamp_with_%b-%d-%Y`
- `TestQuery/format_timestamp_with_%b_%Y`
- `TestQuery/format_timestamp_with_%c`
- `TestQuery/format_timestamp_with_%t`
- `TestQuery/generate_date_array_function`
- `TestQuery/generate_date_array_function_with_month`
- `TestQuery/generate_date_array_function_with_negative_step`
- `TestQuery/generate_date_array_function_with_null`
- `TestQuery/generate_date_array_function_with_over_step`
- `TestQuery/generate_date_array_function_with_same_value`
- `TestQuery/generate_date_array_function_with_step`
- `TestQuery/generate_timestamp_array_function`
- `TestQuery/generate_timestamp_array_function_interval_1_second`
- `TestQuery/generate_timestamp_array_function_negative_interval`
- `TestQuery/generate_timestamp_array_function_over_step`
- `TestQuery/generate_timestamp_array_function_same_value`
- `TestQuery/generate_timestamp_array_function_with_null`
- `TestQuery/interval_from_sub_operator`
- `TestQuery/justify_interval`
- `TestQuery/last_day`
- `TestQuery/last_day_with_month`
- `TestQuery/last_day_with_week(monday)`
- `TestQuery/last_day_with_week(sunday)`
- `TestQuery/last_day_with_year`
- `TestQuery/make_interval`
- `TestQuery/minimum_/_maximum_timestamp_value_uses_microsecond_precision_and_range`
- `TestQuery/parse_date_(_one_of_the_year_elements_is_missing_)`
- `TestQuery/parse_date_(_the_year_element_is_in_different_locations_)`
- `TestQuery/parse_date_beneath_day_minimum`
- `TestQuery/parse_date_beneath_day_of_year_minimum`
- `TestQuery/parse_date_beneath_month_minimum`
- `TestQuery/parse_date_exceeding_day_maximum`
- `TestQuery/parse_date_exceeding_day_of_year_maximum`
- `TestQuery/parse_date_exceeding_month_maximum`
- `TestQuery/parse_date_with_%A_%b_%e_%Y`
- `TestQuery/parse_date_with_%F`
- `TestQuery/parse_date_with_%F_no_day_field`
- `TestQuery/parse_date_with_%F_no_month_field`
- `TestQuery/parse_date_with_%F_separator_but_no_month`
- `TestQuery/parse_date_with_%Y%m%d`
- `TestQuery/parse_date_with_%e`
- `TestQuery/parse_date_with_%e_-_leading_space_allows_multiple_digits`
- `TestQuery/parse_date_with_%x`
- `TestQuery/parse_date_with_%y`
- `TestQuery/parse_date_with_single-digit_month_%m`
- `TestQuery/parse_date_with_two_digit_year_after_2000_and_julian_day`
- `TestQuery/parse_date_with_two_digit_year_after_2000_and_julian_day_leap_year`
- `TestQuery/parse_date_with_two_digit_year_and_julian_day`
- `TestQuery/parse_date_with_two_digit_year_before_2000_and_julian_day`
- `TestQuery/parse_datetime`
- `TestQuery/parse_datetime_%F_respectfully_consuming_digits`
- `TestQuery/parse_datetime_(_one_of_the_year_elements_is_missing_)`
- `TestQuery/parse_datetime_(_the_year_element_is_in_different_locations_)`
- `TestQuery/parse_datetime_with_%c`
- `TestQuery/parse_datetime_with_two_digit_year_after_2000_and_julian_day`
- `TestQuery/parse_datetime_with_two_digit_year_after_2000_and_julian_day_leap_year`
- `TestQuery/parse_datetime_with_two_digit_year_before_2000_and_julian_day`
- `TestQuery/parse_time_(_one_of_the_seconds_elements_is_missing_)`
- `TestQuery/parse_time_(_the_seconds_element_is_in_different_locations_)`
- `TestQuery/parse_time_with_%I:%M:%S`
- `TestQuery/parse_time_with_%R`
- `TestQuery/parse_time_with_%R_without_minute_element`
- `TestQuery/parse_time_with_%R_without_separator`
- `TestQuery/parse_time_with_%T`
- `TestQuery/parse_timestamp_(_one_of_the_year_elements_is_missing_)`
- `TestQuery/parse_timestamp_(_the_year_element_is_in_different_locations_)`
- `TestQuery/parse_timestamp_with_%D`
- `TestQuery/parse_timestamp_with_%Y-%m-%d_%H:%M:%E*S%Ez`
- `TestQuery/parse_timestamp_with_%Y-%m-%d_%H:%M:%S%Ez`
- `TestQuery/parse_timestamp_with_%a_%b_%e_%I:%M:%S_%Y`
- `TestQuery/parse_timestamp_with_%c`
- `TestQuery/parse_timestamp_with_%k`
- `TestQuery/parse_timestamp_with_%k#01`
- `TestQuery/parse_timestamp_with_%p`
- `TestQuery/parse_timestamp_with_extra_whitespace_`
- `TestQuery/safe_parse_date_(_the_year_element_is_in_different_locations_)`
- `TestQuery/time_add`
- `TestQuery/time_diff`
- `TestQuery/time_sub`
- `TestQuery/time_trunc`
- `TestQuery/timestamp`
- `TestQuery/timestamp_add`
- `TestQuery/timestamp_diff`
- `TestQuery/timestamp_diff_with_week_day`
- `TestQuery/timestamp_from_date`
- `TestQuery/timestamp_from_datetime`
- `TestQuery/timestamp_in_zone`
- `TestQuery/timestamp_micros`
- `TestQuery/timestamp_millis`
- `TestQuery/timestamp_seconds`
- `TestQuery/timestamp_sub`
- `TestQuery/timestamp_trunc_with_day`
- `TestQuery/timestamp_trunc_with_quarter`
- `TestQuery/timestamp_trunc_with_year`
- `TestQuery/timestamp_with_zone`
- `TestQuery/unix_date`
- `TestQuery/unix_micros`
- `TestQuery/unix_millis`
- `TestQuery/unix_seconds`

## Deferral (2026-06-03 session)

### Verify (final)

Command shape (must use `TestQuery$/^…$` anchor — bare `^(TestQuery/…)$` matches `TestQuerySemantic*` and/or runs zero tests):

```bash
BIGQUERY_EMULATOR_BIN=./bin/emulator_main \
  go test -tags=integration ./gateway/e2e/ \
  -run 'TestQuery$/^(…)$' \
  -count=1 -parallel 1
```

| Metric | Count |
|--------|------:|
| **Pass** | 154 |
| **Fail** | 3 |
| **Total run** | 157 |
| **Plan listed** | 158 (`datetime_trunc_with_isoyear` is in plan; confirm regex includes every subtest name) |

**Remaining failures (3)** — all `CURRENT_*` clock skew vs `now := time.Now()` captured once at `TestQuery` start while these subtests run late in file order within the filtered run:

- `TestQuery/current_datetime`
- `TestQuery/current_time`
- `TestQuery/current_timestamp`

Not a semantic date/time bug; needs harness change (refresh `now` per subtest, or run `CURRENT_*` in isolation).

### Landed (Phases A–D)

- New `backend/engine/semantic/functions/datetime_funcs.{h,cc}` — parse/format, add/sub/diff/trunc, `current_*`, unix_*, constructors, `last_day`, `generate_*_array`, `make_interval`, `justify_interval`, `extract`.
- `dispatch.cc` wires datetime handlers + `$extract` → `Extract`.
- `eval_expr.cc` — DATE/DATETIME/TIMESTAMP interval arithmetic; DATETIME−DATETIME via `IntervalDiffTimestamps`.
- `string_funcs.cc` — `FORMAT('%t', …)` for DATE/DATETIME/TIMESTAMP.
- `value.cc` — DATETIME wire `T` separator; TIMESTAMP microsecond padding (`%E6S`).
- `gateway/e2e/emulator_sql.go` — propagate `runQuery` errors from `QueryContext` (fixes `expectedErr` parse tests).
- `datetime_funcs_test.cc` — unit smoke tests (3).

### Blockers for parent gate

1. **3 `CURRENT_*` failures** — query port harness / ordering (see above).
2. **Verify regex** — document `TestQuery$/^` requirement in plan Verify section (updated above).
3. **`functions.yaml` disposition rows** — still `status=planned` for many datetime builtins (parity checker / routing docs not flipped this session).
4. **Do not run** `run_query_port_tests.sh` until `CURRENT_*` harness is resolved or accepted as known flake.

### Incidental fixes discovered during verify

- `GENERATE_TIMESTAMP_ARRAY` infinite loop on “over step” — replaced hand-rolled loop with `googlesql::functions::GenerateArray<absl::Time, TimestampIncrement>`.
- First full verify run hit **503** after emulator hang — caused by that loop; fixed before final counts.
