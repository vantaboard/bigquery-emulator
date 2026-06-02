#include "backend/engine/semantic/script/script_driver.h"

#include <string>
#include <utility>

#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "absl/strings/ascii.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/string_view.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace semantic {
namespace script {

namespace {

// BigQuery identifier comparison is case-insensitive; the
// scripting reference specifically calls this out for variable
// names. Lower-case the name before storing / looking up so that
// `DECLARE x` followed by `SET X = 1` resolves to the same
// binding.
std::string Canonicalize(absl::string_view name) {
  return absl::AsciiStrToLower(name);
}

}  // namespace

VariableEnvironment::VariableEnvironment() {
  // Open the global (script-level) frame so callers do not have to
  // special-case top-level `DECLARE`s. `frame_count()` is therefore
  // always >= 1 once construction returns.
  frames_.emplace_back();
}

VariableEnvironment::~VariableEnvironment() = default;

void VariableEnvironment::PushFrame() {
  frames_.emplace_back();
}

absl::Status VariableEnvironment::PopFrame() {
  if (frames_.size() <= 1) {
    return absl::FailedPreconditionError(
        "script::VariableEnvironment: cannot pop the script-level frame; "
        "PushFrame / PopFrame must be balanced");
  }
  frames_.pop_back();
  return absl::OkStatus();
}

absl::Status VariableEnvironment::Declare(absl::string_view name, Value value) {
  if (frames_.empty()) {
    return absl::InternalError(
        "script::VariableEnvironment::Declare: no frame on the stack");
  }
  std::string key = Canonicalize(name);
  Frame& top = frames_.back();
  if (top.contains(key)) {
    // BigQuery rejects `DECLARE x ...` after a previous
    // `DECLARE x ...` in the same block (`Variable already
    // declared: x`).
    return absl::AlreadyExistsError(absl::StrCat(
        "script::VariableEnvironment: variable already declared: ", name));
  }
  top.emplace(std::move(key), std::move(value));
  return absl::OkStatus();
}

absl::Status VariableEnvironment::Set(absl::string_view name, Value value) {
  std::string key = Canonicalize(name);
  // Innermost binding wins: walk from the top of the stack down.
  for (auto it = frames_.rbegin(); it != frames_.rend(); ++it) {
    auto found = it->find(key);
    if (found != it->end()) {
      found->second = std::move(value);
      return absl::OkStatus();
    }
  }
  return absl::NotFoundError(absl::StrCat(
      "script::VariableEnvironment: variable not declared: ", name));
}

absl::StatusOr<Value> VariableEnvironment::Lookup(
    absl::string_view name) const {
  std::string key = Canonicalize(name);
  for (auto it = frames_.rbegin(); it != frames_.rend(); ++it) {
    auto found = it->find(key);
    if (found != it->end()) {
      return found->second;
    }
  }
  return absl::NotFoundError(absl::StrCat(
      "script::VariableEnvironment: variable not declared: ", name));
}

bool VariableEnvironment::Has(absl::string_view name) const {
  std::string key = Canonicalize(name);
  for (auto it = frames_.rbegin(); it != frames_.rend(); ++it) {
    if (it->contains(key)) return true;
  }
  return false;
}

ScriptDriver::ScriptDriver() = default;
ScriptDriver::~ScriptDriver() = default;

}  // namespace script
}  // namespace semantic
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
