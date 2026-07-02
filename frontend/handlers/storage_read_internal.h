#ifndef BIGQUERY_EMULATOR_FRONTEND_HANDLERS_STORAGE_READ_INTERNAL_H_
#define BIGQUERY_EMULATOR_FRONTEND_HANDLERS_STORAGE_READ_INTERNAL_H_

#include <cstddef>
#include <string>
#include <vector>

#include "absl/status/statusor.h"
#include "absl/strings/string_view.h"
#include "backend/schema/schema.h"
#include "backend/storage/storage.h"
#include "frontend/handlers/handler_common.h"
#include "proto/emulator.pb.h"

namespace bigquery_emulator {
namespace frontend {
namespace internal {

// Bundled thirdparty samples read bigquery-public-data tables through a
// caller-scoped parent project (e.g. projects/dev). BigQuery allows that
// shape for public datasets; the emulator seeds the same project id.
inline constexpr char kPublicDataProject[] = "bigquery-public-data";

bool SchemasEqualByShape(const backend::schema::TableSchema& a,
                         const backend::schema::TableSchema& b);

constexpr std::size_t kColumnNotFound = static_cast<std::size_t>(-1);

std::size_t FindColumnByName(const backend::schema::TableSchema& schema,
                             absl::string_view name);

backend::schema::TableSchema ProjectSchemaForResponse(
    const backend::schema::TableSchema& schema,
    const std::vector<std::string>& field_names);

// Parses a TIMESTAMP storage cell (wire string or micros digits) into
// the decimal microsecond string Storage Read clients expect.
absl::StatusOr<std::string> TimestampValueToMicrosString(
    const backend::storage::Value& value);

// Marshals a storage cell onto a proto Cell, encoding TIMESTAMP
// columns as epoch micros to match BigQuery Storage Read semantics.
void ValueToCellForStorageRead(const backend::storage::Value& value,
                               backend::schema::ColumnType column_type,
                               v1::Cell* out);

}  // namespace internal
}  // namespace frontend
}  // namespace bigquery_emulator

#endif  // BIGQUERY_EMULATOR_FRONTEND_HANDLERS_STORAGE_READ_INTERNAL_H_
