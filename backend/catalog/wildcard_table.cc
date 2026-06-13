#include "backend/catalog/wildcard_table.h"

#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/string_view.h"
#include "backend/catalog/storage_table.h"
#include "backend/catalog/virtual_table.h"
#include "duckdb.h"
#include "googlesql/public/evaluator_table_iterator.h"
#include "googlesql/public/type.h"
#include "googlesql/public/value.h"

namespace bigquery_emulator {
namespace backend {
namespace catalog {

namespace {

using ::googlesql::Type;
using ::googlesql::Value;
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
    case ColumnType::kNumeric:
    case ColumnType::kBignumeric:
      return absl::StrCat("'", EscapeStringLiteral(cell.string_value()), "'");
    default:
      return "NULL";
  }
}

std::string DuckDbTypeForColumn(const ColumnSchema& column) {
  switch (column.type) {
    case ColumnType::kBool:
      return "BOOLEAN";
    case ColumnType::kInt64:
      return "BIGINT";
    case ColumnType::kFloat64:
      return "DOUBLE";
    case ColumnType::kString:
    case ColumnType::kBytes:
    case ColumnType::kNumeric:
    case ColumnType::kBignumeric:
      return "VARCHAR";
    default:
      return "VARCHAR";
  }
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
        absl::StrCat("WildcardTable: DuckDB rejected SQL: ", detail));
  }
  ::duckdb_destroy_result(&result);
  return absl::OkStatus();
}

storage::Value NullStorageCell(const ColumnSchema& column) {
  return storage::Value::Null();
}

storage::Row BuildUnionRow(const storage::Row& physical,
                           const WildcardColumnMap& map,
                           const TableSchema& union_schema,
                           absl::string_view table_suffix) {
  storage::Row out;
  out.cells.resize(union_schema.columns.size(), storage::Value::Null());
  for (size_t u = 0; u < union_schema.columns.size(); ++u) {
    const ColumnSchema& col = union_schema.columns[u];
    if (col.name == kTableSuffixColumnName) {
      out.cells[u] = storage::Value::String(std::string(table_suffix));
      continue;
    }
    const int physical_idx = map.union_to_physical[u];
    if (physical_idx < 0 ||
        static_cast<size_t>(physical_idx) >= physical.cells.size()) {
      out.cells[u] = NullStorageCell(col);
      continue;
    }
    out.cells[u] = physical.cells[physical_idx];
  }
  return out;
}

class WildcardEvaluatorIterator : public ::googlesql::EvaluatorTableIterator {
 public:
  WildcardEvaluatorIterator(std::vector<storage::Row> rows,
                            schema::TableSchema schema,
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
  schema::TableSchema schema_;
  std::vector<int> column_idxs_;
  std::vector<std::string> column_names_;
  std::vector<const Type*> column_types_;
  size_t row_idx_ = 0;
  std::vector<Value> current_row_;
  absl::Status status_;
};

}  // namespace

WildcardTable::WildcardTable(absl::string_view wildcard_table_id,
                             absl::string_view full_name,
                             storage::TableId wildcard_id,
                             std::string table_prefix,
                             std::vector<WildcardColumnMap> matched_tables,
                             schema::TableSchema union_schema,
                             absl::Span<const NameAndType> columns,
                             const storage::Storage* storage,
                             ::googlesql::TypeFactory* type_factory)
    : VirtualCatalogTable(std::string(wildcard_table_id), columns),
      wildcard_id_(std::move(wildcard_id)),
      table_prefix_(std::move(table_prefix)),
      matched_tables_(std::move(matched_tables)),
      union_schema_(std::move(union_schema)),
      storage_(storage),
      type_factory_(type_factory) {
  (void)set_full_name(std::string(full_name));
  (void)type_factory_;
}

