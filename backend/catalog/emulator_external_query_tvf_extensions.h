#ifndef BIGQUERY_EMULATOR_BACKEND_CATALOG_EMULATOR_EXTERNAL_QUERY_TVF_EXTENSIONS_H_
#define BIGQUERY_EMULATOR_BACKEND_CATALOG_EMULATOR_EXTERNAL_QUERY_TVF_EXTENSIONS_H_

#include "googlesql/public/simple_catalog.h"

namespace bigquery_emulator {
namespace backend {
namespace catalog {

// Registers EXTERNAL_QUERY(connection STRING, query STRING) as a fixture-backed
// TVF. Analysis resolves output schema from $data_dir/external/connections/;
// evaluation materializes snapshot rows in the semantic executor.
void RegisterEmulatorExternalQueryTvf(::googlesql::SimpleCatalog& catalog);

}  // namespace catalog
}  // namespace backend
}  // namespace bigquery_emulator

#endif  // BIGQUERY_EMULATOR_BACKEND_CATALOG_EMULATOR_EXTERNAL_QUERY_TVF_EXTENSIONS_H_
