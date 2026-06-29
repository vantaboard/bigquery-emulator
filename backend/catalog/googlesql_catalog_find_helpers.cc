#include "backend/catalog/googlesql_catalog_find_helpers.h"

#include <cstddef>
#include <optional>
#include <string>
#include <vector>

#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/string_view.h"
#include "absl/types/span.h"
#include "backend/catalog/info_schema_table.h"
#include "backend/schema/schema.h"
#include "googlesql/public/types/struct_type.h"

namespace bigquery_emulator {
namespace backend {
namespace catalog {

namespace {

using ::googlesql::StructType;

constexpr absl::string_view kInformationSchema = "INFORMATION_SCHEMA";

}  // namespace

absl::StatusOr<ParsedFindTablePath> ParseFindTablePath(
    const absl::Span<const std::string>& path,
    absl::string_view project_id,
    absl::string_view default_dataset_id) {
  auto invalid_path_error = [&path]() -> absl::Status {
    return absl::NotFoundError(
        absl::StrCat("Table path must be <dataset>.<table> or "
                     "<project>.<dataset>.<table>; got ",
                     path.size(),
                     " segments"));
  };

  ParsedFindTablePath parsed;
  if (path.size() == 1) {
    absl::string_view single = path[0];
    const size_t first_dot = single.find('.');
    if (first_dot != absl::string_view::npos) {
      const size_t second_dot = single.find('.', first_dot + 1);
      if (second_dot != absl::string_view::npos) {
        // Backtick-quoted `project.dataset.table` references arrive as one
        // path segment; split into three parts instead of treating everything
        // after the first dot as the table id.
        parsed.project_id = single.substr(0, first_dot);
        parsed.dataset_id =
            single.substr(first_dot + 1, second_dot - first_dot - 1);
        parsed.table_id = single.substr(second_dot + 1);
      } else {
        parsed.project_id = project_id;
        parsed.dataset_id = single.substr(0, first_dot);
        parsed.table_id = single.substr(first_dot + 1);
      }
      return parsed;
    }
    if (default_dataset_id.empty()) return invalid_path_error();
    parsed.project_id = project_id;
    parsed.dataset_id = default_dataset_id;
    parsed.table_id = single;
    return parsed;
  }

  if (path.size() == 2) {
    parsed.project_id = project_id;
    if (path[0] == kInformationSchema) {
      parsed.info_schema_view = path[1];
    } else {
      parsed.dataset_id = path[0];
      parsed.table_id = path[1];
    }
    return parsed;
  }

  if (path.size() == 3) {
    if (path[1] == kInformationSchema) {
      parsed.project_id = project_id;
      parsed.dataset_id = path[0];
      parsed.info_schema_view = path[2];
    } else {
      parsed.project_id = path[0];
      parsed.dataset_id = path[1];
      parsed.table_id = path[2];
    }
    return parsed;
  }

  return invalid_path_error();
}

std::optional<InfoSchemaViewKind> ParseInfoSchemaView(
    absl::string_view view_name) {
  if (view_name == "TABLES") return InfoSchemaViewKind::kTables;
  if (view_name == "COLUMNS") return InfoSchemaViewKind::kColumns;
  if (view_name == "SCHEMATA") return InfoSchemaViewKind::kSchemata;
  if (view_name == "VIEWS") return InfoSchemaViewKind::kViews;
  if (view_name == "ROUTINES") return InfoSchemaViewKind::kRoutines;
  if (view_name == "TABLE_OPTIONS") return InfoSchemaViewKind::kTableOptions;
  if (view_name == "COLUMN_FIELD_PATHS") {
    return InfoSchemaViewKind::kColumnFieldPaths;
  }
  if (view_name == "PARTITIONS") return InfoSchemaViewKind::kPartitions;
  if (view_name == "TABLE_STORAGE" || view_name == "TABLE_STORAGE_BY_PROJECT") {
    return InfoSchemaViewKind::kTableStorage;
  }
  if (view_name == "KEY_COLUMN_USAGE") {
    return InfoSchemaViewKind::kKeyColumnUsage;
  }
  return std::nullopt;
}

absl::StatusOr<const ::googlesql::Type*> ScalarOrStructType(
    const schema::ColumnSchema& column,
    ::googlesql::TypeFactory* type_factory) {
  switch (column.type) {
    case schema::ColumnType::kBool:
      return type_factory->get_bool();
    case schema::ColumnType::kInt64:
      return type_factory->get_int64();
    case schema::ColumnType::kFloat64:
      return type_factory->get_double();
    case schema::ColumnType::kString:
      return type_factory->get_string();
    case schema::ColumnType::kBytes:
      return type_factory->get_bytes();
    case schema::ColumnType::kDate:
      return type_factory->get_date();
    case schema::ColumnType::kTime:
      return type_factory->get_time();
    case schema::ColumnType::kDatetime:
      return type_factory->get_datetime();
    case schema::ColumnType::kTimestamp:
      return type_factory->get_timestamp();
    case schema::ColumnType::kNumeric:
      return type_factory->get_numeric();
    case schema::ColumnType::kBignumeric:
      return type_factory->get_bignumeric();
    case schema::ColumnType::kJson:
      return type_factory->get_json();
    case schema::ColumnType::kStruct: {
      std::vector<StructType::StructField> fields;
      fields.reserve(column.fields.size());
      for (const schema::ColumnSchema& field : column.fields) {
        absl::StatusOr<const ::googlesql::Type*> field_type =
            GoogleSqlCatalog::ToGoogleSqlType(field, type_factory);
        if (!field_type.ok()) return field_type.status();
        fields.emplace_back(field.name, *field_type);
      }
      const StructType* struct_type = nullptr;
      absl::Status s = type_factory->MakeStructType(fields, &struct_type);
      if (!s.ok()) return s;
      return struct_type;
    }
    case schema::ColumnType::kArray:
      // ARRAY without a single nested element schema is malformed;
      // schema::ColumnSchemaToDuckDBType rejects this shape too. We surface it
      // here instead of silently inventing an inner type.
      return absl::InvalidArgumentError(absl::StrCat(
          "column '",
          column.name,
          "' has type ARRAY but no nested element schema; use ColumnMode "
          "REPEATED on the inner type instead"));
    case schema::ColumnType::kGeography:
      return type_factory->get_geography();
    case schema::ColumnType::kUnknown:
      return absl::InvalidArgumentError(
          absl::StrCat("column '",
                       column.name,
                       "' has unknown type (raw '",
                       column.raw_type,
                       "'); the analyzer cannot resolve it"));
  }
  return absl::InvalidArgumentError(
      absl::StrCat("column '", column.name, "': unhandled ColumnType"));
}

}  // namespace catalog
}  // namespace backend
}  // namespace bigquery_emulator
