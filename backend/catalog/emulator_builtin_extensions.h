#ifndef BIGQUERY_EMULATOR_BACKEND_CATALOG_EMULATOR_BUILTIN_EXTENSIONS_H_
#define BIGQUERY_EMULATOR_BACKEND_CATALOG_EMULATOR_BUILTIN_EXTENSIONS_H_

#include "googlesql/public/simple_catalog.h"

namespace bigquery_emulator {
namespace backend {
namespace catalog {

// Registers emulator-specific scalar functions missing from the
// stock GoogleSQL builtin set but required for BigQuery parity
// (e.g. ISNULL). Safe to call on every per-query catalog instance;
// function objects are created once and reused.
void RegisterEmulatorBuiltinFunctions(::googlesql::SimpleCatalog& catalog);

}  // namespace catalog
}  // namespace backend
}  // namespace bigquery_emulator

#endif  // BIGQUERY_EMULATOR_BACKEND_CATALOG_EMULATOR_BUILTIN_EXTENSIONS_H_
