#include "backend/catalog/procedure_registry.h"

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
#include "backend/catalog/create_procedure_util.h"
#include "backend/catalog/stored_procedure.h"
#include "googlesql/public/analyzer_output.h"
#include "googlesql/public/simple_catalog.h"

namespace bigquery_emulator {
namespace backend {
namespace catalog {

namespace {

struct ProjectProcedures {
  std::vector<std::unique_ptr<const ::googlesql::AnalyzerOutput>>
      analyzer_outputs;
  std::vector<std::unique_ptr<const StoredSQLProcedure>> procedures;
  // Replaced/dropped procedures are retired instead of destroyed:
  // catalogs hold raw pointers handed out via AddProcedure, and
  // destroying the object on re-registration leaves them dangling
  // (same use-after-free class as udf_registry.cc).
  std::vector<std::unique_ptr<const StoredSQLProcedure>> retired_procedures;
};

absl::Mutex mu;
absl::flat_hash_map<std::string, ProjectProcedures> by_project
    ABSL_GUARDED_BY(mu);

}  // namespace

absl::Status RegisterProjectProcedure(
    absl::string_view project_id,
    const ::googlesql::ResolvedCreateProcedureStmt& create_procedure_stmt,
    std::unique_ptr<const ::googlesql::AnalyzerOutput> analyzer_output) {
  if (project_id.empty()) {
    return absl::InvalidArgumentError(
        "procedure_registry: project_id must be non-empty");
  }
  absl::StatusOr<std::unique_ptr<StoredSQLProcedure>> proc_or =
      MakeProcedureFromCreateProcedure(create_procedure_stmt);
  if (!proc_or.ok()) return proc_or.status();
  const std::string proc_name = create_procedure_stmt.name_path().back();

  absl::MutexLock lock(&mu);
  ProjectProcedures& bucket = by_project[std::string(project_id)];
  for (auto it = bucket.procedures.begin(); it != bucket.procedures.end();
       ++it) {
    if (*it != nullptr && absl::EqualsIgnoreCase((*it)->Name(), proc_name)) {
      bucket.retired_procedures.push_back(std::move(*it));
      bucket.procedures.erase(it);
      break;
    }
  }
  if (analyzer_output != nullptr) {
    bucket.analyzer_outputs.push_back(std::move(analyzer_output));
  }
  std::unique_ptr<const StoredSQLProcedure> owned = std::move(*proc_or);
  bucket.procedures.push_back(std::move(owned));
  return absl::OkStatus();
}

void ReplayProceduresIntoCatalog(absl::string_view project_id,
                                 ::googlesql::SimpleCatalog& catalog) {
  absl::MutexLock lock(&mu);
  auto it = by_project.find(std::string(project_id));
  if (it == by_project.end()) return;
  for (const auto& proc : it->second.procedures) {
    if (proc == nullptr) continue;
    const ::googlesql::Procedure* existing = nullptr;
    const absl::Status found = catalog.GetProcedure(proc->Name(), &existing);
    if (found.ok() && existing == proc.get()) {
      continue;
    }
    // SimpleCatalog::GetProcedure can return OK with a null pointer when the
    // name is absent; only skip when another procedure already owns the name.
    if (found.ok() && existing != nullptr) {
      continue;
    }
    catalog.AddProcedure(proc.get());
  }
}

absl::Status DropProjectProcedure(absl::string_view project_id,
                                  absl::string_view procedure_name) {
  if (project_id.empty() || procedure_name.empty()) {
    return absl::InvalidArgumentError(
        "procedure_registry: project_id and procedure_name must be non-empty");
  }
  absl::MutexLock lock(&mu);
  auto it = by_project.find(std::string(project_id));
  if (it == by_project.end()) {
    return absl::NotFoundError(
        absl::StrCat("procedure not found: ", procedure_name));
  }
  auto& procs = it->second.procedures;
  for (auto nit = procs.begin(); nit != procs.end(); ++nit) {
    if (*nit != nullptr &&
        absl::EqualsIgnoreCase((*nit)->Name(), procedure_name)) {
      it->second.retired_procedures.push_back(std::move(*nit));
      procs.erase(nit);
      return absl::OkStatus();
    }
  }
  return absl::NotFoundError(
      absl::StrCat("procedure not found: ", procedure_name));
}

const StoredSQLProcedure* FindProjectProcedure(
    absl::string_view project_id, absl::string_view procedure_name) {
  if (project_id.empty() || procedure_name.empty()) return nullptr;
  absl::MutexLock lock(&mu);
  auto it = by_project.find(std::string(project_id));
  if (it == by_project.end()) return nullptr;
  for (const auto& proc : it->second.procedures) {
    if (proc != nullptr &&
        absl::EqualsIgnoreCase(proc->Name(), procedure_name)) {
      return proc.get();
    }
  }
  return nullptr;
}

}  // namespace catalog
}  // namespace backend
}  // namespace bigquery_emulator
