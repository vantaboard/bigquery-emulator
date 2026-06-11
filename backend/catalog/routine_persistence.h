#ifndef BIGQUERY_EMULATOR_BACKEND_CATALOG_ROUTINE_PERSISTENCE_H_
#define BIGQUERY_EMULATOR_BACKEND_CATALOG_ROUTINE_PERSISTENCE_H_

// Write-through + rehydrate helpers for durable UDF / TVF / procedure
// registries backed by `storage::Storage`.

#include "absl/status/status.h"
#include "absl/strings/string_view.h"
#include "backend/engine/engine.h"
#include "backend/storage/storage.h"
#include "googlesql/resolved_ast/resolved_ast.h"

namespace bigquery_emulator {
namespace backend {
namespace catalog {

// Maps a BigQuery routine name path to a stable storage id.
storage::RoutineId RoutineIdFromNamePath(
    const std::vector<std::string>& name_path,
    absl::string_view project_id,
    absl::string_view default_dataset_id);

// Serializes `ResolvedCreateFunctionStmt` argument metadata (including
// ANY TYPE markers) into JSON for REST round-trip.
std::string SerializeFunctionSignatureJson(
    const ::googlesql::ResolvedCreateFunctionStmt& create_fn);

// Persists a routine DDL row after in-memory registration succeeds.
// Skips temp routines and no-ops when `storage` is null.
absl::Status PersistRoutineDdl(storage::Storage* storage,
                               const engine::QueryRequest& request,
                               const ::googlesql::ResolvedStatement& stmt);

// Deletes a persisted routine row. No-ops when `storage` is null.
absl::Status DeletePersistedRoutine(storage::Storage* storage,
                                    const storage::RoutineId& id);

}  // namespace catalog
}  // namespace backend
}  // namespace bigquery_emulator

#endif  // BIGQUERY_EMULATOR_BACKEND_CATALOG_ROUTINE_PERSISTENCE_H_
