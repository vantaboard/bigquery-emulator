#include "backend/catalog/info_schema_table.h"

#include <cstddef>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/string_view.h"
#include "absl/types/span.h"
#include "backend/catalog/info_schema_internal.h"
#include "backend/catalog/storage_table.h"
#include "backend/schema/schema.h"
#include "backend/storage/storage.h"
#include "duckdb.h"
#include "googlesql/public/evaluator_table_iterator.h"
#include "googlesql/public/simple_catalog.h"
#include "googlesql/public/type.h"
#include "googlesql/public/types/array_type.h"
#include "googlesql/public/types/type_factory.h"
#include "googlesql/public/value.h"

namespace bigquery_emulator {
namespace backend {
namespace catalog {

namespace {

using ::googlesql::ArrayType;
using ::googlesql::SimpleTable;
using ::googlesql::Type;
using ::googlesql::TypeFactory;
using ::googlesql::Value;

using schema::ColumnMode;
using schema::ColumnSchema;
using schema::ColumnType;
using schema::TableSchema;

std::string QuoteIdent(absl::string_view ident) {
  std::string escaped;
  escaped.reserve(ident.size() + 2);
  escaped.push_back('"');
  for (char c : ident) {
    if (c == '"')
      escaped.append("\"\"");
    else
      escaped.push_back(c);
  }
  escaped.push_back('"');
  return escaped;
}

std::string EscapeStringLiteral(absl::string_view s) {
  std::string out;
  out.reserve(s.size());
  for (char c : s) {
    if (c == '\'')
      out.append("''");
    else
      out.push_back(c);
  }
  return out;
}

std::string RenderCellLiteral(const storage::Value& cell,
                              const ColumnSchema& column) {
  if (cell.is_null()) return "NULL";
  switch (column.type) {
    case ColumnType::kBool:
      return cell.bool_value() ? "TRUE" : "FALSE";
    case ColumnType::kInt64:
      return std::to_string(cell.int64_value());
    case ColumnType::kFloat64:
      return absl::StrCat(cell.float64_value());
    case ColumnType::kString:
    case ColumnType::kBytes:
      return absl::StrCat("'", EscapeStringLiteral(cell.string_value()), "'");
    default:
      // TIMESTAMP / DATE / ARRAY cells in INFORMATION_SCHEMA views are
      // always materialized as NULL (the emulator does not track the
      // underlying metadata), so the null branch above covers them.
      return "NULL";
  }
}

// DuckDB column type for the materialized in-memory table. Mirrors the
// GoogleSQL types produced by `ViewColumnType` so the transpiled query
// sees consistent column types.
std::string DuckDbTypeForColumn(const ColumnSchema& column) {
  absl::string_view scalar;
  switch (column.type) {
    case ColumnType::kBool:
      scalar = "BOOLEAN";
      break;
    case ColumnType::kInt64:
      scalar = "BIGINT";
      break;
    case ColumnType::kFloat64:
      scalar = "DOUBLE";
      break;
    case ColumnType::kTimestamp:
      scalar = "TIMESTAMP";
      break;
    case ColumnType::kDate:
      scalar = "DATE";
      break;
    case ColumnType::kString:
    case ColumnType::kBytes:
    default:
      scalar = "VARCHAR";
      break;
  }
  if (column.mode == ColumnMode::kRepeated) {
    return absl::StrCat(scalar, "[]");
  }
  return std::string(scalar);
}

std::string RenderColumnList(const TableSchema& schema) {
  std::string out = "(";
  for (size_t i = 0; i < schema.columns.size(); ++i) {
    if (i > 0) out.append(", ");
    out.append(QuoteIdent(schema.columns[i].name));
    out.push_back(' ');
    out.append(DuckDbTypeForColumn(schema.columns[i]));
  }
  out.push_back(')');
  return out;
}

absl::Status RunSqlNoResult(::duckdb_connection conn, absl::string_view sql) {
  ::duckdb_result result;
  if (::duckdb_query(conn, std::string(sql).c_str(), &result) !=
      ::DuckDBSuccess) {
    const char* err = ::duckdb_result_error(&result);
    std::string detail = err == nullptr ? std::string("") : std::string(err);
    ::duckdb_destroy_result(&result);
    return absl::InternalError(
        absl::StrCat("InfoSchemaTable: DuckDB rejected SQL: ", detail));
  }
  ::duckdb_destroy_result(&result);
  return absl::OkStatus();
}

absl::Status InsertRows(::duckdb_connection conn,
                        absl::string_view quoted_table_name,
                        const TableSchema& schema,
                        absl::Span<const storage::Row> rows) {
  if (rows.empty()) return absl::OkStatus();
  std::string insert_sql;
  absl::StrAppend(&insert_sql, "INSERT INTO ", quoted_table_name, " VALUES ");
  for (size_t r = 0; r < rows.size(); ++r) {
    if (rows[r].cells.size() != schema.columns.size()) {
      return absl::InvalidArgumentError("InfoSchemaTable: row width mismatch");
    }
    if (r > 0) insert_sql.append(", ");
    insert_sql.push_back('(');
    for (size_t c = 0; c < schema.columns.size(); ++c) {
      if (c > 0) insert_sql.append(", ");
      insert_sql.append(RenderCellLiteral(rows[r].cells[c], schema.columns[c]));
    }
    insert_sql.push_back(')');
  }
  return RunSqlNoResult(conn, insert_sql);
}

absl::StatusOr<const Type*> ViewColumnType(const ColumnSchema& column,
                                           TypeFactory* factory) {
  const Type* scalar = nullptr;
  switch (column.type) {
    case ColumnType::kBool:
      scalar = factory->get_bool();
      break;
    case ColumnType::kInt64:
      scalar = factory->get_int64();
      break;
    case ColumnType::kFloat64:
      scalar = factory->get_double();
      break;
    case ColumnType::kTimestamp:
      scalar = factory->get_timestamp();
      break;
    case ColumnType::kDate:
      scalar = factory->get_date();
      break;
    case ColumnType::kString:
    case ColumnType::kBytes:
    default:
      scalar = factory->get_string();
      break;
  }
  if (column.mode == ColumnMode::kRepeated) {
    const ArrayType* array_type = nullptr;
    absl::Status s = factory->MakeArrayType(scalar, &array_type);
    if (!s.ok()) return s;
    return array_type;
  }
  return scalar;
}

std::vector<SimpleTable::NameAndType> ColumnsForView(InfoSchemaViewKind kind,
                                                     TypeFactory* factory) {
  const TableSchema schema = info_schema_internal::RowSchemaForView(kind);
  std::vector<SimpleTable::NameAndType> out;
  out.reserve(schema.columns.size());
  for (const ColumnSchema& column : schema.columns) {
    absl::StatusOr<const Type*> type = ViewColumnType(column, factory);
    out.push_back({column.name, type.ok() ? *type : factory->get_string()});
  }
  return out;
}

class InfoSchemaEvaluatorIterator : public ::googlesql::EvaluatorTableIterator {
 public:
  InfoSchemaEvaluatorIterator(std::vector<storage::Row> rows,
                              TableSchema schema,
                              std::vector<int> column_idxs,
                              std::vector<std::string> column_names,
                              std::vector<const Type*> column_types)
      : rows_(std::move(rows)),
        schema_(std::move(schema)),
        column_idxs_(std::move(column_idxs)),
        column_names_(std::move(column_names)),
        column_types_(std::move(column_types)) {}

