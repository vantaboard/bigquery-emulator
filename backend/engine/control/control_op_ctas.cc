// CREATE TABLE AS SELECT for the control-op executor. Split out of
// `control_op_ddl.cc` to keep that file under the cpp-lint file-length cap.

#include <memory>
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
#include "backend/schema/schema.h"
#include "backend/storage/storage.h"
#include "duckdb.h"
#include "googlesql/resolved_ast/resolved_ast.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace control {
namespace internal {

absl::StatusOr<std::string> BuildDuckdbCtasSql(
    absl::string_view request_sql,
    const ::googlesql::ResolvedCreateTableAsSelectStmt* stmt,
    const storage::TableId& target,
    const schema::TableSchema& bq_schema,
    absl::Span<const QueryParameter> parameters) {
  if (stmt == nullptr || stmt->query() == nullptr) {
    return absl::InternalError(
        "ControlOpExecutor::ExecuteDdl: CTAS has null query scan");
  }
  duckdb::transpiler::Transpiler transpiler;
  std::string select_sql = transpiler.EmitCtasSelect(stmt);
  if (select_sql.empty()) {
    return absl::UnimplementedError(
        "control op executor: CTAS query scan did not transpile to DuckDB SQL");
  }
  if (!transpiler.parameter_order().empty()) {
    auto substituted = SubstituteDuckdbParameters(
        std::move(select_sql), transpiler.parameter_order(), parameters);
    if (!substituted.ok()) return substituted.status();
    select_sql = *std::move(substituted);
  }
  (void)bq_schema;
  const bool is_temp = absl::StrContains(request_sql, "CREATE TEMP") ||
                       absl::StrContains(request_sql, "create temp");
  // DuckDB temp tables live in the session default schema; schema-
  // qualified `CREATE TEMP TABLE "ds"."t"` fails with "schema does
  // not exist". Persistent targets use `"dataset"."table"`.
  const std::string dest_name =
      is_temp ? QuoteIdent(target.table_id)
              : absl::StrCat(QuoteIdent(target.dataset_id),
                             ".",
                             QuoteIdent(target.table_id));
  // DuckDB accepts `CREATE TABLE ... AS <select>` but not BigQuery's
  // `CREATE TABLE (col list) AS <select>` spelling. The analyzer's
  // `column_definition_list` still drives the storage-layer schema
  // we persist after draining the DuckDB result.
  return absl::StrCat("CREATE ",
                      is_temp ? "OR REPLACE TEMP " : "",
                      "TABLE ",
                      dest_name,
                      " AS ",
                      select_sql);
}

struct DuckDbSession {
  ::duckdb_database db = nullptr;
  ::duckdb_connection conn = nullptr;

