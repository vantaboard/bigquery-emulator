#ifndef BIGQUERY_EMULATOR_BACKEND_ENGINE_SEMANTIC_STUBS_DISPATCH_H_
#define BIGQUERY_EMULATOR_BACKEND_ENGINE_SEMANTIC_STUBS_DISPATCH_H_

// `local_stub` family dispatch entry point.
//
// `backend/engine/semantic/eval_expr.cc::DispatchFunctionByName`
// calls `Dispatch(name, args, return_type)` after the in-line
// operator / SAFE_* path and the
// `backend/engine/semantic/functions::Dispatch` table miss. The
// name is the dotted, lowercase, namespace-qualified BigQuery
// function name (e.g. `"keys.new_keyset"`, `"keys.keyset_length"`)
// -- the route classifier and dispatcher both rely on
// `Function::FullName(/*include_group=*/false)` to surface the
// namespace prefix so the lookup matches the YAML key.
//
// Returning `std::nullopt` means "this dispatch table does not
// know about that name"; the caller falls through to its own
// NOT_IMPLEMENTED handling. Returning a `Value` means a stub fired
// (BigQuery-shaped placeholder); returning a non-OK
// `absl::StatusOr` means the stub fired but the input was invalid
// (wrong arity, wrong type) and the semantic executor surfaces the
// status to the caller.
//
// A new stub family is wired by:
//   1. Adding the per-family `cc_library` to
//      `backend/engine/semantic/stubs/BUILD.bazel`.
//   2. Adding a Dispatch arm here.
//   3. Confirming the matching `functions.yaml` row is
//      `local_stub plan=specialized-feature-policy.plan.md`.
//   4. Adding a conformance fixture under
//      `conformance/fixtures/specialized/`.

#include <optional>
#include <vector>

#include "absl/status/statusor.h"
#include "absl/strings/string_view.h"
#include "backend/engine/semantic/value.h"
#include "googlesql/public/type.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace semantic {
namespace stubs {

std::optional<absl::StatusOr<Value>> Dispatch(
    absl::string_view name,
    const std::vector<Value>& args,
    const ::googlesql::Type* return_type);

}  // namespace stubs
}  // namespace semantic
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator

#endif  // BIGQUERY_EMULATOR_BACKEND_ENGINE_SEMANTIC_STUBS_DISPATCH_H_
