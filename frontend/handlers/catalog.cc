#include "frontend/handlers/catalog.h"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "absl/container/flat_hash_set.h"
#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "absl/strings/match.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/string_view.h"
#include "absl/types/span.h"
#include "backend/catalog/measure_catalog.h"
#include "backend/catalog/view_registry.h"
#include "backend/schema/googlesql_to_bq.h"
#include "backend/schema/schema.h"
#include "backend/storage/storage.h"
#include "googlesql/public/catalog.h"

namespace bigquery_emulator {
namespace frontend {

namespace {

// Maps storage `absl::Status` codes to gRPC for the gateway wire.
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

// Validates that a `TableRef` carries the three required identifiers.
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

backend::storage::DatasetId DatasetIdFromProto(const v1::DatasetRef& ref) {
  return backend::storage::DatasetId{ref.project_id(), ref.dataset_id()};
}

backend::storage::TableId TableIdFromProto(const v1::TableRef& ref) {
  return backend::storage::TableId{
      ref.project_id(), ref.dataset_id(), ref.table_id()};
}

// Converts an `enginepb::Cell` to a `backend::storage::Value`.
//
// Tabledata.insertAll's REST payload sends every primitive as a JSON
// string (BigQuery's "f"/"v" wire shape). The gateway forwards them
// as `Cell::string_value` and `Cell::null_value`; the engine keeps
// them stored as either `Value::String` or `Value::Null` so a later
// `ListRows` returns the exact same string-shaped bytes back. Typed
// coercion happens at the engine boundary inside the query-execution
// path; the catalog only round-trips opaque wire-shape data.
backend::storage::Value CellToValue(const v1::Cell& cell) {
  switch (cell.value_case()) {
    case v1::Cell::kStringValue:
      return backend::storage::Value::String(cell.string_value());
    case v1::Cell::kNullValue:
      return backend::storage::Value::Null();
    case v1::Cell::kArray: {
      std::vector<backend::storage::Value> elements;
      elements.reserve(cell.array().elements_size());
      for (const auto& el : cell.array().elements()) {
        elements.push_back(CellToValue(el));
      }
      return backend::storage::Value::Array(std::move(elements));
    }
    case v1::Cell::kStructValue: {
      std::vector<backend::storage::Value> fields;
      fields.reserve(cell.struct_value().fields_size());
      for (const auto& f : cell.struct_value().fields()) {
        fields.push_back(CellToValue(f));
      }
      return backend::storage::Value::Struct(std::move(fields));
    }
    case v1::Cell::VALUE_NOT_SET:
      break;
  }
  return backend::storage::Value::Null();
}

// Inverse of CellToValue. Mirrors the `Value::Kind` variant onto the
// `Cell` oneof; primitives that do not have a dedicated proto slot
// (bool/int/float) round-trip as their decimal-string formatting,
// matching the BigQuery REST `f`/`v` wire shape.
void ValueToCell(const backend::storage::Value& value, v1::Cell* out) {
  using Kind = backend::storage::Value::Kind;
  out->Clear();
  switch (value.kind()) {
    case Kind::kNull:
      out->set_null_value(true);
      return;
    case Kind::kBool:
      out->set_string_value(value.bool_value() ? "true" : "false");
      return;
    case Kind::kInt64:
      out->set_string_value(absl::StrCat(value.int64_value()));
      return;
    case Kind::kFloat64:
      out->set_string_value(absl::StrCat(value.float64_value()));
      return;
    case Kind::kString:
    case Kind::kBytes:
      out->set_string_value(value.string_value());
      return;
    case Kind::kArray: {
      auto* arr = out->mutable_array();
      for (const auto& el : value.array_value()) {
        ValueToCell(el, arr->add_elements());
      }
      return;
    }
    case Kind::kStruct: {
      auto* st = out->mutable_struct_value();
      for (const auto& f : value.struct_value()) {
        ValueToCell(f, st->add_fields());
      }
      return;
    }
  }
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
  if (auto v = ValidateDatasetRef(request->dataset(), "DropDataset"); !v.ok()) {
    return v;
  }
  const auto id = DatasetIdFromProto(request->dataset());
  return AbslToGrpcStatus(storage_->DropDataset(
      id, request->delete_contents(), request->rest_metadata_json()));
}

::grpc::Status CatalogService::ListDatasets(
    ::grpc::ServerContext* /*context*/,
    const v1::ListDatasetsRequest* request,
    v1::ListDatasetsResponse* response) {
  if (storage_ == nullptr) {
    return ::grpc::Status(::grpc::StatusCode::INTERNAL,
                          "CatalogService: storage backend is not configured");
  }
  if (request->project_id().empty()) {
    return ::grpc::Status(::grpc::StatusCode::INVALID_ARGUMENT,
                          "ListDatasets: project_id must be non-empty");
  }
  auto datasets_or = storage_->ListDatasets(request->project_id());
  if (!datasets_or.ok()) {
    return AbslToGrpcStatus(datasets_or.status());
  }
  for (const auto& id : *datasets_or) {
    auto* ref = response->add_datasets();
    ref->set_project_id(id.project_id);
    ref->set_dataset_id(id.dataset_id);
  }
  return ::grpc::Status::OK;
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

::grpc::Status CatalogService::DropTable(::grpc::ServerContext* /*context*/,
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

::grpc::Status CatalogService::ListTables(::grpc::ServerContext* /*context*/,
                                          const v1::ListTablesRequest* request,
                                          v1::ListTablesResponse* response) {
  if (storage_ == nullptr) {
    return ::grpc::Status(::grpc::StatusCode::INTERNAL,
                          "CatalogService: storage backend is not configured");
  }
  if (auto v = ValidateDatasetRef(request->dataset(), "ListTables"); !v.ok()) {
    return v;
  }
  const auto id = DatasetIdFromProto(request->dataset());
  auto tables_or = storage_->ListTables(id);
  if (!tables_or.ok()) {
    return AbslToGrpcStatus(tables_or.status());
  }
  // Physical storage tables plus any in-memory views not yet written to
  // a sidecar (CREATE VIEW always writes a sidecar; this merge covers
  // races and legacy state).
  std::vector<std::string> table_ids;
  absl::flat_hash_set<std::string> seen;
  for (const auto& tid : *tables_or) {
    if (seen.insert(tid.table_id).second) {
      table_ids.push_back(tid.table_id);
    }
  }
  for (const auto& view :
       backend::catalog::ListProjectViews(id.project_id, id.dataset_id)) {
    if (seen.insert(view.view_name).second) {
      table_ids.push_back(view.view_name);
    }
  }
  // Keep the documented contract: deterministic, ascending table_id order.
  std::sort(table_ids.begin(), table_ids.end());
  for (const auto& table_id : table_ids) {
    auto* ref = response->add_tables();
    ref->set_project_id(id.project_id);
    ref->set_dataset_id(id.dataset_id);
    ref->set_table_id(table_id);
    const backend::storage::TableId storage_id{
        id.project_id, id.dataset_id, table_id};
    if (absl::StatusOr<backend::storage::TableResourceInfo> info_or =
            storage_->GetTableResourceInfo(storage_id);
        info_or.ok() && !info_or->table_type.empty()) {
      ref->set_table_type(info_or->table_type);
    } else if (backend::catalog::FindProjectView(
                   id.project_id, id.dataset_id, table_id) != nullptr) {
      ref->set_table_type("VIEW");
    }
  }
  return ::grpc::Status::OK;
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
  if (schema_or.ok()) {
    backend::schema::TableSchemaToProto(*schema_or, response->mutable_schema());
    if (absl::StatusOr<backend::storage::TableResourceInfo> info_or =
            storage_->GetTableResourceInfo(id);
        info_or.ok() && absl::EqualsIgnoreCase(info_or->table_type, "VIEW")) {
      response->set_table_type("VIEW");
      if (!info_or->view_query.empty()) {
        response->set_view_query(info_or->view_query);
      }
      response->set_view_use_legacy_sql(false);
    }
    return ::grpc::Status::OK;
  }
  if (!absl::IsNotFound(schema_or.status())) {
    return AbslToGrpcStatus(schema_or.status());
  }
  const ::googlesql::Table* view = backend::catalog::FindProjectView(
      id.project_id, id.dataset_id, id.table_id);
  if (view == nullptr) {
    return AbslToGrpcStatus(schema_or.status());
  }
  backend::catalog::RegisteredViewInfo info;
  if (!backend::catalog::FindRegisteredViewInfo(
          id.project_id, id.dataset_id, id.table_id, &info)) {
    return AbslToGrpcStatus(schema_or.status());
  }
  v1::TableSchema* schema = response->mutable_schema();
  for (int i = 0; i < view->NumColumns(); ++i) {
    const ::googlesql::Column* col = view->GetColumn(i);
    if (col == nullptr || col->GetType() == nullptr) {
      continue;
    }
    absl::Status field_status = backend::schema::TypeToFieldSchema(
        col->GetType(), col->Name(), schema->add_fields());
    if (!field_status.ok()) {
      return AbslToGrpcStatus(field_status);
    }
  }
  response->set_table_type("VIEW");
  response->set_view_query(info.view_definition);
  response->set_view_use_legacy_sql(false);
  return ::grpc::Status::OK;
}

backend::storage::Row PhysicalRowFromLogicalSchema(
    const backend::schema::TableSchema& logical, backend::storage::Row row) {
  if (row.cells.size() != logical.columns.size()) {
    return row;
  }
  backend::storage::Row physical;
  physical.cells.reserve(logical.columns.size());
  for (size_t i = 0; i < logical.columns.size(); ++i) {
    if (backend::catalog::ParseMeasureColumnSpec(logical.columns[i])
            .has_value()) {
      continue;
    }
    physical.cells.push_back(std::move(row.cells[i]));
  }
  return physical;
}

::grpc::Status CatalogService::InsertRows(
    ::grpc::ServerContext* /*context*/,
    const v1::InsertRowsRequest* request,
    v1::InsertRowsResponse* /*response*/) {
  if (storage_ == nullptr) {
    return ::grpc::Status(::grpc::StatusCode::INTERNAL,
                          "CatalogService: storage backend is not configured");
  }
  if (auto v = ValidateTableRef(request->table(), "InsertRows"); !v.ok()) {
    return v;
  }
  const auto id = TableIdFromProto(request->table());
  absl::StatusOr<backend::schema::TableSchema> schema_or =
      storage_->GetSchema(id);
  if (!schema_or.ok()) {
    return AbslToGrpcStatus(schema_or.status());
  }
  std::vector<backend::storage::Row> rows;
  rows.reserve(request->rows_size());
  for (const auto& proto_row : request->rows()) {
    backend::storage::Row row;
    row.cells.reserve(proto_row.cells_size());
    for (const auto& cell : proto_row.cells()) {
      row.cells.push_back(CellToValue(cell));
    }
    rows.push_back(PhysicalRowFromLogicalSchema(*schema_or, std::move(row)));
  }
  return AbslToGrpcStatus(storage_->AppendRows(id, absl::MakeSpan(rows)));
}

::grpc::Status CatalogService::ListRows(::grpc::ServerContext* /*context*/,
                                        const v1::ListRowsRequest* request,
                                        v1::ListRowsResponse* response) {
  if (storage_ == nullptr) {
    return ::grpc::Status(::grpc::StatusCode::INTERNAL,
                          "CatalogService: storage backend is not configured");
  }
  if (auto v = ValidateTableRef(request->table(), "ListRows"); !v.ok()) {
    return v;
  }
  const auto id = TableIdFromProto(request->table());
  auto iter_or = storage_->ScanRows(id);
  if (!iter_or.ok()) {
    return AbslToGrpcStatus(iter_or.status());
  }
  std::unique_ptr<backend::storage::RowIterator> iter = std::move(*iter_or);

  const int64_t start_index =
      request->start_index() > 0 ? request->start_index() : 0;
  const int64_t max_results = request->max_results();
  const bool unlimited = max_results <= 0;

  int64_t row_index = 0;
  int64_t emitted = 0;
  backend::storage::Row row;
  while (true) {
    auto next_or = iter->Next(&row);
    if (!next_or.ok()) {
      return AbslToGrpcStatus(next_or.status());
    }
    if (!*next_or) break;
    if (row_index >= start_index && (unlimited || emitted < max_results)) {
      auto* proto_row = response->add_rows();
      for (const auto& cell : row.cells) {
        ValueToCell(cell, proto_row->add_cells());
      }
      ++emitted;
    }
    ++row_index;
  }
  response->set_total_rows(row_index);
  response->set_next_start_index(start_index + emitted);
  return ::grpc::Status::OK;
}

}  // namespace frontend
}  // namespace bigquery_emulator