  DuckDbSession() = default;
  DuckDbSession(const DuckDbSession&) = delete;
  DuckDbSession& operator=(const DuckDbSession&) = delete;
  DuckDbSession(DuckDbSession&& other) noexcept
      : db(std::exchange(other.db, nullptr)),
        conn(std::exchange(other.conn, nullptr)) {}
  DuckDbSession& operator=(DuckDbSession&& other) noexcept {
    if (this == &other) return *this;
    if (conn != nullptr) ::duckdb_disconnect(&conn);
    if (db != nullptr) ::duckdb_close(&db);
    db = std::exchange(other.db, nullptr);
    conn = std::exchange(other.conn, nullptr);
    return *this;
  }
  ~DuckDbSession() {
    if (conn != nullptr) ::duckdb_disconnect(&conn);
    if (db != nullptr) ::duckdb_close(&db);
  }
};

absl::StatusOr<DuckDbSession> OpenDuckDbSession() {
  DuckDbSession session;
  if (::duckdb_open(nullptr, &session.db) != ::DuckDBSuccess) {
    return absl::InternalError(
        "ControlOpExecutor::ExecuteDdl: duckdb_open(in-memory) failed");
  }
  if (::duckdb_connect(session.db, &session.conn) != ::DuckDBSuccess) {
    return absl::InternalError(
        "ControlOpExecutor::ExecuteDdl: duckdb_connect failed");
  }
  if (auto reg = duckdb::udf::RegisterAll(session.conn); !reg.ok()) {
    return reg;
  }
  return session;
}

absl::Status EnsureTargetSchemaExistsIfNeeded(::duckdb_connection conn,
                                              const storage::TableId& target,
                                              bool is_temp_ctas) {
  if (is_temp_ctas) return absl::OkStatus();
  return RunSqlNoResult(conn,
                        absl::StrCat("CREATE SCHEMA IF NOT EXISTS ",
                                     QuoteIdent(target.dataset_id)));
}

absl::Status AttachCollectorTables(::duckdb_connection conn,
                                   storage::Storage& storage,
                                   const TableScanCollector& collector) {
  for (const ::googlesql::Table* tbl : collector.tables()) {
    if (tbl == nullptr) {
      return absl::FailedPreconditionError(
          "ControlOpExecutor::ExecuteDdl: null table reference in CTAS scan");
    }
    const auto* source_table = dynamic_cast<const catalog::StorageTable*>(tbl);
    if (source_table == nullptr) {
      return absl::FailedPreconditionError(absl::StrCat(
          "ControlOpExecutor::ExecuteDdl: cannot attach non-StorageTable '",
          tbl->Name(),
          "' for CTAS; rebuild against a "
          "GoogleSqlCatalog-backed analyzer"));
    }
    // Attach at the bare table name so transpiled `FROM "people"` (see
    // `EmitTableScan`) resolves in the connection default schema, matching
    // `DuckDbExecutor::ExecuteQuery`.
    absl::Status attach = AttachStorageTableAt(
        conn, &storage, *source_table, QuoteIdent(tbl->Name()));
    if (!attach.ok()) return attach;
  }
  return absl::OkStatus();
}

// --- CREATE TABLE AS SELECT handler ---------------------------------------
// statementType: `CREATE_TABLE_AS_SELECT`. Materializes source tables in an
// in-memory DuckDB connection, executes the CTAS SQL, then persists rows in
// BigQuery schema shape.
absl::Status RunCreateTableAsSelect(
    storage::Storage& storage,
    absl::string_view project_id,
    const QueryRequest& request,
    const ::googlesql::ResolvedCreateTableAsSelectStmt* stmt,
    const ::googlesql::ResolvedStatement* root_stmt) {
  if (stmt == nullptr) {
    return absl::InternalError(
        "ControlOpExecutor::ExecuteDdl: CTAS has null resolved statement");
  }
  absl::StatusOr<storage::TableId> target = NamePathToTableId(
      stmt->name_path(), project_id, request.default_dataset_id);
  if (!target.ok()) return target.status();
  if (auto ds =
          EnsureDatasetExists(storage, target->project_id, target->dataset_id);
      !ds.ok()) {
    return ds;
  }
  // BigQuery leaves an existing table untouched for CREATE TABLE IF NOT
  // EXISTS ... AS SELECT: the query does not run and no rows are added.
  if (stmt->create_mode() ==
          ::googlesql::ResolvedCreateStatement::CREATE_IF_NOT_EXISTS &&
      storage.GetSchema(*target).ok()) {
    return absl::OkStatus();
  }
  absl::StatusOr<schema::TableSchema> bq_schema =
      ColumnDefinitionListToTableSchema(stmt->column_definition_list());
  if (!bq_schema.ok()) return bq_schema.status();

  TableScanCollector collector;
  absl::Status visit_status = root_stmt->Accept(&collector);
  if (!visit_status.ok()) return visit_status;

  absl::StatusOr<DuckDbSession> session = OpenDuckDbSession();
  if (!session.ok()) return session.status();
  ::duckdb_connection conn = session->conn;

  const bool is_temp_ctas = absl::StrContains(request.sql, "CREATE TEMP") ||
                            absl::StrContains(request.sql, "create temp");
  absl::Status target_schema =
      EnsureTargetSchemaExistsIfNeeded(conn, *target, is_temp_ctas);
  if (!target_schema.ok()) return target_schema;
  absl::Status attach = AttachCollectorTables(conn, storage, collector);
  if (!attach.ok()) return attach;

  if (stmt->create_mode() ==
      ::googlesql::ResolvedCreateStatement::CREATE_OR_REPLACE) {
    absl::Status dropped = storage.DropTable(*target);
    if (!dropped.ok() && dropped.code() != absl::StatusCode::kNotFound) {
      return dropped;
    }
  }

  absl::StatusOr<std::string> ctas_sql = BuildDuckdbCtasSql(
      request.sql, stmt, *target, *bq_schema, request.parameters);
  if (!ctas_sql.ok()) return ctas_sql.status();

  ::duckdb_result ctas_result;
  if (::duckdb_query(conn, ctas_sql->c_str(), &ctas_result) !=
      ::DuckDBSuccess) {
    const auto* err = ::duckdb_result_error(&ctas_result);
    std::string detail = err == nullptr ? std::string("") : std::string(err);
    ::duckdb_destroy_result(&ctas_result);
    return absl::InvalidArgumentError(
        absl::StrCat("ControlOpExecutor: DuckDB rejected CTAS: ",
                     detail,
                     " (sql=",
                     *ctas_sql,
                     ")"));
  }
  ::duckdb_destroy_result(&ctas_result);

  const std::string quoted_target =
      is_temp_ctas ? QuoteIdent(target->table_id)
                   : absl::StrCat(QuoteIdent(target->dataset_id),
                                  ".",
                                  QuoteIdent(target->table_id));
  absl::StatusOr<std::vector<storage::Row>> after_rows =
      DrainTableRows(conn, quoted_target, *bq_schema);
  if (!after_rows.ok()) return after_rows.status();

  if (is_temp_ctas) {
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
