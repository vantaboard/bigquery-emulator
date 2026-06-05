#ifndef BIGQUERY_EMULATOR_BACKEND_ENGINE_SEMANTIC_FUNCTIONS_DISPATCH_H_
#define BIGQUERY_EMULATOR_BACKEND_ENGINE_SEMANTIC_FUNCTIONS_DISPATCH_H_

// Function-dispatch table for the semantic executor.
//
// `EvalFunctionCall` in `backend/engine/semantic/eval_expr.cc` first
// handles the analyzer's built-in operators and SAFE_* arithmetic
// in-line (those are tightly coupled to the in-package
// `ArithmeticAdd` / `ArithmeticDiv` / ... helpers). For BigQuery
// functions that the polyfill UDF library cannot model cleanly
// (per the matching `functions.yaml` row's `plan=
// googlesqlite-09-date-time.plan.md` pointer), the inline
// dispatch falls through to `Dispatch` below.
//
// `Dispatch` returns:
//
//   * `std::nullopt`           -- the function name is not yet
//     wired here. Callers surface NOT_IMPLEMENTED with the
//     "semantic: function 'X' is not yet implemented" message so
//     the gateway logs and CI matrix stay consistent.
//   * `std::optional<StatusOr<Value>>` carrying the implementation
//     result -- either a `Value` (BigQuery-exact answer) or a
//     structured `absl::Status` payload (overflow, division-by-
//     zero, invalid argument, etc.) the gateway maps onto the
//     BigQuery REST `error.errors[0].reason` token.
//
// Per the plan's "no silent approximation" rule, every entry here
// either matches the BigQuery contract for its full argument
// domain or surfaces a structured error -- there is no fallback
// to a DuckDB approximation.

#include <optional>
#include <vector>

#include "absl/status/statusor.h"
#include "absl/strings/string_view.h"
#include "backend/engine/semantic/eval_context.h"
#include "backend/engine/semantic/value.h"
#include "googlesql/public/type.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace semantic {

namespace functions {

// Dispatch the lowered BigQuery function `name` with the
// already-evaluated `args` against the analyzer-declared
// `return_type`. Returns nullopt when `name` is not in this
// package's dispatch table; the caller is expected to surface a
// NOT_IMPLEMENTED status in that case.
//
// `name` is the analyzer's canonical lowercase function name
// (e.g. `"bit_count"`, `"ieee_divide"`). NULL handling is the
// per-function responsibility -- most BigQuery scalars propagate
// NULL when ANY argument is NULL, but a few (e.g. `IFNULL`,
// `COALESCE`) do not.
std::optional<absl::StatusOr<Value>> Dispatch(
    absl::string_view name,
    const std::vector<Value>& args,
    const ::googlesql::Type* return_type,
    const EvalContext* ctx = nullptr);

}  // namespace functions
}  // namespace semantic
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator

#endif  // BIGQUERY_EMULATOR_BACKEND_ENGINE_SEMANTIC_FUNCTIONS_DISPATCH_H_
