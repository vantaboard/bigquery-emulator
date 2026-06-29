#include <cstdint>
#include <string>
#include <utility>
#include <vector>

#include "absl/container/flat_hash_map.h"
#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "absl/strings/str_cat.h"
#include "backend/catalog/storage_table.h"
#include "backend/engine/semantic/dml/dml_executor_internal.h"
#include "backend/engine/semantic/error.h"
#include "backend/engine/semantic/row_source.h"
#include "backend/engine/semantic/scan_eval.h"
#include "backend/engine/semantic/value.h"
#include "backend/schema/schema.h"
#include "backend/storage/storage.h"
#include "googlesql/resolved_ast/resolved_ast.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace semantic {
namespace dml {

namespace {

absl::StatusOr<std::vector<int>> BuildInsertColumnIndexMapping(
    const ::googlesql::ResolvedInsertStmt& insert,
    const schema::TableSchema& schema) {
  std::vector<int> column_idx_for_insert_position;
  column_idx_for_insert_position.reserve(insert.insert_column_list_size());
  for (int i = 0; i < insert.insert_column_list_size(); ++i) {
    const ::googlesql::ResolvedColumn& col = insert.insert_column_list(i);
    if (IndexOfColumn(schema, col.name()) < 0) {
      return absl::InternalError(
          absl::StrCat("semantic/dml: INSERT column '",
                       col.name(),
                       "' not found in storage table schema"));
    }
    column_idx_for_insert_position.push_back(IndexOfColumn(schema, col.name()));
  }
  return column_idx_for_insert_position;
}

absl::StatusOr<DmlStats> FinalizeInsertAppend(
    const ::googlesql::ResolvedInsertStmt& insert,
    const catalog::StorageTable* target,
    const schema::TableSchema& schema,
    const absl::flat_hash_map<int, int>& by_id,
    std::vector<storage::Row> rows,
    storage::Storage& storage,
    EvalContext& ctx,
    std::unique_ptr<RowSource>* returning_out) {
  if (rows.empty()) {
    DmlStats stats;
    absl::Status assert_ok = CheckAssertRowsModified(
        insert.assert_rows_modified(), DmlStatementKind::kInsert, stats, ctx);
    if (!assert_ok.ok()) return assert_ok;
    return stats;
  }
  DmlStats stats;
  stats.inserted_row_count = static_cast<int64_t>(rows.size());
  absl::Status assert_ok = CheckAssertRowsModified(
      insert.assert_rows_modified(), DmlStatementKind::kInsert, stats, ctx);
  if (!assert_ok.ok()) return assert_ok;
  if (insert.returning() != nullptr && returning_out != nullptr &&
      insert.table_scan() != nullptr) {
    auto ret_or = BuildInsertReturningIfNeeded(
        insert, target, schema, by_id, rows, returning_out, stats, ctx);
    if (!ret_or.ok()) return ret_or.status();
    stats = *ret_or;
  }
  absl::Status appended =
      // cpp-lint:allow(status-discarded) -- captured into appended
      storage.AppendRows(target->storage_table_id(), rows);
  if (!appended.ok()) return appended;
  return stats;
}

absl::StatusOr<DmlStats> ExecuteInsertFromSelect(
    const ::googlesql::ResolvedInsertStmt& insert,
    const catalog::StorageTable* target,
    const schema::TableSchema& schema,
    const absl::flat_hash_map<int, int>& by_id,
    const std::vector<int>& column_idx_for_insert_position,
    storage::Storage& storage,
    EvalContext& ctx,
    std::unique_ptr<RowSource>* returning_out) {
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
  return FinalizeInsertAppend(insert,
                              target,
                              schema,
                              by_id,
                              std::move(rows),
                              storage,
                              ctx,
                              returning_out);
}

absl::StatusOr<DmlStats> ExecuteInsertFromValues(
    const ::googlesql::ResolvedInsertStmt& insert,
    const catalog::StorageTable* target,
    const schema::TableSchema& schema,
    const absl::flat_hash_map<int, int>& by_id,
    const std::vector<int>& column_idx_for_insert_position,
    storage::Storage& storage,
    EvalContext& ctx,
    std::unique_ptr<RowSource>* returning_out) {
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
  return FinalizeInsertAppend(insert,
                              target,
                              schema,
                              by_id,
                              std::move(rows),
                              storage,
                              ctx,
                              returning_out);
}

}  // namespace

absl::StatusOr<DmlStats> ExecuteInsert(
    const ::googlesql::ResolvedInsertStmt& insert,
    storage::Storage& storage,
    EvalContext& ctx,
    std::unique_ptr<RowSource>* returning_out) {
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
  absl::Status guard = RejectUnsupportedDmlFeatures(
      insert.generated_column_expr_list_size(), "INSERT");
  if (!guard.ok()) return guard;

  auto target_or = StorageTargetFor(insert, "INSERT");
  if (!target_or.ok()) return target_or.status();
  const catalog::StorageTable* target = *target_or;
  const schema::TableSchema& schema = target->bq_schema();
  absl::flat_hash_map<int, int> by_id;
  if (insert.returning() != nullptr && insert.table_scan() != nullptr) {
    auto by_id_or = BuildColumnIndexByColumnId(*insert.table_scan(), schema);
    if (!by_id_or.ok()) return by_id_or.status();
    by_id = *std::move(by_id_or);
  }

  auto column_idx_or = BuildInsertColumnIndexMapping(insert, schema);
  if (!column_idx_or.ok()) return column_idx_or.status();

  if (insert.query() != nullptr) {
    return ExecuteInsertFromSelect(insert,
                                   target,
                                   schema,
                                   by_id,
                                   *column_idx_or,
                                   storage,
                                   ctx,
                                   returning_out);
  }
  return ExecuteInsertFromValues(insert,
                                 target,
                                 schema,
                                 by_id,
                                 *column_idx_or,
                                 storage,
                                 ctx,
                                 returning_out);
}

}  // namespace dml
}  // namespace semantic
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
