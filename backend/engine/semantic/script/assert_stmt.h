#ifndef BIGQUERY_EMULATOR_BACKEND_ENGINE_SEMANTIC_SCRIPT_ASSERT_STMT_H_
#define BIGQUERY_EMULATOR_BACKEND_ENGINE_SEMANTIC_SCRIPT_ASSERT_STMT_H_

// Handler for `ResolvedAssertStmt` (`ASSERT <expr> [AS '<msg>']`).
//
// Family 5 of `local-exec-14-dml-system.plan.md`. The
// statement evaluates `<expr>` (which the analyzer guarantees is
// BOOL-typed) and surfaces a structured BigQuery-shaped error when
// the expression is FALSE or NULL. BigQuery's documented behavior
// (`docs/bigquery/docs/reference/standard-sql/debugging-statements.md`)
// is:
//
//   * `ASSERT TRUE` -- no observable effect; the statement succeeds
//     with no rows.
//   * `ASSERT FALSE [AS '<msg>']` -- the query fails with the
//     `<msg>` description (or a default `Assertion failed` message
//     when no description was provided). The REST envelope's
//     `error.errors[0].reason` is `invalidQuery` and the HTTP
//     status is 400.
//   * `ASSERT NULL` -- treated as a failed assertion (the docs
//     describe the predicate as "must evaluate to true"; NULL is
//     not true).
//
// The handler uses the semantic executor's expression evaluator
// (`backend/engine/semantic/eval_expr.h`) so SAFE-mode, NULL
// propagation, and parameter binding all behave exactly the same
// way they do in a top-level `SELECT`.
//
// Today the handler runs against a single-statement
// `ResolvedAssertStmt` dispatched directly by the coordinator (see
// `backend/engine/semantic/executor.cc::ExecuteDdl`). When Family 3
// (`BEGIN ... END` script sequencing) lands, the same handler will
// be invoked from the script driver's per-statement loop without
// changing the public signature here -- the driver is the future
// caller, not a future replacement.

#include "absl/status/status.h"
#include "backend/engine/engine.h"
#include "backend/engine/semantic/script/script_driver.h"

namespace googlesql {
class ResolvedAssertStmt;
}  // namespace googlesql

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace semantic {
namespace script {

// Run `ASSERT <expr> [AS '<msg>']`. `request` carries the
// BigQuery-side parameter bindings the analyzer declared on the
// `<expr>` subtree (named or positional). `driver` is the per-
// script driver state; today the handler does not touch it (ASSERT
// has no side effect on the variable environment), but the
// signature is in place so a future BEGIN..END caller can pass the
// driver through without re-routing.
//
// Returns:
//   * `OkStatus()` when the predicate evaluates to TRUE.
//   * `INVALID_ARGUMENT` carrying the semantic-error reason
//     `kInvalidArgument` (BigQuery REST `invalidQuery`) when the
//     predicate evaluates to FALSE or NULL. The status message
//     starts with `Assertion failed:` followed by the description
//     the analyzer attached to the statement (or the input SQL
//     fragment when no description was provided).
//   * Any error the expression evaluator surfaces (overflow,
//     division-by-zero, unsupported sub-expression, ...) is
//     propagated verbatim so the caller sees the same envelope it
//     would have seen for `SELECT <expr>`.
absl::Status ExecuteAssert(const QueryRequest& request,
                           const ::googlesql::ResolvedAssertStmt& stmt,
                           const ScriptDriver& driver);

}  // namespace script
}  // namespace semantic
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator

#endif  // BIGQUERY_EMULATOR_BACKEND_ENGINE_SEMANTIC_SCRIPT_ASSERT_STMT_H_
