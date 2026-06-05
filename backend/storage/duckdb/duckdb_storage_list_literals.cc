#include <cstdlib>
#include <string>
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
    if ((*in)[0] == '\'') {
      if (in->size() >= 2 && (*in)[1] == '\'') {
        out.push_back('\'');
        in->remove_prefix(2);
        continue;
      }
      in->remove_prefix(1);
      return out;
    }
    out.push_back((*in)[0]);
    in->remove_prefix(1);
  }
  return absl::InvalidArgumentError(
      "ParseQuotedStringToken: unterminated quoted string");
}

absl::StatusOr<Value> ParseListElementToken(
    absl::string_view token, const schema::ColumnSchema& element) {
  token = TrimAsciiSpace(token);
  if (token.empty() || token == "NULL") {
    return Value::Null();
  }
  if (!token.empty() && token.front() == '\'') {
    auto parsed = ParseQuotedStringToken(&token);
    if (!parsed.ok()) return parsed.status();
    return Value::String(*parsed);
  }
  switch (element.type) {
    case schema::ColumnType::kBool:
      if (token == "true") return Value::Bool(true);
      if (token == "false") return Value::Bool(false);
      break;
    case schema::ColumnType::kInt64: {
      const std::string raw(token);
      char* end = nullptr;
      const long long v = std::strtoll(raw.c_str(), &end, 10);
      if (end == raw.c_str() + raw.size()) return Value::Int64(v);
      break;
    }
    case schema::ColumnType::kFloat64: {
      const std::string raw(token);
      char* end = nullptr;
      const double v = std::strtod(raw.c_str(), &end);
      if (end == raw.c_str() + raw.size()) return Value::Float64(v);
      break;
    }
    default:
      return Value::String(std::string(token));
  }
  return absl::InvalidArgumentError(
      absl::StrCat("ParseListElementToken: cannot parse `", token, "`"));
}

}  // namespace

absl::StatusOr<Value> ParseDuckDBListVarchar(
    absl::string_view text, const schema::ColumnSchema& element) {
  absl::string_view body = TrimAsciiSpace(text);
  if (body.empty() || body == "NULL") {
    return Value::Null();
  }
  if (body.size() < 2 || body.front() != '[' || body.back() != ']') {
    return absl::InvalidArgumentError(absl::StrCat(
        "ParseDuckDBListVarchar: expected `[...]` got `", body, "`"));
  }
  body = TrimAsciiSpace(body.substr(1, body.size() - 2));
  std::vector<Value> elements;
  if (body.empty()) {
    return Value::Array(std::move(elements));
  }
  while (!body.empty()) {
    body = TrimAsciiSpace(body);
    if (body.empty()) break;
    if (body.front() == '\'') {
      auto parsed = ParseQuotedStringToken(&body);
      if (!parsed.ok()) return parsed.status();
      elements.push_back(Value::String(*parsed));
    } else {
      const size_t comma = body.find(',');
      const absl::string_view token = TrimAsciiSpace(
          comma == absl::string_view::npos ? body : body.substr(0, comma));
      body = comma == absl::string_view::npos ? absl::string_view{}
                                              : body.substr(comma + 1);
      auto value_or = ParseListElementToken(token, element);
      if (!value_or.ok()) return value_or.status();
      elements.push_back(std::move(*value_or));
    }
    body = TrimAsciiSpace(body);
    if (!body.empty() && body.front() == ',') {
      body.remove_prefix(1);
    }
  }
  return Value::Array(std::move(elements));
}

}  // namespace internal
}  // namespace duckdb
}  // namespace storage
}  // namespace backend
}  // namespace bigquery_emulator
