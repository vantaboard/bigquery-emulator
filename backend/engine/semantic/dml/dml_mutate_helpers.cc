#include <cstdint>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "absl/container/flat_hash_map.h"
#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "backend/catalog/storage_table.h"
#include "backend/engine/semantic/dml/dml_executor_internal.h"
#include "backend/engine/semantic/error.h"
#include "backend/engine/semantic/eval_expr.h"
#include "backend/engine/semantic/row_source.h"
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

absl::StatusOr<const ::googlesql::Type*> RootColumnTypeForTarget(
    const ::googlesql::ResolvedTableScan& table_scan,
    const schema::TableSchema& schema,
    const UpdateTarget& target) {
  for (int c = 0; c < table_scan.column_list_size(); ++c) {
    const ::googlesql::ResolvedColumn& col = table_scan.column_list(c);
    if (IndexOfColumn(schema, col.name()) == target.column_idx) {
      return col.type();
    }
  }
  return absl::InternalError(
      "semantic/dml: UPDATE target column missing from table scan");
}

struct ParsedUpdateListItem {
  std::optional<SetAssignment> set;
  std::optional<NestedArrayDeleteAssignment> nested_delete;
};

absl::StatusOr<ParsedUpdateListItem> ParseOneUpdateListItem(
    const ::googlesql::ResolvedUpdateItem& item,
    const ::googlesql::ResolvedTableScan& table_scan,
    const schema::TableSchema& schema) {
  ParsedUpdateListItem out;
  if (!item.update_item_element_list().empty() || !item.update_list().empty() ||
      !item.insert_list().empty()) {
    return MakeSemanticError(
        SemanticErrorReason::kNotImplemented,
        "semantic/dml: nested UPDATE (array element / sub-record) is "
        "deferred; see docs/ENGINE_POLICY.md");
  }
  if (!item.delete_list().empty()) {
    if (item.target() == nullptr || item.element_column() == nullptr) {
      return absl::InternalError(
          "semantic/dml: nested DELETE item missing target/element_column");
    }
    auto parsed = ParseUpdateTarget(*item.target(), schema);
    if (!parsed.ok()) return parsed.status();
    auto root_type = RootColumnTypeForTarget(table_scan, schema, *parsed);
    if (!root_type.ok()) return root_type.status();
    out.nested_delete =
        NestedArrayDeleteAssignment{*std::move(parsed), *root_type, &item};
    return out;
  }
  if (item.element_column() != nullptr) {
    return MakeSemanticError(
        SemanticErrorReason::kNotImplemented,
        "semantic/dml: nested UPDATE (array element / sub-record) is "
        "deferred; see docs/ENGINE_POLICY.md");
  }
  if (item.target() == nullptr) {
    return absl::InternalError(
        "semantic/dml: ResolvedUpdateItem has null target");
  }
  if (item.set_value() == nullptr || item.set_value()->value() == nullptr) {
    return absl::InternalError(
        "semantic/dml: scalar UPDATE item has null set_value");
  }
  auto parsed = ParseUpdateTarget(*item.target(), schema);
  if (!parsed.ok()) return parsed.status();
  auto root_type = RootColumnTypeForTarget(table_scan, schema, *parsed);
  if (!root_type.ok()) return root_type.status();
  out.set =
      SetAssignment{*std::move(parsed), item.set_value()->value(), *root_type};
  return out;
}

}  // namespace

absl::StatusOr<bool> EvalWherePredicate(
    const ::googlesql::ResolvedExpr* where_expr, EvalContext& ctx) {
  if (where_expr == nullptr) return true;
  auto v = EvalExpr(*where_expr, ctx);
  if (!v.ok()) return v.status();
  return v->is_valid() && !v->is_null() &&
         v->type_kind() == ::googlesql::TYPE_BOOL && v->bool_value();
}

absl::StatusOr<DmlStats> FinalizeMutateWithReturning(
    const ::googlesql::ResolvedReturningClause* returning,
    std::unique_ptr<RowSource>* returning_out,
    std::vector<ColumnBindings>&& returning_contexts,
    std::vector<std::string>&& returning_actions,
    DmlStats stats,
    EvalContext& ctx) {
  if (returning != nullptr && returning_out != nullptr &&
      !returning_contexts.empty()) {
    auto returning_or = BuildReturningRowSource(*returning,
                                                std::move(returning_contexts),
                                                std::move(returning_actions),
                                                ctx);
    if (!returning_or.ok()) return returning_or.status();
    *returning_out = *std::move(returning_or);
  }
  return stats;
}

absl::StatusOr<std::pair<std::vector<SetAssignment>,
                         std::vector<NestedArrayDeleteAssignment>>>
