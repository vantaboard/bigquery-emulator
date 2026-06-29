#ifndef BIGQUERY_EMULATOR_BACKEND_CATALOG_WILDCARD_TABLE_H_
#define BIGQUERY_EMULATOR_BACKEND_CATALOG_WILDCARD_TABLE_H_

#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "absl/status/statusor.h"
#include "absl/strings/string_view.h"
#include "backend/catalog/virtual_table.h"
#include "backend/catalog/wildcard_table_util.h"
#include "backend/schema/schema.h"
#include "backend/storage/storage.h"
#include "googlesql/public/evaluator_table_iterator.h"
#include "googlesql/public/types/type_factory.h"

namespace bigquery_emulator {
namespace backend {
namespace catalog {

// BigQuery wildcard table (`dataset.prefix_*`) backed by a UNION ALL of
// every matching physical table in the dataset.
class WildcardTable : public VirtualCatalogTable {
 public:
  WildcardTable(absl::string_view wildcard_table_id,
                absl::string_view full_name,
                storage::TableId wildcard_id,
                std::string table_prefix,
                std::vector<WildcardColumnMap> matched_tables,
                schema::TableSchema union_schema,
                absl::Span<const NameAndType> columns,
                const storage::Storage* storage,
                ::googlesql::TypeFactory* type_factory);

  absl::StatusOr<std::unique_ptr<::googlesql::EvaluatorTableIterator>>
  CreateEvaluatorTableIterator(
      absl::Span<const int> column_idxs) const override;

  absl::Status MaterializeInDuckDB(
      ::duckdb_connection conn,
      const storage::Storage* storage,
      absl::string_view quoted_table_name) const override;

  // Same as MaterializeInDuckDB but prunes the matched-table set when
  // `suffix_allowlist` is present (constant `_TABLE_SUFFIX` predicate).
  absl::Status MaterializeInDuckDBWithSuffixAllowList(
      ::duckdb_connection conn,
      const storage::Storage* storage,
      absl::string_view quoted_table_name,
      const std::optional<std::vector<std::string>>& suffix_allowlist) const;

 private:
  absl::StatusOr<std::vector<storage::Row>> CollectUnionRows(
      const std::optional<std::vector<std::string>>& suffix_allowlist) const;

  storage::TableId wildcard_id_;
  std::string table_prefix_;
  std::vector<WildcardColumnMap> matched_tables_{};
  schema::TableSchema union_schema_{};
  const storage::Storage* storage_ = nullptr;
  ::googlesql::TypeFactory* type_factory_ = nullptr;
};

}  // namespace catalog
}  // namespace backend
}  // namespace bigquery_emulator

#endif  // BIGQUERY_EMULATOR_BACKEND_CATALOG_WILDCARD_TABLE_H_
