// BigQuery numeric polyfill macros.
//
// Each macro installed here pins a specific BigQuery / DuckDB
// numeric semantic gap. The body documents the gap; the per-macro
// unit test (`numeric_macros_test.cc`) exercises both the common
// path AND the BigQuery-specific edge case so a future regression
// in DuckDB's behavior surfaces as a unit-test failure rather than
// as a silent semantic drift.

#include "absl/status/status.h"
#include "backend/engine/duckdb/udf/internal/run_macro.h"
#include "duckdb.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace duckdb {
namespace udf {

absl::Status RegisterNumeric(::duckdb_connection conn) {
  // `bq_mod(x, y)` --- BigQuery MOD with sign-of-dividend semantics
  // plus BigQuery's "Y=0 raises" contract.
  //
  // BigQuery MOD(X, Y) returns the remainder of X / Y with the sign
  // of X (truncated-division convention) and "Returns a divide by
  // zero error if Y = 0." DuckDB's `%` operator agrees on the sign
  // convention but DIVERGES on Y=0: DuckDB returns NULL while
  // BigQuery raises. We close that gap with an explicit CASE arm
  // that calls DuckDB's `error()` function, which surfaces as an
  // `Invalid Input Error` the executor folds into BigQuery's
  // `divide-by-zero` shape. CASE only evaluates the matching arm so
  // the common path stays a single `%` operation.
  //
  // Edge cases the unit test pins:
  //   * Sign of result tracks dividend
  //     (`bq_mod(-7, 3) == -1`, `bq_mod(7, -3) == 1`).
  //   * NULL propagation in either operand returns NULL.
  //   * Y=0 raises (matches BQ's non-SAFE behavior); without the
  //     explicit error() arm DuckDB would return NULL and silently
  //     match `SAFE.MOD` semantics instead.
  if (auto s = internal::RunMacroDdl(
          conn,
          "CREATE OR REPLACE MACRO bq_mod(x, y) AS "
          "CASE WHEN y = 0 "
          "THEN error('MOD: divisor must be non-zero (BigQuery raises "
          "a divide-by-zero error when MOD''s second argument is 0)') "
          "ELSE x % y END");
      !s.ok()) {
    return s;
  }

  // `bq_div(x, y)` --- BigQuery DIV (truncated integer division)
  // plus BigQuery's "Y=0 raises" contract.
  //
  // BigQuery DIV(X, Y) returns the truncated integer quotient of
  // X / Y (sign of the dividend, like C's `/` on signed integers).
  // BigQuery raises a divide-by-zero error when Y = 0.
  //
  // DuckDB v1.5.3's `//` operator happens to perform TRUNCATED
  // integer division (matching BigQuery) for the cases the unit
  // test covers, but the operator's documented convention has
  // historically been FLOOR division. We use the identity
  //   truncated_div(x, y) == (x - x % y) / y
  // because `x - (x % y)` is exactly divisible by y (the residual
  // is the dividend-signed `%` we just took), so the trailing `//`
  // is an exact integer division -- floor and truncate agree on
  // values divisible by y. This makes the macro defensive against
  // a future DuckDB upgrade that re-establishes `//` as strict
  // floor division. Y=0 surfaces a `divide-by-zero` raise via the
  // same `error()` arm pattern `bq_mod` uses.
  //
  // Edge cases the unit test pins:
  //   * `bq_div(-5, 2) == -2` (truncate, not floor).
  //   * `bq_div(5, -2) == -2` (truncate, not floor).
  //   * `bq_div(-5, -2) == 2` (matches both truncate and floor).
  //   * NULL propagation in either operand returns NULL.
  //   * Y=0 raises.
  if (auto s = internal::RunMacroDdl(
          conn,
          "CREATE OR REPLACE MACRO bq_div(x, y) AS "
          "CASE WHEN y = 0 "
          "THEN error('DIV: divisor must be non-zero (BigQuery raises "
          "a divide-by-zero error when DIV''s second argument is 0)') "
          "ELSE (x - (x % y)) // y END");
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
