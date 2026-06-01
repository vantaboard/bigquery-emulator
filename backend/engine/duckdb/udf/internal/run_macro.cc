#include "backend/engine/duckdb/udf/internal/run_macro.h"

#include <string>

#include "absl/status/status.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/string_view.h"
#include "duckdb.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace duckdb {
namespace udf {
namespace internal {

absl::Status RunMacroDdl(::duckdb_connection conn, absl::string_view sql) {
  std::string nul_terminated(sql);
  ::duckdb_result result;
  const auto rc = ::duckdb_query(conn, nul_terminated.c_str(), &result);
  if (rc != ::DuckDBSuccess) {
    const char* err = ::duckdb_result_error(&result);
    std::string detail = err == nullptr ? std::string("") : std::string(err);
    ::duckdb_destroy_result(&result);
    return absl::InternalError(absl::StrCat("DuckDB polyfill UDF registrar: ",
                                            detail,
                                            " (sql=",
                                            nul_terminated,
                                            ")"));
  }
  ::duckdb_destroy_result(&result);
  return absl::OkStatus();
}

}  // namespace internal
}  // namespace udf
}  // namespace duckdb
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
