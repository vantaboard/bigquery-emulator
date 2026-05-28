#include "backend/storage/duckdb/duckdb_storage.h"

#include <cerrno>
#include <cmath>
#include <cstdint>
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
#include "absl/strings/str_format.h"
#include "absl/strings/str_replace.h"
#include "absl/strings/string_view.h"
#include "absl/synchronization/mutex.h"
#include "absl/types/span.h"
#include "backend/schema/schema.h"
#include "backend/storage/row_restriction.h"
#include "backend/storage/storage.h"
#include "duckdb.h"
#include "proto/emulator.pb.h"
#include "google/protobuf/util/json_util.h"

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

// Escapes a DuckDB SQL string literal by doubling embedded
// single-quotes. The result is *not* wrapped in quotes; the caller is
// responsible for that so the helper composes cleanly into
// `'...'`, `DATE '...'`, `TIMESTAMP '...'`, etc.
std::string EscapeStringLiteralInner(absl::string_view s) {
  return absl::StrReplaceAll(s, {{"'", "''"}});
}

// Renders raw bytes as a DuckDB BLOB literal (lower-case hex).
// DuckDB accepts both `BLOB '\xAB\xCD'` and the SQL-standard
// `X'ABCD'`; the latter is simpler to emit because every byte is
// exactly two characters of output and there is no escape sequence
// to think about.
std::string RenderBlobLiteral(absl::string_view bytes) {
  static const char* kHex = "0123456789abcdef";
  std::string out;
  out.reserve(bytes.size() * 2 + 4);
  absl::StrAppend(&out, "X'");
  for (unsigned char c : bytes) {
    out += kHex[c >> 4];
    out += kHex[c & 0x0f];
  }
  absl::StrAppend(&out, "'");
  return out;
}

// Forward declaration for the recursive cell renderer.
absl::StatusOr<std::string> RenderCellLiteral(
    const Value& cell, const schema::ColumnSchema& column);

