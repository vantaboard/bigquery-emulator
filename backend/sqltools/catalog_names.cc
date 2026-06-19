#include "backend/sqltools/catalog_names.h"

#include "absl/container/flat_hash_set.h"
#include "absl/status/statusor.h"
#include "absl/strings/str_cat.h"
#include "backend/catalog/view_registry.h"
#include "backend/schema/schema.h"

namespace bigquery_emulator {
namespace backend {
namespace sqltools {
namespace {

void AppendUniqueTable(CatalogNames* names, CatalogTableEntry entry) {
  for (const CatalogTableEntry& existing : names->tables) {
    if (existing.label == entry.label && existing.fqn == entry.fqn) {
      return;
    }
  }
  names->tables.push_back(std::move(entry));
}

void AppendUniqueRoutine(CatalogNames* names, CatalogRoutineEntry entry) {
  for (const CatalogRoutineEntry& existing : names->routines) {
    if (existing.label == entry.label && existing.fqn == entry.fqn) {
      return;
    }
  }
  names->routines.push_back(std::move(entry));
}

std::string ColumnTypeLabel(const schema::ColumnSchema& column) {
  if (column.type == schema::ColumnType::kUnknown && !column.raw_type.empty()) {
    return column.raw_type;
  }
  return std::string(schema::ColumnTypeName(column.type));
}

}  // namespace

absl::Status PopulateCatalogNamesFromStorage(
    absl::string_view project_id,
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
  names->routines.clear();
  names->columns.clear();
  names->columns_by_table.clear();
  names->in_scope_tables.clear();

  absl::flat_hash_set<std::string> view_keys;
  for (const catalog::RegisteredViewInfo& view :
       catalog::ListProjectViews(project_id, "")) {
    view_keys.insert(absl::StrCat(view.dataset_id, ".", view.view_name));
  }

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
          absl::StrCat(dataset.dataset_id, ".", table.table_id);
      const std::string fqn = absl::StrCat(
          project_id, ".", dataset.dataset_id, ".", table.table_id);
      const bool is_view = view_keys.contains(qualified);
      CatalogTableEntry entry;
      entry.label = qualified;
      entry.fqn = fqn;
      entry.kind = is_view ? "view" : "table";
      entry.detail = is_view ? "view" : "table";
      AppendUniqueTable(names, entry);
      if (!default_dataset_id.empty() &&
          dataset.dataset_id == default_dataset_id) {
        CatalogTableEntry unqualified;
        unqualified.label = table.table_id;
        unqualified.fqn = fqn;
        unqualified.kind = entry.kind;
        unqualified.detail = entry.detail;
        AppendUniqueTable(names, unqualified);
      }

      absl::StatusOr<schema::TableSchema> schema_or = storage->GetSchema(table);
      if (!schema_or.ok()) {
        continue;
      }
      std::vector<CatalogColumnEntry> columns;
      for (const schema::ColumnSchema& column : schema_or->columns) {
        CatalogColumnEntry col;
        col.name = column.name;
        col.type = ColumnTypeLabel(column);
        columns.push_back(col);
        names->columns.push_back(col);
      }
      names->columns_by_table[qualified] = columns;
      names->columns_by_table[fqn] = columns;
      if (!default_dataset_id.empty() &&
          dataset.dataset_id == default_dataset_id) {
        names->columns_by_table[table.table_id] = columns;
      }
    }

    absl::StatusOr<std::vector<storage::RoutineRecord>> routines =
        storage->ListRoutines(dataset);
    if (!routines.ok()) {
      return routines.status();
    }
    for (const storage::RoutineRecord& routine : *routines) {
      const std::string qualified =
          absl::StrCat(dataset.dataset_id, ".", routine.id.routine_id);
      const std::string fqn = absl::StrCat(
          project_id, ".", dataset.dataset_id, ".", routine.id.routine_id);
      CatalogRoutineEntry entry;
      entry.label = qualified;
      entry.fqn = fqn;
      entry.detail = routine.language;
      AppendUniqueRoutine(names, entry);
      if (!default_dataset_id.empty() &&
          dataset.dataset_id == default_dataset_id) {
        CatalogRoutineEntry unqualified;
        unqualified.label = routine.id.routine_id;
        unqualified.fqn = fqn;
        unqualified.detail = routine.language;
        AppendUniqueRoutine(names, unqualified);
      }
    }
  }

  for (const catalog::RegisteredViewInfo& view :
       catalog::ListProjectViews(project_id, "")) {
    const std::string qualified =
        absl::StrCat(view.dataset_id, ".", view.view_name);
    const std::string fqn =
        absl::StrCat(project_id, ".", view.dataset_id, ".", view.view_name);
    CatalogTableEntry entry;
    entry.label = qualified;
    entry.fqn = fqn;
    entry.kind = "view";
    entry.detail = "view";
    AppendUniqueTable(names, entry);
    if (!default_dataset_id.empty() && view.dataset_id == default_dataset_id) {
      CatalogTableEntry unqualified;
      unqualified.label = view.view_name;
      unqualified.fqn = fqn;
      unqualified.kind = "view";
      unqualified.detail = "view";
      AppendUniqueTable(names, unqualified);
    }
  }

  return absl::OkStatus();
}

}  // namespace sqltools
}  // namespace backend
}  // namespace bigquery_emulator
