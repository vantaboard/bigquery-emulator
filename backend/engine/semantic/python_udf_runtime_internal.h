#ifndef BIGQUERY_EMULATOR_BACKEND_ENGINE_SEMANTIC_PYTHON_UDF_RUNTIME_INTERNAL_H_
#define BIGQUERY_EMULATOR_BACKEND_ENGINE_SEMANTIC_PYTHON_UDF_RUNTIME_INTERNAL_H_

#include <string>
#include <vector>

#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "absl/strings/string_view.h"
#include "backend/catalog/python_udf_registry.h"
#include "backend/engine/semantic/value.h"
#include "googlesql/public/type.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace semantic {

absl::Status PreflightPythonPackages(absl::string_view python_path,
                                     const std::vector<std::string>& packages);

absl::StatusOr<std::string> BuildPythonUdfRequestJson(
    absl::string_view fn_name,
    const catalog::PythonUdfDefinition& definition,
    const std::vector<Value>& arg_values);

absl::StatusOr<std::string> InvokePythonUdfRunner(
    absl::string_view python_path, absl::string_view request_json);

absl::StatusOr<Value> PopPythonValueToGooglesql(
    absl::string_view raw_result, const ::googlesql::Type* return_type);

}  // namespace semantic
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator

#endif  // BIGQUERY_EMULATOR_BACKEND_ENGINE_SEMANTIC_PYTHON_UDF_RUNTIME_INTERNAL_H_
