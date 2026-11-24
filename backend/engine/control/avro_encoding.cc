#include "backend/engine/control/avro_encoding.h"

#include <cstdint>
#include <cstring>
#include <string>
#include <vector>

#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "absl/strings/ascii.h"
#include "absl/strings/numbers.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/str_format.h"
#include "absl/strings/string_view.h"
#include "absl/strings/strip.h"
#include "backend/schema/schema.h"
#include "backend/storage/storage.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace control {
namespace internal {

void AppendAvroLong(std::string* out, int64_t v) {
  uint64_t zigzag =
      (static_cast<uint64_t>(v) << 1) ^ static_cast<uint64_t>(v >> 63);
  while (zigzag >= 0x80) {
    out->push_back(static_cast<char>((zigzag & 0x7f) | 0x80));
    zigzag >>= 7;
  }
  out->push_back(static_cast<char>(zigzag));
}

void AppendAvroBytes(std::string* out, absl::string_view bytes) {
  AppendAvroLong(out, static_cast<int64_t>(bytes.size()));
  out->append(bytes.data(), bytes.size());
}

namespace {

using schema::ColumnMode;
using schema::ColumnSchema;
using schema::ColumnType;
using storage::Value;

constexpr int64_t kMicrosPerDay = int64_t{86400} * 1'000'000;

std::string JsonString(absl::string_view s) {
  std::string out = "\"";
  for (char c : s) {
    switch (c) {
      case '"':
        out += "\\\"";
        break;
      case '\\':
        out += "\\\\";
        break;
      default:
        if (static_cast<unsigned char>(c) < 0x20) {
          absl::StrAppendFormat(&out, "\\u%04x", c);
        } else {
          out.push_back(c);
        }
    }
  }
  out.push_back('"');
  return out;
}

absl::StatusOr<std::string> FieldTypeJson(const ColumnSchema& column,
                                          absl::string_view ns,
                                          bool logical);

absl::StatusOr<std::string> RecordFieldsJson(
    const std::vector<ColumnSchema>& columns,
    absl::string_view ns,
    bool logical) {
  std::string out = "[";
  for (size_t i = 0; i < columns.size(); ++i) {
    absl::StatusOr<std::string> type = FieldTypeJson(columns[i], ns, logical);
    if (!type.ok()) return type.status();
    absl::StrAppend(&out,
                    i > 0 ? "," : "",
                    "{\"name\":",
                    JsonString(columns[i].name),
                    ",\"type\":",
                    *type,
                    "}");
  }
  out.push_back(']');
  return out;
}

absl::StatusOr<std::string> BaseTypeJson(const ColumnSchema& column,
                                         absl::string_view ns,
                                         bool logical) {
  switch (column.type) {
    case ColumnType::kInt64:
      return std::string("\"long\"");
    case ColumnType::kFloat64:
      return std::string("\"double\"");
    case ColumnType::kBool:
      return std::string("\"boolean\"");
    case ColumnType::kString:
      return std::string("\"string\"");
    case ColumnType::kBytes:
      return std::string("\"bytes\"");
    case ColumnType::kJson:
      return std::string(R"({"type":"string","sqlType":"JSON"})");
    case ColumnType::kGeography:
      return std::string(R"({"type":"string","sqlType":"GEOGRAPHY"})");
    case ColumnType::kNumeric:
      return std::string(
          R"({"type":"bytes","logicalType":"decimal","precision":38,"scale":9})");
    case ColumnType::kBignumeric:
      return std::string(
          R"({"type":"bytes","logicalType":"decimal","precision":77,"scale":38})");
    case ColumnType::kDate:
      return std::string(logical ? R"({"type":"int","logicalType":"date"})"
                                 : "\"string\"");
    case ColumnType::kTime:
      return std::string(logical
                             ? R"({"type":"long","logicalType":"time-micros"})"
                             : "\"string\"");
    case ColumnType::kDatetime:
      return std::string(logical
                             ? R"({"type":"string","logicalType":"datetime"})"
                             : "\"string\"");
    case ColumnType::kTimestamp:
      return std::string(
          logical ? R"({"type":"long","logicalType":"timestamp-micros"})"
                  : "\"string\"");
    case ColumnType::kStruct: {
      absl::StatusOr<std::string> fields = RecordFieldsJson(
          column.fields, absl::StrCat(ns, ".", column.name), logical);
      if (!fields.ok()) return fields.status();
      return absl::StrCat("{\"type\":\"record\",\"name\":",
                          JsonString(column.name),
                          ",\"namespace\":",
                          JsonString(ns),
                          ",\"fields\":",
                          *fields,
                          "}");
    }
    case ColumnType::kArray:
    case ColumnType::kUnknown:
      break;
  }
  return absl::InvalidArgumentError(
      absl::StrCat("EXPORT DATA: column '",
                   column.name,
                   "' of type ",
                   column.raw_type.empty() ? schema::ColumnTypeName(column.type)
                                           : absl::string_view(column.raw_type),
                   " is not supported in AVRO export"));
}

absl::StatusOr<std::string> FieldTypeJson(const ColumnSchema& column,
                                          absl::string_view ns,
                                          bool logical) {
  absl::StatusOr<std::string> base = BaseTypeJson(column, ns, logical);
  if (!base.ok()) return base.status();
  switch (column.mode) {
    case ColumnMode::kRepeated:
      return absl::StrCat("{\"type\":\"array\",\"items\":", *base, "}");
    case ColumnMode::kRequired:
      return *base;
    case ColumnMode::kNullable:
      break;
  }
  return absl::StrCat("[\"null\",", *base, "]");
}

absl::Status BadValue(const ColumnSchema& column, absl::string_view value) {
  return absl::InvalidArgumentError(
      absl::StrCat("EXPORT DATA: cannot encode value '",
                   value,
                   "' of column '",
                   column.name,
                   "' as ",
                   schema::ColumnTypeName(column.type),
                   " in AVRO"));
}

int64_t DaysFromCivil(int64_t y, int64_t m, int64_t d) {
  y -= m <= 2 ? 1 : 0;
  const int64_t era = (y >= 0 ? y : y - 399) / 400;
  const int64_t yoe = y - era * 400;
  const int64_t doy = (153 * (m + (m > 2 ? -3 : 9)) + 2) / 5 + d - 1;
  const int64_t doe = yoe * 365 + yoe / 4 - yoe / 100 + doy;
  return era * 146097 + doe - 719468;
}

bool ParseDays(absl::string_view s, int64_t* days) {
  int64_t y = 0;
  int64_t m = 0;
  int64_t d = 0;
  if (s.size() != 10 || s[4] != '-' || s[7] != '-' ||
      !absl::SimpleAtoi(s.substr(0, 4), &y) ||
      !absl::SimpleAtoi(s.substr(5, 2), &m) ||
      !absl::SimpleAtoi(s.substr(8, 2), &d)) {
    return false;
  }
  *days = DaysFromCivil(y, m, d);
  return true;
}

bool ParseClockMicros(absl::string_view s, int64_t* micros) {
  int64_t h = 0;
  int64_t m = 0;
  int64_t sec = 0;
  if (s.size() < 8 || s[2] != ':' || s[5] != ':' ||
      !absl::SimpleAtoi(s.substr(0, 2), &h) ||
      !absl::SimpleAtoi(s.substr(3, 2), &m) ||
      !absl::SimpleAtoi(s.substr(6, 2), &sec)) {
    return false;
  }
  int64_t frac = 0;
  if (s.size() > 8) {
    std::string digits(s.substr(9));
    if (s[8] != '.' || digits.empty()) return false;
    digits.resize(6, '0');
    if (!absl::SimpleAtoi(digits, &frac)) return false;
  }
  *micros = ((h * 60 + m) * 60 + sec) * 1'000'000 + frac;
  return true;
}

bool ParseDateTimeMicros(absl::string_view s, int64_t* micros) {
  int64_t days = 0;
  int64_t clock = 0;
  if (s.size() < 19 || (s[10] != ' ' && s[10] != 'T') ||
      !ParseDays(s.substr(0, 10), &days) ||
      !ParseClockMicros(s.substr(11), &clock)) {
    return false;
  }
  *micros = days * kMicrosPerDay + clock;
  return true;
}

std::string FormatClock(int64_t micros) {
  const int64_t secs = micros / 1'000'000;
  const int64_t frac = micros % 1'000'000;
  std::string out = absl::StrFormat(
      "%02d:%02d:%02d", secs / 3600, (secs / 60) % 60, secs % 60);
  if (frac != 0) absl::StrAppendFormat(&out, ".%06d", frac);
  return out;
}

std::string FormatDateTime(absl::string_view value, int64_t micros) {
  int64_t clock = micros % kMicrosPerDay;
  if (clock < 0) clock += kMicrosPerDay;
  return absl::StrCat(value.substr(0, 10), "T", FormatClock(clock));
}

absl::Status AppendDecimal(std::string* out,
                           const ColumnSchema& column,
                           absl::string_view text) {
  const int scale = column.type == ColumnType::kNumeric ? 9 : 38;
  absl::string_view s = absl::StripAsciiWhitespace(text);
  const bool negative = absl::ConsumePrefix(&s, "-");
  if (!negative) absl::ConsumePrefix(&s, "+");
  const size_t dot = s.find('.');
  std::string digits(s.substr(0, dot));
  std::string frac(dot == absl::string_view::npos ? "" : s.substr(dot + 1));
  if (frac.size() > static_cast<size_t>(scale)) {
    if (frac.find_first_not_of('0', scale) != std::string::npos) {
      return BadValue(column, text);
    }
    frac.resize(scale);
  }
  frac.resize(scale, '0');
  digits += frac;
  if (digits.empty()) return BadValue(column, text);
  std::vector<uint8_t> le;
  for (char c : digits) {
    if (!absl::ascii_isdigit(static_cast<unsigned char>(c))) {
      return BadValue(column, text);
    }
    uint32_t carry = static_cast<uint32_t>(c - '0');
    for (uint8_t& b : le) {
      const uint32_t v = static_cast<uint32_t>(b) * 10 + carry;
      b = static_cast<uint8_t>(v & 0xff);
      carry = v >> 8;
    }
    if (carry != 0) le.push_back(static_cast<uint8_t>(carry));
  }
  le.push_back(0);
  if (negative) {
    uint32_t carry = 1;
    for (uint8_t& b : le) {
      const uint32_t v =
          static_cast<uint32_t>(static_cast<uint8_t>(~b)) + carry;
      b = static_cast<uint8_t>(v & 0xff);
      carry = v >> 8;
    }
  }
  while (le.size() > 1) {
    const uint8_t top = le.back();
    const bool next_negative = (le[le.size() - 2] & 0x80) != 0;
    if ((top == 0x00 && !next_negative) || (top == 0xff && next_negative)) {
      le.pop_back();
    } else {
      break;
    }
  }
  AppendAvroLong(out, static_cast<int64_t>(le.size()));
  for (auto it = le.rbegin(); it != le.rend(); ++it) {
    out->push_back(static_cast<char>(*it));
  }
  return absl::OkStatus();
}

absl::Status AppendField(std::string* out,
                         const ColumnSchema& column,
                         const Value& value,
                         bool logical);

absl::Status AppendBase(std::string* out,
                        const ColumnSchema& column,
                        const Value& value,
                        bool logical) {
  const std::string& text = value.string_value();
  switch (column.type) {
    case ColumnType::kInt64:
      if (value.kind() != Value::Kind::kInt64) return BadValue(column, text);
      AppendAvroLong(out, value.int64_value());
      return absl::OkStatus();
    case ColumnType::kFloat64: {
      double v = value.float64_value();
      if (value.kind() == Value::Kind::kInt64) {
        v = static_cast<double>(value.int64_value());
      } else if (value.kind() != Value::Kind::kFloat64) {
        return BadValue(column, text);
      }
      char buf[sizeof(double)];
      std::memcpy(buf, &v, sizeof(v));
      out->append(buf, sizeof(buf));
      return absl::OkStatus();
    }
    case ColumnType::kBool:
      if (value.kind() != Value::Kind::kBool) return BadValue(column, text);
      out->push_back(value.bool_value() ? 1 : 0);
      return absl::OkStatus();
    case ColumnType::kString:
    case ColumnType::kBytes:
    case ColumnType::kJson:
    case ColumnType::kGeography:
      AppendAvroBytes(out, text);
      return absl::OkStatus();
    case ColumnType::kNumeric:
    case ColumnType::kBignumeric:
      return AppendDecimal(out, column, text);
    case ColumnType::kDate: {
      int64_t days = 0;
      if (!ParseDays(text, &days)) return BadValue(column, text);
      if (logical) {
        AppendAvroLong(out, days);
      } else {
        AppendAvroBytes(out, text);
      }
      return absl::OkStatus();
    }
    case ColumnType::kTime: {
      int64_t micros = 0;
      if (!ParseClockMicros(text, &micros)) return BadValue(column, text);
      if (logical) {
        AppendAvroLong(out, micros);
      } else {
        AppendAvroBytes(out, FormatClock(micros));
      }
      return absl::OkStatus();
    }
    case ColumnType::kDatetime:
    case ColumnType::kTimestamp: {
      int64_t micros = 0;
      if (!ParseDateTimeMicros(text, &micros)) return BadValue(column, text);
      if (column.type == ColumnType::kDatetime) {
        AppendAvroBytes(out, FormatDateTime(text, micros));
      } else if (logical) {
        AppendAvroLong(out, micros);
      } else {
        AppendAvroBytes(out, absl::StrCat(FormatDateTime(text, micros), "Z"));
      }
      return absl::OkStatus();
    }
    case ColumnType::kStruct: {
      const std::vector<Value>& fields = value.struct_value();
      if (value.kind() != Value::Kind::kStruct ||
          fields.size() != column.fields.size()) {
        return BadValue(column, text);
      }
      for (size_t i = 0; i < fields.size(); ++i) {
        absl::Status s = AppendField(out, column.fields[i], fields[i], logical);
        if (!s.ok()) return s;
      }
      return absl::OkStatus();
    }
    case ColumnType::kArray:
    case ColumnType::kUnknown:
      break;
  }
  return BadValue(column, text);
}

absl::Status AppendField(std::string* out,
                         const ColumnSchema& column,
                         const Value& value,
                         bool logical) {
  if (column.mode == ColumnMode::kRepeated) {
    if (value.is_null()) {
      AppendAvroLong(out, 0);
      return absl::OkStatus();
    }
    if (value.kind() != Value::Kind::kArray) {
      return BadValue(column, value.string_value());
    }
    const std::vector<Value>& elements = value.array_value();
    if (!elements.empty()) {
      AppendAvroLong(out, static_cast<int64_t>(elements.size()));
      for (const Value& element : elements) {
        if (element.is_null()) {
          return absl::InvalidArgumentError(
              absl::StrCat("EXPORT DATA: Array cannot have a null element; "
                           "error in writing field ",
                           column.name));
        }
        absl::Status s = AppendBase(out, column, element, logical);
        if (!s.ok()) return s;
      }
    }
    AppendAvroLong(out, 0);
    return absl::OkStatus();
  }
  if (value.is_null()) {
    if (column.mode == ColumnMode::kRequired) {
      return absl::InvalidArgumentError(absl::StrCat(
          "EXPORT DATA: NULL value in REQUIRED field ", column.name));
    }
    AppendAvroLong(out, 0);
    return absl::OkStatus();
  }
  if (column.mode == ColumnMode::kNullable) AppendAvroLong(out, 1);
  return AppendBase(out, column, value, logical);
}

}  // namespace

absl::StatusOr<std::string> BuildAvroSchemaJson(
    const schema::TableSchema& columns, bool use_logical_types) {
  absl::StatusOr<std::string> fields =
      RecordFieldsJson(columns.columns, "root", use_logical_types);
  if (!fields.ok()) return fields.status();
  return absl::StrCat(
      R"({"type":"record","name":"Root","fields":)", *fields, "}");
}

absl::Status AppendAvroField(std::string* out,
                             const schema::ColumnSchema& column,
                             const storage::Value& value,
                             bool use_logical_types) {
  return AppendField(out, column, value, use_logical_types);
}

}  // namespace internal
}  // namespace control
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
