#ifndef BIGQUERY_EMULATOR_BACKEND_ENGINE_DUCKDB_UDF_REGISTRAR_H_
#define BIGQUERY_EMULATOR_BACKEND_ENGINE_DUCKDB_UDF_REGISTRAR_H_

// BigQuery polyfill UDF library registrar.
//
// The DuckDB fast path lowers BigQuery scalar / aggregate calls into
// DuckDB SQL. Most BigQuery functions have an exact DuckDB analog
// (`ABS`, `LENGTH`, `SUBSTRING`, ...) and route as
// `kDuckdbNative` / `kDuckdbRewrite`. The remaining functions have
// DuckDB analogs that are *close* but disagree on one or more BigQuery
// edge cases (month-end date arithmetic, MOD's sign on negative
// operands, BigQuery-specific REGEXP_* flag semantics, ...). For those
// rows the YAML registry (`functions.yaml`) carries
// `disposition=duckdb_udf` and the transpiler emits a call to a
// DuckDB UDF/macro that we own. The UDF body closes the BigQuery
// semantic gap; the transpiler stays agnostic of the gap and just
// emits `<udf_name>(<args>)`.
//
// `RegisterAll(conn)` is the single entry point that registers every
// UDF / macro in the library against a freshly-opened DuckDB
// connection. `DuckDbExecutor` calls it once per opened connection
// (each query opens its own in-memory DuckDB; the registration is
// cheap, ~one SQL execution per macro, well under a millisecond).
//
// Convention for individual UDFs:
//
//   * Each UDF lives in its own `.cc` file under
//     `backend/engine/duckdb/udf/<family>/<name>.cc`, where
//     `<family>` is one of `numeric`, `string`, `conditional`,
//     `datetime`, `regex`, `json`, `safe`.
//   * Each file exports a single
//     `absl::Status Register<Name>(::duckdb_connection conn)`
//     function (e.g. `RegisterBqMod`).
//   * `registrar.cc`'s `RegisterAll` calls each per-UDF
//     `Register*` in turn. Returning a non-OK status from any
//     `Register*` aborts the whole registration; `DuckDbExecutor`
//     propagates the status up the executor stack so a malformed UDF
//     fails the per-query connection setup loudly rather than
//     silently leaving the macro absent.
//   * The matching `functions.yaml` row stores the registered DuckDB
//     name in its `duckdb_name=<NAME>` field (e.g.
//     `mod: duckdb_udf duckdb_name=bq_mod`); the awk generator and
//     the transpiler treat `duckdb_udf` rows identically to
//     `duckdb_native` rows from the emit standpoint -- the
//     transpiler emits `<NAME>(<args>)` and DuckDB resolves it to
//     the registered UDF.
//   * Per-UDF unit tests live next to the UDF source under
//     `backend/engine/duckdb/udf/<family>/<name>_test.cc` and drive
//     the UDF directly against an in-process DuckDB connection.
//
// Failure modes:
//
//   * A `Register*` function returning non-OK means the UDF body is
//     malformed (DuckDB rejected the CREATE MACRO / function
//     registration). The executor fails fast at connection setup;
//     there is no runtime "missing UDF fall back to another route"
//     path (per the polyfill plan's Done Criterion 2: registration
//     failure fails fast).
//   * Calling `RegisterAll` on a null connection returns
//     `INVALID_ARGUMENT`.

#include "absl/status/status.h"
#include "duckdb.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace duckdb {
namespace udf {

// Registers every BigQuery polyfill UDF / macro against `conn`.
//
// `conn` must be a fresh, healthy `duckdb_connection`. Returns OK on
// success; returns a non-OK status when any individual UDF
// registration fails (the status payload identifies which
// registration call returned the error so a future regression is
// localizable). Safe to call once per connection; idempotency is
// achieved by using `CREATE OR REPLACE` for SQL-defined macros and by
// guarding C-API scalar-function registrations against re-entry.
absl::Status RegisterAll(::duckdb_connection conn);

}  // namespace udf
}  // namespace duckdb
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator

#endif  // BIGQUERY_EMULATOR_BACKEND_ENGINE_DUCKDB_UDF_REGISTRAR_H_
