

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace semantic {
namespace script {

absl::Status ExecuteDeclare(const QueryRequest& request,
                            const ::googlesql::ResolvedCreateConstantStmt& stmt,
                            ScriptDriver& driver) {
  if (stmt.name_path().empty()) {
    return absl::InvalidArgumentError(
        "script::ExecuteDeclare: CREATE CONSTANT has empty name_path");
  }
  const std::string name = stmt.name_path().back();
  Value value;
  if (stmt.expr() == nullptr) {
    return absl::InvalidArgumentError(
        "script::ExecuteDeclare: DECLARE has no default expression");
  }
  EvalContext ctx;
  ctx.project_id = request.project_id;
  ctx.script_variables = &driver.variables();
  ctx.arguments = &driver.variables();
  auto evaluated = EvalExpr(*stmt.expr(), ctx);
  if (!evaluated.ok()) return evaluated.status();
  value = *std::move(evaluated);
  return driver.variables().Declare(name, value);
}

}  // namespace script
}  // namespace semantic
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
