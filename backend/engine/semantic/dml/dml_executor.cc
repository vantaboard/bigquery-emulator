#include "backend/engine/semantic/dml/dml_executor.h"

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
#include "backend/catalog/storage_table.h"
#include "backend/engine/engine.h"
#include "backend/engine/semantic/error.h"
#include "backend/engine/semantic/eval_expr.h"
#include "backend/engine/semantic/scan_eval.h"
#include "backend/engine/semantic/value.h"
#include "backend/schema/schema.h"
#include "backend/storage/storage.h"
#include "googlesql/public/value.h"
#include "googlesql/resolved_ast/resolved_ast.h"
#include "googlesql/resolved_ast/resolved_column.h"
#include "googlesql/resolved_ast/resolved_node_kind.pb.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace semantic {
namespace dml {

namespace {

// Build the `ParameterBindings` the per-row evaluator consults.
// Mirrors the helper in `semantic/executor.cc::BuildParameterBindings`
// verbatim; we duplicate the trivial six-line shape here rather
// than expose a private semantic-executor helper because the DML
// executor sits one package below `semantic` (we cannot cyclically
// depend on the SELECT executor).
absl::StatusOr<ParameterBindings> BuildParameterBindings(
    const QueryRequest& request) {
  ParameterBindings bindings;
  bool any_positional = false;
  bool any_named = false;
  for (const QueryParameter& p : request.parameters) {
    auto value = ParseParameterValue(p.value_json, p.type_kind);
    if (!value.ok()) return value.status();
    if (p.name.empty()) {
      bindings.by_position.push_back(*std::move(value));
      any_positional = true;
    } else if (IsSyntheticPositionalParameterName(p.name)) {
      bindings.by_name[absl::AsciiStrToLower(p.name)] = *std::move(value);
      any_named = true;
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
template <typename StmtT>
absl::StatusOr<const catalog::StorageTable*> StorageTargetFor(
    const StmtT& stmt, absl::string_view kind) {
  if (stmt.table_scan() == nullptr || stmt.table_scan()->table() == nullptr) {
    return absl::InternalError(absl::StrCat(
        "semantic/dml: ", kind, " has no resolved target table scan"));
  }
  const auto* storage_table =
      dynamic_cast<const catalog::StorageTable*>(stmt.table_scan()->table());
  if (storage_table == nullptr) {
    return absl::FailedPreconditionError(
        absl::StrCat("semantic/dml: ",
                     kind,
                     " target '",
                     stmt.table_scan()->table()->FullName(),
                     "' is not backed by a StorageTable; cannot apply DML"));
  }
  return storage_table;
}

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
    auto v = catalog::StorageValueToGoogleSqlValue(row.cells[idx], col.type());
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

// Reject DML shapes the executor does not yet handle. Surfaces
// `kNotImplemented` so the gateway envelope is the same as for
// any other planned-but-not-landed route. Specific deferred
// shapes: `RETURNING` clause (Family 7), `ASSERT_ROWS_MODIFIED`
// modifier (BigQuery-only, no DuckDB analog), generated columns
// (BigQuery doesn't expose CREATE TABLE generated columns
// today). `array_offset_column` is only set on UPDATE/DELETE
// when the target sits inside a `FROM t, UNNEST(t.arr)` shape,
// which Family 4 (correlated array scans, deferred from plan
// 8) is the right home for.
absl::Status RejectUnsupportedDmlFeatures(
    const ::googlesql::ResolvedReturningClause* returning,
    const ::googlesql::ResolvedAssertRowsModified* assert_rows,
    bool has_array_offset_column,
    int generated_column_count,
    absl::string_view kind) {
  if (returning != nullptr) {
    return MakeSemanticError(
        SemanticErrorReason::kNotImplemented,
        absl::StrCat("semantic/dml: ",
                     kind,
                     " RETURNING clause is owned by Family 7 of "
                     "googlesqlite-14-dml-system.plan.md"));
  }
  if (assert_rows != nullptr) {
    return MakeSemanticError(
        SemanticErrorReason::kNotImplemented,
        absl::StrCat("semantic/dml: ",
                     kind,
                     " ASSERT_ROWS_MODIFIED modifier is not yet supported"));
  }
  if (has_array_offset_column) {
    return MakeSemanticError(
        SemanticErrorReason::kNotImplemented,
        absl::StrCat(
            "semantic/dml: ",
            kind,
            " WITH OFFSET requires correlated lateral evaluation owned by "
            "Family 4 of googlesqlite-12-arrays-generators.plan.md"));
  }
  if (generated_column_count > 0) {
    return MakeSemanticError(
        SemanticErrorReason::kNotImplemented,
        absl::StrCat("semantic/dml: ",
                     kind,
                     " on tables with GENERATED columns is not yet supported"));
  }
  return absl::OkStatus();
}

absl::StatusOr<DmlStats> ExecuteInsert(
    const ::googlesql::ResolvedInsertStmt& insert,
    storage::Storage& storage,
    EvalContext& ctx) {
  if (insert.insert_mode() != ::googlesql::ResolvedInsertStmt::OR_ERROR) {
    return MakeSemanticError(
        SemanticErrorReason::kNotImplemented,
        "semantic/dml: INSERT OR {IGNORE,REPLACE,UPDATE} requires "
        "primary-key conflict resolution that is not yet supported");
  }
  if (insert.on_conflict_clause() != nullptr) {
    return MakeSemanticError(
        SemanticErrorReason::kNotImplemented,
        "semantic/dml: INSERT ... ON CONFLICT is not yet supported");
  }
  absl::Status guard =
      RejectUnsupportedDmlFeatures(insert.returning(),
                                   insert.assert_rows_modified(),
                                   /*has_array_offset_column=*/false,
                                   insert.generated_column_expr_list_size(),
                                   "INSERT");
  if (!guard.ok()) return guard;

  auto target_or = StorageTargetFor(insert, "INSERT");
  if (!target_or.ok()) return target_or.status();
  const catalog::StorageTable* target = *target_or;
  const schema::TableSchema& schema = target->bq_schema();

  // Map each `insert_column_list[i]` onto the matching index in
  // the storage schema. The analyzer guarantees `insert_column_list`
  // is non-empty for both `INSERT (a,b) VALUES (...)` and
  // `INSERT VALUES (...)` (it implicitly fills in the table's full
  // column list for the no-column-list form).
  std::vector<int> column_idx_for_insert_position;
  column_idx_for_insert_position.reserve(insert.insert_column_list_size());
  for (int i = 0; i < insert.insert_column_list_size(); ++i) {
    const ::googlesql::ResolvedColumn& col = insert.insert_column_list(i);
    const int idx = IndexOfColumn(schema, col.name());
    if (idx < 0) {
      return absl::InternalError(
          absl::StrCat("semantic/dml: INSERT column '",
                       col.name(),
                       "' not found in storage table schema"));
    }
    column_idx_for_insert_position.push_back(idx);
  }

  // INSERT ... SELECT: materialize `query()` and append rows.
  if (insert.query() != nullptr) {
    if (insert.query_output_column_list_size() !=
        insert.insert_column_list_size()) {
      return absl::InternalError(
          "semantic/dml: INSERT column list size does not match SELECT "
          "output column list size");
    }
    std::vector<int> source_col_ids;
    source_col_ids.reserve(insert.query_output_column_list_size());
    for (int i = 0; i < insert.query_output_column_list_size(); ++i) {
      source_col_ids.push_back(insert.query_output_column_list(i).column_id());
    }

    auto rows_or = MaterializeScan(insert.query(), ctx);
    if (!rows_or.ok()) return rows_or.status();

    std::vector<storage::Row> rows;
    rows.reserve(rows_or->size());
    for (const ColumnBindings& bind : *rows_or) {
      storage::Row out;
      out.cells.assign(schema.columns.size(), storage::Value::Null());
      for (size_t i = 0; i < source_col_ids.size(); ++i) {
        auto it = bind.find(source_col_ids[i]);
        if (it == bind.end()) {
          return absl::InternalError(absl::StrCat(
              "semantic/dml: INSERT ... SELECT row missing column_id=",
              source_col_ids[i]));
        }
        auto cell = ToStorageValue(it->second);
        if (!cell.ok()) return cell.status();
        out.cells[column_idx_for_insert_position[i]] = *std::move(cell);
      }
      rows.push_back(std::move(out));
    }

    if (rows.empty()) {
      DmlStats stats;
      return stats;
    }
    absl::Status appended =
        storage.AppendRows(target->storage_table_id(), rows);
    if (!appended.ok()) return appended;
    DmlStats stats;
    stats.inserted_row_count = static_cast<int64_t>(rows.size());
    return stats;
  }

  // Evaluate each VALUES row and collect the resulting
  // `storage::Row`s. We materialize before calling `AppendRows`
  // so an evaluator failure on row N leaves the table untouched
  // (the storage layer's append is atomic at the batch
  // boundary).
  std::vector<storage::Row> rows;
  rows.reserve(insert.row_list_size());
  for (int i = 0; i < insert.row_list_size(); ++i) {
    const ::googlesql::ResolvedInsertRow* row_node = insert.row_list(i);
    if (row_node == nullptr) {
      return absl::InternalError(
          "semantic/dml: INSERT row_list contains a null entry");
    }
    auto built =
        BuildInsertRow(*row_node, column_idx_for_insert_position, schema, ctx);
    if (!built.ok()) return built.status();
    rows.push_back(*std::move(built));
  }

  absl::Status appended = storage.AppendRows(target->storage_table_id(), rows);
  if (!appended.ok()) return appended;

  DmlStats stats;
  stats.inserted_row_count = static_cast<int64_t>(rows.size());
  return stats;
}

absl::StatusOr<DmlStats> ExecuteDelete(
    const ::googlesql::ResolvedDeleteStmt& del,
    storage::Storage& storage,
    EvalContext& ctx) {
  absl::Status guard = RejectUnsupportedDmlFeatures(
      del.returning(),
      del.assert_rows_modified(),
      /*has_array_offset_column=*/del.array_offset_column() != nullptr,
      /*generated_column_count=*/0,
      "DELETE");
  if (!guard.ok()) return guard;

  auto target_or = StorageTargetFor(del, "DELETE");
  if (!target_or.ok()) return target_or.status();
  const catalog::StorageTable* target = *target_or;
  const schema::TableSchema& schema = target->bq_schema();

  auto by_id = BuildColumnIndexByColumnId(*del.table_scan(), schema);
  if (!by_id.ok()) return by_id.status();

  auto rows_or = ScanAllRows(storage, target->storage_table_id());
  if (!rows_or.ok()) return rows_or.status();
  std::vector<storage::Row> rows = *std::move(rows_or);

  // Walk the row source, evaluate `where_expr` per row. A row
  // survives iff the predicate evaluates to TRUE; FALSE and
  // SQL-NULL both delete the row -- BigQuery treats `WHERE NULL`
  // identically to `WHERE FALSE` for `DELETE` (the predicate is
  // a "delete me" filter, not a "keep me" one, so the no-WHERE
  // case maps to TRUE-everywhere in the analyzer).
  std::vector<storage::Row> kept;
  kept.reserve(rows.size());
  int64_t deleted = 0;
  for (const storage::Row& row : rows) {
    auto bind = BindRow(row, *del.table_scan(), *by_id, schema);
    if (!bind.ok()) return bind.status();
    ColumnBindings local = *std::move(bind);
    ctx.columns = &local;
    Value pred_val;
    if (del.where_expr() != nullptr) {
      auto v = EvalExpr(*del.where_expr(), ctx);
      ctx.columns = nullptr;
      if (!v.ok()) return v.status();
      pred_val = *std::move(v);
    } else {
      ctx.columns = nullptr;
      pred_val = Value::Bool(true);
    }
    const bool matched = pred_val.is_valid() && !pred_val.is_null() &&
                         pred_val.type_kind() == ::googlesql::TYPE_BOOL &&
                         pred_val.bool_value();
    if (matched) {
      ++deleted;
    } else {
      kept.push_back(row);
    }
  }
  ctx.columns = nullptr;

  if (deleted > 0) {
    absl::Status overwrote =
        // cpp-lint:allow(status-discarded) -- captured into overwrote
        storage.OverwriteRows(target->storage_table_id(), kept);
    if (!overwrote.ok()) return overwrote;
  }

  DmlStats stats;
  stats.deleted_row_count = deleted;
  return stats;
}

// UPDATE applies one or more `update_item_list` items per matched
// row. Today we only handle the scalar-SET form
// (`update_item->target() == ResolvedColumnRef` AND `set_value()`
// is non-null AND the nested-update lists are empty); the deep-
// STRUCT path (`SET s.a.b = ...`, where `target()` is a
// `ResolvedGetStructField` chain) is owned by Family 4 of the
// plan. Each unsupported subshape surfaces a structured
// `kNotImplemented`.
absl::StatusOr<DmlStats> ExecuteUpdate(
    const ::googlesql::ResolvedUpdateStmt& upd,
    storage::Storage& storage,
    EvalContext& ctx) {
  absl::Status guard = RejectUnsupportedDmlFeatures(
      upd.returning(),
      upd.assert_rows_modified(),
      /*has_array_offset_column=*/upd.array_offset_column() != nullptr,
      upd.generated_column_expr_list_size(),
      "UPDATE");
  if (!guard.ok()) return guard;
  if (upd.from_scan() != nullptr) {
    return MakeSemanticError(
        SemanticErrorReason::kNotImplemented,
        "semantic/dml: UPDATE ... FROM ... is not yet supported");
  }

  auto target_or = StorageTargetFor(upd, "UPDATE");
  if (!target_or.ok()) return target_or.status();
  const catalog::StorageTable* target = *target_or;
  const schema::TableSchema& schema = target->bq_schema();

  // Validate every update item up-front (cheap; lets us bail
  // before scanning rows when the executor cannot land the SET
  // shape). For the scalar form, `target()` must resolve to a
  // top-level `ResolvedColumnRef` over the same table scan.
  struct ScalarSet {
    int column_idx = -1;  // index into bq_schema_.columns
    const ::googlesql::ResolvedExpr* set_expr = nullptr;
  };
  std::vector<ScalarSet> sets;
  sets.reserve(upd.update_item_list_size());
  for (int i = 0; i < upd.update_item_list_size(); ++i) {
    const ::googlesql::ResolvedUpdateItem* item = upd.update_item_list(i);
    if (item == nullptr) {
      return absl::InternalError(
          "semantic/dml: UPDATE update_item_list contains a null entry");
    }
    if (!item->update_item_element_list().empty() ||
        !item->delete_list().empty() || !item->update_list().empty() ||
        !item->insert_list().empty() || item->element_column() != nullptr) {
      return MakeSemanticError(
          SemanticErrorReason::kNotImplemented,
          "semantic/dml: nested UPDATE (array element / sub-record) is "
          "owned by Family 4 of googlesqlite-14-dml-system.plan.md");
    }
    if (item->target() == nullptr) {
      return absl::InternalError(
          "semantic/dml: ResolvedUpdateItem has null target");
    }
    if (item->target()->node_kind() != ::googlesql::RESOLVED_COLUMN_REF) {
      return MakeSemanticError(
          SemanticErrorReason::kNotImplemented,
          absl::StrCat("semantic/dml: UPDATE SET target kind ",
                       item->target()->node_kind_string(),
                       " (deep STRUCT mutation) is owned by Family 4 of "
                       "googlesqlite-14-dml-system.plan.md"));
    }
    if (item->set_value() == nullptr || item->set_value()->value() == nullptr) {
      return absl::InternalError(
          "semantic/dml: scalar UPDATE item has null set_value");
    }
    const auto* col_ref =
        item->target()->GetAs<::googlesql::ResolvedColumnRef>();
    const int idx = IndexOfColumn(schema, col_ref->column().name());
    if (idx < 0) {
      return absl::InternalError(
          absl::StrCat("semantic/dml: UPDATE target column '",
                       col_ref->column().name(),
                       "' not found in storage table schema"));
    }
    sets.push_back({idx, item->set_value()->value()});
  }

  auto by_id = BuildColumnIndexByColumnId(*upd.table_scan(), schema);
  if (!by_id.ok()) return by_id.status();

  auto rows_or = ScanAllRows(storage, target->storage_table_id());
  if (!rows_or.ok()) return rows_or.status();
  std::vector<storage::Row> rows = *std::move(rows_or);

  std::vector<storage::Row> rewritten;
  rewritten.reserve(rows.size());
  int64_t updated = 0;
  for (const storage::Row& row : rows) {
    auto bind = BindRow(row, *upd.table_scan(), *by_id, schema);
    if (!bind.ok()) return bind.status();
    ColumnBindings local = *std::move(bind);
    ctx.columns = &local;
    bool matched = false;
    if (upd.where_expr() == nullptr) {
      matched = true;
    } else {
      auto v = EvalExpr(*upd.where_expr(), ctx);
      if (!v.ok()) {
        ctx.columns = nullptr;
        return v.status();
      }
      matched = v->is_valid() && !v->is_null() &&
                v->type_kind() == ::googlesql::TYPE_BOOL && v->bool_value();
    }
    if (!matched) {
      ctx.columns = nullptr;
      rewritten.push_back(row);
      continue;
    }
    storage::Row mutated = row;
    for (const ScalarSet& s : sets) {
      auto v = EvalExpr(*s.set_expr, ctx);
      if (!v.ok()) {
        ctx.columns = nullptr;
        return v.status();
      }
      auto cell = ToStorageValue(*v);
      if (!cell.ok()) {
        ctx.columns = nullptr;
        return cell.status();
      }
      mutated.cells[s.column_idx] = *std::move(cell);
    }
    ctx.columns = nullptr;
    ++updated;
    rewritten.push_back(std::move(mutated));
  }
  ctx.columns = nullptr;

  if (updated > 0) {
    absl::Status overwrote =
        // cpp-lint:allow(status-discarded) -- captured into overwrote
        storage.OverwriteRows(target->storage_table_id(), rewritten);
    if (!overwrote.ok()) return overwrote;
  }

  DmlStats stats;
  stats.updated_row_count = updated;
  return stats;
}

}  // namespace

absl::StatusOr<DmlStats> ExecuteDml(const QueryRequest& request,
                                    const ::googlesql::ResolvedStatement& stmt,
                                    ::googlesql::Catalog* catalog,
                                    storage::Storage* storage) {
  (void)catalog;  // analysis is owned by the coordinator above us.

  // Storage is only required for the kinds the executor actually
  // mutates (INSERT / UPDATE / DELETE today). Statements that route
  // through here without a DML kind (e.g. unit tests that hand a
  // SELECT to `ExecuteDml` to assert the kNotImplemented surface)
  // get the same `kNotImplemented` envelope they would get with a
  // configured storage backend.
  const auto kind = stmt.node_kind();
  const bool is_writer_kind = kind == ::googlesql::RESOLVED_INSERT_STMT ||
                              kind == ::googlesql::RESOLVED_UPDATE_STMT ||
                              kind == ::googlesql::RESOLVED_DELETE_STMT;
  if (is_writer_kind && storage == nullptr) {
    return absl::FailedPreconditionError(
        "semantic/dml: ExecuteDml called with null storage");
  }

  ParameterBindings bindings;
  if (!request.parameters.empty()) {
    auto built = BuildParameterBindings(request);
    if (!built.ok()) return built.status();
    bindings = *std::move(built);
  }
  EvalContext ctx;
  ctx.project_id = request.project_id;
  ctx.parameters = &bindings;

  switch (kind) {
    case ::googlesql::RESOLVED_INSERT_STMT:
      return ExecuteInsert(
          *stmt.GetAs<::googlesql::ResolvedInsertStmt>(), *storage, ctx);
    case ::googlesql::RESOLVED_DELETE_STMT:
      return ExecuteDelete(
          *stmt.GetAs<::googlesql::ResolvedDeleteStmt>(), *storage, ctx);
    case ::googlesql::RESOLVED_UPDATE_STMT:
      return ExecuteUpdate(
          *stmt.GetAs<::googlesql::ResolvedUpdateStmt>(), *storage, ctx);
    case ::googlesql::RESOLVED_MERGE_STMT:
      // Simple MERGE branches stay on the DuckDB fast path
      // (`duckdb_rewrite`); the harder matrix
      // (`WHEN NOT MATCHED BY SOURCE`, multi-action sequences)
      // is owned by Family 6 of the plan and deferred from this
      // subagent.
      return MakeSemanticError(
          SemanticErrorReason::kNotImplemented,
          "semantic/dml: MERGE harder branches (WHEN NOT MATCHED BY "
          "SOURCE / multi-action) are owned by Family 6 of "
          "googlesqlite-14-dml-system.plan.md (deferred)");
    case ::googlesql::RESOLVED_TRUNCATE_STMT:
      return MakeSemanticError(
          SemanticErrorReason::kNotImplemented,
          "semantic/dml: TRUNCATE TABLE is owned by control-op-executor "
          "(catalog metadata op), not the DML executor");
    default:
      return MakeSemanticError(
          SemanticErrorReason::kNotImplemented,
          absl::StrCat(
              "semantic/dml: ExecuteDml does not handle ",
              stmt.node_kind_string(),
              "; only INSERT / UPDATE / DELETE / MERGE statement kinds "
              "route through the local DML executor"));
  }
}

}  // namespace dml
}  // namespace semantic
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
