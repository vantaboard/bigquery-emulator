#ifndef BIGQUERY_EMULATOR_FRONTEND_HANDLERS_QUERY_H_
#define BIGQUERY_EMULATOR_FRONTEND_HANDLERS_QUERY_H_

#include <grpcpp/grpcpp.h>

#include "backend/storage/storage.h"
#include "proto/emulator.grpc.pb.h"

namespace bigquery_emulator {
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
//     instance. May be null on builds that compile without GoogleSQL
//     linked in (the legacy CMake target), in which case `DryRun`
//     and `ExecuteQuery` both surface `UNIMPLEMENTED`.
//
// DryRun (Phase 4b) parses + analyzes the SQL via `googlesql::Analyzer`
// and returns the resolved output schema + an estimated bytes-
// processed value the gateway folds into the BigQuery REST
// `Job.statistics.query` response. Analysis errors surface as
// `INVALID_ARGUMENT` with a `line:column:` prefix on the message so
// the gateway can map them to BigQuery's HTTP 400
// `reason: invalidQuery` envelope.
//
// ExecuteQuery is still a Phase 5 stub.
class QueryService final : public v1::Query::Service {
 public:
  explicit QueryService(backend::storage::Storage* storage = nullptr);

  ::grpc::Status DryRun(::grpc::ServerContext* context,
                        const v1::QueryRequest* request,
                        v1::DryRunResponse* response) override;

  ::grpc::Status ExecuteQuery(
      ::grpc::ServerContext* context, const v1::QueryRequest* request,
      ::grpc::ServerWriter<v1::QueryResultRow>* writer) override;

 private:
  backend::storage::Storage* storage_;
};

}  // namespace frontend
}  // namespace bigquery_emulator

#endif  // BIGQUERY_EMULATOR_FRONTEND_HANDLERS_QUERY_H_
