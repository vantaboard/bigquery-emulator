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

std::string RoutineKindLabel(storage::RoutineKind kind) {
  switch (kind) {
    case storage::RoutineKind::kScalarFunction:
      return "scalar function";
    case storage::RoutineKind::kAggregateFunction:
      return "aggregate function";
    case storage::RoutineKind::kTableValuedFunction:
      return "table function";
    case storage::RoutineKind::kProcedure:
      return "procedure";
  }
  return "routine";
}

std::string RoutineKindWire(storage::RoutineKind kind) {
  switch (kind) {
    case storage::RoutineKind::kProcedure:
      return "procedure";
    default:
      return "routine";
  }
}

std::string RoutineDetail(const storage::RoutineRecord& routine) {
  return absl::StrCat(routine.language, " ", RoutineKindLabel(routine.kind));
}

void AppendUniqueDataset(CatalogNames* names, absl::string_view dataset_id) {
  for (const std::string& existing : names->datasets) {
    if (existing == dataset_id) return;
  }
  names->datasets.push_back(std::string(dataset_id));
}

void AppendTableWithAliases(CatalogNames* names,
                            CatalogTableEntry entry,
                            absl::string_view project_id,
                            absl::string_view dataset_id,
                            absl::string_view table_id,
                            absl::string_view default_dataset_id) {
  const std::string qualified = absl::StrCat(dataset_id, ".", table_id);
  const std::string fqn =
      absl::StrCat(project_id, ".", dataset_id, ".", table_id);
  entry.label = qualified;
  entry.fqn = fqn;
  AppendUniqueTable(names, entry);
  CatalogTableEntry fqn_entry = entry;
  fqn_entry.label = fqn;
  AppendUniqueTable(names, std::move(fqn_entry));
  if (!default_dataset_id.empty() && dataset_id == default_dataset_id) {
    CatalogTableEntry unqualified;
    unqualified.label = table_id;
    unqualified.fqn = fqn;
    unqualified.kind = entry.kind;
    unqualified.detail = entry.detail;
    AppendUniqueTable(names, unqualified);
  }
}

void AppendRoutineWithAliases(CatalogNames* names,
                              CatalogRoutineEntry entry,
                              absl::string_view project_id,
                              absl::string_view dataset_id,
                              absl::string_view routine_id,
                              absl::string_view default_dataset_id) {
  const std::string qualified = absl::StrCat(dataset_id, ".", routine_id);
  const std::string fqn =
      absl::StrCat(project_id, ".", dataset_id, ".", routine_id);
  entry.label = qualified;
  entry.fqn = fqn;
  AppendUniqueRoutine(names, entry);
  CatalogRoutineEntry fqn_entry = entry;
  fqn_entry.label = fqn;
  AppendUniqueRoutine(names, std::move(fqn_entry));
  if (!default_dataset_id.empty() && dataset_id == default_dataset_id) {
    CatalogRoutineEntry unqualified;
    unqualified.label = routine_id;
    unqualified.fqn = fqn;
    unqualified.kind = entry.kind;
    unqualified.detail = entry.detail;
    AppendUniqueRoutine(names, unqualified);
  }
}

absl::Status PopulateDatasetTables(
    CatalogNames* names,
    storage::Storage* storage,
    absl::string_view project_id,
    absl::string_view dataset_id,
    absl::string_view default_dataset_id,
    const absl::flat_hash_set<std::string>& view_keys) {
  absl::StatusOr<std::vector<storage::TableId>> tables =
      storage->ListTables(storage::DatasetId{project_id, dataset_id});
  if (!tables.ok()) {
    return tables.status();
  }
  for (const storage::TableId& table : *tables) {
    const std::string qualified = absl::StrCat(dataset_id, ".", table.table_id);
    const bool is_view = view_keys.contains(qualified);
    CatalogTableEntry entry;
    entry.kind = is_view ? "view" : "table";
    entry.detail = is_view ? "view" : "table";
    AppendTableWithAliases(names,
                           entry,
                           project_id,
                           dataset_id,
                           table.table_id,
                           default_dataset_id);

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
    const std::string fqn =
        absl::StrCat(project_id, ".", dataset_id, ".", table.table_id);
    names->columns_by_table[qualified] = columns;
    names->columns_by_table[fqn] = columns;
    if (!default_dataset_id.empty() && dataset_id == default_dataset_id) {
      names->columns_by_table[table.table_id] = columns;
    }
  }
  return absl::OkStatus();
}

absl::Status PopulateDatasetRoutines(CatalogNames* names,
                                     storage::Storage* storage,
                                     absl::string_view project_id,
                                     absl::string_view dataset_id,
                                     absl::string_view default_dataset_id) {
  absl::StatusOr<std::vector<storage::RoutineRecord>> routines =
      storage->ListRoutines(storage::DatasetId{project_id, dataset_id});
  if (!routines.ok()) {
    return routines.status();
  }
  for (const storage::RoutineRecord& routine : *routines) {
    CatalogRoutineEntry entry;
    entry.kind = RoutineKindWire(routine.kind);
    entry.detail = RoutineDetail(routine);
    AppendRoutineWithAliases(names,
                             entry,
                             project_id,
                             dataset_id,
                             routine.id.routine_id,
                             default_dataset_id);
  }
  return absl::OkStatus();
}

void AppendRegisteredViewEntries(CatalogNames* names,
                                 absl::string_view project_id,
                                 absl::string_view default_dataset_id) {
  for (const catalog::RegisteredViewInfo& view :
       catalog::ListProjectViews(project_id, "")) {
    CatalogTableEntry entry;
    entry.kind = "view";
    entry.detail = "view";
    AppendTableWithAliases(names,
                           entry,
                           project_id,
                           view.dataset_id,
                           view.view_name,
                           default_dataset_id);
  }
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
    AppendUniqueDataset(names, dataset.dataset_id);
    AppendUniqueDataset(names,
                        absl::StrCat(project_id, ".", dataset.dataset_id));
    if (absl::Status tables = PopulateDatasetTables(names,
                                                    storage,
                                                    project_id,
                                                    dataset.dataset_id,
                                                    default_dataset_id,
                                                    view_keys);
        !tables.ok()) {
      return tables;
    }
    if (absl::Status routines = PopulateDatasetRoutines(
            names, storage, project_id, dataset.dataset_id, default_dataset_id);
        !routines.ok()) {
      return routines;
    }
  }

  AppendRegisteredViewEntries(names, project_id, default_dataset_id);

  return absl::OkStatus();
}

}  // namespace sqltools
}  // namespace backend
}  // namespace bigquery_emulator
