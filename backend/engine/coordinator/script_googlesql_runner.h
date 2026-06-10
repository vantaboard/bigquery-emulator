#ifndef BIGQUERY_EMULATOR_BACKEND_ENGINE_COORDINATOR_SCRIPT_GOOGLESQL_RUNNER_H_
#define BIGQUERY_EMULATOR_BACKEND_ENGINE_COORDINATOR_SCRIPT_GOOGLESQL_RUNNER_H_

#include "absl/status/statusor.h"
#include "absl/strings/string_view.h"
#include "backend/engine/coordinator/local_coordinator_engine.h"
#include "backend/engine/semantic/row_source.h"
#include "backend/engine/semantic/script/script_driver.h"
#include "googlesql/public/catalog.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace coordinator {

absl::StatusOr<std::unique_ptr<RowSource>> ExecuteScriptViaGoogleSql(
    LocalCoordinatorEngine& engine,
    const QueryRequest& request,
    ::googlesql::Catalog* catalog,
    semantic::script::ScriptDriver* driver = nullptr,
    absl::string_view script_sql = {});

}  // namespace coordinator
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator

#endif  // BIGQUERY_EMULATOR_BACKEND_ENGINE_COORDINATOR_SCRIPT_GOOGLESQL_RUNNER_H_
