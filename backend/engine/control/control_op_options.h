#ifndef BIGQUERY_EMULATOR_BACKEND_ENGINE_CONTROL_CONTROL_OP_OPTIONS_H_
#define BIGQUERY_EMULATOR_BACKEND_ENGINE_CONTROL_CONTROL_OP_OPTIONS_H_

#include <string>
#include <vector>

#include "absl/status/statusor.h"
#include "absl/strings/string_view.h"
#include "googlesql/resolved_ast/resolved_ast.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace control {
namespace internal {

// Returns the string literal held by `opt->value()`, or an error when
// the option value is not a STRING literal.
absl::StatusOr<std::string> OptionStringValue(
    const ::googlesql::ResolvedOption* opt);

// Finds the first option named `name` (case-insensitive) in `options`
// and returns its STRING literal value.
absl::StatusOr<std::string> FindOptionString(
    const std::vector<std::unique_ptr<const ::googlesql::ResolvedOption>>&
        options,
    absl::string_view name);

// Extracts STRING elements from a resolved ARRAY literal expression.
absl::StatusOr<std::vector<std::string>> ExtractStringArrayLiteral(
    const ::googlesql::ResolvedExpr* expr);

// Normalizes export/load URI schemes to a local filesystem path.
// `file://` prefixes are stripped; bare absolute paths pass through.
// `gs://` URIs are materialized under `data_dir/external/gcs-cache/`
// from a local snapshot or STORAGE_EMULATOR_HOST (fake-gcs).
absl::StatusOr<std::string> LocalPathFromUri(absl::string_view uri);
absl::StatusOr<std::string> LocalPathFromUri(absl::string_view uri,
                                             absl::string_view data_dir);

}  // namespace internal
}  // namespace control
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator

#endif  // BIGQUERY_EMULATOR_BACKEND_ENGINE_CONTROL_CONTROL_OP_OPTIONS_H_
