#ifndef BIGQUERY_EMULATOR_BACKEND_CATALOG_UDF_REGISTRY_H_
#define BIGQUERY_EMULATOR_BACKEND_CATALOG_UDF_REGISTRY_H_

// Per-project SQL UDF registrations that survive across
// `jobs.query` RPCs within the same emulator process.
//
// BigQuery temp functions are session-scoped; the query port suite
// issues `CREATE TEMP FUNCTION` in one request and calls the function
// in the next. The analyzer catalog is constructed per query, so we
// keep owned `googlesql::Function` objects (and the `AnalyzerOutput`
// that pins their resolved bodies) in this registry and replay them
// into each new `GoogleSqlCatalog`.

#include <memory>
#include <string>
#include <vector>

#include "absl/status/status.h"
#include "absl/strings/string_view.h"
#include "googlesql/public/analyzer_output.h"
#include "googlesql/public/catalog.h"
#include "googlesql/public/simple_catalog.h"
#include "googlesql/public/types/type_factory.h"

namespace bigquery_emulator {
namespace backend {
namespace catalog {

// Register a SQL UDF (CREATE [TEMP] FUNCTION) for `project_id`.
// When non-null, `analyzer_output` pins resolved UDF bodies for later
// queries; this call takes ownership. May be null when only the
// `Function` object from `MakeFunctionFromCreateFunction` is needed.
absl::Status RegisterProjectFunction(
    absl::string_view project_id,
    absl::string_view dataset_id,
    bool is_temp,
    std::unique_ptr<const ::googlesql::AnalyzerOutput> analyzer_output,
    std::unique_ptr<const ::googlesql::Function> function);

// Returns a registry-owned function matching `dataset_id.routine_name` in
// `project_id`, or nullptr when no such routine is registered.
const ::googlesql::Function* FindProjectFunction(
    absl::string_view project_id,
    absl::string_view dataset_id,
    absl::string_view routine_name);

// Resolves a qualified routine path (`ds.fn`, `proj.ds.fn`, or dotted
// single-segment backtick form) via `FindProjectFunction`.
const ::googlesql::Function* FindProjectFunctionFromPath(
    absl::Span<const std::string> path,
    absl::string_view catalog_project_id,
    absl::string_view default_dataset_id);

// Copy registered functions into `catalog` for analysis of later
// statements in the same project.
void ReplayFunctionsIntoCatalog(absl::string_view project_id,
                                ::googlesql::SimpleCatalog& catalog);

// Returns the per-project type factory used when registering SQL
// UDFs, or null when the project has no registered functions yet.
// Callers analyzing queries in a project with UDFs must use this
// factory (not a per-request stack factory) so replayed function
// signatures stay valid.
::googlesql::TypeFactory* LookupProjectTypeFactory(
    absl::string_view project_id);

// True when `project_id` owns a user-defined function named `fn_name`.
bool IsProjectRegisteredFunction(absl::string_view project_id,
                                 absl::string_view fn_name);

// Ensures a per-project `TypeFactory` exists for UDF registration.
::googlesql::TypeFactory* EnsureProjectTypeFactory(
    absl::string_view project_id);

// Removes a user-defined function from the per-project registry.
absl::Status DropProjectFunction(absl::string_view project_id,
                                 absl::string_view fn_name);

}  // namespace catalog
}  // namespace backend
}  // namespace bigquery_emulator

#endif  // BIGQUERY_EMULATOR_BACKEND_CATALOG_UDF_REGISTRY_H_
