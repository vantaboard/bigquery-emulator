#ifndef BIGQUERY_EMULATOR_BACKEND_CATALOG_CREATE_TVF_UTIL_H_
#define BIGQUERY_EMULATOR_BACKEND_CATALOG_CREATE_TVF_UTIL_H_

#include <memory>

#include "absl/status/statusor.h"
#include "googlesql/public/table_valued_function.h"
#include "googlesql/resolved_ast/resolved_ast.h"

namespace bigquery_emulator {
namespace backend {
namespace catalog {

absl::StatusOr<std::unique_ptr<const ::googlesql::TableValuedFunction>>
MakeTvfFromCreateTableFunction(
    const ::googlesql::ResolvedCreateTableFunctionStmt& create_tvf_stmt);

}  // namespace catalog
}  // namespace backend
}  // namespace bigquery_emulator

#endif  // BIGQUERY_EMULATOR_BACKEND_CATALOG_CREATE_TVF_UTIL_H_
