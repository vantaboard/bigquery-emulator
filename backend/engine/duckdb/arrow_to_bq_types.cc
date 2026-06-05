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

// Pack a signed integer that fits in int64 into DuckDB's HUGEINT wire
// layout (lower + sign-extended upper).
inline ::duckdb_hugeint Int64ToDuckdbHugeint(int64_t raw) {
  ::duckdb_hugeint h;
  h.lower = static_cast<uint64_t>(raw);
  h.upper = (raw < 0) ? -1 : 0;
  return h;
}

// Narrow a HUGEINT vector cell to int64 when the value fits the int64
// range exactly. Window aggregates and SUM/COUNT paths often land here
// even when the analyzer typed the output column INT64.
absl::StatusOr<int64_t> HugeintCellToInt64(const ::duckdb_hugeint& h,
                                           const schema::ColumnSchema& column) {
  if (h.upper == 0) {
    if (h.lower > static_cast<uint64_t>(std::numeric_limits<int64_t>::max())) {
      return absl::OutOfRangeError(
          absl::StrCat("arrow_to_bq: INT64 column '",
                       column.name,
                       "' HUGEINT value exceeds INT64 range"));
    }
    return static_cast<int64_t>(h.lower);
  }
  if (h.upper == -1) {
    return static_cast<int64_t>(h.lower);
  }
  return absl::OutOfRangeError(
      absl::StrCat("arrow_to_bq: INT64 column '",
                   column.name,
                   "' HUGEINT value exceeds INT64 range"));
}

// Read a DECIMAL vector cell as FLOAT64 via DuckDB's decimal helpers.
// DuckDB promotes SUM/AVG over FLOAT columns to DECIMAL; the analyzer
// still types those outputs FLOAT64.
absl::StatusOr<double> ReadDecimalCellAsDouble(
    ::duckdb_vector vector,
    ::idx_t row,
    ::duckdb_logical_type logical,
    const schema::ColumnSchema& column) {
  const auto scale = ::duckdb_decimal_scale(logical);
  const auto width = ::duckdb_decimal_width(logical);
  const ::duckdb_type internal = ::duckdb_decimal_internal_type(logical);
  ::duckdb_decimal dec;
  dec.width = width;
  dec.scale = scale;
  switch (internal) {
    case ::DUCKDB_TYPE_SMALLINT: {
      const auto* data =
          static_cast<const int16_t*>(::duckdb_vector_get_data(vector));
      dec.value = Int64ToDuckdbHugeint(static_cast<int64_t>(data[row]));
      break;
    }
    case ::DUCKDB_TYPE_INTEGER: {
      const auto* data =
          static_cast<const int32_t*>(::duckdb_vector_get_data(vector));
      dec.value = Int64ToDuckdbHugeint(static_cast<int64_t>(data[row]));
      break;
    }
    case ::DUCKDB_TYPE_BIGINT: {
      const auto* data =
          static_cast<const int64_t*>(::duckdb_vector_get_data(vector));
      dec.value = Int64ToDuckdbHugeint(data[row]);
      break;
    }
    case ::DUCKDB_TYPE_HUGEINT: {
      const auto* data = static_cast<const ::duckdb_hugeint*>(
          ::duckdb_vector_get_data(vector));
      dec.value = data[row];
      break;
    }
    default:
      return absl::UnimplementedError(
          absl::StrCat("arrow_to_bq: FLOAT64 column '",
                       column.name,
                       "' DECIMAL internal type is not yet supported"));
  }
  return ::duckdb_decimal_to_double(dec);
}

// DuckDB sometimes renders BLOB cells as VARCHAR holding hex text
// (`X'010203'` or `x010203`). Decode to raw bytes for BYTES columns.
absl::StatusOr<std::string> DecodeDuckDbBlobText(absl::string_view s) {
  if (absl::StartsWith(s, "X'") && absl::EndsWith(s, "'")) {
    std::string hex(s.substr(2, s.size() - 3));
    std::string out;
    if (!absl::HexStringToBytes(hex, &out)) {
      return absl::InvalidArgumentError(
          absl::StrCat("arrow_to_bq: invalid BLOB hex literal X'", hex, "'"));
    }
    return out;
  }
  if (absl::StartsWith(s, "x") && s.size() > 1 && (s.size() - 1) % 2 == 0) {
    std::string hex(s.substr(1));
    bool all_hex = true;
    for (char c : hex) {
      if (!absl::ascii_isxdigit(static_cast<unsigned char>(c))) {
        all_hex = false;
        break;
      }
    }
    if (all_hex) {
      std::string out;
      if (absl::HexStringToBytes(hex, &out)) return out;
    }
  }
  return std::string(s);
}

// Read a single varchar / blob cell out of the vector at `row`. Used
// both for VARCHAR (UTF-8 string) and BLOB (raw bytes) - the caller
// picks the Value variant.
absl::string_view ReadVarchar(::duckdb_vector vector, ::idx_t row) {
  const auto* strings =
      static_cast<const ::duckdb_string_t*>(::duckdb_vector_get_data(vector));
  return StringView(strings[row]);
}

}  // namespace internal
}  // namespace arrow_to_bq
}  // namespace duckdb
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
