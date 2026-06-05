#include "backend/engine/duckdb/duckdb_executor.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <memory>
#include <optional>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include "absl/container/flat_hash_map.h"
#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/str_format.h"
#include "absl/strings/str_replace.h"
#include "absl/strings/string_view.h"
#include "absl/types/span.h"
#include "backend/catalog/storage_table.h"
#include "backend/catalog/virtual_table.h"
#include "backend/engine/duckdb/arrow_to_bq.h"
#include "backend/engine/duckdb/transpiler/transpiler.h"
#include "backend/engine/duckdb/udf/registrar.h"
#include "backend/engine/semantic/value.h"
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
namespace duckdb {

namespace {

// Returns the libduckdb C-API version. Calling this in the
// constructor pulls libduckdb's symbol table into the link line
// under `--as-needed` so libduckdb.so stays on the binary's
// DT_NEEDED list.
const char* DuckDBLibraryVersion() {
  return ::duckdb_library_version();
}

}  // namespace

DuckDbExecutor::DuckDbExecutor(storage::Storage* storage) : storage_(storage) {
  (void)DuckDBLibraryVersion();
}

DuckDbExecutor::~DuckDbExecutor() = default;

namespace {

// Build the BigQuery-shaped output schema from the analyzer's
// resolved-output-column list. Routed through the proto round-trip
// so the REPEATED-mode contract for ARRAY columns matches the
// surface the gateway emits.
absl::StatusOr<schema::TableSchema> ReflectOutputSchema(
    const ::googlesql::ResolvedQueryStmt& stmt) {
  v1::TableSchema proto;
  absl::Status s = backend::schema::OutputColumnListToTableSchema(
      stmt.output_column_list(), &proto);
  if (!s.ok()) return s;
  return backend::schema::TableSchemaFromProto(proto);
}

// Walks the resolved AST and collects every distinct
// `ResolvedTableScan::table()` pointer. We dedupe on identity (the
// catalog hands out one `Table*` per `<project>.<dataset>.<table>`
// path within a query) so a self-join only materializes the table
// in DuckDB once.
class TableScanCollector : public ::googlesql::ResolvedASTVisitor {
 public:
  absl::Status VisitResolvedTableScan(
      const ::googlesql::ResolvedTableScan* node) override {
    if (node == nullptr) {
      // GoogleSQL never hands the visitor a null node, but the base
      // class unconditionally dereferences `node` via DefaultVisit ->
      // ChildrenAccept, so guarding here keeps the static analyzer
      // happy and removes a real (if unreachable) NPE.
      return absl::OkStatus();
    }
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

// --- DuckDB SQL literal rendering -----------------------------------------
//
// Mirrors the helpers in `backend/storage/duckdb/duckdb_storage.cc`
// for the same Value -> DuckDB-literal job. The two copies stay in
// lockstep because the storage layer renders literals for INSERT
// statements against a Parquet-backed table and the engine renders
// literals for INSERT statements against a per-query in-memory
// DuckDB table; folding them into a shared helper is on the followup
// plan that consolidates DuckDB plumbing.

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

// Render the DuckDB literal for a FLOAT64 cell. Special-cases NaN /
// +Inf / -Inf because DuckDB cannot parse them out of a bare numeric
// literal; the typed cast lowers them through the IEEE-754 path
// instead.
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
  return absl::StrFormat("%.17g", v);
}

// Render the DuckDB STRUCT literal `{'k1': v1, ...}` for a STRUCT
// cell. Validates that the cell carries the expected struct shape
// before recursing.
absl::StatusOr<std::string> RenderStructLiteral(
    const storage::Value& cell, const schema::ColumnSchema& column) {
  if (cell.kind() != storage::Value::Kind::kStruct) {
    return absl::InvalidArgumentError(
        absl::StrCat("DuckDBEngine: column '",
                     column.name,
                     "' expects STRUCT but row provided non-struct cell"));
  }
  const auto& fields = cell.struct_value();
  if (fields.size() != column.fields.size()) {
    return absl::InvalidArgumentError(
        absl::StrCat("DuckDBEngine: STRUCT column '",
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
  // Like `duckdb_storage.cc::RenderScalarLiteral`, we accept both the
  // natively-typed Value variants (Int64, Float64, Bool) and a
  // String carrying the textual representation -- the gateway
  // lowers every JSON cell to a string regardless of the column's
  // declared type. DuckDBStorage returns natively-typed
  // values because the Parquet file enforces the schema, so the
  // string branch is a fallback for callers that hand-construct
  // Value::String cells in unit tests.
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
    // REPEATED cells lower onto DuckDB LIST literals (`[v1, v2, ...]`).
    // The element renderer goes through a synthetic non-repeated
    // column so it does not re-enter the array branch.
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

// Emit the parenthesized column-list-with-types for a CREATE TABLE
// statement against the storage table's schema.
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

// Streams rows out of a DuckDB result by fetching one columnar data
// chunk at a time -- the C-API equivalent of pulling Arrow
// RecordBatches off the result iterator. Each chunk's vectors are
// what `duckdb_data_chunk_to_arrow` exports, so this is the same
// columnar interface the Storage Read API path will stream straight
// onto the wire.
//
// Each cell is rendered through `arrow_to_bq::ChunkRowToCells` so
// the resulting `storage::Value` shape lines up with what
// `frontend/handlers/query.cc::ValueToCell` lowers onto the proto
// Cell wire shape. The chunked path replaces the previous
// row-at-a-time `duckdb_value_*` accessors so the engine no longer
// pays one C-API call per cell.
//
// Declaration order pins destruction order: chunk_ first (releases
// the columnar buffers it borrowed from result_), then result_, then
// conn_, then db_.
class DuckDBRowSource : public RowSource {
 public:
  DuckDBRowSource(::duckdb_database db,
                  ::duckdb_connection conn,
                  ::duckdb_result result,
                  schema::TableSchema schema)
      : db_(db), conn_(conn), result_(result), schema_(std::move(schema)) {}

  ~DuckDBRowSource() override {
    if (chunk_ != nullptr) ::duckdb_destroy_data_chunk(&chunk_);
    ::duckdb_destroy_result(&result_);
    if (conn_ != nullptr) ::duckdb_disconnect(&conn_);
    if (db_ != nullptr) ::duckdb_close(&db_);
  }

  DuckDBRowSource(const DuckDBRowSource&) = delete;
  DuckDBRowSource& operator=(const DuckDBRowSource&) = delete;

  const schema::TableSchema& schema() const override {
    return schema_;
  }

  absl::StatusOr<bool> Next(storage::Row* row) override {
    if (row == nullptr) {
      return absl::InvalidArgumentError(
          "DuckDBEngine row source: Next called with null row");
    }
    while (chunk_ == nullptr || next_in_chunk_ >= chunk_size_) {
      if (chunk_ != nullptr) {
        ::duckdb_destroy_data_chunk(&chunk_);
        chunk_ = nullptr;
      }
      chunk_ = ::duckdb_fetch_chunk(result_);
      if (chunk_ == nullptr) return false;
      chunk_size_ = ::duckdb_data_chunk_get_size(chunk_);
      next_in_chunk_ = 0;
      // A zero-sized chunk can show up at end-of-stream; loop to the
      // next fetch which will hand back nullptr and terminate.
    }
    absl::StatusOr<storage::Row> rendered =
        arrow_to_bq::ChunkRowToCells(chunk_, next_in_chunk_, schema_);
    if (!rendered.ok()) return rendered.status();
    *row = std::move(rendered).value();
    ++next_in_chunk_;
    return true;
  }

 private:
  ::duckdb_database db_ = nullptr;
  ::duckdb_connection conn_ = nullptr;
  ::duckdb_result result_{};
  schema::TableSchema schema_{};
  ::duckdb_data_chunk chunk_ = nullptr;
  ::idx_t chunk_size_ = 0;
  ::idx_t next_in_chunk_ = 0;
};

// Runs `sql` on `conn`; returns OK or INTERNAL with the DuckDB
// error message attached. Use this for INSERT / CREATE statements
// where the result rowset is uninteresting.
absl::Status RunSqlNoResult(::duckdb_connection conn, absl::string_view sql) {
  ::duckdb_result result;
  const std::string sql_str(sql);
  const auto state = ::duckdb_query(conn, sql_str.c_str(), &result);
  if (state != ::DuckDBSuccess) {
    const auto* err = ::duckdb_result_error(&result);
    std::string detail = err == nullptr ? std::string("") : std::string(err);
    ::duckdb_destroy_result(&result);
    return absl::InternalError(
        absl::StrCat("DuckDBEngine: query failed: ", sql_str, ": ", detail));
  }
  ::duckdb_destroy_result(&result);
  return absl::OkStatus();
}

// Materialize one storage table inside `conn` at the DuckDB-side name
// `quoted_table_name` (already quoted, may be schema-qualified). We
// CREATE OR REPLACE TABLE with the matching schema, then stream every
// row through `Storage::ScanRows` and INSERT them with one multi-row
// VALUES batch.
//
// Note: this is the engine-agnostic path; when the active storage is
// DuckDB-backed, a future optimization can substitute a
// `CREATE VIEW ... AS SELECT * FROM read_parquet(...)` against the
// storage's Parquet snapshot for a no-copy attach.
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
      return absl::InvalidArgumentError(absl::StrCat("DuckDBEngine: row[",
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

// Convenience wrapper around `AttachStorageTableAt` used by the
// SELECT path (`ExecuteQuery`): attaches the table at its bare
// `Table::Name()` so the transpiler's `EmitTableScan` output
// (`FROM "people"`) resolves in the connection's default schema.
absl::Status AttachStorageTable(::duckdb_connection conn,
                                storage::Storage* storage,
                                const catalog::StorageTable& table) {
  return AttachStorageTableAt(conn, storage, table, QuoteIdent(table.Name()));
}

absl::StatusOr<std::string> RenderSemanticParameterLiteral(
    const semantic::Value& v) {
  if (v.is_null()) return std::string("NULL");
  auto storage_or = semantic::ToStorageValue(v);
  if (!storage_or.ok()) return storage_or.status();
  schema::ColumnSchema col;
  col.name = "p";
  col.mode = schema::ColumnMode::kNullable;
  switch (v.type_kind()) {
    case ::googlesql::TYPE_BOOL:
      col.type = schema::ColumnType::kBool;
      break;
    case ::googlesql::TYPE_INT64:
      col.type = schema::ColumnType::kInt64;
      break;
    case ::googlesql::TYPE_DOUBLE:
      col.type = schema::ColumnType::kFloat64;
      break;
    case ::googlesql::TYPE_STRING:
    case ::googlesql::TYPE_JSON:
    case ::googlesql::TYPE_GEOGRAPHY:
      col.type = schema::ColumnType::kString;
      break;
    case ::googlesql::TYPE_BYTES:
      col.type = schema::ColumnType::kBytes;
      break;
    case ::googlesql::TYPE_DATE:
      col.type = schema::ColumnType::kDate;
      break;
    case ::googlesql::TYPE_TIMESTAMP:
      col.type = schema::ColumnType::kTimestamp;
      break;
    case ::googlesql::TYPE_NUMERIC:
      col.type = schema::ColumnType::kNumeric;
      break;
    case ::googlesql::TYPE_BIGNUMERIC:
      col.type = schema::ColumnType::kBignumeric;
      break;
    default:
      col.type = schema::ColumnType::kString;
      break;
  }
  return RenderScalarLiteral(*storage_or, col);
}

absl::StatusOr<std::string> SubstituteDuckdbParameters(
    std::string sql,
    const std::vector<transpiler::Transpiler::ParameterRef>& order,
    absl::Span<const QueryParameter> parameters) {
  if (order.empty()) return sql;
  std::vector<std::string> literals(order.size());
  for (size_t i = 0; i < order.size(); ++i) {
    const transpiler::Transpiler::ParameterRef& ref = order[i];
    const QueryParameter* param = nullptr;
    if (!ref.name.empty()) {
      for (const QueryParameter& p : parameters) {
        if (absl::EqualsIgnoreCase(p.name, ref.name)) {
          param = &p;
          break;
        }
      }
    } else {
      int seen = 0;
      for (const QueryParameter& p : parameters) {
        if (!p.name.empty()) continue;
        if (++seen == ref.position) {
          param = &p;
          break;
        }
      }
    }
    if (param == nullptr) {
      return absl::InvalidArgumentError(absl::StrCat(
          "DuckDbExecutor: missing query parameter for DuckDB placeholder $",
          i + 1));
    }
    auto value =
        semantic::ParseParameterValue(param->value_json, param->type_kind);
    if (!value.ok()) return value.status();
    auto literal = RenderSemanticParameterLiteral(*value);
    if (!literal.ok()) return literal.status();
    literals[i] = *std::move(literal);
  }
  for (int slot = static_cast<int>(order.size()); slot >= 1; --slot) {
    const std::string placeholder = absl::StrCat("$", slot);
    sql = absl::StrReplaceAll(
        sql, {{placeholder, literals[static_cast<size_t>(slot - 1)]}});
  }
  return sql;
}

}  // namespace

absl::StatusOr<std::unique_ptr<RowSource>> DuckDbExecutor::ExecuteQuery(
    const QueryRequest& request,
    const ::googlesql::ResolvedStatement& stmt,
    ::googlesql::Catalog* catalog) {
  (void)catalog;  // analysis has already happened on the coordinator.

  if (stmt.node_kind() != ::googlesql::RESOLVED_QUERY_STMT) {
    return absl::InvalidArgumentError(absl::StrCat(
        "DuckDbExecutor::ExecuteQuery: only SELECT-shaped queries are "
        "supported; got ",
        stmt.node_kind_string()));
  }
  const auto* query_stmt = stmt.GetAs<::googlesql::ResolvedQueryStmt>();
  absl::StatusOr<schema::TableSchema> output_schema =
      ReflectOutputSchema(*query_stmt);
  if (!output_schema.ok()) return output_schema.status();

  // 2. Lower the query through the transpiler's `EmitQueryStmt`,
  // which (a) walks the scan tree under the QueryStmt and (b) wraps
  // the result in the outermost `output_column_list()` projection so
  // the DuckDB result columns line up with the analyzer's output
  // schema 1:1. Handing `EmitQueryStmt` the QueryStmt itself
  // (instead of the inner scan, like the legacy path did) is what
  // makes `SELECT id, name FROM ds.t` narrow correctly even when
  // the analyzer is configured with `prune_unused_columns=false`
  // and the underlying TableScan keeps all of `ds.t`'s columns
  // (e.g. an extra `tags` column the SELECT discards): the inner
  // scan emit reflects the table's full column list, but the
  // outermost projection brings the cell stream back down to the 2
  // analyzer-output columns the gateway / `arrow_to_bq` expect.
  //
  // If the transpiler cannot lower the shape (a `""` return per the
  // empty-string contract) we surface UNIMPLEMENTED so the gateway
  // emits BigQuery's `notImplemented` reason. Once the route
  // classifier promotes every property-level fast-path gate (see
  // `route_classifier.cc`'s `Visit*` overrides) the only paths left
  // returning `""` are the genuinely-defensive ones (null pointers,
  // analyzer contract violations); reaching this branch is then a
  // signal of either a missing classifier rule or an analyzer-shape
  // surprise we should pin a fixture against.
  if (query_stmt->query() == nullptr) {
    return absl::UnimplementedError("duckdb engine: query has no scan tree");
  }
  transpiler::Transpiler t;
  std::string sql = t.Transpile(query_stmt);
  if (sql.empty()) {
    const char* plan = "googlesqlite-04-scan-emits.plan.md";
    const std::string kind = query_stmt->query()->node_kind_string();
    if (kind == "WithScan" || kind == "WithRefScan") {
      plan = "googlesqlite-02-withscan-cte.plan.md";
    } else if (kind == "OrderByScan") {
      plan = "googlesqlite-04-scan-emits.plan.md";
    }
    return absl::UnimplementedError(absl::StrCat(
        "duckdb engine: transpiler does not yet cover this query shape "
        "(family: node:",
        kind,
        ", route: duckdb_native); see docs/ENGINE_POLICY.md and plan ",
        plan,
        " for the missing emit"));
  }
  if (!t.parameter_order().empty()) {
    auto substituted = SubstituteDuckdbParameters(
        std::move(sql), t.parameter_order(), request.parameters);
    if (!substituted.ok()) return substituted.status();
    sql = *std::move(substituted);
  }

  // 3. Collect every referenced table so we can materialize them
  // inside the DuckDB connection under the bare names the
  // transpiler emitted.
  TableScanCollector collector;
  absl::Status visit_status = stmt.Accept(&collector);
  if (!visit_status.ok()) return visit_status;

  // 4. Open a fresh in-memory DuckDB. The connection / database are
  // per-query: tables we materialize live only for this RPC and
  // are torn down when the returned RowSource is destroyed.
  ::duckdb_database db = nullptr;
  if (::duckdb_open(nullptr, &db) != ::DuckDBSuccess) {
    return absl::InternalError(
        "DuckDbExecutor::ExecuteQuery: duckdb_open(in-memory) failed");
  }
  ::duckdb_connection conn = nullptr;
  if (::duckdb_connect(db, &conn) != ::DuckDBSuccess) {
    ::duckdb_close(&db);
    return absl::InternalError(
        "DuckDbExecutor::ExecuteQuery: duckdb_connect failed");
  }

  // 4b. Register the BigQuery polyfill UDF library on the fresh
  // connection. Every BigQuery function whose disposition is
  // `duckdb_udf` lowers to a UDF / macro the registrar installs
  // here; the transpiler then emits a plain `<udf_name>(<args>)`
  // call below. Registration failure is fail-fast: there is no
  // runtime "missing UDF -> fall back to another route" path
  // (per `googlesqlite-03-operator-disposition.plan.md`'s Done Criterion 2).
  if (auto reg = udf::RegisterAll(conn); !reg.ok()) {
    ::duckdb_disconnect(&conn);
    ::duckdb_close(&db);
    return reg;
  }

  // 5. Materialize each storage table inside the DuckDB connection.
  // The transpiler assumes `Table::Name()` resolves to a relation
  // already present in the connection's default schema.
  for (const ::googlesql::Table* tbl : collector.tables()) {
    if (const auto* virtual_table =
            dynamic_cast<const catalog::VirtualCatalogTable*>(tbl)) {
      absl::Status status = virtual_table->MaterializeInDuckDB(
          conn, storage_, QuoteIdent(tbl->Name()));
      if (!status.ok()) {
        ::duckdb_disconnect(&conn);
        ::duckdb_close(&db);
        return status;
      }
      continue;
    }
    const auto* storage_table = dynamic_cast<const catalog::StorageTable*>(tbl);
    if (storage_table == nullptr) {
      ::duckdb_disconnect(&conn);
      ::duckdb_close(&db);
      return absl::UnimplementedError(absl::StrCat(
          "duckdb engine: cannot attach non-StorageTable '",
          tbl->Name(),
          "'; rebuild against a GoogleSqlCatalog-backed analyzer"));
    }
    absl::Status status = AttachStorageTable(conn, storage_, *storage_table);
    if (!status.ok()) {
      ::duckdb_disconnect(&conn);
      ::duckdb_close(&db);
      return status;
    }
  }

  // 6. Execute the transpiled SQL. A DuckDB rejection folds into
  // UNIMPLEMENTED instead of INTERNAL because a transpiled SQL that
  // DuckDB cannot run is, by definition, a query the DuckDB engine
  // "cannot yet execute".
  ::duckdb_result result;
  if (::duckdb_query(conn, sql.c_str(), &result) != ::DuckDBSuccess) {
    const char* err = ::duckdb_result_error(&result);
    std::string detail = err == nullptr ? std::string("") : std::string(err);
    ::duckdb_destroy_result(&result);
    ::duckdb_disconnect(&conn);
    ::duckdb_close(&db);
    return absl::UnimplementedError(
        absl::StrCat("duckdb engine: DuckDB rejected transpiled SQL: ",
                     detail,
                     " (sql=",
                     sql,
                     ")"));
  }
  return std::unique_ptr<RowSource>(
      new DuckDBRowSource(db, conn, result, std::move(*output_schema)));
}

namespace {

// Stable string representation of a `storage::Value` used as a
// primary-key lookup key when diffing the pre- and post-MERGE row
// sets.
std::string SerializeForPkLookup(const storage::Value& value) {
  using Kind = storage::Value::Kind;
  switch (value.kind()) {
    case Kind::kNull:
      return "n:";
    case Kind::kBool:
      return std::string(value.bool_value() ? "b:1" : "b:0");
    case Kind::kInt64:
      return absl::StrCat("i:", value.int64_value());
    case Kind::kFloat64:
      return absl::StrCat("f:", value.float64_value());
    case Kind::kString:
      return absl::StrCat("s:", value.string_value());
    case Kind::kBytes:
      return absl::StrCat("y:", value.string_value());
    case Kind::kArray: {
      std::string out = "a:[";
      for (const auto& e : value.array_value()) {
        absl::StrAppend(&out, SerializeForPkLookup(e), ",");
      }
      absl::StrAppend(&out, "]");
      return out;
    }
    case Kind::kStruct: {
      std::string out = "t:{";
      for (const auto& f : value.struct_value()) {
        absl::StrAppend(&out, SerializeForPkLookup(f), ",");
      }
      absl::StrAppend(&out, "}");
      return out;
    }
  }
  return "?:";
}

// Deep-equal comparison for two storage::Value cells. We do not
// reuse `SerializeForPkLookup` because it conflates a `Bool(true)`
// with a `String("true")` for PK collapsing purposes; the diff
// classification path needs an exact-shape compare so a MATCHED row
// whose only change is a string-to-bool coercion still surfaces
// as an UPDATE.
bool ValuesEqual(const storage::Value& a, const storage::Value& b) {
  if (a.kind() != b.kind()) return false;
  using Kind = storage::Value::Kind;
  switch (a.kind()) {
    case Kind::kNull:
      return true;
    case Kind::kBool:
      return a.bool_value() == b.bool_value();
    case Kind::kInt64:
      return a.int64_value() == b.int64_value();
    case Kind::kFloat64:
      return a.float64_value() == b.float64_value();
    case Kind::kString:
    case Kind::kBytes:
      return a.string_value() == b.string_value();
    case Kind::kArray: {
      const auto& av = a.array_value();
      const auto& bv = b.array_value();
      if (av.size() != bv.size()) return false;
      for (size_t i = 0; i < av.size(); ++i) {
        if (!ValuesEqual(av[i], bv[i])) return false;
      }
      return true;
    }
    case Kind::kStruct: {
      const auto& af = a.struct_value();
      const auto& bf = b.struct_value();
      if (af.size() != bf.size()) return false;
      for (size_t i = 0; i < af.size(); ++i) {
        if (!ValuesEqual(af[i], bf[i])) return false;
      }
      return true;
    }
  }
  return false;
}

// Drains every row out of a DuckDB result through the same chunked
// `arrow_to_bq::ChunkRowToCells` path the SELECT row source uses, so
// the cells the MERGE diff sees match what `ExecuteQuery` would
// surface for the same table. The result is destroyed in-place; the
// caller owns the lifetime up to the call.
absl::StatusOr<std::vector<storage::Row>> DrainResultToRows(
    ::duckdb_result* result, const schema::TableSchema& schema) {
  std::vector<storage::Row> rows;
  while (true) {
    ::duckdb_data_chunk chunk = ::duckdb_fetch_chunk(*result);
    if (chunk == nullptr) break;
    const ::idx_t n = ::duckdb_data_chunk_get_size(chunk);
    for (::idx_t i = 0; i < n; ++i) {
      absl::StatusOr<storage::Row> rendered =
          arrow_to_bq::ChunkRowToCells(chunk, i, schema);
      if (!rendered.ok()) {
        ::duckdb_destroy_data_chunk(&chunk);
        return rendered.status();
      }
      rows.push_back(std::move(rendered).value());
    }
    ::duckdb_destroy_data_chunk(&chunk);
  }
  return rows;
}

// Read every row out of the DuckDB-side table at `quoted_table_name`,
// converted onto the engine-agnostic `storage::Row` shape that
// matches `bq_schema`. Used to capture the post-MERGE state of the
// target table so we can diff against the pre-MERGE snapshot and
// classify each PK delta as inserted / updated / deleted.
absl::StatusOr<std::vector<storage::Row>> ReadBackTable(
    ::duckdb_connection conn,
    absl::string_view quoted_table_name,
    const schema::TableSchema& bq_schema) {
  const std::string sql = absl::StrCat("SELECT * FROM ", quoted_table_name);
  ::duckdb_result result;
  if (::duckdb_query(conn, sql.c_str(), &result) != ::DuckDBSuccess) {
    const auto* err = ::duckdb_result_error(&result);
    std::string detail = err == nullptr ? std::string("") : std::string(err);
    ::duckdb_destroy_result(&result);
    return absl::InternalError(absl::StrCat(
        "DuckDBEngine: failed to read back target table ", sql, ": ", detail));
  }
  absl::StatusOr<std::vector<storage::Row>> rows =
      DrainResultToRows(&result, bq_schema);
  ::duckdb_destroy_result(&result);
  return rows;
}

// Classify the per-row delta between the pre-MERGE snapshot
// (`before`) and the post-MERGE state (`after`) of the same target
// table by walking both row sets keyed on the synthetic primary key
// (column 0, see `backend/catalog/storage_table.cc`'s constructor).
//
//   * PK present in `after` but not in `before` -> inserted
//   * PK present in both but row contents differ -> updated
//   * PK present in `before` but not in `after` -> deleted
//
// MATCHED-then-no-op rows (every column unchanged) do not count
// toward `updated_row_count`, mirroring BigQuery's behavior of only
// surfacing actually-modified rows in `dmlStats`.
DmlStats DiffByPrimaryKey(absl::Span<const storage::Row> before,
                          absl::Span<const storage::Row> after) {
  DmlStats stats;
  absl::flat_hash_map<std::string, const storage::Row*> before_by_pk;
  before_by_pk.reserve(before.size());
  for (const storage::Row& row : before) {
    if (row.cells.empty()) continue;
    before_by_pk[SerializeForPkLookup(row.cells.front())] = &row;
  }
  absl::flat_hash_map<std::string, const storage::Row*> after_by_pk;
  after_by_pk.reserve(after.size());
  for (const storage::Row& row : after) {
    if (row.cells.empty()) continue;
    after_by_pk[SerializeForPkLookup(row.cells.front())] = &row;
  }
  for (const auto& [pk, after_row] : after_by_pk) {
    auto it = before_by_pk.find(pk);
    if (it == before_by_pk.end()) {
      ++stats.inserted_row_count;
      continue;
    }
    const storage::Row* before_row = it->second;
    if (after_row->cells.size() != before_row->cells.size()) {
      ++stats.updated_row_count;
      continue;
    }
    bool changed = false;
    for (size_t c = 0; c < after_row->cells.size(); ++c) {
      if (!ValuesEqual(after_row->cells[c], before_row->cells[c])) {
        changed = true;
        break;
      }
    }
    if (changed) ++stats.updated_row_count;
  }
  for (const auto& [pk, _] : before_by_pk) {
    if (!after_by_pk.contains(pk)) ++stats.deleted_row_count;
  }
  return stats;
}

}  // namespace

absl::StatusOr<DmlStats> DuckDbExecutor::ExecuteDml(
    const QueryRequest& request,
    const ::googlesql::ResolvedStatement& stmt,
    ::googlesql::Catalog* catalog) {
  (void)catalog;  // analysis has already happened on the coordinator.
  if (storage_ == nullptr) {
    return absl::FailedPreconditionError(
        "DuckDbExecutor::ExecuteDml: storage backend is not configured");
  }

  if (stmt.node_kind() != ::googlesql::RESOLVED_MERGE_STMT) {
    // DML ENGINE POLICY (see `docs/ENGINE_POLICY.md`): the DuckDB
    // engine only implements MERGE today. INSERT / UPDATE / DELETE
    // route to the local DML executor in
    // `backend/engine/semantic/dml/`; reaching this branch means the
    // route classifier dispatched the wrong executor (an internal bug,
    // not a user-facing UNIMPLEMENTED).
    return absl::UnimplementedError(absl::StrCat(
        "duckdb engine: ExecuteDml only implements MERGE today (family: "
        "node:",
        stmt.node_kind_string(),
        ", route: semantic_executor); see docs/ENGINE_POLICY.md and plan "
        "googlesqlite-14-dml-system.plan.md"));
  }
  const auto* merge_stmt = stmt.GetAs<::googlesql::ResolvedMergeStmt>();
  if (merge_stmt->table_scan() == nullptr ||
      merge_stmt->table_scan()->table() == nullptr) {
    return absl::InternalError(
        "DuckDbExecutor::ExecuteDml: MERGE statement has no resolved "
        "target table scan");
  }

  // 2. Collect every referenced storage table. The target appears
  // once (from `table_scan()`); the source side (`from_scan()`) may
  // mention zero or more tables depending on the USING clause.
  TableScanCollector collector;
  absl::Status visit_status = stmt.Accept(&collector);
  if (!visit_status.ok()) return visit_status;

  const auto* target_table = dynamic_cast<const catalog::StorageTable*>(
      merge_stmt->table_scan()->table());
  if (target_table == nullptr) {
    return absl::FailedPreconditionError(
        absl::StrCat("DuckDbExecutor::ExecuteDml: MERGE target '",
                     merge_stmt->table_scan()->table()->FullName(),
                     "' is not backed by a StorageTable; cannot apply DML"));
  }
  const storage::TableId target_id = target_table->storage_table_id();

  // 3. Snapshot the target rows so we can diff post-MERGE to derive
  // per-branch DmlStats counts (DuckDB's MERGE returns a single
  // total via `duckdb_rows_changed`, but BigQuery's wire envelope
  // distinguishes insertedRowCount / updatedRowCount /
  // deletedRowCount).
  absl::StatusOr<std::unique_ptr<storage::RowIterator>> before_iter =
      storage_->ScanRows(target_id);
  if (!before_iter.ok()) return before_iter.status();
  std::vector<storage::Row> before_rows;
  {
    std::unique_ptr<storage::RowIterator> iter = std::move(before_iter).value();
    storage::Row row;
    while (true) {
      absl::StatusOr<bool> has = iter->Next(&row);
      if (!has.ok()) return has.status();
      if (!*has) break;
      before_rows.push_back(row);
    }
  }

  // 4. Open a fresh in-memory DuckDB. Per-query lifetime matches the
  // SELECT path; the connection is torn down before we return.
  ::duckdb_database db = nullptr;
  if (::duckdb_open(nullptr, &db) != ::DuckDBSuccess) {
    return absl::InternalError(
        "DuckDbExecutor::ExecuteDml: duckdb_open(in-memory) failed");
  }
  ::duckdb_connection conn = nullptr;
  if (::duckdb_connect(db, &conn) != ::DuckDBSuccess) {
    ::duckdb_close(&db);
    return absl::InternalError(
        "DuckDbExecutor::ExecuteDml: duckdb_connect failed");
  }
  // Same polyfill registration as the SELECT path: the MERGE
  // statement can carry BigQuery scalar / aggregate calls that
  // route through `duckdb_udf`, so the UDFs must be installed
  // before DuckDB sees the user-submitted SQL.
  if (auto reg = udf::RegisterAll(conn); !reg.ok()) {
    ::duckdb_disconnect(&conn);
    ::duckdb_close(&db);
    return reg;
  }

  // 5. Materialize each referenced storage table inside the DuckDB
  // connection under its schema-qualified `"dataset"."table"` name so
  // the user-submitted MERGE SQL (which typically writes
  // `MERGE INTO ds.people ...`) resolves end-to-end. The target
  // table's qualified name is captured so step 7 can read it back.
  std::string quoted_target;
  for (const ::googlesql::Table* tbl : collector.tables()) {
    const auto* storage_table = dynamic_cast<const catalog::StorageTable*>(tbl);
    if (storage_table == nullptr) {
      ::duckdb_disconnect(&conn);
      ::duckdb_close(&db);
      return absl::FailedPreconditionError(absl::StrCat(
          "DuckDbExecutor::ExecuteDml: cannot attach non-StorageTable '",
          tbl->Name(),
          "' for MERGE; rebuild against a "
          "GoogleSqlCatalog-backed analyzer"));
    }
    const storage::TableId& id = storage_table->storage_table_id();
    const std::string create_schema =
        absl::StrCat("CREATE SCHEMA IF NOT EXISTS ", QuoteIdent(id.dataset_id));
    absl::Status schema_status = RunSqlNoResult(conn, create_schema);
    if (!schema_status.ok()) {
      ::duckdb_disconnect(&conn);
      ::duckdb_close(&db);
      return schema_status;
    }
    const std::string qualified =
        absl::StrCat(QuoteIdent(id.dataset_id), ".", QuoteIdent(id.table_id));
    absl::Status attach =
        AttachStorageTableAt(conn, storage_, *storage_table, qualified);
    if (!attach.ok()) {
      ::duckdb_disconnect(&conn);
      ::duckdb_close(&db);
      return attach;
    }
    if (id == target_id) quoted_target = qualified;
  }
  if (quoted_target.empty()) {
    ::duckdb_disconnect(&conn);
    ::duckdb_close(&db);
    return absl::InternalError(
        "DuckDbExecutor::ExecuteDml: target table was not in the resolved "
        "table-scan set");
  }

  // 6. Execute the MERGE. We pass the user-submitted SQL verbatim:
  // DuckDB v1.2+ supports `MERGE INTO ... WHEN MATCHED / WHEN NOT
  // MATCHED ...` with the same statement shape BigQuery exposes, so
  // for the simple cases the conformance harness will seed in plans
  // 40-42 we do not need a transpiler. Cases DuckDB rejects fold to
  // INTERNAL (rather than UNIMPLEMENTED) because the DuckDB engine
  // is the only path for MERGE today.
  ::duckdb_result merge_result;
  if (::duckdb_query(conn, request.sql.c_str(), &merge_result) !=
      ::DuckDBSuccess) {
    const char* err = ::duckdb_result_error(&merge_result);
    std::string detail = err == nullptr ? std::string("") : std::string(err);
    ::duckdb_destroy_result(&merge_result);
    ::duckdb_disconnect(&conn);
    ::duckdb_close(&db);
    return absl::InvalidArgumentError(
        absl::StrCat("DuckDBEngine: DuckDB rejected MERGE: ",
                     detail,
                     " (sql=",
                     request.sql,
                     ")"));
  }
  ::duckdb_destroy_result(&merge_result);

  // 7. Read back the post-MERGE target rows so we can (a) ship them
  // back into the storage backend via `OverwriteRows` and (b)
  // classify the per-branch DmlStats counts by diffing against the
  // pre-MERGE snapshot.
  absl::StatusOr<std::vector<storage::Row>> after_rows =
      ReadBackTable(conn, quoted_target, target_table->bq_schema());
  ::duckdb_disconnect(&conn);
  ::duckdb_close(&db);
  if (!after_rows.ok()) return after_rows.status();

  absl::Status applied =
      storage_->OverwriteRows(target_id, absl::MakeConstSpan(*after_rows));
  if (!applied.ok()) return applied;

  return DiffByPrimaryKey(absl::MakeConstSpan(before_rows),
                          absl::MakeConstSpan(*after_rows));
}

// ---------------------------------------------------------------------------
// ExecuteDdl (DuckDB engine -- ALTER TABLE only after Plan 47)
//
// CREATE TABLE / CREATE TABLE AS SELECT / DROP TABLE all moved to
// `backend/engine/control/control_op_executor.{h,cc}` when
// `.cursor/plans/googlesqlite-01-ddl-catalog.plan.md` landed: those rows
// dropped `status=planned` from `node_dispositions.yaml`, so the
// route classifier dispatches them to the `ControlOpExecutor`
// directly and the DuckDB executor never sees them.
//
// `ResolvedAlterTableStmt` stays here because the alter-action
// surface is deliberately out of scope for plan 47 (the plan only
// migrated CREATE / DROP / ANALYZE shapes). The classifier's
// fallthrough for shapes missing from `node_dispositions.yaml`
// keeps `ALTER` on the `kDuckdbNative` route, the coordinator
// dispatches it here, and the legacy scan-drop-create-append cycle
// below applies the new column without an in-place schema-evolution
// API on `Storage`.
//
// Shape decisions for the surviving ALTER path:
//
//   * Name-path resolution mirrors `GoogleSqlCatalog::FindTable`'s
//     rules: two-segment paths default the project from
//     `request.project_id`; three-segment paths override it.
//     Anything else surfaces as INVALID_ARGUMENT.
//   * ALTER TABLE ADD COLUMN runs a scan-drop-create-append cycle
//     because the `Storage` interface has no in-place schema
//     evolution today; we leave each new column's cells as
//     `Value::Null()` so existing rows survive the rewrite
//     unchanged. The cycle is not atomic against concurrent
//     readers; `Storage::OverwriteRows`'s contract carries the
//     same caveat, so we inherit it here.

namespace {

// Resolve `name_path` (as produced by `ResolvedCreateStatement::
// name_path()` etc.) to a `storage::TableId` under
// `default_project_id`. Mirrors `GoogleSqlCatalog::FindTable`'s
// rules: two-segment paths default the project, three-segment
// paths override it. Anything else surfaces as INVALID_ARGUMENT so
// the gateway can map it to BigQuery's matching REST error.
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
      "duckdb engine: DDL target name must be <dataset>.<table> or "
      "<project>.<dataset>.<table>; got ",
      name_path.size(),
      " segments"));
}

// Render the BigQuery FieldSchema mode (NULLABLE / REQUIRED /
// REPEATED) from a resolved column definition. The analyzer carries
// the NOT NULL annotation on `annotations()->not_null()`; ARRAY
// types lower onto REPEATED at the field-schema layer (see
// `backend/schema/googlesql_to_bq.cc::TypeToFieldSchema`), so for
// ARRAY columns we leave the mode alone -- TypeToFieldSchema sets
// it to REPEATED already.
void ApplyAnnotations(const ::googlesql::ResolvedColumnDefinition* def,
                      v1::FieldSchema* field) {
  if (def == nullptr) return;
  if (field->mode() == "REPEATED") return;
  const ::googlesql::ResolvedColumnAnnotations* ann = def->annotations();
  if (ann != nullptr && ann->not_null()) {
    field->set_mode("REQUIRED");
  }
}

// ALTER TABLE ADD COLUMN executor. The storage interface has no
// in-place schema evolution today, so we scan-drop-create-append
// to land the new column. `is_if_not_exists()` on the action
// swallows the "column already present" case; we detect that by
// looking up the new column's name in the existing schema before
// touching the storage backend.
// Translate a single RESOLVED_ADD_COLUMN_ACTION into a
// `ColumnSchema`. Returns `nullopt` when `IF NOT EXISTS` skipped the
// column because it already exists, or a non-OK status when the
// action is malformed / refers to an already-existing column without
// IF NOT EXISTS. Used by `RunAlterTableAddColumn` to keep the
// per-action body off the parent's complexity budget.
absl::StatusOr<std::optional<schema::ColumnSchema>> ProcessAddColumnAction(
    const ::googlesql::ResolvedAlterAction* action,
    const schema::TableSchema& existing,
    const storage::TableId& target) {
  if (action == nullptr) return std::optional<schema::ColumnSchema>{};
  if (action->node_kind() != ::googlesql::RESOLVED_ADD_COLUMN_ACTION) {
    return absl::UnimplementedError(absl::StrCat(
        "duckdb engine: ALTER TABLE action ",
        action->node_kind_string(),
        " is not implemented (only ADD COLUMN is supported); ALTER TABLE "
        "is on the control_op route per docs/ENGINE_POLICY.md and plan "
        "googlesqlite-01-ddl-catalog.plan.md, but the per-action handlers "
        "still "
        "live here pending the migration"));
  }
  const auto* add = action->GetAs<::googlesql::ResolvedAddColumnAction>();
  const ::googlesql::ResolvedColumnDefinition* def = add->column_definition();
  if (def == nullptr) {
    return absl::InternalError(
        "DuckDBEngine::ExecuteDdl: ADD COLUMN action has null column "
        "definition");
  }
  const bool already = std::any_of(
      existing.columns.begin(),
      existing.columns.end(),
      [&](const schema::ColumnSchema& c) { return c.name == def->name(); });
  if (already) {
    if (add->is_if_not_exists()) return std::optional<schema::ColumnSchema>{};
    return absl::AlreadyExistsError(
        absl::StrCat("duckdb engine: ALTER TABLE ADD COLUMN: column '",
                     def->name(),
                     "' already exists on table ",
                     target.project_id,
                     ".",
                     target.dataset_id,
                     ".",
                     target.table_id));
  }
  v1::FieldSchema field;
  absl::Status s =
      backend::schema::TypeToFieldSchema(def->type(), def->name(), &field);
  if (!s.ok()) return s;
  ApplyAnnotations(def, &field);
  absl::StatusOr<schema::ColumnSchema> column =
      backend::schema::ColumnSchemaFromProto(field);
  if (!column.ok()) return column.status();
  return std::optional<schema::ColumnSchema>{*std::move(column)};
}

// Drain every row from `target` into an owned vector, padding each
// row's cell list with NULL for every entry in `added_columns` so the
// post-ALTER schema can be applied with a single OverwriteRows-style
// rewrite. Used by `RunAlterTableAddColumn` to keep the row-rewrite
// loop off the parent's complexity budget.
absl::StatusOr<std::vector<storage::Row>> CopyRowsForAlter(
    storage::Storage& storage,
    const storage::TableId& target,
    absl::Span<const schema::ColumnSchema> added_columns) {
  absl::StatusOr<std::unique_ptr<storage::RowIterator>> iter =
      storage.ScanRows(target);
  if (!iter.ok()) return iter.status();
  std::vector<storage::Row> rows;
  std::unique_ptr<storage::RowIterator> rows_iter = std::move(iter).value();
  storage::Row row;
  while (true) {
    absl::StatusOr<bool> has = rows_iter->Next(&row);
    if (!has.ok()) return has.status();
    if (!*has) break;
    rows.push_back(row);
  }
  for (storage::Row& r : rows) {
    for (size_t i = 0; i < added_columns.size(); ++i) {
      r.cells.push_back(storage::Value::Null());
    }
  }
  return rows;
}

// Drop, recreate (with `new_schema`), and re-append `rows` to
// `target` -- the scan-and-rewrite implementation of ALTER TABLE ADD
// COLUMN that this engine relies on because DuckDB's `ALTER TABLE`
// does not propagate to the storage backend's catalog yet.
absl::Status RebuildTableWithColumns(storage::Storage& storage,
                                     const storage::TableId& target,
                                     const schema::TableSchema& new_schema,
                                     absl::Span<const storage::Row> rows) {
  absl::Status dropped = storage.DropTable(target);
  if (!dropped.ok()) return dropped;
  absl::Status created = storage.CreateTable(target, new_schema);
  if (!created.ok()) return created;
  if (rows.empty()) return absl::OkStatus();
  return storage.AppendRows(target, rows);
}

absl::Status RunAlterTableAddColumn(
    storage::Storage& storage,
    absl::string_view project_id,
    const ::googlesql::ResolvedAlterTableStmt* stmt) {
  if (stmt == nullptr) {
    return absl::InternalError(
        "DuckDBEngine::ExecuteDdl: ALTER TABLE has null resolved statement");
  }
  absl::StatusOr<storage::TableId> target =
      NamePathToTableId(stmt->name_path(), project_id);
  if (!target.ok()) return target.status();

  absl::StatusOr<schema::TableSchema> existing = storage.GetSchema(*target);
  if (!existing.ok()) {
    if (existing.status().code() == absl::StatusCode::kNotFound &&
        stmt->is_if_exists()) {
      return absl::OkStatus();
    }
    return existing.status();
  }

  // Collect the new columns we need to add by walking the action
  // list. We only implement RESOLVED_ADD_COLUMN_ACTION today; every
  // other action surfaces as UNIMPLEMENTED so the followup
  // alter-action plans see a stable contract to fill in.
  std::vector<schema::ColumnSchema> added_columns;
  added_columns.reserve(stmt->alter_action_list_size());
  for (int i = 0; i < stmt->alter_action_list_size(); ++i) {
    absl::StatusOr<std::optional<schema::ColumnSchema>> column =
        ProcessAddColumnAction(stmt->alter_action_list(i), *existing, *target);
    if (!column.ok()) return column.status();
    if (column->has_value()) added_columns.push_back(*std::move(*column));
  }
  if (added_columns.empty()) return absl::OkStatus();

  // Scan rows under the *current* schema before we touch anything;
  // `CopyRowsForAlter` pads every existing row's cell list with NULL
  // for each newly added column so storage's per-row shape stays
  // aligned with the post-ALTER schema.
  absl::StatusOr<std::vector<storage::Row>> rows =
      CopyRowsForAlter(storage, *target, added_columns);
  if (!rows.ok()) return rows.status();

  schema::TableSchema new_schema = *existing;
  for (const schema::ColumnSchema& c : added_columns) {
    new_schema.columns.push_back(c);
  }
  return RebuildTableWithColumns(
      storage, *target, new_schema, absl::MakeConstSpan(*rows));
}

}  // namespace

absl::Status DuckDbExecutor::ExecuteDdl(
    const QueryRequest& request,
    const ::googlesql::ResolvedStatement& stmt,
    ::googlesql::Catalog* catalog) {
  (void)catalog;  // analysis has already happened on the coordinator.
  if (storage_ == nullptr) {
    return absl::FailedPreconditionError(
        "DuckDbExecutor::ExecuteDdl: storage backend is not configured");
  }

  const absl::string_view project_id = request.project_id;
  switch (stmt.node_kind()) {
    case ::googlesql::RESOLVED_ALTER_TABLE_STMT:
      return RunAlterTableAddColumn(
          *storage_,
          project_id,
          stmt.GetAs<::googlesql::ResolvedAlterTableStmt>());
    default:
      // CREATE TABLE / CTAS / DROP TABLE / ANALYZE moved to
      // `backend/engine/control/control_op_executor.cc` when
      // `googlesqlite-01-ddl-catalog.plan.md` landed; the route classifier
      // dispatches them to the `ControlOpExecutor` directly. If
      // one reaches us here, the classifier or the YAML
      // disposition row drifted out of sync with this dispatch
      // table -- surface UNIMPLEMENTED so the gateway/e2e suite
      // catches the regression instead of silently routing the
      // statement through DuckDB.
      return absl::UnimplementedError(absl::StrCat(
          "duckdb engine: ExecuteDdl only handles ALTER TABLE today; got ",
          stmt.node_kind_string(),
          ". CREATE TABLE / CREATE TABLE AS SELECT / DROP TABLE / "
          "ANALYZE are owned by ControlOpExecutor "
          "(see backend/engine/control/control_op_executor.cc) and "
          "the route classifier should never dispatch them here."));
  }
}

}  // namespace duckdb
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
