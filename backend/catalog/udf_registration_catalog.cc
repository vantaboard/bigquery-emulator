#include "backend/catalog/udf_registration_catalog.h"

#include <memory>
#include <string>

#include "absl/container/flat_hash_map.h"
#include "absl/synchronization/mutex.h"
#include "backend/catalog/udf_registry.h"

namespace bigquery_emulator {
namespace backend {
namespace catalog {

namespace {

struct RegistrationCatalogEntry {
  std::unique_ptr<GoogleSqlCatalog> catalog;
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
  if (entry.catalog == nullptr) {
    entry.catalog = std::make_unique<GoogleSqlCatalog>(
        project_id, storage, type_factory, language, default_dataset_id);
  }
  ReplayFunctionsIntoCatalog(project_id, *entry.catalog);
  return entry.catalog.get();
}

}  // namespace catalog
}  // namespace backend
}  // namespace bigquery_emulator
