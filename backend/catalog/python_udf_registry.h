#ifndef BIGQUERY_EMULATOR_BACKEND_CATALOG_PYTHON_UDF_REGISTRY_H_
#define BIGQUERY_EMULATOR_BACKEND_CATALOG_PYTHON_UDF_REGISTRY_H_

// Per-project Python UDF metadata keyed by routine name. Bodies are
// captured from `ResolvedCreateFunctionStmt` at registration / rehydrate
// time (the DDL text persisted in `DuckDBStorage.__bqemu_routines` is the
// source of truth that the analyzer replays into the resolved AST).

#include <string>
#include <vector>

#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "absl/strings/string_view.h"
#include "googlesql/public/type.pb.h"
#include "googlesql/resolved_ast/resolved_ast.h"

namespace bigquery_emulator {
namespace backend {
namespace catalog {

struct PythonUdfDefinition {
  std::string python_body;
  std::vector<std::string> arg_names;
  std::vector<::googlesql::TypeKind> arg_type_kinds;
  ::googlesql::TypeKind return_type_kind = ::googlesql::TYPE_UNKNOWN;
  std::string entry_point;
  std::vector<std::string> packages;
  bool is_aggregate = false;
};

// Register or replace Python UDF metadata for `project_id` / `fn_name`.
absl::Status RegisterProjectPythonUdf(absl::string_view project_id,
                                      absl::string_view fn_name,
                                      PythonUdfDefinition definition);

// Populate metadata from a resolved CREATE FUNCTION statement when
// `language()` is `python`.
absl::Status RegisterPythonUdfFromCreateFunction(
    absl::string_view project_id,
    const ::googlesql::ResolvedCreateFunctionStmt& create_function_stmt);

// Returns nullptr when the project has no Python UDF named `fn_name`.
const PythonUdfDefinition* LookupProjectPythonUdf(absl::string_view project_id,
                                                  absl::string_view fn_name);

// Minimal DDL fallback: parse `LANGUAGE python` scalar bodies from
// persisted `ddl_sql` when in-memory metadata is absent (e.g. tests that
// only replay storage rows).
absl::StatusOr<PythonUdfDefinition> ParsePythonUdfFromDdl(
    absl::string_view ddl_sql, absl::string_view fn_name = absl::string_view());

// Removes Python UDF metadata; no-op when absent.
void DropProjectPythonUdf(absl::string_view project_id,
                          absl::string_view fn_name);

}  // namespace catalog
}  // namespace backend
}  // namespace bigquery_emulator

#endif  // BIGQUERY_EMULATOR_BACKEND_CATALOG_PYTHON_UDF_REGISTRY_H_
