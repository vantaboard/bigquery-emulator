#ifndef BIGQUERY_EMULATOR_BACKEND_CATALOG_CREATE_PROCEDURE_UTIL_H_
#define BIGQUERY_EMULATOR_BACKEND_CATALOG_CREATE_PROCEDURE_UTIL_H_

#include <memory>

#include "absl/status/statusor.h"
#include "googlesql/resolved_ast/resolved_ast.h"

namespace bigquery_emulator {
namespace backend {
namespace catalog {

class StoredSQLProcedure;

absl::StatusOr<std::unique_ptr<StoredSQLProcedure>>
MakeProcedureFromCreateProcedure(
    const ::googlesql::ResolvedCreateProcedureStmt& create_procedure_stmt);

}  // namespace catalog
}  // namespace backend
}  // namespace bigquery_emulator

#endif  // BIGQUERY_EMULATOR_BACKEND_CATALOG_CREATE_PROCEDURE_UTIL_H_
