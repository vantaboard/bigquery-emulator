#ifndef BIGQUERY_EMULATOR_BACKEND_ENGINE_SEMANTIC_DML_DML_EXECUTOR_H_
#define BIGQUERY_EMULATOR_BACKEND_ENGINE_SEMANTIC_DML_DML_EXECUTOR_H_

// Storage-aware local DML executor.
//
// Owns the DML statement shapes the DuckDB fast path cannot lower
// cleanly, per `.cursor/plans/local-exec-14-dml-system.plan.md`:
//
//   * `ResolvedInsertStmt` -- INSERT VALUES (single + multi-row).
//     INSERT ... SELECT is recognized but currently surfaces
//     `kNotImplemented` until the SELECT-streaming family lands.
//   * `ResolvedDeleteStmt` -- `DELETE FROM t WHERE <pred>`. Walks
//     the storage row source, evaluates `<pred>` per row, hands
//     the kept rows back through `Storage::OverwriteRows`.
//   * `ResolvedUpdateStmt` -- `UPDATE t SET col = <expr> WHERE
//     <pred>` (scalar SET only; deep-STRUCT mutation via
//     `Value::WithField` is owned by Family 4 of the plan).
//
// The executor reads + mutates storage through the
// `backend::storage::Storage*` the coordinator holds, so the
// catalog needs to back the target table with a
// `backend::catalog::StorageTable` (the production
// `GoogleSqlCatalog` does this; tests fake the same shape).
//
// Statements outside the supported subset (INSERT ... SELECT,
// MERGE harder branches, RETURNING, deep-STRUCT UPDATE,
// `ResolvedPipeInsertScan`) surface a structured
// `SemanticErrorReason::kNotImplemented` so the gateway envelope
// stays the same as for any other "planned but not landed" route.

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
absl::StatusOr<DmlStats> ExecuteDml(const QueryRequest& request,
                                    const ::googlesql::ResolvedStatement& stmt,
                                    ::googlesql::Catalog* catalog,
                                    storage::Storage* storage);

}  // namespace dml
}  // namespace semantic
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator

#endif  // BIGQUERY_EMULATOR_BACKEND_ENGINE_SEMANTIC_DML_DML_EXECUTOR_H_