  int NumColumns() const override {
    return static_cast<int>(column_idxs_.size());
  }
  std::string GetColumnName(int i) const override {
    return column_names_[i];
  }
  const Type* GetColumnType(int i) const override {
    return column_types_[i];
  }

  bool NextRow() override {
    if (row_idx_ >= rows_.size()) {
      current_row_.clear();
      return false;
    }
    const storage::Row& row = rows_[row_idx_++];
    current_row_.clear();
    current_row_.reserve(column_idxs_.size());
    for (size_t i = 0; i < column_idxs_.size(); ++i) {
      const int src = column_idxs_[i];
      const storage::Value& cell = row.cells[src];
      if (cell.is_null()) {
        current_row_.push_back(Value::Null(column_types_[i]));
        continue;
      }
      absl::StatusOr<Value> converted =
          StorageValueToGoogleSqlValue(cell, column_types_[i]);
      if (!converted.ok()) {
        status_ = converted.status();
        current_row_.clear();
        return false;
      }
      current_row_.push_back(*converted);
    }
    return true;
  }

  const Value& GetValue(int i) const override {
    return current_row_[i];
  }
  absl::Status Status() const override {
    return status_;
  }
  absl::Status Cancel() override {
    return absl::OkStatus();
  }

 private:
  std::vector<storage::Row> rows_;
  TableSchema schema_;
  std::vector<int> column_idxs_;
  std::vector<std::string> column_names_;
  std::vector<const Type*> column_types_;
  size_t row_idx_ = 0;
  std::vector<Value> current_row_;
  absl::Status status_;
};

}  // namespace

InfoSchemaTable::InfoSchemaTable(absl::string_view view_name,
                                 absl::string_view full_name,
                                 InfoSchemaViewKind kind,
                                 absl::string_view project_id,
                                 absl::string_view dataset_id,
                                 const storage::Storage* storage,
                                 TypeFactory* type_factory)
    : VirtualCatalogTable(std::string(view_name),
                          ColumnsForView(kind, type_factory)),
      kind_(kind),
      project_id_(project_id),
      dataset_id_(dataset_id),
      storage_(storage),
      type_factory_(type_factory),
      row_schema_(info_schema_internal::RowSchemaForView(kind)) {
  (void)set_full_name(std::string(full_name));
}

absl::StatusOr<std::unique_ptr<::googlesql::EvaluatorTableIterator>>
InfoSchemaTable::CreateEvaluatorTableIterator(
    absl::Span<const int> column_idxs) const {
  absl::StatusOr<std::vector<storage::Row>> rows = GenerateRows();
  if (!rows.ok()) return rows.status();

  std::vector<int> idxs;
  std::vector<std::string> names;
  std::vector<const Type*> types;
  if (column_idxs.empty()) {
    idxs.resize(row_schema_.columns.size());
    for (size_t i = 0; i < row_schema_.columns.size(); ++i)
      idxs[i] = i;
  } else {
    idxs.assign(column_idxs.begin(), column_idxs.end());
  }
  names.reserve(idxs.size());
  types.reserve(idxs.size());
  for (int idx : idxs) {
    if (idx < 0 || idx >= NumColumns()) {
      return absl::InvalidArgumentError("InfoSchemaTable: bad column index");
    }
    names.push_back(GetColumn(idx)->Name());
    types.push_back(GetColumn(idx)->GetType());
  }
  return std::make_unique<InfoSchemaEvaluatorIterator>(std::move(*rows),
                                                       row_schema_,
                                                       std::move(idxs),
                                                       std::move(names),
                                                       std::move(types));
}

absl::Status InfoSchemaTable::MaterializeInDuckDB(
    ::duckdb_connection conn,
    const storage::Storage* storage,
    absl::string_view quoted_table_name) const {
  (void)storage;
  absl::StatusOr<std::vector<storage::Row>> rows = GenerateRows();
  if (!rows.ok()) return rows.status();
  const std::string table_name(quoted_table_name);
  absl::Status create =
      RunSqlNoResult(conn,
                     absl::StrCat("CREATE OR REPLACE TABLE ",
                                  table_name,
                                  " ",
                                  RenderColumnList(row_schema_)));
  if (!create.ok()) return create;
  return InsertRows(conn, table_name, row_schema_, *rows);
}

}  // namespace catalog
}  // namespace backend
}  // namespace bigquery_emulator
