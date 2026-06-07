#include "backend/catalog/create_tvf_util.h"

#include <memory>
#include <utility>

#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "googlesql/public/sql_tvf.h"
#include "googlesql/resolved_ast/resolved_ast.h"

namespace bigquery_emulator {
namespace backend {
namespace catalog {

absl::StatusOr<std::unique_ptr<const ::googlesql::TableValuedFunction>>
MakeTvfFromCreateTableFunction(
    const ::googlesql::ResolvedCreateTableFunctionStmt& create_tvf_stmt) {
  std::unique_ptr<::googlesql::SQLTableValuedFunction> tvf;
  absl::Status created = ::googlesql::SQLTableValuedFunction::Create(
      &create_tvf_stmt, &tvf);
  if (!created.ok()) return created;
  if (tvf == nullptr) {
    return absl::InternalError(
        "create_tvf_util: SQLTableValuedFunction::Create returned null");
  }
  return std::unique_ptr<const ::googlesql::TableValuedFunction>(
      tvf.release());
}

}  // namespace catalog
}  // namespace backend
}  // namespace bigquery_emulator
