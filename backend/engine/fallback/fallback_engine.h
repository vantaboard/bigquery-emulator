#ifndef BIGQUERY_EMULATOR_BACKEND_ENGINE_FALLBACK_FALLBACK_ENGINE_H_
#define BIGQUERY_EMULATOR_BACKEND_ENGINE_FALLBACK_FALLBACK_ENGINE_H_

// FallbackEngine composes a primary `Engine` with a fallback `Engine`
// for the `--on_unknown_fn=fallback` policy. When the primary engine
// reports `UNIMPLEMENTED` (the contract a transpiler-driven engine
// uses to signal "this shape is not yet lowered"), the wrapper retries
// the same request against the fallback engine and surfaces its
// result instead.
//
// The wrapper is the engine factory's lever for the
// `--on_unknown_fn=fallback` flag in `binaries/emulator_main/main.cc`:
// `--engine=duckdb --on_unknown_fn=fallback` constructs a
// `FallbackEngine(primary=duckdb, fallback=reference_impl)` so that
// `SELECT 1`-style shapes the DuckDB transpiler does not yet emit
// transparently retry through the reference-impl evaluator.
//
// Status code policy:
//   * `kUnimplemented` returned by the primary is the only condition
//     that triggers a fallback. Every other status (analysis error,
//     storage error, internal error, ...) surfaces unchanged so the
//     fallback path does not mask real bugs in the primary engine.
//   * The fallback engine's status is surfaced verbatim when the
//     primary returned `UNIMPLEMENTED`; if the fallback also can't
//     handle the query, the caller sees the fallback's
//     `UNIMPLEMENTED`.
//
// Lifetime: both `primary` and `fallback` must outlive this wrapper.
// The wrapper does not own either; the engine factory owns the
// concrete engines and the wrapper itself.

#include <memory>

#include "absl/status/statusor.h"
#include "backend/engine/engine.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace fallback {

class FallbackEngine : public Engine {
 public:
  // `primary` is consulted first; `fallback` is consulted only when
  // `primary` returns `absl::StatusCode::kUnimplemented`. Both
  // pointers must be non-null and must outlive this wrapper.
  FallbackEngine(Engine* primary, Engine* fallback);
  ~FallbackEngine() override;

  FallbackEngine(const FallbackEngine&) = delete;
  FallbackEngine& operator=(const FallbackEngine&) = delete;

  absl::StatusOr<std::unique_ptr<AnalyzedQuery>> Analyze(
      const QueryRequest& request, googlesql::Catalog* catalog) override;

  absl::StatusOr<DryRunResult> DryRun(
      const QueryRequest& request, googlesql::Catalog* catalog) override;

  absl::StatusOr<std::unique_ptr<RowSource>> ExecuteQuery(
      const QueryRequest& request, googlesql::Catalog* catalog) override;

 private:
  Engine* primary_;   // not owned
  Engine* fallback_;  // not owned
};

}  // namespace fallback
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator

#endif  // BIGQUERY_EMULATOR_BACKEND_ENGINE_FALLBACK_FALLBACK_ENGINE_H_
