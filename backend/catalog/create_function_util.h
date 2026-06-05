#ifndef BIGQUERY_EMULATOR_BACKEND_CATALOG_CREATE_FUNCTION_UTIL_H_
#define BIGQUERY_EMULATOR_BACKEND_CATALOG_CREATE_FUNCTION_UTIL_H_

// Minimal replacement for `googlesql::MakeFunctionFromCreateFunction`
// (the upstream helper lives in `simple_catalog_util.cc`, which is not
// linked in the googlesql prebuilt artifact).

#include <memory>

#include "absl/status/statusor.h"
#include "googlesql/public/function.h"
#include "googlesql/resolved_ast/resolved_ast.h"

namespace bigquery_emulator {
namespace backend {
namespace catalog {

absl::StatusOr<std::unique_ptr<const ::googlesql::Function>>
MakeFunctionFromCreateFunction(
    const ::googlesql::ResolvedCreateFunctionStmt& create_function_stmt,
    const ::googlesql::FunctionOptions* function_options);

}  // namespace catalog
}  // namespace backend
}  // namespace bigquery_emulator

#endif  // BIGQUERY_EMULATOR_BACKEND_CATALOG_CREATE_FUNCTION_UTIL_H_
