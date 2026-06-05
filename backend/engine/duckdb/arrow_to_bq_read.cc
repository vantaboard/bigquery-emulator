#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <limits>
#include <string>
#include <utility>
#include <vector>

#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "absl/strings/ascii.h"
#include "absl/strings/escaping.h"
#include "absl/strings/match.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/str_format.h"
#include "backend/engine/duckdb/arrow_to_bq_internal.h"
#include "backend/schema/schema.h"
#include "backend/storage/storage.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace duckdb {
namespace arrow_to_bq {
namespace internal {

absl::StatusOr<storage::Value> ReadInt64Cell(
    ::duckdb_vector vector,
    ::idx_t row,
    ::duckdb_type type_id,
    const schema::ColumnSchema& column) {
  int64_t v = 0;
  switch (type_id) {
    case ::DUCKDB_TYPE_TINYINT: {
      const auto* data =
          static_cast<const int8_t*>(::duckdb_vector_get_data(vector));
      v = static_cast<int64_t>(data[row]);
      break;
    }
    case ::DUCKDB_TYPE_SMALLINT: {
      const auto* data =
          static_cast<const int16_t*>(::duckdb_vector_get_data(vector));
      v = static_cast<int64_t>(data[row]);
      break;
    }
    case ::DUCKDB_TYPE_INTEGER: {
      const auto* data =
          static_cast<const int32_t*>(::duckdb_vector_get_data(vector));
      v = static_cast<int64_t>(data[row]);
      break;
    }
    case ::DUCKDB_TYPE_BIGINT: {
      const auto* data =
          static_cast<const int64_t*>(::duckdb_vector_get_data(vector));
      v = data[row];
      break;
    }
    case ::DUCKDB_TYPE_UTINYINT: {
      const auto* data =
          static_cast<const uint8_t*>(::duckdb_vector_get_data(vector));
      v = static_cast<int64_t>(data[row]);
      break;
    }
    case ::DUCKDB_TYPE_USMALLINT: {
      const auto* data =
          static_cast<const uint16_t*>(::duckdb_vector_get_data(vector));
      v = static_cast<int64_t>(data[row]);
      break;
    }
    case ::DUCKDB_TYPE_UINTEGER: {
      const auto* data =
          static_cast<const uint32_t*>(::duckdb_vector_get_data(vector));
      v = static_cast<int64_t>(data[row]);
      break;
    }
    case ::DUCKDB_TYPE_UBIGINT: {
      const auto* data =
          static_cast<const uint64_t*>(::duckdb_vector_get_data(vector));
      v = static_cast<int64_t>(data[row]);
      break;
    }
    case ::DUCKDB_TYPE_HUGEINT: {
      const auto* data = static_cast<const ::duckdb_hugeint*>(
          ::duckdb_vector_get_data(vector));
      auto narrowed = HugeintCellToInt64(data[row], column);
      if (!narrowed.ok()) return narrowed.status();
      v = *narrowed;
      break;
    }
    case ::DUCKDB_TYPE_VARCHAR: {
      // DuckDB `list()` / `ARRAY_AGG` sometimes materialize INT64
      // elements as VARCHAR; coerce when the analyzer typed INT64.
      const std::string text(std::string(ReadVarchar(vector, row)));
      char* end = nullptr;
      const int64_t parsed = std::strtoll(text.c_str(), &end, 10);
      if (end == text.c_str() || *end != '\0') {
        return absl::InvalidArgumentError(
            absl::StrCat("arrow_to_bq: INT64 column '",
                         column.name,
                         "' VARCHAR cell is not an integer: ",
                         text));
      }
      v = parsed;
      break;
    }
    default:
      return absl::UnimplementedError(
          absl::StrCat("arrow_to_bq: INT64 column '",
                       column.name,
                       "' backed by unsupported DuckDB type_id=",
                       type_id));
  }
  return storage::Value::Int64(v);
}

// Read a Vector cell into a Float64-typed `storage::Value`. Caller
// already established the column type is BigQuery FLOAT64.
absl::StatusOr<storage::Value> ReadFloat64Cell(
    ::duckdb_vector vector,
    ::idx_t row,
    ::duckdb_logical_type logical,
    ::duckdb_type type_id,
    const schema::ColumnSchema& column) {
  double v = 0.0;
  switch (type_id) {
    case ::DUCKDB_TYPE_FLOAT: {
      const auto* data =
          static_cast<const float*>(::duckdb_vector_get_data(vector));
      v = static_cast<double>(data[row]);
      break;
    }
    case ::DUCKDB_TYPE_DOUBLE: {
      const auto* data =
          static_cast<const double*>(::duckdb_vector_get_data(vector));
      v = data[row];
      break;
    }
    case ::DUCKDB_TYPE_DECIMAL: {
      auto as_double = ReadDecimalCellAsDouble(vector, row, logical, column);
      if (!as_double.ok()) return as_double.status();
      v = *as_double;
      break;
    }
    default:
      return absl::UnimplementedError(
          absl::StrCat("arrow_to_bq: FLOAT64 column '",
                       column.name,
                       "' backed by unsupported DuckDB type_id=",
                       type_id));
  }
  return storage::Value::Float64(v);
}

// Read a Vector cell into a string-rendered NUMERIC / BIGNUMERIC
// `storage::Value`. Caller already established the column type. We
// route via a textual cast because DuckDB only exposes the raw
// integer payload + the static scale; HUGEINT-backed decimals need
// 128-bit arithmetic we don't yet pull in and fall back via
// UNIMPLEMENTED.
absl::StatusOr<storage::Value> ReadDecimalCell(
    ::duckdb_vector vector,
    ::idx_t row,
    ::duckdb_logical_type logical,
    ::duckdb_type type_id,
    const schema::ColumnSchema& column) {
  if (type_id != ::DUCKDB_TYPE_DECIMAL) {
    return absl::UnimplementedError(
        absl::StrCat("arrow_to_bq: NUMERIC column '",
                     column.name,
                     "' backed by unsupported DuckDB type_id=",
                     type_id));
  }
  const auto scale = ::duckdb_decimal_scale(logical);
  const ::duckdb_type internal = ::duckdb_decimal_internal_type(logical);
  int64_t raw = 0;
  switch (internal) {
    case ::DUCKDB_TYPE_SMALLINT: {
      const auto* data =
          static_cast<const int16_t*>(::duckdb_vector_get_data(vector));
      raw = static_cast<int64_t>(data[row]);
      break;
    }
    case ::DUCKDB_TYPE_INTEGER: {
      const auto* data =
          static_cast<const int32_t*>(::duckdb_vector_get_data(vector));
      raw = static_cast<int64_t>(data[row]);
      break;
    }
    case ::DUCKDB_TYPE_BIGINT: {
      const auto* data =
          static_cast<const int64_t*>(::duckdb_vector_get_data(vector));
      raw = data[row];
      break;
    }
    default:
      // HUGEINT-backed decimals need 128-bit arithmetic that we
      // don't yet pull in. Fall back to the engine-agnostic
      // string form via the textual cast path the engine takes
      // when a column type is not yet specialized here.
      return absl::UnimplementedError(
          absl::StrCat("arrow_to_bq: DECIMAL column '",
                       column.name,
                       "' with HUGEINT internal storage is not yet supported"));
  }
  return storage::Value::String(FormatDecimalInt64(raw, scale));
}

// Read an ARRAY-typed cell from `vector` at `row`. Caller has
// pre-validated that the column should be REPEATED / ARRAY and that
// the vector's `type_id` is `DUCKDB_TYPE_LIST`.
absl::StatusOr<storage::Value> ReadArrayCell(
    ::duckdb_vector vector, ::idx_t row, const schema::ColumnSchema& column) {
  const auto* entries =
      static_cast<const ::duckdb_list_entry*>(::duckdb_vector_get_data(vector));
  ::duckdb_vector child = ::duckdb_list_vector_get_child(vector);
  schema::ColumnSchema element = column;
  element.mode = schema::ColumnMode::kNullable;
  // ColumnType::kArray means a top-level ARRAY<X> column; the element
  // type lives on column.fields[0] for that shape.
  if (column.type == schema::ColumnType::kArray && !column.fields.empty()) {
    element = column.fields[0];
  }
  std::vector<storage::Value> elements;
  elements.reserve(entries[row].length);
  for (uint64_t i = 0; i < entries[row].length; ++i) {
    auto v =
        DispatchReadCellFromVector(child, entries[row].offset + i, element);
    if (!v.ok()) return v.status();
    elements.push_back(std::move(v).value());
  }
  return storage::Value::Array(std::move(elements));
}

// Read a STRUCT-typed cell from `vector` at `row`. Caller has
// pre-validated that the column should be STRUCT and that the
// vector's `type_id` is `DUCKDB_TYPE_STRUCT`.
absl::StatusOr<storage::Value> ReadStructCell(
    ::duckdb_vector vector, ::idx_t row, const schema::ColumnSchema& column) {
  std::vector<storage::Value> fields;
  fields.reserve(column.fields.size());
  for (size_t i = 0; i < column.fields.size(); ++i) {
    ::duckdb_vector child =
        ::duckdb_struct_vector_get_child(vector, static_cast<::idx_t>(i));
    auto v = DispatchReadCellFromVector(child, row, column.fields[i]);
    if (!v.ok()) return v.status();
    fields.push_back(std::move(v).value());
  }
  return storage::Value::Struct(std::move(fields));
}

// Dispatch the scalar branches of `ReadCellFromVector`. Container
// columns are handled before we reach here; the kArray / kStruct
// arms below only fire when the analyzer disagreed with DuckDB's
// reported chunk type.
absl::StatusOr<storage::Value> ReadScalarCell(
    ::duckdb_vector vector,
    ::idx_t row,
    ::duckdb_logical_type logical,
    ::duckdb_type type_id,
    const schema::ColumnSchema& column) {
  switch (column.type) {
    case schema::ColumnType::kBool: {
      const auto* data =
          static_cast<const bool*>(::duckdb_vector_get_data(vector));
      return storage::Value::Bool(data[row]);
    }
    case schema::ColumnType::kInt64:
      return ReadInt64Cell(vector, row, type_id, column);
    case schema::ColumnType::kFloat64:
      return ReadFloat64Cell(vector, row, logical, type_id, column);
    case schema::ColumnType::kString:
    case schema::ColumnType::kJson:
    case schema::ColumnType::kGeography:
      return storage::Value::String(std::string(ReadVarchar(vector, row)));
    case schema::ColumnType::kBytes: {
      absl::string_view raw = ReadVarchar(vector, row);
      if (type_id == ::DUCKDB_TYPE_BLOB) {
        return storage::Value::Bytes(std::string(raw));
      }
      absl::StatusOr<std::string> decoded = DecodeDuckDbBlobText(raw);
      if (!decoded.ok()) return decoded.status();
      return storage::Value::Bytes(*std::move(decoded));
    }
    case schema::ColumnType::kDate: {
      const auto* data =
          static_cast<const int32_t*>(::duckdb_vector_get_data(vector));
      return storage::Value::String(FormatDate(data[row]));
    }
    case schema::ColumnType::kTime: {
      const auto* data =
          static_cast<const int64_t*>(::duckdb_vector_get_data(vector));
      return storage::Value::String(FormatTimeMicros(data[row]));
    }
    case schema::ColumnType::kDatetime:
    case schema::ColumnType::kTimestamp: {
      const auto* data =
          static_cast<const int64_t*>(::duckdb_vector_get_data(vector));
      return storage::Value::String(FormatTimestampMicros(data[row]));
    }
    case schema::ColumnType::kNumeric:
    case schema::ColumnType::kBignumeric:
      return ReadDecimalCell(vector, row, logical, type_id, column);
    case schema::ColumnType::kArray:
    case schema::ColumnType::kStruct:
      // Already handled above; reaching this branch means the vector
      // type did not advertise LIST/STRUCT and the schema does claim
      // a container type. Surface as a FailedPrecondition because the
      // analyzer disagreed with DuckDB's reported chunk type.
      return absl::FailedPreconditionError(
          absl::StrCat("arrow_to_bq: container column '",
                       column.name,
                       "' has no LIST/STRUCT vector backing"));
    case schema::ColumnType::kUnknown:
      // Fallback path: render the cell as the textual form of the
      // varchar vector if DuckDB shipped one, otherwise empty.
      if (type_id == ::DUCKDB_TYPE_VARCHAR) {
        return storage::Value::String(std::string(ReadVarchar(vector, row)));
      }
      return absl::UnimplementedError(absl::StrCat(
          "arrow_to_bq: column '",
          column.name,
          "' has unknown BigQuery type and non-VARCHAR DuckDB type_id=",
          type_id));
  }
  return absl::InternalError("arrow_to_bq: ReadScalarCell unreachable");
}

absl::StatusOr<storage::Value> DispatchReadCellFromVector(
    ::duckdb_vector vector, ::idx_t row, const schema::ColumnSchema& column) {
  if (vector == nullptr) {
    return absl::InvalidArgumentError("arrow_to_bq: vector is null");
  }
  const auto* const validity = ::duckdb_vector_get_validity(vector);
  if (!RowIsValid(validity, row)) {
    return storage::Value::Null();
  }

  ::duckdb_logical_type logical = ::duckdb_vector_get_column_type(vector);
  if (logical == nullptr) {
    return absl::InternalError("arrow_to_bq: vector has no logical type");
  }
  const ::duckdb_type type_id = ::duckdb_get_type_id(logical);

  if (column.mode == schema::ColumnMode::kRepeated ||
      column.type == schema::ColumnType::kArray ||
      type_id == ::DUCKDB_TYPE_LIST) {
    if (type_id != ::DUCKDB_TYPE_LIST) {
      ::duckdb_destroy_logical_type(&logical);
      return absl::FailedPreconditionError(
          absl::StrCat("arrow_to_bq: REPEATED column '",
                       column.name,
                       "' is backed by non-LIST DuckDB vector (type_id=",
                       type_id,
                       ")"));
    }
    auto result = ReadArrayCell(vector, row, column);
    ::duckdb_destroy_logical_type(&logical);
    return result;
  }

  if (column.type == schema::ColumnType::kStruct ||
      type_id == ::DUCKDB_TYPE_STRUCT) {
    if (type_id != ::DUCKDB_TYPE_STRUCT) {
      ::duckdb_destroy_logical_type(&logical);
      return absl::FailedPreconditionError(
          absl::StrCat("arrow_to_bq: STRUCT column '",
                       column.name,
                       "' is backed by non-STRUCT DuckDB vector (type_id=",
                       type_id,
                       ")"));
    }
    auto result = ReadStructCell(vector, row, column);
    ::duckdb_destroy_logical_type(&logical);
    return result;
  }

  auto result = ReadScalarCell(vector, row, logical, type_id, column);
  ::duckdb_destroy_logical_type(&logical);
  return result;
}

}  // namespace internal
}  // namespace arrow_to_bq
}  // namespace duckdb
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
