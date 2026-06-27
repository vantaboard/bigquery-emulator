#include "backend/engine/semantic/value.h"

#include <cctype>
#include <cstdint>
#include <cstdlib>
#include <string>
#include <vector>

#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "absl/strings/ascii.h"
#include "absl/strings/escaping.h"
#include "absl/strings/match.h"
#include "absl/strings/numbers.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/str_format.h"
#include "absl/strings/string_view.h"
#include "absl/strings/strip.h"
#include "absl/time/civil_time.h"
#include "absl/time/time.h"
#include "backend/engine/semantic/error.h"
#include "backend/engine/semantic/eval_context.h"
#include "backend/engine/semantic/geography_value.h"
#include "backend/schema/schema.h"
#include "backend/storage/storage.h"
#include "googlesql/public/civil_time.h"
#include "googlesql/public/functions/date_time_util.h"
#include "googlesql/public/functions/parse_date_time.h"
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
  const absl::TimeZone utc = absl::UTCTimeZone();
  const int64_t micros = absl::ToUnixMicros(t);
  const absl::CivilSecond cs = absl::ToCivilSecond(t, utc);
  if (micros % 1000000 == 0) {
    return absl::StrFormat("%04d-%02d-%02d %02d:%02d:%02d+00",
                           cs.year(),
                           cs.month(),
                           cs.day(),
                           cs.hour(),
                           cs.minute(),
                           cs.second());
  }
  const int64_t frac = micros >= 0 ? micros % 1000000 : -micros % 1000000;
  return absl::StrFormat("%04d-%02d-%02d %02d:%02d:%02d.%06d+00",
                         cs.year(),
                         cs.month(),
                         cs.day(),
                         cs.hour(),
                         cs.minute(),
                         cs.second(),
                         static_cast<int>(frac));
}

}  // namespace

std::string NormalizeTimestampOffsetSuffix(std::string text) {
  if (text.size() >= 3) {
    const size_t n = text.size();
    const char sign = text[n - 3];
    const char d0 = text[n - 2];
    const char d1 = text[n - 1];
    // Short UTC offset `...+HH` / `...-HH` (not already `...+HH:MM`).
    if ((sign == '+' || sign == '-') && absl::ascii_isdigit(d0) &&
        absl::ascii_isdigit(d1) && text[n - 3] == sign) {
      // Date-only `YYYY-MM-DD` also ends with `-DD`; require a time component.
      if (text.find(':') == std::string::npos &&
          text.find('T') == std::string::npos) {
        return text;
      }
      absl::StrAppend(&text, ":00");
    }
  }
  return text;
}

absl::StatusOr<Value> ParseTimestampWireString(absl::string_view body) {
  std::string text(body);
  if (absl::EndsWith(text, " UTC")) {
    text.resize(text.size() - 4);
  }
  text = NormalizeTimestampOffsetSuffix(std::move(text));
  int64_t micros = 0;
  absl::Time t;
  std::string err;
  const absl::TimeZone utc = absl::UTCTimeZone();
  constexpr ::googlesql::functions::TimestampScale kMicros =
      ::googlesql::functions::TimestampScale::kMicroseconds;
  if (absl::ParseTime(absl::RFC3339_full, text, &t, &err) ||
      absl::ParseTime("%Y-%m-%d %H:%M:%E*S%Ez", text, &t, &err) ||
      absl::ParseTime("%Y-%m-%dT%H:%M:%E*S%Ez", text, &t, &err) ||
      absl::ParseTime("%Y-%m-%d %H:%M:%E*S", text, &t, &err) ||
      absl::ParseTime("%Y-%m-%dT%H:%M:%E*S", text, &t, &err)) {
    return Value::TimestampFromUnixMicros(absl::ToUnixMicros(t));
  }
  if (auto s = ::googlesql::functions::ParseStringToTimestamp(
          "%F %T", text, utc, /*parse_version2=*/true, &micros);
      s.ok()) {
    return Value::TimestampFromUnixMicros(micros);
  }
  if (auto s = ::googlesql::functions::ConvertStringToTimestamp(
          text, utc, kMicros, /*allow_tz_in_str=*/true, &micros);
      s.ok()) {
    return Value::TimestampFromUnixMicros(micros);
  }
  if (absl::ParseTime("%Y-%m-%d", text, &t, &err)) {
    return Value::TimestampFromUnixMicros(absl::ToUnixMicros(t));
  }
  int32_t date = 0;
  if (auto date_status = ::googlesql::functions::ParseStringToDate(
          "%Y-%m-%d", text, /*parse_version2=*/true, &date);
      date_status.ok()) {
    if (auto convert_status = ::googlesql::functions::ConvertDateToTimestamp(
            date, kMicros, utc, &micros);
        convert_status.ok()) {
      return Value::TimestampFromUnixMicros(micros);
    }
  }
  return absl::InvalidArgumentError(
      absl::StrCat("semantic: invalid TIMESTAMP value '", body, "'"));
}

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
    case ::googlesql::TYPE_RANGE:
      return "RANGE";
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
  if (absl::StartsWithIgnoreCase(name, "RANGE")) {
    return ::googlesql::TYPE_RANGE;
  }
  if (absl::EqualsIgnoreCase(name, "UUID")) return ::googlesql::TYPE_UUID;
  if (absl::EqualsIgnoreCase(name, "ARRAY")) return ::googlesql::TYPE_ARRAY;
  if (absl::EqualsIgnoreCase(name, "STRUCT") ||
      absl::EqualsIgnoreCase(name, "RECORD")) {
    return ::googlesql::TYPE_STRUCT;
  }
  return ::googlesql::TYPE_UNKNOWN;
}

