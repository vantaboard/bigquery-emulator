#include "frontend/handlers/catalog.h"

#include <string>
#include <utility>

#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/string_view.h"
#include "backend/schema/schema.h"
#include "backend/storage/storage.h"

namespace bigquery_emulator {
namespace frontend {

namespace {

// Translates an `absl::Status` from the storage layer into the closest
// matching gRPC status the gateway / external clients can route on.
// Mirrors the table in `catalog.h`: only the four storage codes we
// actually surface today get a structured mapping; everything else
// degrades to INTERNAL so a misbehaving backend cannot be mistaken for
// a NOT_FOUND on the wire.
::grpc::Status AbslToGrpcStatus(const absl::Status& status) {
  if (status.ok()) return ::grpc::Status::OK;
  ::grpc::StatusCode code = ::grpc::StatusCode::INTERNAL;
  switch (status.code()) {
    case absl::StatusCode::kNotFound:
      code = ::grpc::StatusCode::NOT_FOUND;
      break;
    case absl::StatusCode::kAlreadyExists:
      code = ::grpc::StatusCode::ALREADY_EXISTS;
      break;
    case absl::StatusCode::kInvalidArgument:
      code = ::grpc::StatusCode::INVALID_ARGUMENT;
      break;
    case absl::StatusCode::kFailedPrecondition:
      code = ::grpc::StatusCode::FAILED_PRECONDITION;
      break;
    case absl::StatusCode::kUnimplemented:
      code = ::grpc::StatusCode::UNIMPLEMENTED;
      break;
    default:
      code = ::grpc::StatusCode::INTERNAL;
      break;
  }
  return ::grpc::Status(code, std::string(status.message()));
}

// Validates that a `DatasetRef` carries the two required identifiers.
// Returns an INVALID_ARGUMENT gRPC status describing which field is
// missing so callers don't have to guess at "empty DatasetRef" errors.
::grpc::Status ValidateDatasetRef(const v1::DatasetRef& ref,
                                   absl::string_view rpc_name) {
  if (ref.project_id().empty()) {
    return ::grpc::Status(::grpc::StatusCode::INVALID_ARGUMENT,
                          absl::StrCat(rpc_name, ": dataset.project_id "
                                                 "is required"));
  }
  if (ref.dataset_id().empty()) {
    return ::grpc::Status(::grpc::StatusCode::INVALID_ARGUMENT,
                          absl::StrCat(rpc_name, ": dataset.dataset_id "
                                                 "is required"));
  }
  return ::grpc::Status::OK;
}

// Validates that a `TableRef` carries the three required identifiers.
::grpc::Status ValidateTableRef(const v1::TableRef& ref,
                                 absl::string_view rpc_name) {
  if (ref.project_id().empty()) {
    return ::grpc::Status(::grpc::StatusCode::INVALID_ARGUMENT,
                          absl::StrCat(rpc_name,
                                       ": table.project_id is required"));
  }
  if (ref.dataset_id().empty()) {
    return ::grpc::Status(::grpc::StatusCode::INVALID_ARGUMENT,
                          absl::StrCat(rpc_name,
                                       ": table.dataset_id is required"));
  }
  if (ref.table_id().empty()) {
    return ::grpc::Status(::grpc::StatusCode::INVALID_ARGUMENT,
                          absl::StrCat(rpc_name,
                                       ": table.table_id is required"));
  }
  return ::grpc::Status::OK;
}

backend::storage::DatasetId DatasetIdFromProto(const v1::DatasetRef& ref) {
  return backend::storage::DatasetId{ref.project_id(), ref.dataset_id()};
}

backend::storage::TableId TableIdFromProto(const v1::TableRef& ref) {
  return backend::storage::TableId{ref.project_id(), ref.dataset_id(),
                                    ref.table_id()};
}

}  // namespace

CatalogService::CatalogService(backend::storage::Storage* storage)
    : storage_(storage) {}

::grpc::Status CatalogService::RegisterDataset(
    ::grpc::ServerContext* /*context*/,
    const v1::RegisterDatasetRequest* request,
    v1::RegisterDatasetResponse* /*response*/) {
  if (storage_ == nullptr) {
    return ::grpc::Status(::grpc::StatusCode::INTERNAL,
                          "CatalogService: storage backend is not configured");
  }
  if (auto v = ValidateDatasetRef(request->dataset(), "RegisterDataset");
      !v.ok()) {
    return v;
  }
  const auto id = DatasetIdFromProto(request->dataset());
  return AbslToGrpcStatus(storage_->CreateDataset(id, request->location()));
}

::grpc::Status CatalogService::DropDataset(
    ::grpc::ServerContext* /*context*/,
    const v1::DropDatasetRequest* request,
    v1::DropDatasetResponse* /*response*/) {
  if (storage_ == nullptr) {
    return ::grpc::Status(::grpc::StatusCode::INTERNAL,
                          "CatalogService: storage backend is not configured");
  }
  if (auto v = ValidateDatasetRef(request->dataset(), "DropDataset");
      !v.ok()) {
    return v;
  }
  const auto id = DatasetIdFromProto(request->dataset());
  return AbslToGrpcStatus(
      storage_->DropDataset(id, request->delete_contents()));
}

::grpc::Status CatalogService::RegisterTable(
    ::grpc::ServerContext* /*context*/,
    const v1::RegisterTableRequest* request,
    v1::RegisterTableResponse* /*response*/) {
  if (storage_ == nullptr) {
    return ::grpc::Status(::grpc::StatusCode::INTERNAL,
                          "CatalogService: storage backend is not configured");
  }
  if (auto v = ValidateTableRef(request->table(), "RegisterTable"); !v.ok()) {
    return v;
  }
  auto schema_or = backend::schema::TableSchemaFromProto(request->schema());
  if (!schema_or.ok()) {
    return AbslToGrpcStatus(schema_or.status());
  }
  const auto id = TableIdFromProto(request->table());
  return AbslToGrpcStatus(storage_->CreateTable(id, *schema_or));
}

::grpc::Status CatalogService::DropTable(
    ::grpc::ServerContext* /*context*/,
    const v1::DropTableRequest* request,
    v1::DropTableResponse* /*response*/) {
  if (storage_ == nullptr) {
    return ::grpc::Status(::grpc::StatusCode::INTERNAL,
                          "CatalogService: storage backend is not configured");
  }
  if (auto v = ValidateTableRef(request->table(), "DropTable"); !v.ok()) {
    return v;
  }
  const auto id = TableIdFromProto(request->table());
  return AbslToGrpcStatus(storage_->DropTable(id));
}

::grpc::Status CatalogService::DescribeTable(
    ::grpc::ServerContext* /*context*/,
    const v1::DescribeTableRequest* request,
    v1::DescribeTableResponse* response) {
  if (storage_ == nullptr) {
    return ::grpc::Status(::grpc::StatusCode::INTERNAL,
                          "CatalogService: storage backend is not configured");
  }
  if (auto v = ValidateTableRef(request->table(), "DescribeTable"); !v.ok()) {
    return v;
  }
  const auto id = TableIdFromProto(request->table());
  auto schema_or = storage_->GetSchema(id);
  if (!schema_or.ok()) {
    return AbslToGrpcStatus(schema_or.status());
  }
  backend::schema::TableSchemaToProto(*schema_or, response->mutable_schema());
  return ::grpc::Status::OK;
}

}  // namespace frontend
}  // namespace bigquery_emulator
