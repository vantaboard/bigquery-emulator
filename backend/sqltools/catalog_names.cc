#include "backend/sqltools/catalog_names.h"

#include "absl/status/statusor.h"

namespace bigquery_emulator {
namespace backend {
namespace sqltools {

absl::Status PopulateCatalogNamesFromStorage(absl::string_view project_id,
                                             absl::string_view default_dataset_id,
                                             storage::Storage* storage,
                                             CatalogNames* names) {
  if (storage == nullptr) {
    return absl::InvalidArgumentError(
        "PopulateCatalogNamesFromStorage: storage is required");
  }
  if (names == nullptr) {
    return absl::InvalidArgumentError(
        "PopulateCatalogNamesFromStorage: names is required");
  }
  names->datasets.clear();
  names->tables.clear();
  names->columns.clear();

  absl::StatusOr<std::vector<storage::DatasetId>> datasets =
      storage->ListDatasets(project_id);
  if (!datasets.ok()) {
    return datasets.status();
  }
  for (const storage::DatasetId& dataset : *datasets) {
    names->datasets.push_back(dataset.dataset_id);
    absl::StatusOr<std::vector<storage::TableId>> tables =
        storage->ListTables(dataset);
    if (!tables.ok()) {
      return tables.status();
    }
    for (const storage::TableId& table : *tables) {
      const std::string qualified =
          dataset.dataset_id + "." + table.table_id;
      names->tables.push_back(qualified);
      if (!default_dataset_id.empty() &&
          dataset.dataset_id == default_dataset_id) {
        names->tables.push_back(table.table_id);
      }
    }
  }
  return absl::OkStatus();
}

}  // namespace sqltools
}  // namespace backend
}  // namespace bigquery_emulator
