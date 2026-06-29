#include <cstddef>
#include <string>

#include "absl/strings/str_cat.h"
#include "absl/strings/string_view.h"
#include "backend/catalog/info_schema_internal.h"
#include "backend/catalog/info_schema_table.h"
#include "backend/schema/schema.h"

namespace bigquery_emulator {
namespace backend {
namespace catalog {
namespace info_schema_internal {

namespace {

using schema::ColumnMode;
using schema::ColumnSchema;
using schema::ColumnType;
using schema::TableSchema;

ColumnSchema Col(absl::string_view name,
                 ColumnType type,
                 ColumnMode mode = ColumnMode::kNullable) {
  ColumnSchema c;
  c.name = std::string(name);
  c.type = type;
  c.mode = mode;
  return c;
}

}  // namespace

std::string InfoSchemaDataType(const ColumnSchema& column) {
  if (column.mode == ColumnMode::kRepeated ||
      column.type == ColumnType::kArray) {
    if (column.fields.empty()) return "ARRAY<INT64>";
    return absl::StrCat("ARRAY<", InfoSchemaDataType(column.fields[0]), ">");
  }
  if (column.type == ColumnType::kStruct) {
    std::string out = "STRUCT<";
    for (size_t i = 0; i < column.fields.size(); ++i) {
      if (i > 0) out.append(", ");
      absl::StrAppend(&out,
                      column.fields[i].name,
                      " ",
                      InfoSchemaDataType(column.fields[i]));
    }
    out.push_back('>');
    return out;
  }
  switch (column.type) {
    case ColumnType::kInt64:
      return "INT64";
    case ColumnType::kFloat64:
      return "FLOAT64";
    case ColumnType::kBool:
      return "BOOL";
    case ColumnType::kString:
      return "STRING";
    case ColumnType::kBytes:
      return "BYTES";
    case ColumnType::kDate:
      return "DATE";
    case ColumnType::kTime:
      return "TIME";
    case ColumnType::kDatetime:
      return "DATETIME";
    case ColumnType::kTimestamp:
      return "TIMESTAMP";
    case ColumnType::kNumeric:
      return "NUMERIC";
    case ColumnType::kBignumeric:
      return "BIGNUMERIC";
    case ColumnType::kJson:
      return "JSON";
    case ColumnType::kGeography:
      return "GEOGRAPHY";
    default:
      return column.raw_type.empty() ? "STRING" : column.raw_type;
  }
}

TableSchema RowSchemaForView(InfoSchemaViewKind kind) {
  const ColumnType kStr = ColumnType::kString;
  const ColumnType kI64 = ColumnType::kInt64;
  const ColumnType kTs = ColumnType::kTimestamp;
  const ColumnType kBoolT = ColumnType::kBool;
  switch (kind) {
    case InfoSchemaViewKind::kTables:
      return TableSchema{.columns = {
                             Col("table_catalog", kStr),
                             Col("table_schema", kStr),
                             Col("table_name", kStr),
                             Col("table_type", kStr),
                         }};
    case InfoSchemaViewKind::kColumns:
      return TableSchema{.columns = {
                             Col("table_catalog", kStr),
                             Col("table_schema", kStr),
                             Col("table_name", kStr),
                             Col("column_name", kStr),
                             Col("ordinal_position", kI64),
                             Col("is_nullable", kStr),
                             Col("data_type", kStr),
                         }};
    case InfoSchemaViewKind::kSchemata:
      return TableSchema{.columns = {
                             Col("catalog_name", kStr),
                             Col("schema_name", kStr),
                         }};
    case InfoSchemaViewKind::kViews:
      return TableSchema{.columns = {
                             Col("table_catalog", kStr),
                             Col("table_schema", kStr),
                             Col("table_name", kStr),
                             Col("view_definition", kStr),
                             Col("check_option", kStr),
                             Col("use_standard_sql", kStr),
                         }};
    case InfoSchemaViewKind::kRoutines:
      return TableSchema{.columns = {
                             Col("specific_catalog", kStr),
                             Col("specific_schema", kStr),
                             Col("specific_name", kStr),
                             Col("routine_catalog", kStr),
                             Col("routine_schema", kStr),
                             Col("routine_name", kStr),
                             Col("routine_type", kStr),
                             Col("data_type", kStr),
                             Col("routine_body", kStr),
                             Col("routine_definition", kStr),
                             Col("external_language", kStr),
                             Col("is_deterministic", kStr),
                             Col("security_type", kStr),
                             Col("created", kTs),
                             Col("last_altered", kTs),
                             Col("ddl", kStr),
                             Col("connection", kStr),
                         }};
    case InfoSchemaViewKind::kTableOptions:
      return TableSchema{.columns = {
                             Col("table_catalog", kStr),
                             Col("table_schema", kStr),
                             Col("table_name", kStr),
                             Col("option_name", kStr),
                             Col("option_type", kStr),
                             Col("option_value", kStr),
                         }};
    case InfoSchemaViewKind::kColumnFieldPaths:
      return TableSchema{.columns = {
                             Col("table_catalog", kStr),
                             Col("table_schema", kStr),
                             Col("table_name", kStr),
                             Col("column_name", kStr),
                             Col("field_path", kStr),
                             Col("data_type", kStr),
                             Col("description", kStr),
                             Col("collation_name", kStr),
                             Col("rounding_mode", kStr),
                             Col("policy_tags", kStr, ColumnMode::kRepeated),
                         }};
    case InfoSchemaViewKind::kPartitions:
      return TableSchema{.columns = {
                             Col("table_catalog", kStr),
                             Col("table_schema", kStr),
                             Col("table_name", kStr),
                             Col("partition_id", kStr),
                             Col("total_rows", kI64),
                             Col("total_logical_bytes", kI64),
                             Col("total_billable_bytes", kI64),
                             Col("last_modified_time", kTs),
                             Col("storage_tier", kStr),
                         }};
    case InfoSchemaViewKind::kTableStorage:
      return TableSchema{.columns = {
                             Col("project_id", kStr),
                             Col("project_number", kI64),
                             Col("table_catalog", kStr),
                             Col("table_schema", kStr),
                             Col("table_name", kStr),
                             Col("creation_time", kTs),
                             Col("total_rows", kI64),
                             Col("total_partitions", kI64),
                             Col("total_logical_bytes", kI64),
                             Col("active_logical_bytes", kI64),
                             Col("long_term_logical_bytes", kI64),
                             Col("current_physical_bytes", kI64),
                             Col("total_physical_bytes", kI64),
                             Col("active_physical_bytes", kI64),
                             Col("long_term_physical_bytes", kI64),
                             Col("time_travel_physical_bytes", kI64),
                             Col("storage_last_modified_time", kTs),
                             Col("deleted", kBoolT),
                             Col("table_type", kStr),
                             Col("managed_table_type", kStr),
                             Col("fail_safe_physical_bytes", kI64),
                             Col("last_metadata_index_refresh_time", kTs),
                             Col("table_deletion_reason", kStr),
                             Col("table_deletion_time", kTs),
                         }};
    case InfoSchemaViewKind::kKeyColumnUsage:
      return TableSchema{.columns = {
                             Col("constraint_catalog", kStr),
                             Col("constraint_schema", kStr),
                             Col("constraint_name", kStr),
                             Col("table_catalog", kStr),
                             Col("table_schema", kStr),
                             Col("table_name", kStr),
                             Col("column_name", kStr),
                             Col("ordinal_position", kI64),
                             Col("position_in_unique_constraint", kI64),
                         }};
  }
  return TableSchema{};
}

}  // namespace info_schema_internal
}  // namespace catalog
}  // namespace backend
}  // namespace bigquery_emulator
