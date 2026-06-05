#ifndef BIGQUERY_EMULATOR_BACKEND_ENGINE_CONTROL_PIPE_CREATE_TABLE_H_
#define BIGQUERY_EMULATOR_BACKEND_ENGINE_CONTROL_PIPE_CREATE_TABLE_H_

// Handler for the pipe-operator form of `CREATE TABLE AS SELECT`.
// The resolved AST shape is
//
//   ResolvedQueryStmt
//     ResolvedPipeCreateTableScan
//       create_table_as_select_stmt -> ResolvedCreateTableAsSelectStmt
//                                       (with query = nullptr)
//
// i.e. the analyzer wraps `<pipe-input> |> CREATE TABLE <name> ...` as
// a `ResolvedQueryStmt` whose body is a `ResolvedPipeCreateTableScan`
// pointing at an inner `ResolvedCreateTableAsSelectStmt` that carries
// the target name + column-definition list. The inner statement's
// `query` field is intentionally left null because the pipe scan's
// input *is* the query (see the GoogleSQL resolved_ast comment on
// `ResolvedCreateTableAsSelectStmt.query`).
//
// The classifier routes the whole query to `control_op` via the
// `ResolvedPipeCreateTableScan` entry in `node_dispositions.yaml`.
// The coordinator (`backend/engine/coordinator/local_coordinator_engine.cc`)
// dispatches the pipe-DDL `ResolvedQueryStmt` here BEFORE handing the
// statement to `ControlOpExecutor::ExecuteQuery` (which always returns
// "control-op statements never produce a row stream" — accurate for
// the executor as a class, but wrong for the pipe-DDL ResolvedQueryStmt
// shape that the analyzer mints today).
//
// Why a separate translation unit
// -------------------------------
//
// `control_op_executor.cc` is a lint-cap carve-out per the
// `local-exec-13-advanced-relational.plan.md` "don'ts" section, so the
// pipe-DDL handlers live in this file (and `pipe_export_data.cc`)
// instead. Both files live next to the executor so they share the
// same Bazel package and dependency graph.
//
// Today's contract
// ----------------
//
// `RunPipeCreateTable` returns `UNIMPLEMENTED`. A "full" landing
// would re-issue the pipe input via the transpiler as a regular
// `CREATE TABLE <name> AS <emitted-pipe-input-sql>` and dispatch to
// the existing `RunCreateTableAsSelect` handler in
// `control_op_executor.cc`. That requires the transpiler to lower
// arbitrary pipe-input scans into a syntactically-valid `SELECT`
// (the pipe operator family is a larger surface than this plan
// owns), so the full implementation is deferred. The coordinator
// pre-dispatch wiring is in place so a future implementation lands
// here without touching the routing surface.

#include <memory>

#include "absl/status/statusor.h"
#include "backend/engine/engine.h"

namespace googlesql {
class ResolvedStatement;
}  // namespace googlesql

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace control {

// Handle `<pipe-input> |> CREATE TABLE <name> ...`. The caller
// (`LocalCoordinatorEngine::ExecuteQuery`) is responsible for
// verifying that the statement is a `ResolvedQueryStmt` or
// `ResolvedGeneralizedQueryStmt` whose body is a
// `ResolvedPipeCreateTableScan` before invoking this function.
//
// Returns `UNIMPLEMENTED` until the pipe-input transpiler surface
// lands (see file header for the deferred plan).
absl::StatusOr<std::unique_ptr<RowSource>> RunPipeCreateTable(
    const QueryRequest& request, const ::googlesql::ResolvedStatement& stmt);

}  // namespace control
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator

#endif  // BIGQUERY_EMULATOR_BACKEND_ENGINE_CONTROL_PIPE_CREATE_TABLE_H_
