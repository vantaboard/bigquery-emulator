#ifndef BIGQUERY_EMULATOR_BACKEND_SQLTOOLS_CATALOG_NAMES_H_
#define BIGQUERY_EMULATOR_BACKEND_SQLTOOLS_CATALOG_NAMES_H_

#include "absl/status/status.h"
#include "absl/strings/string_view.h"
#include "backend/sqltools/sql_tools.h"
#include "backend/storage/storage.h"

namespace bigquery_emulator {
namespace backend {
namespace sqltools {

// Populates dataset and table name lists from durable storage. Used by
// the SqlTools gRPC handler before calling CompleteSqlText.
absl::Status PopulateCatalogNamesFromStorage(
    absl::string_view project_id,
    absl::string_view default_dataset_id,
    storage::Storage* storage,
    CatalogNames* names);

}  // namespace sqltools
}  // namespace backend
}  // namespace bigquery_emulator

#endif  // BIGQUERY_EMULATOR_BACKEND_SQLTOOLS_CATALOG_NAMES_H_
