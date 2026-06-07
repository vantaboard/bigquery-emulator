#include "backend/catalog/create_view_util.h"

#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "googlesql/public/simple_catalog.h"
#include "googlesql/resolved_ast/resolved_ast.h"

namespace bigquery_emulator {
namespace backend {
namespace catalog {

absl::StatusOr<std::unique_ptr<const ::googlesql::Table>>
MakeViewFromCreateView(
    const ::googlesql::ResolvedCreateViewStmt& create_view_stmt,
    ::googlesql::TypeFactory* type_factory) {
  if (type_factory == nullptr) {
    return absl::InvalidArgumentError(
        "create_view_util: type_factory must be non-null");
  }
  if (create_view_stmt.query() == nullptr) {
    return absl::InvalidArgumentError(
        "create_view_util: CREATE VIEW has null query");
  }
  if (create_view_stmt.name_path().empty()) {
    return absl::InvalidArgumentError(
        "create_view_util: CREATE VIEW has empty name_path");
  }
  std::vector<::googlesql::SimpleSQLView::NameAndType> columns;
  columns.reserve(create_view_stmt.column_definition_list_size());
  for (int i = 0; i < create_view_stmt.column_definition_list_size(); ++i) {
    const ::googlesql::ResolvedColumnDefinition* col =
        create_view_stmt.column_definition_list(i);
    if (col == nullptr || col->type() == nullptr) {
      return absl::InvalidArgumentError(
          "create_view_util: CREATE VIEW column has null type");
    }
    columns.push_back(
        {std::string(col->name()), col->type()});
  }
  const std::string view_name = create_view_stmt.name_path().back();
  ::googlesql::ResolvedCreateStatement::SqlSecurity security =
      create_view_stmt.sql_security();
  if (security ==
      ::googlesql::ResolvedCreateStatementEnums::SQL_SECURITY_UNSPECIFIED) {
    security =
        ::googlesql::ResolvedCreateStatementEnums::SQL_SECURITY_INVOKER;
  }
  absl::StatusOr<std::unique_ptr<::googlesql::SimpleSQLView>> view =
      ::googlesql::SimpleSQLView::Create(
          view_name,
          std::move(columns),
          security,
          create_view_stmt.is_value_table(),
          create_view_stmt.query());
  if (!view.ok()) return view.status();
  return std::unique_ptr<const ::googlesql::Table>(
      std::move(view).value().release());
}

}  // namespace catalog
}  // namespace backend
}  // namespace bigquery_emulator
