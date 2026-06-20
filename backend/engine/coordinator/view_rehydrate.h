#ifndef BIGQUERY_EMULATOR_BACKEND_ENGINE_COORDINATOR_VIEW_REHYDRATE_H_
#define BIGQUERY_EMULATOR_BACKEND_ENGINE_COORDINATOR_VIEW_REHYDRATE_H_

#include "absl/status/status.h"
#include "backend/storage/storage.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace coordinator {

absl::Status RehydrateViewRecord(storage::Storage* storage,
                                 const storage::ViewRecord& record);

absl::Status RehydrateViewsFromStorage(storage::Storage* storage);

}  // namespace coordinator
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator

#endif  // BIGQUERY_EMULATOR_BACKEND_ENGINE_COORDINATOR_VIEW_REHYDRATE_H_
