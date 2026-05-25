#ifndef BIGQUERY_EMULATOR_BACKEND_CATALOG_STORAGE_TABLE_H_
#define BIGQUERY_EMULATOR_BACKEND_CATALOG_STORAGE_TABLE_H_

// StorageTable is the `googlesql::Table` adapter that lets the
// GoogleSQL reference-impl evaluator stream rows out of the active
// `backend::storage::Storage`. It is the execution-side counterpart of
// the analysis-only `SimpleTable` instances `GoogleSqlCatalog` used to
// produce in Phase 4: same column shape, same `googlesql::Type*`
// allocations, but with a working `CreateEvaluatorTableIterator`
// override that wraps `Storage::ScanRows` and converts each storage
// `Value` into a `googlesql::Value` of the matching column type as
// the evaluator pulls rows.
//
// We subclass `googlesql::SimpleTable` so column-list management
// (NumColumns / GetColumn / FindColumnByName / set_full_name / ...)
// stays one place. The override is just the iterator factory; nothing
// else changes about how the analyzer or the algebrizer sees the
// table.
//
// `StorageTable` itself does NOT own the `Storage` it reads from --
// the storage instance lives at engine scope (constructed once at
// startup, see `binaries/emulator_main/main.cc`). The catalog +
// `StorageTable` lifetime is per query; the engine creates the
// catalog when a `Query.ExecuteQuery` RPC arrives and destroys it
// once the row stream completes.

#include <memory>
#include <string>
#include <vector>

#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "absl/strings/string_view.h"
#include "absl/types/span.h"
#include "backend/schema/schema.h"
#include "backend/storage/storage.h"
#include "googlesql/public/evaluator_table_iterator.h"
#include "googlesql/public/simple_catalog.h"
#include "googlesql/public/type.h"
#include "googlesql/public/value.h"

namespace bigquery_emulator {
namespace backend {
namespace catalog {

// Adapter `googlesql::Table` whose row stream comes from
// `Storage::ScanRows(table_id)`. The column shape is fixed at
// construction time (matching `bq_schema`); the iterator projects out
// the columns the evaluator requests.
class StorageTable : public ::googlesql::SimpleTable {
 public:
  // Builds the adapter from an engine-agnostic BigQuery schema. The
  // `columns` argument must be aligned 1:1 with `bq_schema.columns`
  // (same length, same order); each entry pairs the column's name with
  // its analyzer-allocated `googlesql::Type*`. The catalog adapter
  // (`GoogleSqlCatalog::MaterializeTable`) is the only caller that
  // already has both pieces in hand, which is why the constructor
  // takes them paired rather than re-running the type translation.
  StorageTable(absl::string_view name, absl::string_view full_name,
               absl::Span<const NameAndType> columns,
               schema::TableSchema bq_schema, storage::TableId table_id,
               const storage::Storage* storage);

  ~StorageTable() override = default;

  StorageTable(const StorageTable&) = delete;
  StorageTable& operator=(const StorageTable&) = delete;

  // Streams the rows of the underlying `Storage` table, projected
  // down to `column_idxs`. The returned iterator captures a snapshot
  // taken at call time (semantics inherited from `Storage::ScanRows`)
  // and converts each storage `Value` to the matching
  // `googlesql::Value` of the column's type on the fly.
  absl::StatusOr<std::unique_ptr<::googlesql::EvaluatorTableIterator>>
  CreateEvaluatorTableIterator(
      absl::Span<const int> column_idxs) const override;

  // Backing storage identifier the analyzer-allocated `Table` adapter
  // is materialized from. Used by the DuckDB engine (Phase 5i) to map
  // a `ResolvedTableScan::table()` pointer back to the
  // `Storage::ScanRows(id)` call site so the engine can ATTACH the
  // rows into its DuckDB connection before executing the transpiled
  // SQL. The catalog hands out the same `Table*` for the lifetime of
  // the catalog instance, so the returned reference is stable across
  // the same query.
  const storage::TableId& storage_table_id() const { return table_id_; }

  // Engine-agnostic schema the storage layer keeps for this table.
  // Companion accessor to `storage_table_id()`: the DuckDB engine
  // (Phase 5i) consults it to emit the matching `CREATE TABLE` DDL
  // when it loads the rows into its in-memory DuckDB connection.
  const schema::TableSchema& bq_schema() const { return bq_schema_; }

 private:
  // The BigQuery-level schema mirrors `SimpleTable::columns_` 1:1 but
  // carries the engine-agnostic `ColumnType` discriminator the cell
  // converter needs to interpret storage `Value`s. We keep a private
  // copy because the catalog re-materializes the catalog per query
  // and we don't want lookups during iteration to touch the storage
  // mutex.
  const schema::TableSchema bq_schema_;
  const storage::TableId table_id_;
  const storage::Storage* const storage_;  // not owned
};

// Convert an engine-agnostic storage `Value` into a `googlesql::Value`
// of `type`. Exposed for unit tests; the iterator implementation is
// the only production caller. NULLs round-trip as
// `googlesql::Value::Null(type)` so the evaluator's NULL bitmap stays
// honest.
//
// Returns `INVALID_ARGUMENT` when the storage Value's kind cannot be
// reconciled with `type` -- e.g. a kInt64 cell paired with a STRING
// column. Storage validates its own shape against the table schema
// on `AppendRows`, so this branch only fires when the schema and the
// rows have diverged on disk.
absl::StatusOr<::googlesql::Value> StorageValueToGoogleSqlValue(
    const storage::Value& value, const ::googlesql::Type* type);

}  // namespace catalog
}  // namespace backend
}  // namespace bigquery_emulator

#endif  // BIGQUERY_EMULATOR_BACKEND_CATALOG_STORAGE_TABLE_H_
