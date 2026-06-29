#include <cmath>
#include <memory>
#include <optional>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "absl/strings/ascii.h"
#include "absl/strings/match.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/str_join.h"
#include "absl/strings/str_replace.h"
#include "absl/strings/string_view.h"
#include "absl/types/span.h"
#include "backend/catalog/create_function_util.h"
#include "backend/catalog/googlesql_catalog.h"
#include "backend/catalog/storage_table.h"
#include "backend/catalog/udf_registry.h"
#include "backend/engine/control/control_op_internal.h"
#include "backend/engine/duckdb/arrow_to_bq.h"
#include "backend/engine/duckdb/transpiler/transpiler.h"
#include "backend/engine/duckdb/udf/registrar.h"
#include "backend/engine/engine.h"
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
namespace control {
namespace internal {

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

absl::StatusOr<std::string> RenderSemanticParameterLiteral(
    const semantic::Value& v) {
  if (v.is_null()) return std::string("NULL");
  switch (v.type_kind()) {
    case ::googlesql::TYPE_BOOL:
      return std::string(v.bool_value() ? "TRUE" : "FALSE");
    case ::googlesql::TYPE_INT64:
      return absl::StrCat(v.int64_value());
    case ::googlesql::TYPE_DOUBLE: {
      storage::Value cell = storage::Value::Float64(v.double_value());
      return RenderFloatLiteral(cell);
    }
    case ::googlesql::TYPE_STRING:
    case ::googlesql::TYPE_JSON:
    case ::googlesql::TYPE_GEOGRAPHY:
      return absl::StrCat("'", EscapeStringLiteralInner(v.string_value()), "'");
    case ::googlesql::TYPE_BYTES:
      return RenderBlobLiteral(v.bytes_value());
    default: {
      auto storage_or = semantic::ToStorageValue(v);
      if (!storage_or.ok()) return storage_or.status();
      schema::ColumnSchema col;
      col.name = "p";
      col.mode = schema::ColumnMode::kNullable;
      switch (v.type_kind()) {
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
  }
}

const QueryParameter* FindParameterForRef(
    const duckdb::transpiler::Transpiler::ParameterRef& ref,
    absl::Span<const QueryParameter> parameters) {
  if (!ref.name.empty()) {
    for (const QueryParameter& p : parameters) {
      if (absl::EqualsIgnoreCase(p.name, ref.name)) return &p;
    }
    return nullptr;
  }
  int seen = 0;
  for (const QueryParameter& p : parameters) {
    if (!p.name.empty()) continue;
    if (++seen == ref.position) return &p;
  }
  return nullptr;
}

absl::StatusOr<std::string> RenderParameterLiteral(
    const QueryParameter& parameter) {
  auto value = semantic::ParseParameterValue(
      parameter.value_json, parameter.type_kind, parameter.type_json);
  if (!value.ok()) return value.status();
  auto literal = RenderSemanticParameterLiteral(*value);
  if (!literal.ok()) return literal.status();
  return *std::move(literal);
}

absl::StatusOr<std::string> SubstituteDuckdbParameters(
    std::string sql,
    const std::vector<duckdb::transpiler::Transpiler::ParameterRef>& order,
    absl::Span<const QueryParameter> parameters) {
  if (order.empty()) return sql;
  std::vector<std::string> literals(order.size());
  for (size_t i = 0; i < order.size(); ++i) {
    const QueryParameter* param = FindParameterForRef(order[i], parameters);
    if (param == nullptr) {
      return absl::InvalidArgumentError(
          absl::StrCat("ControlOpExecutor: missing query parameter for DuckDB ",
                       "placeholder $",
                       i + 1));
    }
    absl::StatusOr<std::string> literal = RenderParameterLiteral(*param);
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

absl::StatusOr<std::optional<std::string>> ResolveParquetPath(
    storage::Storage* storage,
    const storage::TableId& id,
    std::optional<std::int64_t> as_of_ms) {
  if (!as_of_ms.has_value()) return storage->ParquetSnapshotPath(id);
  absl::StatusOr<std::optional<std::string>> path_or =
      storage->ParquetSnapshotPathAt(id, *as_of_ms);
  if (!path_or.ok()) return path_or.status();
  return std::move(*path_or);
}

absl::Status CreateOrReplaceTable(::duckdb_connection conn,
                                  absl::string_view table_name,
                                  const schema::TableSchema& schema) {
  return RunSqlNoResult(conn,
                        absl::StrCat("CREATE OR REPLACE TABLE ",
                                     table_name,
                                     " ",
                                     RenderColumnList(schema)));
}

absl::StatusOr<std::vector<storage::Row>> ReadRowsFromIterator(
    std::unique_ptr<storage::RowIterator> rows_iter) {
  std::vector<storage::Row> rows;
  storage::Row row;
  while (true) {
    absl::StatusOr<bool> has = rows_iter->Next(&row);
    if (!has.ok()) return has.status();
    if (!*has) break;
    rows.push_back(row);
  }
  return rows;
}

absl::StatusOr<std::string> BuildInsertSqlForRows(
    absl::string_view table_name,
    const catalog::StorageTable& table,
    const schema::TableSchema& schema,
    absl::Span<const storage::Row> rows) {
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
  return insert_sql;
}

// Materialize one storage table inside `conn` at the given quoted
// table name. Mirrors `DuckDbExecutor::AttachStorageTableAt` for
// CTAS.
absl::Status AttachStorageTableAt(::duckdb_connection conn,
                                  storage::Storage* storage,
                                  const catalog::StorageTable& table,
                                  absl::string_view quoted_table_name,
                                  std::optional<std::int64_t> as_of_ms) {
  const schema::TableSchema& schema = table.bq_schema();
  const std::string table_name(quoted_table_name);
  const storage::TableId& id = table.storage_table_id();

  absl::StatusOr<std::optional<std::string>> parquet_path_or =
      ResolveParquetPath(storage, id, as_of_ms);
  if (!parquet_path_or.ok()) return parquet_path_or.status();
  std::optional<std::string> parquet_path = std::move(*parquet_path_or);

  if (parquet_path) {
    absl::Status status = CreateOrReplaceTable(conn, table_name, schema);
    if (!status.ok()) return status;
    const std::string escaped = EscapeStringLiteralInner(*parquet_path);
    return RunSqlNoResult(conn,
                          absl::StrCat("INSERT INTO ",
                                       table_name,
                                       " SELECT * FROM read_parquet('",
                                       escaped,
                                       "')"));
  }

  absl::Status status = CreateOrReplaceTable(conn, table_name, schema);
  if (!status.ok()) return status;

  absl::StatusOr<std::unique_ptr<storage::RowIterator>> iter =
      storage->ScanRows(table.storage_table_id());
  if (!iter.ok()) return iter.status();
  absl::StatusOr<std::vector<storage::Row>> rows_or =
      ReadRowsFromIterator(std::move(*iter));
  if (!rows_or.ok()) return rows_or.status();
  std::vector<storage::Row> rows = std::move(*rows_or);
  if (rows.empty()) return absl::OkStatus();

  absl::StatusOr<std::string> insert_sql =
      BuildInsertSqlForRows(table_name, table, schema, rows);
  if (!insert_sql.ok()) return insert_sql.status();
  return RunSqlNoResult(conn, *insert_sql);
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

}  // namespace internal
}  // namespace control
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
