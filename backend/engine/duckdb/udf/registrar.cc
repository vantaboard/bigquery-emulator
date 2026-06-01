#include "backend/engine/duckdb/udf/registrar.h"

#include "absl/status/status.h"
#include "duckdb.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace duckdb {
namespace udf {

// Per-family registrar declarations. Each family lives in its own
// `<family>/<name>.cc` file and exports a single
// `absl::Status RegisterFamily(::duckdb_connection conn)` function.
// The registrar wires each family in order so a failure in family N
// aborts before family N+1 is attempted; the `absl::Status`'s payload
// identifies which family / which underlying SQL DDL failed.
//
// Adding a family: declare a `Register<Family>(conn)` function here
// and call it from `RegisterAll`. The function is responsible for
// installing every macro in its family; see
// `backend/engine/duckdb/udf/numeric/numeric_macros.cc` for the
// convention.
absl::Status RegisterNumeric(::duckdb_connection conn);
absl::Status RegisterConditional(::duckdb_connection conn);
absl::Status RegisterString(::duckdb_connection conn);

absl::Status RegisterAll(::duckdb_connection conn) {
  if (conn == nullptr) {
    return absl::InvalidArgumentError(
        "DuckDB polyfill UDF registrar: connection is null");
  }
  if (auto s = RegisterNumeric(conn); !s.ok()) return s;
  if (auto s = RegisterConditional(conn); !s.ok()) return s;
  if (auto s = RegisterString(conn); !s.ok()) return s;
  return absl::OkStatus();
}

}  // namespace udf
}  // namespace duckdb
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