absl::StatusOr<storage::Value> ToStorageValue(const Value& value,
                                              const EvalContext* ctx) {
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
    case ::googlesql::TYPE_DATETIME: {
      // BigQuery REST / query port parity uses
      // "YYYY-MM-DDTHH:MM:SS[.ffffff]".
      std::string out = value.datetime_value().DebugString();
      const size_t sep = out.find(' ');
      if (sep != std::string::npos) {
        out[sep] = 'T';
      }
      return storage::Value::String(out);
    }
    case ::googlesql::TYPE_TIMESTAMP:
      return storage::Value::String(FormatTimestampUtc(value.ToTime()));
    case ::googlesql::TYPE_NUMERIC:
      return storage::Value::String(value.numeric_value().ToString());
    case ::googlesql::TYPE_BIGNUMERIC: {
      if (ctx != nullptr && ctx->bignumeric_render_override.has_value()) {
        storage::Value out =
            storage::Value::String(*ctx->bignumeric_render_override);
        ctx->bignumeric_render_override.reset();
        return out;
      }
      return storage::Value::String(value.bignumeric_value().ToString());
    }
    case ::googlesql::TYPE_JSON:
      return storage::Value::String(value.json_string());
    case ::googlesql::TYPE_GEOGRAPHY: {
      const std::string wkt = GeographyWkt(value);
      if (!wkt.empty()) {
        return storage::Value::String(wkt);
      }
      return storage::Value::String(
          value.GetSQLLiteral(::googlesql::PRODUCT_EXTERNAL));
    }
    case ::googlesql::TYPE_INTERVAL:
      return storage::Value::String(value.interval_value().ToString());
    case ::googlesql::TYPE_RANGE: {
      std::string lit = value.GetSQLLiteral(::googlesql::PRODUCT_EXTERNAL);
      for (char& c : lit) {
        if (c == '"') c = '\'';
      }
      return storage::Value::String(lit);
    }
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
    case ::googlesql::TYPE_PROTO:
      return storage::Value::Bytes(std::string(value.ToCord()));
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
    case ::googlesql::TYPE_PROTO:
      column.type = schema::ColumnType::kBytes;
      if (core->IsProto() && core->AsProto() != nullptr) {
        column.raw_type = absl::StrCat(
            "PROTO<", core->AsProto()->descriptor()->full_name(), ">");
      } else {
        column.raw_type = "PROTO";
      }
      break;
    case ::googlesql::TYPE_INTERVAL:
    case ::googlesql::TYPE_UUID:
    case ::googlesql::TYPE_RANGE:
      // INTERVAL, UUID, and RANGE have no first-class BigQuery REST
      // schema type in the engine proto today; the gateway accepts
      // STRING for the wire shape when the semantic executor renders
      // them. We surface kString here so the gateway-side schema
      // reflection lines up with the cell shape `ToStorageValue`
      // emits.
      column.type = schema::ColumnType::kString;
      if (core->kind() == ::googlesql::TYPE_INTERVAL) {
        column.raw_type = "INTERVAL";
      } else if (core->kind() == ::googlesql::TYPE_UUID) {
        column.raw_type = "UUID";
      } else {
        column.raw_type = core->TypeName(::googlesql::PRODUCT_EXTERNAL);
      }
      break;
    default:
      return absl::InvalidArgumentError(absl::StrCat("semantic: column '",
                                                     name,
                                                     "': unsupported type ",
                                                     core->DebugString()));
  }
  return column;
}

}  // namespace semantic
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
