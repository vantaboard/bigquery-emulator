#include "backend/engine/semantic/external_query_fixture.h"

#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

#include "absl/status/status.h"
#include "absl/strings/ascii.h"
#include "absl/strings/match.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/strip.h"
#include "backend/engine/semantic/value.h"
#include "googlesql/public/json_value.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace semantic {

namespace {

namespace fs = std::filesystem;
using ::googlesql::JSONValue;
using ::googlesql::JSONValueConstRef;

std::string EmulatorDataDir() {
  const char* env = std::getenv("BIGQUERY_EMULATOR_DATA_DIR");
  return env != nullptr ? std::string(env) : std::string();
}

std::string ConnectionID(absl::string_view name) {
  std::string out = std::string(absl::StripAsciiWhitespace(name));
  if (const size_t slash = out.rfind('/'); slash != std::string::npos) {
    return out.substr(slash + 1);
  }
  if (const size_t dot = out.rfind('.'); dot != std::string::npos) {
    return out.substr(dot + 1);
  }
  return out;
}

absl::StatusOr<std::string> ReadFileToString(const fs::path& path) {
  std::ifstream in(path, std::ios::binary);
  if (!in) {
    return absl::NotFoundError(
        absl::StrCat("EXTERNAL_QUERY fixture file not found: ", path.string()));
  }
  std::ostringstream ss;
  ss << in.rdbuf();
  return ss.str();
}

absl::StatusOr<Value> JsonCellToValue(JSONValueConstRef cell,
                                      absl::string_view type_name) {
  if (cell.IsNull()) {
    return ParseParameterValue("null", type_name);
  }
  if (cell.IsBoolean()) {
    return Value::Bool(cell.GetBoolean());
  }
  if (cell.IsInt64()) {
    return Value::Int64(cell.GetInt64());
  }
  if (cell.IsNumber()) {
    return Value::Double(cell.GetDouble());
  }
  if (cell.IsString()) {
    const std::string text = cell.GetString();
    auto parsed = ParseParameterValue(text, type_name);
    if (parsed.ok()) {
      return *parsed;
    }
    return Value::String(text);
  }
  return absl::InvalidArgumentError(
      "EXTERNAL_QUERY fixture: unsupported JSON cell type");
}

absl::StatusOr<std::vector<ExternalQueryFixtureColumn>> ParseSchema(
    JSONValueConstRef schema) {
  if (!schema.IsArray()) {
    return absl::InvalidArgumentError(
        "EXTERNAL_QUERY fixture: schema must be an array");
  }
  std::vector<ExternalQueryFixtureColumn> out;
  for (JSONValueConstRef col : schema.GetArrayElements()) {
    if (!col.IsObject()) {
      return absl::InvalidArgumentError(
          "EXTERNAL_QUERY fixture: schema entry must be an object");
    }
    auto name = col.GetMemberIfExists("name");
    auto type = col.GetMemberIfExists("type");
    if (!name.has_value() || !type.has_value() || !type->IsString()) {
      return absl::InvalidArgumentError(
          "EXTERNAL_QUERY fixture: schema entry requires name and type");
    }
    out.push_back(
        ExternalQueryFixtureColumn{std::string(name->GetString()),
                                   std::string(type->GetString())});
  }
  return out;
}

absl::StatusOr<ExternalQueryFixtureResult> ParseResultFile(
    const fs::path& path, ::googlesql::TypeFactory* type_factory) {
  absl::StatusOr<std::string> raw = ReadFileToString(path);
  if (!raw.ok()) {
    return raw.status();
  }
  absl::StatusOr<JSONValue> doc = JSONValue::ParseJSONString(*raw);
  if (!doc.ok()) {
    return absl::InvalidArgumentError(absl::StrCat(
        "EXTERNAL_QUERY fixture: invalid JSON in ", path.string(), ": ",
        doc.status().message()));
  }
  JSONValueConstRef root = doc->GetConstRef();
  if (!root.IsObject()) {
    return absl::InvalidArgumentError(absl::StrCat(
        "EXTERNAL_QUERY fixture: root must be object in ", path.string()));
  }
  auto schema_ref = root.GetMemberIfExists("schema");
  auto rows_ref = root.GetMemberIfExists("rows");
  if (!schema_ref.has_value() || !rows_ref.has_value()) {
    return absl::InvalidArgumentError(absl::StrCat(
        "EXTERNAL_QUERY fixture: schema and rows required in ", path.string()));
  }
  absl::StatusOr<std::vector<ExternalQueryFixtureColumn>> schema =
      ParseSchema(*schema_ref);
  if (!schema.ok()) {
    return schema.status();
  }

  ExternalQueryFixtureResult result;
  result.schema = *schema;
  result.column_ids.reserve(result.schema.size());
  int next_id = 1;
  for (size_t i = 0; i < result.schema.size(); ++i) {
    (void)i;
    result.column_ids.push_back(next_id++);
  }

  if (!rows_ref->IsArray()) {
    return absl::InvalidArgumentError(absl::StrCat(
        "EXTERNAL_QUERY fixture: rows must be array in ", path.string()));
  }
  for (JSONValueConstRef row_json : rows_ref->GetArrayElements()) {
    if (!row_json.IsObject()) {
      return absl::InvalidArgumentError(
          "EXTERNAL_QUERY fixture: each row must be an object");
    }
    ColumnBindings row;
    for (size_t i = 0; i < result.schema.size(); ++i) {
      const ExternalQueryFixtureColumn& col = result.schema[i];
      auto cell = row_json.GetMemberIfExists(col.name);
      if (!cell.has_value()) {
        return absl::InvalidArgumentError(absl::StrCat(
            "EXTERNAL_QUERY fixture: row missing column '", col.name, "'"));
      }
      absl::StatusOr<Value> value = JsonCellToValue(*cell, col.type);
      if (!value.ok()) {
        return value.status();
      }
      row.emplace(result.column_ids[i], *std::move(value));
    }
    result.rows.push_back(std::move(row));
  }
  (void)type_factory;
  return result;
}

absl::StatusOr<fs::path> ResolveResultPath(const fs::path& conn_dir,
                                           absl::string_view result_file) {
  fs::path rel(result_file);
  if (rel.is_absolute() || rel.has_parent_path()) {
    return absl::InvalidArgumentError(
        "EXTERNAL_QUERY fixture: result path must be a bare filename");
  }
  fs::path candidate = conn_dir / rel;
  if (fs::exists(candidate)) {
    return candidate;
  }
  const fs::path json_alt =
      conn_dir / absl::StrCat(rel.stem().string(), ".json");
  if (fs::exists(json_alt)) {
    return json_alt;
  }
  return absl::NotFoundError(absl::StrCat(
      "EXTERNAL_QUERY fixture result file not found: ", candidate.string(),
      " (add snapshot under ", conn_dir.string(), ")"));
}

absl::StatusOr<fs::path> FindManifest(const fs::path& conn_dir) {
  const fs::path json_manifest = conn_dir / "queries.json";
  if (fs::exists(json_manifest)) {
    return json_manifest;
  }
  const fs::path yaml_manifest = conn_dir / "queries.yaml";
  if (fs::exists(yaml_manifest)) {
    return yaml_manifest;
  }
  return absl::NotFoundError(absl::StrCat(
      "EXTERNAL_QUERY fixture manifest not found under ", conn_dir.string(),
      " (expected queries.json or queries.yaml)"));
}

absl::StatusOr<std::string> LookupResultFile(JSONValueConstRef manifest_root,
                                             absl::string_view query_sql) {
  auto queries = manifest_root.GetMemberIfExists("queries");
  if (!queries.has_value() || !queries->IsArray()) {
    return absl::InvalidArgumentError(
        "EXTERNAL_QUERY fixture manifest: queries array required");
  }
  const std::string normalized_query = std::string(query_sql);
  for (JSONValueConstRef entry : queries->GetArrayElements()) {
    if (!entry.IsObject()) {
      continue;
    }
    auto result = entry.GetMemberIfExists("result");
    if (!result.has_value() || !result->IsString()) {
      continue;
    }
    if (auto q = entry.GetMemberIfExists("query"); q.has_value() &&
                 q->IsString() && q->GetString() == normalized_query) {
      return std::string(result->GetString());
    }
    if (auto alias = entry.GetMemberIfExists("alias"); alias.has_value() &&
                     alias->IsString() &&
                     alias->GetString() == normalized_query) {
      return std::string(result->GetString());
    }
  }
  return absl::NotFoundError(absl::StrCat(
      "EXTERNAL_QUERY fixture has no entry for query: ", query_sql));
}

absl::StatusOr<std::string> LookupResultFileFromYamlManifest(
    const fs::path& yaml_path, absl::string_view query_sql) {
  absl::StatusOr<std::string> raw = ReadFileToString(yaml_path);
  if (!raw.ok()) {
    return raw.status();
  }
  std::string current_result;
  bool in_queries = false;
  std::istringstream lines(*raw);
  std::string line;
  std::string pending_query;
  std::string pending_alias;
  while (std::getline(lines, line)) {
    const std::string trimmed = std::string(absl::StripAsciiWhitespace(line));
    if (trimmed.empty() || absl::StartsWith(trimmed, "#")) {
      continue;
    }
    if (trimmed == "queries:") {
      in_queries = true;
      continue;
    }
    if (!in_queries) {
      continue;
    }
    if (absl::StartsWith(trimmed, "- ")) {
      pending_query.clear();
      pending_alias.clear();
      current_result.clear();
      const std::string rest = trimmed.substr(2);
      if (absl::StartsWith(rest, "query:")) {
        pending_query = std::string(absl::StripAsciiWhitespace(rest.substr(6)));
        if (!pending_query.empty() && pending_query.front() == '"' &&
            pending_query.back() == '"') {
          pending_query = pending_query.substr(1, pending_query.size() - 2);
        }
      } else if (absl::StartsWith(rest, "alias:")) {
        pending_alias = std::string(absl::StripAsciiWhitespace(rest.substr(6)));
      } else if (absl::StartsWith(rest, "result:")) {
        current_result = std::string(absl::StripAsciiWhitespace(rest.substr(7)));
      }
      continue;
    }
    if (absl::StartsWith(trimmed, "query:")) {
      pending_query = std::string(absl::StripAsciiWhitespace(trimmed.substr(6)));
      if (!pending_query.empty() && pending_query.front() == '"' &&
          pending_query.back() == '"') {
        pending_query = pending_query.substr(1, pending_query.size() - 2);
      }
    } else if (absl::StartsWith(trimmed, "alias:")) {
      pending_alias = std::string(absl::StripAsciiWhitespace(trimmed.substr(6)));
    } else if (absl::StartsWith(trimmed, "result:")) {
      current_result = std::string(absl::StripAsciiWhitespace(trimmed.substr(7)));
      if (pending_query == std::string(query_sql) ||
          pending_alias == std::string(query_sql)) {
        if (current_result.empty()) {
          break;
        }
        return current_result;
      }
    }
  }
  return absl::NotFoundError(absl::StrCat(
      "EXTERNAL_QUERY fixture has no YAML entry for query: ", query_sql));
}

}  // namespace

