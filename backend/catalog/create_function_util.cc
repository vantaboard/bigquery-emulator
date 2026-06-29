#include "backend/catalog/create_function_util.h"

#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "absl/strings/str_join.h"
#include "backend/engine/coordinator/sql_preprocess.h"
#include "googlesql/public/function.h"
#include "googlesql/public/sql_function.h"
#include "googlesql/public/templated_sql_function.h"
#include "googlesql/resolved_ast/resolved_ast.h"
#include "googlesql/resolved_ast/resolved_ast_enums.pb.h"

namespace bigquery_emulator {
namespace backend {
namespace catalog {

namespace {

absl::StatusOr<std::unique_ptr<const ::googlesql::Function>>
MakeFunctionFromCreateFunctionImpl(
    const ::googlesql::ResolvedCreateFunctionStmt& create_function_stmt,
    const ::googlesql::FunctionOptions* function_options) {
  ::googlesql::FunctionOptions options;
  if (function_options != nullptr) {
    options = *function_options;
  } else {
    options.set_uses_upper_case_sql_name(false);
  }
  const ::googlesql::FunctionEnums::Mode function_mode =
      create_function_stmt.is_aggregate()
          ? ::googlesql::FunctionEnums::AGGREGATE
          : ::googlesql::FunctionEnums::SCALAR;
  std::vector<std::string> name_path = create_function_stmt.name_path();
  std::unique_ptr<::googlesql::Function> function;
  if (create_function_stmt.function_expression() != nullptr) {
    absl::StatusOr<std::unique_ptr<::googlesql::SQLFunction>> sql_fn =
        ::googlesql::SQLFunction::Create(
            std::move(name_path),
            function_mode,
            create_function_stmt.signature(),
            std::move(options),
            create_function_stmt.function_expression(),
            create_function_stmt.argument_name_list(),
            &create_function_stmt.aggregate_expression_list(),
            /*parse_resume_location=*/std::nullopt);
    if (!sql_fn.ok()) return sql_fn.status();
    function = std::move(*sql_fn);
  } else if (create_function_stmt.language() == "SQL") {
    const std::string preprocessed_code =
        engine::coordinator::PreprocessFunctionBodyForAnalyzer(
            create_function_stmt.code());
    function = std::make_unique<::googlesql::TemplatedSQLFunction>(
        create_function_stmt.name_path(),
        create_function_stmt.signature(),
        create_function_stmt.argument_name_list(),
        ::googlesql::ParseResumeLocation::FromString(preprocessed_code),
        function_mode,
        options);
  } else {
    std::vector<::googlesql::FunctionSignature> signatures = {
        create_function_stmt.signature()};
    function = std::make_unique<::googlesql::Function>(
        create_function_stmt.name_path(),
        /*group=*/"External_function",
        function_mode,
        signatures,
        options);
  }
  function->set_sql_security(create_function_stmt.sql_security());
  return std::unique_ptr<const ::googlesql::Function>(std::move(function));
}

}  // namespace

absl::StatusOr<std::unique_ptr<const ::googlesql::Function>>
MakeFunctionFromCreateFunction(
    const ::googlesql::ResolvedCreateFunctionStmt& create_function_stmt,
    const ::googlesql::FunctionOptions* function_options) {
  return MakeFunctionFromCreateFunctionImpl(create_function_stmt,
                                            function_options);
}

}  // namespace catalog
}  // namespace backend
}  // namespace bigquery_emulator
