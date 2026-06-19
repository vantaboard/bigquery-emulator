#ifndef BIGQUERY_EMULATOR_BACKEND_CATALOG_VIEW_REGISTRY_H_
#define BIGQUERY_EMULATOR_BACKEND_CATALOG_VIEW_REGISTRY_H_

#include <memory>
#include <string>
#include <vector>

#include "absl/status/status.h"
#include "absl/strings/string_view.h"
#include "googlesql/public/analyzer_output.h"
#include "googlesql/public/catalog.h"
#include "googlesql/public/types/type_factory.h"
#include "googlesql/resolved_ast/resolved_ast.h"

namespace bigquery_emulator {
namespace backend {
namespace catalog {

// One registered view's INFORMATION_SCHEMA-facing metadata: the
// dataset it lives in, its name, and the SQL text it was defined with
// (`ResolvedCreateViewStmt::sql()`). `use_standard_sql` is always true
// because the emulator only registers GoogleSQL views.
struct RegisteredViewInfo {
  std::string dataset_id;
  std::string view_name;
  std::string view_definition;
  bool use_standard_sql = true;
};

// Returns the registered views in `project_id`, optionally filtered to
// `dataset_id` (empty / region-* selectors return every dataset's
// views). Ordered by (dataset_id, view_name) for deterministic output.
std::vector<RegisteredViewInfo> ListProjectViews(absl::string_view project_id,
                                                 absl::string_view dataset_id);

absl::Status RegisterProjectView(
    absl::string_view project_id,
    const ::googlesql::ResolvedCreateViewStmt& create_view_stmt,
    std::unique_ptr<const ::googlesql::AnalyzerOutput> analyzer_output,
    ::googlesql::TypeFactory* type_factory);

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
