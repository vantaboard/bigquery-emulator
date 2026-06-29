

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace semantic {
namespace script {

absl::Status ExecuteCall(const QueryRequest& request,
                         const ::googlesql::ResolvedCallStmt& stmt,
                         const ScriptDriver& driver) {
  (void)request;
  (void)stmt;
  (void)driver;
  return MakeSemanticError(
      SemanticErrorReason::kNotImplemented,
      "semantic: CALL statement execution is not yet implemented");
}

}  // namespace script
}  // namespace semantic
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
