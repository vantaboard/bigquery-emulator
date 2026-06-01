// String-family functions evaluated by the semantic executor.
//
// These functions either have no thin DuckDB equivalent
// (`SOUNDEX` — DuckDB v1.5.3 does not ship the scalar at all) or
// expose a variadic surface DuckDB cannot model with a macro
// wrapper (`INSTR` — BigQuery accepts 2..4 args with negative
// `position` and Nth `occurrence`). The dispatch entries live in
// `backend/engine/semantic/functions/dispatch.cc`; the
// `functions.yaml` rows route the analyzer here.

#ifndef BIGQUERY_EMULATOR_BACKEND_ENGINE_SEMANTIC_FUNCTIONS_STRING_FUNCS_H_
#define BIGQUERY_EMULATOR_BACKEND_ENGINE_SEMANTIC_FUNCTIONS_STRING_FUNCS_H_

#include <vector>

#include "absl/status/statusor.h"
#include "backend/engine/semantic/value.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace semantic {
namespace functions {

// SOUNDEX(value STRING) -> STRING(4).
//
// Returns the 4-character classic SOUNDEX code (first letter
// uppercased + 3 digits, zero-padded). NULL input yields
// `NullString`. Empty / no-letter input yields the input
// verbatim (matches BigQuery's observed behavior).
absl::StatusOr<Value> Soundex(const std::vector<Value>& args);

// INSTR(value STRING, subvalue STRING [, position INT64 [, occurrence INT64]])
// -> INT64.
//
// Returns the 1-based byte index of the Nth occurrence of
// `subvalue` in `value`. `position` defaults to 1; negative
// positions search right-to-left. `occurrence` defaults to 1 and
// MUST be positive. `position` MUST be non-zero. Returns 0 when
// the substring is not found.
//
// NULL in any argument propagates to `NullInt64`.
absl::StatusOr<Value> Instr(const std::vector<Value>& args);

}  // namespace functions
}  // namespace semantic
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator

#endif  // BIGQUERY_EMULATOR_BACKEND_ENGINE_SEMANTIC_FUNCTIONS_STRING_FUNCS_H_
