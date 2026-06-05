#ifndef BIGQUERY_EMULATOR_BACKEND_CATALOG_VIRTUAL_TABLE_H_
#define BIGQUERY_EMULATOR_BACKEND_CATALOG_VIRTUAL_TABLE_H_

// VirtualCatalogTable is the DuckDB attach hook for catalog tables that
// are not backed by a single `Storage::GetSchema` entry (INFORMATION_SCHEMA
// views, wildcard table unions). The analyzer sees them as ordinary
// `googlesql::Table` instances with working `CreateEvaluatorTableIterator`
// implementations; the DuckDB engine materializes their rows into the
// per-query in-memory connection through `MaterializeInDuckDB`.

#include <string>

#include "absl/status/status.h"
#include "absl/strings/string_view.h"
#include "backend/storage/storage.h"
#include "duckdb.h"
#include "googlesql/public/simple_catalog.h"

namespace bigquery_emulator {
namespace backend {
namespace catalog {

class VirtualCatalogTable : public ::googlesql::SimpleTable {
 public:
  using SimpleTable::SimpleTable;

  ~VirtualCatalogTable() override = default;

  // Populate `quoted_table_name` inside `conn` with the virtual table's
  // rows so the transpiler's `FROM <name>` resolves during ExecuteQuery.
  virtual absl::Status MaterializeInDuckDB(
      ::duckdb_connection conn,
      const storage::Storage* storage,
      absl::string_view quoted_table_name) const = 0;
};

}  // namespace catalog
}  // namespace backend
}  // namespace bigquery_emulator

#endif  // BIGQUERY_EMULATOR_BACKEND_CATALOG_VIRTUAL_TABLE_H_
