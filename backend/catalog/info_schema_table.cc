#include "backend/catalog/info_schema_table.h"

#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/string_view.h"
#include "absl/types/span.h"
#include "backend/catalog/storage_table.h"
#include "backend/schema/schema.h"
#include "backend/storage/storage.h"
#include "duckdb.h"
#include "googlesql/public/evaluator_table_iterator.h"
#include "googlesql/public/simple_catalog.h"
#include "googlesql/public/type.h"
#include "googlesql/public/value.h"

namespace bigquery_emulator {
namespace backend {
namespace catalog {

namespace {

using ::googlesql::SimpleColumn;
using ::googlesql::SimpleTable;
using ::googlesql::Type;
using ::googlesql::TypeFactory;
using ::googlesql::Value;

std::string QuoteIdent(absl::string_view ident) {
  std::string escaped;
  escaped.reserve(ident.size() + 2);
  escaped.push_back('"');
  for (char c : ident) {
    if (c == '"') escaped.append("\"\"");
    else escaped.push_back(c);
  }
  escaped.push_back('"');
  return escaped;
}

std::string EscapeStringLiteral(absl::string_view s) {
  std::string out;
  out.reserve(s.size());
  for (char c : s) {
    if (c == '\'') out.append("''");
    else out.push_back(c);
  }
  return out;
}

std::string RenderCellLiteral(const storage::Value& cell,
                              const schema::ColumnSchema& column) {
  if (cell.is_null()) return "NULL";
  switch (column.type) {
    case schema::ColumnType::kBool:
      return cell.bool_value() ? "TRUE" : "FALSE";
    case schema::ColumnType::kInt64:
      return std::to_string(cell.int64_value());
    case schema::ColumnType::kFloat64:
      return absl::StrCat(cell.float64_value());
    case schema::ColumnType::kString:
    case schema::ColumnType::kBytes:
      return absl::StrCat("'", EscapeStringLiteral(cell.string_value()), "'");
    default:
      return "NULL";
  }
}

std::string DuckDbTypeForColumn(const schema::ColumnSchema& column) {
  switch (column.type) {
    case schema::ColumnType::kBool:
      return "BOOLEAN";
    case schema::ColumnType::kInt64:
      return "BIGINT";
    case schema::ColumnType::kFloat64:
      return "DOUBLE";
    case schema::ColumnType::kString:
    case schema::ColumnType::kBytes:
      return "VARCHAR";
    default:
      return "VARCHAR";
  }
}

std::string RenderColumnList(const schema::TableSchema& schema) {
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
  if (::duckdb_query(conn, std::string(sql).c_str(), &result) != ::DuckDBSuccess) {
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
                        const schema::TableSchema& schema,
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

std::string InfoSchemaDataType(const schema::ColumnSchema& column) {
  if (column.mode == schema::ColumnMode::kRepeated ||
      column.type == schema::ColumnType::kArray) {
    if (column.fields.empty()) return "ARRAY<INT64>";
    return absl::StrCat("ARRAY<", InfoSchemaDataType(column.fields[0]), ">");
  }
  if (column.type == schema::ColumnType::kStruct) {
    std::string out = "STRUCT<";
    for (size_t i = 0; i < column.fields.size(); ++i) {
      if (i > 0) out.append(", ");
      absl::StrAppend(&out, column.fields[i].name, " ",
                      InfoSchemaDataType(column.fields[i]));
    }
    out.push_back('>');
    return out;
  }
  switch (column.type) {
    case schema::ColumnType::kInt64:
      return "INT64";
    case schema::ColumnType::kFloat64:
      return "DOUBLE";
    case schema::ColumnType::kBool:
      return "BOOL";
    case schema::ColumnType::kString:
      return "STRING";
    case schema::ColumnType::kBytes:
      return "BYTES";
    case schema::ColumnType::kDate:
      return "DATE";
    case schema::ColumnType::kTime:
      return "TIME";
    case schema::ColumnType::kDatetime:
      return "DATETIME";
    case schema::ColumnType::kTimestamp:
      return "TIMESTAMP";
    case schema::ColumnType::kNumeric:
      return "NUMERIC";
    case schema::ColumnType::kBignumeric:
      return "BIGNUMERIC";
    case schema::ColumnType::kJson:
      return "JSON";
    default:
      return column.raw_type.empty() ? "STRING" : column.raw_type;
  }
}

schema::TableSchema TablesViewSchema() {
  return schema::TableSchema{
      .columns =
          {
              {"table_catalog", schema::ColumnType::kString},
              {"table_schema", schema::ColumnType::kString},
              {"table_name", schema::ColumnType::kString},
              {"table_type", schema::ColumnType::kString},
          },
  };
}

schema::TableSchema ColumnsViewSchema() {
  return schema::TableSchema{
      .columns =
          {
              {"table_catalog", schema::ColumnType::kString},
              {"table_schema", schema::ColumnType::kString},
              {"table_name", schema::ColumnType::kString},
              {"column_name", schema::ColumnType::kString},
              {"ordinal_position", schema::ColumnType::kInt64},
              {"is_nullable", schema::ColumnType::kString},
              {"data_type", schema::ColumnType::kString},
          },
  };
}

schema::TableSchema SchemataViewSchema() {
  return schema::TableSchema{
      .columns =
          {
              {"catalog_name", schema::ColumnType::kString},
              {"schema_name", schema::ColumnType::kString},
          },
  };
}

std::vector<SimpleTable::NameAndType> ColumnsForView(InfoSchemaViewKind kind,
                                                      TypeFactory* factory) {
  std::vector<SimpleTable::NameAndType> out;
  auto str = [&]() { return factory->get_string(); };
  auto i64 = [&]() { return factory->get_int64(); };
  switch (kind) {
    case InfoSchemaViewKind::kTables:
      out = {{"table_catalog", str()}, {"table_schema", str()},
             {"table_name", str()},     {"table_type", str()}};
      break;
    case InfoSchemaViewKind::kColumns:
      out = {{"table_catalog", str()},    {"table_schema", str()},
             {"table_name", str()},       {"column_name", str()},
             {"ordinal_position", i64()}, {"is_nullable", str()},
             {"data_type", str()}};
      break;
    case InfoSchemaViewKind::kSchemata:
      out = {{"catalog_name", str()}, {"schema_name", str()}};
      break;
  }
  return out;
}

class InfoSchemaEvaluatorIterator : public ::googlesql::EvaluatorTableIterator {
 public:
  InfoSchemaEvaluatorIterator(
      std::vector<storage::Row> rows,
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
      row_schema_(kind == InfoSchemaViewKind::kTables
                      ? TablesViewSchema()
                      : kind == InfoSchemaViewKind::kColumns
                            ? ColumnsViewSchema()
                            : SchemataViewSchema()) {
  (void)set_full_name(std::string(full_name));
}

absl::StatusOr<std::vector<storage::Row>> InfoSchemaTable::GenerateRows()
    const {
  std::vector<storage::Row> rows;
  if (storage_ == nullptr) {
    return absl::FailedPreconditionError(
        "InfoSchemaTable: storage backend is not configured");
  }

  auto append_dataset_tables =
      [&](absl::string_view dataset_id) -> absl::Status {
    storage::DatasetId ds_id{project_id_, std::string(dataset_id)};
    absl::StatusOr<std::vector<storage::TableId>> tables =
        storage_->ListTables(ds_id);
    if (!tables.ok()) return tables.status();
    for (const storage::TableId& table : *tables) {
      if (kind_ == InfoSchemaViewKind::kTables) {
        rows.push_back(storage::Row{
            .cells =
                {
                    storage::Value::String(project_id_),
                    storage::Value::String(table.dataset_id),
                    storage::Value::String(table.table_id),
                    storage::Value::String("BASE TABLE"),
                },
        });
        continue;
      }
      if (kind_ != InfoSchemaViewKind::kColumns) continue;
      absl::StatusOr<schema::TableSchema> schema = storage_->GetSchema(table);
      if (!schema.ok()) return schema.status();
      for (size_t i = 0; i < schema->columns.size(); ++i) {
        const schema::ColumnSchema& column = schema->columns[i];
        const std::string nullable =
            column.mode == schema::ColumnMode::kRequired ? "NO" : "YES";
        rows.push_back(storage::Row{
            .cells =
                {
                    storage::Value::String(project_id_),
                    storage::Value::String(table.dataset_id),
                    storage::Value::String(table.table_id),
                    storage::Value::String(column.name),
                    storage::Value::Int64(static_cast<int64_t>(i + 1)),
                    storage::Value::String(nullable),
                    storage::Value::String(InfoSchemaDataType(column)),
                },
        });
      }
    }
    return absl::OkStatus();
  };

  switch (kind_) {
    case InfoSchemaViewKind::kSchemata: {
      absl::StatusOr<std::vector<storage::DatasetId>> datasets =
          storage_->ListDatasets(project_id_);
      if (!datasets.ok()) return datasets.status();
      for (const storage::DatasetId& ds : *datasets) {
        rows.push_back(storage::Row{
            .cells =
                {
                    storage::Value::String(project_id_),
                    storage::Value::String(ds.dataset_id),
                },
        });
      }
      return rows;
    }
    case InfoSchemaViewKind::kTables:
    case InfoSchemaViewKind::kColumns: {
      if (!dataset_id_.empty()) {
        absl::Status s = append_dataset_tables(dataset_id_);
        if (!s.ok()) return s;
        return rows;
      }
      absl::StatusOr<std::vector<storage::DatasetId>> datasets =
          storage_->ListDatasets(project_id_);
      if (!datasets.ok()) return datasets.status();
      for (const storage::DatasetId& ds : *datasets) {
        absl::Status s = append_dataset_tables(ds.dataset_id);
        if (!s.ok()) return s;
      }
      return rows;
    }
  }
  return rows;
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
    for (size_t i = 0; i < row_schema_.columns.size(); ++i) idxs[i] = i;
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
  return std::make_unique<InfoSchemaEvaluatorIterator>(
      std::move(*rows), row_schema_, std::move(idxs), std::move(names),
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
  absl::Status create = RunSqlNoResult(
      conn, absl::StrCat("CREATE OR REPLACE TABLE ", table_name, " ",
                         RenderColumnList(row_schema_)));
  if (!create.ok()) return create;
  return InsertRows(conn, table_name, row_schema_, *rows);
}

}  // namespace catalog
}  // namespace backend
}  // namespace bigquery_emulator
