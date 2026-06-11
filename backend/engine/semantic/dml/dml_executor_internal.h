#ifndef BIGQUERY_EMULATOR_BACKEND_ENGINE_SEMANTIC_DML_DML_EXECUTOR_INTERNAL_H_
#define BIGQUERY_EMULATOR_BACKEND_ENGINE_SEMANTIC_DML_DML_EXECUTOR_INTERNAL_H_

#include <cstdint>
#include <string>
#include <vector>

#include "absl/container/flat_hash_map.h"
#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/string_view.h"
#include "absl/types/span.h"
#include "backend/catalog/storage_table.h"
#include "backend/engine/engine.h"
#include "backend/engine/semantic/eval_expr.h"
#include "backend/engine/semantic/row_source.h"
#include "backend/schema/schema.h"
#include "backend/storage/storage.h"
#include "googlesql/resolved_ast/resolved_ast.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace semantic {
namespace dml {

absl::StatusOr<ParameterBindings> BuildParameterBindings(
    const QueryRequest& request);

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

int IndexOfColumn(const schema::TableSchema& schema, absl::string_view name);

absl::StatusOr<absl::flat_hash_map<int, int>> BuildColumnIndexByColumnId(
    const ::googlesql::ResolvedTableScan& scan,
    const schema::TableSchema& schema);

absl::StatusOr<std::vector<storage::Row>> ScanAllRows(
    storage::Storage& storage, const storage::TableId& id);

absl::StatusOr<ColumnBindings> BindRow(
    const storage::Row& row,
    const ::googlesql::ResolvedTableScan& scan,
    const absl::flat_hash_map<int, int>& by_id,
    const schema::TableSchema& schema);

absl::StatusOr<storage::Row> BuildInsertRow(
    const ::googlesql::ResolvedInsertRow& row_node,
    const std::vector<int>& column_idx_for_insert_position,
    const schema::TableSchema& schema,
    EvalContext& ctx);

enum class DmlStatementKind {
  kInsert,
  kUpdate,
  kDelete,
};

// Target of `UPDATE t SET <target> = ...` — either a top-level column
// or a nested STRUCT field path rooted at a table column.
struct UpdateTarget {
  int column_idx = -1;
  std::vector<int> struct_field_path;
};

absl::StatusOr<UpdateTarget> ParseUpdateTarget(
    const ::googlesql::ResolvedExpr& target, const schema::TableSchema& schema);

absl::StatusOr<Value> SetNestedStructField(const Value& root,
                                           absl::Span<const int> field_path,
                                           const Value& leaf);

int64_t AffectedRowCount(DmlStatementKind kind, const DmlStats& stats);

absl::Status CheckAssertRowsModified(
    const ::googlesql::ResolvedAssertRowsModified* assert_rows,
    DmlStatementKind kind,
    const DmlStats& stats,
    EvalContext& ctx);

ColumnBindings MergeColumnBindings(const ColumnBindings& target,
                                   const ColumnBindings& from);

absl::Status RejectUnsupportedDmlFeatures(int generated_column_count,
                                          absl::string_view kind);

absl::StatusOr<Value> ApplyNestedArrayDeleteItem(
    const ::googlesql::ResolvedUpdateItem& item,
    const Value& array_value,
    const ColumnBindings& row_ctx,
    EvalContext& ctx);

absl::StatusOr<DmlStats> ExecuteMerge(
    const ::googlesql::ResolvedMergeStmt& merge,
    storage::Storage& storage,
    EvalContext& ctx);

absl::StatusOr<std::unique_ptr<RowSource>> BuildReturningRowSource(
    const ::googlesql::ResolvedReturningClause& returning,
    const std::vector<ColumnBindings>& row_contexts,
    const std::vector<std::string>& actions,
    EvalContext& ctx);

absl::StatusOr<DmlStats> ExecuteInsert(
    const ::googlesql::ResolvedInsertStmt& insert,
    storage::Storage& storage,
    EvalContext& ctx,
    std::unique_ptr<RowSource>* returning_out);

absl::StatusOr<DmlStats> ExecuteDelete(
    const ::googlesql::ResolvedDeleteStmt& del,
    storage::Storage& storage,
    EvalContext& ctx,
    std::unique_ptr<RowSource>* returning_out);

absl::StatusOr<DmlStats> ExecuteUpdate(
    const ::googlesql::ResolvedUpdateStmt& upd,
    storage::Storage& storage,
    EvalContext& ctx,
    std::unique_ptr<RowSource>* returning_out);

}  // namespace dml
}  // namespace semantic
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator

#endif  // BIGQUERY_EMULATOR_BACKEND_ENGINE_SEMANTIC_DML_DML_EXECUTOR_INTERNAL_H_
