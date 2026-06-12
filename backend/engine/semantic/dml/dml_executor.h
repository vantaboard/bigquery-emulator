#ifndef BIGQUERY_EMULATOR_BACKEND_ENGINE_SEMANTIC_DML_DML_EXECUTOR_H_
#define BIGQUERY_EMULATOR_BACKEND_ENGINE_SEMANTIC_DML_DML_EXECUTOR_H_

// Storage-aware local DML executor.
//
// Owns the DML statement shapes the DuckDB fast path cannot lower
// cleanly, per `docs/ENGINE_POLICY.md`:
//
//   * `ResolvedInsertStmt` -- INSERT VALUES and INSERT ... SELECT
//     (SELECT half via `MaterializeScan` or DuckDB fast path).
//   * `ResolvedDeleteStmt` -- `DELETE FROM t WHERE <pred>`.
//   * `ResolvedUpdateStmt` -- scalar SET, deep-STRUCT SET, and
//     `UPDATE ... FROM ...`.
//   * `ASSERT_ROWS_MODIFIED` on INSERT / UPDATE / DELETE / MERGE.
//   * Pipe INSERT via `ResolvedGeneralizedQueryStmt` +
//     `ResolvedPipeInsertScan` (pipe input in `insert_stmt()->query()`).

#include "absl/status/statusor.h"
#include "backend/engine/engine.h"
#include "backend/storage/storage.h"

namespace googlesql {
class Catalog;
class ResolvedStatement;
}  // namespace googlesql

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace semantic {
namespace dml {

// Top-level dispatch. Returns the `DmlStats` the gateway folds
// into the BigQuery REST `dmlStats` / `numDmlAffectedRows`
// envelope. `storage` must outlive the call and must back every
// target table the analyzer resolved on `stmt`. Passing a null
// `storage` is a programming error (FAILED_PRECONDITION) -- the
// coordinator only routes here for resolved DML statements that
// reference at least one storage-backed table.
absl::StatusOr<DmlResult> ExecuteDml(const QueryRequest& request,
                                     const ::googlesql::ResolvedStatement& stmt,
                                     ::googlesql::Catalog* catalog,
                                     storage::Storage* storage);

}  // namespace dml
}  // namespace semantic
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator

#endif  // BIGQUERY_EMULATOR_BACKEND_ENGINE_SEMANTIC_DML_DML_EXECUTOR_H_
