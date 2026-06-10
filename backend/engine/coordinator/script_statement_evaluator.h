#ifndef BIGQUERY_EMULATOR_BACKEND_ENGINE_COORDINATOR_SCRIPT_STATEMENT_EVALUATOR_H_
#define BIGQUERY_EMULATOR_BACKEND_ENGINE_COORDINATOR_SCRIPT_STATEMENT_EVALUATOR_H_

#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "absl/types/span.h"
#include "backend/engine/coordinator/local_coordinator_engine.h"
#include "backend/engine/engine.h"
#include "backend/engine/semantic/script/script_driver.h"
#include "google/protobuf/any.pb.h"
#include "googlesql/public/analyzer.h"
#include "googlesql/public/catalog.h"
#include "googlesql/public/types/type_parameters.h"
#include "googlesql/public/value.h"
#include "googlesql/scripting/script_executor.h"
#include "googlesql/scripting/script_segment.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace coordinator {

class EmulatorStatementEvaluator : public ::googlesql::StatementEvaluator {
 public:
  EmulatorStatementEvaluator(LocalCoordinatorEngine* engine,
                             QueryRequest request,
                             ::googlesql::Catalog* catalog,
                             semantic::script::ScriptDriver* driver);

  void SetFinalRowsOut(std::unique_ptr<RowSource>* final_rows_out);

  absl::Status ExecuteStatement(
      const ::googlesql::ScriptExecutor& executor,
      const ::googlesql::ScriptSegment& segment) override;

  absl::StatusOr<std::unique_ptr<::googlesql::EvaluatorTableIterator>>
  ExecuteQueryWithResult(const ::googlesql::ScriptExecutor& executor,
                         const ::googlesql::ScriptSegment& segment) override;

  absl::Status SerializeIterator(
      const ::googlesql::EvaluatorTableIterator& iterator,
      google::protobuf::Any& out) override;

  absl::StatusOr<std::unique_ptr<::googlesql::EvaluatorTableIterator>>
  DeserializeToIterator(
      const google::protobuf::Any& msg,
      const ::googlesql::ScriptExecutor& executor,
      const ::googlesql::ParsedScript& parsed_script) override;

  absl::StatusOr<int> EvaluateCaseExpression(
      const ::googlesql::ScriptSegment& case_value,
      const std::vector<::googlesql::ScriptSegment>& when_values,
      const ::googlesql::ScriptExecutor& executor) override;

  absl::StatusOr<int64_t> GetIteratorMemoryUsage(
      const ::googlesql::EvaluatorTableIterator& iterator) override;

  absl::StatusOr<::googlesql::Value> EvaluateScalarExpression(
      const ::googlesql::ScriptExecutor& executor,
      const ::googlesql::ScriptSegment& segment,
      const ::googlesql::Type* target_type) override;

  absl::StatusOr<::googlesql::TypeWithParameters> ResolveTypeName(
      const ::googlesql::ScriptExecutor& executor,
      const ::googlesql::ScriptSegment& segment) override;

  bool IsSupportedVariableType(
      const ::googlesql::TypeWithParameters& type_with_params) override;

  absl::Status ApplyTypeParameterConstraints(
      const ::googlesql::TypeParameters& type_params,
      ::googlesql::Value* value) override;

  absl::StatusOr<std::unique_ptr<::googlesql::ProcedureDefinition>>
  LoadProcedure(const ::googlesql::ScriptExecutor& executor,
                const absl::Span<const std::string>& path,
                int64_t num_arguments) override;

  absl::Status AssignSystemVariable(
      ::googlesql::ScriptExecutor* executor,
      const ::googlesql::ASTSystemVariableAssignment* ast_assignment,
      const ::googlesql::Value& value) override;

 private:
  absl::StatusOr<::googlesql::Value> EvalScalarSegment(
      const ::googlesql::ScriptSegment& segment,
      semantic::EvalContext& ctx,
      const ::googlesql::Type* target_type);

  LocalCoordinatorEngine* engine_;
  QueryRequest request_;
  ::googlesql::Catalog* catalog_;
  semantic::script::ScriptDriver* driver_;
  std::unique_ptr<RowSource>* final_rows_out_ = nullptr;
};

}  // namespace coordinator
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator

#endif  // BIGQUERY_EMULATOR_BACKEND_ENGINE_COORDINATOR_SCRIPT_STATEMENT_EVALUATOR_H_
