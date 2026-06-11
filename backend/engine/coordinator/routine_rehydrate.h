#ifndef BIGQUERY_EMULATOR_BACKEND_ENGINE_COORDINATOR_ROUTINE_REHYDRATE_H_
#define BIGQUERY_EMULATOR_BACKEND_ENGINE_COORDINATOR_ROUTINE_REHYDRATE_H_

#include "absl/status/status.h"
#include "backend/storage/storage.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace coordinator {

// Replays every non-temp persisted routine into the per-project
// registries. Called once at engine startup.
absl::Status RehydrateRoutinesFromStorage(storage::Storage* storage);

// Replays a single persisted routine row into the in-memory registries.
absl::Status RehydrateRoutineRecord(storage::Storage* storage,
                                    const storage::RoutineRecord& record);

}  // namespace coordinator
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator

#endif  // BIGQUERY_EMULATOR_BACKEND_ENGINE_COORDINATOR_ROUTINE_REHYDRATE_H_
