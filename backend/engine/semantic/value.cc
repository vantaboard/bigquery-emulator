#include "backend/engine/semantic/value.h"

#include <cctype>
#include <cstdint>
#include <cstdlib>
#include <string>
#include <vector>

#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "absl/strings/ascii.h"
#include "absl/strings/match.h"
#include "absl/strings/numbers.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/str_format.h"
#include "absl/strings/string_view.h"
#include "absl/strings/strip.h"
#include "absl/time/civil_time.h"
#include "absl/time/time.h"
#include "backend/engine/semantic/error.h"
#include "backend/schema/schema.h"
#include "backend/storage/storage.h"
#include "googlesql/public/civil_time.h"
#include "googlesql/public/interval_value.h"
#include "googlesql/public/numeric_value.h"
#include "googlesql/public/type.h"
#include "googlesql/public/type.pb.h"
#include "googlesql/public/types/array_type.h"
#include "googlesql/public/types/struct_type.h"
#include "googlesql/public/uuid_value.h"
#include "googlesql/public/value.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace semantic {

namespace {

// Render a TIMESTAMP value (absl::Time -> UTC ISO-ish string the
// gateway can pass through unchanged). BigQuery's wire encoding is
// either RFC3339 with a `Z` suffix or microsecond-precision text;
// the DuckDB fast path's `arrow_to_bq.cc` emits microsecond
// precision without a TZ suffix, so we match that shape here so the
// gateway sees the same envelope regardless of route.
std::string FormatTimestampUtc(absl::Time t) {
  // %E*S formats fractional seconds with as few digits as needed,
  // trimming trailing zeros -- which matches GoogleSQL's
  // `Value::DebugString()` convention for TIMESTAMP and keeps the
  // wire payload compact for round seconds. We pin UTC so the
  // gateway / clients see a single canonical form regardless of
  // the emulator host's TZ.
  return absl::FormatTime("%Y-%m-%d %H:%M:%E*S+00", t, absl::UTCTimeZone());
}

// JSON unquoting helper for parameter parsing: strip a single layer
// of outer double quotes. The gateway already passes the body
// verbatim, so we just normalize the common bare-literal case.
absl::string_view StripJsonQuotes(absl::string_view body) {
  if (body.size() >= 2 && body.front() == '"' && body.back() == '"') {
    return body.substr(1, body.size() - 2);
  }
  return body;
}

}  // namespace

absl::string_view BigQueryTypeName(const ::googlesql::Type* type) {
  if (type == nullptr) return {};
  switch (type->kind()) {
    case ::googlesql::TYPE_BOOL:
      return "BOOL";
    case ::googlesql::TYPE_INT64:
      return "INT64";
    case ::googlesql::TYPE_DOUBLE:
      return "FLOAT64";
    case ::googlesql::TYPE_STRING:
      return "STRING";
    case ::googlesql::TYPE_BYTES:
      return "BYTES";
    case ::googlesql::TYPE_DATE:
      return "DATE";
    case ::googlesql::TYPE_TIME:
      return "TIME";
    case ::googlesql::TYPE_DATETIME:
      return "DATETIME";
    case ::googlesql::TYPE_TIMESTAMP:
      return "TIMESTAMP";
    case ::googlesql::TYPE_NUMERIC:
      return "NUMERIC";
    case ::googlesql::TYPE_BIGNUMERIC:
      return "BIGNUMERIC";
    case ::googlesql::TYPE_JSON:
      return "JSON";
    case ::googlesql::TYPE_GEOGRAPHY:
      return "GEOGRAPHY";
    case ::googlesql::TYPE_INTERVAL:
      return "INTERVAL";
    case ::googlesql::TYPE_UUID:
      return "UUID";
    case ::googlesql::TYPE_ARRAY:
      return "ARRAY";
    case ::googlesql::TYPE_STRUCT:
      return "STRUCT";
    default:
      return {};
  }
}

