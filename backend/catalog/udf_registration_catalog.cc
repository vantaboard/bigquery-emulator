

#include "absl/base/thread_annotations.h"

namespace bigquery_emulator {
namespace backend {
namespace catalog {

namespace {

struct RegistrationCatalogEntry {
  storage::Storage* storage = nullptr;
  std::unique_ptr<GoogleSqlCatalog> catalog{};
};

absl::Mutex mu;
absl::flat_hash_map<std::string, RegistrationCatalogEntry> by_project
    ABSL_GUARDED_BY(mu);

}  // namespace

GoogleSqlCatalog* GetOrCreateRegistrationCatalog(
    absl::string_view project_id,
    storage::Storage* storage,
    ::googlesql::TypeFactory* type_factory,
    const ::googlesql::LanguageOptions& language,
    absl::string_view default_dataset_id) {
  if (project_id.empty() || storage == nullptr || type_factory == nullptr) {
    return nullptr;
  }
  absl::MutexLock lock(&mu);
  RegistrationCatalogEntry& entry = by_project[std::string(project_id)];
  const bool need_new_catalog =
      entry.catalog == nullptr || entry.storage != storage;
  if (need_new_catalog) {
    entry.storage = storage;
    entry.catalog = std::make_unique<GoogleSqlCatalog>(
        project_id, storage, type_factory, language, default_dataset_id);
    // GoogleSqlCatalog's constructor replays project UDFs once; skip a
    // second replay here so AddFunction does not InsertOrDie on a name
    // that was just registered during construction.
    return entry.catalog.get();
  }
  ReplayFunctionsIntoCatalog(project_id, *entry.catalog);
  ReplayProceduresIntoCatalog(project_id, *entry.catalog);
  return entry.catalog.get();
}

GoogleSqlCatalog* LookupRegistrationCatalog(absl::string_view project_id) {
  if (project_id.empty()) return nullptr;
  absl::MutexLock lock(&mu);
  auto it = by_project.find(std::string(project_id));
  if (it == by_project.end() || it->second.catalog == nullptr) {
    return nullptr;
  }
  return it->second.catalog.get();
}

}  // namespace catalog
}  // namespace backend
}  // namespace bigquery_emulator
