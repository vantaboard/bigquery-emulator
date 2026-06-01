#ifndef BIGQUERY_EMULATOR_BACKEND_ENGINE_DUCKDB_UDF_INTERNAL_RUN_MACRO_H_
#define BIGQUERY_EMULATOR_BACKEND_ENGINE_DUCKDB_UDF_INTERNAL_RUN_MACRO_H_

// Internal helper shared by every per-family UDF registrar.
//
// Per-family `.cc` files install their macros by calling
// `RunMacroDdl(conn, "CREATE OR REPLACE MACRO ...")` rather than
// driving the DuckDB C-API directly; this keeps each family file
// free of `duckdb_result` lifecycle boilerplate and means a DuckDB
// rejection propagates through one shared formatter
// (`absl::InternalError` carrying the offending SQL).
//
// This header is `internal/` because external callers (the executor,
// the unit tests) talk to the public `RegisterAll(conn)` entry point
// in `registrar.h`. The per-family files include this header
// directly.

#include <string>

#include "absl/status/status.h"
#include "absl/strings/string_view.h"
#include "duckdb.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace duckdb {
namespace udf {
namespace internal {

// Runs `sql` as a no-result-row DDL against `conn` and folds a DuckDB
// rejection into a non-OK `absl::Status`. The status message includes
// the offending SQL for diagnosability; the caller (the per-family
// registrar) wraps it with family-specific context if needed.
absl::Status RunMacroDdl(::duckdb_connection conn, absl::string_view sql);

}  // namespace internal
}  // namespace udf
}  // namespace duckdb
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator

#endif  // BIGQUERY_EMULATOR_BACKEND_ENGINE_DUCKDB_UDF_INTERNAL_RUN_MACRO_H_
