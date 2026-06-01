// BigQuery conditional polyfill macros.
//
// Each macro installed here pins a specific BigQuery / DuckDB
// conditional-form gap. The body documents the gap; the per-macro
// unit test (`conditional_macros_test.cc`) exercises both the
// common path AND the BigQuery-specific edge case so a future
// DuckDB regression surfaces as a unit-test failure rather than as
// silent semantic drift.

#include "absl/status/status.h"
#include "backend/engine/duckdb/udf/internal/run_macro.h"
#include "duckdb.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace duckdb {
namespace udf {

absl::Status RegisterConditional(::duckdb_connection conn) {
  // `bq_if(cond, true_result, else_result)` --- BigQuery IF.
  //
  // BigQuery IF(cond, A, B) returns A when cond is TRUE and B
  // otherwise; a NULL `cond` falls through to B (not A). This is
  // exactly DuckDB's `CASE WHEN cond THEN A ELSE B END`.
  //
  // DuckDB also has a bare `IF(cond, A, B)` function with the same
  // semantics, but routing to it would couple us to DuckDB's
  // signature (e.g. a future DuckDB upgrade adding a fourth
  // argument). The macro pins the BigQuery contract under a name
  // we own; the body stays a single CASE expression that DuckDB's
  // optimizer inlines.
  //
  // Edge case the unit test pins:
  //   * NULL cond falls through to the ELSE branch.
  if (auto s = internal::RunMacroDdl(
          conn,
          "CREATE OR REPLACE MACRO bq_if(cond, true_result, else_result) AS "
          "CASE WHEN cond THEN true_result ELSE else_result END");
      !s.ok()) {
    return s;
  }

  // `bq_isnull(x)` --- BigQuery ISNULL.
  //
  // BigQuery ISNULL(X) is the call-form of the `IS NULL` operator:
  // returns TRUE when X is NULL else FALSE. DuckDB has a bare
  // `ISNULL(x)` function with identical semantics today; the macro
  // pins the contract under a name we own.
  //
  // Edge cases the unit test pins:
  //   * `bq_isnull(NULL) == TRUE`.
  //   * `bq_isnull(0) == FALSE` (a literal zero is NOT null).
  //   * `bq_isnull('') == FALSE` (empty string is NOT null in BQ).
  if (auto s = internal::RunMacroDdl(
          conn,
          "CREATE OR REPLACE MACRO bq_isnull(x) AS (x IS NULL)");
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
