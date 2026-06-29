#include "backend/schema/schema.h"

#include <cctype>
#include <cstddef>
#include <string>
#include <utility>

#include "absl/container/flat_hash_map.h"
#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "absl/strings/ascii.h"
#include "absl/strings/match.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/str_replace.h"
#include "absl/strings/string_view.h"
#include "proto/emulator.pb.h"

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

absl::string_view ToDuckDBStorageType(ColumnType type) {
  if (type == ColumnType::kBignumeric) {
    return "VARCHAR";
  }
  return ToDuckDBType(type);
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

namespace {

// Table-driven `head -> ColumnType` lookup powering `FromDuckDBType`.
// Each row is an uppercase DuckDB type alias the engine accepts; the
// `TIMESTAMP` head is intentionally absent because it disambiguates
// on the `WITH TIME ZONE` suffix and is handled inline below.
const absl::flat_hash_map<absl::string_view, ColumnType>& DuckDBTypeAliases() {
  static const auto* kAliases =
      new absl::flat_hash_map<absl::string_view, ColumnType>{
          {"BIGINT", ColumnType::kInt64},
          {"INT8", ColumnType::kInt64},
          {"LONG", ColumnType::kInt64},
          {"INT64", ColumnType::kInt64},
          {"INTEGER", ColumnType::kInt64},
          {"INT", ColumnType::kInt64},
          {"INT4", ColumnType::kInt64},
          {"SMALLINT", ColumnType::kInt64},
          {"INT2", ColumnType::kInt64},
          {"TINYINT", ColumnType::kInt64},
          {"INT1", ColumnType::kInt64},
          {"DOUBLE", ColumnType::kFloat64},
          {"FLOAT8", ColumnType::kFloat64},
          {"REAL", ColumnType::kFloat64},
          {"FLOAT4", ColumnType::kFloat64},
          {"FLOAT", ColumnType::kFloat64},
          {"BOOLEAN", ColumnType::kBool},
          {"BOOL", ColumnType::kBool},
          {"VARCHAR", ColumnType::kString},
          {"TEXT", ColumnType::kString},
          {"STRING", ColumnType::kString},
          {"CHAR", ColumnType::kString},
          {"BLOB", ColumnType::kBytes},
          {"BYTEA", ColumnType::kBytes},
          {"BINARY", ColumnType::kBytes},
          {"VARBINARY", ColumnType::kBytes},
          {"BYTES", ColumnType::kBytes},
          {"DATE", ColumnType::kDate},
          {"TIME", ColumnType::kTime},
          {"TIMESTAMPTZ", ColumnType::kTimestamp},
          {"TIMESTAMP_TZ", ColumnType::kTimestamp},
          {"DECIMAL", ColumnType::kNumeric},
          {"NUMERIC", ColumnType::kNumeric},
          {"HUGEINT", ColumnType::kBignumeric},
          {"BIGNUMERIC", ColumnType::kBignumeric},
          {"BIGDECIMAL", ColumnType::kBignumeric},
          {"JSON", ColumnType::kJson},
          {"GEOMETRY", ColumnType::kGeography},
          {"GEOGRAPHY", ColumnType::kGeography},
          {"STRUCT", ColumnType::kStruct},
          {"ROW", ColumnType::kStruct},
          {"RECORD", ColumnType::kStruct},
          {"LIST", ColumnType::kArray},
          {"ARRAY", ColumnType::kArray},
      };
  return *kAliases;
}

// Drop trailing `[]` markers so callers can pass `BIGINT[]` directly.
// Mirrors the BigQuery schema convention where array-ness lives on
// `mode` (REPEATED), not the type itself.
absl::string_view StripArraySuffix(absl::string_view name) {
  while (!name.empty() && name.back() == ']')
    name.remove_suffix(1);
  while (!name.empty() && name.back() == '[')
    name.remove_suffix(1);
  return name;
}

// DuckDB's plain `TIMESTAMP` is naive (no zone) and maps to BigQuery
// DATETIME; the zoned variant is rendered as `TIMESTAMP WITH TIME
// ZONE` and shares the same head, so we peek at the full type
// expression for the suffix to disambiguate.
ColumnType ResolveTimestampVariant(absl::string_view name) {
  if (absl::StrContains(absl::AsciiStrToUpper(std::string(name)),
                        "WITH TIME ZONE")) {
    return ColumnType::kTimestamp;
  }
  return ColumnType::kDatetime;
}

}  // namespace

ColumnType FromDuckDBType(absl::string_view duckdb_type) {
  const absl::string_view name = StripArraySuffix(duckdb_type);
  const absl::string_view head = TypeHead(name);
  if (EqualsIgnoreCase(head, "TIMESTAMP")) {
    return ResolveTimestampVariant(name);
  }
  const std::string head_upper = absl::AsciiStrToUpper(std::string(head));
  const auto& aliases = DuckDBTypeAliases();
  const auto it = aliases.find(head_upper);
  if (it != aliases.end()) return it->second;
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

namespace {

using TypeFn = absl::string_view (*)(ColumnType);

std::string ColumnSchemaToDuckDBTypeWithFn(const ColumnSchema& column,
                                           TypeFn scalar_type) {
  std::string inner;
  if (column.type == ColumnType::kStruct) {
    inner = "STRUCT(";
    for (size_t i = 0; i < column.fields.size(); ++i) {
      if (i > 0) absl::StrAppend(&inner, ", ");
      absl::StrAppend(
          &inner,
          QuoteStructFieldName(column.fields[i].name),
          " ",
          ColumnSchemaToDuckDBTypeWithFn(column.fields[i], scalar_type));
    }
    absl::StrAppend(&inner, ")");
  } else if (column.type == ColumnType::kArray) {
    if (column.fields.empty()) {
      inner = "VARCHAR";
    } else {
      inner =
          ColumnSchemaToDuckDBTypeWithFn(column.fields.front(), scalar_type);
    }
    absl::StrAppend(&inner, "[]");
    if (column.mode == ColumnMode::kRepeated) {
      absl::StrAppend(&inner, "[]");
    }
    return inner;
  } else {
    inner = std::string(scalar_type(column.type));
  }
  if (column.mode == ColumnMode::kRepeated) {
    absl::StrAppend(&inner, "[]");
  }
  return inner;
}

}  // namespace

std::string ColumnSchemaToDuckDBType(const ColumnSchema& column) {
  return ColumnSchemaToDuckDBTypeWithFn(column, ToDuckDBType);
}

std::string ColumnSchemaToDuckDBStorageType(const ColumnSchema& column) {
  return ColumnSchemaToDuckDBTypeWithFn(column, ToDuckDBStorageType);
}

}  // namespace schema
}  // namespace backend
}  // namespace bigquery_emulator
