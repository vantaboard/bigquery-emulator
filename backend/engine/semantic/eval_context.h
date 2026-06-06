#ifndef BIGQUERY_EMULATOR_BACKEND_ENGINE_SEMANTIC_EVAL_CONTEXT_H_
#define BIGQUERY_EMULATOR_BACKEND_ENGINE_SEMANTIC_EVAL_CONTEXT_H_

#include <optional>
#include <string>
#include <vector>

#include "absl/container/flat_hash_map.h"
#include "absl/strings/string_view.h"
#include "googlesql/public/value.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace semantic {

class FrameStack;
struct CteTable;

struct ParameterBindings {
  absl::flat_hash_map<std::string, ::googlesql::Value> by_name;
  std::vector<::googlesql::Value> by_position;
};

using ColumnBindings = absl::flat_hash_map<int, ::googlesql::Value>;

struct EvalContext {
  absl::string_view project_id;
  const ParameterBindings* parameters = nullptr;
  const ColumnBindings* columns = nullptr;
  const absl::flat_hash_map<std::string, ::googlesql::Value>* columns_by_name =
      nullptr;
  const FrameStack* arguments = nullptr;
  const absl::flat_hash_map<std::string, CteTable>* with_tables = nullptr;
  mutable std::optional<std::string> bignumeric_render_override;
};

}  // namespace semantic
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator

#endif  // BIGQUERY_EMULATOR_BACKEND_ENGINE_SEMANTIC_EVAL_CONTEXT_H_
