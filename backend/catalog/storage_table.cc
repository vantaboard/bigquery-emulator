#include "backend/catalog/storage_table.h"

#include <cstddef>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "absl/strings/cord.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/string_view.h"
#include "absl/types/span.h"
#include "backend/engine/semantic/value.h"
#include "backend/schema/schema.h"
#include "backend/storage/storage.h"
#include "googlesql/public/evaluator_table_iterator.h"
#include "googlesql/public/numeric_value.h"
#include "googlesql/public/simple_catalog.h"
#include "googlesql/public/type.h"
#include "googlesql/public/types/array_type.h"
#include "googlesql/public/types/proto_type.h"
#include "googlesql/public/types/struct_type.h"
#include "googlesql/public/value.h"

namespace bigquery_emulator {
namespace backend {
namespace catalog {

namespace {

// Translate a storage `Value` to a `googlesql::Value` of `type`,
// recursing for ARRAY / STRUCT containers. The column schema is
// threaded alongside so STRUCT fields can pick up their per-field
// types in the order GoogleSQL expects.
absl::StatusOr<::googlesql::Value> ConvertCell(
    const storage::Value& value,
    const ::googlesql::Type* type,
    const schema::ColumnSchema& column);

// Project columns: given the full row from storage, return the
// `googlesql::Value` for each requested column index. The cell at
// index `i` corresponds to `bq_schema.columns[i]`; the iterator
// stores its own per-column googlesql `Type*` snapshot in parallel.
absl::Status ProjectRowCells(
    const storage::Row& row,
    const schema::TableSchema& bq_schema,
    absl::Span<const int> column_idxs,
    absl::Span<const ::googlesql::Type* const> column_types,
    std::vector<::googlesql::Value>* out) {
  out->clear();
  out->reserve(column_idxs.size());
  for (std::vector<int>::size_type i = 0; i < column_idxs.size(); ++i) {
    const int idx = column_idxs[i];
    if (idx < 0 || static_cast<size_t>(idx) >= row.cells.size()) {
      return absl::InvalidArgumentError(
          absl::StrCat("StorageTable iterator: requested column index ",
                       idx,
                       " is outside the row's ",
                       row.cells.size(),
                       " cells"));
    }
    absl::StatusOr<::googlesql::Value> converted =
        ConvertCell(row.cells[idx], column_types[i], bq_schema.columns[idx]);
    if (!converted.ok()) return converted.status();
    out->push_back(std::move(converted).value());
  }
  return absl::OkStatus();
}

absl::StatusOr<::googlesql::Value> ConvertStringTypedScalar(
    absl::string_view string_value,
    const ::googlesql::Type* type,
    absl::string_view column_name) {
  if (type->kind() == ::googlesql::TYPE_NUMERIC) {
    auto n = ::googlesql::NumericValue::FromString(string_value);
    if (!n.ok()) {
      return absl::InvalidArgumentError(
          absl::StrCat("StorageTable iterator: column '",
                       column_name,
                       "': invalid NUMERIC value '",
                       string_value,
                       "': ",
                       n.status().message()));
    }
    return ::googlesql::Value::Numeric(*n);
  }
  if (type->kind() == ::googlesql::TYPE_BIGNUMERIC) {
    auto n = ::googlesql::BigNumericValue::FromString(string_value);
    if (!n.ok()) {
      return absl::InvalidArgumentError(
          absl::StrCat("StorageTable iterator: column '",
                       column_name,
                       "': invalid BIGNUMERIC value '",
                       string_value,
                       "': ",
                       n.status().message()));
    }
    return ::googlesql::Value::BigNumeric(*n);
  }
  if (type->kind() == ::googlesql::TYPE_TIMESTAMP) {
    auto parsed = engine::semantic::ParseTimestampWireString(string_value);
    if (!parsed.ok()) {
      return parsed.status();
    }
    return *parsed;
  }
  return ::googlesql::Value::StringValue(string_value);
}

absl::StatusOr<::googlesql::Value> ConvertStructScalar(
    const storage::Value& value,
    const ::googlesql::Type* type,
    const schema::ColumnSchema& column) {
  if (type == nullptr || !type->IsStruct()) {
    return absl::InvalidArgumentError(
        absl::StrCat("StorageTable iterator: column '",
                     column.name,
                     "' carries a STRUCT cell but the GoogleSQL type is ",
                     type == nullptr ? "<null>" : type->DebugString()));
  }
  const ::googlesql::StructType* st = type->AsStruct();
  const std::vector<storage::Value>& fields = value.struct_value();
  if (static_cast<int>(fields.size()) != st->num_fields() ||
      fields.size() != column.fields.size()) {
    return absl::InvalidArgumentError(
        absl::StrCat("StorageTable iterator: column '",
                     column.name,
                     "' STRUCT has ",
                     fields.size(),
                     " field(s) but schema has ",
                     column.fields.size(),
                     " and analyzer Type has ",
                     st->num_fields()));
  }
  std::vector<::googlesql::Value> converted;
  converted.reserve(fields.size());
  for (int i = 0; i < st->num_fields(); ++i) {
    absl::StatusOr<::googlesql::Value> field_value =
        ConvertCell(fields[i], st->field(i).type, column.fields[i]);
    if (!field_value.ok()) return field_value.status();
    converted.push_back(std::move(field_value).value());
  }
  return ::googlesql::Value::Struct(st, converted);
}

absl::StatusOr<::googlesql::Value> ConvertSimpleScalarKind(
    const storage::Value& value,
    const ::googlesql::Type* type,
    const schema::ColumnSchema& column) {
  using Kind = storage::Value::Kind;
  switch (value.kind()) {
    case Kind::kNull:
      return ::googlesql::Value::Null(type);
    case Kind::kBool:
      return ::googlesql::Value::Bool(value.bool_value());
    case Kind::kInt64:
      return ::googlesql::Value::Int64(value.int64_value());
    case Kind::kFloat64:
      return ::googlesql::Value::Double(value.float64_value());
    case Kind::kString:
      return ::googlesql::Value::StringValue(value.string_value());
    case Kind::kBytes:
      if (type != nullptr && type->kind() == ::googlesql::TYPE_PROTO) {
        const auto* proto_type = type->AsProto();
        if (proto_type == nullptr) {
          return absl::InvalidArgumentError(
              "StorageTable iterator: PROTO column missing ProtoType payload");
        }
        return ::googlesql::Value::Proto(proto_type,
                                         absl::Cord(value.string_value()));
      }
      return ::googlesql::Value::Bytes(value.string_value());
    case Kind::kStruct:
      return ConvertStructScalar(value, type, column);
    case Kind::kArray:
      return absl::InvalidArgumentError(
          absl::StrCat("StorageTable iterator: column '",
                       column.name,
                       "': unexpected ARRAY cell while converting scalar type ",
                       type == nullptr ? "<null>" : type->DebugString()));
  }
  return absl::InvalidArgumentError(
      absl::StrCat("StorageTable iterator: column '",
                   column.name,
                   "': unhandled storage::Value::Kind ",
                   static_cast<int>(value.kind())));
}

absl::StatusOr<::googlesql::Value> ConvertScalar(
    const storage::Value& value,
    const ::googlesql::Type* type,
    const schema::ColumnSchema& column) {
  using Kind = storage::Value::Kind;
  // NUMERIC / BIGNUMERIC are persisted as decimal text (DuckDB
  // VARCHAR storage type preserves the full magnitude), so the
  // storage cell arrives as kString. The semantic executor's
  // decimal path (arithmetic, AVG, MIN/MAX) needs a real
  // NumericValue / BigNumericValue, not a STRING, so parse against
  // the analyzer's target type here rather than mis-typing the cell
  // as STRING.
  if (type != nullptr && value.kind() == Kind::kString) {
    return ConvertStringTypedScalar(value.string_value(), type, column.name);
  }
  return ConvertSimpleScalarKind(value, type, column);
}

absl::StatusOr<::googlesql::Value> ConvertCell(
    const storage::Value& value,
    const ::googlesql::Type* type,
    const schema::ColumnSchema& column) {
  if (type == nullptr) {
    return absl::InvalidArgumentError(absl::StrCat(
        "StorageTable iterator: column '", column.name, "': null target type"));
  }
  // ARRAY column: storage stores the elements in a single kArray
  // cell whose contents are typed by the column's nested element
  // schema (column.fields[0] is the element). A NULL ARRAY arrives
  // as a kNull cell; treat that the same way scalars do.
  if (type->IsArray()) {
    if (value.kind() == storage::Value::Kind::kNull) {
      return ::googlesql::Value::Null(type);
    }
    if (value.kind() != storage::Value::Kind::kArray) {
      return absl::InvalidArgumentError(
          absl::StrCat("StorageTable iterator: column '",
                       column.name,
                       "' is ARRAY but cell kind is ",
                       static_cast<int>(value.kind())));
    }
    const ::googlesql::ArrayType* at = type->AsArray();
    const ::googlesql::Type* element_type = at->element_type();
    // For an ARRAY column the schema's `fields` list carries either
    // (a) one entry describing the element schema (legacy ARRAY-as-
    // container case) OR (b) is empty when the cardinality was
    // signalled via `mode = REPEATED` on the column itself. The
    // analyzer Type already knows the element type either way; we
    // build a synthetic element ColumnSchema with the column's
    // top-level metadata so error messages still point at the
    // offending field.
    schema::ColumnSchema element_schema;
    if (!column.fields.empty()) {
      element_schema = column.fields.front();
    } else {
      element_schema = column;
      element_schema.mode = schema::ColumnMode::kNullable;
      element_schema.fields.clear();
    }
    const std::vector<storage::Value>& elements = value.array_value();
    std::vector<::googlesql::Value> converted;
    converted.reserve(elements.size());
    for (const storage::Value& element : elements) {
      absl::StatusOr<::googlesql::Value> v =
          ConvertCell(element, element_type, element_schema);
      if (!v.ok()) return v.status();
      converted.push_back(std::move(v).value());
    }
    return ::googlesql::Value::Array(at, converted);
  }
  return ConvertScalar(value, type, column);
}

// `EvaluatorTableIterator` implementation that walks a Storage
// `RowIterator`, projects the columns the evaluator asked for, and
// converts each cell on demand.
class StorageEvaluatorTableIterator
    : public ::googlesql::EvaluatorTableIterator {
 public:
  StorageEvaluatorTableIterator(
      std::unique_ptr<storage::RowIterator> rows_iter,
      schema::TableSchema bq_schema,
      std::vector<int> column_idxs,
      std::vector<std::string> column_names,
      std::vector<const ::googlesql::Type*> column_types)
      : rows_iter_(std::move(rows_iter)),
        bq_schema_(std::move(bq_schema)),
        column_idxs_(std::move(column_idxs)),
        column_names_(std::move(column_names)),
        column_types_(std::move(column_types)) {}

  ~StorageEvaluatorTableIterator() override = default;

  int NumColumns() const override {
    return static_cast<int>(column_idxs_.size());
  }
  std::string GetColumnName(int i) const override {
    return column_names_[i];
  }
  const ::googlesql::Type* GetColumnType(int i) const override {
    return column_types_[i];
  }

  bool NextRow() override {
    storage::Row row;
    absl::StatusOr<bool> has = rows_iter_->Next(&row);
    if (!has.ok()) {
      status_ = has.status();
      current_row_.clear();
      return false;
    }
    if (!*has) {
      current_row_.clear();
      return false;
    }
    absl::Status s = ProjectRowCells(
        row, bq_schema_, column_idxs_, column_types_, &current_row_);
    if (!s.ok()) {
      status_ = s;
      current_row_.clear();
      return false;
    }
    return true;
  }

  const ::googlesql::Value& GetValue(int i) const override {
    return current_row_[i];
  }

  absl::Status Status() const override {
    return status_;
  }
  absl::Status Cancel() override {
    return absl::OkStatus();
  }

 private:
  std::unique_ptr<storage::RowIterator> rows_iter_{};
  schema::TableSchema bq_schema_{};
  std::vector<int> column_idxs_{};
  std::vector<std::string> column_names_{};
  std::vector<const ::googlesql::Type*> column_types_{};
  std::vector<::googlesql::Value> current_row_{};
  absl::Status status_{};
};

}  // namespace

absl::StatusOr<::googlesql::Value> StorageValueToGoogleSqlValue(
    const storage::Value& value, const ::googlesql::Type* type) {
  schema::ColumnSchema synthetic;
  synthetic.name = "<value>";
  return ConvertCell(value, type, synthetic);
}

absl::StatusOr<::googlesql::Value> StorageValueToGoogleSqlValue(
    const storage::Value& value,
    const ::googlesql::Type* type,
    const schema::ColumnSchema& column) {
  return ConvertCell(value, type, column);
}

StorageTable::StorageTable(absl::string_view name,
                           absl::string_view full_name,
                           absl::Span<const NameAndType> columns,
                           schema::TableSchema bq_schema,
                           storage::TableId table_id,
                           const storage::Storage* storage)
    : ::googlesql::SimpleTable(name, columns),
      bq_schema_(std::move(bq_schema)),
      table_id_(std::move(table_id)),
      storage_(storage) {
  // SimpleTable defaults the full name to the table name; the
  // catalog typically wants the dotted `project.dataset.table` shape
  // in error messages. The setter returns absl::Status but is
  // infallible for our inputs, so we discard.
  (void)set_full_name(full_name);

  // Designate the first column as the (synthetic) primary key. This
  // is vestigial from when the now-removed reference-impl evaluator's
  // `PreparedModify` drove UPDATE / DELETE: it called
  // `Table::PrimaryKey()` to drive
  // `EvaluatorTableModifyIterator::GetOriginalKeyValue` and to dedupe
  // post-mutation row sets. The DuckDB engine does its own DML
  // through a transpiled UPDATE / DELETE SQL fragment and does not
  // consult `PrimaryKey()`, so the designation is currently a
  // no-op on the live execution path -- but BigQuery itself has no
  // primary-key concept either, so leaving the first column marked
  // is harmless and keeps `googlesql::Table` consumers (analyzer
  // optimizations, future planners) happy. Tables with zero columns
  // leave the PK unset; SimpleTable's `SetPrimaryKey({0})` would
  // otherwise return an InvalidArgument status we have no good way
  // to surface from a constructor.
  if (NumColumns() > 0) {
    (void)SetPrimaryKey({0});
  }
}

absl::StatusOr<std::unique_ptr<::googlesql::EvaluatorTableIterator>>
StorageTable::CreateEvaluatorTableIterator(
    absl::Span<const int> column_idxs) const {
  if (storage_ == nullptr) {
    return absl::FailedPreconditionError(absl::StrCat(
        "StorageTable '", FullName(), "': storage backend is not configured"));
  }
  std::vector<std::string> column_names;
  std::vector<const ::googlesql::Type*> column_types;
  std::vector<int> idxs;
  column_names.reserve(column_idxs.size());
  column_types.reserve(column_idxs.size());
  idxs.reserve(column_idxs.size());
  for (int idx : column_idxs) {
    if (idx < 0 || idx >= NumColumns()) {
      return absl::InvalidArgumentError(absl::StrCat("StorageTable '",
                                                     FullName(),
                                                     "': column index ",
                                                     idx,
                                                     " is outside [0, ",
                                                     NumColumns(),
                                                     ")"));
    }
    const ::googlesql::Column* col = GetColumn(idx);
    if (col == nullptr) {
      return absl::InvalidArgumentError(absl::StrCat(
          "StorageTable '", FullName(), "': column #", idx, " is null"));
    }
    column_names.push_back(col->Name());
    column_types.push_back(col->GetType());
    idxs.push_back(idx);
  }
  absl::StatusOr<std::unique_ptr<storage::RowIterator>> iter =
      storage_->ScanRows(table_id_);
  if (!iter.ok()) return iter.status();
  return std::make_unique<StorageEvaluatorTableIterator>(
      std::move(iter).value(),
      bq_schema_,
      std::move(idxs),
      std::move(column_names),
      std::move(column_types));
}

}  // namespace catalog
}  // namespace backend
}  // namespace bigquery_emulator
