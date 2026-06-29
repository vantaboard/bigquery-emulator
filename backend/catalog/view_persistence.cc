

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
  // BigQuery accepts a fully backtick-quoted path (`p.d.v`) where the
  // analyzer returns the whole dotted string as a single name-path
  // segment. Split before applying the 1/2/3-segment rules (mirrors
  // control_op_ddl.cc NamePathToTableId and gateway splitViewTableName).
  std::vector<std::string> segments = name_path;
  if (segments.size() == 1 && absl::StrContains(segments[0], '.')) {
    segments = absl::StrSplit(segments[0], '.');
  }
  storage::ViewId id;
  switch (segments.size()) {
    case 1:
      id.project_id = std::string(project_id);
      id.dataset_id = std::string(default_dataset_id);
      id.view_id = segments[0];
      break;
    case 2:
      id.project_id = std::string(project_id);
      id.dataset_id = segments[0];
      id.view_id = segments[1];
      break;
    default:
      if (segments.size() >= 3) {
        id.project_id = segments[0];
        id.dataset_id = segments[1];
        id.view_id = segments.back();
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
  rec.view_query = std::string(stmt.sql());
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
