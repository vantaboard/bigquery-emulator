#ifndef BIGQUERY_EMULATOR_BACKEND_CATALOG_PYTHON_UDF_REGISTRY_INTERNAL_H_
#define BIGQUERY_EMULATOR_BACKEND_CATALOG_PYTHON_UDF_REGISTRY_INTERNAL_H_

#include "absl/status/statusor.h"
#include "absl/strings/string_view.h"
#include "backend/catalog/python_udf_registry.h"

namespace bigquery_emulator {
namespace backend {
namespace catalog {

std::string InferEntryPointFromBody(absl::string_view body,
                                    absl::string_view fn_name);

absl::StatusOr<PythonUdfDefinition> ParsePythonUdfFromDdlImpl(
    absl::string_view ddl_sql, absl::string_view fn_name);

}  // namespace catalog
}  // namespace backend
}  // namespace bigquery_emulator

#endif  // BIGQUERY_EMULATOR_BACKEND_CATALOG_PYTHON_UDF_REGISTRY_INTERNAL_H_
