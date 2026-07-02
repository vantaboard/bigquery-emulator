#ifndef BIGQUERY_EMULATOR_BACKEND_ENGINE_SEMANTIC_PYTHON_UDF_RUNTIME_H_
#define BIGQUERY_EMULATOR_BACKEND_ENGINE_SEMANTIC_PYTHON_UDF_RUNTIME_H_

#include <vector>

#include "absl/status/statusor.h"
#include "absl/strings/string_view.h"
#include "backend/catalog/python_udf_registry.h"
#include "backend/engine/semantic/value.h"
#include "googlesql/public/type.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace semantic {

// Evaluate a registered Python scalar UDF at call time. Argument values
// must already be evaluated by the caller.
absl::StatusOr<Value> EvalPythonUdfCall(
    absl::string_view fn_name,
    const catalog::PythonUdfDefinition& definition,
    const std::vector<Value>& arg_values,
    const ::googlesql::Type* return_type,
    const std::vector<const ::googlesql::Type*>& arg_types);

// Maps a CREATE FUNCTION `packages` entry (e.g. "lxml==4.9") to the
// importlib module name probed at call time.
std::string ImportModuleNameFromPackageSpec(absl::string_view package_spec);

// Interpreter resolution order: `BIGQUERY_EMULATOR_PYTHON`, then
// `$BIGQUERY_EMULATOR_DATA_DIR/python-udf-env/bin/python3` when present,
// else host `python3`.
absl::StatusOr<std::string> ResolvePythonInterpreterPath();

}  // namespace semantic
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator

#endif  // BIGQUERY_EMULATOR_BACKEND_ENGINE_SEMANTIC_PYTHON_UDF_RUNTIME_H_
