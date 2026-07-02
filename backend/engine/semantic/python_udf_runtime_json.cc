#include "backend/engine/semantic/python_udf_runtime_json.h"

#include <cmath>
#include <limits>
#include <string>
#include <vector>

#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "absl/strings/ascii.h"
#include "absl/strings/escaping.h"
#include "absl/strings/numbers.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/str_join.h"
#include "absl/strings/string_view.h"
#include "backend/engine/semantic/error.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace semantic {
namespace {

absl::Status PythonUdfError(absl::string_view message) {
  return MakeSemanticError(
      SemanticErrorReason::kInvalidArgument,
      absl::StrCat("User-defined function error: ", message));
}

}  // namespace

std::string JsonEscape(absl::string_view s) {
  std::string out;
  out.reserve(s.size() + 8);
  out.push_back('"');
  for (char ch : s) {
    switch (ch) {
      case '"':
        out.append("\\\"");
        break;
      case '\\':
        out.append("\\\\");
        break;
      case '\b':
        out.append("\\b");
        break;
      case '\f':
        out.append("\\f");
        break;
      case '\n':
        out.append("\\n");
        break;
      case '\r':
        out.append("\\r");
        break;
      case '\t':
        out.append("\\t");
        break;
      default:
        if (static_cast<unsigned char>(ch) < 0x20) {
          absl::StrAppend(&out, absl::CEscape(absl::string_view(&ch, 1)));
        } else {
          out.push_back(ch);
        }
        break;
    }
  }
  out.push_back('"');
  return out;
}

namespace {

std::string JsonValue(const Value& value, ::googlesql::TypeKind type_kind) {
  if (value.is_null()) return "null";
  switch (type_kind) {
    case ::googlesql::TYPE_BOOL:
      return value.bool_value() ? "true" : "false";
    case ::googlesql::TYPE_INT64:
      return std::to_string(value.int64_value());
    case ::googlesql::TYPE_DOUBLE: {
      const double d = value.double_value();
      if (!std::isfinite(d)) return "null";
      return absl::StrCat(d);
    }
    case ::googlesql::TYPE_STRING:
      return JsonEscape(value.string_value());
    default:
      return "null";
  }
}

}  // namespace

absl::StatusOr<std::string> BuildPythonUdfRequestJson(
    absl::string_view fn_name,
    const catalog::PythonUdfDefinition& definition,
    const std::vector<Value>& arg_values) {
  std::string json = "{";
  absl::StrAppend(&json, "\"body\":", JsonEscape(definition.python_body), ",");
  absl::StrAppend(&json, "\"fn_name\":", JsonEscape(fn_name), ",");
  absl::StrAppend(
      &json, "\"entry_point\":", JsonEscape(definition.entry_point), ",");
  absl::StrAppend(&json,
                  "\"arg_names\":[",
                  absl::StrJoin(definition.arg_names,
                                ",",
                                [](std::string* out, const std::string& name) {
                                  absl::StrAppend(out, JsonEscape(name));
                                }),
                  "],\"args\":{");
  for (size_t i = 0; i < arg_values.size(); ++i) {
    if (i > 0) json.push_back(',');
    const ::googlesql::TypeKind type_kind = i < definition.arg_type_kinds.size()
                                                ? definition.arg_type_kinds[i]
                                                : ::googlesql::TYPE_UNKNOWN;
    absl::StrAppend(&json,
                    JsonEscape(definition.arg_names[i]),
                    ":",
                    JsonValue(arg_values[i], type_kind));
  }
  json.append("}}");
  return json;
}

