#include <memory>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "absl/strings/match.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/string_view.h"
#include "absl/types/span.h"
#include "backend/catalog/storage_table.h"
#include "backend/engine/control/control_op_internal.h"
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

absl::StatusOr<std::string> BuildMaterializeSql(
    absl::string_view request_sql,
    const ::googlesql::ResolvedCreateMaterializedViewStmt* stmt,
    const storage::TableId& target,
    absl::Span<const QueryParameter> parameters) {
  if (stmt == nullptr || stmt->query() == nullptr) {
    return absl::InternalError(
        "control op executor: CREATE MATERIALIZED VIEW has null query scan");
  }
  duckdb::transpiler::Transpiler transpiler;
  std::string select_sql = transpiler.TranspileScan(stmt->query());
  if (select_sql.empty()) {
    return absl::UnimplementedError(
        "control op executor: CREATE MATERIALIZED VIEW query did not transpile "
        "to DuckDB SQL");
  }
  if (!transpiler.parameter_order().empty()) {
    auto substituted = SubstituteDuckdbParameters(
        std::move(select_sql), transpiler.parameter_order(), parameters);
    if (!substituted.ok()) return substituted.status();
    select_sql = *std::move(substituted);
  }
  const bool is_temp = absl::StrContains(request_sql, "CREATE TEMP") ||
                       absl::StrContains(request_sql, "create temp");
  const std::string dest_name =
      is_temp ? QuoteIdent(target.table_id)
              : absl::StrCat(QuoteIdent(target.dataset_id),
                             ".",
                             QuoteIdent(target.table_id));
  return absl::StrCat("CREATE ",
                      is_temp ? "OR REPLACE TEMP " : "",
                      "TABLE ",
                      dest_name,
                      " AS ",
                      select_sql);
}

}  // namespace

absl::Status RunCreateMaterializedView(
    storage::Storage& storage,
    absl::string_view project_id,
    const QueryRequest& request,
    const ::googlesql::ResolvedCreateMaterializedViewStmt* stmt,
    const ::googlesql::ResolvedStatement& root_stmt) {
  if (stmt == nullptr) {
    return absl::InternalError(
        "control op executor: CREATE MATERIALIZED VIEW has null resolved "
        "statement");
  }
  absl::StatusOr<storage::TableId> target = NamePathToTableId(
      stmt->name_path(), project_id, request.default_dataset_id);
  if (!target.ok()) return target.status();
  if (auto ds =
          EnsureDatasetExists(storage, target->project_id, target->dataset_id);
      !ds.ok()) {
    return ds;
  }

  absl::StatusOr<schema::TableSchema> bq_schema =
      ColumnDefinitionListToTableSchema(stmt->column_definition_list());
  if (!bq_schema.ok()) return bq_schema.status();

  TableScanCollector collector;
  absl::Status visit_status = root_stmt.Accept(&collector);
  if (!visit_status.ok()) return visit_status;

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

  const bool is_temp_mv = absl::StrContains(request.sql, "CREATE TEMP") ||
                          absl::StrContains(request.sql, "create temp");
  if (!is_temp_mv) {
    absl::Status target_schema_status =
        RunSqlNoResult(conn,
                       absl::StrCat("CREATE SCHEMA IF NOT EXISTS ",
                                    QuoteIdent(target->dataset_id)));
    if (!target_schema_status.ok()) {
      ::duckdb_disconnect(&conn);
      ::duckdb_close(&db);
      return target_schema_status;
    }
  }

  for (const ::googlesql::Table* tbl : collector.tables()) {
    if (tbl == nullptr) {
      ::duckdb_disconnect(&conn);
      ::duckdb_close(&db);
      return absl::FailedPreconditionError(
          "control op executor: null table reference in CREATE MATERIALIZED "
          "VIEW scan");
    }
    const auto* source_table = dynamic_cast<const catalog::StorageTable*>(tbl);
    if (source_table == nullptr) {
      ::duckdb_disconnect(&conn);
      ::duckdb_close(&db);
      return absl::FailedPreconditionError(
          absl::StrCat("control op executor: cannot attach non-StorageTable '",
                       tbl->Name(),
                       "' for CREATE MATERIALIZED VIEW"));
    }
    absl::Status attach = AttachStorageTableAt(
        conn, &storage, *source_table, QuoteIdent(tbl->Name()));
    if (!attach.ok()) {
      ::duckdb_disconnect(&conn);
      ::duckdb_close(&db);
      return attach;
    }
  }

  if (stmt->create_mode() ==
      ::googlesql::ResolvedCreateStatement::CREATE_OR_REPLACE) {
    absl::Status dropped = storage.DropTable(*target);
    if (!dropped.ok() && dropped.code() != absl::StatusCode::kNotFound) {
      ::duckdb_disconnect(&conn);
      ::duckdb_close(&db);
      return dropped;
    }
  }

  absl::StatusOr<std::string> materialize_sql =
      BuildMaterializeSql(request.sql, stmt, *target, request.parameters);
  if (!materialize_sql.ok()) {
    ::duckdb_disconnect(&conn);
    ::duckdb_close(&db);
    return materialize_sql.status();
  }

  ::duckdb_result mv_result;
  if (::duckdb_query(conn, materialize_sql->c_str(), &mv_result) !=
      ::DuckDBSuccess) {
    const auto* err = ::duckdb_result_error(&mv_result);
    std::string detail = err == nullptr ? std::string("") : std::string(err);
    ::duckdb_destroy_result(&mv_result);
    ::duckdb_disconnect(&conn);
    ::duckdb_close(&db);
    return absl::InvalidArgumentError(
        absl::StrCat("control op executor: DuckDB rejected materialized view: ",
                     detail,
                     " (sql=",
                     *materialize_sql,
                     ")"));
  }
  ::duckdb_destroy_result(&mv_result);

  const std::string quoted_target =
      is_temp_mv ? QuoteIdent(target->table_id)
                 : absl::StrCat(QuoteIdent(target->dataset_id),
                                ".",
                                QuoteIdent(target->table_id));
  absl::StatusOr<std::vector<storage::Row>> after_rows =
      DrainTableRows(conn, quoted_target, *bq_schema);
  ::duckdb_disconnect(&conn);
  ::duckdb_close(&db);
  if (!after_rows.ok()) return after_rows.status();

  if (is_temp_mv) {
    (void)storage.DropTable(*target);
  }

  absl::Status created = ApplyCreateMode(
      storage.CreateTable(*target, *bq_schema), stmt->create_mode());
  if (!created.ok()) return created;
  if (after_rows->empty()) return absl::OkStatus();
  return storage.AppendRows(*target, absl::MakeConstSpan(*after_rows));
}

}  // namespace internal
}  // namespace control
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
