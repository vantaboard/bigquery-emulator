// Row-generation arms for the INFORMATION_SCHEMA materializer. Split
// from info_schema_table.cc (which owns the descriptor + DuckDB
// materialization + iterator) to keep each translation unit focused.

namespace bigquery_emulator {
namespace backend {
namespace catalog {

namespace {

using schema::ColumnSchema;
using schema::ColumnType;

// storage::Value cell shorthands. INFORMATION_SCHEMA rows are
// hand-built so these keep the row literals readable.
storage::Value Str(absl::string_view v) {
  return storage::Value::String(std::string(v));
}
storage::Value Int(int64_t v) {
  return storage::Value::Int64(v);
}
storage::Value Bool(bool v) {
  return storage::Value::Bool(v);
}
storage::Value Null() {
  return storage::Value::Null();
}

bool IsRegionScope(absl::string_view dataset_id) {
  return absl::StartsWith(dataset_id, "region-");
}

absl::Status AppendRowsForTableKind(InfoSchemaViewKind kind,
                                    const storage::Storage* storage,
                                    absl::string_view project_id,
                                    const storage::TableId& table,
                                    std::vector<storage::Row>* rows) {
  switch (kind) {
    case InfoSchemaViewKind::kTables:
      rows->push_back(storage::Row{.cells = {Str(project_id),
                                             Str(table.dataset_id),
                                             Str(table.table_id),
                                             Str("BASE TABLE")}});
      return absl::OkStatus();
    case InfoSchemaViewKind::kColumns: {
      absl::StatusOr<schema::TableSchema> table_schema =
          storage->GetSchema(table);
      if (!table_schema.ok()) return table_schema.status();
      for (size_t i = 0; i < table_schema->columns.size(); ++i) {
        const ColumnSchema& column = table_schema->columns[i];
        auto nullable =
            column.mode == schema::ColumnMode::kRequired ? "NO" : "YES";
        rows->push_back(storage::Row{
            .cells = {Str(project_id),
                      Str(table.dataset_id),
                      Str(table.table_id),
                      Str(column.name),
                      Int(static_cast<int64_t>(i + 1)),
                      Str(nullable),
                      Str(info_schema_internal::InfoSchemaDataType(column))}});
      }
      return absl::OkStatus();
    }
    case InfoSchemaViewKind::kPartitions: {
      absl::StatusOr<std::int64_t> count = storage->CountRows(table);
      if (!count.ok()) return count.status();
      rows->push_back(
          storage::Row{.cells = {Str(project_id),
                                 Str(table.dataset_id),
                                 Str(table.table_id),
                                 Null(),  // partition_id: unpartitioned table
                                 Int(*count),
                                 Null(),  // total_logical_bytes: not tracked
                                 Null(),  // total_billable_bytes: not tracked
                                 Null(),  // last_modified_time: not tracked
                                 Str("ACTIVE")}});
      return absl::OkStatus();
    }
    case InfoSchemaViewKind::kTableStorage: {
      absl::StatusOr<std::int64_t> count = storage->CountRows(table);
      if (!count.ok()) return count.status();
      rows->push_back(
          storage::Row{.cells = {Str(project_id),  // project_id
                                 Null(),           // project_number
                                 Str(project_id),  // table_catalog
                                 Str(table.dataset_id),
                                 Str(table.table_id),
                                 Null(),       // creation_time
                                 Int(*count),  // total_rows
                                 Int(0),  // total_partitions (unpartitioned)
                                 Null(),  // total_logical_bytes
                                 Null(),  // active_logical_bytes
                                 Null(),  // long_term_logical_bytes
                                 Null(),  // current_physical_bytes
                                 Null(),  // total_physical_bytes
                                 Null(),  // active_physical_bytes
                                 Null(),  // long_term_physical_bytes
                                 Null(),  // time_travel_physical_bytes
                                 Null(),  // storage_last_modified_time
                                 Bool(false),  // deleted
                                 Str("BASE TABLE"),
                                 Str("NATIVE"),
                                 Null(),     // fail_safe_physical_bytes
                                 Null(),     // last_metadata_index_refresh_time
                                 Null(),     // table_deletion_reason
                                 Null()}});  // table_deletion_time
      return absl::OkStatus();
    }
    default:
      return absl::OkStatus();
  }
}

}  // namespace

absl::StatusOr<std::vector<storage::Row>> InfoSchemaTable::GenerateRows()
    const {
  std::vector<storage::Row> rows;
  if (storage_ == nullptr) {
    return absl::FailedPreconditionError(
        "InfoSchemaTable: storage backend is not configured");
  }

  // Datasets the view scans. Dataset-qualified views (`<ds>.INFO...`)
  // scope to that dataset; project-scoped / region-qualified
  // (`region-*.INFO...`) views enumerate every dataset in the project.
  auto datasets_in_scope = [&]() -> absl::StatusOr<std::vector<std::string>> {
    std::vector<std::string> out;
    if (!dataset_id_.empty() && !IsRegionScope(dataset_id_)) {
      out.push_back(dataset_id_);
      return out;
    }
    absl::StatusOr<std::vector<storage::DatasetId>> datasets =
        storage_->ListDatasets(project_id_);
    if (!datasets.ok()) return datasets.status();
    for (const storage::DatasetId& ds : *datasets) {
      out.push_back(ds.dataset_id);
    }
    return out;
  };

  switch (kind_) {
    case InfoSchemaViewKind::kSchemata: {
      absl::StatusOr<std::vector<storage::DatasetId>> datasets =
          storage_->ListDatasets(project_id_);
      if (!datasets.ok()) return datasets.status();
      for (const storage::DatasetId& ds : *datasets) {
        rows.push_back(
            storage::Row{.cells = {Str(project_id_), Str(ds.dataset_id)}});
      }
      return rows;
    }
    case InfoSchemaViewKind::kTables:
    case InfoSchemaViewKind::kColumns:
    case InfoSchemaViewKind::kColumnFieldPaths:
    case InfoSchemaViewKind::kPartitions:
    case InfoSchemaViewKind::kTableStorage:
    case InfoSchemaViewKind::kTableOptions:
    case InfoSchemaViewKind::kKeyColumnUsage: {
      absl::StatusOr<std::vector<std::string>> datasets = datasets_in_scope();
      if (!datasets.ok()) return datasets.status();
      for (const std::string& ds : *datasets) {
        absl::Status s = AppendTableRows(ds, &rows);
        if (!s.ok()) return s;
      }
      return rows;
    }
    case InfoSchemaViewKind::kViews: {
      std::vector<RegisteredViewInfo> views =
          ListProjectViews(project_id_, dataset_id_);
      for (const RegisteredViewInfo& view : views) {
        rows.push_back(storage::Row{
            .cells = {Str(project_id_),
                      Str(view.dataset_id),
                      Str(view.view_name),
                      Str(view.view_definition),
                      Null(),  // check_option: always NULL per BigQuery
                      Str(view.use_standard_sql ? "YES" : "NO")}});
      }
      return rows;
    }
    case InfoSchemaViewKind::kRoutines:
      return GenerateRoutineRows();
  }
  return rows;
}

absl::Status InfoSchemaTable::AppendTableRows(
    absl::string_view dataset_id, std::vector<storage::Row>* rows) const {
  storage::DatasetId ds_id{project_id_, std::string(dataset_id)};
  absl::StatusOr<std::vector<storage::TableId>> tables =
      storage_->ListTables(ds_id);
  if (!tables.ok()) return tables.status();

  // KEY_COLUMN_USAGE / TABLE_OPTIONS have no engine-side source: the
  // emulator tracks neither primary/foreign keys nor table options, so
  // these views are correctly empty (BigQuery emits no rows for a
  // table without constraints / options either). See plan notes.
  if (kind_ == InfoSchemaViewKind::kKeyColumnUsage ||
      kind_ == InfoSchemaViewKind::kTableOptions) {
    return absl::OkStatus();
  }

  for (const storage::TableId& table : *tables) {
    if (kind_ == InfoSchemaViewKind::kColumnFieldPaths) {
      absl::StatusOr<schema::TableSchema> table_schema =
          storage_->GetSchema(table);
      if (!table_schema.ok()) return table_schema.status();
      for (const ColumnSchema& column : table_schema->columns) {
        AppendFieldPathRows(table, column, column.name, column.name, rows);
      }
      continue;
    }
    absl::Status append_status =
        AppendRowsForTableKind(kind_, storage_, project_id_, table, rows);
    if (!append_status.ok()) return append_status;
  }
  return absl::OkStatus();
}

void InfoSchemaTable::AppendFieldPathRows(
    const storage::TableId& table,
    const schema::ColumnSchema& column,
    absl::string_view top_level_name,
    absl::string_view field_path,
    std::vector<storage::Row>* rows) const {
  storage::Value description =
      column.description.empty() ? Null() : Str(column.description);
  rows->push_back(storage::Row{
      .cells = {Str(project_id_),
                Str(table.dataset_id),
                Str(table.table_id),
                Str(top_level_name),
                Str(field_path),
                Str(info_schema_internal::InfoSchemaDataType(column)),
                std::move(description),
                Null(),     // collation_name
                Null(),     // rounding_mode
                Null()}});  // policy_tags (ARRAY<STRING>)

  // Recurse into STRUCT fields (including ARRAY<STRUCT> element fields).
  const std::vector<schema::ColumnSchema>* sub_fields = nullptr;
  if (column.type == ColumnType::kStruct) {
    sub_fields = &column.fields;
  } else if (column.type == ColumnType::kArray && !column.fields.empty() &&
             column.fields[0].type == ColumnType::kStruct) {
    sub_fields = &column.fields[0].fields;
  }
  if (sub_fields == nullptr) return;
  for (const schema::ColumnSchema& field : *sub_fields) {
    AppendFieldPathRows(table,
                        field,
                        top_level_name,
                        absl::StrCat(field_path, ".", field.name),
                        rows);
  }
}

absl::StatusOr<std::vector<storage::Row>> InfoSchemaTable::GenerateRoutineRows()
    const {
  std::vector<storage::Row> rows;
  std::vector<storage::RoutineRecord> records;
  if (!dataset_id_.empty() && !IsRegionScope(dataset_id_)) {
    absl::StatusOr<std::vector<storage::RoutineRecord>> listed =
        storage_->ListRoutines(storage::DatasetId{project_id_, dataset_id_});
    if (!listed.ok()) return listed.status();
    records = std::move(*listed);
  } else {
    absl::StatusOr<std::vector<storage::RoutineRecord>> listed =
        storage_->ListAllRoutines();
    if (!listed.ok()) return listed.status();
    for (storage::RoutineRecord& record : *listed) {
      if (record.id.project_id == project_id_) {
        records.push_back(std::move(record));
      }
    }
  }

  for (const storage::RoutineRecord& record : records) {
    if (record.is_temp) continue;  // temp routines are session-only
    std::string routine_type;
    switch (record.kind) {
      case storage::RoutineKind::kScalarFunction:
        routine_type = "FUNCTION";
        break;
      case storage::RoutineKind::kAggregateFunction:
        routine_type = "AGGREGATE FUNCTION";
        break;
      case storage::RoutineKind::kTableValuedFunction:
        routine_type = "TABLE FUNCTION";
        break;
      case storage::RoutineKind::kProcedure:
        routine_type = "PROCEDURE";
        break;
    }
    const std::string lang = absl::AsciiStrToLower(record.language);
    const bool external = lang == "js" || lang == "javascript";
    rows.push_back(
        storage::Row{.cells = {Str(record.id.project_id),
                               Str(record.id.dataset_id),
                               Str(record.id.routine_id),
                               Str(record.id.project_id),
                               Str(record.id.dataset_id),
                               Str(record.id.routine_id),
                               Str(routine_type),
                               Null(),  // data_type: return type not tracked
                               Str(external ? "EXTERNAL" : "SQL"),
                               Str(record.ddl_sql),  // routine_definition
                               external ? Str("JAVASCRIPT") : Null(),
                               Null(),               // is_deterministic
                               Null(),               // security_type
                               Null(),               // created
                               Null(),               // last_altered
                               Str(record.ddl_sql),  // ddl
                               Null()}});            // connection
  }
  return rows;
}

}  // namespace catalog
}  // namespace backend
}  // namespace bigquery_emulator
