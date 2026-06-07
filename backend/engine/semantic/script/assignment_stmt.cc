#include "backend/engine/semantic/script/assignment_stmt.h"

#include "absl/strings/str_cat.h"
#include "backend/engine/semantic/error.h"
#include "backend/engine/semantic/eval_expr.h"
#include "backend/engine/semantic/value.h"
#include "googlesql/resolved_ast/resolved_ast.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace semantic {
namespace script {

namespace {

std::string ScriptVariableName(const ::googlesql::ResolvedExpr& target) {
  if (target.node_kind() == ::googlesql::RESOLVED_CONSTANT) {
    const auto* constant =
        target.GetAs<::googlesql::ResolvedConstant>()->constant();
    if (constant != nullptr && !constant->Name().empty()) {
      return std::string(constant->Name());
    }
  }
  if (target.node_kind() == ::googlesql::RESOLVED_GET_STRUCT_FIELD) {
    const auto* gsf = target.GetAs<::googlesql::ResolvedGetStructField>();
    if (gsf != nullptr && gsf->expr() != nullptr) {
      return ScriptVariableName(*gsf->expr());
    }
  }
  return "";
}

}  // namespace

absl::Status ExecuteScriptAssignment(
    const QueryRequest& request,
    const ::googlesql::ResolvedAssignmentStmt& stmt,
    ScriptDriver& driver) {
  if (stmt.target() == nullptr || stmt.expr() == nullptr) {
    return absl::InvalidArgumentError(
        "script::ExecuteScriptAssignment: null target or expr");
  }
  const std::string name = ScriptVariableName(*stmt.target());
  if (name.empty()) {
    return MakeSemanticError(
        SemanticErrorReason::kNotImplemented,
        "script::ExecuteScriptAssignment: target is not a script variable");
  }
  if (stmt.target()->node_kind() == ::googlesql::RESOLVED_ARGUMENT_REF) {
    const auto& ref = *stmt.target()->GetAs<::googlesql::ResolvedArgumentRef>();
    EvalContext ctx;
    ctx.project_id = request.project_id;
    ctx.arguments = &driver.variables();
    ctx.script_variables = &driver.variables();
    auto value = EvalExpr(*stmt.expr(), ctx);
    if (!value.ok()) return value.status();
    absl::Status set = driver.variables().Set(ref.name(), *std::move(value));
    if (!set.ok()) {
      return MakeSemanticError(
          SemanticErrorReason::kInvalidArgument,
          absl::StrCat("script::ExecuteScriptAssignment: cannot assign to '",
                       ref.name(),
                       "': ",
                       set.message()));
    }
    return absl::OkStatus();
  }
  EvalContext ctx;
  ctx.project_id = request.project_id;
  ctx.script_variables = &driver.variables();
  ctx.arguments = &driver.variables();
  auto value = EvalExpr(*stmt.expr(), ctx);
  if (!value.ok()) return value.status();
  absl::Status set = driver.variables().Set(name, *std::move(value));
  if (!set.ok()) {
    return MakeSemanticError(
        SemanticErrorReason::kInvalidArgument,
        absl::StrCat("script::ExecuteScriptAssignment: cannot assign to '",
                     name,
                     "': ",
                     set.message()));
  }
  return absl::OkStatus();
}

}  // namespace script
}  // namespace semantic
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
