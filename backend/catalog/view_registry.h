#ifndef BIGQUERY_EMULATOR_BACKEND_CATALOG_VIEW_REGISTRY_H_
#define BIGQUERY_EMULATOR_BACKEND_CATALOG_VIEW_REGISTRY_H_

#include <memory>

#include "absl/status/status.h"
#include "absl/strings/string_view.h"
#include "googlesql/public/analyzer_output.h"
#include "googlesql/public/simple_catalog.h"
#include "googlesql/public/types/type_factory.h"
#include "googlesql/resolved_ast/resolved_ast.h"

namespace bigquery_emulator {
namespace backend {
namespace catalog {

absl::Status RegisterProjectView(
    absl::string_view project_id,
    const ::googlesql::ResolvedCreateViewStmt& create_view_stmt,
    std::unique_ptr<const ::googlesql::AnalyzerOutput> analyzer_output,
    ::googlesql::TypeFactory* type_factory);

void ReplayViewsIntoCatalog(absl::string_view project_id,
                            ::googlesql::SimpleCatalog& catalog);

// Returns a registry-owned view matching `dataset_id.view_name` in
// `project_id`, or nullptr when no such view is registered.
const ::googlesql::Table* FindProjectView(absl::string_view project_id,
                                          absl::string_view dataset_id,
                                          absl::string_view view_name);

absl::Status DropProjectView(absl::string_view project_id,
                             absl::string_view view_name);

}  // namespace catalog
}  // namespace backend
}  // namespace bigquery_emulator

#endif  // BIGQUERY_EMULATOR_BACKEND_CATALOG_VIEW_REGISTRY_H_
