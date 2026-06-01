// BigQuery conditional polyfill macros.
//
// This file installs DuckDB SQL macros that close the BigQuery /
// DuckDB semantic gap for the conditional functions whose
// `functions.yaml` row is `duckdb_udf`. Each macro's body documents
// the specific BigQuery edge case it pins; the per-macro unit test
// (`conditional_macros_test.cc`) drives the macro directly against
// an in-process DuckDB connection and exercises both the common path
// and the edge case.
//
// Foundation commit: this file ships empty. Subsequent commits
// (see `.cursor/plans/duckdb-polyfill-udf-library.plan.md` step 7)
// install one macro per BigQuery function and flip its
// `functions.yaml` row from `status=planned` to ready.

#include "absl/status/status.h"
#include "duckdb.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace duckdb {
namespace udf {

absl::Status RegisterConditional(::duckdb_connection /*conn*/) {
  // Intentionally empty: foundation commit only wires the registrar
  // skeleton. Each conditional macro lands in a follow-up commit
  // alongside its functions.yaml row flip + unit test + conformance
  // fixture.
  return absl::OkStatus();
}

}  // namespace udf
}  // namespace duckdb
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
