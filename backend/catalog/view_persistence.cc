#include "backend/catalog/view_persistence.h"

#include <string>
#include <vector>

#include "absl/status/status.h"
#include "absl/strings/string_view.h"
#include "backend/engine/engine.h"
#include "backend/storage/storage.h"
#include "googlesql/resolved_ast/resolved_ast.h"

namespace bigquery_emulator {
namespace backend {
namespace catalog {

namespace {

bool ViewIdIsComplete(const storage::ViewId& id) {
  return !id.project_id.empty() && !id.dataset_id.empty() &&
         !id.view_id.empty();
}

}  // namespace

storage::ViewId ViewIdFromNamePath(const std::vector<std::string>& name_path,
                                   absl::string_view project_id,
                                   absl::string_view default_dataset_id) {
  storage::ViewId id;
  switch (name_path.size()) {
    case 1:
      id.project_id = std::string(project_id);
      id.dataset_id = std::string(default_dataset_id);
      id.view_id = name_path[0];
      break;
    case 2:
      id.project_id = std::string(project_id);
      id.dataset_id = name_path[0];
      id.view_id = name_path[1];
      break;
    default:
      if (name_path.size() >= 3) {
        id.project_id = name_path[0];
        id.dataset_id = name_path[1];
        id.view_id = name_path.back();
      }
      break;
  }
  return id;
}

absl::Status PersistViewDdl(storage::Storage* storage,
                            const engine::QueryRequest& request,
                            const ::googlesql::ResolvedCreateViewStmt& stmt) {
  if (storage == nullptr) return absl::OkStatus();
  storage::ViewRecord rec;
  rec.ddl_sql = request.sql;
  rec.id = ViewIdFromNamePath(
      stmt.name_path(), request.project_id, request.default_dataset_id);
  if (!ViewIdIsComplete(rec.id)) return absl::OkStatus();
  return storage->UpsertView(rec);
}

absl::Status DeletePersistedView(storage::Storage* storage,
                                 const storage::ViewId& id) {
  if (storage == nullptr) return absl::OkStatus();
  absl::Status deleted = storage->DeleteView(id);
  if (deleted.ok() || deleted.code() == absl::StatusCode::kNotFound) {
    return absl::OkStatus();
  }
  return deleted;
}

}  // namespace catalog
}  // namespace backend
}  // namespace bigquery_emulator
