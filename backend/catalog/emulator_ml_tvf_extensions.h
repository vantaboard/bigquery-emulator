#ifndef BIGQUERY_EMULATOR_BACKEND_CATALOG_EMULATOR_ML_TVF_EXTENSIONS_H_
#define BIGQUERY_EMULATOR_BACKEND_CATALOG_EMULATOR_ML_TVF_EXTENSIONS_H_

#include "googlesql/public/simple_catalog.h"
#include "googlesql/public/types/type_factory.h"

namespace bigquery_emulator {
namespace backend {
namespace catalog {

// Registers minimal ML.* TVF stubs missing from the stock GoogleSQL
// prebuilt artifact. Analysis resolves MODEL + TABLE arguments and
// computes a BigQuery-shaped output schema; evaluation is handled by
// `backend/engine/semantic/stubs/ml.cc` (NULL placeholders).
void RegisterEmulatorMlTvfStubs(::googlesql::SimpleCatalog& catalog);

// Materializes a placeholder model for ML.* analysis when no CREATE
// MODEL metadata exists. Used by `GoogleSqlCatalog::FindModel` and unit
// tests that analyze ML TVFs without a full model registry.
absl::StatusOr<const ::googlesql::Model*> MaterializeMlStubModel(
    const ::googlesql::TypeFactory* type_factory,
    const absl::Span<const std::string>& path);

// `FindModel` fallback for catalogs that serve ML.* analysis without a
// real model registry (production `GoogleSqlCatalog` and test helpers).
absl::Status ResolveMlStubModelForAnalysis(
    ::googlesql::SimpleCatalog& catalog,
    const ::googlesql::TypeFactory* type_factory,
    const absl::Span<const std::string>& path,
    const ::googlesql::Model** model,
    const ::googlesql::Catalog::FindOptions& options = {});

// Retry TVF lookup with only the final path segment when a qualified
// builtin name (e.g. `ML.PREDICT`) misses on the first pass.
absl::Status FindTableValuedFunctionWithUnqualifiedFallback(
    ::googlesql::SimpleCatalog& catalog,
    const absl::Span<const std::string>& path,
    const ::googlesql::TableValuedFunction** function,
    const ::googlesql::Catalog::FindOptions& options = {});

}  // namespace catalog
}  // namespace backend
}  // namespace bigquery_emulator

#endif  // BIGQUERY_EMULATOR_BACKEND_CATALOG_EMULATOR_ML_TVF_EXTENSIONS_H_
