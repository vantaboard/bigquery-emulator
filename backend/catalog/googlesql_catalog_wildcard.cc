#include "backend/catalog/googlesql_catalog.h"

#include <algorithm>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "absl/status/statusor.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/string_view.h"
#include "backend/catalog/wildcard_table.h"
#include "backend/catalog/wildcard_table_util.h"
#include "backend/schema/schema.h"
#include "backend/storage/storage.h"
#include "googlesql/public/simple_table.h"
#include "googlesql/public/type.h"

namespace bigquery_emulator {
namespace backend {
namespace catalog {

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

  std::sort(matched.begin(),
            matched.end(),
            [](const storage::TableId& a, const storage::TableId& b) {
              return a.table_id < b.table_id;
            });

  absl::StatusOr<schema::TableSchema> union_schema =
      UnifyWildcardTableSchemas(storage_, matched);
  if (!union_schema.ok()) return union_schema.status();

  absl::StatusOr<std::vector<WildcardColumnMap>> column_maps =
      BuildWildcardColumnMaps(storage_, matched, *union_schema);
  if (!column_maps.ok()) return column_maps.status();

  std::vector<SimpleTable::NameAndType> columns;
  columns.reserve(union_schema->columns.size());
  for (const schema::ColumnSchema& column : union_schema->columns) {
    absl::StatusOr<const Type*> column_type =
        ToGoogleSqlType(column, type_factory_);
    if (!column_type.ok()) return column_type.status();
    columns.emplace_back(column.name, *column_type);
  }

  const std::string table_prefix = WildcardTablePrefix(wildcard_table_id);
  storage::TableId wildcard_id{std::string(project_id),
                               std::string(dataset_id),
                               std::string(wildcard_table_id)};
  const std::string full_name =
      absl::StrCat(project_id, ".", dataset_id, ".", wildcard_table_id);
  auto wildcard = std::make_unique<WildcardTable>(wildcard_table_id,
                                                  full_name,
                                                  wildcard_id,
                                                  table_prefix,
                                                  std::move(*column_maps),
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
