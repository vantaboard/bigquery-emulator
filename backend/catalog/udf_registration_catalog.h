#ifndef BIGQUERY_EMULATOR_BACKEND_CATALOG_UDF_REGISTRATION_CATALOG_H_
#define BIGQUERY_EMULATOR_BACKEND_CATALOG_UDF_REGISTRATION_CATALOG_H_

// Process-wide `GoogleSqlCatalog` instances used only while analyzing
// `CREATE FUNCTION` statements. SQL UDF bodies store `Function*`
// pointers resolved against this catalog; it must outlive the stored
// `SQLFunction::FunctionExpression()` trees in `udf_registry`.

#include "absl/strings/string_view.h"
#include "backend/catalog/googlesql_catalog.h"
#include "backend/storage/storage.h"
#include "googlesql/public/language_options.h"
#include "googlesql/public/types/type_factory.h"

namespace bigquery_emulator {
namespace backend {
namespace catalog {

// Returns (and creates if needed) the registration catalog for
// `project_id`. Replays any UDFs already registered for the project.
GoogleSqlCatalog* GetOrCreateRegistrationCatalog(
    absl::string_view project_id,
    storage::Storage* storage,
    ::googlesql::TypeFactory* type_factory,
    const ::googlesql::LanguageOptions& language,
    absl::string_view default_dataset_id = "");

// Returns nullptr when the project has no registration catalog yet.
GoogleSqlCatalog* LookupRegistrationCatalog(absl::string_view project_id);

}  // namespace catalog
}  // namespace backend
}  // namespace bigquery_emulator

#endif  // BIGQUERY_EMULATOR_BACKEND_CATALOG_UDF_REGISTRATION_CATALOG_H_