::googlesql::TypeKind ParseTypeKindName(absl::string_view type_kind_name) {
  // Normalize: accept both BigQuery REST (`INT64`) and GoogleSQL
  // (`TYPE_INT64`) spellings. We don't bother case-normalizing the
  // BigQuery side: BigQuery's REST API uses upper-case verbatim.
  absl::string_view name = type_kind_name;
  if (absl::StartsWithIgnoreCase(name, "TYPE_")) {
    name.remove_prefix(5);
  }
  if (absl::EqualsIgnoreCase(name, "BOOL")) return ::googlesql::TYPE_BOOL;
  if (absl::EqualsIgnoreCase(name, "BOOLEAN")) return ::googlesql::TYPE_BOOL;
  if (absl::EqualsIgnoreCase(name, "INT64")) return ::googlesql::TYPE_INT64;
  if (absl::EqualsIgnoreCase(name, "INTEGER")) return ::googlesql::TYPE_INT64;
  if (absl::EqualsIgnoreCase(name, "FLOAT64")) return ::googlesql::TYPE_DOUBLE;
  if (absl::EqualsIgnoreCase(name, "DOUBLE")) return ::googlesql::TYPE_DOUBLE;
  if (absl::EqualsIgnoreCase(name, "FLOAT")) return ::googlesql::TYPE_DOUBLE;
  if (absl::EqualsIgnoreCase(name, "STRING")) return ::googlesql::TYPE_STRING;
  if (absl::EqualsIgnoreCase(name, "BYTES")) return ::googlesql::TYPE_BYTES;
  if (absl::EqualsIgnoreCase(name, "DATE")) return ::googlesql::TYPE_DATE;
  if (absl::EqualsIgnoreCase(name, "TIME")) return ::googlesql::TYPE_TIME;
  if (absl::EqualsIgnoreCase(name, "DATETIME"))
    return ::googlesql::TYPE_DATETIME;
  if (absl::EqualsIgnoreCase(name, "TIMESTAMP"))
    return ::googlesql::TYPE_TIMESTAMP;
  if (absl::EqualsIgnoreCase(name, "NUMERIC")) return ::googlesql::TYPE_NUMERIC;
  if (absl::EqualsIgnoreCase(name, "BIGNUMERIC"))
    return ::googlesql::TYPE_BIGNUMERIC;
  if (absl::EqualsIgnoreCase(name, "JSON")) return ::googlesql::TYPE_JSON;
  if (absl::EqualsIgnoreCase(name, "GEOGRAPHY"))
    return ::googlesql::TYPE_GEOGRAPHY;
  if (absl::EqualsIgnoreCase(name, "INTERVAL"))
    return ::googlesql::TYPE_INTERVAL;
  if (absl::EqualsIgnoreCase(name, "UUID")) return ::googlesql::TYPE_UUID;
  return ::googlesql::TYPE_UNKNOWN;
}

