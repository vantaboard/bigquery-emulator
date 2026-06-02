#ifndef BIGQUERY_EMULATOR_BACKEND_ENGINE_SEMANTIC_FRAME_STACK_H_
#define BIGQUERY_EMULATOR_BACKEND_ENGINE_SEMANTIC_FRAME_STACK_H_

// Name-keyed `Value` frame stack shared by the script driver and
// the UDF / TVF invocation surface.
//
// Two contexts in the semantic executor need a stack of name ->
// `Value` frames with case-insensitive identifier matching:
//
//   * BigQuery scripting (`DECLARE`, `SET`, `BEGIN ... END`,
//     `IF` / `WHILE` / ...). Each `BEGIN ... END` block pushes a
//     frame, declarations register fresh bindings in the innermost
//     frame, and the innermost matching binding wins for `Set` /
//     `Lookup`. Owned by
//     `.cursor/plans/procedural-scripting-executor.plan.md`.
//   * SQL UDF / TVF invocation. Each call binds the argument list
//     into a fresh frame the body's `ResolvedArgumentRef` /
//     `ResolvedRelationArgumentScan` nodes resolve against. Owned by
//     `.cursor/plans/udf-tvf-module-routing.plan.md`.
//
// Both contexts share the same primitive: a stack of frames keyed
// on the analyzer-lowered identifier, with `Value` payloads. The
// type lives in `backend/engine/semantic/` (one level above
// `script/`) so the UDF / TVF call sites can depend on it without
// taking a transitive dependency on the script-statement
// translation units.
//
// Thread-safety: a `FrameStack` is owned by exactly one in-flight
// invocation (script run or UDF / TVF call). Callers do not share
// it across threads. The owning driver constructs one per run and
// destroys it when the run ends.

#include <string>
#include <vector>

#include "absl/container/flat_hash_map.h"
#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "absl/strings/string_view.h"
#include "backend/engine/semantic/value.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace semantic {

// A stack of `(name -> Value)` frames. The constructor opens the
// outer (global) frame so callers do not have to special-case
// top-level declarations; `frame_count()` is therefore always >= 1
// once construction returns.
//
// BigQuery identifier comparison is case-insensitive (see the
// BigQuery scripting reference for `DECLARE` and the UDF /
// argument resolution rules); names are lower-cased on `Declare` /
// `Lookup` / `Set` so `DECLARE x` followed by `SET X = 1` (or a
// UDF body referencing `X` when the analyzer canonicalized the
// argument name to `x`) resolves to the same binding.
class FrameStack {
 public:
  FrameStack();
  ~FrameStack();

  FrameStack(const FrameStack&) = delete;
  FrameStack& operator=(const FrameStack&) = delete;

  // Open a new frame. Bindings declared in this frame disappear
  // when `PopFrame` is called.
  void PushFrame();

  // Close the most recent frame. Returns `kFailedPrecondition` if
  // there is no frame above the outer (global) frame to pop;
  // popping the outer frame is a programmer error (the call stack
  // would no longer support `Declare`).
  absl::Status PopFrame();

  // Number of frames currently on the stack (always >= 1).
  size_t frame_count() const {
    return frames_.size();
  }

  // Register a fresh binding in the innermost frame. Returns
  // `kAlreadyExists` if `name` is already bound in that frame
  // (BigQuery rejects `DECLARE x` after a previous `DECLARE x` in
  // the same block, and UDF / TVF signatures may not declare the
  // same argument name twice).
  absl::Status Declare(absl::string_view name, Value value);

  // Update the innermost binding for `name`. Returns `kNotFound`
  // when no frame on the stack has a binding for `name`.
  absl::Status Set(absl::string_view name, Value value);

  // Read the innermost binding for `name`. Returns `kNotFound`
  // when no frame on the stack has a binding for `name`.
  absl::StatusOr<Value> Lookup(absl::string_view name) const;

  // Convenience: does any frame on the stack have a binding for
  // `name`?
  bool Has(absl::string_view name) const;

 private:
  // Each frame is a flat map keyed on the analyzer-lowered name.
  // The payload is the current `Value`; updates via `Set`
  // overwrite the existing entry in the innermost frame that owns
  // the name (rather than allocating a fresh slot in the top
  // frame).
  using Frame = absl::flat_hash_map<std::string, Value>;
  std::vector<Frame> frames_;
};

}  // namespace semantic
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator

#endif  // BIGQUERY_EMULATOR_BACKEND_ENGINE_SEMANTIC_FRAME_STACK_H_
