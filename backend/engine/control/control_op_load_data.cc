#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "absl/strings/ascii.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/str_replace.h"
#include "absl/strings/string_view.h"
#include "backend/engine/control/control_op_internal.h"
#include "backend/engine/control/control_op_options.h"
#include "backend/engine/duckdb/udf/registrar.h"
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

std::string EscapePathLiteral(absl::string_view path) {
  return absl::StrReplaceAll(path, {{ "'", "''" }});
}

absl::StatusOr<std::string> BuildReadSql(absl::string_view path,
                                         absl::string_view format) {
  const std::string escaped = EscapePathLiteral(path);
  const std::string upper = absl::AsciiStrToUpper(format);
  if (upper == "CSV" || upper.empty()) {
    return absl::StrCat(
        "SELECT * FROM read_csv_auto('", escaped, "', header = true)");
  }
  if (upper == "JSON" || upper == "NEWLINE_DELIMITED_JSON") {
    return absl::StrCat("SELECT * FROM read_json_auto('", escaped, "')");
  }
  if (upper == "PARQUET") {
    return absl::StrCat("SELECT * FROM read_parquet('", escaped, "')");
  }
  return absl::InvalidArgumentError(absl::StrCat(
      "control op executor: LOAD DATA unsupported format '", format, "'"));
}

absl::StatusOr<schema::TableSchema> ResolveLoadSchema(
    storage::Storage& storage,
    const storage::TableId& target,
    const ::googlesql::ResolvedAuxLoadDataStmt* stmt) {
  if (stmt->column_definition_list_size() > 0) {
    return ColumnDefinitionListToTableSchema(stmt->column_definition_list());
  }
  auto existing = storage.GetSchema(target);
  if (existing.ok()) return *existing;
  return absl::InvalidArgumentError(
      "control op executor: LOAD DATA requires an explicit column list or an "
      "existing destination table schema");
}

}  // namespace

absl::Status RunLoadData(storage::Storage& storage,
                         absl::string_view project_id,
                         absl::string_view default_dataset_id,
                         const ::googlesql::ResolvedAuxLoadDataStmt* stmt) {
  if (stmt == nullptr) {
    return absl::InternalError(
        "control op executor: LOAD DATA has null resolved statement");
  }

  absl::StatusOr<storage::TableId> target =
      NamePathToTableId(stmt->name_path(), project_id, default_dataset_id);
  if (!target.ok()) return target.status();
  if (auto ds =
          EnsureDatasetExists(storage, target->project_id, target->dataset_id);
      !ds.ok()) {
    return ds;
  }

  std::string format = "CSV";
  std::vector<std::string> uris;
  for (int i = 0; i < stmt->from_files_option_list_size(); ++i) {
    const ::googlesql::ResolvedOption* opt = stmt->from_files_option_list(i);
    if (opt == nullptr) continue;
    if (absl::EqualsIgnoreCase(opt->name(), "format")) {
      auto fmt = OptionStringValue(opt);
      if (!fmt.ok()) return fmt.status();
      format = *fmt;
      continue;
    }
    if (absl::EqualsIgnoreCase(opt->name(), "uris")) {
      auto parsed = ExtractStringArrayLiteral(opt->value());
      if (!parsed.ok()) return parsed.status();
      uris = *std::move(parsed);
    }
  }
  if (uris.empty()) {
    return absl::InvalidArgumentError(
        "control op executor: LOAD DATA requires FROM FILES (uris = [...])");
  }
  if (uris.size() != 1) {
    return absl::UnimplementedError(
        "control op executor: LOAD DATA multi-file uris are not implemented "
        "yet");
  }

  absl::StatusOr<std::string> path_or = LocalPathFromUri(uris[0]);
  if (!path_or.ok()) return path_or.status();

  absl::StatusOr<schema::TableSchema> bq_schema =
      ResolveLoadSchema(storage, *target, stmt);
  if (!bq_schema.ok()) return bq_schema.status();

  ::duckdb_database db = nullptr;
  if (::duckdb_open(nullptr, &db) != ::DuckDBSuccess) {
    return absl::InternalError(
        "control op executor: duckdb_open(in-memory) failed");
  }
  ::duckdb_connection conn = nullptr;
  if (::duckdb_connect(db, &conn) != ::DuckDBSuccess) {
    ::duckdb_close(&db);
    return absl::InternalError(
        "control op executor: duckdb_connect failed");
  }
  if (auto reg = duckdb::udf::RegisterAll(conn); !reg.ok()) {
    ::duckdb_disconnect(&conn);
    ::duckdb_close(&db);
    return reg;
  }

  absl::StatusOr<std::string> read_sql = BuildReadSql(*path_or, format);
  if (!read_sql.ok()) {
    ::duckdb_disconnect(&conn);
    ::duckdb_close(&db);
    return read_sql.status();
  }

  const std::string staging = "__bqemu_load_staging";
  absl::Status staged = RunSqlNoResult(
      conn,
      absl::StrCat("CREATE OR REPLACE TEMP TABLE ",
                   QuoteIdent(staging),
                   " AS ",
                   *read_sql));
  if (!staged.ok()) {
    ::duckdb_disconnect(&conn);
    ::duckdb_close(&db);
    return staged;
  }

  absl::StatusOr<std::vector<storage::Row>> rows =
      DrainTableRows(conn, QuoteIdent(staging), *bq_schema);
  ::duckdb_disconnect(&conn);
  ::duckdb_close(&db);
  if (!rows.ok()) return rows.status();

  if (stmt->insertion_mode() ==
      ::googlesql::ResolvedAuxLoadDataStmt::OVERWRITE) {
    absl::Status dropped = storage.DropTable(*target);
    if (!dropped.ok() && dropped.code() != absl::StatusCode::kNotFound) {
      return dropped;
    }
    absl::Status created = storage.CreateTable(*target, *bq_schema);
    if (!created.ok()) return created;
  } else {
    auto existing = storage.GetSchema(*target);
    if (!existing.ok()) {
      absl::Status created = storage.CreateTable(*target, *bq_schema);
      if (!created.ok()) return created;
    }
  }

  if (rows->empty()) return absl::OkStatus();
  return storage.AppendRows(*target, absl::MakeConstSpan(*rows));
}

}  // namespace internal
}  // namespace control
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
