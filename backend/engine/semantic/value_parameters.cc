#include "backend/engine/semantic/value.h"

#include <cctype>
#include <cstdint>
#include <string>
#include <vector>

#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "absl/strings/ascii.h"
#include "absl/strings/escaping.h"
#include "absl/strings/match.h"
#include "absl/strings/numbers.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/str_split.h"
#include "absl/strings/string_view.h"
#include "absl/strings/strip.h"
#include "absl/time/time.h"
#include "absl/types/span.h"
#include "backend/engine/semantic/error.h"
#include "googlesql/public/numeric_value.h"
#include "googlesql/public/type.h"
#include "googlesql/public/types/struct_type.h"
#include "googlesql/public/types/type_factory.h"
#include "googlesql/public/value.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace semantic {

namespace {

absl::string_view StripJsonQuotes(absl::string_view body) {
  if (body.size() >= 2 && body.front() == '"' && body.back() == '"') {
    return body.substr(1, body.size() - 2);
  }
  return body;
}

struct StructFieldSpec {
  std::string name;
  std::string type_kind;
};

std::vector<StructFieldSpec> ParseStructFieldSpecs(
    absl::string_view type_json) {
  std::vector<StructFieldSpec> out;
  for (absl::string_view part : absl::StrSplit(type_json, ',')) {
    part = absl::StripAsciiWhitespace(part);
    if (part.empty()) continue;
    const size_t colon = part.find(':');
    if (colon == absl::string_view::npos) continue;
    out.push_back(
        {std::string(part.substr(0, colon)),
         std::string(absl::StripAsciiWhitespace(part.substr(colon + 1)))});
  }
  return out;
}

absl::StatusOr<const ::googlesql::Type*> PrimitiveTypeForKind(
    ::googlesql::TypeKind kind) {
  switch (kind) {
    case ::googlesql::TYPE_BOOL:
      return ::googlesql::types::BoolType();
    case ::googlesql::TYPE_INT64:
      return ::googlesql::types::Int64Type();
    case ::googlesql::TYPE_DOUBLE:
      return ::googlesql::types::DoubleType();
    case ::googlesql::TYPE_STRING:
      return ::googlesql::types::StringType();
    case ::googlesql::TYPE_BYTES:
      return ::googlesql::types::BytesType();
    case ::googlesql::TYPE_DATE:
      return ::googlesql::types::DateType();
    case ::googlesql::TYPE_TIME:
      return ::googlesql::types::TimeType();
    case ::googlesql::TYPE_DATETIME:
      return ::googlesql::types::DatetimeType();
    case ::googlesql::TYPE_TIMESTAMP:
      return ::googlesql::types::TimestampType();
    case ::googlesql::TYPE_NUMERIC:
      return ::googlesql::types::NumericType();
    case ::googlesql::TYPE_BIGNUMERIC:
      return ::googlesql::types::BigNumericType();
    case ::googlesql::TYPE_JSON:
      return ::googlesql::types::JsonType();
    default:
      return absl::InvalidArgumentError(
          absl::StrCat("semantic: unsupported struct field type kind '",
                       ::googlesql::TypeKind_Name(kind),
                       "'"));
  }
}

::googlesql::TypeFactory& ParameterTypeFactory() {
  static ::googlesql::TypeFactory* factory = new ::googlesql::TypeFactory();
  return *factory;
}

absl::StatusOr<const ::googlesql::StructType*> StructTypeFromFieldSpecs(
    absl::Span<const StructFieldSpec> specs) {
  if (specs.empty()) {
    return absl::InvalidArgumentError(
        "semantic: STRUCT parameter missing type_json field specs");
  }
  std::vector<::googlesql::StructType::StructField> fields;
  fields.reserve(specs.size());
  for (const StructFieldSpec& spec : specs) {
    auto type_or = PrimitiveTypeForKind(ParseTypeKindName(spec.type_kind));
    if (!type_or.ok()) return type_or.status();
    fields.emplace_back(spec.name, *type_or);
  }
  const ::googlesql::StructType* struct_type = nullptr;
  absl::Status s = ParameterTypeFactory().MakeStructType(fields, &struct_type);
  if (!s.ok()) return s;
  return struct_type;
}

absl::StatusOr<std::vector<std::string>> ParseJsonArrayElements(
    absl::string_view json) {
  json = absl::StripAsciiWhitespace(json);
  if (json.empty() || json.front() != '[') {
    return absl::InvalidArgumentError(
        "semantic: STRUCT parameter value_json must be a JSON array");
  }
  if (json.size() == 2 && json[1] == ']') {
    return std::vector<std::string>{};
  }
  std::vector<std::string> out;
  size_t i = 1;
  while (i < json.size()) {
    while (i < json.size() && absl::ascii_isspace(json[i])) ++i;
    if (i >= json.size()) break;
    if (json[i] == ']') break;
    const size_t start = i;
    if (json[i] == '"') {
      ++i;
      while (i < json.size()) {
        if (json[i] == '\\') {
          i += 2;
          continue;
        }
        if (json[i] == '"') {
          ++i;
          break;
        }
        ++i;
      }
    } else {
      while (i < json.size() && json[i] != ',') {
        if (json[i] == ']') break;
        ++i;
      }
    }
    out.push_back(
        std::string(absl::StripAsciiWhitespace(json.substr(start, i - start))));
    while (i < json.size() && absl::ascii_isspace(json[i])) ++i;
    if (i < json.size() && json[i] == ',') ++i;
  }
  return out;
}

absl::StatusOr<Value> ParseTimestampParameter(absl::string_view body) {
  const std::string text(body);
  absl::Time t;
  std::string err;
  if (absl::ParseTime(absl::RFC3339_full, text, &t, &err)) {
    return Value::Timestamp(t);
  }
  if (absl::ParseTime("%Y-%m-%d %H:%M:%E*S%Ez", text, &t, &err)) {
    return Value::Timestamp(t);
  }
  if (absl::ParseTime("%Y-%m-%d %H:%M:%E*S", text, &t, &err)) {
    return Value::Timestamp(t);
  }
  return absl::InvalidArgumentError(
      absl::StrCat("semantic: invalid TIMESTAMP parameter value '", body, "'"));
}

}  // namespace

