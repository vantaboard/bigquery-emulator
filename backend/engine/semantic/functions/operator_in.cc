#include "backend/engine/semantic/functions/operator_funcs.h"

#include <vector>

#include "absl/status/statusor.h"
#include "absl/strings/string_view.h"
#include "backend/engine/semantic/value.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace semantic {
namespace functions {

absl::StatusOr<Value> DispatchIn(absl::string_view name,
                                 const std::vector<Value>& args) {
  if (args.size() < 2) {
    return absl::InvalidArgumentError(
        "semantic: IN expects at least two arguments");
  }
  if (args[0].is_null()) {
    return Value::NullBool();
  }
  const Value& lhs = args[0];
  bool saw_null_rhs = false;
  for (size_t i = 1; i < args.size(); ++i) {
    if (args[i].is_null()) {
      saw_null_rhs = true;
      continue;
    }
    if (lhs.Equals(args[i])) {
      bool found = true;
      if (name == "$not_in") {
        found = false;
      }
      return Value::Bool(found);
    }
  }
  if (saw_null_rhs) {
    return Value::NullBool();
  }
  bool found = false;
  if (name == "$not_in") {
    found = true;
  }
  return Value::Bool(found);
}

}  // namespace functions
}  // namespace semantic
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
