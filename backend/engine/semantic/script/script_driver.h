#ifndef BIGQUERY_EMULATOR_BACKEND_ENGINE_SEMANTIC_SCRIPT_SCRIPT_DRIVER_H_
#define BIGQUERY_EMULATOR_BACKEND_ENGINE_SEMANTIC_SCRIPT_SCRIPT_DRIVER_H_

// Script-level execution driver and variable environment.
//
// `procedural-scripting-executor.plan.md` describes the local
// interpreter for BigQuery scripting (`DECLARE`, `SET`, `BEGIN ...
// END`, `IF`, `WHILE`, `LOOP`, `FOR`, `BREAK`, `CONTINUE`, `RAISE`,
// `CALL`, `ASSERT`, `EXECUTE IMMEDIATE`, `CREATE PROCEDURE`). Most
// of those constructs need a place to thread (a) the per-script
// variable state across statements and (b) the error-handler stack
// for `EXCEPTION WHEN ERROR THEN ...`. This file is the foundation
// scaffold for both.
//
// What this file owns today (Family 1 of the plan)
// ------------------------------------------------
//
//   * `VariableEnvironment` -- a stack of per-`BEGIN ... END` frames
//     keyed by `(scope, name)` with `Value` payloads (BigQuery
//     identifier comparison is case-insensitive, so we lower-case
//     names on `Declare` / `Lookup`). Frames are pushed when a new
//     `BEGIN ... END` block opens and popped when it closes; the
//     innermost matching binding wins for `Set` / `Lookup`. Variable
//     redeclaration in the same frame is rejected per BigQuery's
//     scripting reference.
//
//   * `ScriptDriver` -- the type the rest of the engine reaches
//     through. The class shape is in place so future families
//     (DECLARE/SET, BEGIN..END, IF/WHILE/LOOP, EXECUTE IMMEDIATE,
//     CALL, RAISE/EXCEPTION) can hang their state off it without
//     re-routing every call site. Today the driver only owns the
//     variable environment and exposes the per-statement helpers
//     this package's already-landed handlers (e.g. ASSERT) need.
//
// What this file does NOT own (deferred per the plan's pragmatic
// posture)
// --------
//
//   * Statement sequencing for `BEGIN ... END` -- requires
//     `googlesql/scripting/script_executor.h` (analyzer-level script
//     parser; the coordinator analyzer is per-statement). Tracked by
//     `procedural-scripting-executor.plan.md` Family 3.
//   * `IF` / `WHILE` / `LOOP` / `FOR` / `BREAK` / `CONTINUE` --
//     Families 4, 6, 11.
//   * `RAISE` / `EXCEPTION` handlers -- Family 7.
//   * `EXECUTE IMMEDIATE` -- Family 8.
//   * `CREATE PROCEDURE` body storage -- Family 9 (storage primitive
//     missing in `Storage`).
//   * `CALL` -- Family 10 (depends on Family 9).
//   * `@@error.message`, `@@last_statement_type`, ... reads on
//     `ResolvedSystemVariable` -- Family 12.
//
// Each deferred family preserves the YAML row at
// `backend/engine/duckdb/transpiler/node_dispositions.yaml` with
// `status=planned` so the engine surfaces the structured
// `notImplemented` envelope rather than silently approximating
// (the plan's hard rule).

#include <string>
#include <utility>
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
namespace script {

// Variable environment: a stack of frames keyed by `(scope, name)`
// with `Value` payloads. Each frame represents one `BEGIN ... END`
// block; `PushFrame` opens a new frame, `PopFrame` closes the most
// recent one. `Declare` registers a fresh variable in the
// innermost frame; `Set` updates the innermost binding for a name;
// `Lookup` reads the innermost binding. Names are matched case-
// insensitively (BigQuery identifier comparison; the BigQuery
// scripting reference for `DECLARE` lowercases the variable name
// before resolution).
//
// Thread-safety: a `VariableEnvironment` is owned by exactly one
// in-flight script invocation; callers do not share it across
// threads. The script driver creates one per script run and
// destroys it when the run ends.
class VariableEnvironment {
 public:
  VariableEnvironment();
  ~VariableEnvironment();

  VariableEnvironment(const VariableEnvironment&) = delete;
  VariableEnvironment& operator=(const VariableEnvironment&) = delete;

  // Open a new frame. Variables declared in this frame disappear
  // when `PopFrame` is called. The constructor opens the global
  // (script-level) frame so callers do not have to special-case
  // top-level `DECLARE`s.
  void PushFrame();

  // Close the most recent frame. Asserts (DCHECK) that there is at
  // least one frame still open above the global frame; popping the
  // global frame is a programmer error.
  absl::Status PopFrame();

  // Number of frames currently on the stack (always >= 1).
  size_t frame_count() const {
    return frames_.size();
  }

  // Register a fresh variable in the innermost frame. Returns
  // `kAlreadyExists` if `name` is already declared in that frame
  // (BigQuery rejects `DECLARE x` after a previous `DECLARE x` in
  // the same block; redeclaration is allowed across nested
  // blocks). The initial value is taken from `value`, which the
  // caller has already evaluated against the (typed) DEFAULT
  // expression -- the analyzer guarantees the type matches the
  // declared variable type.
  absl::Status Declare(absl::string_view name, Value value);

  // Update the innermost binding for `name`. Returns `kNotFound`
  // when no frame on the stack has a binding for `name`.
  absl::Status Set(absl::string_view name, Value value);

  // Read the innermost binding for `name`. Returns `kNotFound` when
  // no frame on the stack has a binding for `name`.
  absl::StatusOr<Value> Lookup(absl::string_view name) const;

  // Convenience: does the environment have any binding for `name`?
  bool Has(absl::string_view name) const;

 private:
  // Each frame is a flat map keyed on the analyzer-lowered name
  // (BigQuery identifier comparison is case-insensitive). The
  // payload is the current `Value`; updates via `Set` overwrite
  // the existing entry rather than allocating a fresh frame slot.
  using Frame = absl::flat_hash_map<std::string, Value>;
  std::vector<Frame> frames_;
};

// Per-script-run driver state. Today the driver only owns the
// variable environment; future families (error stack, registered
// procedure bodies, system-variable values) hang off the same
// object so call sites do not have to re-route.
//
// The driver is constructed at the top of a script invocation and
// destroyed at the bottom. The semantic executor's per-statement
// handlers receive a (mutable) reference to it via their statement-
// specific entry points (e.g. `script::ExecuteAssert`).
class ScriptDriver {
 public:
  ScriptDriver();
  ~ScriptDriver();

  ScriptDriver(const ScriptDriver&) = delete;
  ScriptDriver& operator=(const ScriptDriver&) = delete;

  VariableEnvironment& variables() {
    return variables_;
  }
  const VariableEnvironment& variables() const {
    return variables_;
  }

 private:
  VariableEnvironment variables_;
};

}  // namespace script
}  // namespace semantic
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator

#endif  // BIGQUERY_EMULATOR_BACKEND_ENGINE_SEMANTIC_SCRIPT_SCRIPT_DRIVER_H_
