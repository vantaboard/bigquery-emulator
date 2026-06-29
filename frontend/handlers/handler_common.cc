#include "frontend/handlers/handler_common.h"

#include <grpcpp/support/status.h>

#include <string>
#include <utility>
#include <vector>

#include "absl/status/status.h"
#include "absl/strings/str_cat.h"
#include "backend/storage/storage.h"
#include "proto/emulator.pb.h"

namespace bigquery_emulator {
namespace frontend {
namespace internal {

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

}  // namespace internal
}  // namespace frontend
}  // namespace bigquery_emulator
