#include "frontend/handlers/query.h"

namespace bigquery_emulator {
namespace frontend {

namespace {

constexpr char kAnalysisUnimplemented[] =
    "Query.DryRun is not implemented yet (Phase 4 of ROADMAP.md)";

constexpr char kExecutionUnimplemented[] =
    "Query.ExecuteQuery is not implemented yet (Phase 5 of ROADMAP.md)";

}  // namespace

::grpc::Status QueryService::DryRun(::grpc::ServerContext* /*context*/,
                                    const v1::QueryRequest* /*request*/,
                                    v1::DryRunResponse* /*response*/) {
  return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED,
                        kAnalysisUnimplemented);
}

::grpc::Status QueryService::ExecuteQuery(
    ::grpc::ServerContext* /*context*/, const v1::QueryRequest* /*request*/,
    ::grpc::ServerWriter<v1::QueryResultRow>* /*writer*/) {
  return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED,
                        kExecutionUnimplemented);
}

}  // namespace frontend
}  // namespace bigquery_emulator
