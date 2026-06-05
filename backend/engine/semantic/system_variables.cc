#include "backend/engine/semantic/system_variables.h"

#include <string>
#include <utility>
#include <vector>

#include "absl/container/flat_hash_map.h"
#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/string_view.h"
#include "absl/synchronization/mutex.h"
#include "backend/engine/semantic/value.h"
#include "googlesql/public/type.h"
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
  return absl::OkStatus();
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