bool IsSyntheticPositionalParameterName(absl::string_view name) {
  return SyntheticPositionalParameterIndex(name) >= 0;
}

int SyntheticPositionalParameterIndex(absl::string_view name) {
  if (name.empty() || !absl::StartsWith(name, "p")) return -1;
  name.remove_prefix(1);
  if (name.empty()) return -1;
  int idx = -1;
  if (!absl::SimpleAtoi(name, &idx) || idx < 0) return -1;
  return idx;
}

bool IsPositionalParameterName(absl::string_view name) {
  return name.empty() || IsSyntheticPositionalParameterName(name);
}

absl::StatusOr<Value> ParseParameterValue(absl::string_view value_json,
                                          absl::string_view type_kind_name,
                                          absl::string_view type_json) {
  ::googlesql::TypeKind kind = ParseTypeKindName(type_kind_name);
  if (kind == ::googlesql::TYPE_UNKNOWN) {
    return absl::InvalidArgumentError(absl::StrCat(
        "semantic: unknown parameter type kind '", type_kind_name, "'"));
  }
  absl::string_view trimmed = absl::StripAsciiWhitespace(value_json);
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
    case ::googlesql::TYPE_BYTES: {
      std::string decoded;
      if (!absl::Base64Unescape(body, &decoded)) {
        return absl::InvalidArgumentError(absl::StrCat(
            "semantic: invalid BYTES parameter value '", value_json, "'"));
      }
      return Value::Bytes(decoded);
    }
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
    case ::googlesql::TYPE_JSON:
    case ::googlesql::TYPE_INTERVAL:
    case ::googlesql::TYPE_UUID:
      return MakeSemanticError(SemanticErrorReason::kNotImplemented,
                               absl::StrCat("semantic: parameter type '",
                                            type_kind_name,
                                            "' is not yet supported; see "
                                            "docs/ENGINE_POLICY.md"));
    case ::googlesql::TYPE_TIMESTAMP:
      return ParseTimestampParameter(body);
    case ::googlesql::TYPE_ARRAY:
      return MakeSemanticError(
          SemanticErrorReason::kNotImplemented,
          absl::StrCat("semantic: ARRAY parameters are owned by "
                       "docs/ENGINE_POLICY.md"));
    case ::googlesql::TYPE_STRUCT: {
      auto specs = ParseStructFieldSpecs(type_json);
      auto struct_type_or = StructTypeFromFieldSpecs(specs);
      if (!struct_type_or.ok()) return struct_type_or.status();
      auto elems_or = ParseJsonArrayElements(trimmed);
      if (!elems_or.ok()) return elems_or.status();
      if (elems_or->size() != specs.size()) {
        return absl::InvalidArgumentError(absl::StrCat(
            "semantic: STRUCT parameter field count mismatch: type_json has ",
            specs.size(),
            " fields but value_json has ",
            elems_or->size(),
            " elements"));
      }
      std::vector<Value> fields;
      fields.reserve(specs.size());
      for (size_t i = 0; i < specs.size(); ++i) {
        auto field_or =
            ParseParameterValue((*elems_or)[i], specs[i].type_kind, {});
        if (!field_or.ok()) return field_or.status();
        fields.push_back(*std::move(field_or));
      }
      return Value::Struct(*struct_type_or, std::move(fields));
    }
    default:
      return absl::InvalidArgumentError(absl::StrCat(
          "semantic: unsupported parameter type kind '", type_kind_name, "'"));
  }
}

}  // namespace semantic
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
