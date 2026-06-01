#include "backend/engine/coordinator/executor.h"

// Out-of-line definition for the abstract `Executor` destructor.
// Same rationale as the `Engine` / `AnalyzedQuery` / `RowSource`
// anchors in `backend/engine/engine.cc`: pin the v-table to a
// single .o so `-Wweak-vtables` does not fire on the concrete
// executors that inherit from this interface.

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace coordinator {

Executor::~Executor() = default;

}  // namespace coordinator
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
