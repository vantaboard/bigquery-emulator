#include <filesystem>
#include <string>
#include <system_error>

#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/string_view.h"
#include "absl/types/span.h"
#include "backend/catalog/measure_catalog.h"
#include "backend/schema/schema.h"
#include "backend/storage/duckdb/duckdb_storage_internal.h"
#include "backend/storage/row_restriction.h"
#include "backend/storage/storage.h"
#include "duckdb.h"

namespace bigquery_emulator {
namespace backend {
namespace storage {
namespace duckdb {
namespace internal {

namespace fs = std::filesystem;

schema::TableSchema ParquetStorageSchema(const schema::TableSchema& logical) {
  return catalog::StripMeasureColumns(logical);
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
                    schema::ColumnSchemaToDuckDBStorageType(schema.columns[i]));
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

std::string RenderWhereSqlClause(absl::string_view where_sql) {
  if (where_sql.empty()) return "";
  return absl::StrCat(" WHERE (", where_sql, ")");
}

std::string RenderRowPartitionClause(std::int64_t row_start,
                                     std::int64_t row_end) {
  std::string out;
  if (row_start > 0 || row_end >= 0) {
    absl::StrAppend(&out, " WHERE ");
    bool need_and = false;
    if (row_start > 0) {
      absl::StrAppend(&out, "file_row_number >= ", row_start);
      need_and = true;
    }
    if (row_end >= 0) {
      if (need_and) absl::StrAppend(&out, " AND ");
      absl::StrAppend(&out, "file_row_number < ", row_end);
    }
  }
  return out;
}

absl::StatusOr<std::int64_t> CountParquetRows(DuckDBStorage::Impl* impl,
                                              absl::string_view parquet_path,
                                              absl::string_view where_sql) {
  if (impl == nullptr) {
    return absl::InternalError("CountParquetRows: impl must be non-null");
  }
  std::string sql = absl::StrCat("SELECT COUNT(*) FROM read_parquet('",
                                 EscapeStringLiteralInner(parquet_path),
                                 "', file_row_number = true)");
  absl::StrAppend(&sql, RenderWhereSqlClause(where_sql));
  ::duckdb_result result;
  const std::string sql_str(sql);
  const auto state = ::duckdb_query(impl->connection, sql_str.c_str(), &result);
  if (state != ::DuckDBSuccess) {
    const auto* err = ::duckdb_result_error(&result);
    std::string detail = err == nullptr ? std::string("") : std::string(err);
    ::duckdb_destroy_result(&result);
    return absl::InternalError(
        absl::StrCat("CountParquetRows: duckdb_query failed: ", detail));
  }
  if (::duckdb_row_count(&result) != 1 || ::duckdb_column_count(&result) != 1) {
    ::duckdb_destroy_result(&result);
    return absl::InternalError(
        "CountParquetRows: unexpected COUNT(*) result shape");
  }
  std::int64_t count = 0;
  count = ::duckdb_value_int64(&result, 0, 0);
  ::duckdb_destroy_result(&result);
  return count;
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
