#ifndef BIGQUERY_EMULATOR_BACKEND_CATALOG_JS_UDF_REGISTRY_H_
#define BIGQUERY_EMULATOR_BACKEND_CATALOG_JS_UDF_REGISTRY_H_

// Per-project JavaScript UDF metadata keyed by routine name. Bodies are
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

struct JsUdfDefinition {
  std::string js_body;
  std::vector<std::string> arg_names;
  std::vector<::googlesql::TypeKind> arg_type_kinds;
  ::googlesql::TypeKind return_type_kind = ::googlesql::TYPE_UNKNOWN;
  bool is_aggregate = false;
};

// Register or replace JS UDF metadata for `project_id` / `fn_name`.
absl::Status RegisterProjectJsUdf(absl::string_view project_id,
                                  absl::string_view fn_name,
                                  JsUdfDefinition definition);

// Populate metadata from a resolved CREATE FUNCTION statement when
// `language()` is `js`.
absl::Status RegisterJsUdfFromCreateFunction(
    absl::string_view project_id,
    const ::googlesql::ResolvedCreateFunctionStmt& create_function_stmt);

// Returns nullptr when the project has no JS UDF named `fn_name`.
const JsUdfDefinition* LookupProjectJsUdf(absl::string_view project_id,
                                          absl::string_view fn_name);

// Minimal DDL fallback: parse `LANGUAGE js` scalar bodies from persisted
// `ddl_sql` when in-memory metadata is absent (e.g. tests that only
// replay storage rows).
absl::StatusOr<JsUdfDefinition> ParseJsUdfFromDdl(absl::string_view ddl_sql);

// Removes JS UDF metadata; no-op when absent.
void DropProjectJsUdf(absl::string_view project_id, absl::string_view fn_name);

}  // namespace catalog
}  // namespace backend
}  // namespace bigquery_emulator

#endif  // BIGQUERY_EMULATOR_BACKEND_CATALOG_JS_UDF_REGISTRY_H_
