#include "backend/engine/control/control_op_executor.h"

#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/str_replace.h"
#include "absl/strings/string_view.h"
#include "absl/types/span.h"
#include "backend/catalog/storage_table.h"
#include "backend/engine/duckdb/arrow_to_bq.h"
#include "backend/engine/duckdb/udf/registrar.h"
#include "backend/engine/engine.h"
#include "backend/schema/googlesql_to_bq.h"
#include "backend/schema/schema.h"
#include "backend/storage/storage.h"
#include "duckdb.h"
#include "googlesql/public/catalog.h"
#include "googlesql/resolved_ast/resolved_ast.h"
#include "googlesql/resolved_ast/resolved_ast_visitor.h"
#include "googlesql/resolved_ast/resolved_node_kind.pb.h"
#include "proto/emulator.pb.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace control {

namespace {

// Returns the libduckdb C-API version. Calling it once at executor
// construction pulls libduckdb's symbol table into the link line so
// the binary's DT_NEEDED list keeps libduckdb.so even under
// `--as-needed`. Mirrors the same anchor `DuckDbExecutor` carries.
const char* DuckDBLibraryVersion() {
  return ::duckdb_library_version();
}

// --- DuckDB SQL literal rendering -----------------------------------------
//
// Mirrors the helpers in `backend/engine/duckdb/duckdb_executor.cc`
// (and `backend/storage/duckdb/duckdb_storage.cc`) for the
// `Value -> DuckDB-literal` job CTAS uses when streaming source rows
// into a per-query DuckDB connection. The two copies exist because
// the storage layer renders literals for INSERT against a Parquet-
// backed table while the engine renders literals for INSERT against
// a per-query in-memory DuckDB table; consolidating into a shared
// helper is on a follow-up plan.

std::string QuoteIdent(absl::string_view ident) {
  std::string escaped = absl::StrReplaceAll(ident, {{"\"", "\"\""}});
  return absl::StrCat("\"", escaped, "\"");
}

std::string EscapeStringLiteralInner(absl::string_view s) {
  return absl::StrReplaceAll(s, {{"'", "''"}});
}

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

absl::StatusOr<std::string> RenderCellLiteral(
    const storage::Value& cell, const schema::ColumnSchema& column);

std::string RenderFloatLiteral(const storage::Value& cell) {
  if (cell.kind() == storage::Value::Kind::kString) {
    return absl::StrCat("CAST('",
                        EscapeStringLiteralInner(cell.string_value()),
                        "' AS DOUBLE)");
  }
  const double v = cell.float64_value();
  if (std::isnan(v)) return std::string("'NaN'::DOUBLE");
  if (std::isinf(v)) {
    return std::string(v > 0 ? "'Infinity'::DOUBLE" : "'-Infinity'::DOUBLE");
  }
  return absl::StrCat(v);
}

absl::StatusOr<std::string> RenderStructLiteral(
    const storage::Value& cell, const schema::ColumnSchema& column) {
  if (cell.kind() != storage::Value::Kind::kStruct) {
    return absl::InvalidArgumentError(
        absl::StrCat("ControlOpExecutor: column '",
                     column.name,
                     "' expects STRUCT but row provided non-struct cell"));
  }
  const auto& fields = cell.struct_value();
  if (fields.size() != column.fields.size()) {
    return absl::InvalidArgumentError(
        absl::StrCat("ControlOpExecutor: STRUCT column '",
                     column.name,
                     "' has ",
                     column.fields.size(),
                     " fields but row provided ",
                     fields.size()));
  }
  std::string out = "{";
  for (size_t i = 0; i < fields.size(); ++i) {
    if (i > 0) absl::StrAppend(&out, ", ");
    absl::StrAppend(
        &out, "'", EscapeStringLiteralInner(column.fields[i].name), "': ");
    auto inner_or = RenderCellLiteral(fields[i], column.fields[i]);
    if (!inner_or.ok()) return inner_or.status();
    absl::StrAppend(&out, *inner_or);
  }
  absl::StrAppend(&out, "}");
  return out;
}

absl::StatusOr<std::string> RenderScalarLiteral(
    const storage::Value& cell, const schema::ColumnSchema& column) {
  switch (column.type) {
    case schema::ColumnType::kBool:
      if (cell.kind() == storage::Value::Kind::kString) {
        return absl::StrCat("CAST('",
                            EscapeStringLiteralInner(cell.string_value()),
                            "' AS BOOLEAN)");
      }
      return std::string(cell.bool_value() ? "TRUE" : "FALSE");
    case schema::ColumnType::kInt64:
      if (cell.kind() == storage::Value::Kind::kString) {
        return absl::StrCat("CAST('",
                            EscapeStringLiteralInner(cell.string_value()),
                            "' AS BIGINT)");
      }
      return absl::StrCat(cell.int64_value());
    case schema::ColumnType::kFloat64:
      return RenderFloatLiteral(cell);
    case schema::ColumnType::kString:
    case schema::ColumnType::kJson:
    case schema::ColumnType::kGeography:
      return absl::StrCat(
          "'", EscapeStringLiteralInner(cell.string_value()), "'");
    case schema::ColumnType::kBytes:
      return RenderBlobLiteral(cell.string_value());
    case schema::ColumnType::kDate:
      return absl::StrCat(
          "DATE '", EscapeStringLiteralInner(cell.string_value()), "'");
    case schema::ColumnType::kTime:
      return absl::StrCat(
          "TIME '", EscapeStringLiteralInner(cell.string_value()), "'");
    case schema::ColumnType::kDatetime:
      return absl::StrCat(
          "TIMESTAMP '", EscapeStringLiteralInner(cell.string_value()), "'");
    case schema::ColumnType::kTimestamp:
      return absl::StrCat(
          "TIMESTAMPTZ '", EscapeStringLiteralInner(cell.string_value()), "'");
    case schema::ColumnType::kNumeric:
    case schema::ColumnType::kBignumeric:
      return absl::StrCat("CAST('",
                          EscapeStringLiteralInner(cell.string_value()),
                          "' AS ",
                          schema::ToDuckDBType(column.type),
                          ")");
    case schema::ColumnType::kStruct:
      return RenderStructLiteral(cell, column);
    case schema::ColumnType::kArray:
    case schema::ColumnType::kUnknown:
      return absl::StrCat(
          "'", EscapeStringLiteralInner(cell.string_value()), "'");
  }
  return absl::InternalError("RenderScalarLiteral: unreachable");
}

absl::StatusOr<std::string> RenderCellLiteral(
    const storage::Value& cell, const schema::ColumnSchema& column) {
  if (cell.is_null()) return std::string("NULL");
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

std::string RenderColumnList(const schema::TableSchema& schema) {
  std::string out = "(";
  for (size_t i = 0; i < schema.columns.size(); ++i) {
    if (i > 0) absl::StrAppend(&out, ", ");
    absl::StrAppend(&out,
                    QuoteIdent(schema.columns[i].name),
                    " ",
                    schema::ColumnSchemaToDuckDBType(schema.columns[i]));
  }
  absl::StrAppend(&out, ")");
  return out;
}

// Walks the resolved AST and collects every distinct
// `ResolvedTableScan::table()` pointer. Mirrors
// `DuckDbExecutor`'s `TableScanCollector`. CTAS uses this to
// materialize source tables into the per-query DuckDB connection
// before lowering the user-submitted DDL onto DuckDB.
class TableScanCollector : public ::googlesql::ResolvedASTVisitor {
 public:
  absl::Status VisitResolvedTableScan(
      const ::googlesql::ResolvedTableScan* node) override {
    if (node == nullptr) return absl::OkStatus();
    if (node->table() != nullptr) {
      tables_.insert(node->table());
    }
    return ::googlesql::ResolvedASTVisitor::VisitResolvedTableScan(node);
  }

  const std::set<const ::googlesql::Table*>& tables() const {
    return tables_;
  }

 private:
  std::set<const ::googlesql::Table*> tables_{};
};

// Runs `sql` on `conn`; returns OK or INTERNAL with the DuckDB
// error message attached.
absl::Status RunSqlNoResult(::duckdb_connection conn, absl::string_view sql) {
  ::duckdb_result result;
  const std::string sql_str(sql);
  const auto state = ::duckdb_query(conn, sql_str.c_str(), &result);
  if (state != ::DuckDBSuccess) {
    const auto* err = ::duckdb_result_error(&result);
    std::string detail = err == nullptr ? std::string("") : std::string(err);
    ::duckdb_destroy_result(&result);
    return absl::InternalError(absl::StrCat(
        "ControlOpExecutor: query failed: ", sql_str, ": ", detail));
  }
  ::duckdb_destroy_result(&result);
  return absl::OkStatus();
}

// Materialize one storage table inside `conn` at the given quoted
// table name. Mirrors `DuckDbExecutor::AttachStorageTableAt` for
// CTAS.
absl::Status AttachStorageTableAt(::duckdb_connection conn,
                                  storage::Storage* storage,
                                  const catalog::StorageTable& table,
                                  absl::string_view quoted_table_name) {
  const schema::TableSchema& schema = table.bq_schema();
  const std::string table_name(quoted_table_name);
  const std::string columns = RenderColumnList(schema);

  absl::Status status = RunSqlNoResult(
      conn, absl::StrCat("CREATE OR REPLACE TABLE ", table_name, " ", columns));
  if (!status.ok()) return status;

  absl::StatusOr<std::unique_ptr<storage::RowIterator>> iter =
      storage->ScanRows(table.storage_table_id());
  if (!iter.ok()) return iter.status();

  std::unique_ptr<storage::RowIterator> rows_iter = std::move(iter).value();
  std::vector<storage::Row> rows;
  storage::Row row;
  while (true) {
    absl::StatusOr<bool> has = rows_iter->Next(&row);
    if (!has.ok()) return has.status();
    if (!*has) break;
    rows.push_back(row);
  }
  if (rows.empty()) return absl::OkStatus();

  std::string insert_sql;
  absl::StrAppend(&insert_sql, "INSERT INTO ", table_name, " VALUES ");
  const size_t ncols = schema.columns.size();
  for (size_t r = 0; r < rows.size(); ++r) {
    if (rows[r].cells.size() != ncols) {
      return absl::InvalidArgumentError(absl::StrCat("ControlOpExecutor: row[",
                                                     r,
                                                     "] has ",
                                                     rows[r].cells.size(),
                                                     " cells but table '",
                                                     table.Name(),
                                                     "' has ",
                                                     ncols,
                                                     " columns"));
    }
    if (r > 0) absl::StrAppend(&insert_sql, ", ");
    absl::StrAppend(&insert_sql, "(");
    for (size_t c = 0; c < ncols; ++c) {
      if (c > 0) absl::StrAppend(&insert_sql, ", ");
      auto cell_or = RenderCellLiteral(rows[r].cells[c], schema.columns[c]);
      if (!cell_or.ok()) return cell_or.status();
      absl::StrAppend(&insert_sql, *cell_or);
    }
    absl::StrAppend(&insert_sql, ")");
  }
  return RunSqlNoResult(conn, insert_sql);
}

// Drain every row out of `quoted_table_name` in `conn` and return
// them in the engine-agnostic `storage::Row` shape that matches
// `bq_schema`.
absl::StatusOr<std::vector<storage::Row>> DrainTableRows(
    ::duckdb_connection conn,
    absl::string_view quoted_table_name,
    const schema::TableSchema& bq_schema) {
  const std::string sql = absl::StrCat("SELECT * FROM ", quoted_table_name);
  ::duckdb_result result;
  if (::duckdb_query(conn, sql.c_str(), &result) != ::DuckDBSuccess) {
    const auto* err = ::duckdb_result_error(&result);
    std::string detail = err == nullptr ? std::string("") : std::string(err);
    ::duckdb_destroy_result(&result);
    return absl::InternalError(
        absl::StrCat("ControlOpExecutor: failed to read back DDL target ",
                     sql,
                     ": ",
                     detail));
  }
  std::vector<storage::Row> rows;
  while (true) {
    ::duckdb_data_chunk chunk = ::duckdb_fetch_chunk(result);
    if (chunk == nullptr) break;
    const ::idx_t n = ::duckdb_data_chunk_get_size(chunk);
    for (::idx_t i = 0; i < n; ++i) {
      absl::StatusOr<storage::Row> rendered =
          duckdb::arrow_to_bq::ChunkRowToCells(chunk, i, bq_schema);
      if (!rendered.ok()) {
        ::duckdb_destroy_data_chunk(&chunk);
        ::duckdb_destroy_result(&result);
        return rendered.status();
      }
      rows.push_back(std::move(rendered).value());
    }
    ::duckdb_destroy_data_chunk(&chunk);
  }
  ::duckdb_destroy_result(&result);
  return rows;
}

// --- Name-path resolution -------------------------------------------------

// Resolve `name_path` (as produced by `ResolvedCreateStatement::
// name_path()` / `ResolvedDropStmt::name_path()` etc.) to a
// `storage::TableId` under `default_project_id`. Two-segment paths
// default the project, three-segment paths override it.
absl::StatusOr<storage::TableId> NamePathToTableId(
    const std::vector<std::string>& name_path,
    absl::string_view default_project_id) {
  if (name_path.size() == 2) {
    return storage::TableId{
        std::string(default_project_id), name_path[0], name_path[1]};
  }
  if (name_path.size() == 3) {
    return storage::TableId{name_path[0], name_path[1], name_path[2]};
  }
  return absl::InvalidArgumentError(absl::StrCat(
      "control op executor: DDL target name must be <dataset>.<table> or "
      "<project>.<dataset>.<table>; got ",
      name_path.size(),
      " segments"));
}

// --- Schema helpers -------------------------------------------------------

void ApplyAnnotations(const ::googlesql::ResolvedColumnDefinition* def,
                      v1::FieldSchema* field) {
  if (def == nullptr) return;
  if (field->mode() == "REPEATED") return;
  const ::googlesql::ResolvedColumnAnnotations* ann = def->annotations();
  if (ann != nullptr && ann->not_null()) {
    field->set_mode("REQUIRED");
  }
}

absl::StatusOr<schema::TableSchema> ColumnDefinitionListToTableSchema(
    const std::vector<
        std::unique_ptr<const ::googlesql::ResolvedColumnDefinition>>&
        column_definition_list) {
  v1::TableSchema proto;
  for (const auto& def : column_definition_list) {
    if (def == nullptr) {
      return absl::InvalidArgumentError(
          "control op executor: column_definition_list has a null entry");
    }
    v1::FieldSchema* field = proto.add_fields();
    absl::Status s =
        backend::schema::TypeToFieldSchema(def->type(), def->name(), field);
    if (!s.ok()) return s;
    ApplyAnnotations(def.get(), field);
  }
  return backend::schema::TableSchemaFromProto(proto);
}

// Apply CREATE-mode semantics on top of a `Storage::CreateTable`
// status. CREATE_IF_NOT_EXISTS swallows ALREADY_EXISTS; the other
// modes propagate it.
absl::Status ApplyCreateMode(
    absl::Status existing_status,
    ::googlesql::ResolvedCreateStatement::CreateMode create_mode) {
  if (existing_status.ok()) return absl::OkStatus();
  if (create_mode ==
          ::googlesql::ResolvedCreateStatement::CREATE_IF_NOT_EXISTS &&
      existing_status.code() == absl::StatusCode::kAlreadyExists) {
    return absl::OkStatus();
  }
  return existing_status;
}

// --- CREATE TABLE handler -------------------------------------------------
//
// statementType: `CREATE_TABLE` per BigQuery REST documentation
// (`Job.statistics.query.statementType`).
absl::Status RunCreateTable(storage::Storage& storage,
                            absl::string_view project_id,
                            const ::googlesql::ResolvedCreateTableStmt* stmt) {
  if (stmt == nullptr) {
    return absl::InternalError(
        "ControlOpExecutor::ExecuteDdl: CREATE TABLE has null resolved "
        "statement");
  }
  absl::StatusOr<storage::TableId> target =
      NamePathToTableId(stmt->name_path(), project_id);
  if (!target.ok()) return target.status();
  absl::StatusOr<schema::TableSchema> bq_schema =
      ColumnDefinitionListToTableSchema(stmt->column_definition_list());
  if (!bq_schema.ok()) return bq_schema.status();

  if (stmt->create_mode() ==
      ::googlesql::ResolvedCreateStatement::CREATE_OR_REPLACE) {
    absl::Status dropped = storage.DropTable(*target);
    if (!dropped.ok() && dropped.code() != absl::StatusCode::kNotFound) {
      return dropped;
    }
  }
  return ApplyCreateMode(storage.CreateTable(*target, *bq_schema),
                         stmt->create_mode());
}

// --- CREATE TABLE AS SELECT handler ---------------------------------------
//
// statementType: `CREATE_TABLE_AS_SELECT`. The handler materializes
// each referenced source table inside a per-query in-memory DuckDB
// connection, runs the user-submitted SQL verbatim, then reads back
// the resulting rows under the BigQuery-typed schema (so storage
// records the BigQuery view of the schema, not DuckDB's inferred
// types). This is the same migration-compatible path the prior
// `DuckDbExecutor::ExecuteDdl::RunCreateTableAsSelect` carried -- the
// "run SELECT half through the coordinator" refactor the plan
// envisions is deferred to a follow-up because it requires injecting
// a `coordinator::Engine*` back into this executor (circular
// dependency with `LocalCoordinatorEngine`).
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
  absl::StatusOr<storage::TableId> target =
      NamePathToTableId(stmt->name_path(), project_id);
  if (!target.ok()) return target.status();
  absl::StatusOr<schema::TableSchema> bq_schema =
      ColumnDefinitionListToTableSchema(stmt->column_definition_list());
  if (!bq_schema.ok()) return bq_schema.status();

  TableScanCollector collector;
  absl::Status visit_status = root_stmt->Accept(&collector);
  if (!visit_status.ok()) return visit_status;

  ::duckdb_database db = nullptr;
  if (::duckdb_open(nullptr, &db) != ::DuckDBSuccess) {
    return absl::InternalError(
        "ControlOpExecutor::ExecuteDdl: duckdb_open(in-memory) failed");
  }
  ::duckdb_connection conn = nullptr;
  if (::duckdb_connect(db, &conn) != ::DuckDBSuccess) {
    ::duckdb_close(&db);
    return absl::InternalError(
        "ControlOpExecutor::ExecuteDdl: duckdb_connect failed");
  }
  if (auto reg = duckdb::udf::RegisterAll(conn); !reg.ok()) {
    ::duckdb_disconnect(&conn);
    ::duckdb_close(&db);
    return reg;
  }

  absl::Status target_schema_status =
      RunSqlNoResult(conn,
                     absl::StrCat("CREATE SCHEMA IF NOT EXISTS ",
                                  QuoteIdent(target->dataset_id)));
  if (!target_schema_status.ok()) {
    ::duckdb_disconnect(&conn);
    ::duckdb_close(&db);
    return target_schema_status;
  }

  for (const ::googlesql::Table* tbl : collector.tables()) {
    const auto* source_table = dynamic_cast<const catalog::StorageTable*>(tbl);
    if (source_table == nullptr) {
      ::duckdb_disconnect(&conn);
      ::duckdb_close(&db);
      return absl::FailedPreconditionError(absl::StrCat(
          "ControlOpExecutor::ExecuteDdl: cannot attach non-StorageTable '",
          tbl->Name(),
          "' for CTAS; rebuild against a "
          "GoogleSqlCatalog-backed analyzer"));
    }
    const storage::TableId& id = source_table->storage_table_id();
    absl::Status schema_status =
        RunSqlNoResult(conn,
                       absl::StrCat("CREATE SCHEMA IF NOT EXISTS ",
                                    QuoteIdent(id.dataset_id)));
    if (!schema_status.ok()) {
      ::duckdb_disconnect(&conn);
      ::duckdb_close(&db);
      return schema_status;
    }
    const std::string qualified =
        absl::StrCat(QuoteIdent(id.dataset_id), ".", QuoteIdent(id.table_id));
    absl::Status attach =
        AttachStorageTableAt(conn, &storage, *source_table, qualified);
    if (!attach.ok()) {
      ::duckdb_disconnect(&conn);
      ::duckdb_close(&db);
      return attach;
    }
  }

  if (stmt->name_path_size() == 3) {
    ::duckdb_disconnect(&conn);
    ::duckdb_close(&db);
    return absl::UnimplementedError(
        absl::StrCat("control op executor: CTAS with a project-qualified "
                     "target ('",
                     stmt->name_path(0),
                     ".",
                     stmt->name_path(1),
                     ".",
                     stmt->name_path(2),
                     "') is not yet implemented; drop the project segment "
                     "(the BigQuery REST `projectId` path parameter already "
                     "carries it) and re-run with a 2-segment target"));
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

  ::duckdb_result ctas_result;
  if (::duckdb_query(conn, request.sql.c_str(), &ctas_result) !=
      ::DuckDBSuccess) {
    const auto* err = ::duckdb_result_error(&ctas_result);
    std::string detail = err == nullptr ? std::string("") : std::string(err);
    ::duckdb_destroy_result(&ctas_result);
    ::duckdb_disconnect(&conn);
    ::duckdb_close(&db);
    return absl::InvalidArgumentError(
        absl::StrCat("ControlOpExecutor: DuckDB rejected CTAS: ",
                     detail,
                     " (sql=",
                     request.sql,
                     ")"));
  }
  ::duckdb_destroy_result(&ctas_result);

  const std::string quoted_target = absl::StrCat(
      QuoteIdent(target->dataset_id), ".", QuoteIdent(target->table_id));
  absl::StatusOr<std::vector<storage::Row>> after_rows =
      DrainTableRows(conn, quoted_target, *bq_schema);
  ::duckdb_disconnect(&conn);
  ::duckdb_close(&db);
  if (!after_rows.ok()) return after_rows.status();

  absl::Status created = ApplyCreateMode(
      storage.CreateTable(*target, *bq_schema), stmt->create_mode());
  if (!created.ok()) return created;
  if (after_rows->empty()) return absl::OkStatus();
  return storage.AppendRows(*target, absl::MakeConstSpan(*after_rows));
}

// --- DROP TABLE handler ---------------------------------------------------
//
// statementType: `DROP_TABLE`. Currently only `DROP TABLE`; other
// `DROP <kind>` forms surface UNIMPLEMENTED. View / materialized-
// view drops belong here once view storage exists (today the
// `Storage` interface has no view-CRUD surface).
absl::Status RunDropTable(storage::Storage& storage,
                          absl::string_view project_id,
                          const ::googlesql::ResolvedDropStmt* stmt) {
  if (stmt == nullptr) {
    return absl::InternalError(
        "ControlOpExecutor::ExecuteDdl: DROP has null resolved statement");
  }
  if (stmt->object_type() != "TABLE") {
    return absl::UnimplementedError(absl::StrCat(
        "control op executor: DROP ",
        stmt->object_type(),
        " is not implemented yet (only DROP TABLE is supported); the "
        "view / materialized-view drop paths land alongside the storage-"
        "side view-CRUD surface tracked by the same plan "
        "(control-op-executor.plan.md)"));
  }
  absl::StatusOr<storage::TableId> target =
      NamePathToTableId(stmt->name_path(), project_id);
  if (!target.ok()) return target.status();
  absl::Status dropped = storage.DropTable(*target);
  if (!dropped.ok() && stmt->is_if_exists() &&
      dropped.code() == absl::StatusCode::kNotFound) {
    return absl::OkStatus();
  }
  return dropped;
}

// --- ANALYZE handler ------------------------------------------------------
//
// statementType: `ANALYZE`. The emulator's storage layer does not
// keep optimizer statistics, so ANALYZE is a no-op metadata refresh:
// we validate that every named table exists (NOT_FOUND otherwise)
// and return OK. Callers see the operation succeed without any
// observable side-effect, which matches the BigQuery posture where
// `ANALYZE` is a metadata hint the query optimizer is free to
// ignore. The deeper "actually compute and persist statistics" path
// is tracked by `specialized-feature-policy.plan.md`.
absl::Status RunAnalyze(storage::Storage& storage,
                        absl::string_view project_id,
                        const ::googlesql::ResolvedAnalyzeStmt* stmt) {
  if (stmt == nullptr) {
    return absl::InternalError(
        "ControlOpExecutor::ExecuteDdl: ANALYZE has null resolved statement");
  }
  for (const auto& target : stmt->table_and_column_index_list()) {
    if (target == nullptr || target->table() == nullptr) continue;
    const auto* storage_table =
        dynamic_cast<const catalog::StorageTable*>(target->table());
    if (storage_table == nullptr) continue;
    const storage::TableId& id = storage_table->storage_table_id();
    auto schema_or = storage.GetSchema(id);
    if (!schema_or.ok()) return schema_or.status();
  }
  (void)project_id;
  return absl::OkStatus();
}

}  // namespace

ControlOpExecutor::ControlOpExecutor(storage::Storage* storage)
    : storage_(storage) {
  (void)DuckDBLibraryVersion();
}

ControlOpExecutor::~ControlOpExecutor() = default;

absl::StatusOr<std::unique_ptr<RowSource>> ControlOpExecutor::ExecuteQuery(
    const QueryRequest& request,
    const ::googlesql::ResolvedStatement& stmt,
    ::googlesql::Catalog* catalog) {
  (void)request;
  (void)catalog;
  return absl::InvalidArgumentError(absl::StrCat(
      "ControlOpExecutor::ExecuteQuery: control-op statements never produce "
      "a row stream; coordinator routed ",
      stmt.node_kind_string(),
      " through the wrong dispatch surface (control_op statements use "
      "ExecuteDdl)"));
}

absl::StatusOr<DmlStats> ControlOpExecutor::ExecuteDml(
    const QueryRequest& request,
    const ::googlesql::ResolvedStatement& stmt,
    ::googlesql::Catalog* catalog) {
  (void)request;
  (void)catalog;
  return absl::InvalidArgumentError(absl::StrCat(
      "ControlOpExecutor::ExecuteDml: control-op statements never produce a "
      "DML stats summary; coordinator routed ",
      stmt.node_kind_string(),
      " through the wrong dispatch surface (control_op statements use "
      "ExecuteDdl)"));
}

absl::Status ControlOpExecutor::ExecuteDdl(
    const QueryRequest& request,
    const ::googlesql::ResolvedStatement& stmt,
    ::googlesql::Catalog* catalog) {
  (void)catalog;  // analysis already happened on the coordinator.
  if (storage_ == nullptr) {
    return absl::FailedPreconditionError(
        "ControlOpExecutor::ExecuteDdl: storage backend is not configured");
  }

  const absl::string_view project_id = request.project_id;
  switch (stmt.node_kind()) {
    case ::googlesql::RESOLVED_CREATE_TABLE_STMT:
      return RunCreateTable(*storage_,
                            project_id,
                            stmt.GetAs<::googlesql::ResolvedCreateTableStmt>());
    case ::googlesql::RESOLVED_CREATE_TABLE_AS_SELECT_STMT:
      return RunCreateTableAsSelect(
          *storage_,
          project_id,
          request,
          stmt.GetAs<::googlesql::ResolvedCreateTableAsSelectStmt>(),
          &stmt);
    case ::googlesql::RESOLVED_DROP_STMT:
      return RunDropTable(
          *storage_, project_id, stmt.GetAs<::googlesql::ResolvedDropStmt>());
    case ::googlesql::RESOLVED_ANALYZE_STMT:
      return RunAnalyze(*storage_,
                        project_id,
                        stmt.GetAs<::googlesql::ResolvedAnalyzeStmt>());

    // Deferred control-op shapes. Each handler returns
    // UNIMPLEMENTED with a focused message that names the missing
    // infrastructure plus the plan owning the follow-up work. The
    // classifier still routes these statements here (so a future
    // implementation lands in this file, not the DuckDB SQL
    // evaluator); the route stays observable for the conformance
    // routing matrix in `conformance-routing-matrix.plan.md`.
    case ::googlesql::RESOLVED_CREATE_VIEW_STMT:
      return absl::UnimplementedError(
          "control op executor: CREATE VIEW is not implemented yet; the "
          "Storage interface has no view-CRUD surface today (see "
          "backend/storage/storage.h). Tracked by "
          "control-op-executor.plan.md follow-up: 'add Storage view "
          "CRUD + handler'.");
    case ::googlesql::RESOLVED_CREATE_MATERIALIZED_VIEW_STMT:
      return absl::UnimplementedError(
          "control op executor: CREATE MATERIALIZED VIEW is not "
          "implemented yet; the metadata side requires the same view-"
          "CRUD surface CREATE VIEW needs, and the full-refresh "
          "execution behavior is owned by "
          "specialized-feature-policy.plan.md per "
          "control-op-executor.plan.md (better-than-silently-wrong).");
    case ::googlesql::RESOLVED_CREATE_FUNCTION_STMT:
      // CREATE FUNCTION splits by `LANGUAGE` at implementation
      // time. SQL UDFs (`LANGUAGE SQL`) belong on
      // `udf-tvf-module-routing.plan.md` Family 4 (deferred at the
      // end of plan 13 -- needs cross-request UDF body storage
      // through `DuckDBStorage`'s catalog). JavaScript UDFs
      // (`LANGUAGE js`) are `local_stub`-posture metadata-only per
      // `specialized-feature-policy.plan.md`: the registration
      // succeeds (BigQuery-shaped placeholder for client tools
      // that issue CREATE FUNCTION as a setup step) and the call
      // site surfaces UNIMPLEMENTED with the JavaScript-named
      // family envelope. The JS UDF stub IS DEFERRED until the
      // UDF body storage round-trip lands per plan 13's family-4
      // deferral -- a registration that does not persist the body
      // is silent approximation in the "did the catalog accept the
      // create" sense. Today both flavors share this UNIMPLEMENTED
      // envelope; the splits land alongside the storage work. See
      // docs/ENGINE_POLICY.md.
      return absl::UnimplementedError(
          "control op executor: CREATE FUNCTION registration is not "
          "implemented yet; needs a per-engine functions registry. "
          "SQL UDFs tracked by udf-tvf-module-routing.plan.md "
          "(family 4, deferred -- needs UDF body storage). JavaScript "
          "UDFs (LANGUAGE js) are local-stub metadata-only per "
          "specialized-feature-policy.plan.md but their stub is "
          "BLOCKED on the same UDF body storage round-trip; "
          "registering a body that does not persist would silently "
          "approximate the BigQuery contract. See "
          "docs/ENGINE_POLICY.md.");
    case ::googlesql::RESOLVED_CREATE_TABLE_FUNCTION_STMT:
      return absl::UnimplementedError(
          "control op executor: CREATE TABLE FUNCTION registration is "
          "not implemented yet; tracked by "
          "udf-tvf-module-routing.plan.md.");
    case ::googlesql::RESOLVED_DROP_FUNCTION_STMT:
      return absl::UnimplementedError(
          "control op executor: DROP FUNCTION is not implemented yet; "
          "needs the functions registry CREATE FUNCTION will land. "
          "Tracked by udf-tvf-module-routing.plan.md (and "
          "control-op-executor.plan.md for the metadata side).");
    case ::googlesql::RESOLVED_AUX_LOAD_DATA_STMT:
      // LOAD DATA splits by URI scheme at implementation time:
      // `LOAD DATA LOCAL ...` belongs on the control-op route
      // (local filesystem reader; deferred to the follow-up below).
      // `LOAD DATA <gs://...>` (cloud-storage) is `unsupported`
      // per `specialized-feature-policy.plan.md` -- the emulator
      // deliberately does NOT model the BigQuery cloud-storage
      // ingest surface. `ResolvedAuxLoadDataStmt` carries no
      // `is_local` flag; the differentiation happens when the
      // reader inspects the URI. Today both shapes share this
      // UNIMPLEMENTED envelope; when the LOCAL reader family
      // lands, the cloud-storage URIs will surface the
      // unsupported-family envelope from the unsupported stub
      // executor instead. See docs/ENGINE_POLICY.md.
      return absl::UnimplementedError(
          "control op executor: LOAD DATA is not implemented yet; "
          "needs source-file readers (CSV / JSON / Avro / Parquet / "
          "ORC) for `LOAD DATA LOCAL <local-uri>` plus URI-scheme "
          "differentiation so cloud-storage `LOAD DATA <gs://...>` "
          "falls through to the unsupported route. "
          "Tracked by control-op-executor.plan.md follow-up: 'add "
          "LOAD DATA reader family'. Cloud-storage LOAD DATA stays "
          "unsupported per specialized-feature-policy.plan.md; see "
          "docs/ENGINE_POLICY.md.");
    case ::googlesql::RESOLVED_EXPORT_DATA_STMT:
      return absl::UnimplementedError(
          "control op executor: EXPORT DATA is not implemented yet; "
          "needs Arrow / Parquet / CSV / JSON writers and the "
          "fake-gcs-server / local-filesystem URI scheme dispatch. "
          "Tracked by control-op-executor.plan.md follow-up: 'add "
          "EXPORT DATA writer family'.");
    default:
      return absl::UnimplementedError(
          absl::StrCat("control op executor: ExecuteDdl does not implement ",
                       stmt.node_kind_string(),
                       " yet; the row carries disposition=control_op in "
                       "node_dispositions.yaml but no handler in "
                       "control_op_executor.cc dispatches it"));
  }
}

}  // namespace control
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
