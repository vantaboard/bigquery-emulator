#include <cmath>
#include <cstdlib>
#include <optional>
#include <string>

#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "absl/strings/ascii.h"
#include "absl/strings/numbers.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/str_format.h"
#include "absl/strings/str_replace.h"
#include "absl/strings/string_view.h"
#include "absl/time/time.h"
#include "absl/types/span.h"
#include "backend/schema/schema.h"
#include "backend/storage/duckdb/duckdb_storage_internal.h"
#include "backend/storage/row_restriction.h"
#include "backend/storage/storage.h"
#include "googlesql/public/functions/date_time_util.h"
#include "googlesql/public/functions/datetime.pb.h"

namespace bigquery_emulator {
namespace backend {
namespace storage {
namespace duckdb {
namespace internal {

std::string QuoteIdent(absl::string_view ident) {
  std::string escaped = absl::StrReplaceAll(ident, {{"\"", "\"\""}});
  return absl::StrCat("\"", escaped, "\"");
}

// Escapes a DuckDB SQL string literal by doubling embedded
// single-quotes. The result is *not* wrapped in quotes; the caller is
// responsible for that so the helper composes cleanly into
// `'...'`, `DATE '...'`, `TIMESTAMP '...'`, etc.
std::string EscapeStringLiteralInner(absl::string_view s) {
  return absl::StrReplaceAll(s, {{"'", "''"}});
}

// Renders raw bytes as a DuckDB BLOB literal (lower-case hex).
// DuckDB accepts both `BLOB '\xAB\xCD'` and the SQL-standard
// `X'ABCD'`; the latter is simpler to emit because every byte is
// exactly two characters of output and there is no escape sequence
// to think about.
std::string RenderBlobLiteral(absl::string_view bytes) {
  static const char* kHex = "0123456789abcdef";
  std::string out;
  out.reserve(bytes.size() * 2 + 4);
  absl::StrAppend(&out, "X'");
  for (unsigned char c : bytes) {
    out += kHex[c >> 4];
    out += kHex[c & 0x0f];
  }
  absl::StrAppend(&out, "'");
  return out;
}

// Forward declaration for the recursive cell renderer.
absl::StatusOr<std::string> RenderCellLiteral(
    const Value& cell, const schema::ColumnSchema& column);

namespace {

const ::googlesql::functions::FormatDateTimestampOptions kAppendFormatOpts{
    .expand_Q = true, .expand_J = true};

std::optional<std::string> TryFormatPackedDateString(absl::string_view s) {
  if (s.empty()) return std::nullopt;
  for (char c : s) {
    if (!absl::ascii_isdigit(static_cast<unsigned char>(c))) {
      return std::nullopt;
    }
  }
  int32_t packed = 0;
  if (!absl::SimpleAtoi(s, &packed)) return std::nullopt;
  std::string out;
  if (!::googlesql::functions::FormatDateToString(
           "%F", packed, kAppendFormatOpts, &out)
           .ok()) {
    return std::nullopt;
  }
  return out;
}

std::optional<std::string> TryFormatMicrosTimestampString(absl::string_view s) {
  if (s.empty()) return std::nullopt;
  for (char c : s) {
    if (!absl::ascii_isdigit(static_cast<unsigned char>(c))) {
      return std::nullopt;
    }
  }
  int64_t micros = 0;
  if (!absl::SimpleAtoi(s, &micros)) return std::nullopt;
  const absl::Time t = absl::FromUnixMicros(micros);
  return absl::FormatTime("%E4S", t, absl::UTCTimeZone());
}

}  // namespace

// Renders a single non-repeated scalar value as a DuckDB SQL literal,
// excluding the NULL case (the caller short-circuits that before
// calling). The column metadata is needed to pick the right SQL
// literal form for the temporal / numeric types that round-trip as
// strings in our Value union.
//
// The gateway `tabledata.insertAll` path lowers every JSON cell
// to a `Value::String` regardless of the column's declared type
// (see the comment on `frontend/handlers/catalog.cc::CellToValue`).
// For numeric / boolean columns we therefore accept both the natively-
// typed `Value::Int64` / `Value::Float64` / `Value::Bool` and a
// `Value::String` carrying the textual representation, and we delegate
// the final parse to DuckDB by emitting a CAST literal. This keeps
// the storage layer compatible with the wire-shape stringification
// the gateway performs while still surfacing malformed values as a
// CAST failure from DuckDB rather than silently storing zero.
// Render the DuckDB literal for a FLOAT64 cell. Mirrors the
// counterpart in `duckdb_executor.cc::RenderFloatLiteral`; kept
// duplicated for now (see follow-up de-dup task).
std::string RenderFloatLiteral(const Value& cell) {
  if (cell.kind() == Value::Kind::kString) {
    return absl::StrCat("CAST('",
                        EscapeStringLiteralInner(cell.string_value()),
                        "' AS DOUBLE)");
  }
  const double v = cell.float64_value();
  if (std::isnan(v)) return std::string("'NaN'::DOUBLE");
  if (std::isinf(v)) {
    return std::string(v > 0 ? "'Infinity'::DOUBLE" : "'-Infinity'::DOUBLE");
  }
  return absl::StrFormat("%.17g", v);
}

// Render the DuckDB STRUCT literal `{'k1': v1, ...}` for a STRUCT
// cell. Mirrors the counterpart in
// `duckdb_executor.cc::RenderStructLiteral` (see follow-up de-dup task).
absl::StatusOr<std::string> RenderStructLiteral(
    const Value& cell, const schema::ColumnSchema& column) {
  if (cell.kind() == Value::Kind::kString) {
    absl::string_view text = absl::StripAsciiWhitespace(cell.string_value());
    if (!text.empty() && text.front() == '{') {
      return std::string(text);
    }
  }
  if (cell.kind() != Value::Kind::kStruct) {
    return absl::InvalidArgumentError(
        absl::StrCat("AppendRows: column '",
                     column.name,
                     "' expects STRUCT but row provided non-struct cell"));
  }
  const auto& fields = cell.struct_value();
  if (fields.size() != column.fields.size()) {
    return absl::InvalidArgumentError(
        absl::StrCat("AppendRows: STRUCT column '",
                     column.name,
                     "' has ",
                     column.fields.size(),
                     " fields but row provided ",
                     fields.size()));
  }
  std::string out = "{";
  for (size_t i = 0; i < fields.size(); ++i) {
    if (i > 0) absl::StrAppend(&out, ", ");
    absl::StrAppend(
        &out, "'", EscapeStringLiteralInner(column.fields[i].name), "': ");
    auto inner_or = RenderCellLiteral(fields[i], column.fields[i]);
    if (!inner_or.ok()) return inner_or.status();
    absl::StrAppend(&out, *inner_or);
  }
  absl::StrAppend(&out, "}");
  return out;
}

absl::StatusOr<std::string> RenderScalarLiteral(
    const Value& cell, const schema::ColumnSchema& column) {
  switch (column.type) {
    case schema::ColumnType::kBool:
      if (cell.kind() == Value::Kind::kString) {
        return absl::StrCat("CAST('",
                            EscapeStringLiteralInner(cell.string_value()),
                            "' AS BOOLEAN)");
      }
      return std::string(cell.bool_value() ? "TRUE" : "FALSE");
    case schema::ColumnType::kInt64:
      if (cell.kind() == Value::Kind::kString) {
        return absl::StrCat("CAST('",
                            EscapeStringLiteralInner(cell.string_value()),
                            "' AS BIGINT)");
      }
      return absl::StrCat(cell.int64_value());
    case schema::ColumnType::kFloat64:
      return RenderFloatLiteral(cell);
    case schema::ColumnType::kString:
    case schema::ColumnType::kJson:
    case schema::ColumnType::kGeography:
      return absl::StrCat(
          "'", EscapeStringLiteralInner(cell.string_value()), "'");
    case schema::ColumnType::kBytes:
      return RenderBlobLiteral(cell.string_value());
    case schema::ColumnType::kDate: {
      std::string iso = cell.string_value();
      if (auto packed = TryFormatPackedDateString(iso); packed.has_value()) {
        iso = *packed;
      }
      return absl::StrCat("DATE '", EscapeStringLiteralInner(iso), "'");
    }
    case schema::ColumnType::kTime:
      return absl::StrCat(
          "TIME '", EscapeStringLiteralInner(cell.string_value()), "'");
    case schema::ColumnType::kDatetime:
      return absl::StrCat(
          "TIMESTAMP '", EscapeStringLiteralInner(cell.string_value()), "'");
    case schema::ColumnType::kTimestamp: {
      std::string ts = cell.string_value();
      if (auto micros = TryFormatMicrosTimestampString(ts);
          micros.has_value()) {
        ts = *micros;
      }
      return absl::StrCat("TIMESTAMPTZ '", EscapeStringLiteralInner(ts), "'");
    }
    case schema::ColumnType::kNumeric:
      return absl::StrCat("CAST('",
                          EscapeStringLiteralInner(cell.string_value()),
                          "' AS ",
                          schema::ToDuckDBType(column.type),
                          ")");
    case schema::ColumnType::kBignumeric:
      return absl::StrCat("CAST('",
                          EscapeStringLiteralInner(cell.string_value()),
                          "' AS ",
                          schema::ToDuckDBStorageType(column.type),
                          ")");
    case schema::ColumnType::kStruct:
      return RenderStructLiteral(cell, column);
    case schema::ColumnType::kArray:
    case schema::ColumnType::kUnknown:
      return absl::StrCat(
          "'", EscapeStringLiteralInner(cell.string_value()), "'");
  }
  return absl::InternalError("RenderScalarLiteral: unreachable");
}

absl::StatusOr<std::string> RenderCellLiteral(
    const Value& cell, const schema::ColumnSchema& column) {
  if (cell.is_null()) return std::string("NULL");
  // REPEATED cells carry an array on the wire even when the column
  // type itself is a scalar like INT64 — DuckDB's LIST literal form
  // is `[v1, v2, ...]`. Use a synthetic non-repeated column for the
  // element renderer so the recursive call doesn't re-enter the
  // array branch.
  if (column.mode == schema::ColumnMode::kRepeated) {
    if (cell.kind() != Value::Kind::kArray) {
      return absl::InvalidArgumentError(
          absl::StrCat("AppendRows: REPEATED column '",
                       column.name,
                       "' expects ARRAY but row provided kind ",
                       static_cast<int>(cell.kind())));
    }
    schema::ColumnSchema element = column;
    element.mode = schema::ColumnMode::kNullable;
    const auto& elems = cell.array_value();
    const std::string duckdb_list_type =
        schema::ColumnSchemaToDuckDBStorageType(column);
    if (elems.empty()) {
      return absl::StrCat("CAST([] AS ", duckdb_list_type, ")");
    }
    std::string out = "CAST([";
    for (size_t i = 0; i < elems.size(); ++i) {
      if (i > 0) absl::StrAppend(&out, ", ");
      auto inner_or = RenderCellLiteral(elems[i], element);
      if (!inner_or.ok()) return inner_or.status();
      absl::StrAppend(&out, *inner_or);
    }
    absl::StrAppend(&out, "] AS ", duckdb_list_type, ")");
    return out;
  }
  return RenderScalarLiteral(cell, column);
}

}  // namespace internal
}  // namespace duckdb
}  // namespace storage
}  // namespace backend
}  // namespace bigquery_emulator
