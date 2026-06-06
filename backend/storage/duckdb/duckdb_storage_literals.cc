#include <cmath>
#include <cstdlib>
#include <string>

#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "absl/strings/ascii.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/str_format.h"
#include "absl/strings/str_replace.h"
#include "absl/strings/string_view.h"
#include "absl/types/span.h"
#include "backend/schema/schema.h"
#include "backend/storage/duckdb/duckdb_storage_internal.h"
#include "backend/storage/row_restriction.h"
#include "backend/storage/storage.h"

namespace bigquery_emulator {
namespace backend {
namespace storage {
namespace duckdb {
namespace internal {

std::string QuoteIdent(absl::string_view ident) {
  std::string escaped = absl::StrReplaceAll(ident, {{"\"", "\"\""}});
  return absl::StrCat("\"", escaped, "\"");
}

// Escapes a DuckDB SQL string literal by doubling embedded
// single-quotes. The result is *not* wrapped in quotes; the caller is
// responsible for that so the helper composes cleanly into
// `'...'`, `DATE '...'`, `TIMESTAMP '...'`, etc.
std::string EscapeStringLiteralInner(absl::string_view s) {
  return absl::StrReplaceAll(s, {{"'", "''"}});
}

// Renders raw bytes as a DuckDB BLOB literal (lower-case hex).
// DuckDB accepts both `BLOB '\xAB\xCD'` and the SQL-standard
// `X'ABCD'`; the latter is simpler to emit because every byte is
// exactly two characters of output and there is no escape sequence
// to think about.
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

// Forward declaration for the recursive cell renderer.
absl::StatusOr<std::string> RenderCellLiteral(
    const Value& cell, const schema::ColumnSchema& column);

// Renders a single non-repeated scalar value as a DuckDB SQL literal,
// excluding the NULL case (the caller short-circuits that before
// calling). The column metadata is needed to pick the right SQL
// literal form for the temporal / numeric types that round-trip as
// strings in our Value union.
//
// The gateway `tabledata.insertAll` path lowers every JSON cell
// to a `Value::String` regardless of the column's declared type
// (see the comment on `frontend/handlers/catalog.cc::CellToValue`).
// For numeric / boolean columns we therefore accept both the natively-
// typed `Value::Int64` / `Value::Float64` / `Value::Bool` and a
// `Value::String` carrying the textual representation, and we delegate
// the final parse to DuckDB by emitting a CAST literal. This keeps
// the storage layer compatible with the wire-shape stringification
// the gateway performs while still surfacing malformed values as a
// CAST failure from DuckDB rather than silently storing zero.
// Render the DuckDB literal for a FLOAT64 cell. Mirrors the
// counterpart in `duckdb_executor.cc::RenderFloatLiteral`; kept
// duplicated for now (see follow-up de-dup task).
std::string RenderFloatLiteral(const Value& cell) {
  if (cell.kind() == Value::Kind::kString) {
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
// cell. Mirrors the counterpart in
// `duckdb_executor.cc::RenderStructLiteral` (see follow-up de-dup task).
absl::StatusOr<std::string> RenderStructLiteral(
    const Value& cell, const schema::ColumnSchema& column) {
  if (cell.kind() == Value::Kind::kString) {
    absl::string_view text = absl::StripAsciiWhitespace(cell.string_value());
    if (!text.empty() && text.front() == '{') {
      return std::string(text);
    }
  }
  if (cell.kind() != Value::Kind::kStruct) {
    return absl::InvalidArgumentError(
        absl::StrCat("AppendRows: column '",
                     column.name,
                     "' expects STRUCT but row provided non-struct cell"));
  }
  const auto& fields = cell.struct_value();
  if (fields.size() != column.fields.size()) {
    return absl::InvalidArgumentError(
        absl::StrCat("AppendRows: STRUCT column '",
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
    const Value& cell, const schema::ColumnSchema& column) {
  switch (column.type) {
    case schema::ColumnType::kBool:
      if (cell.kind() == Value::Kind::kString) {
        return absl::StrCat("CAST('",
                            EscapeStringLiteralInner(cell.string_value()),
                            "' AS BOOLEAN)");
      }
      return std::string(cell.bool_value() ? "TRUE" : "FALSE");
    case schema::ColumnType::kInt64:
      if (cell.kind() == Value::Kind::kString) {
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
      // Stored as a textual decimal in our Value union; let DuckDB
      // re-parse it under the declared precision/scale so out-of-
      // range values surface as an INTERNAL from RunSql.
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
    const Value& cell, const schema::ColumnSchema& column) {
  if (cell.is_null()) return std::string("NULL");
  // REPEATED cells carry an array on the wire even when the column
  // type itself is a scalar like INT64 — DuckDB's LIST literal form
  // is `[v1, v2, ...]`. Use a synthetic non-repeated column for the
  // element renderer so the recursive call doesn't re-enter the
  // array branch.
  if (column.mode == schema::ColumnMode::kRepeated) {
    if (cell.kind() != Value::Kind::kArray) {
      return absl::InvalidArgumentError(
          absl::StrCat("AppendRows: REPEATED column '",
                       column.name,
                       "' expects ARRAY but row provided kind ",
                       static_cast<int>(cell.kind())));
    }
    schema::ColumnSchema element = column;
    element.mode = schema::ColumnMode::kNullable;
    const auto& elems = cell.array_value();
    const std::string duckdb_list_type =
        schema::ColumnSchemaToDuckDBType(column);
    if (elems.empty()) {
      return absl::StrCat("CAST([] AS ", duckdb_list_type, ")");
    }
    std::string out = "CAST([";
    for (size_t i = 0; i < elems.size(); ++i) {
      if (i > 0) absl::StrAppend(&out, ", ");
      auto inner_or = RenderCellLiteral(elems[i], element);
      if (!inner_or.ok()) return inner_or.status();
      absl::StrAppend(&out, *inner_or);
    }
    absl::StrAppend(&out, "] AS ", duckdb_list_type, ")");
    return out;
  }
  return RenderScalarLiteral(cell, column);
}

// Builds the parenthesized column list / type list for a CREATE TABLE
// or COPY ... TO statement against the given schema.
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

// Renders an EqualityPredicate as a DuckDB `WHERE` clause fragment
// (with leading space, no trailing semicolon). The column identifier
// is double-quote escaped and the literal goes through the same
// `'...'` / numeric / CAST shapes the rest of this file uses, so a
// caller cannot escape the WHERE via crafted input — the parser
// already restricted the input to INT64 / BOOL / STRING literals
// (see `row_restriction.cc`) and the string literal is rendered with
// `EscapeStringLiteralInner`.
//
// Returned shape examples:
//   ` WHERE "id" = 42`
//   ` WHERE "active" = TRUE`
//   ` WHERE "name" = 'ada''s laptop'`
std::string RenderPredicateClause(const EqualityPredicate& pred) {
  std::string out = " WHERE ";
  absl::StrAppend(&out, QuoteIdent(pred.column), " = ");
  switch (pred.kind) {
    case EqualityPredicate::Kind::kInt64:
      absl::StrAppend(&out, pred.int64_value);
      return out;
    case EqualityPredicate::Kind::kBool:
      absl::StrAppend(&out, pred.bool_value ? "TRUE" : "FALSE");
      return out;
    case EqualityPredicate::Kind::kString:
      absl::StrAppend(
          &out, "'", EscapeStringLiteralInner(pred.string_value), "'");
      return out;
  }
  return out;
}

// Just the bare comma-separated identifier list (no type info, no
// trailing parenthesis). Used inside the SELECT projection list for
// `read_parquet` scans so the column order matches the table schema
// even if the on-disk parquet shuffled them.
std::string RenderColumnSelectExpr(const schema::ColumnSchema& column) {
  const std::string ident = QuoteIdent(column.name);
  if (column.mode == schema::ColumnMode::kRepeated) {
    return absl::StrCat("CAST(", ident, " AS VARCHAR) AS ", ident);
  }
  // Parquet-backed tables often store TIMESTAMP/DATETIME columns as
  // native Arrow/DuckDB temporal types. The C API's
  // `duckdb_value_varchar` returns an empty string for those cells,
  // so project them through VARCHAR in the SELECT list (same pattern
  // as REPEATED LIST columns).
  switch (column.type) {
    case schema::ColumnType::kDate:
    case schema::ColumnType::kTime:
    case schema::ColumnType::kDatetime:
    case schema::ColumnType::kTimestamp:
    case schema::ColumnType::kStruct:
      return absl::StrCat("CAST(", ident, " AS VARCHAR) AS ", ident);
    default:
      return ident;
  }
}

std::string RenderColumnIdentList(const schema::TableSchema& schema) {
  std::string out;
  for (size_t i = 0; i < schema.columns.size(); ++i) {
    if (i > 0) absl::StrAppend(&out, ", ");
    absl::StrAppend(&out, RenderColumnSelectExpr(schema.columns[i]));
  }
  return out;
}

// Renders an explicit comma-separated identifier list for the
// requested `names`. The caller must have already validated that
// every name exists in `schema`; we look up each one and emit
// `QuoteIdent(name)` so the resulting SELECT projects in the
// caller-supplied order. Empty `names` returns an empty string;
// callers must fall back to `RenderColumnIdentList(schema)` in that
// case (the SQL parser does not accept an empty SELECT list).
std::string RenderSelectedColumnIdentList(const schema::TableSchema& schema,
                                          absl::Span<const std::string> names) {
  std::string out;
  for (size_t i = 0; i < names.size(); ++i) {
    if (i > 0) absl::StrAppend(&out, ", ");
    bool found = false;
    for (const auto& column : schema.columns) {
      if (column.name == names[i]) {
        absl::StrAppend(&out, RenderColumnSelectExpr(column));
        found = true;
        break;
      }
    }
    if (!found) {
      absl::StrAppend(&out, QuoteIdent(names[i]));
    }
  }
  return out;
}

// Returns a projected `TableSchema` whose `columns` are the entries
// from `schema` named by `names`, in the order `names` lists them.
// Returns INVALID_ARGUMENT (with the offending name) on the first
// unknown column. This is the helper that gives the
// `selected_fields` Storage Read API knob a real projection: the
// `ExecuteSelect` row decoder reads cells back into the projected
// schema's column order so the `Row` shape downstream matches
// exactly what the SELECT projected.
absl::StatusOr<schema::TableSchema> ProjectSchema(
    const schema::TableSchema& schema, absl::Span<const std::string> names) {
  schema::TableSchema out;
  out.columns.reserve(names.size());
  for (const std::string& name : names) {
    const schema::ColumnSchema* match = nullptr;
    for (const auto& col : schema.columns) {
      if (col.name == name) {
        match = &col;
        break;
      }
    }
    if (match == nullptr) {
      return absl::InvalidArgumentError(
          absl::StrCat("selected_fields: unknown column `",
                       name,
                       "` (table has no top-level column with that name)"));
    }
    out.columns.push_back(*match);
  }
  return out;
}

void TryDropTempTable(DuckDBStorage::Impl* impl,
                      absl::string_view qualified_name) {
  RunSql(impl, absl::StrCat("DROP TABLE IF EXISTS ", qualified_name))
      .IgnoreError();
}

// Validate that `id` names an existing dataset + table on disk.
// Returns the on-disk `meta` path on success so callers can read it
// directly instead of recomputing it.
absl::StatusOr<fs::path> EnsureTableMetaExists(const TableId& id,
                                               const fs::path& ds_dir,
                                               const fs::path& meta_path) {
  std::error_code ec;
  if (!fs::exists(ds_dir, ec)) {
    return absl::NotFoundError(
        absl::StrCat("dataset not found: ", id.project_id, ".", id.dataset_id));
  }
  if (!fs::exists(meta_path, ec)) {
    return absl::NotFoundError(absl::StrCat("table not found: ",
                                            id.project_id,
                                            ".",
                                            id.dataset_id,
                                            ".",
                                            id.table_id));
  }
  return meta_path;
}

// Confirm every row's cell count matches the table's column count.
// `tag` flows into the error message so the AppendRows / OverwriteRows
// callers report which entry point caught the mismatch.
absl::Status ValidateRowsShape(absl::string_view tag,
                               const TableId& id,
                               absl::Span<const Row> rows,
                               size_t ncols) {
  for (size_t i = 0; i < rows.size(); ++i) {
    if (rows[i].cells.size() != ncols) {
      return absl::InvalidArgumentError(absl::StrCat(tag,
                                                     ": row[",
                                                     i,
                                                     "] has ",
                                                     rows[i].cells.size(),
                                                     " cell(s) but table ",
                                                     id.project_id,
                                                     ".",
                                                     id.dataset_id,
                                                     ".",
                                                     id.table_id,
                                                     " has ",
                                                     ncols,
                                                     " column(s)"));
    }
  }
  return absl::OkStatus();
}

// Render `INSERT INTO <tmp_table> VALUES (...), (...), ...` for the
// caller's row batch. Returns the literal-render error on the first
// failing cell so the caller can tear the temp table down.
absl::StatusOr<std::string> BuildBatchInsertSql(
    absl::string_view tmp_table,
    absl::Span<const Row> rows,
    const schema::TableSchema& schema) {
  const size_t ncols = schema.columns.size();
  std::string insert_sql;
  absl::StrAppend(&insert_sql, "INSERT INTO ", tmp_table, " VALUES ");
  for (size_t r = 0; r < rows.size(); ++r) {
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

// COPY the temp table to `tmp_path`, then atomically rename it over
// `parquet_path`. Cleans up the temp table + temp file on any error.
// `tag` flows into the error message so AppendRows / OverwriteRows
// surface which caller hit the failure.
absl::Status SnapshotTempTableToParquet(absl::string_view tag,
                                        DuckDBStorage::Impl* impl,
                                        absl::string_view tmp_table,
                                        const std::string& tmp_path,
                                        const std::string& parquet_path) {
  std::error_code ec;
  fs::remove(tmp_path, ec);
  auto status = RunSql(impl,
                       absl::StrCat("COPY ",
                                    tmp_table,
                                    " TO '",
                                    EscapeStringLiteralInner(tmp_path),
                                    "' (FORMAT PARQUET)"));
  if (!status.ok()) {
    TryDropTempTable(impl, std::string(tmp_table));
    fs::remove(tmp_path, ec);
    return status;
  }
  fs::rename(tmp_path, parquet_path, ec);
  if (ec) {
    TryDropTempTable(impl, std::string(tmp_table));
    fs::remove(tmp_path, ec);
    return absl::Status(absl::StatusCode::kInternal,
                        absl::StrCat(tag,
                                     ": failed to rename ",
                                     tmp_path,
                                     " -> ",
                                     parquet_path,
                                     ": ",
                                     ec.message()));
  }
  TryDropTempTable(impl, std::string(tmp_table));
  return absl::OkStatus();
}

}  // namespace internal
}  // namespace duckdb
}  // namespace storage
}  // namespace backend
}  // namespace bigquery_emulator
