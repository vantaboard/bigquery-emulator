#include "backend/catalog/googlesql_catalog.h"

#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "absl/log/log.h"
#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "absl/strings/match.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/string_view.h"
#include "absl/synchronization/mutex.h"
#include "absl/types/span.h"
#include "backend/catalog/info_schema_table.h"
#include "backend/catalog/storage_table.h"
#include "backend/catalog/udf_registry.h"
#include "backend/catalog/wildcard_table.h"
#include "backend/schema/schema.h"
#include "backend/storage/storage.h"
#include "googlesql/public/builtin_function_options.h"
#include "googlesql/public/catalog.h"
#include "googlesql/public/language_options.h"
#include "googlesql/public/simple_catalog.h"
#include "googlesql/public/type.h"
#include "googlesql/public/types/struct_type.h"
#include "googlesql/public/types/type_factory.h"

namespace bigquery_emulator {
namespace backend {
namespace catalog {

namespace {

using ::googlesql::BuiltinFunctionOptions;
using ::googlesql::LanguageOptions;
using ::googlesql::SimpleColumn;
using ::googlesql::SimpleTable;
using ::googlesql::StructType;
using ::googlesql::Type;
using ::googlesql::TypeFactory;

constexpr absl::string_view kInformationSchema = "INFORMATION_SCHEMA";

bool IsWildcardTableId(absl::string_view table_id) {
  return !table_id.empty() && table_id.back() == '*';
}

std::optional<InfoSchemaViewKind> ParseInfoSchemaView(
    absl::string_view view_name) {
  if (view_name == "TABLES") return InfoSchemaViewKind::kTables;
  if (view_name == "COLUMNS") return InfoSchemaViewKind::kColumns;
  if (view_name == "SCHEMATA") return InfoSchemaViewKind::kSchemata;
  return std::nullopt;
}

bool TableMatchesWildcard(absl::string_view table_id,
                          absl::string_view wildcard_table_id) {
  if (!IsWildcardTableId(wildcard_table_id)) return false;
  const absl::string_view prefix =
      wildcard_table_id.substr(0, wildcard_table_id.size() - 1);
  return absl::StartsWith(table_id, prefix);
}

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
          "column '",
          column.name,
          "' has type ARRAY but no nested element schema; use ColumnMode "
          "REPEATED on the inner type instead"));
    case schema::ColumnType::kGeography:
      return absl::InvalidArgumentError(absl::StrCat(
          "column '",
          column.name,
          "': GEOGRAPHY is not yet supported by the analyzer catalog"));
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
                                   TypeFactory* type_factory,
                                   const LanguageOptions& language,
                                   absl::string_view default_dataset_id)
    : ::googlesql::SimpleCatalog(std::string(project_id), type_factory),
      project_id_(project_id),
      default_dataset_id_(default_dataset_id),
      storage_(storage),
      type_factory_(type_factory) {
  // Register every GoogleSQL builtin function and type that `language`
  // enables on this catalog. The analyzer's name resolution falls
  // back to the catalog for non-operator function calls (`COUNT`,
  // `SUM`, ...) so the resolved AST the DuckDB transpiler walks
  // carries the right `googlesql::Function*` for each call.
  //
  // This registration is per-catalog (i.e., per-query in our
  // usage). The underlying built-in tables are populated lazily once
  // per process and then cached; the per-call cost is the
  // catalog-side hash-map insertions for the names we want to expose.
  absl::Status s =
      AddBuiltinFunctionsAndTypes(BuiltinFunctionOptions(language));
  if (!s.ok()) {
    // The only documented failure mode is a programmer error (e.g.
    // the same name added twice); log so the per-RPC analyzer error
    // is actionable but do not raise -- the catalog still works,
    // just without the failing builtin entry.
    LOG(ERROR) << "GoogleSqlCatalog: AddBuiltinFunctionsAndTypes failed: " << s;
  }
  ReplayFunctionsIntoCatalog(project_id_, *this);
}

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
    const ::googlesql::Table** table,
    const FindOptions& options) {
  if (table == nullptr) {
    return absl::InvalidArgumentError("FindTable: output pointer is null");
  }
  *table = nullptr;

  absl::string_view project_id;
  absl::string_view dataset_id;
  absl::string_view table_id;
  absl::string_view info_schema_view;

  if (path.size() == 1) {
    absl::string_view single = path[0];
    const size_t dot = single.find('.');
    if (dot != absl::string_view::npos) {
      project_id = project_id_;
      dataset_id = single.substr(0, dot);
      table_id = single.substr(dot + 1);
    } else if (default_dataset_id_.empty()) {
      return absl::NotFoundError(
          absl::StrCat("Table path must be <dataset>.<table> or "
                       "<project>.<dataset>.<table>; got ",
                       path.size(),
                       " segments"));
    } else {
      project_id = project_id_;
      dataset_id = default_dataset_id_;
      table_id = single;
    }
  } else if (path.size() == 2) {
    if (path[0] == kInformationSchema) {
      project_id = project_id_;
      info_schema_view = path[1];
    } else {
      project_id = project_id_;
      dataset_id = path[0];
      table_id = path[1];
    }
  } else if (path.size() == 3) {
    if (path[1] == kInformationSchema) {
      project_id = project_id_;
      dataset_id = path[0];
      info_schema_view = path[2];
    } else {
      project_id = path[0];
      dataset_id = path[1];
      table_id = path[2];
    }
  } else {
    return absl::NotFoundError(
        absl::StrCat("Table path must be <dataset>.<table> or "
                     "<project>.<dataset>.<table>; got ",
                     path.size(),
                     " segments"));
  }

  absl::MutexLock lock(&mu_);
  if (!info_schema_view.empty()) {
    absl::StatusOr<const ::googlesql::Table*> resolved =
        MaterializeInfoSchemaView(project_id, dataset_id, info_schema_view);
    if (!resolved.ok()) return resolved.status();
    *table = *resolved;
    return absl::OkStatus();
  }
  if (IsWildcardTableId(table_id)) {
    absl::StatusOr<const ::googlesql::Table*> resolved =
        MaterializeWildcardTable(project_id, dataset_id, table_id);
    if (!resolved.ok()) return resolved.status();
    *table = *resolved;
    return absl::OkStatus();
  }
  absl::StatusOr<const ::googlesql::Table*> resolved =
      MaterializeTable(project_id, dataset_id, table_id);
  if (!resolved.ok()) return resolved.status();
  *table = *resolved;
  return absl::OkStatus();
}

