
#include <cstddef>
#include <string>
#include <utility>

#include "absl/base/thread_annotations.h"
#include "absl/status/status.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/string_view.h"
#include "absl/synchronization/mutex.h"
#include "googlesql/public/type.h"
#include "googlesql/public/types/array_type.h"
#include "googlesql/public/types/struct_type.h"
#include "googlesql/public/types/type_factory.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace semantic {

namespace {

absl::Mutex mu;
// project_id -> (canonical_name -> Value)
absl::flat_hash_map<std::string, absl::flat_hash_map<std::string, Value>>
    values_by_project ABSL_GUARDED_BY(mu);

std::string CanonicalName(const std::vector<std::string>& name_path) {
  if (name_path.empty()) return "";
  std::string out;
  for (size_t i = 0; i < name_path.size(); ++i) {
    if (i > 0) out.push_back('.');
    out.append(name_path[i]);
  }
  return out;
}

}  // namespace

absl::Status RegisterAnalyzerSystemVariables(
    ::googlesql::TypeFactory* type_factory,
    ::googlesql::AnalyzerOptions& options) {
  if (type_factory == nullptr) {
    return absl::InvalidArgumentError(
        "semantic/system_variables: type_factory is null");
  }
  const ::googlesql::Type* string_type = type_factory->get_string();
  if (string_type == nullptr) {
    return absl::InternalError(
        "semantic/system_variables: string type allocation failed");
  }
  absl::Status s = options.AddSystemVariable({"time_zone"}, string_type);
  if (!s.ok()) return s;
  // Scripting @@error.* variables (populated at runtime by
  // googlesql::ScriptExecutor during EXCEPTION handlers).
  s = options.AddSystemVariable({"error", "message"}, string_type);
  if (!s.ok()) return s;
  s = options.AddSystemVariable({"error", "statement_text"}, string_type);
  if (!s.ok()) return s;
  s = options.AddSystemVariable({"error", "formatted_stack_trace"},
                                string_type);
  if (!s.ok()) return s;
  const ::googlesql::StructType* stack_frame_type = nullptr;
  s = type_factory->MakeStructType(
      {::googlesql::StructField("line", type_factory->get_int64()),
       ::googlesql::StructField("column", type_factory->get_int64()),
       ::googlesql::StructField("filename", string_type),
       ::googlesql::StructField("location", string_type)},
      &stack_frame_type);
  if (!s.ok()) return s;
  const ::googlesql::ArrayType* stack_trace_type = nullptr;
  s = type_factory->MakeArrayType(stack_frame_type, &stack_trace_type);
  if (!s.ok()) return s;
  return options.AddSystemVariable({"error", "stack_trace"}, stack_trace_type);
}

absl::StatusOr<Value> GetSystemVariable(
    absl::string_view project_id, const std::vector<std::string>& name_path) {
  const std::string key = CanonicalName(name_path);
  if (key.empty()) {
    return absl::InvalidArgumentError(
        "semantic/system_variables: empty system variable name");
  }
  absl::MutexLock lock(&mu);
  auto pit = values_by_project.find(std::string(project_id));
  if (pit != values_by_project.end()) {
    auto vit = pit->second.find(key);
    if (vit != pit->second.end()) {
      return vit->second;
    }
  }
  if (key == "time_zone") {
    return Value::String("UTC");
  }
  return absl::NotFoundError(
      absl::StrCat("semantic/system_variables: @@", key, " is not set"));
}

absl::Status SetSystemVariable(absl::string_view project_id,
                               const std::vector<std::string>& name_path,
                               Value value) {
  const std::string key = CanonicalName(name_path);
  if (key.empty()) {
    return absl::InvalidArgumentError(
        "semantic/system_variables: empty system variable name");
  }
  absl::MutexLock lock(&mu);
  values_by_project[std::string(project_id)][key] = std::move(value);
  return absl::OkStatus();
}

}  // namespace semantic
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
