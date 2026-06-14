#ifndef BIGQUERY_EMULATOR_BACKEND_CATALOG_EMULATOR_ML_TEST_CATALOG_H_
#define BIGQUERY_EMULATOR_BACKEND_CATALOG_EMULATOR_ML_TEST_CATALOG_H_

#include "absl/status/status.h"
#include "absl/types/span.h"
#include "backend/catalog/emulator_ml_tvf_extensions.h"
#include "googlesql/public/catalog.h"
#include "googlesql/public/simple_catalog.h"

namespace bigquery_emulator {
namespace backend {
namespace catalog {

// Test/catalog helper that resolves any MODEL path to a placeholder
// `SimpleModel` so ML.* TVF analysis can proceed without CREATE MODEL
// metadata.
class EmulatorMlTestCatalog : public ::googlesql::SimpleCatalog {
 public:
  using ::googlesql::SimpleCatalog::SimpleCatalog;

  absl::Status FindModel(const absl::Span<const std::string>& path,
                         const ::googlesql::Model** model,
                         const FindOptions& options = FindOptions()) override {
    return ResolveMlStubModelForAnalysis(*this, type_factory(), path, model,
                                         options);
  }
};

}  // namespace catalog
}  // namespace backend
}  // namespace bigquery_emulator

#endif  // BIGQUERY_EMULATOR_BACKEND_CATALOG_EMULATOR_ML_TEST_CATALOG_H_
