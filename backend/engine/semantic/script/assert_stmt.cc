#include "backend/engine/semantic/script/assert_stmt.h"

#include <string>
#include <utility>

#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "absl/strings/ascii.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/string_view.h"
#include "backend/engine/engine.h"
#include "backend/engine/semantic/error.h"
#include "backend/engine/semantic/eval_expr.h"
#include "backend/engine/semantic/script/script_driver.h"
#include "backend/engine/semantic/value.h"
#include "googlesql/public/type.h"
#include "googlesql/public/type.pb.h"
#include "googlesql/resolved_ast/resolved_ast.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace semantic {
namespace script {

namespace {

// Mirror `executor.cc::BuildParameterBindings`. The semantic
// executor's parameter-binding helper lives in an anonymous
// namespace so we re-build a small replica here. Keeping the
// duplication intentional rather than promoting the helper to a
// public header until at least two scripting handlers share the
// surface (next consumer is `DECLARE` / `SET` once Family 2
// lands).
absl::StatusOr<ParameterBindings> BuildParameterBindings(
    const QueryRequest& request) {
  ParameterBindings bindings;
  bool any_positional = false;
  bool any_named = false;
  for (const QueryParameter& p : request.parameters) {
    auto value = ParseParameterValue(p.value_json, p.type_kind);
    if (!value.ok()) return value.status();
    if (p.name.empty()) {
      bindings.by_position.push_back(*std::move(value));
      any_positional = true;
    } else {
      bindings.by_name[absl::AsciiStrToLower(p.name)] = *std::move(value);
      any_named = true;
    }
  }
  if (any_positional && any_named) {
    return absl::InvalidArgumentError(
        "script: request mixes named and positional parameters");
  }
  return bindings;
}

}  // namespace

absl::Status ExecuteAssert(const QueryRequest& request,
                           const ::googlesql::ResolvedAssertStmt& stmt,
                           const ScriptDriver& driver) {
  (void)driver;  // ASSERT does not mutate the variable environment.
  const ::googlesql::ResolvedExpr* predicate = stmt.expression();
  if (predicate == nullptr) {
    return absl::InternalError(
        "script::ExecuteAssert: ResolvedAssertStmt has null expression");
  }
  if (predicate->type() == nullptr ||
      predicate->type()->kind() != ::googlesql::TYPE_BOOL) {
    // The analyzer's `ASSERT` resolution coerces the predicate to
    // BOOL or rejects the statement; if we ever see a non-BOOL
    // predicate it is an analyzer / engine wiring bug, not a user
    // error.
    return absl::InternalError(
        "script::ExecuteAssert: ResolvedAssertStmt predicate is not BOOL-"
        "typed; analyzer should have rejected it");
  }
  // Read the description regardless of the result so the analyzer's
  // field-access tracker on the owning `AnalyzerOutput` is satisfied.
  const std::string description(stmt.description());

  ParameterBindings bindings;
  if (!request.parameters.empty()) {
    auto built = BuildParameterBindings(request);
    if (!built.ok()) return built.status();
    bindings = *std::move(built);
  }
  EvalContext ctx;
  ctx.parameters = &bindings;
  absl::StatusOr<Value> evaluated = EvalExpr(*predicate, ctx);
  if (!evaluated.ok()) return evaluated.status();

  // BigQuery's `ASSERT` treats a NULL predicate the same as FALSE
  // ("the predicate must evaluate to true"). Empty / unset BOOL
  // values (the analyzer's `Value::NullBool()`) flag the failure
  // before the runtime tries to read the bool payload.
  if (evaluated->is_null() || !evaluated->bool_value()) {
    std::string detail = description.empty()
                             ? std::string("Assertion failed")
                             : absl::StrCat("Assertion failed: ", description);
    return MakeSemanticError(SemanticErrorReason::kInvalidArgument, detail);
  }
  return absl::OkStatus();
}

}  // namespace script
}  // namespace semantic
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
