

#include "absl/base/thread_annotations.h"
#include "googlesql/public/analyzer_output.h"
#include "googlesql/public/catalog.h"

namespace bigquery_emulator {
namespace backend {
namespace catalog {

namespace {

struct RegisteredViewEntry {
  std::string dataset_id;
  // SQL text the view was defined with (ResolvedCreateViewStmt::sql()),
  // surfaced verbatim through INFORMATION_SCHEMA.VIEWS.view_definition.
  std::string view_definition;
  std::unique_ptr<const ::googlesql::Table> view;
};

struct ProjectViews {
  std::vector<std::unique_ptr<const ::googlesql::AnalyzerOutput>>
      analyzer_outputs{};
  std::vector<RegisteredViewEntry> views{};
  // Replaced/dropped views are retired instead of destroyed: catalogs
  // hold raw pointers handed out via AddTable, and destroying the
  // object on re-registration leaves them dangling (same
  // use-after-free class as udf_registry.cc).
  std::vector<RegisteredViewEntry> retired_views{};
};

absl::Mutex mu;
absl::flat_hash_map<std::string, ProjectViews> by_project ABSL_GUARDED_BY(mu);

}  // namespace

absl::Status RegisterProjectView(
    absl::string_view project_id,
    absl::string_view default_dataset_id,
    const ::googlesql::ResolvedCreateViewStmt& create_view_stmt,
    std::unique_ptr<const ::googlesql::AnalyzerOutput> analyzer_output,
    ::googlesql::TypeFactory* type_factory) {
  if (project_id.empty()) {
    return absl::InvalidArgumentError(
        "view_registry: project_id must be non-empty");
  }
  if (type_factory == nullptr) {
    return absl::InvalidArgumentError(
        "view_registry: type_factory must be non-null");
  }
  const storage::ViewId vid = ViewIdFromNamePath(
      create_view_stmt.name_path(), project_id, default_dataset_id);
  const std::string view_name = vid.view_id;
  const std::string dataset_id = vid.dataset_id;
  if (view_name.empty()) {
    return absl::InvalidArgumentError(
        "view_registry: CREATE VIEW has empty view name");
  }
  absl::StatusOr<std::unique_ptr<const ::googlesql::Table>> view_or =
      MakeViewFromCreateView(create_view_stmt, type_factory, view_name);
  if (!view_or.ok()) return view_or.status();

  absl::MutexLock lock(&mu);
  ProjectViews& bucket = by_project[std::string(project_id)];
  for (auto it = bucket.views.begin(); it != bucket.views.end(); ++it) {
    if (it->view != nullptr &&
        absl::EqualsIgnoreCase(it->view->Name(), view_name) &&
        (dataset_id.empty() ||
         absl::EqualsIgnoreCase(it->dataset_id, dataset_id))) {
      bucket.retired_views.push_back(std::move(*it));
      bucket.views.erase(it);
      break;
    }
  }
  if (analyzer_output != nullptr) {
    bucket.analyzer_outputs.push_back(std::move(analyzer_output));
  }
  bucket.views.push_back(RegisteredViewEntry{
      dataset_id, std::string(create_view_stmt.sql()), std::move(*view_or)});
  return absl::OkStatus();
}

const ::googlesql::Table* FindProjectView(absl::string_view project_id,
                                          absl::string_view dataset_id,
                                          absl::string_view view_name) {
  if (project_id.empty() || view_name.empty()) return nullptr;
  absl::MutexLock lock(&mu);
  auto it = by_project.find(std::string(project_id));
  if (it == by_project.end()) return nullptr;
  for (const RegisteredViewEntry& entry : it->second.views) {
    if (entry.view == nullptr) continue;
    if (!absl::EqualsIgnoreCase(entry.view->Name(), view_name)) continue;
    if (!dataset_id.empty() && !entry.dataset_id.empty() &&
        !absl::EqualsIgnoreCase(entry.dataset_id, dataset_id)) {
      continue;
    }
    return entry.view.get();
  }
  return nullptr;
}

bool FindRegisteredViewInfo(absl::string_view project_id,
                            absl::string_view dataset_id,
                            absl::string_view view_name,
                            RegisteredViewInfo* out) {
  if (project_id.empty() || view_name.empty() || out == nullptr) {
    return false;
  }
  absl::MutexLock lock(&mu);
  auto it = by_project.find(std::string(project_id));
  if (it == by_project.end()) return false;
  for (const RegisteredViewEntry& entry : it->second.views) {
    if (entry.view == nullptr) continue;
    if (!absl::EqualsIgnoreCase(entry.view->Name(), view_name)) continue;
    if (!dataset_id.empty() && !entry.dataset_id.empty() &&
        !absl::EqualsIgnoreCase(entry.dataset_id, dataset_id)) {
      continue;
    }
    *out = RegisteredViewInfo{entry.dataset_id,
                              std::string(entry.view->Name()),
                              entry.view_definition,
                              /*use_standard_sql=*/true};
    return true;
  }
  return false;
}

std::vector<RegisteredViewInfo> ListProjectViews(absl::string_view project_id,
                                                 absl::string_view dataset_id) {
  std::vector<RegisteredViewInfo> out;
  if (project_id.empty()) return out;
  // A region-* selector (e.g. `region-us`) is project-scoped: it
  // returns every dataset's views, same as an unqualified call.
  const bool all_datasets =
      dataset_id.empty() || absl::StartsWith(dataset_id, "region-");
  absl::MutexLock lock(&mu);
  auto it = by_project.find(std::string(project_id));
  if (it == by_project.end()) return out;
  for (const RegisteredViewEntry& entry : it->second.views) {
    if (entry.view == nullptr) continue;
    if (!all_datasets &&
        !absl::EqualsIgnoreCase(entry.dataset_id, dataset_id)) {
      continue;
    }
    out.push_back(RegisteredViewInfo{entry.dataset_id,
                                     std::string(entry.view->Name()),
                                     entry.view_definition,
                                     /*use_standard_sql=*/true});
  }
  std::sort(out.begin(),
            out.end(),
            [](const RegisteredViewInfo& a, const RegisteredViewInfo& b) {
              if (a.dataset_id != b.dataset_id)
                return a.dataset_id < b.dataset_id;
              return a.view_name < b.view_name;
            });
  return out;
}

absl::Status DropProjectView(absl::string_view project_id,
                             absl::string_view view_name) {
  if (project_id.empty() || view_name.empty()) {
    return absl::InvalidArgumentError(
        "view_registry: project_id and view_name must be non-empty");
  }
  absl::MutexLock lock(&mu);
  auto it = by_project.find(std::string(project_id));
  if (it == by_project.end()) {
    return absl::NotFoundError(absl::StrCat("view not found: ", view_name));
  }
  auto& views = it->second.views;
  for (auto vit = views.begin(); vit != views.end(); ++vit) {
    if (vit->view != nullptr &&
        absl::EqualsIgnoreCase(vit->view->Name(), view_name)) {
      it->second.retired_views.push_back(std::move(*vit));
      views.erase(vit);
      return absl::OkStatus();
    }
  }
  return absl::NotFoundError(absl::StrCat("view not found: ", view_name));
}

}  // namespace catalog
}  // namespace backend
}  // namespace bigquery_emulator