absl::StatusOr<storage::Value> ToStorageValue(const Value& value) {
  if (!value.is_valid()) {
    return absl::InvalidArgumentError(
        "semantic: cannot lower invalid Value onto storage::Value");
  }
  if (value.is_null()) return storage::Value::Null();
  switch (value.type_kind()) {
    case ::googlesql::TYPE_BOOL:
      return storage::Value::Bool(value.bool_value());
    case ::googlesql::TYPE_INT64:
      return storage::Value::Int64(value.int64_value());
    case ::googlesql::TYPE_DOUBLE:
      return storage::Value::Float64(value.double_value());
    case ::googlesql::TYPE_STRING:
      return storage::Value::String(value.string_value());
    case ::googlesql::TYPE_BYTES:
      return storage::Value::Bytes(value.bytes_value());
    case ::googlesql::TYPE_DATE: {
      // BigQuery wire shape is "YYYY-MM-DD"; GoogleSQL stores DATE
      // as int32 days since 1970-01-01.
      absl::CivilDay day = value.ToCivilDay();
      return storage::Value::String(absl::StrFormat(
          "%04d-%02d-%02d", day.year(), day.month(), day.day()));
    }
    case ::googlesql::TYPE_TIME:
      // TimeValue::DebugString returns "HH:MM:SS[.ffffff]" with
      // trailing zeros trimmed (and the dot itself trimmed when
      // sub-second is zero), matching GoogleSQL's canonical TIME
      // text form.
      return storage::Value::String(value.time_value().DebugString());
    case ::googlesql::TYPE_DATETIME:
      // DatetimeValue::DebugString returns
      // "YYYY-MM-DD HH:MM:SS[.ffffff]" with the same trim rule.
      return storage::Value::String(value.datetime_value().DebugString());
    case ::googlesql::TYPE_TIMESTAMP:
      return storage::Value::String(FormatTimestampUtc(value.ToTime()));
    case ::googlesql::TYPE_NUMERIC:
      return storage::Value::String(value.numeric_value().ToString());
    case ::googlesql::TYPE_BIGNUMERIC:
      return storage::Value::String(value.bignumeric_value().ToString());
    case ::googlesql::TYPE_JSON:
      return storage::Value::String(value.json_string());
    case ::googlesql::TYPE_GEOGRAPHY:
      // Reuse the SQL literal -- GoogleSQL emits WKT for GEOGRAPHY.
      return storage::Value::String(
          value.GetSQLLiteral(::googlesql::PRODUCT_EXTERNAL));
    case ::googlesql::TYPE_INTERVAL:
      return storage::Value::String(value.interval_value().ToString());
    case ::googlesql::TYPE_UUID: {
      auto uuid = value.uuid_value();
      if (!uuid.ok()) return uuid.status();
      return storage::Value::String(uuid->ToString());
    }
    case ::googlesql::TYPE_ARRAY: {
      std::vector<storage::Value> elements;
      elements.reserve(value.num_elements());
      for (int i = 0; i < value.num_elements(); ++i) {
        auto e = ToStorageValue(value.element(i));
        if (!e.ok()) return e.status();
        elements.push_back(*std::move(e));
      }
      return storage::Value::Array(std::move(elements));
    }
    case ::googlesql::TYPE_STRUCT: {
      std::vector<storage::Value> fields;
      fields.reserve(value.num_fields());
      for (int i = 0; i < value.num_fields(); ++i) {
        auto f = ToStorageValue(value.field(i));
        if (!f.ok()) return f.status();
        fields.push_back(*std::move(f));
      }
      return storage::Value::Struct(std::move(fields));
    }
    default:
      return absl::InvalidArgumentError(absl::StrCat(
          "semantic: unsupported type for storage::Value lowering: ",
          value.type()->DebugString()));
  }
}

