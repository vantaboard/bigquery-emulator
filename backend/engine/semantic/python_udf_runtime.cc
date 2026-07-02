#include "backend/engine/semantic/python_udf_runtime.h"

#include <unistd.h>

#include <cstdlib>
#include <string>
#include <vector>

#include "absl/status/statusor.h"
#include "absl/strings/ascii.h"
#include "absl/strings/string_view.h"
#include "backend/engine/semantic/error.h"
#include "backend/engine/semantic/python_udf_runtime_internal.h"
#include "backend/engine/semantic/python_udf_runtime_json.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace semantic {
namespace {

bool PathIsExecutable(const std::string& path) {
  return !path.empty() && access(path.c_str(), X_OK) == 0;
}

std::string ManagedVenvPythonPath() {
  const char* data_dir = std::getenv("BIGQUERY_EMULATOR_DATA_DIR");
  if (data_dir == nullptr || data_dir[0] == '\0') {
    return std::string();
  }
  std::string path(data_dir);
  if (!path.empty() && path.back() != '/') {
    path.push_back('/');
  }
  path.append("python-udf-env/bin/python3");
  return path;
}

}  // namespace

std::string ImportModuleNameFromPackageSpec(absl::string_view package_spec) {
  std::string name(package_spec);
  const char* separators[] = {"==", ">=", "<=", "!=", "~=", "<", ">"};
  size_t cut = name.size();
  for (const char* sep : separators) {
    const size_t pos = name.find(sep);
    if (pos != std::string::npos && pos < cut) {
      cut = pos;
    }
  }
  name.resize(cut);
  absl::StripAsciiWhitespace(&name);
  return name;
}

absl::StatusOr<std::string> ResolvePythonInterpreterPath() {
  if (const char* env = std::getenv("BIGQUERY_EMULATOR_PYTHON");
      env != nullptr && env[0] != '\0') {
    return std::string(env);
  }
  const std::string managed = ManagedVenvPythonPath();
  if (PathIsExecutable(managed)) {
    return managed;
  }
  return std::string("python3");
}

absl::StatusOr<Value> EvalPythonUdfCall(
    absl::string_view fn_name,
    const catalog::PythonUdfDefinition& definition,
    const std::vector<Value>& arg_values,
    const ::googlesql::Type* return_type,
    const std::vector<const ::googlesql::Type*>& arg_types) {
  (void)arg_types;
  if (definition.is_aggregate) {
    return MakeSemanticError(
        SemanticErrorReason::kNotImplemented,
        "CREATE AGGREGATE FUNCTION with language python is not supported");
  }
  if (definition.arg_names.size() != arg_values.size()) {
    return absl::InvalidArgumentError(
        absl::StrCat("semantic: Python UDF argument count mismatch (expected ",
                     definition.arg_names.size(),
                     ", got ",
                     arg_values.size(),
                     ")"));
  }

  absl::StatusOr<std::string> python_or = ResolvePythonInterpreterPath();
  if (!python_or.ok()) return python_or.status();

  absl::Status packages_ok =
      PreflightPythonPackages(*python_or, definition.packages);
  if (!packages_ok.ok()) {
    return packages_ok;
  }

  absl::StatusOr<std::string> request_or =
      BuildPythonUdfRequestJson(fn_name, definition, arg_values);
  if (!request_or.ok()) return request_or.status();

  absl::StatusOr<std::string> response_or =
      InvokePythonUdfRunner(*python_or, *request_or);
  if (!response_or.ok()) return response_or.status();

  absl::StatusOr<bool> ok_or = ExtractJsonBoolField(*response_or, "ok");
  if (!ok_or.ok()) return ok_or.status();
  if (!*ok_or) {
    absl::StatusOr<std::string> err_or =
        ExtractJsonStringField(*response_or, "error");
    return MakeSemanticError(
        SemanticErrorReason::kInvalidArgument,
        absl::StrCat("User-defined function error: ",
                     err_or.ok() ? *err_or : "Python UDF failed"));
  }

  const std::string response = *response_or;
  const std::string result_needle = "\"result\":";
  const size_t result_pos = response.find(result_needle);
  if (result_pos == std::string::npos) {
    return absl::InternalError("python_udf_runtime: response missing result");
  }
  absl::string_view result_json = response;
  result_json.remove_prefix(result_pos + result_needle.size());
  while (!result_json.empty() && absl::ascii_isspace(result_json.front())) {
    result_json.remove_prefix(1);
  }
  const size_t end = result_json.find('}');
  if (end != absl::string_view::npos) {
    result_json = result_json.substr(0, end);
  }
  return PopPythonValueToGooglesql(result_json, return_type);
}

}  // namespace semantic
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
