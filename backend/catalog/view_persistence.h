#ifndef BIGQUERY_EMULATOR_BACKEND_CATALOG_VIEW_PERSISTENCE_H_
#define BIGQUERY_EMULATOR_BACKEND_CATALOG_VIEW_PERSISTENCE_H_

#include "absl/status/status.h"
#include "absl/strings/string_view.h"
#include "backend/engine/engine.h"
#include "backend/storage/storage.h"
#include "googlesql/resolved_ast/resolved_ast.h"

namespace bigquery_emulator {
namespace backend {
namespace catalog {

storage::ViewId ViewIdFromNamePath(const std::vector<std::string>& name_path,
                                   absl::string_view project_id,
                                   absl::string_view default_dataset_id);

absl::Status PersistViewDdl(storage::Storage* storage,
                            const engine::QueryRequest& request,
                            const ::googlesql::ResolvedCreateViewStmt& stmt);

absl::Status DeletePersistedView(storage::Storage* storage,
                                 const storage::ViewId& id);

}  // namespace catalog
}  // namespace backend
}  // namespace bigquery_emulator

#endif  // BIGQUERY_EMULATOR_BACKEND_CATALOG_VIEW_PERSISTENCE_H_
