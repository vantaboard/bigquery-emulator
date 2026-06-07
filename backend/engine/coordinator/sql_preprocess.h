#ifndef BIGQUERY_EMULATOR_BACKEND_ENGINE_COORDINATOR_SQL_PREPROCESS_H_
#define BIGQUERY_EMULATOR_BACKEND_ENGINE_COORDINATOR_SQL_PREPROCESS_H_

#include <string>

#include "absl/strings/string_view.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace coordinator {

// Applies BigQuery-parity SQL rewrites before the analyzer sees the
// text. Today: anonymous-struct positional access spelled as
// `expr._0` is rewritten to `expr[OFFSET(0)]`, matching the
// transpiler's synthesized `_N` field naming on the DuckDB emit side.
std::string PreprocessSqlForAnalyzer(absl::string_view sql);

}  // namespace coordinator
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator

#endif  // BIGQUERY_EMULATOR_BACKEND_ENGINE_COORDINATOR_SQL_PREPROCESS_H_
