#ifndef BIGQUERY_EMULATOR_BACKEND_ENGINE_SEMANTIC_SCRIPT_SCRIPT_DRIVER_H_
#define BIGQUERY_EMULATOR_BACKEND_ENGINE_SEMANTIC_SCRIPT_SCRIPT_DRIVER_H_

// Script-level execution driver.
//
// `local-exec-14-dml-system.plan.md` describes the local
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
//   * `ScriptDriver` -- the type the rest of the engine reaches
//     through. The class shape is in place so future families
//     (DECLARE/SET, BEGIN..END, IF/WHILE/LOOP, EXECUTE IMMEDIATE,
//     CALL, RAISE/EXCEPTION) can hang their state off it without
//     re-routing every call site. Today the driver owns a
//     `semantic::FrameStack` for the variable environment and
//     exposes the per-statement helpers this package's already-
//     landed handlers (e.g. ASSERT) need.
//
// The variable-environment primitive itself lives in
// `backend/engine/semantic/frame_stack.h` so the UDF / TVF
// invocation surface (`local-exec-15-specialized-stubs.plan.md`) can reuse
// the same stack-of-frames type for argument bindings without
// taking a transitive dependency on the script-statement
// translation units.
//
// What this file does NOT own (deferred per the plan's pragmatic
// posture)
// --------
//
//   * Statement sequencing for `BEGIN ... END` -- requires
//     `googlesql/scripting/script_executor.h` (analyzer-level script
//     parser; the coordinator analyzer is per-statement). Tracked by
//     `local-exec-14-dml-system.plan.md` Family 3.
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

#include "backend/engine/semantic/frame_stack.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace semantic {
namespace script {

// Per-script-run driver state. Today the driver only owns the
// shared `semantic::FrameStack` for the script-level variable
// environment; future families (error stack, registered procedure
// bodies, system-variable values) hang off the same object so call
// sites do not have to re-route.
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

  FrameStack& variables() {
    return variables_;
  }
  const FrameStack& variables() const {
    return variables_;
  }

 private:
  FrameStack variables_;
};

}  // namespace script
}  // namespace semantic
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator

#endif  // BIGQUERY_EMULATOR_BACKEND_ENGINE_SEMANTIC_SCRIPT_SCRIPT_DRIVER_H_
