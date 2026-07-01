#include "backend/engine/duckdb/duckdb_executor_insert_select_helpers.h"

#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "absl/strings/str_cat.h"
#include "backend/engine/duckdb/duckdb_executor_internal.h"
#include "backend/engine/duckdb/transpiler/transpiler.h"
#include "backend/engine/duckdb/udf/registrar.h"
#include "backend/schema/googlesql_to_bq.h"
#include "proto/emulator.pb.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace duckdb {
namespace internal {

InsertSelectConnection::InsertSelectConnection(
    InsertSelectConnection&& other) noexcept
    : db(std::exchange(other.db, nullptr)),
      conn(std::exchange(other.conn, nullptr)) {}

InsertSelectConnection& InsertSelectConnection::operator=(
    InsertSelectConnection&& other) noexcept {
  if (this == &other) return *this;
  if (conn != nullptr) ::duckdb_disconnect(&conn);
  if (db != nullptr) ::duckdb_close(&db);
  db = std::exchange(other.db, nullptr);
  conn = std::exchange(other.conn, nullptr);
  return *this;
}

InsertSelectConnection::~InsertSelectConnection() {
  if (conn != nullptr) ::duckdb_disconnect(&conn);
  if (db != nullptr) ::duckdb_close(&db);
}

absl::StatusOr<schema::TableSchema> InsertSelectDrainSchema(
    const ::googlesql::ResolvedInsertStmt& insert) {
  v1::TableSchema proto;
  std::vector<std::unique_ptr<const ::googlesql::ResolvedOutputColumn>> outputs;
  outputs.reserve(insert.query_output_column_list_size());
  for (int i = 0; i < insert.query_output_column_list_size(); ++i) {
    const ::googlesql::ResolvedColumn& col = insert.query_output_column_list(i);
    outputs.push_back(::googlesql::MakeResolvedOutputColumn(col.name(), col));
  }
  absl::Status mapped = schema::OutputColumnListToTableSchema(outputs, &proto);
  if (!mapped.ok()) return mapped;
  return schema::TableSchemaFromProto(proto);
}

absl::StatusOr<InsertSelectTarget> ValidateInsertSelectTarget(
    const ::googlesql::ResolvedInsertStmt& insert) {
  if (insert.table_scan() == nullptr ||
      insert.table_scan()->table() == nullptr) {
    return absl::InternalError(
        "DuckDbExecutor::ExecuteDml: INSERT has no resolved target table scan");
  }
  if (insert.query() == nullptr) {
    return absl::InternalError(
        "DuckDbExecutor::ExecuteDml: INSERT ... SELECT has null query scan");
  }
  if (insert.insert_mode() != ::googlesql::ResolvedInsertStmt::OR_ERROR) {
    return absl::UnimplementedError(
        "duckdb engine: INSERT OR {IGNORE,REPLACE,UPDATE} is not supported");
  }

  const auto* target_table =
      dynamic_cast<const catalog::StorageTable*>(insert.table_scan()->table());
  if (target_table == nullptr) {
    return absl::FailedPreconditionError(
        "DuckDbExecutor::ExecuteDml: INSERT target is not backed by "
        "StorageTable");
  }
  InsertSelectTarget out;
  out.table = target_table;
  out.table_id = target_table->storage_table_id();
  out.schema = &target_table->bq_schema();
  return out;
}

absl::StatusOr<std::string> PrepareInsertSelectSql(
    const QueryRequest& request,
    const ::googlesql::ResolvedInsertStmt& insert,
    internal::TableScanCollector* collector) {
  absl::Status visit_status = insert.query()->Accept(collector);
  if (!visit_status.ok()) return visit_status;

  transpiler::Transpiler transpiler;
  std::string select_sql = transpiler.EmitInsertSelect(&insert);
  if (select_sql.empty()) {
    return absl::UnimplementedError(
        "duckdb engine: INSERT ... SELECT query did not transpile to DuckDB "
        "SQL");
  }
  if (!transpiler.parameter_order().empty()) {
    absl::StatusOr<std::string> substituted =
        internal::SubstituteDuckdbParameters(std::move(select_sql),
                                             transpiler.parameter_order(),
                                             request.parameters);
    if (!substituted.ok()) return substituted.status();
    select_sql = *std::move(substituted);
  }
  return select_sql;
}

absl::StatusOr<InsertSelectConnection> OpenInsertSelectConnection() {
  InsertSelectConnection out;
  if (::duckdb_open(nullptr, &out.db) != ::DuckDBSuccess) {
    return absl::InternalError(
        "DuckDbExecutor::ExecuteDml: duckdb_open(in-memory) failed");
  }
  if (::duckdb_connect(out.db, &out.conn) != ::DuckDBSuccess) {
    ::duckdb_close(&out.db);
    out.db = nullptr;
    return absl::InternalError(
        "DuckDbExecutor::ExecuteDml: duckdb_connect failed");
  }
  if (auto reg = udf::RegisterAll(out.conn); !reg.ok()) {
    ::duckdb_disconnect(&out.conn);
    out.conn = nullptr;
    ::duckdb_close(&out.db);
    out.db = nullptr;
    return reg;
  }
  return out;
}

absl::Status AttachInsertSelectSourceTables(
    ::duckdb_connection conn,
    storage::Storage* storage,
    const internal::TableScanCollector& collector) {
  for (const ::googlesql::Table* tbl : collector.tables()) {
    if (tbl == nullptr) {
      return absl::FailedPreconditionError(
          "DuckDbExecutor::ExecuteDml: null table reference in INSERT ... "
          "SELECT scan");
    }
    const auto* storage_table = dynamic_cast<const catalog::StorageTable*>(tbl);
    if (storage_table == nullptr) {
      return absl::FailedPreconditionError(absl::StrCat(
          "DuckDbExecutor::ExecuteDml: cannot attach non-StorageTable '",
          tbl->Name(),
          "' for INSERT ... SELECT"));
    }
    absl::Status attach =
        internal::AttachStorageTable(conn, storage, *storage_table);
    if (!attach.ok()) return attach;
  }
  return absl::OkStatus();
}

absl::StatusOr<std::vector<storage::Row>> ExecuteInsertSelectQuery(
    InsertSelectConnection* connection,
    absl::string_view select_sql,
    const ::googlesql::ResolvedInsertStmt& insert) {
  ::duckdb_result result;
  if (::duckdb_query(connection->conn, select_sql.data(), &result) !=
      ::DuckDBSuccess) {
    const char* err = nullptr;
    err = ::duckdb_result_error(&result);
    std::string detail = err == nullptr ? std::string("") : std::string(err);
    ::duckdb_destroy_result(&result);
    return absl::InvalidArgumentError(
        absl::StrCat("DuckDBEngine: DuckDB rejected INSERT ... SELECT source: ",
                     detail,
                     " (sql=",
                     select_sql,
                     ")"));
  }

  absl::StatusOr<schema::TableSchema> drain_schema =
      InsertSelectDrainSchema(insert);
  if (!drain_schema.ok()) {
    ::duckdb_destroy_result(&result);
    return drain_schema.status();
  }
  absl::StatusOr<std::vector<storage::Row>> selected =
      internal::DrainResultToRows(&result, *drain_schema);
  ::duckdb_destroy_result(&result);
  return selected;
}

absl::StatusOr<std::vector<storage::Row>> RemapInsertSelectRows(
    const ::googlesql::ResolvedInsertStmt& insert,
    const schema::TableSchema& full_schema,
    const std::vector<storage::Row>& selected) {
  std::vector<int> target_column_indices;
  target_column_indices.reserve(insert.insert_column_list_size());
  for (int i = 0; i < insert.insert_column_list_size(); ++i) {
    const std::string& name = insert.insert_column_list(i).name();
    int idx = -1;
    for (size_t c = 0; c < full_schema.columns.size(); ++c) {
      if (full_schema.columns[c].name == name) {
        idx = static_cast<int>(c);
        break;
      }
    }
    if (idx < 0) {
      return absl::InternalError(
          absl::StrCat("DuckDbExecutor::ExecuteDml: INSERT column '",
                       name,
                       "' not found in target schema"));
    }
    target_column_indices.push_back(idx);
  }

  if (insert.query_output_column_list_size() !=
      insert.insert_column_list_size()) {
    return absl::InternalError(
        "DuckDbExecutor::ExecuteDml: INSERT column list size does not match "
        "SELECT output column list size");
  }

  std::vector<storage::Row> rows;
  rows.reserve(selected.size());
  for (const storage::Row& src : selected) {
    if (src.cells.size() != target_column_indices.size()) {
      return absl::InternalError(
          "DuckDbExecutor::ExecuteDml: INSERT ... SELECT row width mismatch");
    }
    storage::Row out;
    out.cells.assign(full_schema.columns.size(), storage::Value::Null());
    for (size_t i = 0; i < src.cells.size(); ++i) {
      out.cells[target_column_indices[i]] = src.cells[i];
    }
    rows.push_back(std::move(out));
  }
  return rows;
}

}  // namespace internal
}  // namespace duckdb
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
