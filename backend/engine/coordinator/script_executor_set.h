#ifndef BIGQUERY_EMULATOR_BACKEND_ENGINE_COORDINATOR_SCRIPT_EXECUTOR_SET_H_
#define BIGQUERY_EMULATOR_BACKEND_ENGINE_COORDINATOR_SCRIPT_EXECUTOR_SET_H_

#include "absl/status/status.h"
#include "backend/catalog/googlesql_catalog.h"
#include "backend/engine/engine.h"
#include "backend/engine/semantic/script/script_driver.h"
#include "googlesql/public/catalog.h"
#include "googlesql/public/parse_resume_location.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace coordinator {

absl::Status ExecuteProcedureSet(const QueryRequest& request,
                                 absl::string_view target_name,
                                 absl::string_view expr,
                                 catalog::GoogleSqlCatalog* bq_catalog,
                                 ::googlesql::Catalog* catalog,
                                 semantic::script::ScriptDriver& driver);

absl::Status TryExecuteLeadingSetStatement(
    const QueryRequest& request,
    ::googlesql::ParseResumeLocation& resume,
    catalog::GoogleSqlCatalog* bq_catalog,
    ::googlesql::Catalog* catalog,
    semantic::script::ScriptDriver& driver,
    bool* handled);

}  // namespace coordinator
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator

#endif  // BIGQUERY_EMULATOR_BACKEND_ENGINE_COORDINATOR_SCRIPT_EXECUTOR_SET_H_
