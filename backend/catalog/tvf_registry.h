#ifndef BIGQUERY_EMULATOR_BACKEND_CATALOG_TVF_REGISTRY_H_
#define BIGQUERY_EMULATOR_BACKEND_CATALOG_TVF_REGISTRY_H_

#include <memory>

#include "absl/status/status.h"
#include "absl/strings/string_view.h"
#include "googlesql/public/analyzer_output.h"
#include "googlesql/public/simple_catalog.h"
#include "googlesql/resolved_ast/resolved_ast.h"

namespace bigquery_emulator {
namespace backend {
namespace catalog {

absl::Status RegisterProjectTvf(
    absl::string_view project_id,
    const ::googlesql::ResolvedCreateTableFunctionStmt& create_tvf_stmt,
    std::unique_ptr<const ::googlesql::AnalyzerOutput> analyzer_output);

void ReplayTvfsIntoCatalog(absl::string_view project_id,
                           ::googlesql::SimpleCatalog& catalog);

absl::Status DropProjectTvf(absl::string_view project_id,
                            absl::string_view tvf_name);

}  // namespace catalog
}  // namespace backend
}  // namespace bigquery_emulator

#endif  // BIGQUERY_EMULATOR_BACKEND_CATALOG_TVF_REGISTRY_H_
