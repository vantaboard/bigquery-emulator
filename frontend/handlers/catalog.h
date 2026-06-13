#ifndef BIGQUERY_EMULATOR_FRONTEND_HANDLERS_CATALOG_H_
#define BIGQUERY_EMULATOR_FRONTEND_HANDLERS_CATALOG_H_

#include <grpcpp/grpcpp.h>

#include "backend/storage/storage.h"
#include "proto/emulator.grpc.pb.h"

namespace bigquery_emulator {
namespace frontend {

// CatalogService is the C++ engine's implementation of the
// bigquery_emulator.v1.Catalog gRPC service. It is the dataset/table
// catalog the Go gateway pokes whenever it sees REST mutations against
// the BigQuery resource model.
//
// Each RPC is wired through a `backend::storage::Storage` instance:
// dataset / table CRUD calls delegate straight to the store, and the
// proto request/response shapes are translated to and from the
// engine-agnostic `backend::schema` structs at the service boundary.
// `Storage` errors map to gRPC status codes via `AbslToGrpcStatus` in
// `catalog.cc` (see also `catalog-errors` in the matching plan):
//
//   absl::NotFound          -> grpc::NOT_FOUND
//   absl::AlreadyExists     -> grpc::ALREADY_EXISTS
//   absl::InvalidArgument   -> grpc::INVALID_ARGUMENT
//   absl::FailedPrecondition-> grpc::FAILED_PRECONDITION
//   anything else           -> grpc::INTERNAL
//
// The service does not own the storage pointer; the caller (typically
// `Server::Create`) keeps the `Storage` alive for the gRPC server's
// lifetime. `storage` must be non-null.
class CatalogService final : public v1::Catalog::Service {
 public:
  explicit CatalogService(backend::storage::Storage* storage);

  ::grpc::Status RegisterDataset(
      ::grpc::ServerContext* context,
      const v1::RegisterDatasetRequest* request,
      v1::RegisterDatasetResponse* response) override;

  ::grpc::Status DropDataset(::grpc::ServerContext* context,
                             const v1::DropDatasetRequest* request,
                             v1::DropDatasetResponse* response) override;

  ::grpc::Status ListDatasets(::grpc::ServerContext* context,
                              const v1::ListDatasetsRequest* request,
                              v1::ListDatasetsResponse* response) override;

  ::grpc::Status RegisterTable(::grpc::ServerContext* context,
                               const v1::RegisterTableRequest* request,
                               v1::RegisterTableResponse* response) override;

  ::grpc::Status DropTable(::grpc::ServerContext* context,
                           const v1::DropTableRequest* request,
                           v1::DropTableResponse* response) override;

  ::grpc::Status ListTables(::grpc::ServerContext* context,
                            const v1::ListTablesRequest* request,
                            v1::ListTablesResponse* response) override;

  ::grpc::Status DescribeTable(::grpc::ServerContext* context,
                               const v1::DescribeTableRequest* request,
                               v1::DescribeTableResponse* response) override;

  ::grpc::Status InsertRows(::grpc::ServerContext* context,
                            const v1::InsertRowsRequest* request,
                            v1::InsertRowsResponse* response) override;

  ::grpc::Status ListRows(::grpc::ServerContext* context,
                          const v1::ListRowsRequest* request,
                          v1::ListRowsResponse* response) override;

  ::grpc::Status ListRoutines(::grpc::ServerContext* context,
                              const v1::ListRoutinesRequest* request,
                              v1::ListRoutinesResponse* response) override;

  ::grpc::Status GetRoutine(::grpc::ServerContext* context,
                            const v1::GetRoutineRequest* request,
                            v1::GetRoutineResponse* response) override;

  ::grpc::Status UpsertRoutine(::grpc::ServerContext* context,
                               const v1::UpsertRoutineRequest* request,
                               v1::UpsertRoutineResponse* response) override;

  ::grpc::Status DeleteRoutine(::grpc::ServerContext* context,
                               const v1::DeleteRoutineRequest* request,
                               v1::DeleteRoutineResponse* response) override;

  ::grpc::Status UpsertRowAccessPolicy(
      ::grpc::ServerContext* context,
      const v1::UpsertRowAccessPolicyRequest* request,
      v1::UpsertRowAccessPolicyResponse* response) override;

  ::grpc::Status DeleteRowAccessPolicy(
      ::grpc::ServerContext* context,
      const v1::DeleteRowAccessPolicyRequest* request,
      v1::DeleteRowAccessPolicyResponse* response) override;

  ::grpc::Status ListRowAccessPolicies(
      ::grpc::ServerContext* context,
      const v1::ListRowAccessPoliciesRequest* request,
      v1::ListRowAccessPoliciesResponse* response) override;

  ::grpc::Status SetColumnGovernance(
      ::grpc::ServerContext* context,
      const v1::SetColumnGovernanceRequest* request,
      v1::SetColumnGovernanceResponse* response) override;

 private:
  backend::storage::Storage* storage_;
};

}  // namespace frontend
}  // namespace bigquery_emulator

#endif  // BIGQUERY_EMULATOR_FRONTEND_HANDLERS_CATALOG_H_
