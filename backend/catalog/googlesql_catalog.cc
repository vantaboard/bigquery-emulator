

#include "absl/log/log.h"
#include "googlesql/public/builtin_function_options.h"
#include "googlesql/public/catalog.h"
#include "googlesql/public/language_options.h"
#include "googlesql/public/simple_catalog.h"
#include "googlesql/public/type.h"
#include "googlesql/public/types/type_factory.h"

namespace bigquery_emulator {
namespace backend {
namespace catalog {

namespace {

using ::googlesql::BuiltinFunctionOptions;
using ::googlesql::LanguageOptions;
using ::googlesql::SimpleColumn;
using ::googlesql::SimpleTable;
using ::googlesql::Type;
using ::googlesql::TypeFactory;

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
  RegisterEmulatorBuiltinFunctions(*this);
  RegisterEmulatorMlTvfStubs(*this);
  ReplayFunctionsIntoCatalog(project_id_, *this);
  // NOTE: views are intentionally NOT eagerly replayed into the catalog.
  // `SimpleCatalog::AddTable` keys tables by their bare name, but BigQuery
  // scopes view names to their dataset, so two datasets are free to define
  // a view with the same name (e.g. a per-tenant `profiles` view). Replaying
  // them all by bare name tripped a duplicate-key CHECK in AddTable and
  // aborted the engine. View references resolve lazily and dataset-scoped
  // through `FindProjectView` in `FindTable` below, which is the only path
  // the analyzer takes for this fully-overridden catalog.
  ReplayTvfsIntoCatalog(project_id_, *this);
  ReplayProceduresIntoCatalog(project_id_, *this);
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
  absl::StatusOr<ParsedFindTablePath> parsed_or =
      ParseFindTablePath(path, project_id_, default_dataset_id_);
  if (!parsed_or.ok()) return parsed_or.status();
  project_id = parsed_or->project_id;
  dataset_id = parsed_or->dataset_id;
  table_id = parsed_or->table_id;
  info_schema_view = parsed_or->info_schema_view;

  absl::ReleasableMutexLock lock(&mu_);
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

  const std::string key = CacheKey(project_id, dataset_id, table_id);
  for (std::vector<std::string>::size_type i = 0; i < keys_.size(); ++i) {
    if (keys_[i] == key) {
      *table = tables_[i].get();
      return absl::OkStatus();
    }
  }

  // Registered SQL views must win over persisted view sidecars. Sidecars
  // exist for REST listing and restart rehydration but carry an empty
  // schema; treating them as physical tables yields zero-column reads.
  const ::googlesql::Table* registered_view =
      FindProjectView(project_id, dataset_id, table_id);
  if (registered_view != nullptr) {
    registered_view_keys_.push_back(key);
    registered_views_.push_back(registered_view);
    *table = registered_view;
    return absl::OkStatus();
  }

  storage::TableId storage_id{
      std::string(project_id), std::string(dataset_id), std::string(table_id)};
  absl::StatusOr<schema::TableSchema> schema_or =
      storage_->GetSchema(storage_id);
  if (!schema_or.ok()) {
    return schema_or.status();
  }

  absl::StatusOr<MaterializedTableBuild> built =
      MaterializeTablePhysical(project_id, dataset_id, table_id);
  if (!built.ok()) return built.status();
  StorageTable* storage_table = built->table;
  schema::TableSchema logical_schema = std::move(built->logical_schema);

  lock.Release();

  absl::Status measures =
      ApplyMeasureColumnsFromSchema(*storage_table,
                                    logical_schema,
                                    *this,
                                    *type_factory_,
                                    MakeCatalogLanguageOptions(),
                                    measure_outputs_,
                                    measure_resolved_exprs_);
  if (!measures.ok()) {
    absl::MutexLock relock(&mu_);
    if (!keys_.empty() && keys_.back() == key) {
      keys_.pop_back();
      tables_.pop_back();
    }
    return measures;
  }

