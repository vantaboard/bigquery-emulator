#ifndef BIGQUERY_EMULATOR_BACKEND_ENGINE_SEMANTIC_STUBS_ML_H_
#define BIGQUERY_EMULATOR_BACKEND_ENGINE_SEMANTIC_STUBS_ML_H_

// ML.* `local_stub` TVF family. BigQuery ML depends on Vertex AI /
// real model training + serving that a local emulator cannot model.
// `docs/ENGINE_POLICY.md` picks the `local_stub` posture for the
// three inference entry points client libraries routinely reference
// after a metadata-only `CREATE MODEL`:
//
//   * `ML.PREDICT(MODEL <id>, TABLE <input>)` — pass through every
//     input row and every input column; append the analyzer-resolved
//     predicted output column(s) as typed NULL placeholders.
//   * `ML.EVALUATE(MODEL <id> [, TABLE <input>])` — return a single
//     metrics row whose fields match the documented evaluate schema,
//     each value typed NULL.
//   * `ML.FORECAST(MODEL <id> [, STRUCT(...)])` — return a single
//     forecast-schema row whose fields match the documented forecast
//     output, each value typed NULL.
//
// The stubs are deliberately deterministic and side-effect-free: no
// model is loaded, no Vertex AI call is made, and an unregistered
// model name still produces a clean (non-crashing) placeholder result
// rather than a storage lookup failure. Values are explicitly NOT
// predictions — they exist only so a query containing ML.* does not
// fail and client libraries can parse the schema.
//
// TVFs route through `MaterializeTvfScan` in `eval_tvf.cc` (before
// the SQL TVF path) and the route classifier promotes any query
// containing a `local_stub`-marked ML.* name to `kLocalStub`.

#include <vector>

#include "absl/status/statusor.h"
#include "backend/engine/semantic/value.h"
#include "googlesql/resolved_ast/resolved_ast.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace semantic {
namespace stubs {

absl::StatusOr<std::vector<ColumnBindings>> MlPredictStub(
    const ::googlesql::ResolvedTVFScan& scan,
    const std::vector<ColumnBindings>& input_rows,
    const ::googlesql::ResolvedScan* input_scan);

absl::StatusOr<std::vector<ColumnBindings>> MlEvaluateStub(
    const ::googlesql::ResolvedTVFScan& scan);

absl::StatusOr<std::vector<ColumnBindings>> MlForecastStub(
    const ::googlesql::ResolvedTVFScan& scan);

}  // namespace stubs
}  // namespace semantic
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator

#endif  // BIGQUERY_EMULATOR_BACKEND_ENGINE_SEMANTIC_STUBS_ML_H_
