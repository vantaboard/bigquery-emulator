#ifndef BIGQUERY_EMULATOR_FRONTEND_HANDLERS_CATALOG_H_
#define BIGQUERY_EMULATOR_FRONTEND_HANDLERS_CATALOG_H_

#include <grpcpp/grpcpp.h>

#include "emulator.grpc.pb.h"

namespace bigquery_emulator {
namespace frontend {

// CatalogService is the C++ engine's implementation of the
// bigquery_emulator.v1.Catalog gRPC service. It is the dataset/table
// catalog the Go gateway pokes whenever it sees REST mutations against
// the BigQuery resource model.
//
// Phase 2b ships an UNIMPLEMENTED stub for every RPC so the gRPC plumbing
// can be exercised end-to-end (health check, reflection, gateway client)
// before any real catalog state lands in Phase 3.
class CatalogService final : public v1::Catalog::Service {
 public:
  ::grpc::Status RegisterDataset(
      ::grpc::ServerContext* context,
      const v1::RegisterDatasetRequest* request,
      v1::RegisterDatasetResponse* response) override;

  ::grpc::Status DropDataset(
      ::grpc::ServerContext* context,
      const v1::DropDatasetRequest* request,
      v1::DropDatasetResponse* response) override;

  ::grpc::Status RegisterTable(
      ::grpc::ServerContext* context,
      const v1::RegisterTableRequest* request,
      v1::RegisterTableResponse* response) override;

  ::grpc::Status DropTable(
      ::grpc::ServerContext* context,
      const v1::DropTableRequest* request,
      v1::DropTableResponse* response) override;

  ::grpc::Status DescribeTable(
      ::grpc::ServerContext* context,
      const v1::DescribeTableRequest* request,
      v1::DescribeTableResponse* response) override;
};

}  // namespace frontend
}  // namespace bigquery_emulator

#endif  // BIGQUERY_EMULATOR_FRONTEND_HANDLERS_CATALOG_H_
