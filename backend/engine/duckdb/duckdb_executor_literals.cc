#include <cmath>
#include <cstdint>
#include <filesystem>
#include <memory>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include "absl/container/flat_hash_map.h"
#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "absl/strings/ascii.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/str_format.h"
#include "absl/strings/str_replace.h"
#include "absl/strings/string_view.h"
#include "absl/time/clock.h"
#include "absl/time/time.h"
#include "absl/types/span.h"
#include "backend/catalog/storage_table.h"
#include "backend/catalog/virtual_table.h"
#include "backend/engine/duckdb/arrow_to_bq.h"
#include "backend/engine/duckdb/duckdb_executor_internal.h"
#include "backend/engine/duckdb/transpiler/transpiler.h"
#include "backend/engine/engine.h"
#include "backend/engine/phase_recorder.h"
#include "backend/engine/semantic/value.h"
#include "backend/schema/schema.h"
#include "backend/storage/duckdb/duckdb_storage_internal.h"
#include "backend/storage/storage.h"
#include "duckdb.h"
#include "googlesql/resolved_ast/resolved_ast.h"
#include "googlesql/resolved_ast/resolved_ast_visitor.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace duckdb {
namespace internal {

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
  // Parquet round-trip via DuckDBStorage::ReadCell materializes STRUCT
  // columns as VARCHAR; pass the literal through when it already looks
  // like `{'field': value, ...}`.
  if (cell.kind() == storage::Value::Kind::kString) {
    absl::string_view text = absl::StripAsciiWhitespace(cell.string_value());
    if (!text.empty() && text.front() == '{') {
      return std::string(text);
    }
  }
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

std::string RenderColumnIdentList(const schema::TableSchema& schema) {
  std::string out;
  for (size_t i = 0; i < schema.columns.size(); ++i) {
    if (i > 0) absl::StrAppend(&out, ", ");
    absl::StrAppend(&out, QuoteIdent(schema.columns[i].name));
  }
  return out;
}

void RecordPhase(PhaseRecorder* recorder,
                 absl::string_view name,
                 absl::Duration elapsed) {
  if (recorder != nullptr) {
    recorder->Record(name, absl::ToInt64Microseconds(elapsed));
  }
}

bool SchemaHasFloat64Column(const schema::TableSchema& schema) {
  for (const auto& column : schema.columns) {
    if (column.type == schema::ColumnType::kFloat64) {
      return true;
    }
  }
  return false;
}

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
                                  absl::string_view quoted_table_name,
                                  PhaseRecorder* phase_recorder) {
  const schema::TableSchema& schema = table.bq_schema();
  const std::string table_name(quoted_table_name);
  const storage::TableId& id = table.storage_table_id();

  if (auto parquet_path = storage->ParquetSnapshotPath(id);
      parquet_path && !SchemaHasFloat64Column(schema)) {
    const absl::Time attach_start = absl::Now();
    const std::string select_cols =
        storage::duckdb::internal::RenderColumnIdentList(schema);
    const std::string escaped = EscapeStringLiteralInner(*parquet_path);
    const std::string columns = RenderColumnList(schema);
    absl::Status status = RunSqlNoResult(
        conn,
        absl::StrCat("CREATE OR REPLACE TABLE ", table_name, " ", columns));
    if (!status.ok()) return status;
    status = RunSqlNoResult(conn,
                            absl::StrCat("INSERT INTO ",
                                         table_name,
                                         " SELECT ",
                                         select_cols,
                                         " FROM read_parquet('",
                                         escaped,
                                         "')"));
    RecordPhase(
        phase_recorder, "table_attach_parquet", absl::Now() - attach_start);
    return status;
  }

  const std::string columns = RenderColumnList(schema);

  absl::Status status = RunSqlNoResult(
      conn, absl::StrCat("CREATE OR REPLACE TABLE ", table_name, " ", columns));
  if (!status.ok()) return status;

  const absl::Time scan_start = absl::Now();
  absl::StatusOr<std::unique_ptr<storage::RowIterator>> iter =
      storage->ScanRows(id);
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
  RecordPhase(phase_recorder, "scan_rows", absl::Now() - scan_start);
  if (rows.empty()) return absl::OkStatus();

  const absl::Time insert_start = absl::Now();
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
  status = RunSqlNoResult(conn, insert_sql);
  RecordPhase(phase_recorder, "insert_render_exec", absl::Now() - insert_start);
  return status;
}

// Convenience wrapper around `AttachStorageTableAt` used by the
// SELECT path (`ExecuteQuery`): attaches the table at its bare
// `Table::Name()` so the transpiler's `EmitTableScan` output
// (`FROM "people"`) resolves in the connection's default schema.
absl::Status AttachStorageTable(::duckdb_connection conn,
                                storage::Storage* storage,
                                const catalog::StorageTable& table,
                                PhaseRecorder* phase_recorder) {
  return AttachStorageTableAt(
      conn, storage, table, QuoteIdent(table.Name()), phase_recorder);
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
    case ::googlesql::TYPE_ARRAY:
      col.mode = schema::ColumnMode::kRepeated;
      col.type = schema::ColumnType::kString;
      return RenderCellLiteral(*storage_or, col);
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
    auto value = semantic::ParseParameterValue(
        param->value_json, param->type_kind, param->type_json);
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

}  // namespace internal
}  // namespace duckdb
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
