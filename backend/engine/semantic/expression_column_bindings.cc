#include "backend/engine/semantic/expression_column_bindings.h"

#include "absl/strings/str_cat.h"
#include "backend/engine/semantic/error.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace semantic {

absl::Status RegisterExpressionColumnsOnAnalyzerOptions(
    const FrameStack& variables, ::googlesql::AnalyzerOptions& options) {
  for (const auto& [name, value] : variables.VisibleBindings()) {
    if (value.type() == nullptr) {
      return MakeSemanticError(
          SemanticErrorReason::kInvalidArgument,
          absl::StrCat("semantic: expression column '",
                       name,
                       "' has no type for AnalyzeExpression registration"));
    }
    absl::Status added = options.AddExpressionColumn(name, value.type());
    if (!added.ok()) return added;
  }
  return absl::OkStatus();
}

void PopulateEvalContextExpressionColumns(
    const FrameStack& variables,
    EvalContext& ctx,
    absl::flat_hash_map<std::string, ::googlesql::Value>* storage) {
  storage->clear();
  for (const auto& [name, value] : variables.VisibleBindings()) {
    (*storage)[name] = value;
  }
  ctx.columns_by_name = storage;
}

}  // namespace semantic
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
