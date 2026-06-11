#include "frontend/handlers/catalog.h"

#include "frontend/handlers/handler_common.h"

#include <string>
#include <vector>

#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "absl/strings/str_cat.h"
#include "backend/catalog/procedure_registry.h"
#include "backend/catalog/routine_persistence.h"
#include "backend/catalog/tvf_registry.h"
#include "backend/catalog/udf_registry.h"
#include "backend/engine/coordinator/routine_rehydrate.h"
#include "backend/storage/storage.h"

namespace bigquery_emulator {
namespace frontend {

namespace {

::grpc::Status ValidateRoutineRef(const v1::RoutineRef& ref,
                                  absl::string_view rpc_name) {
  if (ref.project_id().empty() || ref.dataset_id().empty() ||
      ref.routine_id().empty()) {
    return ::grpc::Status(
        ::grpc::StatusCode::INVALID_ARGUMENT,
        absl::StrCat(rpc_name,
                     ": routine ref requires project_id, dataset_id, "
                     "routine_id"));
  }
  return ::grpc::Status::OK;
}

backend::storage::RoutineId RoutineIdFromProto(const v1::RoutineRef& ref) {
  return backend::storage::RoutineId{ref.project_id(), ref.dataset_id(),
                                     ref.routine_id()};
}

std::string RoutineTypeFromKind(backend::storage::RoutineKind kind) {
  switch (kind) {
    case backend::storage::RoutineKind::kScalarFunction:
      return "SCALAR_FUNCTION";
    case backend::storage::RoutineKind::kAggregateFunction:
      return "AGGREGATE_FUNCTION";
    case backend::storage::RoutineKind::kTableValuedFunction:
      return "TABLE_VALUED_FUNCTION";
    case backend::storage::RoutineKind::kProcedure:
      return "PROCEDURE";
  }
  return "SCALAR_FUNCTION";
}

backend::storage::RoutineKind RoutineKindFromType(absl::string_view type) {
  if (type == "AGGREGATE_FUNCTION") {
    return backend::storage::RoutineKind::kAggregateFunction;
  }
  if (type == "TABLE_VALUED_FUNCTION") {
    return backend::storage::RoutineKind::kTableValuedFunction;
  }
  if (type == "PROCEDURE") {
    return backend::storage::RoutineKind::kProcedure;
  }
  return backend::storage::RoutineKind::kScalarFunction;
}

void RecordToProto(const backend::storage::RoutineRecord& rec,
                   v1::RoutineDescriptor* out) {
  out->mutable_routine()->set_project_id(rec.id.project_id);
  out->mutable_routine()->set_dataset_id(rec.id.dataset_id);
  out->mutable_routine()->set_routine_id(rec.id.routine_id);
  out->set_routine_type(RoutineTypeFromKind(rec.kind));
  out->set_language(rec.language);
  out->set_ddl_sql(rec.ddl_sql);
  out->set_signature_json(rec.signature_json);
}

backend::storage::RoutineRecord RecordFromProto(
    const v1::RoutineDescriptor& desc) {
  backend::storage::RoutineRecord rec;
  rec.id = RoutineIdFromProto(desc.routine());
  rec.kind = RoutineKindFromType(desc.routine_type());
  rec.language = desc.language().empty() ? "SQL" : desc.language();
  rec.ddl_sql = desc.ddl_sql();
  rec.signature_json = desc.signature_json();
  return rec;
}

}  // namespace

::grpc::Status CatalogService::ListRoutines(
    ::grpc::ServerContext* /*context*/,
    const v1::ListRoutinesRequest* request,
    v1::ListRoutinesResponse* response) {
  if (storage_ == nullptr) {
    return ::grpc::Status(::grpc::StatusCode::INTERNAL,
                          "CatalogService: storage backend is not configured");
  }
  if (request->dataset().project_id().empty() ||
      request->dataset().dataset_id().empty()) {
    return ::grpc::Status(::grpc::StatusCode::INVALID_ARGUMENT,
                          "ListRoutines: dataset ref is required");
  }
  backend::storage::DatasetId ds{request->dataset().project_id(),
                                 request->dataset().dataset_id()};
  auto rows_or = storage_->ListRoutines(ds);
  if (!rows_or.ok()) {
    return internal::AbslToGrpcStatus(rows_or.status());
  }
  for (const auto& rec : *rows_or) {
    RecordToProto(rec, response->add_routines());
  }
  return ::grpc::Status::OK;
}

::grpc::Status CatalogService::GetRoutine(
    ::grpc::ServerContext* /*context*/,
    const v1::GetRoutineRequest* request,
    v1::GetRoutineResponse* response) {
  if (storage_ == nullptr) {
    return ::grpc::Status(::grpc::StatusCode::INTERNAL,
                          "CatalogService: storage backend is not configured");
  }
  if (auto v = ValidateRoutineRef(request->routine(), "GetRoutine"); !v.ok()) {
    return v;
  }
  auto rec_or = storage_->GetRoutine(RoutineIdFromProto(request->routine()));
  if (!rec_or.ok()) {
    return internal::AbslToGrpcStatus(rec_or.status());
  }
  RecordToProto(*rec_or, response->mutable_routine());
  return ::grpc::Status::OK;
}

::grpc::Status CatalogService::UpsertRoutine(
    ::grpc::ServerContext* /*context*/,
    const v1::UpsertRoutineRequest* request,
    v1::UpsertRoutineResponse* /*response*/) {
  if (storage_ == nullptr) {
    return ::grpc::Status(::grpc::StatusCode::INTERNAL,
                          "CatalogService: storage backend is not configured");
  }
  if (request->routine().ddl_sql().empty()) {
    return ::grpc::Status(::grpc::StatusCode::INVALID_ARGUMENT,
                          "UpsertRoutine: ddl_sql is required");
  }
  backend::storage::RoutineRecord rec = RecordFromProto(request->routine());
  absl::Status stored = storage_->UpsertRoutine(rec);
  if (!stored.ok()) {
    return internal::AbslToGrpcStatus(stored);
  }
  return internal::AbslToGrpcStatus(
      backend::engine::coordinator::RehydrateRoutineRecord(storage_, rec));
}

::grpc::Status CatalogService::DeleteRoutine(
    ::grpc::ServerContext* /*context*/,
    const v1::DeleteRoutineRequest* request,
    v1::DeleteRoutineResponse* /*response*/) {
  if (storage_ == nullptr) {
    return ::grpc::Status(::grpc::StatusCode::INTERNAL,
                          "CatalogService: storage backend is not configured");
  }
  if (auto v = ValidateRoutineRef(request->routine(), "DeleteRoutine");
      !v.ok()) {
    return v;
  }
  const backend::storage::RoutineId id =
      RoutineIdFromProto(request->routine());
  absl::Status dropped =
      backend::catalog::DropProjectFunction(id.project_id, id.routine_id);
  if (!dropped.ok()) {
    dropped = backend::catalog::DropProjectTvf(id.project_id, id.routine_id);
  }
  if (!dropped.ok()) {
    dropped =
        backend::catalog::DropProjectProcedure(id.project_id, id.routine_id);
  }
  return internal::AbslToGrpcStatus(
      backend::catalog::DeletePersistedRoutine(storage_, id));
}

}  // namespace frontend
}  // namespace bigquery_emulator
