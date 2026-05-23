#ifndef BIGQUERY_EMULATOR_FRONTEND_HANDLERS_QUERY_H_
#define BIGQUERY_EMULATOR_FRONTEND_HANDLERS_QUERY_H_

#include <grpcpp/grpcpp.h>

#include "emulator.grpc.pb.h"

namespace bigquery_emulator {
namespace frontend {

// QueryService is the C++ engine's implementation of the
// bigquery_emulator.v1.Query gRPC service. The Go gateway forwards
// `bigquery.jobs.query` (and the query branch of `bigquery.jobs.insert`)
// here so GoogleSQL can analyze and execute the SQL.
//
// Phase 2b ships UNIMPLEMENTED stubs so the gateway can wire up its
// client and gracefully surface "engine not ready yet" to BigQuery
// callers. Real analysis lands in Phase 4 and real execution in
// Phase 5 (see ROADMAP.md).
class QueryService final : public v1::Query::Service {
 public:
  ::grpc::Status DryRun(::grpc::ServerContext* context,
                        const v1::QueryRequest* request,
                        v1::DryRunResponse* response) override;

  ::grpc::Status ExecuteQuery(
      ::grpc::ServerContext* context, const v1::QueryRequest* request,
      ::grpc::ServerWriter<v1::QueryResultRow>* writer) override;
};

}  // namespace frontend
}  // namespace bigquery_emulator

#endif  // BIGQUERY_EMULATOR_FRONTEND_HANDLERS_QUERY_H_
