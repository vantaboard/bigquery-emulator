#include "backend/catalog/udf_registry.h"

#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "absl/container/flat_hash_map.h"
#include "absl/container/flat_hash_set.h"
#include "absl/status/status.h"
#include "absl/strings/match.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/string_view.h"
#include "absl/synchronization/mutex.h"
#include "absl/types/span.h"
#include "backend/catalog/js_udf_registry.h"
#include "backend/catalog/python_udf_registry.h"
#include "googlesql/public/analyzer_output.h"
#include "googlesql/public/catalog.h"
#include "googlesql/public/function.h"
#include "googlesql/public/simple_catalog.h"
#include "googlesql/public/types/type_factory.h"

namespace bigquery_emulator {
namespace backend {
namespace catalog {

namespace {

struct RegisteredFunctionEntry {
  std::string dataset_id;
  std::unique_ptr<const ::googlesql::Function> function;
};

struct ProjectFunctions {
  std::unique_ptr<::googlesql::TypeFactory> type_factory{};
  std::vector<std::unique_ptr<const ::googlesql::AnalyzerOutput>>
      analyzer_outputs{};
  std::vector<RegisteredFunctionEntry> functions{};
  // Replaced/dropped functions are retired here instead of destroyed:
  // long-lived catalogs (the per-project registration catalog in
  // udf_registration_catalog.cc and any in-flight query catalogs) hold
  // raw pointers handed out via SimpleCatalog::AddFunction, and
  // ReplayFunctionsIntoCatalog dereferences them when deciding what to
  // remove. Destroying the object on re-registration left those
  // pointers dangling, which crashed the engine on the next replay
  // (use-after-free -> InsertOrDie duplicate key / SIGSEGV).
  std::vector<std::unique_ptr<const ::googlesql::Function>> retired_functions{};
};

absl::Mutex mu;
absl::flat_hash_map<std::string, ProjectFunctions> by_project
    ABSL_GUARDED_BY(mu);

}  // namespace

absl::Status RegisterProjectFunction(
    absl::string_view project_id,
    absl::string_view dataset_id,
    bool is_temp,
    std::unique_ptr<const ::googlesql::AnalyzerOutput> analyzer_output,
    std::unique_ptr<const ::googlesql::Function> function) {
  (void)is_temp;
  if (project_id.empty()) {
    return absl::InvalidArgumentError(
        "udf_registry: project_id must be non-empty");
  }
  if (function == nullptr) {
    return absl::InvalidArgumentError(
        "udf_registry: function must be non-null");
  }
  absl::MutexLock lock(&mu);
  ProjectFunctions& bucket = by_project[std::string(project_id)];
  if (bucket.type_factory == nullptr) {
    bucket.type_factory = std::make_unique<::googlesql::TypeFactory>();
  }
  if (analyzer_output != nullptr) {
    bucket.analyzer_outputs.push_back(std::move(analyzer_output));
  }
  const std::string fn_name = function->Name();
  for (auto it = bucket.functions.begin(); it != bucket.functions.end(); ++it) {
    if (it->function == nullptr) continue;
    if (!absl::EqualsIgnoreCase(it->function->Name(), fn_name)) continue;
    if (!dataset_id.empty() && !it->dataset_id.empty() &&
        !absl::EqualsIgnoreCase(it->dataset_id, dataset_id)) {
      continue;
    }
    bucket.retired_functions.push_back(std::move(it->function));
    bucket.functions.erase(it);
    break;
  }
  bucket.functions.push_back(
      RegisteredFunctionEntry{std::string(dataset_id), std::move(function)});
  return absl::OkStatus();
}

const ::googlesql::Function* FindProjectFunction(
    absl::string_view project_id,
    absl::string_view dataset_id,
    absl::string_view routine_name) {
  if (project_id.empty() || routine_name.empty()) return nullptr;
  absl::MutexLock lock(&mu);
  auto it = by_project.find(std::string(project_id));
  if (it == by_project.end()) return nullptr;
  for (const RegisteredFunctionEntry& entry : it->second.functions) {
    if (entry.function == nullptr) continue;
    if (!absl::EqualsIgnoreCase(entry.function->Name(), routine_name)) continue;
    if (!dataset_id.empty() && !entry.dataset_id.empty() &&
        !absl::EqualsIgnoreCase(entry.dataset_id, dataset_id)) {
      continue;
    }
    return entry.function.get();
  }
  return nullptr;
}

namespace {

bool ParseRoutinePath(absl::Span<const std::string> path,
                      absl::string_view catalog_project_id,
                      absl::string_view default_dataset_id,
                      absl::string_view* project_id,
                      absl::string_view* dataset_id,
                      absl::string_view* routine_id) {
  if (project_id == nullptr || dataset_id == nullptr || routine_id == nullptr) {
    return false;
  }
  if (path.empty()) return false;

  if (path.size() == 1) {
    absl::string_view single = path[0];
    const size_t first_dot = single.find('.');
    if (first_dot == absl::string_view::npos) {
      return false;
    }
    const size_t second_dot = single.find('.', first_dot + 1);
    if (second_dot != absl::string_view::npos) {
      *project_id = single.substr(0, first_dot);
      *dataset_id = single.substr(first_dot + 1, second_dot - first_dot - 1);
      *routine_id = single.substr(second_dot + 1);
      return true;
    }
    *project_id = catalog_project_id;
    *dataset_id = single.substr(0, first_dot);
    *routine_id = single.substr(first_dot + 1);
    return true;
  }
  if (path.size() == 2) {
    *project_id = catalog_project_id;
    *dataset_id = path[0];
    *routine_id = path[1];
    return true;
  }
  if (path.size() == 3) {
    *project_id = path[0];
    *dataset_id = path[1];
    *routine_id = path[2];
    return true;
  }
  return false;
}

}  // namespace

const ::googlesql::Function* FindProjectFunctionFromPath(
    absl::Span<const std::string> path,
    absl::string_view catalog_project_id,
    absl::string_view default_dataset_id) {
  absl::string_view project_id;
  absl::string_view dataset_id;
  absl::string_view routine_id;
  if (!ParseRoutinePath(path,
                        catalog_project_id,
                        default_dataset_id,
                        &project_id,
                        &dataset_id,
                        &routine_id)) {
    return nullptr;
  }
  return FindProjectFunction(project_id, dataset_id, routine_id);
}

::googlesql::TypeFactory* LookupProjectTypeFactory(
    absl::string_view project_id) {
  if (project_id.empty()) return nullptr;
  absl::MutexLock lock(&mu);
  auto it = by_project.find(std::string(project_id));
  if (it == by_project.end() || it->second.type_factory == nullptr) {
    return nullptr;
  }
  return it->second.type_factory.get();
}

::googlesql::TypeFactory* EnsureProjectTypeFactory(
    absl::string_view project_id) {
  if (project_id.empty()) return nullptr;
  absl::MutexLock lock(&mu);
  ProjectFunctions& bucket = by_project[std::string(project_id)];
  if (bucket.type_factory == nullptr) {
    bucket.type_factory = std::make_unique<::googlesql::TypeFactory>();
  }
  return bucket.type_factory.get();
}

bool IsProjectRegisteredFunction(absl::string_view project_id,
                                 absl::string_view fn_name) {
  if (project_id.empty() || fn_name.empty()) return false;
  absl::MutexLock lock(&mu);
  auto it = by_project.find(std::string(project_id));
  if (it == by_project.end()) return false;
  for (const RegisteredFunctionEntry& entry : it->second.functions) {
    if (entry.function != nullptr &&
        absl::EqualsIgnoreCase(entry.function->Name(), fn_name)) {
      return true;
    }
  }
  return false;
}

absl::Status DropProjectFunction(absl::string_view project_id,
                                 absl::string_view fn_name) {
  if (project_id.empty() || fn_name.empty()) {
    return absl::InvalidArgumentError(
        "udf_registry: project_id and fn_name must be non-empty");
  }
  absl::MutexLock lock(&mu);
  auto it = by_project.find(std::string(project_id));
  if (it == by_project.end()) {
    return absl::NotFoundError(absl::StrCat("function not found: ", fn_name));
  }
  auto& fns = it->second.functions;
  for (auto nit = fns.begin(); nit != fns.end(); ++nit) {
    if (nit->function != nullptr &&
        absl::EqualsIgnoreCase(nit->function->Name(), fn_name)) {
      it->second.retired_functions.push_back(std::move(nit->function));
      fns.erase(nit);
      DropProjectJsUdf(project_id, fn_name);
      DropProjectPythonUdf(project_id, fn_name);
      return absl::OkStatus();
    }
  }
  return absl::NotFoundError(absl::StrCat("function not found: ", fn_name));
}

void ReplayFunctionsIntoCatalog(absl::string_view project_id,
                                ::googlesql::SimpleCatalog& catalog) {
  absl::MutexLock lock(&mu);
  auto it = by_project.find(std::string(project_id));
  if (it == by_project.end()) return;
  // Purge entries that point at retired (replaced or dropped)
  // functions so a long-lived catalog does not keep resolving a
  // dropped routine or shadow the re-registered one.
  if (!it->second.retired_functions.empty()) {
    absl::flat_hash_set<const ::googlesql::Function*> retired;
    retired.reserve(it->second.retired_functions.size());
    for (const auto& fn : it->second.retired_functions) {
      retired.insert(fn.get());
    }
    catalog.RemoveFunctions([&retired](const ::googlesql::Function* fn) {
      return retired.contains(fn);
    });
  }
  for (const RegisteredFunctionEntry& entry : it->second.functions) {
    if (entry.function == nullptr) continue;
    const std::string name = entry.function->Name();
    const ::googlesql::Function* existing = nullptr;
    if (catalog.GetFunction(name, &existing).ok() &&
        existing == entry.function.get()) {
      continue;
    }
    // User-defined functions shadow built-ins with the same name
    // (e.g. migration `nullifzero` / community `typeof` UDFs).
    catalog.RemoveFunctions([&name](const ::googlesql::Function* existing_fn) {
      return absl::EqualsIgnoreCase(existing_fn->Name(), name);
    });
    catalog.AddFunction(entry.function.get());
  }
}

}  // namespace catalog
}  // namespace backend
}  // namespace bigquery_emulator
