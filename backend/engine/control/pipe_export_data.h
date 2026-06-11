#ifndef BIGQUERY_EMULATOR_BACKEND_ENGINE_CONTROL_PIPE_EXPORT_DATA_H_
#define BIGQUERY_EMULATOR_BACKEND_ENGINE_CONTROL_PIPE_EXPORT_DATA_H_

// Handler for the pipe-operator form of `EXPORT DATA`. The resolved
// AST shape is
//
//   ResolvedQueryStmt
//     ResolvedPipeExportDataScan
//       export_data_stmt -> ResolvedExportDataStmt
//
// i.e. the analyzer wraps `<pipe-input> |> EXPORT DATA OPTIONS (...)`
// as a `ResolvedQueryStmt` whose body is a `ResolvedPipeExportDataScan`
// pointing at an inner `ResolvedExportDataStmt` that carries the
// options + `output_column_list`. The pipe scan's input is implicit
// (the pipe input rows are the rows the inner `ResolvedExportDataStmt`
// would export).
//
// The classifier routes the whole query to `control_op` via the
// `ResolvedPipeExportDataScan` entry in `node_dispositions.yaml`. The
// coordinator (`backend/engine/coordinator/local_coordinator_engine.cc`)
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
// `docs/ENGINE_POLICY.md` "don'ts" section, so the
// pipe-DDL handlers live in this file (and `pipe_create_table.cc`)
// instead. Both files live next to the executor so they share the
// same Bazel package and dependency graph.
//
// Today's contract
// ----------------
//
// `RunPipeExportData` returns `UNIMPLEMENTED` with a message that
// names the deferred follow-up: the EXPORT DATA writer family
// (`docs/ENGINE_POLICY.md` follow-up "add EXPORT DATA writer
// family"). This matches the existing `ResolvedExportDataStmt` row
// in `ControlOpExecutor::ExecuteDdl`: `EXPORT DATA` cannot ship
// until the local emulator grows Arrow / Parquet / CSV / JSON
// writers + a URI scheme dispatch surface, and approximating the
// behavior would violate the "no silent approximation" rule. The
// coordinator pre-dispatch wiring is in place so a future
// implementation lands here without touching the routing surface.

#include <memory>

#include "absl/status/statusor.h"
#include "backend/engine/engine.h"
#include "backend/storage/storage.h"

namespace googlesql {
class ResolvedStatement;
}  // namespace googlesql

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace control {

// Handle `<pipe-input> |> EXPORT DATA OPTIONS (...)`. The caller
// (`LocalCoordinatorEngine::ExecuteQuery`) is responsible for
// verifying that the statement is a `ResolvedQueryStmt` or
// `ResolvedGeneralizedQueryStmt` whose body is a
// `ResolvedPipeExportDataScan` before invoking this function.
//
// Delegates to `internal::RunExportData` on the inner
// `ResolvedExportDataStmt`. Returns an empty `RowSource` on success.
absl::StatusOr<std::unique_ptr<RowSource>> RunPipeExportData(
    storage::Storage& storage,
    const QueryRequest& request,
    const ::googlesql::ResolvedStatement& stmt);

}  // namespace control
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator

#endif  // BIGQUERY_EMULATOR_BACKEND_ENGINE_CONTROL_PIPE_EXPORT_DATA_H_
