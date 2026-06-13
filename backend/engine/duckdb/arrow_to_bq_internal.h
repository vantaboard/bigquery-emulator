#ifndef BIGQUERY_EMULATOR_BACKEND_ENGINE_DUCKDB_ARROW_TO_BQ_INTERNAL_H_
#define BIGQUERY_EMULATOR_BACKEND_ENGINE_DUCKDB_ARROW_TO_BQ_INTERNAL_H_

#include <cstdint>
#include <string>

#include "absl/status/statusor.h"
#include "absl/strings/string_view.h"
#include "backend/schema/schema.h"
#include "backend/storage/storage.h"
#include "duckdb.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace duckdb {
namespace arrow_to_bq {
namespace internal {

inline bool RowIsValid(const uint64_t* mask, ::idx_t row) {
  if (mask == nullptr) return true;
  return (mask[row / 64] & (uint64_t{1} << (row % 64))) != 0;
}

std::string FormatDate(int32_t days_since_epoch);
std::string FormatTimeMicros(int64_t micros);
std::string FormatTimestampMicros(int64_t micros);
std::string FormatDecimalInt64(int64_t v, uint8_t scale);
std::string FormatDecimalHugeint(::duckdb_hugeint h, uint8_t scale);

absl::StatusOr<int64_t> HugeintCellToInt64(const ::duckdb_hugeint& h,
                                           const schema::ColumnSchema& column);

absl::StatusOr<double> ReadDecimalCellAsDouble(
    ::duckdb_vector vector,
    ::idx_t row,
    ::duckdb_logical_type logical,
    const schema::ColumnSchema& column);

absl::StatusOr<std::string> DecodeDuckDbBlobText(absl::string_view s);

absl::string_view ReadVarchar(::duckdb_vector vector, ::idx_t row);

absl::StatusOr<storage::Value> ReadInt64Cell(
    ::duckdb_vector vector,
    ::idx_t row,
    ::duckdb_type type_id,
    const schema::ColumnSchema& column);

absl::StatusOr<storage::Value> ReadFloat64Cell(
    ::duckdb_vector vector,
    ::idx_t row,
    ::duckdb_logical_type logical,
    ::duckdb_type type_id,
    const schema::ColumnSchema& column);

absl::StatusOr<storage::Value> ReadDecimalCell(
    ::duckdb_vector vector,
    ::idx_t row,
    ::duckdb_logical_type logical,
    ::duckdb_type type_id,
    const schema::ColumnSchema& column);

absl::StatusOr<storage::Value> ReadArrayCell(
    ::duckdb_vector vector, ::idx_t row, const schema::ColumnSchema& column);

absl::StatusOr<storage::Value> ReadStructCell(
    ::duckdb_vector vector, ::idx_t row, const schema::ColumnSchema& column);

absl::StatusOr<storage::Value> ReadScalarCell(
    ::duckdb_vector vector,
    ::idx_t row,
    ::duckdb_logical_type logical,
    ::duckdb_type type_id,
    const schema::ColumnSchema& column);

absl::StatusOr<storage::Value> DispatchReadCellFromVector(
    ::duckdb_vector vector, ::idx_t row, const schema::ColumnSchema& column);

}  // namespace internal
}  // namespace arrow_to_bq
}  // namespace duckdb
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator

#endif  // BIGQUERY_EMULATOR_BACKEND_ENGINE_DUCKDB_ARROW_TO_BQ_INTERNAL_H_
