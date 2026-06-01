#ifndef BIGQUERY_EMULATOR_BACKEND_ENGINE_SEMANTIC_FUNCTIONS_NUMERIC_EDGES_H_
#define BIGQUERY_EMULATOR_BACKEND_ENGINE_SEMANTIC_FUNCTIONS_NUMERIC_EDGES_H_

// Numeric-edges family: BigQuery scalar functions whose contract
// diverges from any thin DuckDB wrapper on a numeric corner case.
// Each declaration below pins the BigQuery contract in C++ so the
// semantic executor (not DuckDB) owns the answer.
//
//   * `BitCount(INT64) -> INT64` -- popcount on the two's-complement
//     representation. Documented as "the number of bits set to 1 in
//     the input". Negative integers have the high-order bits set
//     (e.g. `BIT_COUNT(-1) == 64`). DuckDB v1.5.3's `bit_count`
//     accepts only unsigned types, so the negative-input case
//     cannot route through a thin macro without an explicit cast
//     that loses BigQuery's two's-complement contract.
//
//   * `IeeeDivide(FLOAT64, FLOAT64) -> FLOAT64` -- IEEE 754
//     division that NEVER errors. `1.0 / 0.0` -> `+Inf`,
//     `-1.0 / 0.0` -> `-Inf`, `0.0 / 0.0` -> `NaN`. BigQuery's
//     SAFE_DIVIDE returns NULL on division-by-zero; IEEE_DIVIDE
//     returns the IEEE 754 sentinel. DuckDB's `/` raises on `/0`
//     for floats (it does NOT honor IEEE 754 by default), so the
//     thin-macro route cannot match.

#include <cstdint>
#include <vector>

#include "absl/status/statusor.h"
#include "backend/engine/semantic/value.h"
#include "googlesql/public/type.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace semantic {
namespace functions {

// BIT_COUNT: returns the count of bits set to 1 in the
// two's-complement representation of an INT64. NULL input
// propagates to NULL output. Non-INT64 input surfaces an
// INVALID_ARGUMENT status; the analyzer typically inserts an
// implicit cast for INT32 but BIGNUMERIC / NUMERIC stay as-is so a
// caller passing the wrong type surfaces a clean failure.
absl::StatusOr<Value> BitCount(const std::vector<Value>& args);

// IEEE_DIVIDE: IEEE 754 floating-point division that never errors.
// NULL operands propagate to NULL FLOAT64 output. Non-FLOAT64
// arguments surface INVALID_ARGUMENT (the analyzer should have
// inserted explicit casts).
absl::StatusOr<Value> IeeeDivide(const std::vector<Value>& args);

}  // namespace functions
}  // namespace semantic
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator

#endif  // BIGQUERY_EMULATOR_BACKEND_ENGINE_SEMANTIC_FUNCTIONS_NUMERIC_EDGES_H_
