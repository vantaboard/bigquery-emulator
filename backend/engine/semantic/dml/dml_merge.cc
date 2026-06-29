#include <cstddef>
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

namespace {

struct MergeSetAssignment {
  UpdateTarget target;
  const ::googlesql::ResolvedExpr* set_expr = nullptr;
  const ::googlesql::Type* root_column_type = nullptr;
};

bool EvalBoolExpr(const ::googlesql::ResolvedExpr* expr, EvalContext& ctx) {
  if (expr == nullptr) return true;
  auto v = EvalExpr(*expr, ctx);
  return v.ok() && v->is_valid() && !v->is_null() &&
         v->type_kind() == ::googlesql::TYPE_BOOL && v->bool_value();
}

absl::StatusOr<std::vector<MergeSetAssignment>> ParseMergeUpdateItems(
    const ::googlesql::ResolvedMergeWhen& when,
    const schema::TableSchema& schema,
    const ::googlesql::ResolvedTableScan& target_scan) {
  std::vector<MergeSetAssignment> sets;
  sets.reserve(when.update_item_list_size());
  for (int i = 0; i < when.update_item_list_size(); ++i) {
    const ::googlesql::ResolvedUpdateItem* item = when.update_item_list(i);
    if (item == nullptr || item->target() == nullptr ||
        item->set_value() == nullptr || item->set_value()->value() == nullptr) {
      return absl::InternalError(
          "semantic/dml: MERGE UPDATE item is missing target or set_value");
    }
    if (!item->update_item_element_list().empty() ||
        !item->delete_list().empty() || !item->update_list().empty() ||
        !item->insert_list().empty() || item->element_column() != nullptr) {
      return MakeSemanticError(
          SemanticErrorReason::kNotImplemented,
          "semantic/dml: nested MERGE UPDATE items are deferred");
    }
    auto parsed = ParseUpdateTarget(*item->target(), schema);
    if (!parsed.ok()) return parsed.status();
    const ::googlesql::Type* root_type = nullptr;
    for (int c = 0; c < target_scan.column_list_size(); ++c) {
      const ::googlesql::ResolvedColumn& col = target_scan.column_list(c);
      if (IndexOfColumn(schema, col.name()) == parsed->column_idx) {
        root_type = col.type();
        break;
      }
    }
    if (root_type == nullptr) {
      return absl::InternalError(
          "semantic/dml: MERGE UPDATE target column missing from table scan");
    }
    sets.push_back({*std::move(parsed), item->set_value()->value(), root_type});
  }
  return sets;
}

absl::Status ApplyMergeSets(storage::Row& mutated,
                            const std::vector<MergeSetAssignment>& sets,
                            const ColumnBindings& row_ctx,
                            const schema::TableSchema& schema,
                            EvalContext& ctx) {
  ctx.columns = &row_ctx;
  for (const MergeSetAssignment& s : sets) {
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
  ctx.columns = nullptr;
  return absl::OkStatus();
}

struct TargetMergeState {
  ColumnBindings target_bind{};
  storage::Row row{};
  bool has_source_match = false;
  int match_count = 0;
  ColumnBindings matched_bind{};
  bool acted = false;
  bool deleted = false;
};

struct SourceMergeState {
  ColumnBindings source_bind{};
  bool has_target_match = false;
  bool acted = false;
};

void CorrelateMergeTargetsAndSources(
    const ::googlesql::ResolvedMergeStmt& merge,
    std::vector<TargetMergeState>& targets,
    std::vector<SourceMergeState>& sources,
    EvalContext& ctx) {
  for (size_t s = 0; s < sources.size(); ++s) {
    for (size_t t = 0; t < targets.size(); ++t) {
      ColumnBindings merged =
          MergeColumnBindings(targets[t].target_bind, sources[s].source_bind);
      ctx.columns = &merged;
      if (!EvalBoolExpr(merge.merge_expr(), ctx)) {
        ctx.columns = nullptr;
        continue;
      }
      ctx.columns = nullptr;
      targets[t].has_source_match = true;
      targets[t].match_count += 1;
      targets[t].matched_bind = merged;
      sources[s].has_target_match = true;
    }
  }
}

absl::Status CheckMatchedTargetMultiMatch(
    const ::googlesql::ResolvedMergeStmt& merge,
    const std::vector<TargetMergeState>& targets) {
  bool matched_mutates = false;
  for (int i = 0; i < merge.when_clause_list_size(); ++i) {
    const ::googlesql::ResolvedMergeWhen* when = merge.when_clause_list(i);
    if (when != nullptr &&
        when->match_type() == ::googlesql::ResolvedMergeWhen::MATCHED &&
        (when->action_type() == ::googlesql::ResolvedMergeWhen::UPDATE ||
         when->action_type() == ::googlesql::ResolvedMergeWhen::DELETE)) {
      matched_mutates = true;
      break;
    }
  }
  if (!matched_mutates) return absl::OkStatus();
  for (const TargetMergeState& st : targets) {
    if (st.match_count > 1) {
      return MakeSemanticError(
          SemanticErrorReason::kInvalidArgument,
          "semantic/dml: MERGE must match at most one source row for "
          "each target row");
    }
  }
  return absl::OkStatus();
}

absl::Status ApplyMatchedMergeWhen(
    const ::googlesql::ResolvedMergeWhen& when,
    const schema::TableSchema& schema,
    const ::googlesql::ResolvedTableScan& target_scan,
    std::vector<TargetMergeState>& targets,
    DmlStats& stats,
    EvalContext& ctx) {
  for (TargetMergeState& st : targets) {
    if (!st.has_source_match || st.acted || st.deleted) continue;
    ctx.columns = &st.matched_bind;
    if (!EvalBoolExpr(when.match_expr(), ctx)) {
      ctx.columns = nullptr;
      continue;
    }
    ctx.columns = nullptr;
    if (when.action_type() == ::googlesql::ResolvedMergeWhen::DELETE) {
      st.deleted = true;
      st.acted = true;
      ++stats.deleted_row_count;
      continue;
    }
    if (when.action_type() == ::googlesql::ResolvedMergeWhen::UPDATE) {
      auto sets = ParseMergeUpdateItems(when, schema, target_scan);
      if (!sets.ok()) return sets.status();
      absl::Status applied =
          ApplyMergeSets(st.row, *sets, st.matched_bind, schema, ctx);
      if (!applied.ok()) return applied;
      st.acted = true;
      ++stats.updated_row_count;
      continue;
    }
    return absl::InternalError(
        "semantic/dml: MATCHED MERGE clause has unexpected action");
  }
  return absl::OkStatus();
}

absl::Status ApplyNotMatchedByTargetMergeWhen(
    const ::googlesql::ResolvedMergeWhen& when,
    const schema::TableSchema& schema,
    std::vector<SourceMergeState>& sources,
    std::vector<storage::Row>& inserts,
    DmlStats& stats,
    EvalContext& ctx) {
  for (size_t s = 0; s < sources.size(); ++s) {
    SourceMergeState& src = sources[s];
    if (src.has_target_match || src.acted) continue;
    ctx.columns = &src.source_bind;
    if (!EvalBoolExpr(when.match_expr(), ctx)) {
      ctx.columns = nullptr;
      continue;
    }
    if (when.action_type() != ::googlesql::ResolvedMergeWhen::INSERT) {
      ctx.columns = nullptr;
      return absl::InternalError(
          "semantic/dml: NOT MATCHED BY TARGET requires INSERT");
    }
    if (when.insert_row() == nullptr || when.insert_column_list().empty()) {
      ctx.columns = nullptr;
      return absl::InternalError(
          "semantic/dml: MERGE INSERT clause missing row/columns");
    }
    std::vector<int> column_idx;
    column_idx.reserve(when.insert_column_list_size());
    for (int c = 0; c < when.insert_column_list_size(); ++c) {
      if (IndexOfColumn(schema, when.insert_column_list(c).name()) < 0) {
        ctx.columns = nullptr;
        return absl::InternalError(
            absl::StrCat("semantic/dml: MERGE INSERT column '",
                         when.insert_column_list(c).name(),
                         "' not found in target schema"));
      }
      column_idx.push_back(
          IndexOfColumn(schema, when.insert_column_list(c).name()));
    }
    auto built = BuildInsertRow(*when.insert_row(), column_idx, schema, ctx);
    ctx.columns = nullptr;
    if (!built.ok()) return built.status();
    inserts.push_back(*std::move(built));
    src.acted = true;
    ++stats.inserted_row_count;
  }
  return absl::OkStatus();
}

absl::Status ApplyNotMatchedBySourceMergeWhen(
    const ::googlesql::ResolvedMergeWhen& when,
    const schema::TableSchema& schema,
    const ::googlesql::ResolvedTableScan& target_scan,
    std::vector<TargetMergeState>& targets,
    DmlStats& stats,
    EvalContext& ctx) {
  for (TargetMergeState& st : targets) {
    if (st.has_source_match || st.acted || st.deleted) continue;
    ctx.columns = &st.target_bind;
    if (!EvalBoolExpr(when.match_expr(), ctx)) {
      ctx.columns = nullptr;
      continue;
    }
    ctx.columns = nullptr;
    if (when.action_type() == ::googlesql::ResolvedMergeWhen::DELETE) {
      st.deleted = true;
      st.acted = true;
      ++stats.deleted_row_count;
      continue;
    }
    if (when.action_type() == ::googlesql::ResolvedMergeWhen::UPDATE) {
      auto sets = ParseMergeUpdateItems(when, schema, target_scan);
      if (!sets.ok()) return sets.status();
      absl::Status applied =
          ApplyMergeSets(st.row, *sets, st.target_bind, schema, ctx);
      if (!applied.ok()) return applied;
      st.acted = true;
      ++stats.updated_row_count;
      continue;
    }
    return absl::InternalError(
        "semantic/dml: NOT MATCHED BY SOURCE has unexpected action");
  }
  return absl::OkStatus();
}

}  // namespace

absl::StatusOr<DmlStats> ExecuteMerge(
    const ::googlesql::ResolvedMergeStmt& merge,
    storage::Storage& storage,
    EvalContext& ctx) {
  auto target_or = StorageTargetFor(merge, "MERGE");
  if (!target_or.ok()) return target_or.status();
  const catalog::StorageTable* target = *target_or;
  const schema::TableSchema& schema = target->bq_schema();

  auto target_by_id = BuildColumnIndexByColumnId(*merge.table_scan(), schema);
  if (!target_by_id.ok()) return target_by_id.status();

  auto target_rows_or = ScanAllRows(storage, target->storage_table_id());
  if (!target_rows_or.ok()) return target_rows_or.status();
  std::vector<storage::Row> target_rows = *std::move(target_rows_or);

  auto source_rows_or = MaterializeScan(merge.from_scan(), ctx);
  if (!source_rows_or.ok()) return source_rows_or.status();
  const std::vector<ColumnBindings>& source_rows = *source_rows_or;

  std::vector<TargetMergeState> targets;
  targets.reserve(target_rows.size());
  for (const storage::Row& row : target_rows) {
    auto bind = BindRow(row, *merge.table_scan(), *target_by_id, schema);
    if (!bind.ok()) return bind.status();
    targets.push_back({*std::move(bind), row, false, 0, {}, false, false});
  }

  std::vector<SourceMergeState> sources;
  sources.reserve(source_rows.size());
  for (const ColumnBindings& bind : source_rows) {
    sources.push_back({bind, false, false});
  }

  CorrelateMergeTargetsAndSources(merge, targets, sources, ctx);

  absl::Status multi_match = CheckMatchedTargetMultiMatch(merge, targets);
  if (!multi_match.ok()) return multi_match;

  DmlStats stats;
  std::vector<storage::Row> inserts;

  for (int i = 0; i < merge.when_clause_list_size(); ++i) {
    const ::googlesql::ResolvedMergeWhen* when = merge.when_clause_list(i);
    if (when == nullptr) {
      return absl::InternalError(
          "semantic/dml: MERGE when_clause_list contains null entry");
    }

    switch (when->match_type()) {
      case ::googlesql::ResolvedMergeWhen::MATCHED: {
        absl::Status applied = ApplyMatchedMergeWhen(
            *when, schema, *merge.table_scan(), targets, stats, ctx);
        if (!applied.ok()) return applied;
        break;
      }
      case ::googlesql::ResolvedMergeWhen::NOT_MATCHED_BY_TARGET: {
        absl::Status applied = ApplyNotMatchedByTargetMergeWhen(
            *when, schema, sources, inserts, stats, ctx);
        if (!applied.ok()) return applied;
        break;
      }
      case ::googlesql::ResolvedMergeWhen::NOT_MATCHED_BY_SOURCE: {
        absl::Status applied = ApplyNotMatchedBySourceMergeWhen(
            *when, schema, *merge.table_scan(), targets, stats, ctx);
        if (!applied.ok()) return applied;
        break;
      }
    }
  }

  std::vector<storage::Row> final_rows;
  final_rows.reserve(targets.size() + inserts.size());
  for (const TargetMergeState& st : targets) {
    if (!st.deleted) final_rows.push_back(st.row);
  }
  final_rows.insert(final_rows.end(), inserts.begin(), inserts.end());

  absl::Status overwrote =
      // cpp-lint:allow(status-discarded) -- captured into overwrote
      storage.OverwriteRows(target->storage_table_id(), final_rows);
  if (!overwrote.ok()) return overwrote;
  return stats;
}

}  // namespace dml
}  // namespace semantic
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
