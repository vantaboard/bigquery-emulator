#include "backend/engine/duckdb/arrow_to_bq.h"

#include <cmath>
#include <cstdint>
#include <cstring>
#include <string>
#include <utility>
#include <vector>

#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/str_format.h"
#include "backend/schema/schema.h"
#include "backend/storage/storage.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace duckdb {
namespace arrow_to_bq {

namespace {

// duckdb_string_t is the inline-or-pointer string layout DuckDB uses
// for VARCHAR / BLOB / BIT / BIGNUM vectors. Strings <= 12 bytes live
// in `value.inlined.inlined` next to a 4-byte length; longer strings
// live behind `value.pointer.ptr`. We avoid `duckdb_string_is_inlined`
// because that helper takes the struct by value which costs a copy
// per cell; reading the length directly is what every duckdb-built-in
// scan operator does too.
inline absl::string_view StringView(const ::duckdb_string_t& s) {
  const uint32_t len = s.value.inlined.length;
  if (len <= 12) {
    return absl::string_view(s.value.inlined.inlined, len);
  }
  return absl::string_view(s.value.pointer.ptr, s.value.pointer.length);
}

// DuckDB's validity mask is a packed uint64 per 64 rows. When the
// underlying vector has no NULLs at all DuckDB returns a nullptr
// mask, which we treat as "always valid".
inline bool RowIsValid(const uint64_t* mask, ::idx_t row) {
  if (mask == nullptr) return true;
  return (mask[row / 64] & (uint64_t{1} << (row % 64))) != 0;
}

// Render a DuckDB DATE column (days since 1970-01-01) as an ISO-8601
// "YYYY-MM-DD" string. We compute the calendar date ourselves rather
// than calling `duckdb_from_date` because that helper sits inside the
// `Safe Fetch` API surface and would require synthesising a
// `duckdb_date` from the raw int32_t we already have.
std::string FormatDate(int32_t days_since_epoch) {
  // Howard Hinnant's days_from_civil() inverse, exact for the
  // proleptic Gregorian calendar across the full int32 range.
  int32_t z = days_since_epoch + 719468;
  int32_t era = (z >= 0 ? z : z - 146096) / 146097;
  unsigned doe = static_cast<unsigned>(z - era * 146097);
  unsigned yoe = (doe - doe / 1460 + doe / 36524 - doe / 146096) / 365;
  int y = static_cast<int>(yoe) + era * 400;
  unsigned doy = doe - (365 * yoe + yoe / 4 - yoe / 100);
  unsigned mp = (5 * doy + 2) / 153;
  unsigned d = doy - (153 * mp + 2) / 5 + 1;
  unsigned m = mp < 10 ? mp + 3 : mp - 9;
  if (m <= 2) ++y;
  return absl::StrFormat("%04d-%02u-%02u", y, m, d);
}

// DuckDB's TIME / TIMESTAMP family of types are microsecond ints.
// We render them in the BigQuery wire-friendly form
// ("HH:MM:SS.ffffff" / "YYYY-MM-DD HH:MM:SS.ffffff") so the gateway
// can pass them through unchanged.
std::string FormatTimeMicros(int64_t micros) {
  if (micros < 0) micros = 0;
  int64_t total_seconds = micros / 1'000'000;
  int64_t fractional = micros % 1'000'000;
  int hh = static_cast<int>((total_seconds / 3600) % 24);
  int mm = static_cast<int>((total_seconds / 60) % 60);
  int ss = static_cast<int>(total_seconds % 60);
  return absl::StrFormat("%02d:%02d:%02d.%06d", hh, mm, ss, fractional);
}

std::string FormatTimestampMicros(int64_t micros) {
  // Split into whole-day component and the time-of-day fragment so
  // FormatDate / FormatTimeMicros stay independent.
  int64_t day = micros / (int64_t{86400} * 1'000'000);
  int64_t rem = micros - day * (int64_t{86400} * 1'000'000);
  if (rem < 0) {
    rem += int64_t{86400} * 1'000'000;
    --day;
  }
  std::string date = FormatDate(static_cast<int32_t>(day));
  std::string time = FormatTimeMicros(rem);
  return absl::StrCat(date, " ", time);
}

// Renders DECIMAL stored as int16 / int32 / int64 / hugeint using
// the column's `width` / `scale`. We compute the textual form
// manually so the rendering pipeline does not need to round-trip
// through DuckDB's internal cast machinery.
std::string FormatDecimalInt64(int64_t v, uint8_t scale) {
  bool negative = v < 0;
  uint64_t magnitude =
      negative ? static_cast<uint64_t>(-(v + 1)) + 1 : static_cast<uint64_t>(v);
  std::string digits = absl::StrCat(magnitude);
  if (scale == 0) {
    return negative ? absl::StrCat("-", digits) : digits;
  }
  if (digits.size() <= scale) {
    digits.insert(0, scale + 1 - digits.size(), '0');
  }
  std::string integer = digits.substr(0, digits.size() - scale);
  std::string fractional = digits.substr(digits.size() - scale);
  std::string out = absl::StrCat(integer, ".", fractional);
  return negative ? absl::StrCat("-", out) : out;
}

// Read a single varchar / blob cell out of the vector at `row`. Used
// both for VARCHAR (UTF-8 string) and BLOB (raw bytes) - the caller
// picks the Value variant.
absl::string_view ReadVarchar(::duckdb_vector vector, ::idx_t row) {
  const auto* strings =
      static_cast<const ::duckdb_string_t*>(::duckdb_vector_get_data(vector));
  return StringView(strings[row]);
}

}  // namespace

absl::StatusOr<storage::Value> ReadCellFromVector(
    ::duckdb_vector vector, ::idx_t row, const schema::ColumnSchema& column) {
  if (vector == nullptr) {
    return absl::InvalidArgumentError("arrow_to_bq: vector is null");
  }
  const uint64_t* validity = ::duckdb_vector_get_validity(vector);
  if (!RowIsValid(validity, row)) {
    return storage::Value::Null();
  }

  ::duckdb_logical_type logical = ::duckdb_vector_get_column_type(vector);
  if (logical == nullptr) {
    return absl::InternalError("arrow_to_bq: vector has no logical type");
  }
  const ::duckdb_type type_id = ::duckdb_get_type_id(logical);

  // REPEATED columns become DuckDB LIST vectors regardless of the
  // inner type, so we recurse on the child vector before consulting
  // the per-cell type id.
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
    const auto* entries = static_cast<const ::duckdb_list_entry*>(
        ::duckdb_vector_get_data(vector));
    ::duckdb_vector child = ::duckdb_list_vector_get_child(vector);
    schema::ColumnSchema element = column;
    element.mode = schema::ColumnMode::kNullable;
    // ColumnType::kArray means a top-level ARRAY<X> column; the
    // element type lives on column.fields[0] for that shape.
    if (column.type == schema::ColumnType::kArray && !column.fields.empty()) {
      element = column.fields[0];
    }
    std::vector<storage::Value> elements;
    elements.reserve(entries[row].length);
    for (uint64_t i = 0; i < entries[row].length; ++i) {
      auto v = ReadCellFromVector(child, entries[row].offset + i, element);
      if (!v.ok()) {
        ::duckdb_destroy_logical_type(&logical);
        return v.status();
      }
      elements.push_back(std::move(v).value());
    }
    ::duckdb_destroy_logical_type(&logical);
    return storage::Value::Array(std::move(elements));
  }

