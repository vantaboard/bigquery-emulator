#include "backend/catalog/tvf_registry.h"

#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "absl/container/flat_hash_map.h"
#include "absl/status/status.h"
#include "absl/strings/match.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/string_view.h"
#include "absl/synchronization/mutex.h"
#include "backend/catalog/create_tvf_util.h"
#include "googlesql/public/analyzer_output.h"
#include "googlesql/public/simple_catalog.h"
#include "googlesql/public/table_valued_function.h"

namespace bigquery_emulator {
namespace backend {
namespace catalog {

namespace {

struct ProjectTvfs {
  std::vector<std::unique_ptr<const ::googlesql::AnalyzerOutput>>
      analyzer_outputs;
  std::vector<std::unique_ptr<const ::googlesql::TableValuedFunction>> tvfs;
  // Replaced/dropped TVFs are retired instead of destroyed: catalogs
  // hold raw pointers handed out via AddTableValuedFunction, and
  // destroying the object on re-registration leaves them dangling
  // (same use-after-free class as udf_registry.cc).
  std::vector<std::unique_ptr<const ::googlesql::TableValuedFunction>>
      retired_tvfs;
};

absl::Mutex mu;
absl::flat_hash_map<std::string, ProjectTvfs> by_project ABSL_GUARDED_BY(mu);

}  // namespace

absl::Status RegisterProjectTvf(
    absl::string_view project_id,
    const ::googlesql::ResolvedCreateTableFunctionStmt& create_tvf_stmt,
    std::unique_ptr<const ::googlesql::AnalyzerOutput> analyzer_output) {
  if (project_id.empty()) {
    return absl::InvalidArgumentError(
        "tvf_registry: project_id must be non-empty");
  }
  absl::StatusOr<std::unique_ptr<const ::googlesql::TableValuedFunction>>
      tvf_or = MakeTvfFromCreateTableFunction(create_tvf_stmt);
  if (!tvf_or.ok()) return tvf_or.status();
  const std::string tvf_name = create_tvf_stmt.name_path().back();

  absl::MutexLock lock(&mu);
  ProjectTvfs& bucket = by_project[std::string(project_id)];
  for (auto it = bucket.tvfs.begin(); it != bucket.tvfs.end(); ++it) {
    if (*it != nullptr && absl::EqualsIgnoreCase((*it)->Name(), tvf_name)) {
      bucket.retired_tvfs.push_back(std::move(*it));
      bucket.tvfs.erase(it);
      break;
    }
  }
  if (analyzer_output != nullptr) {
    bucket.analyzer_outputs.push_back(std::move(analyzer_output));
  }
  bucket.tvfs.push_back(std::move(*tvf_or));
  return absl::OkStatus();
}

void ReplayTvfsIntoCatalog(absl::string_view project_id,
                           ::googlesql::SimpleCatalog& catalog) {
  absl::MutexLock lock(&mu);
  auto it = by_project.find(std::string(project_id));
  if (it == by_project.end()) return;
  for (const auto& tvf : it->second.tvfs) {
    if (tvf == nullptr) continue;
    catalog.AddTableValuedFunction(tvf.get());
  }
}

absl::Status DropProjectTvf(absl::string_view project_id,
                            absl::string_view tvf_name) {
  if (project_id.empty() || tvf_name.empty()) {
    return absl::InvalidArgumentError(
        "tvf_registry: project_id and tvf_name must be non-empty");
  }
  absl::MutexLock lock(&mu);
  auto it = by_project.find(std::string(project_id));
  if (it == by_project.end()) {
    return absl::NotFoundError(absl::StrCat("TVF not found: ", tvf_name));
  }
  auto& tvfs = it->second.tvfs;
  for (auto nit = tvfs.begin(); nit != tvfs.end(); ++nit) {
    if (*nit != nullptr && absl::EqualsIgnoreCase((*nit)->Name(), tvf_name)) {
      it->second.retired_tvfs.push_back(std::move(*nit));
      tvfs.erase(nit);
      return absl::OkStatus();
    }
  }
  return absl::NotFoundError(absl::StrCat("TVF not found: ", tvf_name));
}

}  // namespace catalog
}  // namespace backend
}  // namespace bigquery_emulator
