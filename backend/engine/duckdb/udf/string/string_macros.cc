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

  return absl::OkStatus();
}

}  // namespace udf
}  // namespace duckdb
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
