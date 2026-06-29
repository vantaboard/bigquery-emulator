#include <grpcpp/server_context.h>
#include <grpcpp/support/status.h>

#include <string>

#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/string_view.h"
#include "backend/storage/storage.h"
#include "backend/storage/table_governance.h"
#include "frontend/handlers/catalog.h"
#include "frontend/handlers/handler_common.h"
#include "proto/emulator.pb.h"

namespace bigquery_emulator {
namespace frontend {

namespace {

::grpc::Status ValidateTableRef(const v1::TableRef& ref,
                                absl::string_view rpc_name) {
  if (ref.project_id().empty()) {
    return ::grpc::Status(
        ::grpc::StatusCode::INVALID_ARGUMENT,
        absl::StrCat(rpc_name, ": table.project_id is required"));
  }
  if (ref.dataset_id().empty()) {
    return ::grpc::Status(
        ::grpc::StatusCode::INVALID_ARGUMENT,
        absl::StrCat(rpc_name, ": table.dataset_id is required"));
  }
  if (ref.table_id().empty()) {
    return ::grpc::Status(
        ::grpc::StatusCode::INVALID_ARGUMENT,
        absl::StrCat(rpc_name, ": table.table_id is required"));
  }
  return ::grpc::Status::OK;
}

backend::storage::TableId TableIdFromProto(const v1::TableRef& ref) {
  return backend::storage::TableId{
      ref.project_id(), ref.dataset_id(), ref.table_id()};
}

backend::storage::DataMaskKind ParseGovernanceMaskKind(absl::string_view s) {
  if (s == "NULLIFY") return backend::storage::DataMaskKind::kNullify;
  if (s == "SHA256") return backend::storage::DataMaskKind::kSha256;
  if (s == "DEFAULT_VALUE") {
    return backend::storage::DataMaskKind::kDefaultValue;
  }
  if (s == "DENIED") return backend::storage::DataMaskKind::kDenied;
  return backend::storage::DataMaskKind::kNone;
}

void RowAccessPolicyToProto(const backend::storage::RowAccessPolicyRecord& in,
                            const v1::TableRef& table,
                            v1::RowAccessPolicy* out) {
  *out->mutable_table() = table;
  out->set_policy_id(in.policy_id);
  out->set_filter_predicate(in.filter_predicate);
  out->clear_grantees();
  for (const std::string& g : in.grantees) {
    out->add_grantees(g);
  }
  out->set_creation_time_ms(in.creation_time_ms);
  out->set_last_modified_time_ms(in.last_modified_time_ms);
}

}  // namespace

::grpc::Status CatalogService::UpsertRowAccessPolicy(
    ::grpc::ServerContext* /*context*/,
    const v1::UpsertRowAccessPolicyRequest* request,
    v1::UpsertRowAccessPolicyResponse* response) {
  if (storage_ == nullptr) {
    return ::grpc::Status(::grpc::StatusCode::FAILED_PRECONDITION,
                          "CatalogService: storage backend is not configured");
  }
  if (request == nullptr || response == nullptr || !request->has_policy()) {
    return ::grpc::Status(::grpc::StatusCode::INVALID_ARGUMENT,
                          "UpsertRowAccessPolicy: policy is required");
  }
  const v1::RowAccessPolicy& policy = request->policy();
  if (!policy.has_table()) {
    return ::grpc::Status(::grpc::StatusCode::INVALID_ARGUMENT,
                          "UpsertRowAccessPolicy: policy.table is required");
  }
  if (auto v = ValidateTableRef(policy.table(), "UpsertRowAccessPolicy");
      !v.ok()) {
    return v;
  }
  if (policy.policy_id().empty() || policy.filter_predicate().empty()) {
    return ::grpc::Status(
        ::grpc::StatusCode::INVALID_ARGUMENT,
        "UpsertRowAccessPolicy: policy_id and filter_predicate are required");
  }
  backend::storage::RowAccessPolicyRecord rec;
  rec.policy_id = policy.policy_id();
  rec.filter_predicate = policy.filter_predicate();
  rec.grantees.assign(policy.grantees().begin(), policy.grantees().end());
  rec.creation_time_ms = policy.creation_time_ms();
  rec.last_modified_time_ms = policy.last_modified_time_ms();
  const backend::storage::TableId id = TableIdFromProto(policy.table());
  absl::Status upserted = storage_->UpsertRowAccessPolicy(id, rec);
  if (!upserted.ok()) return internal::AbslToGrpcStatus(upserted);
  absl::StatusOr<backend::storage::TableGovernance> gov_or =
      storage_->GetTableGovernance(id);
  if (!gov_or.ok()) return internal::AbslToGrpcStatus(gov_or.status());
  for (const backend::storage::RowAccessPolicyRecord& stored :
       gov_or->row_access_policies) {
    if (stored.policy_id == rec.policy_id) {
      RowAccessPolicyToProto(
          stored, policy.table(), response->mutable_policy());
      break;
    }
  }
  return ::grpc::Status::OK;
}

::grpc::Status CatalogService::DeleteRowAccessPolicy(
    ::grpc::ServerContext* /*context*/,
    const v1::DeleteRowAccessPolicyRequest* request,
    v1::DeleteRowAccessPolicyResponse* /*response*/) {
  if (storage_ == nullptr) {
    return ::grpc::Status(::grpc::StatusCode::FAILED_PRECONDITION,
                          "CatalogService: storage backend is not configured");
  }
  if (request == nullptr || !request->has_table()) {
    return ::grpc::Status(::grpc::StatusCode::INVALID_ARGUMENT,
                          "DeleteRowAccessPolicy: table is required");
  }
  if (auto v = ValidateTableRef(request->table(), "DeleteRowAccessPolicy");
      !v.ok()) {
    return v;
  }
  if (request->policy_id().empty()) {
    return ::grpc::Status(::grpc::StatusCode::INVALID_ARGUMENT,
                          "DeleteRowAccessPolicy: policy_id is required");
  }
  return internal::AbslToGrpcStatus(storage_->DeleteRowAccessPolicy(
      TableIdFromProto(request->table()), request->policy_id()));
}

::grpc::Status CatalogService::ListRowAccessPolicies(
    ::grpc::ServerContext* /*context*/,
    const v1::ListRowAccessPoliciesRequest* request,
    v1::ListRowAccessPoliciesResponse* response) {
  if (storage_ == nullptr) {
    return ::grpc::Status(::grpc::StatusCode::FAILED_PRECONDITION,
                          "CatalogService: storage backend is not configured");
  }
  if (request == nullptr || response == nullptr || !request->has_table()) {
    return ::grpc::Status(::grpc::StatusCode::INVALID_ARGUMENT,
                          "ListRowAccessPolicies: table is required");
  }
  if (auto v = ValidateTableRef(request->table(), "ListRowAccessPolicies");
      !v.ok()) {
    return v;
  }
  const backend::storage::TableId id = TableIdFromProto(request->table());
  absl::StatusOr<backend::storage::TableGovernance> gov_or =
      storage_->GetTableGovernance(id);
  if (!gov_or.ok()) return internal::AbslToGrpcStatus(gov_or.status());
  for (const backend::storage::RowAccessPolicyRecord& policy :
       gov_or->row_access_policies) {
    RowAccessPolicyToProto(policy, request->table(), response->add_policies());
  }
  return ::grpc::Status::OK;
}

::grpc::Status CatalogService::SetColumnGovernance(
    ::grpc::ServerContext* /*context*/,
    const v1::SetColumnGovernanceRequest* request,
    v1::SetColumnGovernanceResponse* /*response*/) {
  if (storage_ == nullptr) {
    return ::grpc::Status(::grpc::StatusCode::FAILED_PRECONDITION,
                          "CatalogService: storage backend is not configured");
  }
  if (request == nullptr || !request->has_table() || !request->has_column()) {
    return ::grpc::Status(::grpc::StatusCode::INVALID_ARGUMENT,
                          "SetColumnGovernance: table and column are required");
  }
  if (auto v = ValidateTableRef(request->table(), "SetColumnGovernance");
      !v.ok()) {
    return v;
  }
  const v1::ColumnGovernance& col = request->column();
  if (col.column_name().empty()) {
    return ::grpc::Status(::grpc::StatusCode::INVALID_ARGUMENT,
                          "SetColumnGovernance: column_name is required");
  }
  backend::storage::ColumnGovernanceRecord record;
  record.policy_tags.assign(col.policy_tags().begin(), col.policy_tags().end());
  record.mask_kind = ParseGovernanceMaskKind(col.mask_kind());
  record.mask_grantees.assign(col.mask_grantees().begin(),
                              col.mask_grantees().end());
  record.default_mask_value = col.default_mask_value();
  return internal::AbslToGrpcStatus(storage_->SetColumnGovernance(
      TableIdFromProto(request->table()), col.column_name(), record));
}

}  // namespace frontend
}  // namespace bigquery_emulator
