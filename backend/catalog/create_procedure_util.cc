#include "backend/catalog/create_procedure_util.h"

#include <memory>
#include <utility>

#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "backend/catalog/stored_procedure.h"
#include "googlesql/resolved_ast/resolved_ast.h"

namespace bigquery_emulator {
namespace backend {
namespace catalog {

absl::StatusOr<std::unique_ptr<StoredSQLProcedure>>
MakeProcedureFromCreateProcedure(
    const ::googlesql::ResolvedCreateProcedureStmt& create_procedure_stmt) {
  if (create_procedure_stmt.name_path().empty()) {
    return absl::InvalidArgumentError(
        "create_procedure_util: CREATE PROCEDURE has empty name_path");
  }
  std::vector<std::string> arg_names;
  arg_names.reserve(create_procedure_stmt.argument_name_list_size());
  for (int i = 0; i < create_procedure_stmt.argument_name_list_size(); ++i) {
    arg_names.push_back(create_procedure_stmt.argument_name_list(i));
  }
  return std::make_unique<StoredSQLProcedure>(
      create_procedure_stmt.name_path(),
      create_procedure_stmt.signature(),
      std::move(arg_names),
      std::string(create_procedure_stmt.procedure_body()));
}

}  // namespace catalog
}  // namespace backend
}  // namespace bigquery_emulator
