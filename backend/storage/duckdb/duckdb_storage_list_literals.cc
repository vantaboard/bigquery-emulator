#include <cstdint>
#include <cstdlib>
#include <string>
#include <utility>
#include <vector>

#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/string_view.h"
#include "backend/schema/schema.h"
#include "backend/storage/duckdb/duckdb_storage_internal.h"
#include "backend/storage/storage.h"

namespace bigquery_emulator {
namespace backend {
namespace storage {
namespace duckdb {
namespace internal {

namespace {

absl::string_view TrimAsciiSpace(absl::string_view s) {
  while (!s.empty() && (s.front() == ' ' || s.front() == '\t')) {
    s.remove_prefix(1);
  }
  while (!s.empty() && (s.back() == ' ' || s.back() == '\t')) {
    s.remove_suffix(1);
  }
  return s;
}

absl::StatusOr<std::string> ParseQuotedStringToken(absl::string_view* in) {
  if (in->empty() || (*in)[0] != '\'') {
    return absl::InvalidArgumentError(
        "ParseQuotedStringToken: expected leading quote");
  }
  in->remove_prefix(1);
  std::string out;
  while (!in->empty()) {
    if ((*in)[0] == '\\' && in->size() >= 2) {
      out.push_back((*in)[1]);
      in->remove_prefix(2);
      continue;
    }
    if ((*in)[0] == '\'') {
      in->remove_prefix(1);
      return out;
    }
    out.push_back((*in)[0]);
    in->remove_prefix(1);
  }
  return absl::InvalidArgumentError(
      "ParseQuotedStringToken: unterminated quoted string");
}

bool StartsWithBareNull(absl::string_view in) {
  if (in.substr(0, 4) != "NULL") return false;
  if (in.size() == 4) return true;
  const char next = in[4];
  return next == ',' || next == ']' || next == '}' || next == ' ' ||
         next == '\t';
}

// Takes the text of one unquoted value: up to the next `,`, `]` or `}`
// outside brackets, braces and quotes.
absl::string_view TakeBareToken(absl::string_view* in) {
  int depth = 0;
  bool quoted = false;
  size_t i = 0;
  for (; i < in->size(); ++i) {
    const char c = (*in)[i];
    if (quoted) {
      if (c == '\\') {
        ++i;
      } else if (c == '\'') {
        quoted = false;
      }
      continue;
    }
    if (c == '\'') {
      quoted = true;
    } else if (c == '[' || c == '{') {
      ++depth;
    } else if (c == ']' || c == '}' || c == ',') {
      if (depth == 0) break;
      if (c != ',') --depth;
    }
  }
  const absl::string_view token = TrimAsciiSpace(in->substr(0, i));
  in->remove_prefix(i);
  return token;
}

Value ScalarFromBareToken(absl::string_view token,
                          const schema::ColumnSchema& column) {
  switch (column.type) {
    case schema::ColumnType::kBool:
      if (token == "true" || token == "TRUE") return Value::Bool(true);
      if (token == "false" || token == "FALSE") return Value::Bool(false);
      break;
    case schema::ColumnType::kInt64: {
      const std::string raw(token);
      char* end = nullptr;
      const int64_t v = std::strtoll(raw.c_str(), &end, 10);
      if (!raw.empty() && end == raw.c_str() + raw.size()) {
        return Value::Int64(v);
      }
      break;
    }
    case schema::ColumnType::kFloat64: {
      const std::string raw(token);
      char* end = nullptr;
      const double v = std::strtod(raw.c_str(), &end);
      if (!raw.empty() && end == raw.c_str() + raw.size()) {
        return Value::Float64(v);
      }
      break;
    }
    default:
      break;
  }
  return Value::String(std::string(token));
}

bool IsList(const schema::ColumnSchema& column) {
  return column.mode == schema::ColumnMode::kRepeated ||
         column.type == schema::ColumnType::kArray;
}

absl::StatusOr<Value> ParseValue(absl::string_view* in,
                                 const schema::ColumnSchema& column);

absl::StatusOr<Value> ParseList(absl::string_view* in,
                                const schema::ColumnSchema& element) {
  if (in->empty() || in->front() != '[') {
    return absl::InvalidArgumentError(
        absl::StrCat("ParseDuckDBListVarchar: expected `[` at `", *in, "`"));
  }
  in->remove_prefix(1);
  std::vector<Value> elements;
  while (true) {
    *in = TrimAsciiSpace(*in);
    if (in->empty()) {
      return absl::InvalidArgumentError(
          "ParseDuckDBListVarchar: unterminated list");
    }
    if (in->front() == ']') {
      in->remove_prefix(1);
      return Value::Array(std::move(elements));
    }
    auto value = ParseValue(in, element);
    if (!value.ok()) return value.status();
    elements.push_back(*std::move(value));
    *in = TrimAsciiSpace(*in);
    if (!in->empty() && in->front() == ',') in->remove_prefix(1);
  }
}

absl::StatusOr<Value> ParseStruct(absl::string_view* in,
                                  const schema::ColumnSchema& column) {
  if (in->empty() || in->front() != '{') {
    return absl::InvalidArgumentError(
        absl::StrCat("ParseDuckDBStructVarchar: expected `{` at `", *in, "`"));
  }
  in->remove_prefix(1);
  std::vector<Value> fields(column.fields.size(), Value::Null());
  while (true) {
    *in = TrimAsciiSpace(*in);
    if (in->empty()) {
      return absl::InvalidArgumentError(
          "ParseDuckDBStructVarchar: unterminated struct");
    }
    if (in->front() == '}') {
      in->remove_prefix(1);
      return Value::Struct(std::move(fields));
    }
    auto key = ParseQuotedStringToken(in);
    if (!key.ok()) return key.status();
    *in = TrimAsciiSpace(*in);
    if (in->empty() || in->front() != ':') {
      return absl::InvalidArgumentError(absl::StrCat(
          "ParseDuckDBStructVarchar: expected `:` after field `", *key, "`"));
    }
    in->remove_prefix(1);
    *in = TrimAsciiSpace(*in);
    size_t index = 0;
    while (index < column.fields.size() && column.fields[index].name != *key) {
      ++index;
    }
    if (index == column.fields.size()) {
      TakeBareToken(in);
    } else {
      auto value = ParseValue(in, column.fields[index]);
      if (!value.ok()) return value.status();
      fields[index] = *std::move(value);
    }
    *in = TrimAsciiSpace(*in);
    if (!in->empty() && in->front() == ',') in->remove_prefix(1);
  }
}

// Parses the value at the front of `in` as `column` and consumes it.
absl::StatusOr<Value> ParseValue(absl::string_view* in,
                                 const schema::ColumnSchema& column) {
  *in = TrimAsciiSpace(*in);
  if (StartsWithBareNull(*in)) {
    in->remove_prefix(4);
    return Value::Null();
  }
  if (IsList(column) && !in->empty() && in->front() == '[') {
    return ParseList(in, ListElementSchema(column));
  }
  if (column.type == schema::ColumnType::kStruct && !in->empty() &&
      in->front() == '{') {
    return ParseStruct(in, column);
  }
  if (!in->empty() && in->front() == '\'') {
    auto parsed = ParseQuotedStringToken(in);
    if (!parsed.ok()) return parsed.status();
    return Value::String(*std::move(parsed));
  }
  return ScalarFromBareToken(TakeBareToken(in), column);
}

absl::StatusOr<Value> ParseWhole(absl::string_view text,
                                 const schema::ColumnSchema& column,
                                 bool list) {
  absl::string_view body = TrimAsciiSpace(text);
  if (body.empty() || body == "NULL") return Value::Null();
  auto value = list ? ParseList(&body, column) : ParseValue(&body, column);
  if (!value.ok()) return value.status();
  if (!TrimAsciiSpace(body).empty()) {
    return absl::InvalidArgumentError(
        absl::StrCat("trailing text `", body, "` after a nested value"));
  }
  return value;
}

}  // namespace

schema::ColumnSchema ListElementSchema(const schema::ColumnSchema& column) {
  if (column.mode == schema::ColumnMode::kRepeated) {
    schema::ColumnSchema element = column;
    element.mode = schema::ColumnMode::kNullable;
    return element;
  }
  if (!column.fields.empty()) return column.fields.front();
  schema::ColumnSchema element;
  element.name = column.name;
  element.type = schema::ColumnType::kString;
  return element;
}

absl::StatusOr<Value> ParseDuckDBListVarchar(
    absl::string_view text, const schema::ColumnSchema& element) {
  return ParseWhole(text, element, /*list=*/true);
}

absl::StatusOr<Value> ParseDuckDBStructVarchar(
    absl::string_view text, const schema::ColumnSchema& column) {
  return ParseWhole(text, column, /*list=*/false);
}

}  // namespace internal
}  // namespace duckdb
}  // namespace storage
}  // namespace backend
}  // namespace bigquery_emulator