  // STRUCT columns: recurse into each child vector at the same row.
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
    std::vector<storage::Value> fields;
    fields.reserve(column.fields.size());
    for (size_t i = 0; i < column.fields.size(); ++i) {
      ::duckdb_vector child =
          ::duckdb_struct_vector_get_child(vector, static_cast<::idx_t>(i));
      auto v = ReadCellFromVector(child, row, column.fields[i]);
      if (!v.ok()) {
        ::duckdb_destroy_logical_type(&logical);
        return v.status();
      }
      fields.push_back(std::move(v).value());
    }
    ::duckdb_destroy_logical_type(&logical);
    return storage::Value::Struct(std::move(fields));
  }

  // Scalar dispatch. We key on `column.type` (the analyzer's BigQuery
  // typing) rather than the vector's duckdb_type because an INT32
  // vector for a column the analyzer typed INT64 still has to land
  // on the wire as a decimal string of an int64.
  switch (column.type) {
    case schema::ColumnType::kBool: {
      const auto* data =
          static_cast<const bool*>(::duckdb_vector_get_data(vector));
      ::duckdb_destroy_logical_type(&logical);
      return storage::Value::Bool(data[row]);
    }
    case schema::ColumnType::kInt64: {
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
        default:
          ::duckdb_destroy_logical_type(&logical);
          return absl::UnimplementedError(
              absl::StrCat("arrow_to_bq: INT64 column '",
                           column.name,
                           "' backed by unsupported DuckDB type_id=",
                           type_id));
      }
      ::duckdb_destroy_logical_type(&logical);
      return storage::Value::Int64(v);
    }
    case schema::ColumnType::kFloat64: {
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
        default:
          ::duckdb_destroy_logical_type(&logical);
          return absl::UnimplementedError(
              absl::StrCat("arrow_to_bq: FLOAT64 column '",
                           column.name,
                           "' backed by unsupported DuckDB type_id=",
                           type_id));
      }
      ::duckdb_destroy_logical_type(&logical);
      return storage::Value::Float64(v);
    }
    case schema::ColumnType::kString:
    case schema::ColumnType::kJson:
    case schema::ColumnType::kGeography: {
      absl::string_view sv = ReadVarchar(vector, row);
      ::duckdb_destroy_logical_type(&logical);
      return storage::Value::String(std::string(sv));
    }
    case schema::ColumnType::kBytes: {
      // VARCHAR and BLOB share the duckdb_string_t storage layout;
      // only the cell-level kind differs.
      absl::string_view sv = ReadVarchar(vector, row);
      ::duckdb_destroy_logical_type(&logical);
      return storage::Value::Bytes(std::string(sv));
    }
    case schema::ColumnType::kDate: {
      const auto* data =
          static_cast<const int32_t*>(::duckdb_vector_get_data(vector));
      ::duckdb_destroy_logical_type(&logical);
      return storage::Value::String(FormatDate(data[row]));
    }
    case schema::ColumnType::kTime: {
      const auto* data =
          static_cast<const int64_t*>(::duckdb_vector_get_data(vector));
      ::duckdb_destroy_logical_type(&logical);
      return storage::Value::String(FormatTimeMicros(data[row]));
    }
    case schema::ColumnType::kDatetime:
    case schema::ColumnType::kTimestamp: {
      const auto* data =
          static_cast<const int64_t*>(::duckdb_vector_get_data(vector));
      ::duckdb_destroy_logical_type(&logical);
      return storage::Value::String(FormatTimestampMicros(data[row]));
    }
    case schema::ColumnType::kNumeric:
    case schema::ColumnType::kBignumeric: {
      if (type_id != ::DUCKDB_TYPE_DECIMAL) {
        ::duckdb_destroy_logical_type(&logical);
        return absl::UnimplementedError(
            absl::StrCat("arrow_to_bq: NUMERIC column '",
                         column.name,
                         "' backed by unsupported DuckDB type_id=",
                         type_id));
      }
      const uint8_t scale = ::duckdb_decimal_scale(logical);
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
          ::duckdb_destroy_logical_type(&logical);
          return absl::UnimplementedError(absl::StrCat(
              "arrow_to_bq: DECIMAL column '",
              column.name,
              "' with HUGEINT internal storage is not yet supported"));
      }
      std::string rendered = FormatDecimalInt64(raw, scale);
      ::duckdb_destroy_logical_type(&logical);
      return storage::Value::String(std::move(rendered));
    }
    case schema::ColumnType::kArray:
    case schema::ColumnType::kStruct:
      // Already handled above; reaching this branch means the vector
      // type did not advertise LIST/STRUCT and the schema does claim
      // a container type. Surface as a FailedPrecondition because the
      // analyzer disagreed with DuckDB's reported chunk type.
      ::duckdb_destroy_logical_type(&logical);
      return absl::FailedPreconditionError(
          absl::StrCat("arrow_to_bq: container column '",
                       column.name,
                       "' has no LIST/STRUCT vector backing"));
    case schema::ColumnType::kUnknown:
      // Fallback path: render the cell as the textual form of the
      // varchar vector if DuckDB shipped one, otherwise empty.
      if (type_id == ::DUCKDB_TYPE_VARCHAR) {
        absl::string_view sv = ReadVarchar(vector, row);
        ::duckdb_destroy_logical_type(&logical);
        return storage::Value::String(std::string(sv));
      }
      ::duckdb_destroy_logical_type(&logical);
      return absl::UnimplementedError(absl::StrCat(
          "arrow_to_bq: column '",
          column.name,
          "' has unknown BigQuery type and non-VARCHAR DuckDB type_id=",
          type_id));
  }
  ::duckdb_destroy_logical_type(&logical);
  return absl::InternalError("arrow_to_bq: ReadCellFromVector unreachable");
}

absl::StatusOr<storage::Row> ChunkRowToCells(
    ::duckdb_data_chunk chunk,
    ::idx_t row,
    const schema::TableSchema& output_schema) {
  if (chunk == nullptr) {
    return absl::InvalidArgumentError("arrow_to_bq: chunk is null");
  }
  const ::idx_t ncols = ::duckdb_data_chunk_get_column_count(chunk);
  if (ncols != static_cast<::idx_t>(output_schema.columns.size())) {
    return absl::InvalidArgumentError(
        absl::StrCat("arrow_to_bq: chunk has ",
                     ncols,
                     " columns but analyzer output schema has ",
                     output_schema.columns.size()));
  }
  storage::Row out;
  out.cells.reserve(ncols);
  for (::idx_t c = 0; c < ncols; ++c) {
    ::duckdb_vector vector = ::duckdb_data_chunk_get_vector(chunk, c);
    auto cell = ReadCellFromVector(vector, row, output_schema.columns[c]);
    if (!cell.ok()) return cell.status();
    out.cells.push_back(std::move(cell).value());
  }
  return out;
}

}  // namespace arrow_to_bq
}  // namespace duckdb
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