ParseUpdateAssignments(const ::googlesql::ResolvedUpdateStmt& upd,
                       const schema::TableSchema& schema) {
  std::vector<SetAssignment> sets;
  std::vector<NestedArrayDeleteAssignment> nested_deletes;
  sets.reserve(upd.update_item_list_size());
  nested_deletes.reserve(upd.update_item_list_size());
  for (int i = 0; i < upd.update_item_list_size(); ++i) {
    const ::googlesql::ResolvedUpdateItem* item = upd.update_item_list(i);
    if (item == nullptr) {
      return absl::InternalError(
          "semantic/dml: UPDATE update_item_list contains a null entry");
    }
    auto parsed_item = ParseOneUpdateListItem(*item, *upd.table_scan(), schema);
    if (!parsed_item.ok()) return parsed_item.status();
    if (parsed_item->set.has_value()) {
      sets.push_back(*std::move(parsed_item->set));
    }
    if (parsed_item->nested_delete.has_value()) {
      nested_deletes.push_back(*std::move(parsed_item->nested_delete));
    }
  }
  return std::make_pair(std::move(sets), std::move(nested_deletes));
}

absl::Status ApplyUpdateSets(
    storage::Row& mutated,
    const std::vector<SetAssignment>& sets,
    const std::vector<NestedArrayDeleteAssignment>& nested_deletes,
    const ColumnBindings& row_ctx,
    const schema::TableSchema& schema,
    EvalContext& ctx) {
  ctx.columns = &row_ctx;
  for (const SetAssignment& s : sets) {
    auto v = EvalExpr(*s.set_expr, ctx);
    if (!v.ok()) return v.status();
    if (s.target.struct_field_path.empty()) {
      auto cell = ToStorageValue(*v);
      if (!cell.ok()) return cell.status();
      mutated.cells[s.target.column_idx] = *std::move(cell);
      continue;
    }
    auto current = catalog::StorageValueToGoogleSqlValue(
        mutated.cells[s.target.column_idx],
        s.root_column_type,
        schema.columns[s.target.column_idx]);
    if (!current.ok()) return current.status();
    auto rewritten =
        SetNestedStructField(*current, s.target.struct_field_path, *v);
    if (!rewritten.ok()) return rewritten.status();
    auto cell = ToStorageValue(*rewritten);
    if (!cell.ok()) return cell.status();
    mutated.cells[s.target.column_idx] = *std::move(cell);
  }
  for (const NestedArrayDeleteAssignment& nested : nested_deletes) {
    auto current = catalog::StorageValueToGoogleSqlValue(
        mutated.cells[nested.target.column_idx],
        nested.root_column_type,
        schema.columns[nested.target.column_idx]);
    if (!current.ok()) return current.status();
    if (!nested.target.struct_field_path.empty()) {
      return MakeSemanticError(
          SemanticErrorReason::kNotImplemented,
          "semantic/dml: nested DELETE on nested STRUCT arrays is deferred");
    }
    auto rewritten =
        ApplyNestedArrayDeleteItem(*nested.item, *current, row_ctx, ctx);
    if (!rewritten.ok()) return rewritten.status();
    auto cell = ToStorageValue(*rewritten);
    if (!cell.ok()) return cell.status();
    mutated.cells[nested.target.column_idx] = *std::move(cell);
  }
  ctx.columns = nullptr;
  return absl::OkStatus();
}

absl::StatusOr<bool> CountFromScanMatches(
    const ::googlesql::ResolvedUpdateStmt& upd,
    const ColumnBindings& target_bind,
    const std::vector<ColumnBindings>& from_rows,
    EvalContext& ctx,
    ColumnBindings* matched_from) {
  int match_count = 0;
  for (const ColumnBindings& from_bind : from_rows) {
    ColumnBindings merged = MergeColumnBindings(target_bind, from_bind);
    ctx.columns = &merged;
    auto matched_or = EvalWherePredicate(upd.where_expr(), ctx);
    if (!matched_or.ok()) {
      ctx.columns = nullptr;
      return matched_or.status();
    }
    if (*matched_or) {
      ++match_count;
      *matched_from = std::move(merged);
    }
  }
  ctx.columns = nullptr;
  if (match_count > 1) {
    return MakeSemanticError(
        SemanticErrorReason::kInvalidArgument,
        "semantic/dml: UPDATE/MERGE must match at most one source row for "
        "each target row");
  }
  return match_count > 0;
}

absl::StatusOr<DmlStats> BuildInsertReturningIfNeeded(
    const ::googlesql::ResolvedInsertStmt& insert,
    const catalog::StorageTable* target,
    const schema::TableSchema& schema,
    const absl::flat_hash_map<int, int>& by_id,
    const std::vector<storage::Row>& rows,
    std::unique_ptr<RowSource>* returning_out,
    DmlStats stats,
    EvalContext& ctx) {
  (void)target;
  if (insert.returning() == nullptr || returning_out == nullptr ||
      insert.table_scan() == nullptr || rows.empty()) {
    return stats;
  }
  std::vector<ColumnBindings> contexts;
  std::vector<std::string> actions;
  contexts.reserve(rows.size());
  actions.reserve(rows.size());
  for (const storage::Row& row : rows) {
    auto bind = BindRow(row, *insert.table_scan(), by_id, schema);
    if (!bind.ok()) return bind.status();
    contexts.push_back(*std::move(bind));
    actions.push_back("INSERT");
  }
  auto ret_or = BuildReturningRowSource(
      *insert.returning(), std::move(contexts), std::move(actions), ctx);
  if (!ret_or.ok()) return ret_or.status();
  *returning_out = *std::move(ret_or);
  return stats;
}

}  // namespace dml
}  // namespace semantic
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
