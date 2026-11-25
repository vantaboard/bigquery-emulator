#include <cstddef>
#include <filesystem>
#include <memory>
#include <optional>
#include <set>
#include <string>
#include <system_error>
#include <utility>
#include <vector>

#include "absl/functional/function_ref.h"
#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "absl/strings/ascii.h"
#include "absl/strings/match.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/str_replace.h"
#include "absl/strings/string_view.h"
#include "absl/types/span.h"
#include "backend/catalog/storage_table.h"
#include "backend/engine/control/avro_writer.h"
#include "backend/engine/control/control_op_internal.h"
#include "backend/engine/control/control_op_options.h"
#include "backend/engine/control/gcs_uri_resolver.h"
#include "backend/engine/duckdb/arrow_to_bq.h"
#include "backend/engine/duckdb/transpiler/transpiler.h"
#include "backend/engine/duckdb/udf/registrar.h"
#include "backend/engine/engine.h"
#include "backend/schema/googlesql_to_bq.h"
#include "backend/schema/schema.h"
#include "backend/storage/storage.h"
#include "duckdb.h"
#include "googlesql/resolved_ast/resolved_ast.h"
#include "googlesql/resolved_ast/resolved_node_kind.pb.h"
#include "proto/emulator.pb.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace control {
namespace internal {

namespace {

constexpr size_t kInsertBatchRows = 1000;

std::string EscapePathLiteral(absl::string_view path) {
  return absl::StrReplaceAll(path, {{"'", "''"}});
}

absl::StatusOr<std::string> CopyFormatOptions(absl::string_view format) {
  const std::string upper = absl::AsciiStrToUpper(format);
  if (upper == "CSV") return std::string("FORMAT CSV, HEADER TRUE");
  if (upper == "JSON" || upper == "NEWLINE_DELIMITED_JSON") {
    return std::string("FORMAT JSON");
  }
  if (upper == "PARQUET") return std::string("FORMAT PARQUET");
  return absl::InvalidArgumentError(absl::StrCat(
      "control op executor: EXPORT DATA unsupported format '", format, "'"));
}

absl::Status AttachReferencedTables(
    ::duckdb_connection conn,
    storage::Storage& storage,
    const ::googlesql::ResolvedStatement& root) {
  TableScanCollector collector;
  absl::Status visit_status = root.Accept(&collector);
  if (!visit_status.ok()) return visit_status;
  for (const ::googlesql::Table* tbl : collector.tables()) {
    if (tbl == nullptr) {
      return absl::FailedPreconditionError(
          "control op executor: null table reference in EXPORT DATA scan");
    }
    const auto* source_table = dynamic_cast<const catalog::StorageTable*>(tbl);
    if (source_table == nullptr) {
      return absl::FailedPreconditionError(
          absl::StrCat("control op executor: cannot attach non-StorageTable '",
                       tbl->Name(),
                       "' for EXPORT DATA"));
    }
    absl::Status attach = AttachStorageTableAt(
        conn, &storage, *source_table, QuoteIdent(tbl->Name()));
    if (!attach.ok()) return attach;
  }
  return absl::OkStatus();
}

absl::StatusOr<schema::TableSchema> ExportColumns(
    const ::googlesql::ResolvedExportDataStmt& stmt) {
  v1::TableSchema proto;
  absl::Status reflected =
      schema::OutputColumnListToTableSchema(stmt.output_column_list(), &proto);
  if (!reflected.ok()) return reflected;
  return schema::TableSchemaFromProto(proto);
}

struct AvroExport {
  schema::TableSchema columns;
  AvroFileWriter::Options options;
};

absl::StatusOr<AvroExport> ParseAvroExport(
    const ::googlesql::ResolvedExportDataStmt& stmt) {
  AvroExport out;
  if (auto c = FindOptionString(stmt.option_list(), "compression"); c.ok()) {
    absl::StatusOr<AvroCodec> codec = ParseAvroCompression(*c);
    if (!codec.ok()) return codec.status();
    out.options.codec = *codec;
  } else if (!absl::IsNotFound(c.status())) {
    return c.status();
  }
  if (auto logical =
          FindOptionBool(stmt.option_list(), "use_avro_logical_types");
      logical.ok()) {
    out.options.use_logical_types = *logical;
  } else if (!absl::IsNotFound(logical.status())) {
    return logical.status();
  }
  absl::StatusOr<schema::TableSchema> columns = ExportColumns(stmt);
  if (!columns.ok()) return columns.status();
  out.columns = *std::move(columns);
  return out;
}

struct ExportTarget {
  std::string dest_uri;
  bool to_gcs = false;
  std::string cache_path;
  std::string write_path;
  std::string format;
  std::string copy_options;
  std::optional<AvroExport> avro;
};

absl::StatusOr<ExportTarget> PrepareExportTarget(
    storage::Storage& storage, const ::googlesql::ResolvedExportDataStmt& stmt) {
  absl::StatusOr<std::string> uri_or =
      FindOptionString(stmt.option_list(), "uri");
  if (!uri_or.ok()) return uri_or.status();
  ExportTarget target;
  // A gs:// destination is written to a staging file under the GCS cache
  // and uploaded afterwards. Resolving it with `LocalPathFromUri` would try
  // to download the destination object, which does not exist yet.
  target.to_gcs = absl::StartsWith(*uri_or, "gs://");
  absl::StatusOr<std::string> shard_uri = ExportShardUri(*uri_or);
  if (!shard_uri.ok()) return shard_uri.status();
  target.dest_uri = *std::move(shard_uri);
  if (target.to_gcs) {
    absl::StatusOr<std::string> cache_or =
        PrepareGCSExportCachePath(target.dest_uri, storage.data_dir());
    if (!cache_or.ok()) return cache_or.status();
    target.cache_path = *std::move(cache_or);
    target.write_path = absl::StrCat(target.cache_path, ".export.tmp");
  } else {
    absl::StatusOr<std::string> path_or =
        LocalPathFromUri(target.dest_uri, storage.data_dir());
    if (!path_or.ok()) return path_or.status();
    target.write_path = *std::move(path_or);
  }

  target.format = "CSV";
  if (auto fmt = FindOptionString(stmt.option_list(), "format"); fmt.ok()) {
    target.format = *fmt;
  }

  if (absl::AsciiStrToUpper(target.format) == "AVRO") {
    absl::StatusOr<AvroExport> parsed = ParseAvroExport(stmt);
    if (!parsed.ok()) return parsed.status();
    target.avro = *std::move(parsed);
  } else {
    absl::StatusOr<std::string> copy_options = CopyFormatOptions(target.format);
    if (!copy_options.ok()) return copy_options.status();
    target.copy_options = *std::move(copy_options);
  }
  return target;
}

std::string BuildCopyToSql(absl::string_view select_sql,
                           const ExportTarget& target) {
  return absl::StrCat("COPY (",
                      select_sql,
                      ") TO '",
                      EscapePathLiteral(target.write_path),
                      "' (",
                      target.copy_options,
                      ")");
}

absl::Status PublishExport(const ExportTarget& target) {
  if (!target.to_gcs) {
    return absl::OkStatus();
  }
  const std::string upper_format = absl::AsciiStrToUpper(target.format);
  std::string content_type = "text/csv";
  if (upper_format == "JSON" || upper_format == "NEWLINE_DELIMITED_JSON") {
    content_type = "application/json";
  } else if (upper_format == "PARQUET" || upper_format == "AVRO") {
    content_type = "application/octet-stream";
  }
  absl::Status uploaded =
      UploadGCSObjectFromFile(target.dest_uri, target.write_path, content_type);
  std::error_code ec;
  std::filesystem::remove(target.write_path, ec);
  if (!uploaded.ok()) return uploaded;
  // Drop any cached copy of an earlier object at this name so a later
  // LOAD DATA / external table read fetches the bytes just exported.
  std::filesystem::remove(target.cache_path, ec);
  return absl::OkStatus();
}

class InMemoryDuckDb {
 public:
  InMemoryDuckDb() = default;
  InMemoryDuckDb(const InMemoryDuckDb&) = delete;
  InMemoryDuckDb& operator=(const InMemoryDuckDb&) = delete;
  ~InMemoryDuckDb() {
    if (conn_ != nullptr) ::duckdb_disconnect(&conn_);
    if (db_ != nullptr) ::duckdb_close(&db_);
  }