// Renders a single non-repeated scalar value as a DuckDB SQL literal,
// excluding the NULL case (the caller short-circuits that before
// calling). The column metadata is needed to pick the right SQL
// literal form for the temporal / numeric types that round-trip as
// strings in our Value union.
//
// The Phase 1 gateway tabledata.insertAll path lowers every JSON cell
// to a `Value::String` regardless of the column's declared type
// (see the comment on `frontend/handlers/catalog.cc::CellToValue`).
// For numeric / boolean columns we therefore accept both the natively-
// typed `Value::Int64` / `Value::Float64` / `Value::Bool` and a
// `Value::String` carrying the textual representation, and we delegate
// the final parse to DuckDB by emitting a CAST literal. This keeps
// the storage layer compatible with the wire-shape stringification
// the gateway performs while still surfacing malformed values as a
// CAST failure from DuckDB rather than silently storing zero.
absl::StatusOr<std::string> RenderScalarLiteral(
    const Value& cell, const schema::ColumnSchema& column) {
  switch (column.type) {
    case schema::ColumnType::kBool:
      if (cell.kind() == Value::Kind::kString) {
        return absl::StrCat("CAST('",
                             EscapeStringLiteralInner(cell.string_value()),
                             "' AS BOOLEAN)");
      }
      return std::string(cell.bool_value() ? "TRUE" : "FALSE");
    case schema::ColumnType::kInt64:
      if (cell.kind() == Value::Kind::kString) {
        return absl::StrCat("CAST('",
                             EscapeStringLiteralInner(cell.string_value()),
                             "' AS BIGINT)");
      }
      return absl::StrCat(cell.int64_value());
    case schema::ColumnType::kFloat64: {
      if (cell.kind() == Value::Kind::kString) {
        return absl::StrCat("CAST('",
                             EscapeStringLiteralInner(cell.string_value()),
                             "' AS DOUBLE)");
      }
      const double v = cell.float64_value();
      if (std::isnan(v)) return std::string("'NaN'::DOUBLE");
      if (std::isinf(v)) {
        return std::string(v > 0 ? "'Infinity'::DOUBLE"
                                  : "'-Infinity'::DOUBLE");
      }
      return absl::StrFormat("%.17g", v);
    }
    case schema::ColumnType::kString:
    case schema::ColumnType::kJson:
    case schema::ColumnType::kGeography:
      return absl::StrCat("'", EscapeStringLiteralInner(cell.string_value()),
                          "'");
    case schema::ColumnType::kBytes:
      return RenderBlobLiteral(cell.string_value());
    case schema::ColumnType::kDate:
      return absl::StrCat("DATE '",
                           EscapeStringLiteralInner(cell.string_value()), "'");
    case schema::ColumnType::kTime:
      return absl::StrCat("TIME '",
                           EscapeStringLiteralInner(cell.string_value()), "'");
    case schema::ColumnType::kDatetime:
      return absl::StrCat("TIMESTAMP '",
                           EscapeStringLiteralInner(cell.string_value()), "'");
    case schema::ColumnType::kTimestamp:
      return absl::StrCat("TIMESTAMPTZ '",
                           EscapeStringLiteralInner(cell.string_value()), "'");
    case schema::ColumnType::kNumeric:
    case schema::ColumnType::kBignumeric:
      // Stored as a textual decimal in our Value union; let DuckDB
      // re-parse it under the declared precision/scale so out-of-
      // range values surface as an INTERNAL from RunSql.
      return absl::StrCat(
          "CAST('", EscapeStringLiteralInner(cell.string_value()),
          "' AS ", schema::ToDuckDBType(column.type), ")");
    case schema::ColumnType::kStruct: {
      if (cell.kind() != Value::Kind::kStruct) {
        return absl::InvalidArgumentError(absl::StrCat(
            "AppendRows: column '", column.name,
            "' expects STRUCT but row provided non-struct cell"));
      }
      const auto& fields = cell.struct_value();
      if (fields.size() != column.fields.size()) {
        return absl::InvalidArgumentError(absl::StrCat(
            "AppendRows: STRUCT column '", column.name, "' has ",
            column.fields.size(), " fields but row provided ", fields.size()));
      }
      std::string out = "{";
      for (size_t i = 0; i < fields.size(); ++i) {
        if (i > 0) absl::StrAppend(&out, ", ");
        absl::StrAppend(&out, "'",
                         EscapeStringLiteralInner(column.fields[i].name),
                         "': ");
        auto inner_or = RenderCellLiteral(fields[i], column.fields[i]);
        if (!inner_or.ok()) return inner_or.status();
        absl::StrAppend(&out, *inner_or);
      }
      absl::StrAppend(&out, "}");
      return out;
    }
    case schema::ColumnType::kArray:
    case schema::ColumnType::kUnknown:
      return absl::StrCat("'",
                           EscapeStringLiteralInner(cell.string_value()),
                           "'");
  }
  return absl::InternalError("RenderScalarLiteral: unreachable");
}

absl::StatusOr<std::string> RenderCellLiteral(
    const Value& cell, const schema::ColumnSchema& column) {
  if (cell.is_null()) return std::string("NULL");
  // REPEATED cells carry an array on the wire even when the column
  // type itself is a scalar like INT64 — DuckDB's LIST literal form
  // is `[v1, v2, ...]`. Use a synthetic non-repeated column for the
  // element renderer so the recursive call doesn't re-enter the
  // array branch.
  if (column.mode == schema::ColumnMode::kRepeated) {
    schema::ColumnSchema element = column;
    element.mode = schema::ColumnMode::kNullable;
    std::string out = "[";
    const auto& elems = cell.array_value();
    for (size_t i = 0; i < elems.size(); ++i) {
      if (i > 0) absl::StrAppend(&out, ", ");
      auto inner_or = RenderCellLiteral(elems[i], element);
      if (!inner_or.ok()) return inner_or.status();
      absl::StrAppend(&out, *inner_or);
    }
    absl::StrAppend(&out, "]");
    return out;
  }
  return RenderScalarLiteral(cell, column);
}

