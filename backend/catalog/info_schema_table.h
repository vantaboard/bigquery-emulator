#ifndef BIGQUERY_EMULATOR_BACKEND_CATALOG_INFO_SCHEMA_TABLE_H_
#define BIGQUERY_EMULATOR_BACKEND_CATALOG_INFO_SCHEMA_TABLE_H_

#include <memory>
#include <string>
#include <vector>

#include "absl/status/statusor.h"
#include "absl/strings/string_view.h"
#include "absl/types/span.h"
#include "backend/catalog/virtual_table.h"
#include "backend/schema/schema.h"
#include "backend/storage/storage.h"
#include "googlesql/public/evaluator_table_iterator.h"
#include "googlesql/public/types/type_factory.h"

namespace bigquery_emulator {
namespace backend {
namespace catalog {

// The set of INFORMATION_SCHEMA views the emulator materializes. Each
// value pairs with a row-schema descriptor + a `GenerateRows()` arm in
// info_schema_table.cc; the materializer (`MaterializeInDuckDB` /
// `CreateEvaluatorTableIterator`) is generic over the kind. Add a new
// view by extending this enum, `RowSchemaForView`, and `GenerateRows`.
//
// JOBS / JOBS_BY_PROJECT are intentionally absent: BigQuery job state
// lives in the Go gateway's job store, not the C++ engine's `Storage`,
// so those views are unresolved here (NOT_FOUND) rather than faked.
// See full-01 plan notes for the deferral rationale.
enum class InfoSchemaViewKind {
  kTables,
  kColumns,
  kSchemata,
  kViews,
  kRoutines,
  kTableOptions,
  kColumnFieldPaths,
  kPartitions,
  kTableStorage,
  kKeyColumnUsage,
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

  // Appends the per-table rows for the table-scoped views (TABLES,
  // COLUMNS, COLUMN_FIELD_PATHS, PARTITIONS, TABLE_STORAGE) for every
  // table in `dataset_id`. KEY_COLUMN_USAGE / TABLE_OPTIONS append
  // nothing (no engine-side metadata source).
  absl::Status AppendTableRows(absl::string_view dataset_id,
                               std::vector<storage::Row>* rows) const;

  // Recursively emits COLUMN_FIELD_PATHS rows for `column`, descending
  // into STRUCT (and ARRAY<STRUCT>) fields with a dotted `field_path`.
  void AppendFieldPathRows(const storage::TableId& table,
                           const schema::ColumnSchema& column,
                           absl::string_view top_level_name,
                           absl::string_view field_path,
                           std::vector<storage::Row>* rows) const;

  // Materializes ROUTINES rows from the durable routine store.
  absl::StatusOr<std::vector<storage::Row>> GenerateRoutineRows() const;

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
