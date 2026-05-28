#include "backend/schema/schema.h"

#include <cctype>
#include <string>

#include "absl/status/status.h"
#include "absl/strings/ascii.h"
#include "absl/strings/match.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/str_replace.h"
#include "absl/strings/string_view.h"

namespace bigquery_emulator {
namespace backend {
namespace schema {

namespace {

// Case-insensitive equality for the well-known BigQuery type / mode
// names. The wire protocol carries them as plain strings and BigQuery
// itself accepts either case, so we just delegate to absl's
// `EqualsIgnoreCase` (in absl/strings/match.h) under a local alias
// so the conversion sites read like a switch on a constant.
using ::absl::EqualsIgnoreCase;

}  // namespace

absl::string_view ColumnTypeName(ColumnType type) {
  switch (type) {
    case ColumnType::kInt64:
      return "INT64";
    case ColumnType::kFloat64:
      return "FLOAT64";
    case ColumnType::kBool:
      return "BOOL";
    case ColumnType::kString:
      return "STRING";
    case ColumnType::kBytes:
      return "BYTES";
    case ColumnType::kDate:
      return "DATE";
    case ColumnType::kTime:
      return "TIME";
    case ColumnType::kDatetime:
      return "DATETIME";
    case ColumnType::kTimestamp:
      return "TIMESTAMP";
    case ColumnType::kNumeric:
      return "NUMERIC";
    case ColumnType::kBignumeric:
      return "BIGNUMERIC";
    case ColumnType::kJson:
      return "JSON";
    case ColumnType::kGeography:
      return "GEOGRAPHY";
    case ColumnType::kArray:
      return "ARRAY";
    case ColumnType::kStruct:
      return "STRUCT";
    case ColumnType::kUnknown:
      return "";
  }
  return "";
}

ColumnType ParseColumnType(absl::string_view name) {
  // BigQuery v2 accepts both `INTEGER`/`INT64` and `FLOAT`/`FLOAT64` as
  // type aliases (see docs/bigquery/docs/data-types.md); legacy
  // `RECORD` maps to STRUCT. We accept all of them here and round-trip
  // back to the canonical name through ColumnTypeName.
  if (EqualsIgnoreCase(name, "INT64") || EqualsIgnoreCase(name, "INTEGER")) {
    return ColumnType::kInt64;
  }
  if (EqualsIgnoreCase(name, "FLOAT64") || EqualsIgnoreCase(name, "FLOAT")) {
    return ColumnType::kFloat64;
  }
  if (EqualsIgnoreCase(name, "BOOL") || EqualsIgnoreCase(name, "BOOLEAN")) {
    return ColumnType::kBool;
  }
  if (EqualsIgnoreCase(name, "STRING")) return ColumnType::kString;
  if (EqualsIgnoreCase(name, "BYTES")) return ColumnType::kBytes;
  if (EqualsIgnoreCase(name, "DATE")) return ColumnType::kDate;
  if (EqualsIgnoreCase(name, "TIME")) return ColumnType::kTime;
  if (EqualsIgnoreCase(name, "DATETIME")) return ColumnType::kDatetime;
  if (EqualsIgnoreCase(name, "TIMESTAMP")) return ColumnType::kTimestamp;
  if (EqualsIgnoreCase(name, "NUMERIC")) return ColumnType::kNumeric;
  if (EqualsIgnoreCase(name, "BIGNUMERIC") ||
      EqualsIgnoreCase(name, "BIGDECIMAL")) {
    return ColumnType::kBignumeric;
  }
  if (EqualsIgnoreCase(name, "JSON")) return ColumnType::kJson;
  if (EqualsIgnoreCase(name, "GEOGRAPHY")) return ColumnType::kGeography;
  if (EqualsIgnoreCase(name, "ARRAY")) return ColumnType::kArray;
  if (EqualsIgnoreCase(name, "STRUCT") || EqualsIgnoreCase(name, "RECORD")) {
    return ColumnType::kStruct;
  }
  return ColumnType::kUnknown;
}

absl::string_view ColumnModeName(ColumnMode mode) {
  switch (mode) {
    case ColumnMode::kNullable:
      return "NULLABLE";
    case ColumnMode::kRequired:
      return "REQUIRED";
    case ColumnMode::kRepeated:
      return "REPEATED";
  }
  return "NULLABLE";
}

ColumnMode ParseColumnMode(absl::string_view name) {
  if (name.empty()) return ColumnMode::kNullable;
  if (EqualsIgnoreCase(name, "REQUIRED")) return ColumnMode::kRequired;
  if (EqualsIgnoreCase(name, "REPEATED")) return ColumnMode::kRepeated;
  return ColumnMode::kNullable;
}

absl::StatusOr<ColumnSchema> ColumnSchemaFromProto(
    const v1::FieldSchema& proto) {
  if (proto.name().empty()) {
    return absl::InvalidArgumentError(
        "FieldSchema is missing the required `name` field");
  }

  ColumnSchema column;
  column.name = proto.name();
  column.type = ParseColumnType(proto.type());
  if (column.type == ColumnType::kUnknown) {
    column.raw_type = proto.type();
  }
  column.mode = ParseColumnMode(proto.mode());
  column.description = proto.description();

  column.fields.reserve(proto.fields_size());
  for (const auto& nested : proto.fields()) {
    auto child = ColumnSchemaFromProto(nested);
    if (!child.ok()) {
      return absl::Status(
          child.status().code(),
          absl::StrCat(proto.name(), ".", child.status().message()));
    }
    column.fields.push_back(std::move(*child));
  }
  return column;
}

absl::StatusOr<TableSchema> TableSchemaFromProto(const v1::TableSchema& proto) {
  TableSchema schema;
  schema.columns.reserve(proto.fields_size());
  for (const auto& field : proto.fields()) {
    auto column = ColumnSchemaFromProto(field);
    if (!column.ok()) return column.status();
    schema.columns.push_back(std::move(*column));
  }
  return schema;
}

void ColumnSchemaToProto(const ColumnSchema& column, v1::FieldSchema* out) {
  out->Clear();
  out->set_name(column.name);
  if (column.type == ColumnType::kUnknown && !column.raw_type.empty()) {
    out->set_type(column.raw_type);
  } else {
    out->set_type(std::string(ColumnTypeName(column.type)));
  }
  out->set_mode(std::string(ColumnModeName(column.mode)));
  out->set_description(column.description);
  for (const auto& nested : column.fields) {
    ColumnSchemaToProto(nested, out->add_fields());
  }
}

void TableSchemaToProto(const TableSchema& schema, v1::TableSchema* out) {
  out->Clear();
  for (const auto& column : schema.columns) {
    ColumnSchemaToProto(column, out->add_fields());
  }
}

absl::string_view ToDuckDBType(ColumnType type) {
  switch (type) {
    case ColumnType::kInt64:
      return "BIGINT";
    case ColumnType::kFloat64:
      return "DOUBLE";
    case ColumnType::kBool:
      return "BOOLEAN";
    case ColumnType::kString:
      return "VARCHAR";
    case ColumnType::kBytes:
      return "BLOB";
    case ColumnType::kDate:
      return "DATE";
    case ColumnType::kTime:
      return "TIME";
    // BigQuery DATETIME is naive (no zone). DuckDB TIMESTAMP is the
    // matching naive type.
    case ColumnType::kDatetime:
      return "TIMESTAMP";
    // BigQuery TIMESTAMP is always UTC; DuckDB has a TIMESTAMPTZ
    // alias that round-trips RFC 3339 strings the way the gateway
    // wire layer expects.
    case ColumnType::kTimestamp:
      return "TIMESTAMP WITH TIME ZONE";
    // BigQuery NUMERIC is fixed at 38/9; BIGNUMERIC at 76/38 which
    // exceeds DuckDB's max precision (38), so we lossily clamp the
    // scale on output. The DML work revisits the lossy-cast policy.
    case ColumnType::kNumeric:
      return "DECIMAL(38, 9)";
    case ColumnType::kBignumeric:
      return "DECIMAL(38, 38)";
    case ColumnType::kJson:
      return "JSON";
    case ColumnType::kGeography:
      return "VARCHAR";
    // ARRAY / STRUCT need their inner shape to form a real DuckDB
    // type expression; the caller should reach for
    // ColumnSchemaToDuckDBType instead. We return the bare kind name
    // so log messages and error strings remain readable.
    case ColumnType::kArray:
      return "LIST";
    case ColumnType::kStruct:
      return "STRUCT";
    case ColumnType::kUnknown:
      return "VARCHAR";
  }
  return "VARCHAR";
}

namespace {

// Returns the head identifier of a DuckDB type expression — the
// substring up to the first `(`, `[`, or whitespace. DuckDB renders
// parameterized types as `DECIMAL(38, 9)` / `STRUCT(a INT, b VARCHAR)`
// / `BIGINT[]`; for `FromDuckDBType` we only care about the head so a
// caller passing the full expression is treated the same as a caller
// passing the bare name.
absl::string_view TypeHead(absl::string_view name) {
  size_t i = 0;
  while (i < name.size()) {
    const char c = name[i];
    if (c == '(' || c == '[' || c == ' ' || c == '\t') break;
    ++i;
  }
  return name.substr(0, i);
}

}  // namespace

ColumnType FromDuckDBType(absl::string_view duckdb_type) {
  // Tolerate trailing `[]` so callers can pass `BIGINT[]` directly;
  // strip it before inspecting the head so the array-ness is
  // surfaced through the return value. We mirror the BigQuery
  // schema convention where the "array-ness" lives on `mode`
  // (REPEATED), not the type, so a `BIGINT[]` payload becomes
  // `kInt64` here — the caller is responsible for flipping the
  // owning ColumnSchema's mode to REPEATED.
  absl::string_view name = duckdb_type;
  while (!name.empty() && name.back() == ']')
    name.remove_suffix(1);
  while (!name.empty() && name.back() == '[')
    name.remove_suffix(1);
  const absl::string_view head = TypeHead(name);
  if (EqualsIgnoreCase(head, "BIGINT") || EqualsIgnoreCase(head, "INT8") ||
      EqualsIgnoreCase(head, "LONG") || EqualsIgnoreCase(head, "INT64") ||
      EqualsIgnoreCase(head, "INTEGER") || EqualsIgnoreCase(head, "INT") ||
      EqualsIgnoreCase(head, "INT4") || EqualsIgnoreCase(head, "SMALLINT") ||
      EqualsIgnoreCase(head, "INT2") || EqualsIgnoreCase(head, "TINYINT") ||
      EqualsIgnoreCase(head, "INT1")) {
    return ColumnType::kInt64;
  }
  if (EqualsIgnoreCase(head, "DOUBLE") || EqualsIgnoreCase(head, "FLOAT8") ||
      EqualsIgnoreCase(head, "REAL") || EqualsIgnoreCase(head, "FLOAT4") ||
      EqualsIgnoreCase(head, "FLOAT")) {
    return ColumnType::kFloat64;
  }
  if (EqualsIgnoreCase(head, "BOOLEAN") || EqualsIgnoreCase(head, "BOOL")) {
    return ColumnType::kBool;
  }
  if (EqualsIgnoreCase(head, "VARCHAR") || EqualsIgnoreCase(head, "TEXT") ||
      EqualsIgnoreCase(head, "STRING") || EqualsIgnoreCase(head, "CHAR")) {
    return ColumnType::kString;
  }
  if (EqualsIgnoreCase(head, "BLOB") || EqualsIgnoreCase(head, "BYTEA") ||
      EqualsIgnoreCase(head, "BINARY") || EqualsIgnoreCase(head, "VARBINARY") ||
      EqualsIgnoreCase(head, "BYTES")) {
    return ColumnType::kBytes;
  }
  if (EqualsIgnoreCase(head, "DATE")) return ColumnType::kDate;
  if (EqualsIgnoreCase(head, "TIME")) return ColumnType::kTime;
  if (EqualsIgnoreCase(head, "TIMESTAMPTZ") ||
      EqualsIgnoreCase(head, "TIMESTAMP_TZ")) {
    return ColumnType::kTimestamp;
  }
  if (EqualsIgnoreCase(head, "TIMESTAMP")) {
    // DuckDB's plain `TIMESTAMP` is naive (no zone) and maps to
    // BigQuery DATETIME; the zoned variant is rendered as
    // `TIMESTAMP WITH TIME ZONE`, sharing the same head — peek
    // for the suffix to disambiguate before falling through to
    // the naive case.
    if (absl::StrContains(absl::AsciiStrToUpper(std::string(name)),
                          "WITH TIME ZONE")) {
      return ColumnType::kTimestamp;
    }
    return ColumnType::kDatetime;
  }
  if (EqualsIgnoreCase(head, "DECIMAL") || EqualsIgnoreCase(head, "NUMERIC")) {
    return ColumnType::kNumeric;
  }
  if (EqualsIgnoreCase(head, "HUGEINT") ||
      EqualsIgnoreCase(head, "BIGNUMERIC") ||
      EqualsIgnoreCase(head, "BIGDECIMAL")) {
    return ColumnType::kBignumeric;
  }
  if (EqualsIgnoreCase(head, "JSON")) return ColumnType::kJson;
  if (EqualsIgnoreCase(head, "GEOMETRY") ||
      EqualsIgnoreCase(head, "GEOGRAPHY")) {
    return ColumnType::kGeography;
  }
  if (EqualsIgnoreCase(head, "STRUCT") || EqualsIgnoreCase(head, "ROW") ||
      EqualsIgnoreCase(head, "RECORD")) {
    return ColumnType::kStruct;
  }
  if (EqualsIgnoreCase(head, "LIST") || EqualsIgnoreCase(head, "ARRAY")) {
    return ColumnType::kArray;
  }
  return ColumnType::kUnknown;
}

namespace {

// Quotes a STRUCT field identifier for embedding in a DuckDB type
// expression: doubles embedded `"` and wraps the result. The DuckDB
// parser accepts the same identifier rules SQL does, so this is the
// same escape DuckDBStorage uses for schema / table names.
std::string QuoteStructFieldName(absl::string_view name) {
  std::string escaped = absl::StrReplaceAll(name, {{"\"", "\"\""}});
  return absl::StrCat("\"", escaped, "\"");
}

}  // namespace

std::string ColumnSchemaToDuckDBType(const ColumnSchema& column) {
  std::string inner;
  if (column.type == ColumnType::kStruct) {
    inner = "STRUCT(";
    for (size_t i = 0; i < column.fields.size(); ++i) {
      if (i > 0) absl::StrAppend(&inner, ", ");
      absl::StrAppend(&inner,
                      QuoteStructFieldName(column.fields[i].name),
                      " ",
                      ColumnSchemaToDuckDBType(column.fields[i]));
    }
    absl::StrAppend(&inner, ")");
  } else if (column.type == ColumnType::kArray) {
    // BigQuery treats ARRAY as a top-level kind only on REST-side
    // StandardSqlDataType payloads. Inside our internal ColumnSchema,
    // ARRAY columns are usually expressed by `mode = kRepeated` on
    // the element row. We still handle a literal `kArray` here for
    // the rare round-trip case: a one-element `fields` list carries
    // the element schema.
    if (column.fields.empty()) {
      inner = "VARCHAR";
    } else {
      inner = ColumnSchemaToDuckDBType(column.fields.front());
    }
    absl::StrAppend(&inner, "[]");
    if (column.mode == ColumnMode::kRepeated) {
      // Doubly-arrayed (REPEATED ARRAY) — rare but well-defined.
      absl::StrAppend(&inner, "[]");
    }
    return inner;
  } else {
    inner = std::string(ToDuckDBType(column.type));
  }
  if (column.mode == ColumnMode::kRepeated) {
    absl::StrAppend(&inner, "[]");
  }
  return inner;
}

}  // namespace schema
}  // namespace backend
}  // namespace bigquery_emulator