absl::StatusOr<const ::googlesql::Table*> GoogleSqlCatalog::MaterializeTable(
    absl::string_view project_id,
    absl::string_view dataset_id,
    absl::string_view table_id) {
  const std::string key = CacheKey(project_id, dataset_id, table_id);
  for (std::vector<std::string>::size_type i = 0; i < keys_.size(); ++i) {
    if (keys_[i] == key) return tables_[i].get();
  }

  storage::TableId id{
      std::string(project_id), std::string(dataset_id), std::string(table_id)};
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

  // The fully-qualified `project.dataset.table` path is what
  // GoogleSQL prints in error messages; SimpleTable's
  // `set_full_name` carries that through.
  const std::string full_name =
      absl::StrCat(project_id, ".", dataset_id, ".", table_id);
  auto storage_table = std::make_unique<StorageTable>(
      table_id, full_name, columns, *table_schema, id, storage_);
  const ::googlesql::Table* raw = storage_table.get();
  tables_.push_back(std::move(storage_table));
  keys_.push_back(key);
  return raw;
}

absl::StatusOr<const ::googlesql::Table*>
GoogleSqlCatalog::MaterializeInfoSchemaView(absl::string_view project_id,
                                            absl::string_view dataset_id,
                                            absl::string_view view_name) {
  const std::optional<InfoSchemaViewKind> kind = ParseInfoSchemaView(view_name);
  if (!kind.has_value()) {
    return absl::NotFoundError(absl::StrCat(
        "INFORMATION_SCHEMA view '", view_name, "' is not supported"));
  }
  const std::string key = CacheKey(project_id, dataset_id, view_name);
  for (std::vector<std::string>::size_type i = 0; i < keys_.size(); ++i) {
    if (keys_[i] == key) return tables_[i].get();
  }
  const std::string full_name =
      dataset_id.empty()
          ? absl::StrCat(project_id, ".", kInformationSchema, ".", view_name)
          : absl::StrCat(project_id,
                         ".",
                         dataset_id,
                         ".",
                         kInformationSchema,
                         ".",
                         view_name);
  auto info_table = std::make_unique<InfoSchemaTable>(view_name,
                                                      full_name,
                                                      *kind,
                                                      project_id,
                                                      dataset_id,
                                                      storage_,
                                                      type_factory_);
  const ::googlesql::Table* raw = info_table.get();
  tables_.push_back(std::move(info_table));
  keys_.push_back(key);
  return raw;
}

absl::StatusOr<const ::googlesql::Table*>
GoogleSqlCatalog::MaterializeWildcardTable(
    absl::string_view project_id,
    absl::string_view dataset_id,
    absl::string_view wildcard_table_id) {
  const std::string key = CacheKey(project_id, dataset_id, wildcard_table_id);
  for (std::vector<std::string>::size_type i = 0; i < keys_.size(); ++i) {
    if (keys_[i] == key) return tables_[i].get();
  }

  storage::DatasetId ds_id{std::string(project_id), std::string(dataset_id)};
  absl::StatusOr<std::vector<storage::TableId>> listed =
      storage_->ListTables(ds_id);
  if (!listed.ok()) return listed.status();

  std::vector<storage::TableId> matched;
  matched.reserve(listed->size());
  for (const storage::TableId& id : *listed) {
    if (TableMatchesWildcard(id.table_id, wildcard_table_id)) {
      matched.push_back(id);
    }
  }
  if (matched.empty()) {
    return absl::NotFoundError(absl::StrCat("No tables match wildcard '",
                                            wildcard_table_id,
                                            "' in dataset ",
                                            dataset_id));
  }

  absl::StatusOr<schema::TableSchema> union_schema =
      storage_->GetSchema(matched.front());
  if (!union_schema.ok()) return union_schema.status();

  std::vector<SimpleTable::NameAndType> columns;
  columns.reserve(union_schema->columns.size());
  for (const schema::ColumnSchema& column : union_schema->columns) {
    absl::StatusOr<const Type*> column_type =
        ToGoogleSqlType(column, type_factory_);
    if (!column_type.ok()) return column_type.status();
    columns.emplace_back(column.name, *column_type);
  }

  storage::TableId wildcard_id{std::string(project_id),
                               std::string(dataset_id),
                               std::string(wildcard_table_id)};
  const std::string full_name =
      absl::StrCat(project_id, ".", dataset_id, ".", wildcard_table_id);
  auto wildcard = std::make_unique<WildcardTable>(wildcard_table_id,
                                                  full_name,
                                                  wildcard_id,
                                                  std::move(matched),
                                                  std::move(*union_schema),
                                                  columns,
                                                  storage_,
                                                  type_factory_);
  const ::googlesql::Table* raw = wildcard.get();
  tables_.push_back(std::move(wildcard));
  keys_.push_back(key);
  return raw;
}

}  // namespace catalog
}  // namespace backend
}  // namespace bigquery_emulator
