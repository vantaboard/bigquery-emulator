// BigQuery datetime polyfill macros.
//
// Each macro installed here pins a specific BigQuery / DuckDB
// datetime-function gap. The body documents the gap; the per-macro
// unit test (`datetime_macros_test.cc`) exercises both the common
// path AND the BigQuery-specific edge case so a future DuckDB
// regression surfaces as a unit-test failure rather than as silent
// semantic drift.
//
// Family scope (this commit ships the four Unix-epoch wrappers):
//
//   * `bq_unix_seconds(timestamp)` -- BIGINT seconds since
//     1970-01-01 UTC.
//   * `bq_unix_millis(timestamp)`  -- BIGINT milliseconds.
//   * `bq_unix_micros(timestamp)`  -- BIGINT microseconds.
//   * `bq_unix_date(date)`         -- BIGINT days since 1970-01-01.
//
// Out of scope (re-pointed to `semantic-functions-compliance.plan.md`
// in the wrap-up commit): `date_add` / `date_sub` / `date_diff` /
// `date_trunc` and the `datetime_*` / `timestamp_*` variants
// (BigQuery's month-end snap semantics for interval addition,
// calendar-week / ISO-year discrimination in DIFF, and the
// DATETIME-vs-TIMESTAMP type discrimination all need richer
// semantics than a thin macro can express). Same for
// `extract`, `format_*`, and `parse_*` (format-string dialect
// differences between BQ's `%E*S` extensions and DuckDB's
// strftime).

#include "absl/status/status.h"
#include "backend/engine/duckdb/udf/internal/run_macro.h"
#include "duckdb.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace duckdb {
namespace udf {

absl::Status RegisterDatetime(::duckdb_connection conn) {
  // `bq_unix_seconds(t)` --- BigQuery UNIX_SECONDS.
  //
  // BigQuery UNIX_SECONDS(timestamp) returns BIGINT seconds since
  // 1970-01-01 00:00:00 UTC, with fractional precision truncated
  // by "rounding down to the beginning of the second" (per
  // BigQuery's UNIX_SECONDS documentation -- floor toward negative
  // infinity, not toward zero). DuckDB's `epoch(timestamp)`
  // returns DOUBLE seconds (including the fractional part), and
  // DuckDB's bare CAST(DOUBLE AS BIGINT) ROUNDS (so .999 becomes
  // +1) rather than truncating. The wrapper applies an explicit
  // `FLOOR(epoch(...))` so the result matches BigQuery's
  // round-down contract for both positive and negative epoch
  // values.
  //
  // Edge cases the unit test pins:
  //   * Whole-second timestamp returns the expected BIGINT epoch.
  //   * Sub-second precision is truncated DOWN (BQ contract:
  //     `.999 -> +0 seconds`, not `+1`). A regression that
  //     dropped the FLOOR wrap would surface here.
  //   * NULL propagation.
  if (auto s =
          internal::RunMacroDdl(conn,
                                "CREATE OR REPLACE MACRO bq_unix_seconds(t) AS "
                                "CAST(FLOOR(epoch(t)) AS BIGINT)");
      !s.ok()) {
    return s;
  }

  // `bq_unix_millis(t)` --- BigQuery UNIX_MILLIS.
  //
  // BigQuery UNIX_MILLIS(timestamp) returns BIGINT milliseconds
  // since 1970-01-01 00:00:00 UTC. DuckDB's `epoch_ms(timestamp)`
  // already returns BIGINT with the expected truncation semantics,
  // so the macro is a thin alias. We pin the contract under our
  // own name so a future DuckDB upgrade that changed
  // `epoch_ms`'s precision would surface as a unit-test failure
  // rather than as silent semantic drift.
  if (auto s = internal::RunMacroDdl(
          conn, "CREATE OR REPLACE MACRO bq_unix_millis(t) AS epoch_ms(t)");
      !s.ok()) {
    return s;
  }

  // `bq_unix_micros(t)` --- BigQuery UNIX_MICROS.
  //
  // BigQuery UNIX_MICROS(timestamp) returns BIGINT microseconds
  // since 1970-01-01 00:00:00 UTC. DuckDB's `epoch_us(timestamp)`
  // returns BIGINT with the expected semantics; the macro is a
  // thin alias under our name.
  if (auto s = internal::RunMacroDdl(
          conn, "CREATE OR REPLACE MACRO bq_unix_micros(t) AS epoch_us(t)");
      !s.ok()) {
    return s;
  }

  // `bq_unix_date(d)` --- BigQuery UNIX_DATE.
  //
  // BigQuery UNIX_DATE(date) returns BIGINT days since 1970-01-01.
  // DuckDB has `epoch(date)` but it returns seconds (BIGINT), not
  // days, so dividing by 86400 would be ambiguous if the date
  // happens to be on a DST-shift day in a TIMESTAMP context. The
  // safest identity is `date_diff('day', '1970-01-01'::DATE,
  // value)` -- DuckDB's `date_diff` is calendar-day-boundary
  // counted, which matches BQ's UNIX_DATE definition exactly.
  //
  // Edge cases the unit test pins:
  //   * `bq_unix_date(DATE '1970-01-01') == 0` (epoch is day 0).
  //   * `bq_unix_date(DATE '1969-12-31') == -1` (pre-epoch is
  //     negative; BQ documents this explicitly).
  //   * NULL propagation.
  if (auto s =
          internal::RunMacroDdl(conn,
                                "CREATE OR REPLACE MACRO bq_unix_date(d) AS "
                                "date_diff('day', DATE '1970-01-01', d)");
      !s.ok()) {
    return s;
  }

  return absl::OkStatus();
}

}  // namespace udf
}  // namespace duckdb
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
