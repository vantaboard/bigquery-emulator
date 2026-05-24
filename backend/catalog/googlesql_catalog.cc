#include "backend/catalog/googlesql_catalog.h"

#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/string_view.h"
#include "absl/synchronization/mutex.h"
#include "absl/types/span.h"
#include "backend/schema/schema.h"
#include "backend/storage/storage.h"
#include "googlesql/public/catalog.h"
#include "googlesql/public/simple_catalog.h"
#include "googlesql/public/type.h"
#include "googlesql/public/types/struct_type.h"
#include "googlesql/public/types/type_factory.h"

namespace bigquery_emulator {
namespace backend {
namespace catalog {

namespace {

using ::googlesql::SimpleColumn;
using ::googlesql::SimpleTable;
using ::googlesql::StructType;
using ::googlesql::Type;
using ::googlesql::TypeFactory;

// Translate a *scalar-or-struct* `schema::ColumnSchema` into a
// GoogleSQL `Type*` without consulting the cardinality. Wrapping in
// `ARRAY<T>` happens once at the top level so nested STRUCT fields
// inherit their own per-field cardinality.
absl::StatusOr<const Type*> ScalarOrStructType(
    const schema::ColumnSchema& column, TypeFactory* type_factory) {
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
        absl::StatusOr<const Type*> field_type =
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
      // schema::ColumnSchemaToDuckDBType rejects this shape too. We
      // surface it here instead of silently inventing an inner type.
      return absl::InvalidArgumentError(absl::StrCat(
          "column '", column.name,
          "' has type ARRAY but no nested element schema; use ColumnMode "
          "REPEATED on the inner type instead"));
    case schema::ColumnType::kGeography:
      return absl::InvalidArgumentError(absl::StrCat(
          "column '", column.name,
          "': GEOGRAPHY is not yet supported by the analyzer catalog"));
    case schema::ColumnType::kUnknown:
      return absl::InvalidArgumentError(absl::StrCat(
          "column '", column.name,
          "' has unknown type (raw '", column.raw_type,
          "'); the analyzer cannot resolve it"));
  }
  return absl::InvalidArgumentError(
      absl::StrCat("column '", column.name, "': unhandled ColumnType"));
}

}  // namespace

absl::StatusOr<const Type*> GoogleSqlCatalog::ToGoogleSqlType(
    const schema::ColumnSchema& column, TypeFactory* type_factory) {
  absl::StatusOr<const Type*> scalar = ScalarOrStructType(column, type_factory);
  if (!scalar.ok()) return scalar.status();

  if (column.mode == schema::ColumnMode::kRepeated) {
    const Type* array_type = nullptr;
    absl::Status s = type_factory->MakeArrayType(*scalar, &array_type);
    if (!s.ok()) return s;
    return array_type;
  }
  return *scalar;
}

GoogleSqlCatalog::GoogleSqlCatalog(absl::string_view project_id,
                                   storage::Storage* storage,
                                   TypeFactory* type_factory)
    : project_id_(project_id),
      storage_(storage),
      type_factory_(type_factory) {}

std::string GoogleSqlCatalog::CacheKey(absl::string_view project_id,
                                       absl::string_view dataset_id,
                                       absl::string_view table_id) {
  // Same `\x1f` (Unit Separator) trick the in-memory storage uses for
  // dataset keys -- guaranteed to never appear in a BigQuery
  // project/dataset/table id.
  return absl::StrCat(project_id, "\x1f", dataset_id, "\x1f", table_id);
}

absl::Status GoogleSqlCatalog::FindTable(
    const absl::Span<const std::string>& path,
    const ::googlesql::Table** table, const FindOptions& options) {
  if (table == nullptr) {
    return absl::InvalidArgumentError("FindTable: output pointer is null");
  }
  *table = nullptr;

  absl::string_view project_id;
  absl::string_view dataset_id;
  absl::string_view table_id;
  if (path.size() == 2) {
    project_id = project_id_;
    dataset_id = path[0];
    table_id = path[1];
  } else if (path.size() == 3) {
    project_id = path[0];
    dataset_id = path[1];
    table_id = path[2];
  } else {
    return absl::NotFoundError(absl::StrCat(
        "Table path must be <dataset>.<table> or "
        "<project>.<dataset>.<table>; got ", path.size(), " segments"));
  }

  absl::MutexLock lock(&mu_);
  absl::StatusOr<const ::googlesql::Table*> resolved =
      MaterializeTable(project_id, dataset_id, table_id);
  if (!resolved.ok()) return resolved.status();
  *table = *resolved;
  return absl::OkStatus();
}

absl::StatusOr<const ::googlesql::Table*> GoogleSqlCatalog::MaterializeTable(
    absl::string_view project_id, absl::string_view dataset_id,
    absl::string_view table_id) {
  const std::string key = CacheKey(project_id, dataset_id, table_id);
  for (std::vector<std::string>::size_type i = 0; i < keys_.size(); ++i) {
    if (keys_[i] == key) return tables_[i].get();
  }

  storage::TableId id{std::string(project_id), std::string(dataset_id),
                       std::string(table_id)};
  absl::StatusOr<schema::TableSchema> table_schema = storage_->GetSchema(id);
  if (!table_schema.ok()) return table_schema.status();

  std::vector<SimpleTable::NameAndType> columns;
  columns.reserve(table_schema->columns.size());
  for (const schema::ColumnSchema& column : table_schema->columns) {
    absl::StatusOr<const Type*> column_type =
        ToGoogleSqlType(column, type_factory_);
    if (!column_type.ok()) return column_type.status();
    columns.emplace_back(column.name, *column_type);
  }

  // SimpleTable's name is the unqualified table id; FullName() returns
  // the dotted path which is what GoogleSQL prints in error messages.
  auto simple_table = std::make_unique<SimpleTable>(table_id, columns);
  const ::googlesql::Table* raw = simple_table.get();
  tables_.push_back(std::move(simple_table));
  keys_.push_back(key);
  return raw;
}

}  // namespace catalog
}  // namespace backend
}  // namespace bigquery_emulator
