#include <cstddef>
#include <cstdint>
#include <string>
#include <utility>
#include <vector>

#include "absl/container/flat_hash_map.h"
#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "absl/strings/ascii.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/string_view.h"
#include "absl/types/span.h"
#include "backend/catalog/storage_table.h"
#include "backend/engine/semantic/dml/dml_executor.h"
#include "backend/engine/semantic/dml/dml_executor_internal.h"
#include "backend/engine/semantic/error.h"
#include "backend/engine/semantic/eval_expr.h"
#include "backend/engine/semantic/scan_eval.h"
#include "backend/engine/semantic/value.h"
#include "backend/schema/schema.h"
#include "backend/storage/storage.h"
#include "googlesql/public/value.h"
#include "googlesql/resolved_ast/resolved_ast.h"
#include "googlesql/resolved_ast/resolved_node_kind.pb.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace semantic {
namespace dml {

absl::StatusOr<ParameterBindings> BuildParameterBindings(
    const QueryRequest& request) {
  ParameterBindings bindings;
  bool any_positional = false;
  bool any_named = false;
  for (const QueryParameter& p : request.parameters) {
    auto value = ParseParameterValue(p.value_json, p.type_kind, p.type_json);
    if (!value.ok()) return value.status();
    if (p.name.empty()) {
      bindings.by_position.push_back(*std::move(value));
      any_positional = true;
    } else {
      bindings.by_name[absl::AsciiStrToLower(p.name)] = *std::move(value);
      any_named = true;
    }
  }
  if (any_positional && any_named) {
    return absl::InvalidArgumentError(
        "semantic/dml: request mixes named and positional parameters");
  }
  return bindings;
}

// Resolve `stmt`'s `table_scan()` -> `table()` to the matching
// `backend::catalog::StorageTable` (which carries the `TableId` /
// `bq_schema()` the storage layer indexes by). DML can only
// proceed against a `StorageTable`-backed analyzer table; bare
// `SimpleTable` instances (e.g. fixtures the analyzer's built-in
// catalog ships) raise FAILED_PRECONDITION so the gateway can
// surface a clean error. Returns the storage table pointer (not
// owned).
// Find the index of the column named `name` in `schema.columns`.
// Lookup is case-insensitive to match BigQuery column-name
// resolution semantics. Returns -1 when the column is absent.
int IndexOfColumn(const schema::TableSchema& schema, absl::string_view name) {
  for (size_t i = 0; i < schema.columns.size(); ++i) {
    if (absl::EqualsIgnoreCase(schema.columns[i].name, name)) {
      return static_cast<int>(i);
    }
  }
  return -1;
}

// Build a `column_id -> table_column_index` map for the columns
// the `ResolvedTableScan` exposes. The analyzer issues fresh
// `ResolvedColumn` ids per query; the engine resolves them by
// lining up the table-scan's `column_list` against the storage
// table's schema by NAME (case-insensitive) so column_id-to-cell
// mapping stays stable across DML statements that omit some
// columns from the projection. Columns in the table schema not
// referenced by the scan are simply absent from the map; callers
// fall back to NULL / pass-through on lookup misses.
absl::StatusOr<absl::flat_hash_map<int, int>> BuildColumnIndexByColumnId(
    const ::googlesql::ResolvedTableScan& scan,
    const schema::TableSchema& schema) {
  absl::flat_hash_map<int, int> by_id;
  by_id.reserve(scan.column_list_size());
  for (int i = 0; i < scan.column_list_size(); ++i) {
    const ::googlesql::ResolvedColumn& col = scan.column_list(i);
    const int idx = IndexOfColumn(schema, col.name());
    if (idx < 0) {
      return absl::InternalError(
          absl::StrCat("semantic/dml: ResolvedTableScan column '",
                       col.name(),
                       "' not found in storage table schema"));
    }
    by_id[col.column_id()] = idx;
  }
  return by_id;
}

// Drain every row of `id` out of `storage` into a vector. Used by
// UPDATE / DELETE to materialize the pre-mutation snapshot before
// applying the predicate / SET list. The storage layer does not
// expose a streaming partial-mutate API today (`OverwriteRows`
// replaces the entire row vector), so DML reads everything,
// computes the post-mutation shape in memory, and writes it back
// in one shot. This matches the DuckDB executor's MERGE strategy.
absl::StatusOr<std::vector<storage::Row>> ScanAllRows(
    storage::Storage& storage, const storage::TableId& id) {
  absl::StatusOr<std::unique_ptr<storage::RowIterator>> iter_or =
      storage.ScanRows(id);
  if (!iter_or.ok()) return iter_or.status();
  std::unique_ptr<storage::RowIterator> iter = std::move(iter_or).value();
  std::vector<storage::Row> rows;
  storage::Row row;
  while (true) {
    absl::StatusOr<bool> has = iter->Next(&row);
    if (!has.ok()) return has.status();
    if (!*has) break;
    rows.push_back(row);
  }
  return rows;
}

// Convert one row of a `ResolvedTableScan` row stream onto the
// `ColumnBindings` shape `EvalExpr` consumes for `WHERE` /
// `SET = <expr>` evaluation. The analyzer's `column_list()` is
// the ordered set of columns the scan exposes (one
// `ResolvedColumn` per scan column); the storage row's cells line
// up with the table schema by index, which we resolved into
// `by_id` once outside the loop.
absl::StatusOr<ColumnBindings> BindRow(
    const storage::Row& row,
    const ::googlesql::ResolvedTableScan& scan,
    const absl::flat_hash_map<int, int>& by_id,
    const schema::TableSchema& schema) {
  ColumnBindings bindings;
  bindings.reserve(scan.column_list_size());
  for (int i = 0; i < scan.column_list_size(); ++i) {
    const ::googlesql::ResolvedColumn& col = scan.column_list(i);
    auto it = by_id.find(col.column_id());
    if (it == by_id.end()) {
      return absl::InternalError(
          absl::StrCat("semantic/dml: column_id=",
                       col.column_id(),
                       " missing from storage column-index map"));
    }
    const int idx = it->second;
    if (idx < 0 || static_cast<size_t>(idx) >= row.cells.size()) {
      return absl::InternalError(absl::StrCat("semantic/dml: column index ",
                                              idx,
                                              " out of range for row with ",
                                              row.cells.size(),
                                              " cells"));
    }
    auto v = catalog::StorageValueToGoogleSqlValue(
        row.cells[idx], col.type(), schema.columns[idx]);
    if (!v.ok()) return v.status();
    bindings[col.column_id()] = *std::move(v);
  }
  return bindings;
}

// INSERT VALUES helper: build one `storage::Row` from a
// `ResolvedInsertRow`'s `value_list()`. Each value evaluates
// against the same `EvalContext` (no per-row column bindings;
// VALUES expressions are scalar / parameter-bound). Columns of
// the destination table NOT named in `insert_column_list` are
// filled with NULL -- BigQuery's INSERT contract honors the
// table's NULLABLE / REQUIRED modes at the storage layer
// (REQUIRED columns rejected as NULL surface from
// `Storage::AppendRows`).
absl::StatusOr<storage::Row> BuildInsertRow(
    const ::googlesql::ResolvedInsertRow& row_node,
    const std::vector<int>& column_idx_for_insert_position,
    const schema::TableSchema& schema,
    EvalContext& ctx) {
  if (row_node.value_list_size() !=
      static_cast<int>(column_idx_for_insert_position.size())) {
    return absl::InternalError(
        absl::StrCat("semantic/dml: INSERT row has ",
                     row_node.value_list_size(),
                     " values but the column list named ",
                     column_idx_for_insert_position.size(),
                     " columns"));
  }
  storage::Row out;
  out.cells.assign(schema.columns.size(), storage::Value::Null());
  for (int i = 0; i < row_node.value_list_size(); ++i) {
    const ::googlesql::ResolvedDMLValue* dml = row_node.value_list(i);
    if (dml == nullptr || dml->value() == nullptr) {
      return absl::InternalError(
          "semantic/dml: ResolvedDMLValue carries a null inner expr");
    }
    auto v = EvalExpr(*dml->value(), ctx);
    if (!v.ok()) return v.status();
    auto cell = ToStorageValue(*v);
    if (!cell.ok()) return cell.status();
    out.cells[column_idx_for_insert_position[i]] = *std::move(cell);
  }
  return out;
}

absl::StatusOr<UpdateTarget> ParseUpdateTarget(
    const ::googlesql::ResolvedExpr& target,
    const schema::TableSchema& schema) {
  UpdateTarget out;
  std::vector<int> path_inner_to_outer;
  const ::googlesql::ResolvedExpr* cur = &target;
  while (cur->node_kind() == ::googlesql::RESOLVED_GET_STRUCT_FIELD) {
    const auto* gsf = cur->GetAs<::googlesql::ResolvedGetStructField>();
    if (gsf == nullptr) {
      return absl::InternalError(
          "semantic/dml: ResolvedGetStructField cast failed");
    }
    path_inner_to_outer.push_back(gsf->field_idx());
    cur = gsf->expr();
    if (cur == nullptr) {
      return absl::InternalError(
          "semantic/dml: ResolvedGetStructField has null expr");
    }
  }
  if (cur->node_kind() != ::googlesql::RESOLVED_COLUMN_REF) {
    return MakeSemanticError(
        SemanticErrorReason::kNotImplemented,
        absl::StrCat("semantic/dml: UPDATE SET target kind ",
                     cur->node_kind_string(),
                     " is not yet supported; see docs/ENGINE_POLICY.md"));
  }
  const auto* col_ref = cur->GetAs<::googlesql::ResolvedColumnRef>();
  const int idx = IndexOfColumn(schema, col_ref->column().name());
  if (idx < 0) {
    return absl::InternalError(
        absl::StrCat("semantic/dml: UPDATE target column '",
                     col_ref->column().name(),
                     "' not found in storage table schema"));
  }
  out.column_idx = idx;
  out.struct_field_path.assign(path_inner_to_outer.rbegin(),
                               path_inner_to_outer.rend());
  return out;
}

absl::StatusOr<Value> SetNestedStructField(const Value& root,
                                           absl::Span<const int> field_path,
                                           const Value& leaf) {
  if (field_path.empty()) {
    return leaf;
  }
  if (root.is_null()) {
    return MakeSemanticError(
        SemanticErrorReason::kInvalidArgument,
        "semantic/dml: cannot SET a nested STRUCT field on NULL");
  }
  if (!root.type()->IsStruct()) {
    return absl::InvalidArgumentError(
        "semantic/dml: SetNestedStructField base is not STRUCT");
  }
  const ::googlesql::StructType* st = root.type()->AsStruct();
  if (st == nullptr) {
    return absl::InvalidArgumentError(
        "semantic/dml: SetNestedStructField STRUCT type cast failed");
  }
  const int idx = field_path[0];
  if (idx < 0 || idx >= root.num_fields()) {
    return absl::InvalidArgumentError(
        "semantic/dml: SetNestedStructField index out of range");
  }
  std::vector<Value> fields;
  fields.reserve(root.num_fields());
  for (int i = 0; i < root.num_fields(); ++i) {
    fields.push_back(root.field(i));
  }
  if (field_path.size() == 1) {
    fields[idx] = leaf;
    return Value::Struct(st, std::move(fields));
  }
  auto nested = SetNestedStructField(fields[idx], field_path.subspan(1), leaf);
  if (!nested.ok()) return nested.status();
  fields[idx] = *std::move(nested);
  return Value::Struct(st, std::move(fields));
}

int64_t AffectedRowCount(DmlStatementKind kind, const DmlStats& stats) {
  switch (kind) {
    case DmlStatementKind::kInsert:
      return stats.inserted_row_count;
    case DmlStatementKind::kUpdate:
      return stats.updated_row_count;
    case DmlStatementKind::kDelete:
      return stats.deleted_row_count;
  }
  return 0;
}

absl::Status CheckAssertRowsModified(
    const ::googlesql::ResolvedAssertRowsModified* assert_rows,
    DmlStatementKind kind,
    const DmlStats& stats,
    EvalContext& ctx) {
  if (assert_rows == nullptr) return absl::OkStatus();
  const ::googlesql::ResolvedExpr* rows_expr = assert_rows->rows();
  if (rows_expr == nullptr) {
    return absl::InternalError(
        "semantic/dml: ResolvedAssertRowsModified has null rows expr");
  }
  auto expected_or = EvalExpr(*rows_expr, ctx);
  if (!expected_or.ok()) return expected_or.status();
  const Value& expected = *expected_or;
  if (!expected.is_valid() || expected.is_null() ||
      expected.type_kind() != ::googlesql::TYPE_INT64) {
    return absl::InternalError(
        "semantic/dml: ASSERT_ROWS_MODIFIED rows must be a non-null INT64");
  }
  const int64_t expected_count = expected.int64_value();
  const int64_t actual = AffectedRowCount(kind, stats);
  if (actual != expected_count) {
    return MakeSemanticError(SemanticErrorReason::kInvalidArgument,
                             absl::StrCat("DML statement modified ",
                                          actual,
                                          " row(s), but ",
                                          expected_count,
                                          " row(s) were expected."));
  }
  return absl::OkStatus();
}

ColumnBindings MergeColumnBindings(const ColumnBindings& target,
                                   const ColumnBindings& from) {
  ColumnBindings merged = target;
  for (const auto& [col_id, value] : from) {
    merged[col_id] = value;
  }
  return merged;
}

// Reject DML shapes the executor does not yet handle. Surfaces
// `kNotImplemented` so the gateway envelope is the same as for
// any other planned-but-not-landed route. Specific deferred
// shapes: generated columns (BigQuery doesn't expose CREATE TABLE
// generated columns today).
absl::Status RejectUnsupportedDmlFeatures(int generated_column_count,
                                          absl::string_view kind) {
  if (generated_column_count > 0) {
    return MakeSemanticError(
        SemanticErrorReason::kNotImplemented,
        absl::StrCat("semantic/dml: ",
                     kind,
                     " on tables with GENERATED columns is not yet supported"));
  }
  return absl::OkStatus();
}

namespace {

enum class ArrayElementState {
  kKeep,
  kDelete,
};

absl::StatusOr<Value> ApplyNestedArrayDeleteStmt(
    const ::googlesql::ResolvedDeleteStmt& nested_delete,
    const ::googlesql::ResolvedColumn& element_column,
    const Value& array_value,
    const ColumnBindings& row_ctx,
    EvalContext& ctx,
    std::vector<ArrayElementState>* element_states) {
  if (nested_delete.where_expr() == nullptr) {
    return absl::InternalError(
        "semantic/dml: nested DELETE requires a WHERE clause");
  }
  if (array_value.is_null()) {
    return MakeSemanticError(
        SemanticErrorReason::kInvalidArgument,
        "semantic/dml: cannot execute nested DELETE on a NULL array value");
  }
  if (!array_value.type()->IsArray()) {
    return absl::InternalError(
        "semantic/dml: nested DELETE target is not an ARRAY");
  }
  const int n = array_value.num_elements();
  if (static_cast<int>(element_states->size()) != n) {
    element_states->assign(n, ArrayElementState::kKeep);
  }
  for (int i = 0; i < n; ++i) {
    if ((*element_states)[i] == ArrayElementState::kDelete) {
      continue;
    }
    ColumnBindings local = row_ctx;
    local.emplace(element_column.column_id(), array_value.element(i));
    if (nested_delete.array_offset_column() != nullptr) {
      local.emplace(nested_delete.array_offset_column()->column().column_id(),
                    Value::Int64(i));
    }
    ctx.columns = &local;
    auto pred = EvalExpr(*nested_delete.where_expr(), ctx);
    ctx.columns = nullptr;
    if (!pred.ok()) return pred.status();
    const bool matched = pred->is_valid() && !pred->is_null() &&
                         pred->type_kind() == ::googlesql::TYPE_BOOL &&
                         pred->bool_value();
    if (matched) {
      (*element_states)[i] = ArrayElementState::kDelete;
    }
  }
  return array_value;
}

}  // namespace

absl::StatusOr<Value> ApplyNestedArrayDeleteItem(
    const ::googlesql::ResolvedUpdateItem& item,
    const Value& array_value,
    const ColumnBindings& row_ctx,
    EvalContext& ctx) {
  if (item.element_column() == nullptr) {
    return absl::InternalError(
        "semantic/dml: nested DELETE update item missing element_column");
  }
  if (item.delete_list().empty()) {
    return absl::InternalError(
        "semantic/dml: nested DELETE update item has empty delete_list");
  }
  const ::googlesql::ResolvedColumn& element_column =
      item.element_column()->column();
  std::vector<ArrayElementState> element_states;
  if (!array_value.is_null()) {
    element_states.assign(array_value.num_elements(), ArrayElementState::kKeep);
  }
  for (int i = 0; i < item.delete_list_size(); ++i) {
    const ::googlesql::ResolvedDeleteStmt* nested = item.delete_list(i);
    if (nested == nullptr) {
      return absl::InternalError(
          "semantic/dml: nested DELETE list contains null entry");
    }
    auto status = ApplyNestedArrayDeleteStmt(*nested, element_column,
                                             array_value, row_ctx, ctx,
                                             &element_states);
    if (!status.ok()) return status.status();
  }
  if (array_value.is_null()) {
    return array_value;
  }
  std::vector<Value> kept;
  kept.reserve(array_value.num_elements());
  for (int i = 0; i < array_value.num_elements(); ++i) {
    if (element_states[i] == ArrayElementState::kKeep) {
      kept.push_back(array_value.element(i));
    }
  }
  return Value::Array(array_value.type()->AsArray(), kept);
}

}  // namespace dml
}  // namespace semantic
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