absl::StatusOr<schema::ColumnSchema> ColumnSchemaForType(
    const ::googlesql::Type* type, absl::string_view name) {
  if (type == nullptr) {
    return absl::InvalidArgumentError("semantic: null type for column schema");
  }
  schema::ColumnSchema column;
  column.name = std::string(name);
  column.mode = schema::ColumnMode::kNullable;
  const ::googlesql::Type* core = type;
  if (type->IsArray()) {
    column.mode = schema::ColumnMode::kRepeated;
    core = type->AsArray()->element_type();
    if (core != nullptr && core->IsArray()) {
      return absl::InvalidArgumentError(absl::StrCat(
          "semantic: column '",
          name,
          "': ARRAY<ARRAY<...>> is not representable in BigQuery"));
    }
  }
  if (core == nullptr) {
    return absl::InvalidArgumentError(
        absl::StrCat("semantic: column '", name, "': null element type"));
  }
  switch (core->kind()) {
    case ::googlesql::TYPE_BOOL:
      column.type = schema::ColumnType::kBool;
      break;
    case ::googlesql::TYPE_INT64:
      column.type = schema::ColumnType::kInt64;
      break;
    case ::googlesql::TYPE_DOUBLE:
      column.type = schema::ColumnType::kFloat64;
      break;
    case ::googlesql::TYPE_STRING:
      column.type = schema::ColumnType::kString;
      break;
    case ::googlesql::TYPE_BYTES:
      column.type = schema::ColumnType::kBytes;
      break;
    case ::googlesql::TYPE_DATE:
      column.type = schema::ColumnType::kDate;
      break;
    case ::googlesql::TYPE_TIME:
      column.type = schema::ColumnType::kTime;
      break;
    case ::googlesql::TYPE_DATETIME:
      column.type = schema::ColumnType::kDatetime;
      break;
    case ::googlesql::TYPE_TIMESTAMP:
      column.type = schema::ColumnType::kTimestamp;
      break;
    case ::googlesql::TYPE_NUMERIC:
      column.type = schema::ColumnType::kNumeric;
      break;
    case ::googlesql::TYPE_BIGNUMERIC:
      column.type = schema::ColumnType::kBignumeric;
      break;
    case ::googlesql::TYPE_JSON:
      column.type = schema::ColumnType::kJson;
      break;
    case ::googlesql::TYPE_GEOGRAPHY:
      column.type = schema::ColumnType::kGeography;
      break;
    case ::googlesql::TYPE_STRUCT: {
      column.type = schema::ColumnType::kStruct;
      const auto* st = core->AsStruct();
      for (int i = 0; i < st->num_fields(); ++i) {
        const auto& f = st->field(i);
        auto child = ColumnSchemaForType(f.type, f.name);
        if (!child.ok()) return child.status();
        column.fields.push_back(*std::move(child));
      }
      break;
    }
    case ::googlesql::TYPE_INTERVAL:
    case ::googlesql::TYPE_UUID:
      // INTERVAL and UUID have no first-class BigQuery REST schema
      // type today; the gateway accepts STRING for the wire shape
      // when the semantic executor renders them. We surface kString
      // here so the gateway-side schema reflection lines up with
      // the cell shape `ToStorageValue` emits.
      column.type = schema::ColumnType::kString;
      column.raw_type = std::string(
          core->kind() == ::googlesql::TYPE_INTERVAL ? "INTERVAL" : "UUID");
      break;
    default:
      return absl::InvalidArgumentError(absl::StrCat("semantic: column '",
                                                     name,
                                                     "': unsupported type ",
                                                     core->DebugString()));
  }
  return column;
}