// Builds the parenthesized column list / type list for a CREATE TABLE
// or COPY ... TO statement against the given schema.
std::string RenderColumnList(const schema::TableSchema& schema) {
  std::string out = "(";
  for (size_t i = 0; i < schema.columns.size(); ++i) {
    if (i > 0) absl::StrAppend(&out, ", ");
    absl::StrAppend(&out, QuoteIdent(schema.columns[i].name), " ",
                     schema::ColumnSchemaToDuckDBType(schema.columns[i]));
  }
  absl::StrAppend(&out, ")");
  return out;
}

// Renders an EqualityPredicate as a DuckDB `WHERE` clause fragment
// (with leading space, no trailing semicolon). The column identifier
// is double-quote escaped and the literal goes through the same
// `'...'` / numeric / CAST shapes the rest of this file uses, so a
// caller cannot escape the WHERE via crafted input — the parser
// already restricted the input to INT64 / BOOL / STRING literals
// (see `row_restriction.cc`) and the string literal is rendered with
// `EscapeStringLiteralInner`.
//
// Returned shape examples:
//   ` WHERE "id" = 42`
//   ` WHERE "active" = TRUE`
//   ` WHERE "name" = 'ada''s laptop'`
std::string RenderPredicateClause(const EqualityPredicate& pred) {
  std::string out = " WHERE ";
  absl::StrAppend(&out, QuoteIdent(pred.column), " = ");
  switch (pred.kind) {
    case EqualityPredicate::Kind::kInt64:
      absl::StrAppend(&out, pred.int64_value);
      return out;
    case EqualityPredicate::Kind::kBool:
      absl::StrAppend(&out, pred.bool_value ? "TRUE" : "FALSE");
      return out;
    case EqualityPredicate::Kind::kString:
      absl::StrAppend(&out, "'",
                       EscapeStringLiteralInner(pred.string_value), "'");
      return out;
  }
  return out;
}

// Just the bare comma-separated identifier list (no type info, no
// trailing parenthesis). Used inside the SELECT projection list for
// `read_parquet` scans so the column order matches the table schema
// even if the on-disk parquet shuffled them.
std::string RenderColumnIdentList(const schema::TableSchema& schema) {
  std::string out;
  for (size_t i = 0; i < schema.columns.size(); ++i) {
    if (i > 0) absl::StrAppend(&out, ", ");
    absl::StrAppend(&out, QuoteIdent(schema.columns[i].name));
  }
  return out;
}

}  // namespace

// ---------------------------------------------------------------------------
// Pimpl: holds the DuckDB C handles. Kept out of the header so callers
// in `backend/engine/...` cannot reach for the raw connection by
// accident (the only legal surface is the abstract `Storage`
// interface).
// ---------------------------------------------------------------------------
struct DuckDBStorage::Impl {
  ::duckdb_database database = nullptr;
  ::duckdb_connection connection = nullptr;

  ~Impl() {
    if (connection != nullptr) {
      ::duckdb_disconnect(&connection);
    }
    if (database != nullptr) {
      ::duckdb_close(&database);
    }
  }
};

