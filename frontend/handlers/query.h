#ifndef BIGQUERY_EMULATOR_FRONTEND_HANDLERS_QUERY_H_
#define BIGQUERY_EMULATOR_FRONTEND_HANDLERS_QUERY_H_

#include <grpcpp/grpcpp.h>

#include <functional>

#include "backend/storage/storage.h"
#include "proto/emulator.grpc.pb.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
class Engine;
}  // namespace engine
}  // namespace backend
namespace frontend {

// QueryService is the C++ engine's implementation of the
// bigquery_emulator.v1.Query gRPC service. The Go gateway forwards
// `bigquery.jobs.query` (and the query branch of `bigquery.jobs.insert`)
// here so GoogleSQL can analyze and execute the SQL.
//
// Lifetime:
//
//   * `storage` is the `Storage` the analyzer's catalog adapter
//     consults during name resolution. It must outlive the service
//     instance.
//
// DryRun (Phase 4b) parses + analyzes the SQL via `googlesql::Analyzer`
// and returns the resolved output schema + an estimated bytes-
// processed value the gateway folds into the BigQuery REST
// `Job.statistics.query` response. Analysis errors surface as
// `INVALID_ARGUMENT` with a `line:column:` prefix on the message so
// the gateway can map them to BigQuery's HTTP 400
// `reason: invalidQuery` envelope.
//
// ExecuteQuery drives the DuckDB engine and streams the resolved
// schema as the first `QueryResultRow` followed by one
// `QueryResultRow` per result row. The `ExecuteQuery` gRPC handler is
// a thin shim over `StreamQueryResults` (below) so unit tests can
// exercise the streaming logic without a real `grpc::ServerWriter`.
class QueryService final : public v1::Query::Service {
 public:
  // `storage` is the catalog adapter's backing store (used to
  // materialize `googlesql::Table*`s for name resolution). `engine`
  // is the execution backend the handler forwards `DryRun` /
  // `ExecuteQuery` to. Both pointers must be non-null at startup;
  // `binaries/emulator_main/main.cc` constructs them and threads
  // them through. `ExecuteQuery` returns `FAILED_PRECONDITION` when
  // `engine` is null.
  explicit QueryService(backend::storage::Storage* storage = nullptr,
                         backend::engine::Engine* engine = nullptr);

  ::grpc::Status DryRun(::grpc::ServerContext* context,
                        const v1::QueryRequest* request,
                        v1::DryRunResponse* response) override;

  ::grpc::Status ExecuteQuery(
      ::grpc::ServerContext* context, const v1::QueryRequest* request,
      ::grpc::ServerWriter<v1::QueryResultRow>* writer) override;

 private:
  backend::storage::Storage* storage_;
  backend::engine::Engine* engine_;  // not owned; may be null
};

// Executes `request` against the reference-impl engine and emits the
// result through `write`. The first emitted message carries the
// resolved output schema (with `cells` empty); every subsequent
// message carries one result row (with `schema` unset).
//
// `write(msg)` is invoked once per emitted message. It must return
// `true` if the message was accepted by the downstream consumer and
// `false` to abort the stream early (mirroring the
// `grpc::ServerWriter::Write` contract). A `false` return is reported
// back to the caller as `CANCELLED`.
//
// Mirrors the validation rules in `QueryService::DryRun`:
//
//   * `storage` must be non-null (`FAILED_PRECONDITION` otherwise).
//   * `request.use_legacy_sql` is rejected (`INVALID_ARGUMENT`).
//   * `request.project_id` and `request.sql` are required.
//
// The gRPC handler wraps this helper with a one-line lambda; tests
// call it directly with a capturing lambda and inspect the emitted
// messages.
//
// `engine` is the execution backend to forward to. The handler
// returns `FAILED_PRECONDITION` when the engine is null; the
// `binaries/emulator_main` wire path and unit tests construct a
// `DuckDBEngine` explicitly.
::grpc::Status StreamQueryResults(
    backend::storage::Storage* storage, const v1::QueryRequest& request,
    const std::function<bool(const v1::QueryResultRow&)>& write,
    backend::engine::Engine* engine = nullptr);

}  // namespace frontend
}  // namespace bigquery_emulator

#endif  // BIGQUERY_EMULATOR_FRONTEND_HANDLERS_QUERY_H_
