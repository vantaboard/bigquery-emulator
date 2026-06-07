// BigQuery string polyfill macros.
//
// Each macro installed here pins a specific BigQuery / DuckDB
// string-function gap. The body documents the gap; the per-macro
// unit test (`string_macros_test.cc`) exercises both the common
// path AND the BigQuery-specific edge case so a future DuckDB
// regression surfaces as a unit-test failure rather than as silent
// semantic drift.

#include "absl/status/status.h"
#include "backend/engine/duckdb/udf/internal/run_macro.h"
#include "duckdb.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace duckdb {
namespace udf {

absl::Status RegisterString(::duckdb_connection conn) {
  // `bq_strpos(value, subvalue)` --- BigQuery STRPOS (1-based).
  //
  // BigQuery STRPOS(value, subvalue) returns the 1-based index of
  // the first occurrence of `subvalue` inside `value`, or 0 if
  // `subvalue` is not found. DuckDB's `strpos(value, subvalue)`
  // agrees today on the 1-based-index-or-zero contract and on the
  // NULL propagation contract, so the macro is a thin alias that
  // pins the BigQuery contract under our own name. Future DuckDB
  // upgrades that change `strpos`'s indexing convention would
  // surface as the unit test below failing rather than as a silent
  // off-by-one regression at the gateway.
  //
  // Edge cases the unit test pins:
  //   * 1-based index (`bq_strpos('hello', 'll') == 3`).
  //   * Empty needle is matched at position 1 (BQ contract).
  //   * Missing needle returns 0 (NOT -1, NOT NULL).
  //   * NULL propagation in either operand returns NULL.
  if (auto s = internal::RunMacroDdl(
          conn,
          "CREATE OR REPLACE MACRO bq_strpos(value, subvalue) AS "
          "strpos(value, subvalue)");
      !s.ok()) {
    return s;
  }

  // `bq_split(value [, delimiter])` --- BigQuery SPLIT with `,` as
  // the default delimiter.
  //
  // BigQuery SPLIT(VALUE [, DELIMITER]) returns ARRAY<STRING> of the
  // substrings of `VALUE` separated by `DELIMITER`; the default
  // delimiter when omitted is the comma `,`. DuckDB's
  // `string_split(string, separator)` returns LIST(VARCHAR) and
  // requires both arguments. The macro uses a DEFAULT value
  // (`delimiter := ','`) to support both `SPLIT(v)` and
  // `SPLIT(v, d)` callsites under a single registered name; DuckDB
  // allows the default to be omitted at the call site, so
  // `bq_split('a,b,c')` evaluates to `['a','b','c']` (single-arg
  // BQ contract).
  //
  // Edge cases the unit test pins:
  //   * Default delimiter is the comma (`bq_split('a,b,c') ==
  //     ['a','b','c']`).
  //   * Custom delimiter (`bq_split('a;b;c', ';') ==
  //     ['a','b','c']`).
  //   * Empty input with non-empty delimiter returns a list
  //     containing one empty string (`bq_split('', ',') == ['']`,
  //     matching BigQuery's "still one element, even if the input
  //     is empty" contract).
  //   * NULL propagation in either operand returns NULL. DuckDB's
  //     `string_split` propagates NULL in `value` but NOT in
  //     `separator` (a NULL separator returns the input wrapped
  //     as a single-element list, not NULL). BigQuery DOES
  //     propagate NULL in either argument, so the macro adds an
  //     explicit `CASE WHEN delimiter IS NULL THEN NULL` arm.
  //
  // Edge case the macro does NOT pin (documented in YAML notes):
  // `bq_split(value, '')` -- BigQuery splits into individual
  // characters; DuckDB's `string_split(value, '')` returns the
  // input wrapped in a single-element list. BYTES inputs are also
  // not supported by this macro (DuckDB's string_split is
  // VARCHAR-only); BigQuery SPLIT on BYTES needs a separate path
  // tracked in the YAML notes.
  if (auto s = internal::RunMacroDdl(
          conn,
          "CREATE OR REPLACE MACRO bq_split(value, delimiter := ',') AS "
          "CASE WHEN value IS NULL OR delimiter IS NULL THEN NULL "
          "ELSE string_split(value, delimiter) END");
      !s.ok()) {
    return s;
  }

  // `bq_lpad_bytes(val, len, pattern)` --- BigQuery LPAD on BYTES.
  //
  // DuckDB's `lpad` is VARCHAR-only. BigQuery pads on the left to
  // `len` bytes, repeating `pattern` cyclically (truncating the
  // final repetition). NULL in any argument returns NULL.
  if (auto s = internal::RunMacroDdl(
          conn,
          "CREATE OR REPLACE MACRO bq_lpad_bytes(val, len, pattern) AS "
          "CAST(CASE "
          "WHEN val IS NULL OR len IS NULL OR pattern IS NULL THEN NULL "
          "WHEN octet_length(val) >= len THEN CAST(array_slice(val, 1, len) AS "
          "BLOB) "
          "ELSE CAST(concat("
          "array_slice(repeat(pattern, CAST(ceil((len - "
          "octet_length(val))::DOUBLE "
          "/ greatest(octet_length(pattern), 1)) AS BIGINT)), 1, "
          "greatest(0, len - octet_length(val))), val) AS BLOB) "
          "END AS BLOB)");
      !s.ok()) {
    return s;
  }

  if (auto s = internal::RunMacroDdl(
          conn,
          "CREATE OR REPLACE MACRO bq_reverse_bytes(val) AS "
          "CASE WHEN val IS NULL THEN NULL "
          "ELSE list_reduce("
          "list_reverse(list_transform(range(1, octet_length(val) + 1), "
          "i -> array_slice(val, i, i))), "
          "CAST(NULL AS BLOB), "
          "(acc, b) -> CASE WHEN acc IS NULL THEN b ELSE acc || b END) "
          "END");
      !s.ok()) {
    return s;
  }

  if (auto s = internal::RunMacroDdl(
          conn,
          "CREATE OR REPLACE MACRO bq_rpad_bytes(val, len, pattern) AS "
          "CAST(CASE "
          "WHEN val IS NULL OR len IS NULL OR pattern IS NULL THEN NULL "
          "WHEN octet_length(val) >= len THEN CAST(array_slice(val, 1, len) AS "
          "BLOB) "
          "ELSE CAST(concat(val, "
          "array_slice(repeat(pattern, CAST(ceil((len - "
          "octet_length(val))::DOUBLE "
          "/ greatest(octet_length(pattern), 1)) AS BIGINT)), 1, "
          "greatest(0, len - octet_length(val)))) AS BLOB) "
          "END AS BLOB)");
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
