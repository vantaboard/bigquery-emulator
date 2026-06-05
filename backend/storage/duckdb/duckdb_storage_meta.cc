#include <fstream>
#include <sstream>
#include <string>
#include <system_error>

#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/str_replace.h"
#include "absl/strings/string_view.h"
#include "backend/schema/schema.h"
#include "backend/storage/duckdb/duckdb_storage_internal.h"
#include "google/protobuf/util/json_util.h"
#include "proto/emulator.pb.h"

namespace bigquery_emulator {
namespace backend {
namespace storage {
namespace duckdb {
namespace internal {

namespace fs = std::filesystem;

absl::Status DuckDBError(absl::StatusCode code,
                         absl::string_view what,
                         absl::string_view detail) {
  if (detail.empty()) {
    return absl::Status(code, std::string(what));
  }
  return absl::Status(code, absl::StrCat(what, ": ", detail));
}
// Writes `contents` to `path` atomically by rendering to `path.tmp`
// first and renaming on top. Avoids torn sidecars if the process
// dies mid-write; subsequent opens always see a complete JSON file
// or no file at all.
absl::Status WriteFileAtomic(const fs::path& path, absl::string_view contents) {
  const fs::path tmp = path.string() + ".tmp";
  {
    std::ofstream out(tmp, std::ios::binary | std::ios::trunc);
    if (!out) {
      return absl::Status(
          absl::StatusCode::kInternal,
          absl::StrCat("failed to open ", tmp.string(), " for write"));
    }
    out.write(contents.data(), static_cast<std::streamsize>(contents.size()));
    if (!out) {
      return absl::Status(absl::StatusCode::kInternal,
                          absl::StrCat("failed to write ", tmp.string()));
    }
  }
  std::error_code ec;
  fs::rename(tmp, path, ec);
  if (ec) {
    fs::remove(tmp, ec);
    return absl::Status(
        absl::StatusCode::kInternal,
        absl::StrCat("failed to rename ", tmp.string(), " -> ", path.string()));
  }
  return absl::OkStatus();
}

absl::StatusOr<std::string> ReadFile(const fs::path& path) {
  std::ifstream in(path, std::ios::binary);
  if (!in) {
    return absl::Status(absl::StatusCode::kNotFound,
                        absl::StrCat("file not found: ", path.string()));
  }
  std::ostringstream ss;
  ss << in.rdbuf();
  if (!in && !in.eof()) {
    return absl::Status(absl::StatusCode::kInternal,
                        absl::StrCat("failed to read ", path.string()));
  }
  return ss.str();
}

// Sidecar layout (top-level fields the gateway can edit by hand):
//
//   {
//     "description": "...",
//     "friendlyName": "...",
//     "etag": "...",
//     "labels": { "k": "v", ... },
//     "schema": { ...proto3 TableSchema as JSON... }
//   }
//
// Only `schema` is required for the core plan; the BigQuery metadata
// fields are written empty so the file is consistent with the public
// REST shape from day one. Future plans (catalog gRPC) populate them.
absl::StatusOr<std::string> RenderTableMetaJson(
    const schema::TableSchema& schema) {
  v1::TableSchema proto;
  schema::TableSchemaToProto(schema, &proto);
  std::string schema_json;
  google::protobuf::util::JsonPrintOptions opts;
  opts.add_whitespace = true;
  opts.preserve_proto_field_names = true;
  const auto status =
      google::protobuf::util::MessageToJsonString(proto, &schema_json, opts);
  if (!status.ok()) {
    return absl::Status(absl::StatusCode::kInternal,
                        absl::StrCat("failed to render schema as JSON: ",
                                     std::string(status.message())));
  }
  // Hand-rolled outer wrapper. The four BigQuery metadata fields are
  // empty placeholders today; the catalog handler is what mutates
  // them.
  std::string out;
  out.reserve(schema_json.size() + 128);
  absl::StrAppend(&out,
                  "{\n"
                  "  \"description\": \"\",\n"
                  "  \"friendlyName\": \"\",\n"
                  "  \"etag\": \"\",\n"
                  "  \"labels\": {},\n"
                  "  \"schema\": ",
                  schema_json,
                  "}\n");
  return out;
}

// Tiny dataset sidecar — just the BigQuery region for now. Mirrors
// the same atomic-write story as the table sidecar so the catalog
// stays consistent across crashes.
std::string RenderDatasetMetaJson(absl::string_view location) {
  std::string escaped =
      absl::StrReplaceAll(location, {{"\"", "\\\""}, {"\\", "\\\\"}});
  return absl::StrCat("{\n  \"location\": \"", escaped, "\"\n}\n");
}

// Lifts the embedded `schema` object out of a table sidecar JSON
// blob and parses it back into a `schema::TableSchema`. The parser
// is intentionally narrow: it does not validate the metadata fields
// at the top level so a developer can hand-edit the file without
// the storage layer slapping their hand.
absl::StatusOr<schema::TableSchema> ParseTableMetaJson(absl::string_view json) {
  // Locate the `"schema":` key. We don't want to drag a full JSON
  // parser in just to skip three lines of metadata; the file is
  // written by `RenderTableMetaJson` above and the schema object is
  // always the last top-level field, so a substring lookup is safe
  // enough for the round-trip case.
  const auto schema_pos = json.find("\"schema\"");
  if (schema_pos == std::string_view::npos) {
    return absl::InvalidArgumentError(
        "table sidecar: missing \"schema\" field");
  }
  const auto colon = json.find(':', schema_pos);
  if (colon == std::string_view::npos) {
    return absl::InvalidArgumentError(
        "table sidecar: malformed \"schema\" entry");
  }
  // Find the opening brace of the schema object.
  const auto open = json.find('{', colon);
  if (open == std::string_view::npos) {
    return absl::InvalidArgumentError(
        "table sidecar: schema is not a JSON object");
  }
  // Scan to the matching closing brace, respecting strings so a
  // literal `}` inside a description doesn't trip us up. We do not
  // honor escape sequences inside strings because the writer above
  // only emits ASCII identifiers plus protobuf JSON output, which
  // never includes raw `}` characters inside a string literal.
  size_t depth = 0;
  bool in_string = false;
  size_t close = std::string_view::npos;
  for (size_t i = open; i < json.size(); ++i) {
    const char c = json[i];
    if (in_string) {
      if (c == '\\' && i + 1 < json.size()) {
        ++i;
        continue;
      }
      if (c == '"') in_string = false;
      continue;
    }
    if (c == '"') {
      in_string = true;
      continue;
    }
    if (c == '{') {
      ++depth;
    } else if (c == '}') {
      --depth;
      if (depth == 0) {
        close = i;
        break;
      }
    }
  }
  if (close == std::string_view::npos) {
    return absl::InvalidArgumentError(
        "table sidecar: unterminated schema object");
  }
  const std::string schema_json(json.substr(open, close - open + 1));
  v1::TableSchema proto;
  google::protobuf::util::JsonParseOptions opts;
  opts.ignore_unknown_fields = true;
  const auto status =
      google::protobuf::util::JsonStringToMessage(schema_json, &proto, opts);
  if (!status.ok()) {
    return absl::Status(absl::StatusCode::kInternal,
                        absl::StrCat("failed to parse schema JSON: ",
                                     std::string(status.message())));
  }
  return schema::TableSchemaFromProto(proto);
}

}  // namespace internal
}  // namespace duckdb
}  // namespace storage
}  // namespace backend
}  // namespace bigquery_emulator
