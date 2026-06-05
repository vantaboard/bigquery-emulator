#ifndef BIGQUERY_EMULATOR_BACKEND_ENGINE_SEMANTIC_ERROR_H_
#define BIGQUERY_EMULATOR_BACKEND_ENGINE_SEMANTIC_ERROR_H_

// Semantic-executor structured errors.
//
// Every evaluation failure surfaces an `absl::Status` whose code is
// `absl::StatusCode::kInvalidArgument` (the BigQuery REST envelope
// for an in-query runtime error) carrying a `SemanticErrorReason`
// payload. The gateway maps the payload onto the BigQuery REST
// `error.errors[0].reason` token; see
// `.cursor/plans/googlesqlite-07-semantic-core-expr.plan.md` "Step 7".
//
// The plan rule is "no silent approximation": the semantic executor
// owns the BigQuery-exact error surface. Every reason in the enum
// below corresponds to a distinct BigQuery REST `reason` value the
// real product returns for the matching failure class.

#include <ostream>
#include <string>

#include "absl/status/status.h"
#include "absl/strings/string_view.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace semantic {

// Classification for an evaluation-time failure. Each value lines up
// 1:1 with a BigQuery REST `error.errors[0].reason` token so the
// gateway can translate a `SemanticError` payload into the wire
// error envelope without losing fidelity. Adding a new value here
// MUST be paired with a matching gateway-side mapping update.
enum class SemanticErrorReason {
  // Generic INVALID_ARGUMENT (BigQuery REST `invalidQuery`). Use
  // sparingly; reach for one of the more specific reasons below
  // whenever possible.
  kInvalidArgument = 0,
  // Division (or modulo) by zero. BigQuery surfaces this as
  // `divisionByZero` in the REST error envelope.
  kDivisionByZero = 1,
  // Arithmetic overflow on the current numeric type. BigQuery
  // surfaces this as `overflow`.
  kOverflow = 2,
  // A function / shape the semantic executor recognizes but has
  // not yet implemented. BigQuery surfaces this as `notImplemented`;
  // the coordinator may translate it to gRPC UNIMPLEMENTED so the
  // gateway folds it into the standard `notImplemented` reason.
  kNotImplemented = 3,
};

// String form of `reason` used in log lines / debug output. The
// returned `string_view` references a static literal; callers do
// not own it.
absl::string_view SemanticErrorReasonName(SemanticErrorReason reason);

// BigQuery REST `error.errors[0].reason` token for `reason`. The
// returned `string_view` references a static literal.
absl::string_view BigQueryReasonToken(SemanticErrorReason reason);

// Build the `absl::Status` payload that carries the structured
// error. The status code is INVALID_ARGUMENT for every reason
// except `kNotImplemented`, which uses UNIMPLEMENTED so the gateway
// maps it onto BigQuery's `notImplemented` reason without an
// explicit payload lookup. The `message` is rendered verbatim onto
// the status message.
absl::Status MakeSemanticError(SemanticErrorReason reason,
                               absl::string_view message);

// Extract the `SemanticErrorReason` payload from `status`. Returns
// `kInvalidArgument` when `status` is OK or carries no semantic
// payload, so callers do not have to guard the call.
SemanticErrorReason GetSemanticErrorReason(const absl::Status& status);

// String key used to attach the reason on the absl::Status payload
// map. Exported so the coordinator / gateway can read the payload
// off a status that crossed an executor boundary.
inline constexpr absl::string_view kSemanticErrorReasonPayloadUrl =
    "type.googleapis.com/bigquery_emulator.semantic.ErrorReason";

inline std::ostream& operator<<(std::ostream& os, SemanticErrorReason reason) {
  return os << SemanticErrorReasonName(reason);
}

}  // namespace semantic
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator

#endif  // BIGQUERY_EMULATOR_BACKEND_ENGINE_SEMANTIC_ERROR_H_