  absl::Status Open() {
    if (::duckdb_open(nullptr, &db_) != ::DuckDBSuccess) {
      return absl::InternalError(
          "control op executor: duckdb_open(in-memory) failed");
    }
    if (::duckdb_connect(db_, &conn_) != ::DuckDBSuccess) {
      return absl::InternalError("control op executor: duckdb_connect failed");
    }
    return absl::OkStatus();
  }

  ::duckdb_connection conn() const { return conn_; }

 private:
  ::duckdb_database db_ = nullptr;
  ::duckdb_connection conn_ = nullptr;
};

absl::Status WriteAvroFromQuery(::duckdb_connection conn,
                                const std::string& select_sql,
                                const std::string& path,
                                const AvroExport& avro) {
  absl::StatusOr<std::unique_ptr<AvroFileWriter>> writer =
      AvroFileWriter::Create(path, avro.columns, avro.options);
  if (!writer.ok()) return writer.status();
  ::duckdb_result result;
  if (::duckdb_query(conn, select_sql.c_str(), &result) != ::DuckDBSuccess) {
    const char* err = ::duckdb_result_error(&result);
    absl::Status failed = absl::InternalError(
        absl::StrCat("control op executor: EXPORT DATA query failed: ",
                     select_sql,
                     ": ",
                     err == nullptr ? "" : err));
    ::duckdb_destroy_result(&result);
    return failed;
  }
  absl::Status status = absl::OkStatus();
  while (status.ok()) {
    ::duckdb_data_chunk chunk = ::duckdb_fetch_chunk(result);
    if (chunk == nullptr) break;
    const ::idx_t n = ::duckdb_data_chunk_get_size(chunk);
    for (::idx_t i = 0; i < n && status.ok(); ++i) {
      absl::StatusOr<storage::Row> row =
          duckdb::arrow_to_bq::ChunkRowToCells(chunk, i, avro.columns);
      status = row.ok() ? (*writer)->Write(*row) : row.status();
    }
    ::duckdb_destroy_data_chunk(&chunk);
  }
  ::duckdb_destroy_result(&result);
  if (!status.ok()) return status;
  return (*writer)->Finish();
}

absl::Status WriteAvroFromRows(RowSource& rows,
                               const std::string& path,
                               const AvroExport& avro) {
  absl::StatusOr<std::unique_ptr<AvroFileWriter>> writer =
      AvroFileWriter::Create(path, avro.columns, avro.options);
  if (!writer.ok()) return writer.status();
  storage::Row row;
  while (true) {
    absl::StatusOr<bool> has = rows.Next(&row);
    if (!has.ok()) return has.status();
    if (!*has) break;
    absl::Status written = (*writer)->Write(row);
    if (!written.ok()) return written;
  }
  return (*writer)->Finish();
}

absl::Status CopyRowsToFile(RowSource& rows,
                            const schema::TableSchema& columns,
                            const ExportTarget& target) {
  InMemoryDuckDb duck;
  absl::Status opened = duck.Open();
  if (!opened.ok()) return opened;
  const std::string table = QuoteIdent("export_rows");
  absl::Status created = CreateOrReplaceTable(duck.conn(), table, columns);
  if (!created.ok()) return created;
  std::vector<storage::Row> batch;
  batch.reserve(kInsertBatchRows);
  auto flush = [&]() -> absl::Status {
    if (batch.empty()) return absl::OkStatus();
    absl::StatusOr<std::string> insert_sql = BuildInsertSqlForRows(
        table, "EXPORT DATA output", columns, absl::MakeConstSpan(batch));
    batch.clear();
    if (!insert_sql.ok()) return insert_sql.status();
    return RunSqlNoResult(duck.conn(), *insert_sql);
  };
  storage::Row row;
  while (true) {
    absl::StatusOr<bool> has = rows.Next(&row);
    if (!has.ok()) return has.status();
    if (!*has) break;
    batch.push_back(std::move(row));
    row = storage::Row();
    if (batch.size() >= kInsertBatchRows) {
      absl::Status flushed = flush();
      if (!flushed.ok()) return flushed;
    }
  }
  absl::Status flushed = flush();
  if (!flushed.ok()) return flushed;
  return RunSqlNoResult(
      duck.conn(),
      BuildCopyToSql(absl::StrCat("SELECT * FROM ", table), target));
}

}  // namespace

absl::Status RunExportData(storage::Storage& storage,
                           const QueryRequest& request,
                           const ::googlesql::ResolvedExportDataStmt* stmt,
                           const ::googlesql::ResolvedStatement& root_stmt) {
  if (stmt == nullptr || stmt->query() == nullptr) {
    return absl::InternalError(
        "control op executor: EXPORT DATA has null query scan");
  }

  absl::StatusOr<ExportTarget> target = PrepareExportTarget(storage, *stmt);
  if (!target.ok()) return target.status();

  duckdb::transpiler::Transpiler transpiler;
  std::string select_sql = transpiler.EmitExportDataSelect(stmt);
  if (select_sql.empty()) {
    return absl::UnimplementedError(
        "control op executor: EXPORT DATA query did not transpile to DuckDB "
        "SQL");
  }
  if (!transpiler.parameter_order().empty()) {
    auto substituted = SubstituteDuckdbParameters(std::move(select_sql),
                                                  transpiler.parameter_order(),
                                                  request.parameters);
    if (!substituted.ok()) return substituted.status();
    select_sql = *std::move(substituted);
  }

  absl::Status copied;
  {
    InMemoryDuckDb duck;
    absl::Status opened = duck.Open();
    if (!opened.ok()) return opened;
    if (auto reg = duckdb::udf::RegisterAll(duck.conn()); !reg.ok()) {
      return reg;
    }
    absl::Status attach =
        AttachReferencedTables(duck.conn(), storage, root_stmt);
    if (!attach.ok()) return attach;
    if (target->avro.has_value()) {
      copied = WriteAvroFromQuery(
          duck.conn(), select_sql, target->write_path, *target->avro);
    } else {
      copied = RunSqlNoResult(duck.conn(), BuildCopyToSql(select_sql, *target));
    }
  }
  if (!copied.ok()) return copied;
  return PublishExport(*target);
}

absl::Status RunExportDataFromRows(
    storage::Storage& storage,
    const ::googlesql::ResolvedExportDataStmt& stmt,
    absl::FunctionRef<absl::StatusOr<std::unique_ptr<RowSource>>()> run_query) {
  absl::StatusOr<ExportTarget> target = PrepareExportTarget(storage, stmt);
  if (!target.ok()) return target.status();
  std::optional<schema::TableSchema> columns;
  if (!target->avro.has_value()) {
    absl::StatusOr<schema::TableSchema> reflected = ExportColumns(stmt);
    if (!reflected.ok()) return reflected.status();
    columns = *std::move(reflected);
  }

  absl::StatusOr<std::unique_ptr<RowSource>> rows = run_query();
  if (!rows.ok()) return rows.status();
  if (*rows == nullptr) {
    return absl::InternalError(
        "control op executor: EXPORT DATA query returned no row source");
  }

  absl::Status written =
      target->avro.has_value()
          ? WriteAvroFromRows(**rows, target->write_path, *target->avro)
          : CopyRowsToFile(**rows, *columns, *target);
  if (!written.ok()) return written;
  return PublishExport(*target);
}

}  // namespace internal
}  // namespace control
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
