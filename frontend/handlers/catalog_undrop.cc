#include "frontend/handlers/catalog.h"

#include <string>

#include "absl/status/status.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/string_view.h"
#include "backend/engine/coordinator/routine_rehydrate.h"
#include "backend/engine/coordinator/view_rehydrate.h"
#include "backend/storage/duckdb/duckdb_storage.h"
#include "backend/storage/storage.h"
#include "frontend/handlers/handler_common.h"

namespace bigquery_emulator {
namespace frontend {

namespace {

::grpc::Status ValidateDatasetRef(const v1::DatasetRef& ref,
                                  absl::string_view rpc_name) {
  if (ref.project_id().empty()) {
    return ::grpc::Status(::grpc::StatusCode::INVALID_ARGUMENT,
                          absl::StrCat(rpc_name,
                                       ": dataset.project_id "
                                       "is required"));
  }
  if (ref.dataset_id().empty()) {
    return ::grpc::Status(::grpc::StatusCode::INVALID_ARGUMENT,
                          absl::StrCat(rpc_name,
                                       ": dataset.dataset_id "
                                       "is required"));
  }
  return ::grpc::Status::OK;
}

backend::storage::DatasetId DatasetIdFromProto(const v1::DatasetRef& ref) {
  return backend::storage::DatasetId{ref.project_id(), ref.dataset_id()};
}

absl::Status RehydrateDatasetCatalog(backend::storage::Storage* storage,
                                     const backend::storage::DatasetId& id) {
  auto routines_or = storage->ListRoutines(id);
  if (!routines_or.ok()) return routines_or.status();
  for (const backend::storage::RoutineRecord& rec : *routines_or) {
    if (rec.is_temp) continue;
    absl::Status reg = backend::engine::coordinator::RehydrateRoutineRecord(
        storage, rec);
    if (!reg.ok()) return reg;
  }
  auto views_or = storage->ListAllViews();
  if (!views_or.ok()) return views_or.status();
  for (const backend::storage::ViewRecord& rec : *views_or) {
    if (rec.id.project_id != id.project_id ||
        rec.id.dataset_id != id.dataset_id) {
      continue;
    }
    absl::Status reg =
        backend::engine::coordinator::RehydrateViewRecord(storage, rec);
    if (!reg.ok()) return reg;
  }
  return absl::OkStatus();
}

std::string RestMetadataFromDuckDBStorage(
    backend::storage::Storage* storage, const backend::storage::DatasetId& id) {
  auto* duckdb =
      dynamic_cast<backend::storage::duckdb::DuckDBStorage*>(storage);
  if (duckdb == nullptr) return {};
  auto meta_or = duckdb->GetDatasetRestMetadataJson(id);
  if (!meta_or.ok()) return {};
  return *meta_or;
}

}  // namespace

::grpc::Status CatalogService::UndeleteDataset(
    ::grpc::ServerContext* /*context*/,
    const v1::UndeleteDatasetRequest* request,
    v1::UndeleteDatasetResponse* response) {
  if (storage_ == nullptr) {
    return ::grpc::Status(::grpc::StatusCode::INTERNAL,
                          "CatalogService: storage backend is not configured");
  }
  if (auto v = ValidateDatasetRef(request->dataset(), "UndeleteDataset");
      !v.ok()) {
    return v;
  }
  const auto id = DatasetIdFromProto(request->dataset());
  absl::Status restored = storage_->RestoreDataset(id);
  if (!restored.ok()) {
    return internal::AbslToGrpcStatus(restored);
  }
  absl::Status rehydrated = RehydrateDatasetCatalog(storage_, id);
  if (!rehydrated.ok()) {
    return internal::AbslToGrpcStatus(rehydrated);
  }
  if (response != nullptr) {
    response->set_rest_metadata_json(
        RestMetadataFromDuckDBStorage(storage_, id));
  }
  return ::grpc::Status::OK;
}

}  // namespace frontend
}  // namespace bigquery_emulator
