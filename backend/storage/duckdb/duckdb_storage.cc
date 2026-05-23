#include "backend/storage/duckdb/duckdb_storage.h"

#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <memory>
#include <sstream>
#include <string>
#include <system_error>
#include <utility>
#include <vector>

#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "absl/strings/match.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/str_replace.h"
#include "absl/strings/string_view.h"
#include "absl/synchronization/mutex.h"
#include "absl/types/span.h"
#include "backend/schema/schema.h"
#include "backend/storage/storage.h"
#include "emulator.pb.h"
#include "google/protobuf/util/json_util.h"

#ifdef BIGQUERY_EMULATOR_HAS_DUCKDB
#include "duckdb.h"
#endif

namespace bigquery_emulator {
namespace backend {
namespace storage {
namespace duckdb {

namespace {

namespace fs = std::filesystem;

// Sidecar filename suffix that distinguishes BigQuery-typed metadata
// from the underlying parquet file. Two tables in the same dataset
// cannot share a `table_id` so the prefix is unambiguous.
constexpr absl::string_view kTableMetaSuffix = ".meta.json";
constexpr absl::string_view kDatasetMetaFile = "_dataset.meta.json";
constexpr absl::string_view kCatalogDbFile = "catalog.duckdb";

// Builds an absl::Status from a duckdb error string. The DuckDB C API
// does not bucket errors by category, so we surface the raw message
// at the requested status code.
absl::Status DuckDBError(absl::StatusCode code, absl::string_view what,
                          absl::string_view detail) {
  if (detail.empty()) {
    return absl::Status(code, std::string(what));
  }
  return absl::Status(code, absl::StrCat(what, ": ", detail));
}

// Escapes a DuckDB SQL identifier by doubling embedded double-quotes,
// then wrapping the result in double-quotes. Used so dataset / table
// ids that contain hyphens (BigQuery permits them) round-trip
// through a `CREATE SCHEMA` / `CREATE TABLE` statement safely.
std::string QuoteIdent(absl::string_view ident) {
  std::string escaped = absl::StrReplaceAll(ident, {{"\"", "\"\""}});
  return absl::StrCat("\"", escaped, "\"");
}

}  // namespace

// ---------------------------------------------------------------------------
// Pimpl: holds the DuckDB C handles. Kept out of the header so callers
// in `backend/engine/...` cannot reach for the raw connection by
// accident (the only legal surface is the abstract `Storage`
// interface).
// ---------------------------------------------------------------------------
struct DuckDBStorage::Impl {
#ifdef BIGQUERY_EMULATOR_HAS_DUCKDB
  ::duckdb_database database = nullptr;
  ::duckdb_connection connection = nullptr;
#endif

