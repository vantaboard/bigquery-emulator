#ifndef BIGQUERY_EMULATOR_BACKEND_ENGINE_SEMANTIC_SYSTEM_VARIABLES_H_
#define BIGQUERY_EMULATOR_BACKEND_ENGINE_SEMANTIC_SYSTEM_VARIABLES_H_

// BigQuery @@ system variable registration and per-project session
// values for the semantic executor (googlesqlite plan 14).

#include <string>
#include <vector>

#include "absl/status/status.h"
#include "absl/strings/string_view.h"
#include "backend/engine/semantic/value.h"
#include "googlesql/public/analyzer_options.h"
#include "googlesql/public/types/type_factory.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace semantic {

// Registers @@time_zone (and future vars) on `options` so the
// analyzer resolves `SET @@time_zone` / `SELECT @@time_zone`.
absl::Status RegisterAnalyzerSystemVariables(
    ::googlesql::TypeFactory* type_factory,
    ::googlesql::AnalyzerOptions& options);

// Read the current value for `name_path` (e.g. {"time_zone"}).
// Returns UTC when unset (BigQuery default for tests).
absl::StatusOr<Value> GetSystemVariable(absl::string_view project_id,
                                        const std::vector<std::string>& name_path);

// Persist `value` for `name_path` on `project_id` (conn/session scope
// is approximated by per-test project ids in googlesqlite e2e).
absl::Status SetSystemVariable(absl::string_view project_id,
                               const std::vector<std::string>& name_path,
                               Value value);

}  // namespace semantic
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator

#endif  // BIGQUERY_EMULATOR_BACKEND_ENGINE_SEMANTIC_SYSTEM_VARIABLES_H_
