#ifndef BIGQUERY_EMULATOR_FRONTEND_HANDLERS_STORAGE_READ_INTERNAL_H_
#define BIGQUERY_EMULATOR_FRONTEND_HANDLERS_STORAGE_READ_INTERNAL_H_

#include <cstddef>
#include <string>
#include <vector>

#include "absl/status/status.h"
#include "absl/strings/string_view.h"
#include "backend/schema/schema.h"
#include "backend/storage/storage.h"
#include "grpcpp/support/status.h"
#include "proto/emulator.pb.h"

namespace bigquery_emulator {
namespace frontend {
namespace internal {

::grpc::Status AbslToGrpcStatus(const absl::Status& status);

void ValueToCell(const backend::storage::Value& value, v1::Cell* out);

bool SchemasEqualByShape(const backend::schema::TableSchema& a,
                         const backend::schema::TableSchema& b);

constexpr std::size_t kColumnNotFound = static_cast<std::size_t>(-1);

std::size_t FindColumnByName(const backend::schema::TableSchema& schema,
                             absl::string_view name);

backend::schema::TableSchema ProjectSchemaForResponse(
    const backend::schema::TableSchema& schema,
    const std::vector<std::string>& field_names);

}  // namespace internal
}  // namespace frontend
}  // namespace bigquery_emulator

#endif  // BIGQUERY_EMULATOR_FRONTEND_HANDLERS_STORAGE_READ_INTERNAL_H_