namespace {

// Runs `sql` on `impl`'s connection and returns OK or an INTERNAL
// status with the DuckDB error message attached. The caller is
// responsible for ensuring `sql` is already escaped / parameterized
// against injection — every site in this file uses `QuoteIdent` on
// the user-supplied portions before reaching here.
absl::Status RunSql(DuckDBStorage::Impl* impl, absl::string_view sql) {
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

  // Materialize an empty Parquet file alongside the sidecar so the
  // table file exists from day one: subsequent `read_parquet(...)`
  // calls in ScanRows can rely on the path being valid even before
  // any rows are appended, and a hand-curated --data_dir is
  // inspectable with stock parquet tools without going through the
  // emulator. The DDL plan (`duckdb-storage-ddl_p1e2f3a4`) lays the
  // file out via a transient DuckDB table because COPY needs a
  // bound logical type list, and the empty-result `SELECT ... WHERE
  // FALSE` trick loses the column-type information.
  const std::string parquet_path = TableParquetPath(id);
  const std::string tmp_table = "main.__bqemu_mkempty";
  const std::string cols = RenderColumnList(schema);
  const auto create_status =
      RunSql(impl_.get(), absl::StrCat("CREATE OR REPLACE TEMP TABLE ",
                                         tmp_table, " ", cols));
  if (!create_status.ok()) {
    fs::remove(meta_path, ec);
    return create_status;
  }
  const auto copy_status = RunSql(
      impl_.get(),
      absl::StrCat("COPY ", tmp_table, " TO '",
                    EscapeStringLiteralInner(parquet_path),
                    "' (FORMAT PARQUET)"));
  const auto drop_status =
      RunSql(impl_.get(), absl::StrCat("DROP TABLE ", tmp_table));
  if (!copy_status.ok()) {
    fs::remove(meta_path, ec);
    fs::remove(parquet_path, ec);
    return copy_status;
  }
  if (!drop_status.ok()) return drop_status;
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
// Row CRUD
//
// The on-disk Parquet file is the source of truth: each AppendRows
// call loads the existing rows into a transient DuckDB table, layers
// the new batch on top via a multi-row `INSERT ... VALUES`, then
// rewrites the Parquet file via an atomic rename. ScanRows reads the
// Parquet file straight through `read_parquet(...)` without
// touching a DuckDB table at all.
//
// We deliberately do NOT keep a long-lived DuckDB table mirroring
// the rows (Design B). Parquet survives DuckDB-catalog corruption,
// is inspectable from outside the emulator, and matches the
// per-table layout documented at the top of this file.
// ---------------------------------------------------------------------------

namespace {

// Drops the temp table used to stage AppendRows. Best-effort; the
// outer logic has already either succeeded or surfaced the original
// failure, so any error here is logged at INTERNAL but does not
// override the caller's status.
void TryDropTempTable(DuckDBStorage::Impl* impl,
                       absl::string_view qualified_name) {
  RunSql(impl, absl::StrCat("DROP TABLE IF EXISTS ", qualified_name))
      .IgnoreError();
}

// Reads a single DuckDB cell into a storage::Value, using the
// column type from `column` to pick the right C-API accessor. NULL
// cells become Value::Null() regardless of column type.
absl::StatusOr<Value> ReadCell(::duckdb_result* result, idx_t col,
                                 idx_t row,
                                 const schema::ColumnSchema& column) {
  if (::duckdb_value_is_null(result, col, row)) return Value::Null();
  // REPEATED cells / ARRAY: pull them through `duckdb_value_varchar`
  // and parse — DuckDB renders LIST values as `[a, b, c]` which is
  // the same shape the engines emit; the integration test does not
  // exercise repeated cells, so we keep this branch on the textual
  // path and let Phase 5.B revisit when the engine needs typed
  // arrays. Future TODO: switch to `duckdb_result_get_chunk` for
  // typed extraction without the lossy varchar round-trip.
  if (column.mode == schema::ColumnMode::kRepeated ||
      column.type == schema::ColumnType::kArray ||
      column.type == schema::ColumnType::kStruct) {
    char* str = ::duckdb_value_varchar(result, col, row);
    Value out = Value::String(str == nullptr ? std::string("") : str);
    if (str != nullptr) ::duckdb_free(str);
    return out;
  }
  switch (column.type) {
    case schema::ColumnType::kBool:
      return Value::Bool(::duckdb_value_boolean(result, col, row));
    case schema::ColumnType::kInt64:
      return Value::Int64(::duckdb_value_int64(result, col, row));
    case schema::ColumnType::kFloat64:
      return Value::Float64(::duckdb_value_double(result, col, row));
    case schema::ColumnType::kBytes: {
      ::duckdb_blob blob = ::duckdb_value_blob(result, col, row);
      std::string bytes;
      if (blob.data != nullptr) {
        bytes.assign(static_cast<const char*>(blob.data), blob.size);
        ::duckdb_free(blob.data);
      }
      return Value::Bytes(std::move(bytes));
    }
    // Every remaining type round-trips through the canonical DuckDB
    // CAST-to-VARCHAR rendering (RFC 3339 for dates/timestamps, plain
    // decimal for NUMERIC, etc.). The storage layer is engine-
    // agnostic, so we keep these as kString cells and let the
    // downstream encoder pick the BigQuery wire shape.
    case schema::ColumnType::kString:
    case schema::ColumnType::kDate:
    case schema::ColumnType::kTime:
    case schema::ColumnType::kDatetime:
    case schema::ColumnType::kTimestamp:
    case schema::ColumnType::kNumeric:
    case schema::ColumnType::kBignumeric:
    case schema::ColumnType::kJson:
    case schema::ColumnType::kGeography:
    case schema::ColumnType::kArray:
    case schema::ColumnType::kStruct:
    case schema::ColumnType::kUnknown: {
      char* str = ::duckdb_value_varchar(result, col, row);
      Value out = Value::String(str == nullptr ? std::string("") : str);
      if (str != nullptr) ::duckdb_free(str);
      return out;
    }
  }
  return absl::InternalError("ReadCell: unreachable column type");
}

class VectorRowIterator : public RowIterator {
 public:
  explicit VectorRowIterator(std::vector<Row> rows)
      : rows_(std::move(rows)), pos_(0) {}

  absl::StatusOr<bool> Next(Row* row) override {
    if (pos_ >= rows_.size()) return false;
    *row = rows_[pos_++];
    return true;
  }

 private:
  std::vector<Row> rows_;
  size_t pos_;
};

}  // namespace

absl::Status DuckDBStorage::AppendRows(const TableId& id,
                                         absl::Span<const Row> rows) {
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
  if (rows.empty()) return absl::OkStatus();

  auto contents_or = ReadFile(meta_path);
  if (!contents_or.ok()) return contents_or.status();
  auto schema_or = ParseTableMetaJson(*contents_or);
  if (!schema_or.ok()) return schema_or.status();
  const schema::TableSchema& schema = *schema_or;
  const size_t ncols = schema.columns.size();
  for (size_t i = 0; i < rows.size(); ++i) {
    if (rows[i].cells.size() != ncols) {
      return absl::InvalidArgumentError(absl::StrCat(
          "AppendRows: row[", i, "] has ", rows[i].cells.size(),
          " cell(s) but table ", id.project_id, ".", id.dataset_id, ".",
          id.table_id, " has ", ncols, " column(s)"));
    }
  }

  const std::string parquet_path = TableParquetPath(id);
  const std::string tmp_path = absl::StrCat(parquet_path, ".tmp");
  // Use a deterministic temp-table name so a crashed previous
  // AppendRows leaves a recoverable scratch object; CREATE OR
  // REPLACE wipes it on the next call.
  const std::string tmp_table = "main.__bqemu_append";

  // 1. Stage a fresh temp table with the explicit column schema.
  // CREATE OR REPLACE TEMP TABLE coerces a stale row from an
  // interrupted previous run if any.
  const std::string col_list = RenderColumnList(schema);
  auto status = RunSql(impl_.get(),
                        absl::StrCat("CREATE OR REPLACE TEMP TABLE ",
                                     tmp_table, " ", col_list));
  if (!status.ok()) return status;

  // 2. Carry over existing rows if a parquet snapshot exists. The
  // file is created by CreateTable so the normal path always has
  // one; a hand-curated dataset directory may not, in which case
  // we treat the table as empty.
  if (fs::exists(parquet_path, ec)) {
    const std::string select_cols = RenderColumnIdentList(schema);
    status =
        RunSql(impl_.get(),
                absl::StrCat("INSERT INTO ", tmp_table, " SELECT ",
                             select_cols, " FROM read_parquet('",
                             EscapeStringLiteralInner(parquet_path), "')"));
    if (!status.ok()) {
      TryDropTempTable(impl_.get(), tmp_table);
      return status;
    }
  }

  // 3. Emit one multi-row INSERT VALUES so we hit the planner once
  // for the whole batch. The literal renderer raises on shape
  // mismatches; on error we tear down the temp table and bail
  // without touching the parquet file.
  std::string insert_sql;
  absl::StrAppend(&insert_sql, "INSERT INTO ", tmp_table, " VALUES ");
  for (size_t r = 0; r < rows.size(); ++r) {
    if (r > 0) absl::StrAppend(&insert_sql, ", ");
    absl::StrAppend(&insert_sql, "(");
    for (size_t c = 0; c < ncols; ++c) {
      if (c > 0) absl::StrAppend(&insert_sql, ", ");
      auto cell_or = RenderCellLiteral(rows[r].cells[c], schema.columns[c]);
      if (!cell_or.ok()) {
        TryDropTempTable(impl_.get(), tmp_table);
        return cell_or.status();
      }
      absl::StrAppend(&insert_sql, *cell_or);
    }
    absl::StrAppend(&insert_sql, ")");
  }
  status = RunSql(impl_.get(), insert_sql);
  if (!status.ok()) {
    TryDropTempTable(impl_.get(), tmp_table);
    return status;
  }

  // 4. Write the rewritten table to a sibling tmp file so the
  // rename in step 5 is the only window where readers see partial
  // state — DuckDB's COPY itself is not atomic.
  fs::remove(tmp_path, ec);
  status =
      RunSql(impl_.get(),
              absl::StrCat("COPY ", tmp_table, " TO '",
                           EscapeStringLiteralInner(tmp_path),
                           "' (FORMAT PARQUET)"));
  if (!status.ok()) {
    TryDropTempTable(impl_.get(), tmp_table);
    fs::remove(tmp_path, ec);
    return status;
  }

  // 5. Atomic swap. POSIX rename(2) replaces the target on Linux
  // so a concurrent reader either sees the old or the new snapshot,
  // never a torn file.
  fs::rename(tmp_path, parquet_path, ec);
  if (ec) {
    TryDropTempTable(impl_.get(), tmp_table);
    fs::remove(tmp_path, ec);
    return absl::Status(
        absl::StatusCode::kInternal,
        absl::StrCat("AppendRows: failed to rename ", tmp_path, " -> ",
                     parquet_path, ": ", ec.message()));
  }
  TryDropTempTable(impl_.get(), tmp_table);
  return absl::OkStatus();
}

absl::Status DuckDBStorage::OverwriteRows(const TableId& id,
                                            absl::Span<const Row> rows) {
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
  auto schema_or = ParseTableMetaJson(*contents_or);
  if (!schema_or.ok()) return schema_or.status();
  const schema::TableSchema& schema = *schema_or;
  const size_t ncols = schema.columns.size();
  for (size_t i = 0; i < rows.size(); ++i) {
    if (rows[i].cells.size() != ncols) {
      return absl::InvalidArgumentError(absl::StrCat(
          "OverwriteRows: row[", i, "] has ", rows[i].cells.size(),
          " cell(s) but table ", id.project_id, ".", id.dataset_id, ".",
          id.table_id, " has ", ncols, " column(s)"));
    }
  }

  const std::string parquet_path = TableParquetPath(id);
  const std::string tmp_path = absl::StrCat(parquet_path, ".tmp");
  const std::string tmp_table = "main.__bqemu_overwrite";

  // Stage a fresh temp table holding just the new rows. We deliberately
  // do NOT carry over existing rows (unlike AppendRows) -- this is the
  // overwrite contract: replace the parquet file with whatever the
  // caller hands us, including the empty-vector case.
  const std::string col_list = RenderColumnList(schema);
  auto status = RunSql(impl_.get(),
                        absl::StrCat("CREATE OR REPLACE TEMP TABLE ",
                                     tmp_table, " ", col_list));
  if (!status.ok()) return status;

  if (!rows.empty()) {
    std::string insert_sql;
    absl::StrAppend(&insert_sql, "INSERT INTO ", tmp_table, " VALUES ");
    for (size_t r = 0; r < rows.size(); ++r) {
      if (r > 0) absl::StrAppend(&insert_sql, ", ");
      absl::StrAppend(&insert_sql, "(");
      for (size_t c = 0; c < ncols; ++c) {
        if (c > 0) absl::StrAppend(&insert_sql, ", ");
        auto cell_or = RenderCellLiteral(rows[r].cells[c], schema.columns[c]);
        if (!cell_or.ok()) {
          TryDropTempTable(impl_.get(), tmp_table);
          return cell_or.status();
        }
        absl::StrAppend(&insert_sql, *cell_or);
      }
      absl::StrAppend(&insert_sql, ")");
    }
    status = RunSql(impl_.get(), insert_sql);
    if (!status.ok()) {
      TryDropTempTable(impl_.get(), tmp_table);
      return status;
    }
  }

  fs::remove(tmp_path, ec);
  status =
      RunSql(impl_.get(),
              absl::StrCat("COPY ", tmp_table, " TO '",
                           EscapeStringLiteralInner(tmp_path),
                           "' (FORMAT PARQUET)"));
  if (!status.ok()) {
    TryDropTempTable(impl_.get(), tmp_table);
    fs::remove(tmp_path, ec);
    return status;
  }

  fs::rename(tmp_path, parquet_path, ec);
  if (ec) {
    TryDropTempTable(impl_.get(), tmp_table);
    fs::remove(tmp_path, ec);
    return absl::Status(
        absl::StatusCode::kInternal,
        absl::StrCat("OverwriteRows: failed to rename ", tmp_path, " -> ",
                     parquet_path, ": ", ec.message()));
  }
  TryDropTempTable(impl_.get(), tmp_table);
  return absl::OkStatus();
}

namespace {

// Runs `sql` (a SELECT) against `impl`'s connection and materializes
// every emitted row into `*out`. Shared between `ScanRows` and
// `CreateReadStream`: both differ only in whether they push LIMIT /
// OFFSET into the SQL, but the post-execute decoding loop is
// identical, and lifting it out avoids two near-duplicate copies of
// the duckdb_result handling.
//
// `tag` is the caller-facing name plumbed into error messages
// ("ScanRows" / "CreateReadStream") so a failure in either branch is
// attributed to the right surface.
absl::Status ExecuteSelect(DuckDBStorage::Impl* impl, absl::string_view sql,
                            const schema::TableSchema& schema,
                            absl::string_view tag,
                            const TableId& id,
                            absl::string_view parquet_path,
                            std::vector<Row>* out) {
  const std::string sql_str(sql);
  ::duckdb_result result;
  const auto state =
      ::duckdb_query(impl->connection, sql_str.c_str(), &result);
  if (state != ::DuckDBSuccess) {
    const char* err = ::duckdb_result_error(&result);
    std::string detail = err == nullptr ? std::string("") : std::string(err);
    ::duckdb_destroy_result(&result);
    return absl::InternalError(absl::StrCat(
        tag, ": duckdb_query failed for ", parquet_path, ": ", detail));
  }
  const idx_t row_count = ::duckdb_row_count(&result);
  const idx_t col_count = ::duckdb_column_count(&result);
  if (col_count != schema.columns.size()) {
    ::duckdb_destroy_result(&result);
    return absl::InternalError(absl::StrCat(
        tag, ": parquet file has ", col_count,
        " column(s) but sidecar schema declares ", schema.columns.size(),
        " for table ", id.project_id, ".", id.dataset_id, ".", id.table_id));
  }
  out->reserve(out->size() + row_count);
  for (idx_t r = 0; r < row_count; ++r) {
    Row row;
    row.cells.reserve(col_count);
    for (idx_t c = 0; c < col_count; ++c) {
      auto cell_or = ReadCell(&result, c, r, schema.columns[c]);
      if (!cell_or.ok()) {
        ::duckdb_destroy_result(&result);
        return cell_or.status();
      }
      row.cells.push_back(std::move(*cell_or));
    }
    out->push_back(std::move(row));
  }
  ::duckdb_destroy_result(&result);
  return absl::OkStatus();
}

}  // namespace

absl::StatusOr<std::unique_ptr<RowIterator>> DuckDBStorage::ScanRows(
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
  auto schema_or = ParseTableMetaJson(*contents_or);
  if (!schema_or.ok()) return schema_or.status();
  const schema::TableSchema& schema = *schema_or;

  const std::string parquet_path = TableParquetPath(id);
  std::vector<Row> rows;
  if (!fs::exists(parquet_path, ec)) {
    return std::unique_ptr<RowIterator>(
        new VectorRowIterator(std::move(rows)));
  }

  // Explicit projection: ScanRows promises rows in column-list
  // order regardless of how the parquet file laid them out (the
  // file is written by us, but a user could hand-edit it).
  const std::string select_cols = RenderColumnIdentList(schema);
  const std::string sql = absl::StrCat(
      "SELECT ", select_cols, " FROM read_parquet('",
      EscapeStringLiteralInner(parquet_path), "')");
  auto status = ExecuteSelect(impl_.get(), sql, schema, "ScanRows", id,
                                parquet_path, &rows);
  if (!status.ok()) return status;
  return std::unique_ptr<RowIterator>(
      new VectorRowIterator(std::move(rows)));
}

absl::StatusOr<std::unique_ptr<RowIterator>> DuckDBStorage::CreateReadStream(
    const TableId& id, const ReadFilter& filter) const {
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
  auto schema_or = ParseTableMetaJson(*contents_or);
  if (!schema_or.ok()) return schema_or.status();
  const schema::TableSchema& schema = *schema_or;

  const std::string parquet_path = TableParquetPath(id);
  std::vector<Row> rows;
  if (!fs::exists(parquet_path, ec)) {
    return std::unique_ptr<RowIterator>(
        new VectorRowIterator(std::move(rows)));
  }

  // ORDER BY a stable row identifier so OFFSET / LIMIT yield
  // deterministic windows across calls. The plan-31 Arrow pipeline
  // uses `read_parquet`'s `file_row_number` extra column for the
  // same purpose; reuse that here so a caller resuming a stream
  // (plan-39 territory) at offset=N gets the same rows it would
  // have received if it had stayed connected. The column is
  // synthesized by DuckDB at scan time and never selected into the
  // result.
  const std::string select_cols = RenderColumnIdentList(schema);
  std::string sql = absl::StrCat(
      "SELECT ", select_cols,
      " FROM read_parquet('", EscapeStringLiteralInner(parquet_path),
      "', file_row_number = true)");
  // Plan 39: push the parsed `<column> = <literal>` predicate down
  // into a SQL WHERE clause. The clause lands before ORDER BY so
  // DuckDB filters on the parquet scan side rather than buffering the
  // full table.
  if (filter.equality_predicate.has_value()) {
    absl::StrAppend(&sql, RenderPredicateClause(*filter.equality_predicate));
  }
  absl::StrAppend(&sql, " ORDER BY file_row_number");
  if (filter.row_limit > 0) {
    absl::StrAppend(&sql, " LIMIT ", filter.row_limit);
  }
  if (filter.offset > 0) {
    // DuckDB requires LIMIT to appear before OFFSET. When the caller
    // did not pin a limit we still need to emit one for OFFSET to
    // parse — use the DuckDB `LIMIT ALL` form so the optimizer keeps
    // the cap unbounded.
    if (filter.row_limit <= 0) {
      absl::StrAppend(&sql, " LIMIT ALL");
    }
    absl::StrAppend(&sql, " OFFSET ", filter.offset);
  }
  auto status = ExecuteSelect(impl_.get(), sql, schema,
                                "CreateReadStream", id, parquet_path, &rows);
  if (!status.ok()) return status;
  return std::unique_ptr<RowIterator>(
      new VectorRowIterator(std::move(rows)));
}

}  // namespace duckdb
}  // namespace storage
}  // namespace backend
}  // namespace bigquery_emulator