absl::StatusOr<std::string> ExtractJsonStringField(absl::string_view json,
                                                   absl::string_view field) {
  const std::string needle = absl::StrCat("\"", field, "\":");
  const size_t pos = json.find(needle);
  if (pos == absl::string_view::npos) {
    return absl::InternalError(
        absl::StrCat("python_udf_runtime: response missing field ", field));
  }
  absl::string_view rest = json.substr(pos + needle.size());
  while (!rest.empty() && absl::ascii_isspace(rest.front())) {
    rest.remove_prefix(1);
  }
  if (rest.empty() || rest.front() != '"') {
    return absl::InternalError(
        "python_udf_runtime: expected string field in response");
  }
  rest.remove_prefix(1);
  std::string out;
  while (!rest.empty()) {
    const char ch = rest.front();
    rest.remove_prefix(1);
    if (ch == '"') return out;
    if (ch == '\\') {
      if (rest.empty()) break;
      const char esc = rest.front();
      rest.remove_prefix(1);
      switch (esc) {
        case 'n':
          out.push_back('\n');
          break;
        case 'r':
          out.push_back('\r');
          break;
        case 't':
          out.push_back('\t');
          break;
        case '"':
          out.push_back('"');
          break;
        case '\\':
          out.push_back('\\');
          break;
        default:
          out.push_back(esc);
          break;
      }
      continue;
    }
    out.push_back(ch);
  }
  return absl::InternalError(
      "python_udf_runtime: unterminated string in response");
}

absl::StatusOr<bool> ExtractJsonBoolField(absl::string_view json,
                                          absl::string_view field) {
  const std::string needle = absl::StrCat("\"", field, "\":");
  const size_t pos = json.find(needle);
  if (pos == absl::string_view::npos) {
    return absl::InternalError(
        absl::StrCat("python_udf_runtime: response missing field ", field));
  }
  absl::string_view rest = json.substr(pos + needle.size());
  while (!rest.empty() && absl::ascii_isspace(rest.front())) {
    rest.remove_prefix(1);
  }
  if (absl::StartsWith(rest, "true")) return true;
  if (absl::StartsWith(rest, "false")) return false;
  return absl::InternalError("python_udf_runtime: expected bool field");
}

absl::StatusOr<Value> PopPythonValueToGooglesql(
    absl::string_view raw_result, const ::googlesql::Type* return_type) {
  if (return_type == nullptr) {
    return absl::InternalError("python_udf_runtime: missing return type");
  }
  absl::string_view trimmed = raw_result;
  while (!trimmed.empty() && absl::ascii_isspace(trimmed.front())) {
    trimmed.remove_prefix(1);
  }
  if (trimmed == "null") {
    return Value::Null(return_type);
  }
  switch (return_type->kind()) {
    case ::googlesql::TYPE_BOOL: {
      if (trimmed == "true") return Value::Bool(true);
      if (trimmed == "false") return Value::Bool(false);
      return PythonUdfError("Python UDF must return BOOL");
    }
    case ::googlesql::TYPE_INT64: {
      int64_t out = 0;
      if (!absl::SimpleAtoi(trimmed, &out)) {
        return PythonUdfError("Python UDF INT64 return is not a valid integer");
      }
      return Value::Int64(out);
    }
    case ::googlesql::TYPE_DOUBLE: {
      double out = 0.0;
      if (!absl::SimpleAtod(trimmed, &out) || !std::isfinite(out)) {
        return PythonUdfError("Python UDF must return FLOAT64");
      }
      return Value::Double(out);
    }
    case ::googlesql::TYPE_STRING: {
      absl::string_view text = trimmed;
      if (text.empty() || text.front() != '"') {
        return PythonUdfError("Python UDF must return STRING");
      }
      auto parsed = ExtractJsonStringField(
          absl::StrCat("{\"result\":", text, "}"), "result");
      if (!parsed.ok()) return parsed.status();
      return Value::String(*parsed);
    }
    default:
      return MakeSemanticError(
          SemanticErrorReason::kNotImplemented,
          absl::StrCat("Python UDF return type ",
                       ::googlesql::TypeKind_Name(return_type->kind()),
                       " is not supported"));
  }
}


}  // namespace semantic
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