absl::StatusOr<Value> ParseParameterValue(absl::string_view value_json,
                                          absl::string_view type_kind_name) {
  ::googlesql::TypeKind kind = ParseTypeKindName(type_kind_name);
  if (kind == ::googlesql::TYPE_UNKNOWN) {
    return absl::InvalidArgumentError(absl::StrCat(
        "semantic: unknown parameter type kind '", type_kind_name, "'"));
  }
  absl::string_view trimmed = absl::StripAsciiWhitespace(value_json);
  // BigQuery's REST `QueryParameterValue.value` is always a JSON
  // string; the gateway forwards it verbatim. NULL is encoded as a
  // missing field at the gateway, which collapses to an empty
  // `value_json` here.
  if (trimmed.empty() || trimmed == "null") {
    switch (kind) {
      case ::googlesql::TYPE_BOOL:
        return Value::NullBool();
      case ::googlesql::TYPE_INT64:
        return Value::NullInt64();
      case ::googlesql::TYPE_DOUBLE:
        return Value::NullDouble();
      case ::googlesql::TYPE_STRING:
        return Value::NullString();
      case ::googlesql::TYPE_BYTES:
        return Value::NullBytes();
      case ::googlesql::TYPE_DATE:
        return Value::NullDate();
      case ::googlesql::TYPE_TIME:
        return Value::NullTime();
      case ::googlesql::TYPE_DATETIME:
        return Value::NullDatetime();
      case ::googlesql::TYPE_TIMESTAMP:
        return Value::NullTimestamp();
      case ::googlesql::TYPE_NUMERIC:
        return Value::NullNumeric();
      case ::googlesql::TYPE_BIGNUMERIC:
        return Value::NullBigNumeric();
      case ::googlesql::TYPE_JSON:
        return Value::NullJson();
      default:
        return MakeSemanticError(
            SemanticErrorReason::kNotImplemented,
            absl::StrCat("semantic: NULL parameter for type kind '",
                         type_kind_name,
                         "' is not yet implemented"));
    }
  }
  absl::string_view body = StripJsonQuotes(trimmed);
  switch (kind) {
    case ::googlesql::TYPE_BOOL: {
      if (absl::EqualsIgnoreCase(trimmed, "true") ||
          absl::EqualsIgnoreCase(body, "true")) {
        return Value::Bool(true);
      }
      if (absl::EqualsIgnoreCase(trimmed, "false") ||
          absl::EqualsIgnoreCase(body, "false")) {
        return Value::Bool(false);
      }
      return absl::InvalidArgumentError(absl::StrCat(
          "semantic: invalid BOOL parameter value '", value_json, "'"));
    }
    case ::googlesql::TYPE_INT64: {
      int64_t v = 0;
      if (!absl::SimpleAtoi(body, &v)) {
        return absl::InvalidArgumentError(absl::StrCat(
            "semantic: invalid INT64 parameter value '", value_json, "'"));
      }
      return Value::Int64(v);
    }
    case ::googlesql::TYPE_DOUBLE: {
      double v = 0;
      if (!absl::SimpleAtod(body, &v)) {
        return absl::InvalidArgumentError(absl::StrCat(
            "semantic: invalid FLOAT64 parameter value '", value_json, "'"));
      }
      return Value::Double(v);
    }
    case ::googlesql::TYPE_STRING:
      return Value::String(std::string(body));
    case ::googlesql::TYPE_BYTES:
      return Value::Bytes(std::string(body));
    case ::googlesql::TYPE_NUMERIC: {
      auto n = ::googlesql::NumericValue::FromString(body);
      if (!n.ok()) return n.status();
      return Value::Numeric(*n);
    }
    case ::googlesql::TYPE_BIGNUMERIC: {
      auto n = ::googlesql::BigNumericValue::FromString(body);
      if (!n.ok()) return n.status();
      return Value::BigNumeric(*n);
    }
    case ::googlesql::TYPE_DATE:
    case ::googlesql::TYPE_TIME:
    case ::googlesql::TYPE_DATETIME:
    case ::googlesql::TYPE_TIMESTAMP:
    case ::googlesql::TYPE_JSON:
    case ::googlesql::TYPE_INTERVAL:
    case ::googlesql::TYPE_UUID:
      // Date / time / json / interval / uuid parameter binding
      // requires the same FromString-style parsers GoogleSQL ships
      // for literals; routing them through is in scope for the
      // semantic functions plan
      // (`semantic-functions-compliance.plan.md`). Surface
      // NOT_IMPLEMENTED today so the wire envelope is consistent.
      return MakeSemanticError(
          SemanticErrorReason::kNotImplemented,
          absl::StrCat("semantic: parameter type '",
                       type_kind_name,
                       "' is not yet supported; see "
                       "semantic-functions-compliance.plan.md"));
    case ::googlesql::TYPE_ARRAY:
    case ::googlesql::TYPE_STRUCT:
      return MakeSemanticError(
          SemanticErrorReason::kNotImplemented,
          absl::StrCat("semantic: ARRAY / STRUCT parameters are owned by "
                       "array-struct-semantic-path.plan.md"));
    default:
      return absl::InvalidArgumentError(absl::StrCat(
          "semantic: unsupported parameter type kind '", type_kind_name, "'"));
  }
}

}  // namespace semantic
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
