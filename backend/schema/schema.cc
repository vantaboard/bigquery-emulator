#include "backend/schema/schema.h"

#include <cctype>
#include <string>

#include "absl/status/status.h"
#include "absl/strings/ascii.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/string_view.h"

namespace bigquery_emulator {
namespace backend {
namespace schema {

namespace {

// Case-insensitive equality for the well-known BigQuery type / mode
// names. The wire protocol carries them as plain strings and BigQuery
// itself accepts either case, so we do the same.
bool EqualsIgnoreCase(absl::string_view a, absl::string_view b) {
  if (a.size() != b.size()) return false;
  for (size_t i = 0; i < a.size(); ++i) {
    if (absl::ascii_toupper(a[i]) != absl::ascii_toupper(b[i])) return false;
  }
  return true;
}

}  // namespace

absl::string_view ColumnTypeName(ColumnType type) {
  switch (type) {
    case ColumnType::kInt64: return "INT64";
    case ColumnType::kFloat64: return "FLOAT64";
    case ColumnType::kBool: return "BOOL";
    case ColumnType::kString: return "STRING";
    case ColumnType::kBytes: return "BYTES";
    case ColumnType::kDate: return "DATE";
    case ColumnType::kTime: return "TIME";
    case ColumnType::kDatetime: return "DATETIME";
    case ColumnType::kTimestamp: return "TIMESTAMP";
    case ColumnType::kNumeric: return "NUMERIC";
    case ColumnType::kBignumeric: return "BIGNUMERIC";
    case ColumnType::kJson: return "JSON";
    case ColumnType::kGeography: return "GEOGRAPHY";
    case ColumnType::kArray: return "ARRAY";
    case ColumnType::kStruct: return "STRUCT";
    case ColumnType::kUnknown: return "";
  }
  return "";
}

ColumnType ParseColumnType(absl::string_view name) {
  // BigQuery v2 accepts both `INTEGER`/`INT64` and `FLOAT`/`FLOAT64` as
  // type aliases (see docs/bigquery/docs/data-types.md); legacy
  // `RECORD` maps to STRUCT. We accept all of them here and round-trip
  // back to the canonical name through ColumnTypeName.
  if (EqualsIgnoreCase(name, "INT64") ||
      EqualsIgnoreCase(name, "INTEGER")) {
    return ColumnType::kInt64;
  }
  if (EqualsIgnoreCase(name, "FLOAT64") ||
      EqualsIgnoreCase(name, "FLOAT")) {
    return ColumnType::kFloat64;
  }
  if (EqualsIgnoreCase(name, "BOOL") ||
      EqualsIgnoreCase(name, "BOOLEAN")) {
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
  if (EqualsIgnoreCase(name, "STRUCT") ||
      EqualsIgnoreCase(name, "RECORD")) {
    return ColumnType::kStruct;
  }
  return ColumnType::kUnknown;
}

absl::string_view ColumnModeName(ColumnMode mode) {
  switch (mode) {
    case ColumnMode::kNullable: return "NULLABLE";
    case ColumnMode::kRequired: return "REQUIRED";
    case ColumnMode::kRepeated: return "REPEATED";
  }
  return "NULLABLE";
}

ColumnMode ParseColumnMode(absl::string_view name) {
  if (name.empty()) return ColumnMode::kNullable;
  if (EqualsIgnoreCase(name, "REQUIRED")) return ColumnMode::kRequired;
  if (EqualsIgnoreCase(name, "REPEATED")) return ColumnMode::kRepeated;
  return ColumnMode::kNullable;
}

absl::StatusOr<ColumnSchema> ColumnSchemaFromProto(const v1::FieldSchema& proto) {
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
      return absl::Status(child.status().code(),
                          absl::StrCat(proto.name(), ".",
                                       child.status().message()));
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

}  // namespace schema
}  // namespace backend
}  // namespace bigquery_emulator
