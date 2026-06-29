#include <filesystem>
#include <memory>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "absl/strings/ascii.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/str_replace.h"
#include "absl/strings/string_view.h"
#include "backend/catalog/storage_table.h"
#include "backend/engine/control/control_op_internal.h"
#include "backend/engine/control/control_op_options.h"
#include "backend/engine/control/gcs_uri_resolver.h"
#include "backend/engine/duckdb/transpiler/transpiler.h"
#include "backend/engine/duckdb/udf/registrar.h"
#include "backend/engine/engine.h"
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

std::string EscapePathLiteral(absl::string_view path) {
  return absl::StrReplaceAll(path, {{"'", "''"}});
}

absl::StatusOr<std::string> BuildCopyToSql(absl::string_view select_sql,
                                           absl::string_view path,
                                           absl::string_view format) {
  const std::string escaped = EscapePathLiteral(path);
  const std::string upper = absl::AsciiStrToUpper(format);
  if (upper == "CSV") {
    return absl::StrCat(
        "COPY (", select_sql, ") TO '", escaped, "' (FORMAT CSV, HEADER TRUE)");
  }
  if (upper == "JSON" || upper == "NEWLINE_DELIMITED_JSON") {
    return absl::StrCat(
        "COPY (", select_sql, ") TO '", escaped, "' (FORMAT JSON)");
  }
  if (upper == "PARQUET") {
    return absl::StrCat(
        "COPY (", select_sql, ") TO '", escaped, "' (FORMAT PARQUET)");
  }
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

}  // namespace

absl::Status RunExportData(storage::Storage& storage,
                           const QueryRequest& request,
                           const ::googlesql::ResolvedExportDataStmt* stmt,
                           const ::googlesql::ResolvedStatement& root_stmt) {
  if (stmt == nullptr || stmt->query() == nullptr) {
    return absl::InternalError(
        "control op executor: EXPORT DATA has null query scan");
  }

  absl::StatusOr<std::string> uri_or =
      FindOptionString(stmt->option_list(), "uri");
  if (!uri_or.ok()) return uri_or.status();
  absl::StatusOr<std::string> path_or =
      LocalPathFromUri(*uri_or, storage.data_dir());
  if (!path_or.ok()) return path_or.status();

  std::string format = "CSV";
  if (auto fmt = FindOptionString(stmt->option_list(), "format"); fmt.ok()) {
    format = *fmt;
  }

  duckdb::transpiler::Transpiler transpiler;
  std::string select_sql = transpiler.TranspileScan(stmt->query());
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

  ::duckdb_database db = nullptr;
  if (::duckdb_open(nullptr, &db) != ::DuckDBSuccess) {
    return absl::InternalError(
        "control op executor: duckdb_open(in-memory) failed");
  }
  ::duckdb_connection conn = nullptr;
  if (::duckdb_connect(db, &conn) != ::DuckDBSuccess) {
    ::duckdb_close(&db);
    return absl::InternalError("control op executor: duckdb_connect failed");
  }
  if (auto reg = duckdb::udf::RegisterAll(conn); !reg.ok()) {
    ::duckdb_disconnect(&conn);
    ::duckdb_close(&db);
    return reg;
  }

  absl::Status attach = AttachReferencedTables(conn, storage, root_stmt);
  if (!attach.ok()) {
    ::duckdb_disconnect(&conn);
    ::duckdb_close(&db);
    return attach;
  }

  auto export_to_gcs = absl::StartsWith(*uri_or, "gs://");
  std::string write_path = *path_or;
  if (export_to_gcs) {
    write_path = absl::StrCat(*path_or, ".export.tmp");
  }

  absl::StatusOr<std::string> copy_sql =
      BuildCopyToSql(select_sql, write_path, format);
  if (!copy_sql.ok()) {
    ::duckdb_disconnect(&conn);
    ::duckdb_close(&db);
    return copy_sql.status();
  }

  absl::Status copied = RunSqlNoResult(conn, *copy_sql);
  ::duckdb_disconnect(&conn);
  ::duckdb_close(&db);
  if (!copied.ok()) return copied;
  if (!export_to_gcs) {
    return absl::OkStatus();
  }
  std::string content_type = "text/csv";
  if (format == "JSON" || format == "NEWLINE_DELIMITED_JSON") {
    content_type = "application/json";
  } else if (format == "PARQUET") {
    content_type = "application/octet-stream";
  }
  absl::Status uploaded =
      UploadGCSObjectFromFile(*uri_or, write_path, content_type);
  if (!uploaded.ok()) return uploaded;
  std::error_code ec;
  std::filesystem::remove(write_path, ec);
  return absl::OkStatus();
}

}  // namespace internal
}  // namespace control
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