absl::StatusOr<std::vector<storage::Row>> WildcardTable::CollectUnionRows(
    const std::optional<std::vector<std::string>>& suffix_allowlist) const {
  if (storage_ == nullptr) {
    return absl::FailedPreconditionError(
        "WildcardTable: storage backend is not configured");
  }
  std::vector<storage::Row> rows;
  for (const WildcardColumnMap& map : matched_tables_) {
    const std::string suffix =
        TableSuffixFor(map.table_id.table_id, table_prefix_);
    if (suffix_allowlist.has_value() &&
        !SuffixMatchesAllowList(suffix, *suffix_allowlist)) {
      continue;
    }
    absl::StatusOr<std::unique_ptr<storage::RowIterator>> iter =
        storage_->ScanRows(map.table_id);
    if (!iter.ok()) return iter.status();
    storage::Row physical;
    while (true) {
      absl::StatusOr<bool> has = (*iter)->Next(&physical);
      if (!has.ok()) return has.status();
      if (!*has) break;
      rows.push_back(BuildUnionRow(physical, map, union_schema_, suffix));
    }
  }
  return rows;
}

absl::StatusOr<std::unique_ptr<::googlesql::EvaluatorTableIterator>>
WildcardTable::CreateEvaluatorTableIterator(
    absl::Span<const int> column_idxs) const {
  absl::StatusOr<std::vector<storage::Row>> rows =
      CollectUnionRows(std::nullopt);
  if (!rows.ok()) return rows.status();

  std::vector<int> idxs;
  std::vector<std::string> names;
  std::vector<const Type*> types;
  if (column_idxs.empty()) {
    idxs.resize(union_schema_.columns.size());
    for (size_t i = 0; i < union_schema_.columns.size(); ++i)
      idxs[i] = i;
  } else {
    idxs.assign(column_idxs.begin(), column_idxs.end());
  }
  names.reserve(idxs.size());
  types.reserve(idxs.size());
  for (int idx : idxs) {
    if (idx < 0 || idx >= NumColumns()) {
      return absl::InvalidArgumentError("WildcardTable: bad column index");
    }
    names.push_back(GetColumn(idx)->Name());
    types.push_back(GetColumn(idx)->GetType());
  }
  return std::make_unique<WildcardEvaluatorIterator>(std::move(*rows),
                                                     union_schema_,
                                                     std::move(idxs),
                                                     std::move(names),
                                                     std::move(types));
}

absl::Status WildcardTable::MaterializeInDuckDB(
    ::duckdb_connection conn,
    const storage::Storage* storage,
    absl::string_view quoted_table_name) const {
  return MaterializeInDuckDBWithSuffixAllowList(
      conn, storage, quoted_table_name, std::nullopt);
}

absl::Status WildcardTable::MaterializeInDuckDBWithSuffixAllowList(
    ::duckdb_connection conn,
    const storage::Storage* storage,
    absl::string_view quoted_table_name,
    const std::optional<std::vector<std::string>>& suffix_allowlist) const {
  if (storage == nullptr) {
    return absl::FailedPreconditionError(
        "WildcardTable: storage backend is not configured");
  }
  const std::string table_name(quoted_table_name);
  absl::Status create =
      RunSqlNoResult(conn,
                     absl::StrCat("CREATE OR REPLACE TABLE ",
                                  table_name,
                                  " ",
                                  RenderColumnList(union_schema_)));
  if (!create.ok()) return create;

  absl::StatusOr<std::vector<storage::Row>> rows =
      CollectUnionRows(suffix_allowlist);
  if (!rows.ok()) return rows.status();
  if (rows->empty()) return absl::OkStatus();

  std::string insert_sql;
  absl::StrAppend(&insert_sql, "INSERT INTO ", table_name, " VALUES ");
  for (size_t r = 0; r < rows->size(); ++r) {
    if (r > 0) insert_sql.append(", ");
    insert_sql.push_back('(');
    for (size_t c = 0; c < union_schema_.columns.size(); ++c) {
      if (c > 0) insert_sql.append(", ");
      insert_sql.append(
          RenderCellLiteral((*rows)[r].cells[c], union_schema_.columns[c]));
    }
    insert_sql.push_back(')');
  }
  return RunSqlNoResult(conn, insert_sql);
}

}  // namespace catalog
}  // namespace backend
}  // namespace bigquery_emulator
