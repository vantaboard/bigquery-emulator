#ifndef BIGQUERY_EMULATOR_BACKEND_ENGINE_SEMANTIC_EVAL_EXPR_H_
#define BIGQUERY_EMULATOR_BACKEND_ENGINE_SEMANTIC_EVAL_EXPR_H_

// Expression evaluation entry point.
//
// `EvalExpr` is the top-level switch on `ResolvedExpr::node_kind()`.
// For each node kind the analyzer can produce in a scalar
// expression position, we have a per-shape evaluator that returns a
// `semantic::Value` (alias for `googlesql::Value`) -- the unit of
// evaluation the rest of the package speaks.
//
// Implementation contract (see
// `.cursor/plans/semantic-executor-core.plan.md`):
//
//   * The evaluator NEVER re-analyzes SQL. It walks the resolved
//     AST the analyzer hands it.
//   * Each operator implements BigQuery-exact semantics. NULL
//     propagation, overflow, error surfaces are owned here -- no
//     silent approximation.
//   * Errors are `absl::Status` payloads via `MakeSemanticError`
//     so the coordinator can map them onto the BigQuery REST
//     error envelope without per-operator handling.
//   * `SAFE.<fn>` (the analyzer's `SAFE_ERROR_MODE`) is recognized
//     on `ResolvedFunctionCall` and converts evaluation failures
//     to the matching NULL value of the function's return type.

#include <string>

#include "absl/container/flat_hash_map.h"
#include "absl/status/statusor.h"
#include "absl/strings/string_view.h"
#include "backend/engine/semantic/value.h"
#include "googlesql/resolved_ast/resolved_ast.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace semantic {

// Binding map for GoogleSQL query parameters.
//
//   * Named parameters (`@<name>`) live in `by_name` keyed on the
//     analyzer-lowercased identifier. The analyzer lowercases
//     parameter names verbatim before resolution, so the executor
//     looks up with the lowercase form.
//   * Positional parameters (`?` -> analyzer's 1-based
//     `ResolvedParameter::position()`) live in `by_position` keyed
//     on `position - 1` so the vector index is the slot number the
//     analyzer assigned.
struct ParameterBindings {
  absl::flat_hash_map<std::string, Value> by_name;
  std::vector<Value> by_position;
};

// Read-only evaluation context shared across every `EvalExpr` call
// in a single query. We keep it small (just parameter bindings
// today) so callers can stack it on the C++ frame. Downstream
// plans extend the context for FROM-clause column bindings and
// correlated-subquery rebinding.
struct EvalContext {
  const ParameterBindings* parameters = nullptr;
};

// Evaluate `expr` against `ctx` and return the resulting
// `semantic::Value`. The dispatch is exhaustive over the
// `ResolvedExpr` node kinds the semantic executor handles today;
// any unrecognized kind returns NOT_IMPLEMENTED so the coordinator
// surfaces a clean envelope.
absl::StatusOr<Value> EvalExpr(const ::googlesql::ResolvedExpr& expr,
                               const EvalContext& ctx);

// Evaluate the `ResolvedFunctionCall` at `call`. Public so the
// per-function dispatch tests can exercise individual operators
// without having to wrap each one in a SELECT.
absl::StatusOr<Value> EvalFunctionCall(
    const ::googlesql::ResolvedFunctionCall& call, const EvalContext& ctx);

}  // namespace semantic
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator

#endif  // BIGQUERY_EMULATOR_BACKEND_ENGINE_SEMANTIC_EVAL_EXPR_H_
