#ifndef BIGQUERY_EMULATOR_BACKEND_CATALOG_INFO_SCHEMA_TABLE_H_
#define BIGQUERY_EMULATOR_BACKEND_CATALOG_INFO_SCHEMA_TABLE_H_

#include <memory>
#include <string>
#include <vector>

#include "absl/status/statusor.h"
#include "absl/strings/string_view.h"
#include "backend/catalog/virtual_table.h"
#include "backend/schema/schema.h"
#include "backend/storage/storage.h"
#include "absl/types/span.h"
#include "googlesql/public/evaluator_table_iterator.h"
#include "googlesql/public/types/type_factory.h"

namespace bigquery_emulator {
namespace backend {
namespace catalog {

enum class InfoSchemaViewKind {
  kTables,
  kColumns,
  kSchemata,
};

// BigQuery INFORMATION_SCHEMA.* view materialized from `Storage` metadata.
class InfoSchemaTable : public VirtualCatalogTable {
 public:
  InfoSchemaTable(absl::string_view view_name,
                  absl::string_view full_name,
                  InfoSchemaViewKind kind,
                  absl::string_view project_id,
                  absl::string_view dataset_id,
                  const storage::Storage* storage,
                  ::googlesql::TypeFactory* type_factory);

  absl::StatusOr<std::unique_ptr<::googlesql::EvaluatorTableIterator>>
  CreateEvaluatorTableIterator(
      absl::Span<const int> column_idxs) const override;

  absl::Status MaterializeInDuckDB(
      ::duckdb_connection conn,
      const storage::Storage* storage,
      absl::string_view quoted_table_name) const override;

 private:
  absl::StatusOr<std::vector<storage::Row>> GenerateRows() const;

  InfoSchemaViewKind kind_;
  std::string project_id_;
  std::string dataset_id_;
  const storage::Storage* storage_;
  ::googlesql::TypeFactory* type_factory_;
  schema::TableSchema row_schema_;
};

}  // namespace catalog
}  // namespace backend
}  // namespace bigquery_emulator

#endif  // BIGQUERY_EMULATOR_BACKEND_CATALOG_INFO_SCHEMA_TABLE_H_