absl::StatusOr<ExternalQueryFixtureResult> LoadExternalQueryFixture(
    absl::string_view connection_arg, absl::string_view query_sql,
    ::googlesql::TypeFactory* type_factory) {
  const std::string data_dir = EmulatorDataDir();
  if (data_dir.empty()) {
    return absl::FailedPreconditionError(
        "EXTERNAL_QUERY requires BIGQUERY_EMULATOR_DATA_DIR");
  }
  const std::string conn_id = ConnectionID(connection_arg);
  const fs::path conn_dir =
      fs::path(data_dir) / "external" / "connections" / conn_id;
  if (!fs::exists(conn_dir)) {
    return absl::NotFoundError(absl::StrCat(
        "EXTERNAL_QUERY connection fixture directory not found: ",
        conn_dir.string(),
        " (create $data_dir/external/connections/", conn_id, "/)"));
  }

  absl::StatusOr<fs::path> manifest_path = FindManifest(conn_dir);
  if (!manifest_path.ok()) {
    return manifest_path.status();
  }

  absl::StatusOr<std::string> result_file;
  if (manifest_path->extension() == ".json") {
    absl::StatusOr<std::string> raw = ReadFileToString(*manifest_path);
    if (!raw.ok()) {
      return raw.status();
    }
    absl::StatusOr<JSONValue> doc = JSONValue::ParseJSONString(*raw);
    if (!doc.ok()) {
      return doc.status();
    }
    result_file = LookupResultFile(doc->GetConstRef(), query_sql);
  } else {
    result_file = LookupResultFileFromYamlManifest(*manifest_path, query_sql);
  }
  if (!result_file.ok()) {
    return absl::NotFoundError(absl::StrCat(
        result_file.status().message(),
        " — add an entry to ",
        manifest_path->string()));
  }

  absl::StatusOr<fs::path> result_path =
      ResolveResultPath(conn_dir, *result_file);
  if (!result_path.ok()) {
    return result_path.status();
  }
  return ParseResultFile(*result_path, type_factory);
}

}  // namespace semantic
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