  ~Impl() {
#ifdef BIGQUERY_EMULATOR_HAS_DUCKDB
    if (connection != nullptr) {
      ::duckdb_disconnect(&connection);
    }
    if (database != nullptr) {
      ::duckdb_close(&database);
    }
#endif
  }
};

namespace {

// Runs `sql` on `impl`'s connection and returns OK or an INTERNAL
// status with the DuckDB error message attached. The caller is
// responsible for ensuring `sql` is already escaped / parameterized
// against injection — every site in this file uses `QuoteIdent` on
// the user-supplied portions before reaching here.
absl::Status RunSql(DuckDBStorage::Impl* impl, absl::string_view sql) {
#ifdef BIGQUERY_EMULATOR_HAS_DUCKDB
  if (impl == nullptr || impl->connection == nullptr) {
    return absl::FailedPreconditionError(
        "DuckDBStorage: connection is not initialized");
  }
  ::duckdb_result result;
  const std::string sql_str(sql);
  const auto state =
      ::duckdb_query(impl->connection, sql_str.c_str(), &result);
  if (state != ::DuckDBSuccess) {
    const char* err = ::duckdb_result_error(&result);
    std::string detail = err == nullptr ? std::string("") : std::string(err);
    ::duckdb_destroy_result(&result);
    return DuckDBError(absl::StatusCode::kInternal,
                        absl::StrCat("DuckDB query failed: ", sql_str),
                        detail);
  }
  ::duckdb_destroy_result(&result);
  return absl::OkStatus();
#else
  (void)impl;
  (void)sql;
  return absl::UnimplementedError(
      "DuckDBStorage: built without BIGQUERY_EMULATOR_HAS_DUCKDB");
#endif
}

// Translates a std::error_code from a std::filesystem call into an
// absl::Status. We deliberately bucket "already exists" the same way
// the in-memory store does so callers can switch backends without
// rewriting their error-handling code.
absl::Status FilesystemStatus(absl::string_view what,
                               const std::error_code& ec) {
  if (!ec) return absl::OkStatus();
  return absl::Status(absl::StatusCode::kInternal,
                       absl::StrCat(what, ": ", ec.message()));
}

// Writes `contents` to `path` atomically by rendering to `path.tmp`
// first and renaming on top. Avoids torn sidecars if the process
// dies mid-write; subsequent opens always see a complete JSON file
// or no file at all.
absl::Status WriteFileAtomic(const fs::path& path,
                              absl::string_view contents) {
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
      return absl::Status(
          absl::StatusCode::kInternal,
          absl::StrCat("failed to write ", tmp.string()));
    }
  }
  std::error_code ec;
  fs::rename(tmp, path, ec);
  if (ec) {
    fs::remove(tmp, ec);
    return absl::Status(
        absl::StatusCode::kInternal,
        absl::StrCat("failed to rename ", tmp.string(), " -> ",
                     path.string()));
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
    return absl::Status(
        absl::StatusCode::kInternal,
        absl::StrCat("failed to render schema as JSON: ",
                     std::string(status.message())));
  }
  // Hand-rolled outer wrapper. The four BigQuery metadata fields are
  // empty placeholders today; the catalog handler (Phase 3g) is what
  // mutates them.
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
  std::string escaped = absl::StrReplaceAll(location, {{"\"", "\\\""},
                                                          {"\\", "\\\\"}});
  return absl::StrCat("{\n  \"location\": \"", escaped, "\"\n}\n");
}

// Lifts the embedded `schema` object out of a table sidecar JSON
// blob and parses it back into a `schema::TableSchema`. The parser
// is intentionally narrow: it does not validate the metadata fields
// at the top level so a developer can hand-edit the file without
// the storage layer slapping their hand.
absl::StatusOr<schema::TableSchema> ParseTableMetaJson(
    absl::string_view json) {
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
    return absl::Status(
        absl::StatusCode::kInternal,
        absl::StrCat("failed to parse schema JSON: ",
                     std::string(status.message())));
  }
  return schema::TableSchemaFromProto(proto);
}

}  // namespace

// ---------------------------------------------------------------------------
// Construction
// ---------------------------------------------------------------------------

DuckDBStorage::DuckDBStorage(std::string data_dir,
                              std::unique_ptr<Impl> impl)
    : data_dir_(std::move(data_dir)), impl_(std::move(impl)) {}

DuckDBStorage::~DuckDBStorage() = default;

absl::StatusOr<std::unique_ptr<DuckDBStorage>> DuckDBStorage::Open(
    absl::string_view data_dir) {
  if (data_dir.empty()) {
    return absl::InvalidArgumentError(
        "DuckDBStorage::Open: data_dir must be non-empty");
  }
#ifndef BIGQUERY_EMULATOR_HAS_DUCKDB
  return absl::UnimplementedError(
      "DuckDBStorage: this build was configured with "
      "-DBIGQUERY_EMULATOR_ENABLE_DUCKDB=OFF");
#else
  fs::path root = fs::path(std::string(data_dir));
  std::error_code ec;
  fs::create_directories(root, ec);
  if (ec) {
    return absl::FailedPreconditionError(absl::StrCat(
        "DuckDBStorage::Open: could not create data_dir ", root.string(),
        ": ", ec.message()));
  }
  const fs::path catalog_path = root / std::string(kCatalogDbFile);
  auto impl = std::make_unique<Impl>();
  const auto open_state = ::duckdb_open(catalog_path.c_str(), &impl->database);
  if (open_state != ::DuckDBSuccess) {
    return absl::InternalError(absl::StrCat(
        "DuckDBStorage::Open: duckdb_open failed for ", catalog_path.string()));
  }
  const auto conn_state =
      ::duckdb_connect(impl->database, &impl->connection);
  if (conn_state != ::DuckDBSuccess) {
    return absl::InternalError(absl::StrCat(
        "DuckDBStorage::Open: duckdb_connect failed for ",
        catalog_path.string()));
  }
  return std::unique_ptr<DuckDBStorage>(
      new DuckDBStorage(std::string(data_dir), std::move(impl)));
#endif
}

// ---------------------------------------------------------------------------
// Path / name helpers
// ---------------------------------------------------------------------------

std::string DuckDBStorage::DatasetDir(absl::string_view project_id,
                                       absl::string_view dataset_id) const {
  return (fs::path(data_dir_) / std::string(project_id) /
          std::string(dataset_id))
      .string();
}

std::string DuckDBStorage::DatasetDir(const DatasetId& id) const {
  return DatasetDir(id.project_id, id.dataset_id);
}

std::string DuckDBStorage::DatasetMetaPath(const DatasetId& id) const {
  return (fs::path(DatasetDir(id)) / std::string(kDatasetMetaFile)).string();
}

std::string DuckDBStorage::TableMetaPath(const TableId& id) const {
  return (fs::path(DatasetDir(id.project_id, id.dataset_id)) /
          absl::StrCat(id.table_id, kTableMetaSuffix))
      .string();
}

std::string DuckDBStorage::TableParquetPath(const TableId& id) const {
  return (fs::path(DatasetDir(id.project_id, id.dataset_id)) /
          absl::StrCat(id.table_id, ".parquet"))
      .string();
}

std::string DuckDBStorage::DuckDBSchemaName(absl::string_view project_id,
                                              absl::string_view dataset_id) {
  // Mirror the storage key separator from `InMemoryStorage`. BigQuery
  // disallows control characters in either id, so the result is
  // always a fresh, unambiguous DuckDB identifier even after
  // double-quote escaping.
  return absl::StrCat(project_id, "__", dataset_id);
}

std::string DuckDBStorage::DuckDBSchemaName(const DatasetId& id) {
  return DuckDBSchemaName(id.project_id, id.dataset_id);
}

// ---------------------------------------------------------------------------
// Dataset CRUD
// ---------------------------------------------------------------------------

absl::Status DuckDBStorage::CreateDataset(const DatasetId& id,
                                            absl::string_view location) {
  if (id.project_id.empty() || id.dataset_id.empty()) {
    return absl::InvalidArgumentError(
        "CreateDataset: project_id and dataset_id must be non-empty");
  }
  absl::MutexLock lock(&mu_);
  const fs::path ds_dir = DatasetDir(id);
  std::error_code ec;
  if (fs::exists(ds_dir, ec)) {
    return absl::AlreadyExistsError(absl::StrCat(
        "dataset already exists: ", id.project_id, ".", id.dataset_id));
  }
  fs::create_directories(ds_dir, ec);
  if (ec) {
    return FilesystemStatus(
        absl::StrCat("failed to create dataset dir: ", ds_dir.string()), ec);
  }
  const auto meta_status =
      WriteFileAtomic(DatasetMetaPath(id), RenderDatasetMetaJson(location));
  if (!meta_status.ok()) {
    fs::remove_all(ds_dir, ec);
    return meta_status;
  }
  const std::string schema_name = DuckDBSchemaName(id);
  // CREATE SCHEMA so the catalog file mirrors the on-disk layout.
  // The two are not strictly redundant: the schema lets the DuckDB
  // engine (Phase 5.B) attach tables by qualified name without
  // re-deriving the directory from the data dir on every query.
  const auto sql_status = RunSql(
      impl_.get(),
      absl::StrCat("CREATE SCHEMA IF NOT EXISTS ", QuoteIdent(schema_name)));
  if (!sql_status.ok()) {
    fs::remove_all(ds_dir, ec);
    return sql_status;
  }
  return absl::OkStatus();
}

absl::Status DuckDBStorage::DropDataset(const DatasetId& id,
                                          bool delete_contents) {
  absl::MutexLock lock(&mu_);
  const fs::path ds_dir = DatasetDir(id);
  std::error_code ec;
  if (!fs::exists(ds_dir, ec)) {
    return absl::NotFoundError(absl::StrCat(
        "dataset not found: ", id.project_id, ".", id.dataset_id));
  }
  // Count non-metadata entries (any `.meta.json` / `.parquet` pair
  // counts as one table; the dataset sidecar is excluded).
  bool has_tables = false;
  for (const auto& entry : fs::directory_iterator(ds_dir, ec)) {
    if (ec) break;
    const std::string name = entry.path().filename().string();
    if (name == kDatasetMetaFile) continue;
    if (name.size() > kTableMetaSuffix.size() &&
        absl::EndsWith(name, kTableMetaSuffix)) {
      has_tables = true;
      break;
    }
    if (entry.path().extension() == ".parquet") {
      has_tables = true;
      break;
    }
  }
  if (ec) {
    return FilesystemStatus(
        absl::StrCat("failed to enumerate dataset dir: ", ds_dir.string()),
        ec);
  }
  if (has_tables && !delete_contents) {
    return absl::FailedPreconditionError(absl::StrCat(
        "dataset is not empty: ", id.project_id, ".", id.dataset_id,
        " (use delete_contents=true to drop with tables)"));
  }
  fs::remove_all(ds_dir, ec);
  if (ec) {
    return FilesystemStatus(
        absl::StrCat("failed to remove dataset dir: ", ds_dir.string()), ec);
  }
  // Drop the matching DuckDB schema. We use CASCADE here to mirror
  // the file-tree removal above (every table file is gone, so any
  // catalog references in DuckDB must go too).
  const std::string schema_name = DuckDBSchemaName(id);
  return RunSql(impl_.get(),
                 absl::StrCat("DROP SCHEMA IF EXISTS ",
                              QuoteIdent(schema_name), " CASCADE"));
}

// ---------------------------------------------------------------------------
// Table CRUD
// ---------------------------------------------------------------------------

absl::Status DuckDBStorage::CreateTable(
    const TableId& id, const schema::TableSchema& schema) {
  if (id.table_id.empty()) {
    return absl::InvalidArgumentError(
        "CreateTable: table_id must be non-empty");
  }
  absl::MutexLock lock(&mu_);
  const fs::path ds_dir = DatasetDir(id.project_id, id.dataset_id);
  std::error_code ec;
  if (!fs::exists(ds_dir, ec)) {
    return absl::NotFoundError(absl::StrCat(
        "dataset not found: ", id.project_id, ".", id.dataset_id));
  }
  const fs::path meta_path = TableMetaPath(id);
  if (fs::exists(meta_path, ec)) {
    return absl::AlreadyExistsError(absl::StrCat(
        "table already exists: ", id.project_id, ".", id.dataset_id, ".",
        id.table_id));
  }
  auto meta_json_or = RenderTableMetaJson(schema);
  if (!meta_json_or.ok()) return meta_json_or.status();
  const auto write_status = WriteFileAtomic(meta_path, *meta_json_or);
  if (!write_status.ok()) return write_status;
  return absl::OkStatus();
}

absl::Status DuckDBStorage::DropTable(const TableId& id) {
  absl::MutexLock lock(&mu_);
  const fs::path ds_dir = DatasetDir(id.project_id, id.dataset_id);
  std::error_code ec;
  if (!fs::exists(ds_dir, ec)) {
    return absl::NotFoundError(absl::StrCat(
        "dataset not found: ", id.project_id, ".", id.dataset_id));
  }
  const fs::path meta_path = TableMetaPath(id);
  if (!fs::exists(meta_path, ec)) {
    return absl::NotFoundError(absl::StrCat(
        "table not found: ", id.project_id, ".", id.dataset_id, ".",
        id.table_id));
  }
  fs::remove(meta_path, ec);
  if (ec) {
    return FilesystemStatus(
        absl::StrCat("failed to remove table sidecar: ", meta_path.string()),
        ec);
  }
  const fs::path parquet_path = TableParquetPath(id);
  if (fs::exists(parquet_path, ec)) {
    fs::remove(parquet_path, ec);
    if (ec) {
      return FilesystemStatus(
          absl::StrCat("failed to remove table parquet: ",
                       parquet_path.string()),
          ec);
    }
  }
  return absl::OkStatus();
}

absl::StatusOr<schema::TableSchema> DuckDBStorage::GetSchema(
    const TableId& id) const {
  absl::MutexLock lock(&mu_);
  const fs::path ds_dir = DatasetDir(id.project_id, id.dataset_id);
  std::error_code ec;
  if (!fs::exists(ds_dir, ec)) {
    return absl::NotFoundError(absl::StrCat(
        "dataset not found: ", id.project_id, ".", id.dataset_id));
  }
  const fs::path meta_path = TableMetaPath(id);
  if (!fs::exists(meta_path, ec)) {
    return absl::NotFoundError(absl::StrCat(
        "table not found: ", id.project_id, ".", id.dataset_id, ".",
        id.table_id));
  }
  auto contents_or = ReadFile(meta_path);
  if (!contents_or.ok()) return contents_or.status();
  return ParseTableMetaJson(*contents_or);
}

// ---------------------------------------------------------------------------
// Row CRUD — deferred to `duckdb-storage-ddl_p1e2f3a4`.
// ---------------------------------------------------------------------------

absl::Status DuckDBStorage::AppendRows(const TableId& /*id*/,
                                         absl::Span<const Row> /*rows*/) {
  return absl::UnimplementedError(
      "DuckDBStorage::AppendRows is wired in plan duckdb-storage-ddl");
}

absl::StatusOr<std::unique_ptr<RowIterator>> DuckDBStorage::ScanRows(
    const TableId& /*id*/) const {
  return absl::UnimplementedError(
      "DuckDBStorage::ScanRows is wired in plan duckdb-storage-ddl");
}

}  // namespace duckdb
}  // namespace storage
}  // namespace backend
}  // namespace bigquery_emulator
