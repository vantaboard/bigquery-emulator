#include "frontend/handlers/catalog.h"

namespace bigquery_emulator {
namespace frontend {

namespace {

constexpr char kUnimplementedMessage[] =
    "Catalog service is not implemented yet (Phase 3 of ROADMAP.md)";

::grpc::Status Unimplemented() {
  return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED,
                        kUnimplementedMessage);
}

}  // namespace

::grpc::Status CatalogService::RegisterDataset(
    ::grpc::ServerContext* /*context*/,
    const v1::RegisterDatasetRequest* /*request*/,
    v1::RegisterDatasetResponse* /*response*/) {
  return Unimplemented();
}

::grpc::Status CatalogService::DropDataset(
    ::grpc::ServerContext* /*context*/,
    const v1::DropDatasetRequest* /*request*/,
    v1::DropDatasetResponse* /*response*/) {
  return Unimplemented();
}

::grpc::Status CatalogService::RegisterTable(
    ::grpc::ServerContext* /*context*/,
    const v1::RegisterTableRequest* /*request*/,
    v1::RegisterTableResponse* /*response*/) {
  return Unimplemented();
}

::grpc::Status CatalogService::DropTable(
    ::grpc::ServerContext* /*context*/,
    const v1::DropTableRequest* /*request*/,
    v1::DropTableResponse* /*response*/) {
  return Unimplemented();
}

::grpc::Status CatalogService::DescribeTable(
    ::grpc::ServerContext* /*context*/,
    const v1::DescribeTableRequest* /*request*/,
    v1::DescribeTableResponse* /*response*/) {
  return Unimplemented();
}

}  // namespace frontend
}  // namespace bigquery_emulator
