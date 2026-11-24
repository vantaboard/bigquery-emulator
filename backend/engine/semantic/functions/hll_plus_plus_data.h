#ifndef BIGQUERY_EMULATOR_BACKEND_ENGINE_SEMANTIC_FUNCTIONS_HLL_PLUS_PLUS_DATA_H_
#define BIGQUERY_EMULATOR_BACKEND_ENGINE_SEMANTIC_FUNCTIONS_HLL_PLUS_PLUS_DATA_H_

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace semantic {
namespace functions {
namespace hll_plus_plus {

// Empirical HLL++ constants, ported from zetasketch's
// `com.google.zetasketch.internal.hllplus.Data` so that estimates match
// BigQuery's `HLL_COUNT.EXTRACT` exactly.

// The alpha_m constant of the raw HLL estimate for the given precision.
double Alpha(int precision);

// The estimate at or below which linear counting replaces HLL.
int LinearCountingThreshold(int precision);

// The bias to subtract from a raw estimate: the inverse-distance weighted
// mean of the 6 nearest empirical biases, or 0 outside the measured range
// (precisions 4 to 18).
double EstimateBias(double estimate, int precision);

}  // namespace hll_plus_plus
}  // namespace functions
}  // namespace semantic
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator

#endif  // BIGQUERY_EMULATOR_BACKEND_ENGINE_SEMANTIC_FUNCTIONS_HLL_PLUS_PLUS_DATA_H_
