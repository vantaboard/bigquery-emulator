#ifndef BIGQUERY_EMULATOR_BACKEND_CATALOG_CREATE_VIEW_UTIL_H_
#define BIGQUERY_EMULATOR_BACKEND_CATALOG_CREATE_VIEW_UTIL_H_

#include <memory>

#include "absl/status/statusor.h"
#include "googlesql/public/simple_catalog.h"
#include "googlesql/public/types/type_factory.h"
#include "googlesql/resolved_ast/resolved_ast.h"

namespace bigquery_emulator {
namespace backend {
namespace catalog {

absl::StatusOr<std::unique_ptr<const ::googlesql::Table>>
MakeViewFromCreateView(
    const ::googlesql::ResolvedCreateViewStmt& create_view_stmt,
    const ::googlesql::TypeFactory* type_factory);

}  // namespace catalog
}  // namespace backend
}  // namespace bigquery_emulator

#endif  // BIGQUERY_EMULATOR_BACKEND_CATALOG_CREATE_VIEW_UTIL_H_