  *table = storage_table;
  return absl::OkStatus();
}

absl::Status GoogleSqlCatalog::FindFunction(
    const absl::Span<const std::string>& path,
    const ::googlesql::Function** function,
    const FindOptions& options) {
  if (function == nullptr) {
    return absl::InvalidArgumentError("FindFunction: output pointer is null");
  }
  *function = nullptr;
  absl::Status found = SimpleCatalog::FindFunction(path, function, options);
  if (found.ok() && *function != nullptr) return found;
  const ::googlesql::Function* registered =
      FindProjectFunctionFromPath(path, project_id_, default_dataset_id_);
  if (registered == nullptr) return found;
  *function = registered;
  return absl::OkStatus();
}

absl::Status GoogleSqlCatalog::FindModel(
    const absl::Span<const std::string>& path,
    const ::googlesql::Model** model,
    const FindOptions& options) {
  return ResolveMlStubModelForAnalysis(
      *this, type_factory_, path, model, options);
}

absl::Status GoogleSqlCatalog::FindTableValuedFunction(
    const absl::Span<const std::string>& path,
    const ::googlesql::TableValuedFunction** function,
    const FindOptions& options) {
  return FindTableValuedFunctionWithUnqualifiedFallback(
      *this, path, function, options);
}

absl::Status GoogleSqlCatalog::FindProcedure(
    const absl::Span<const std::string>& path,
    const ::googlesql::Procedure** procedure,
    const FindOptions& options) {
  if (procedure == nullptr) {
    return absl::InvalidArgumentError("FindProcedure: output pointer is null");
  }
  *procedure = nullptr;
  absl::Status found = SimpleCatalog::FindProcedure(path, procedure, options);
  if (found.ok() && *procedure != nullptr) return found;
  if (path.size() >= 2) {
    const std::vector<std::string> unqualified = {path.back()};
    return SimpleCatalog::FindProcedure(unqualified, procedure, options);
  }
  return found;
}

absl::StatusOr<GoogleSqlCatalog::MaterializedTableBuild>
GoogleSqlCatalog::MaterializeTablePhysical(absl::string_view project_id,
                                           absl::string_view dataset_id,
                                           absl::string_view table_id) {
  const std::string key = CacheKey(project_id, dataset_id, table_id);
  for (std::vector<std::string>::size_type i = 0; i < keys_.size(); ++i) {
    if (keys_[i] == key) {
      MaterializedTableBuild built;
      built.table = dynamic_cast<StorageTable*>(tables_[i].get());
      if (built.table == nullptr) {
        return absl::InternalError(absl::StrCat(
            "GoogleSqlCatalog: cached table is not a StorageTable: ", key));
      }
      return built;
    }
  }

  storage::TableId id{
      std::string(project_id), std::string(dataset_id), std::string(table_id)};
  absl::StatusOr<schema::TableSchema> table_schema = storage_->GetSchema(id);
  if (!table_schema.ok()) {
    const ::googlesql::Table* registered_view =
        FindProjectView(project_id, dataset_id, table_id);
    if (registered_view != nullptr) {
      registered_view_keys_.push_back(key);
      registered_views_.push_back(registered_view);
      return absl::NotFoundError(
          "MaterializeTablePhysical called for registered view");
    }
    return table_schema.status();
  }

  std::vector<SimpleTable::NameAndType> columns;
  schema::TableSchema physical_schema;
  absl::StatusOr<std::vector<SimpleTable::NameAndType>> physical_columns_or =
      BuildPhysicalNameAndTypes(*table_schema,
                                &physical_schema,
                                [this](const schema::ColumnSchema& column) {
                                  return ToGoogleSqlType(column, type_factory_);
                                });
  if (!physical_columns_or.ok()) return physical_columns_or.status();
  columns = std::move(*physical_columns_or);

  const std::string full_name =
      absl::StrCat(project_id, ".", dataset_id, ".", table_id);
  auto storage_table = std::make_unique<StorageTable>(
      table_id, full_name, columns, physical_schema, id, storage_);
  StorageTable* raw = storage_table.get();
  tables_.push_back(std::move(storage_table));
  keys_.push_back(key);

  MaterializedTableBuild built;
  built.table = raw;
  built.logical_schema = std::move(*table_schema);
  return built;
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
          ? absl::StrCat(project_id, ".", "INFORMATION_SCHEMA", ".", view_name)
          : absl::StrCat(project_id,
                         ".",
                         dataset_id,
                         ".",
                         "INFORMATION_SCHEMA",
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

}  // namespace catalog
}  // namespace backend
}  // namespace bigquery_emulator
