#ifndef BIGQUERY_EMULATOR_BACKEND_ENGINE_REFERENCE_IMPL_REFERENCE_IMPL_ENGINE_H_
#define BIGQUERY_EMULATOR_BACKEND_ENGINE_REFERENCE_IMPL_REFERENCE_IMPL_ENGINE_H_

// ReferenceImplEngine is the Phase 3c scaffold for the GoogleSQL
// reference-impl backend. Every `Engine` method returns
// `absl::UnimplementedError` so the CLI factory in
// `binaries/emulator_main/main.cc` can already construct the engine
// while the real wiring (`Algebrizer` + `Evaluator`, see ROADMAP
// Phase 5.A) lands in a later plan.
//
// The constructor takes a non-owning `Storage*` because Phase 5.A will
// adapt the active storage backend into a `googlesql::Table` source so
// the reference impl can stream rows out of either the in-memory store
// or the DuckDB store. We thread the pointer through now so the
// scaffold compiles against the real interface and not a stub.

#include <memory>

#include "absl/status/statusor.h"
#include "backend/engine/engine.h"
#include "backend/storage/storage.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace reference_impl {

class ReferenceImplEngine : public Engine {
 public:
  // `storage` must outlive this engine instance. The scaffold does not
  // dereference the pointer; Phase 5.A wires it through a
  // `googlesql::Table` adapter.
  explicit ReferenceImplEngine(storage::Storage* storage);
  ~ReferenceImplEngine() override;

  ReferenceImplEngine(const ReferenceImplEngine&) = delete;
  ReferenceImplEngine& operator=(const ReferenceImplEngine&) = delete;

  absl::StatusOr<std::unique_ptr<AnalyzedQuery>> Analyze(
      const QueryRequest& request, googlesql::Catalog* catalog) override;

  absl::StatusOr<DryRunResult> DryRun(
      const QueryRequest& request, googlesql::Catalog* catalog) override;

  absl::StatusOr<std::unique_ptr<RowSource>> ExecuteQuery(
      const QueryRequest& request, googlesql::Catalog* catalog) override;

 private:
  storage::Storage* storage_;  // not owned
};

}  // namespace reference_impl
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator

#endif  // BIGQUERY_EMULATOR_BACKEND_ENGINE_REFERENCE_IMPL_REFERENCE_IMPL_ENGINE_H_
