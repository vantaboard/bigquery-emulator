#include "backend/engine/semantic/scan_eval.h"

#include <algorithm>
#include <cstdint>
#include <map>
#include <string>
#include <utility>
#include <vector>

#include "absl/container/flat_hash_map.h"
#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "absl/strings/ascii.h"
#include "absl/strings/str_cat.h"
#include "backend/engine/semantic/array_struct/array_scan.h"
#include "backend/engine/semantic/error.h"
#include "backend/engine/semantic/eval_expr.h"
#include "backend/engine/semantic/functions/specialized_funcs.h"
#include "backend/engine/semantic/value.h"
#include "googlesql/public/simple_catalog.h"
#include "googlesql/public/type.h"
#include "googlesql/public/types/struct_type.h"
#include "googlesql/resolved_ast/resolved_ast.h"
#include "googlesql/resolved_ast/resolved_node_kind.pb.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace semantic {

namespace {

const ::googlesql::ResolvedScan* StripBarrierScans(
    const ::googlesql::ResolvedScan* scan) {
  while (scan != nullptr &&
         scan->node_kind() == ::googlesql::RESOLVED_BARRIER_SCAN) {
    scan = scan->GetAs<::googlesql::ResolvedBarrierScan>()->input_scan();
  }
  return scan;
}

bool ValueLess(const Value& a, const Value& b) {
  if (a.is_null() && b.is_null()) return false;
  if (a.is_null()) return true;
  if (b.is_null()) return false;
  if (a.type_kind() != b.type_kind()) {
    return static_cast<int>(a.type_kind()) < static_cast<int>(b.type_kind());
  }
  switch (a.type_kind()) {
    case ::googlesql::TYPE_BOOL:
      return !a.bool_value() && b.bool_value();
    case ::googlesql::TYPE_INT64:
      return a.int64_value() < b.int64_value();
    case ::googlesql::TYPE_DOUBLE:
      return a.double_value() < b.double_value();
    case ::googlesql::TYPE_STRING:
      return a.string_value() < b.string_value();
    case ::googlesql::TYPE_BYTES:
      return a.bytes_value() < b.bytes_value();
    default:
      return a.DebugString() < b.DebugString();
  }
}

bool ValueEqual(const Value& a, const Value& b) {
  if (a.is_null() && b.is_null()) return true;
  if (a.is_null() || b.is_null()) return false;
  return a.Equals(b);
}

int CompareArrayAggOrderKey(const ::googlesql::ResolvedOrderByItem& item,
                            const Value& va,
                            const Value& vb) {
  if (ValueEqual(va, vb)) return 0;
  if (va.is_null() || vb.is_null()) {
    bool nulls_first;
    switch (item.null_order()) {
      case ::googlesql::ResolvedOrderByItem::NULLS_FIRST:
        nulls_first = true;
        break;
      case ::googlesql::ResolvedOrderByItem::NULLS_LAST:
        nulls_first = false;
        break;
      default:
        nulls_first = !item.is_descending();
        break;
    }
    if (va.is_null() && vb.is_null()) return 0;
    if (va.is_null()) return nulls_first ? -1 : 1;
    return nulls_first ? 1 : -1;
  }
  bool less = ValueLess(va, vb);
  if (item.is_descending()) less = !less;
  return less ? -1 : 1;
}

absl::StatusOr<Value> EvalArrayAgg(
    const ::googlesql::ResolvedAggregateFunctionCall& call,
    const std::vector<std::vector<Value>>& input_column_values,
    const std::vector<ColumnBindings>& input_rows,
    EvalContext& ctx) {
  if (input_column_values.size() != 1) {
    return MakeSemanticError(SemanticErrorReason::kInvalidArgument,
                             "semantic: ARRAY_AGG expects one argument");
  }
  const ::googlesql::Type* out_type = call.type();
  if (out_type == nullptr || !out_type->IsArray()) {
    return MakeSemanticError(SemanticErrorReason::kInvalidArgument,
                             "semantic: ARRAY_AGG return type must be ARRAY");
  }
  const ::googlesql::ArrayType* arr_type = out_type->AsArray();
  const bool ignore_nulls =
      call.null_handling_modifier() ==
      ::googlesql::ResolvedNonScalarFunctionCallBase::IGNORE_NULLS;
  const bool distinct = call.distinct();

  struct Row {
    Value val;
    std::vector<Value> sort_keys;
  };
  std::vector<Row> rows;
  rows.reserve(input_rows.size());
  for (size_t r = 0; r < input_rows.size(); ++r) {
    const Value& v = input_column_values[0][r];
    if (ignore_nulls && v.is_null()) continue;
    Row row;
    row.val = v;
    EvalContext row_ctx = ctx;
    row_ctx.columns = &input_rows[r];
    row.sort_keys.reserve(call.order_by_item_list_size());
    for (int i = 0; i < call.order_by_item_list_size(); ++i) {
      const ::googlesql::ResolvedOrderByItem* item = call.order_by_item_list(i);
      if (item == nullptr || item->column_ref() == nullptr) {
        return MakeSemanticError(
            SemanticErrorReason::kInvalidArgument,
            "semantic: ARRAY_AGG ORDER BY requires column references");
      }
      auto key_or = EvalExpr(*item->column_ref(), row_ctx);
      if (!key_or.ok()) return key_or.status();
      row.sort_keys.push_back(*std::move(key_or));
    }
    rows.push_back(std::move(row));
  }

  if (distinct) {
    std::vector<Row> deduped;
    for (Row& row : rows) {
      bool seen = false;
      for (const Row& prior : deduped) {
        if (ValueEqual(prior.val, row.val)) {
          seen = true;
          break;
        }
      }
      if (!seen) deduped.push_back(std::move(row));
    }
    rows = std::move(deduped);
  }

  if (call.order_by_item_list_size() > 0) {
    std::stable_sort(rows.begin(), rows.end(), [&](const Row& a, const Row& b) {
      for (int i = 0; i < call.order_by_item_list_size(); ++i) {
        const ::googlesql::ResolvedOrderByItem* item =
            call.order_by_item_list(i);
        int cmp =
            CompareArrayAggOrderKey(*item, a.sort_keys[i], b.sort_keys[i]);
        if (cmp != 0) return cmp < 0;
      }
      return false;
    });
  }

  if (call.limit() != nullptr) {
    auto limit_or = EvalExpr(*call.limit(), ctx);
    if (!limit_or.ok()) return limit_or.status();
    if (limit_or->is_null()) {
      return MakeSemanticError(SemanticErrorReason::kInvalidArgument,
                               "semantic: ARRAY_AGG LIMIT must not be NULL");
    }
    if (limit_or->type_kind() != ::googlesql::TYPE_INT64) {
      return MakeSemanticError(SemanticErrorReason::kInvalidArgument,
                               "semantic: ARRAY_AGG LIMIT must be INT64");
    }
    const int64_t limit = limit_or->int64_value();
    if (limit < 0) {
      return MakeSemanticError(
          SemanticErrorReason::kInvalidArgument,
          "semantic: ARRAY_AGG LIMIT must be non-negative");
    }
    if (rows.size() > static_cast<size_t>(limit)) {
      rows.resize(static_cast<size_t>(limit));
    }
  }

  std::vector<Value> elements;
  elements.reserve(rows.size());
  for (const Row& row : rows) {
    elements.push_back(row.val);
  }
  return Value::Array(arr_type, std::move(elements));
}

absl::StatusOr<bool> EvalBoolExpr(const ::googlesql::ResolvedExpr* expr,
                                  EvalContext& ctx) {
  if (expr == nullptr) {
    return absl::InvalidArgumentError("semantic: null filter expression");
  }
  auto v = EvalExpr(*expr, ctx);
  if (!v.ok()) return v.status();
  if (v->is_null()) return false;
  if (v->type_kind() != ::googlesql::TYPE_BOOL) {
    return MakeSemanticError(SemanticErrorReason::kInvalidArgument,
                             "semantic: expected BOOL in predicate");
  }
  return v->bool_value();
}

absl::StatusOr<std::vector<ColumnBindings>> MaterializeScanImpl(
    const ::googlesql::ResolvedScan* scan, EvalContext& ctx);

namespace {

std::string GroupKeyFingerprint(const std::vector<Value>& keys) {
  std::string fp;
  for (const Value& key : keys) {
    absl::StrAppend(&fp, key.DebugString(), "\x1e");
  }
  return fp;
}

absl::StatusOr<Value> EvalAggregateForRows(
    const ::googlesql::ResolvedAggregateFunctionCall& agg,
    const std::vector<ColumnBindings>& input_rows,
    const std::vector<size_t>& row_indices,
    EvalContext& ctx) {
  std::vector<std::vector<Value>> arg_columns(
      static_cast<size_t>(agg.argument_list_size()));
  for (size_t r : row_indices) {
    EvalContext row_ctx = ctx;
    row_ctx.columns = &input_rows[r];
    for (int a = 0; a < agg.argument_list_size(); ++a) {
      auto v = EvalExpr(*agg.argument_list(a), row_ctx);
      if (!v.ok()) return v.status();
      arg_columns[static_cast<size_t>(a)].push_back(*std::move(v));
    }
  }
  std::string agg_name;
  if (agg.function() != nullptr) {
    agg_name = absl::AsciiStrToLower(
        agg.function()->FullName(/*include_group=*/false));
    if (agg_name.empty()) {
      agg_name = absl::AsciiStrToLower(agg.function()->Name());
    }
  }
  if (agg_name == "$count_star") {
    return Value::Int64(static_cast<int64_t>(row_indices.size()));
  }
  if (agg.function() != nullptr &&
      absl::AsciiStrToLower(agg.function()->Name()) == "array_agg") {
    return EvalArrayAgg(agg, arg_columns, input_rows, ctx);
  }
  return functions::EvalAggregateCall(agg, arg_columns);
}

absl::StatusOr<ColumnBindings> MaterializeAggregateGroup(
    const ::googlesql::ResolvedAggregateScan& aggregate,
    const std::vector<ColumnBindings>& input_rows,
    const std::vector<size_t>& row_indices,
    const std::vector<Value>* group_keys,
    EvalContext& ctx) {
  ColumnBindings out_row;
  if (group_keys != nullptr) {
    if (static_cast<int>(group_keys->size()) !=
        aggregate.group_by_list_size()) {
      return absl::InternalError(
          "semantic: aggregate group key arity mismatch");
    }
    for (int g = 0; g < aggregate.group_by_list_size(); ++g) {
      out_row.emplace(aggregate.group_by_list(g)->column().column_id(),
                      (*group_keys)[static_cast<size_t>(g)]);
    }
  }
  for (int i = 0; i < aggregate.aggregate_list_size(); ++i) {
    const ::googlesql::ResolvedComputedColumnBase* cc =
        aggregate.aggregate_list(i);
    if (cc == nullptr || cc->expr() == nullptr) {
      return absl::InternalError("semantic: aggregate column has null expr");
    }
    const auto* agg =
        cc->expr()->GetAs<::googlesql::ResolvedAggregateFunctionCall>();
    if (agg == nullptr) {
      return MakeSemanticError(
          SemanticErrorReason::kNotImplemented,
          "semantic: aggregate expression is not a function call");
    }
    auto result = EvalAggregateForRows(*agg, input_rows, row_indices, ctx);
    if (!result.ok()) return result.status();
    out_row.emplace(cc->column().column_id(), *std::move(result));
  }
  return out_row;
}

}  // namespace

absl::StatusOr<std::vector<ColumnBindings>> MaterializeAggregateScan(
    const ::googlesql::ResolvedAggregateScan& aggregate, EvalContext& ctx) {
  auto input_or = MaterializeScanImpl(aggregate.input_scan(), ctx);
  if (!input_or.ok()) return input_or.status();
  const std::vector<ColumnBindings>& input_rows = *input_or;

  if (aggregate.grouping_set_list_size() > 0) {
    return MakeSemanticError(
        SemanticErrorReason::kNotImplemented,
        "semantic: GROUPING SETS aggregate scans are not implemented");
  }

  if (aggregate.group_by_list_size() == 0) {
    std::vector<size_t> all_rows;
    all_rows.reserve(input_rows.size());
    for (size_t r = 0; r < input_rows.size(); ++r) {
      all_rows.push_back(r);
    }
    auto row_or = MaterializeAggregateGroup(aggregate,
                                            input_rows,
                                            all_rows,
                                            /*group_keys=*/nullptr,
                                            ctx);
    if (!row_or.ok()) return row_or.status();
    return std::vector<ColumnBindings>{std::move(*row_or)};
  }

  std::map<std::string, std::pair<std::vector<Value>, std::vector<size_t>>>
      groups;
  for (size_t r = 0; r < input_rows.size(); ++r) {
    EvalContext row_ctx = ctx;
    row_ctx.columns = &input_rows[r];
    std::vector<Value> keys;
    keys.reserve(aggregate.group_by_list_size());
    for (int g = 0; g < aggregate.group_by_list_size(); ++g) {
      const ::googlesql::ResolvedComputedColumn* gc =
          aggregate.group_by_list(g);
      if (gc == nullptr || gc->expr() == nullptr) {
        return absl::InternalError("semantic: group by column has null expr");
      }
      auto key = EvalExpr(*gc->expr(), row_ctx);
      if (!key.ok()) return key.status();
      keys.push_back(*std::move(key));
    }
    const std::string fp = GroupKeyFingerprint(keys);
    auto it = groups.find(fp);
    if (it == groups.end()) {
      groups.emplace(fp,
                     std::make_pair(std::move(keys), std::vector<size_t>{r}));
    } else {
      it->second.second.push_back(r);
    }
  }

  std::vector<ColumnBindings> out;
  out.reserve(groups.size());
  for (auto& kv : groups) {
    auto row_or = MaterializeAggregateGroup(
        aggregate, input_rows, kv.second.second, &kv.second.first, ctx);
    if (!row_or.ok()) return row_or.status();
    out.push_back(std::move(*row_or));
  }
  return out;
}

absl::StatusOr<std::vector<ColumnBindings>> ProjectRows(
    const ::googlesql::ResolvedProjectScan& project,
    const std::vector<ColumnBindings>& input_rows,
    EvalContext& ctx) {
  absl::flat_hash_map<int, const ::googlesql::ResolvedExpr*> expr_by_column_id;
  for (int i = 0; i < project.expr_list_size(); ++i) {
    const ::googlesql::ResolvedComputedColumn* cc = project.expr_list(i);
    if (cc == nullptr || cc->expr() == nullptr) {
      return absl::InternalError(
          "semantic: ResolvedComputedColumn has null expr");
    }
    expr_by_column_id[cc->column().column_id()] = cc->expr();
  }

  if (input_rows.empty()) {
    return std::vector<ColumnBindings>{};
  }
  std::vector<ColumnBindings> out;
  out.reserve(input_rows.size());
  for (const ColumnBindings& input : input_rows) {
    // Keep inner scan bindings (e.g. `$agg1`) so the executor's
    // `ProjectOneRow` pass can still resolve column refs in the
    // output projection expressions.
    ColumnBindings row = input;
    row.reserve(row.size() + project.column_list_size());
    for (int i = 0; i < project.column_list_size(); ++i) {
      const ::googlesql::ResolvedColumn& col = project.column_list(i);
      const int col_id = col.column_id();
      auto eit = expr_by_column_id.find(col_id);
      EvalContext row_ctx = ctx;
      row_ctx.columns = &input;
      Value v;
      if (eit != expr_by_column_id.end()) {
        auto eval_v = EvalExpr(*eit->second, row_ctx);
        if (!eval_v.ok()) return eval_v.status();
        v = *std::move(eval_v);
      } else {
        auto cit = input.find(col_id);
        if (cit == input.end()) {
          return absl::InternalError(
              absl::StrCat("semantic: ProjectScan missing binding for column '",
                           col.name(),
                           "'"));
        }
        v = cit->second;
      }
      row.emplace(col_id, std::move(v));
    }
    out.push_back(std::move(row));
  }
  return out;
}

absl::StatusOr<std::vector<ColumnBindings>> MaterializeTableScan(
    const ::googlesql::ResolvedTableScan& scan) {
  if (scan.table() == nullptr) {
    return absl::InternalError("semantic: TableScan has null table");
  }
  const auto* simple_table =
      dynamic_cast<const ::googlesql::SimpleTable*>(scan.table());
  if (simple_table == nullptr) {
    return MakeSemanticError(SemanticErrorReason::kNotImplemented,
                             absl::StrCat("semantic: table '",
                                          scan.table()->FullName(),
                                          "' is not iterable via SimpleTable"));
  }
  if (scan.column_list_size() != scan.column_index_list_size()) {
    return absl::InternalError(
        "semantic: TableScan column_list / column_index_list size mismatch");
  }
  std::vector<int> column_idxs;
  column_idxs.reserve(scan.column_list_size());
  for (int i = 0; i < scan.column_list_size(); ++i) {
    column_idxs.push_back(scan.column_index_list(i));
  }
  absl::StatusOr<std::unique_ptr<::googlesql::EvaluatorTableIterator>> iter_or =
      simple_table->CreateEvaluatorTableIterator(column_idxs);
  if (!iter_or.ok()) return iter_or.status();
  std::unique_ptr<::googlesql::EvaluatorTableIterator> iter =
      std::move(iter_or).value();
  std::vector<ColumnBindings> out;
  while (iter->NextRow()) {
    absl::Status st = iter->Status();
    if (!st.ok()) return st;
    ColumnBindings row;
    row.reserve(scan.column_list_size());
    for (int i = 0; i < scan.column_list_size(); ++i) {
      const ::googlesql::ResolvedColumn& col = scan.column_list(i);
      row.emplace(col.column_id(), iter->GetValue(i));
    }
    out.push_back(std::move(row));
  }
  absl::Status st = iter->Status();
  if (!st.ok()) return st;
  return out;
}

absl::StatusOr<std::vector<ColumnBindings>> MaterializeSingleRowScan(
    const ::googlesql::ResolvedSingleRowScan& scan) {
  ColumnBindings row;
  for (int i = 0; i < scan.column_list_size(); ++i) {
    const ::googlesql::ResolvedColumn& col = scan.column_list(i);
    row.emplace(col.column_id(), Value::Null(col.type()));
  }
  return std::vector<ColumnBindings>{std::move(row)};
}

absl::StatusOr<std::vector<ColumnBindings>> MaterializeWithRefScan(
    const ::googlesql::ResolvedWithRefScan& ref, EvalContext& ctx) {
  if (ctx.with_tables == nullptr) {
    return absl::InternalError(
        "semantic: WithRefScan without active WithScan bindings");
  }
  auto it = ctx.with_tables->find(std::string(ref.with_query_name()));
  if (it == ctx.with_tables->end()) {
    return MakeSemanticError(
        SemanticErrorReason::kInvalidArgument,
        absl::StrCat("semantic: unknown CTE '", ref.with_query_name(), "'"));
  }
  const CteTable& cte = it->second;
  if (static_cast<int>(cte.column_ids.size()) != ref.column_list_size()) {
    return absl::InternalError(
        "semantic: WithRefScan column count does not match CTE");
  }
  std::vector<ColumnBindings> out;
  out.reserve(cte.rows.size());
  for (const ColumnBindings& cte_row : cte.rows) {
    ColumnBindings row;
    row.reserve(ref.column_list_size());
    for (int i = 0; i < ref.column_list_size(); ++i) {
      const ::googlesql::ResolvedColumn& dst = ref.column_list(i);
      const int src_id = cte.column_ids[i];
      auto cit = cte_row.find(src_id);
      if (cit == cte_row.end()) {
        return absl::InternalError(
            absl::StrCat("semantic: CTE row missing column_id=", src_id));
      }
      row.emplace(dst.column_id(), cit->second);
    }
    out.push_back(std::move(row));
  }
  return out;
}

absl::StatusOr<std::vector<ColumnBindings>> MaterializeSetOperationScan(
    const ::googlesql::ResolvedSetOperationScan& set_op, EvalContext& ctx) {
  if (set_op.op_type() != ::googlesql::ResolvedSetOperationScan::UNION_ALL) {
    return MakeSemanticError(SemanticErrorReason::kNotImplemented,
                             "semantic: SetOperationScan op is not UNION ALL");
  }
  std::vector<ColumnBindings> out;
  for (int i = 0; i < set_op.input_item_list_size(); ++i) {
    const ::googlesql::ResolvedSetOperationItem* item =
        set_op.input_item_list(i);
    if (item == nullptr || item->scan() == nullptr) {
      return absl::InternalError("semantic: SetOperationItem has null scan");
    }
    auto part = MaterializeScanImpl(item->scan(), ctx);
    if (!part.ok()) return part.status();
    const ::googlesql::ResolvedScan* part_scan = item->scan();
    for (const ColumnBindings& part_row : *part) {
      ColumnBindings remapped;
      for (int j = 0; j < set_op.column_list_size(); ++j) {
        const int out_id = set_op.column_list(j).column_id();
        const ::googlesql::Type* out_type = set_op.column_list(j).type();
        if (part_scan != nullptr && j < part_scan->column_list_size()) {
          const int src_id = part_scan->column_list(j).column_id();
          auto it = part_row.find(src_id);
          if (it != part_row.end()) {
            remapped.emplace(out_id, it->second);
            continue;
          }
        }
        remapped.emplace(out_id, Value::Null(out_type));
      }
      out.push_back(std::move(remapped));
    }
  }
  return out;
}

absl::StatusOr<std::vector<ColumnBindings>> MaterializeJoinScan(
    const ::googlesql::ResolvedJoinScan& join, EvalContext& ctx) {
  if (join.is_lateral()) {
    return MakeSemanticError(
        SemanticErrorReason::kNotImplemented,
        "semantic: lateral JoinScan is not yet implemented");
  }
  if (join.join_type() == ::googlesql::ResolvedJoinScan::RIGHT ||
      join.join_type() == ::googlesql::ResolvedJoinScan::FULL) {
    return MakeSemanticError(SemanticErrorReason::kNotImplemented,
                             "semantic: RIGHT/FULL JOIN not yet implemented");
  }
  auto left_or = MaterializeScanImpl(join.left_scan(), ctx);
  if (!left_or.ok()) return left_or.status();
  auto right_or = MaterializeScanImpl(join.right_scan(), ctx);
  if (!right_or.ok()) return right_or.status();

  const bool is_left_outer =
      join.join_type() == ::googlesql::ResolvedJoinScan::LEFT;
  const bool is_cross =
      join.join_expr() == nullptr &&
      join.join_type() == ::googlesql::ResolvedJoinScan::INNER;

  const ::googlesql::ResolvedScan* rscan = StripBarrierScans(join.right_scan());

  std::vector<ColumnBindings> out;
  for (const ColumnBindings& lrow : *left_or) {
    bool any_match = false;
    for (const ColumnBindings& rrow : *right_or) {
      ColumnBindings merged = lrow;
      merged.insert(rrow.begin(), rrow.end());
      EvalContext merged_ctx = ctx;
      merged_ctx.columns = &merged;
      bool include = is_cross || join.join_expr() == nullptr;
      if (!include) {
        auto ok = EvalBoolExpr(join.join_expr(), merged_ctx);
        if (!ok.ok()) return ok.status();
        include = *ok;
      }
      if (include) {
        any_match = true;
        out.push_back(std::move(merged));
      }
    }
    if (!any_match && is_left_outer) {
      ColumnBindings merged = lrow;
      if (rscan != nullptr) {
        for (int i = 0; i < rscan->column_list_size(); ++i) {
          const ::googlesql::ResolvedColumn& col = rscan->column_list(i);
          merged.emplace(col.column_id(), Value::Null(col.type()));
        }
      }
      out.push_back(std::move(merged));
    }
  }
  return out;
}

absl::StatusOr<std::vector<ColumnBindings>> MaterializeArrayScan(
    const ::googlesql::ResolvedArrayScan& scan, EvalContext& ctx) {
  if (scan.join_expr() != nullptr && scan.input_scan() != nullptr) {
    auto left_or = MaterializeScanImpl(scan.input_scan(), ctx);
    if (!left_or.ok()) return left_or.status();
    std::vector<ColumnBindings> out;
    for (const ColumnBindings& lrow : *left_or) {
      EvalContext row_ctx = ctx;
      row_ctx.columns = &lrow;
      auto array_rows = array_struct::EvaluateArrayScan(scan, row_ctx);
      if (!array_rows.ok()) return array_rows.status();
      bool any = false;
      for (const ColumnBindings& arow : *array_rows) {
        ColumnBindings merged = lrow;
        merged.insert(arow.begin(), arow.end());
        EvalContext merged_ctx = ctx;
        merged_ctx.columns = &merged;
        auto ok = EvalBoolExpr(scan.join_expr(), merged_ctx);
        if (!ok.ok()) return ok.status();
        if (*ok) {
          any = true;
          out.push_back(std::move(merged));
        }
      }
      if (!any && scan.is_outer()) {
        ColumnBindings merged = lrow;
        for (int i = 0; i < scan.element_column_list_size(); ++i) {
          merged.emplace(scan.element_column_list(i).column_id(),
                         Value::Null(scan.element_column_list(i).type()));
        }
        if (scan.array_offset_column() != nullptr) {
          merged.emplace(scan.array_offset_column()->column().column_id(),
                         Value::NullInt64());
        }
        out.push_back(std::move(merged));
      }
    }
    return out;
  }

  if (scan.input_scan() != nullptr &&
      scan.input_scan()->node_kind() != ::googlesql::RESOLVED_SINGLE_ROW_SCAN) {
    auto left_or = MaterializeScanImpl(scan.input_scan(), ctx);
    if (!left_or.ok()) return left_or.status();
    std::vector<ColumnBindings> out;
    for (const ColumnBindings& lrow : *left_or) {
      EvalContext row_ctx = ctx;
      row_ctx.columns = &lrow;
      auto array_rows = array_struct::EvaluateArrayScan(scan, row_ctx);
      if (!array_rows.ok()) return array_rows.status();
      for (const ColumnBindings& arow : *array_rows) {
        ColumnBindings merged = lrow;
        merged.insert(arow.begin(), arow.end());
        out.push_back(std::move(merged));
      }
    }
    return out;
  }

  auto rows_or = array_struct::EvaluateArrayScan(scan, ctx);
  if (!rows_or.ok()) return rows_or.status();
  const int n_arrays = scan.array_expr_list_size();
  for (ColumnBindings& row : *rows_or) {
    array_struct::AliasUnnestPublicColumnIds(scan, n_arrays, row);
  }
  return rows_or;
}

absl::StatusOr<int64_t> EvalLimitOffsetInt64(
    const ::googlesql::ResolvedExpr* expr,
    const EvalContext& ctx,
    absl::string_view role) {
  if (expr == nullptr) {
    return absl::InvalidArgumentError(
        absl::StrCat("semantic: LIMIT/OFFSET missing ", role, " expression"));
  }
  auto value_or = EvalExpr(*expr, ctx);
  if (!value_or.ok()) return value_or.status();
  const Value& value = *value_or;
  if (value.is_null()) {
    return MakeSemanticError(
        SemanticErrorReason::kInvalidArgument,
        absl::StrCat("semantic: LIMIT/OFFSET ", role, " must not be NULL"));
  }
  if (value.type_kind() != ::googlesql::TYPE_INT64) {
    return MakeSemanticError(
        SemanticErrorReason::kInvalidArgument,
        absl::StrCat("semantic: LIMIT/OFFSET ", role, " must be INT64"));
  }
  return value.int64_value();
}

absl::StatusOr<std::vector<ColumnBindings>> MaterializeLimitOffsetScan(
    const ::googlesql::ResolvedLimitOffsetScan& scan, EvalContext& ctx) {
  auto input_or = MaterializeScanImpl(scan.input_scan(), ctx);
  if (!input_or.ok()) return input_or.status();
  std::vector<ColumnBindings> rows = *std::move(input_or);

  int64_t offset = 0;
  if (scan.offset() != nullptr) {
    auto offset_or = EvalLimitOffsetInt64(scan.offset(), ctx, "offset");
    if (!offset_or.ok()) return offset_or.status();
    offset = *offset_or;
    if (offset < 0) {
      return MakeSemanticError(SemanticErrorReason::kInvalidArgument,
                               "semantic: OFFSET must be non-negative");
    }
  }

  size_t start = static_cast<size_t>(offset);
  if (start >= rows.size()) {
    return std::vector<ColumnBindings>{};
  }

  size_t end = rows.size();
  if (scan.limit() != nullptr) {
    auto limit_or = EvalLimitOffsetInt64(scan.limit(), ctx, "limit");
    if (!limit_or.ok()) return limit_or.status();
    const int64_t limit = *limit_or;
    if (limit < 0) {
      return MakeSemanticError(SemanticErrorReason::kInvalidArgument,
                               "semantic: LIMIT must be non-negative");
    }
    end = std::min(end, start + static_cast<size_t>(limit));
  }

  return std::vector<ColumnBindings>(rows.begin() + start, rows.begin() + end);
}

absl::StatusOr<std::vector<ColumnBindings>> MaterializeScanImpl(
    const ::googlesql::ResolvedScan* scan, EvalContext& ctx) {
  scan = StripBarrierScans(scan);
  if (scan == nullptr) {
    return absl::InvalidArgumentError("semantic: null scan");
  }
  switch (scan->node_kind()) {
    case ::googlesql::RESOLVED_TABLE_SCAN:
      return MaterializeTableScan(
          *scan->GetAs<::googlesql::ResolvedTableScan>());
    case ::googlesql::RESOLVED_SINGLE_ROW_SCAN:
      return MaterializeSingleRowScan(
          *scan->GetAs<::googlesql::ResolvedSingleRowScan>());
    case ::googlesql::RESOLVED_ARRAY_SCAN:
      return MaterializeArrayScan(
          *scan->GetAs<::googlesql::ResolvedArrayScan>(), ctx);
    case ::googlesql::RESOLVED_PROJECT_SCAN: {
      const auto* project = scan->GetAs<::googlesql::ResolvedProjectScan>();
      auto input = MaterializeScanImpl(project->input_scan(), ctx);
      if (!input.ok()) return input.status();
      return ProjectRows(*project, *input, ctx);
    }
    case ::googlesql::RESOLVED_FILTER_SCAN: {
      const auto* filter = scan->GetAs<::googlesql::ResolvedFilterScan>();
      auto input = MaterializeScanImpl(filter->input_scan(), ctx);
      if (!input.ok()) return input.status();
      std::vector<ColumnBindings> out;
      for (const ColumnBindings& row : *input) {
        EvalContext row_ctx = ctx;
        row_ctx.columns = &row;
        auto ok = EvalBoolExpr(filter->filter_expr(), row_ctx);
        if (!ok.ok()) return ok.status();
        if (*ok) out.push_back(row);
      }
      return out;
    }
    case ::googlesql::RESOLVED_ORDER_BY_SCAN: {
      const auto* order = scan->GetAs<::googlesql::ResolvedOrderByScan>();
      auto input = MaterializeScanImpl(order->input_scan(), ctx);
      if (!input.ok()) return input.status();
      std::vector<ColumnBindings> rows = *std::move(input);
      std::stable_sort(
          rows.begin(),
          rows.end(),
          [&](const ColumnBindings& a, const ColumnBindings& b) {
            for (int i = 0; i < order->order_by_item_list_size(); ++i) {
              const ::googlesql::ResolvedOrderByItem* item =
                  order->order_by_item_list(i);
              if (item == nullptr || item->column_ref() == nullptr) {
                continue;
              }
              const int col_id = item->column_ref()->column().column_id();
              auto av = a.find(col_id);
              auto bv = b.find(col_id);
              Value va = av == a.end() ? Value() : av->second;
              Value vb = bv == b.end() ? Value() : bv->second;
              if (ValueEqual(va, vb)) continue;
              if (va.is_null() || vb.is_null()) {
                bool nulls_first;
                switch (item->null_order()) {
                  case ::googlesql::ResolvedOrderByItem::NULLS_FIRST:
                    nulls_first = true;
                    break;
                  case ::googlesql::ResolvedOrderByItem::NULLS_LAST:
                    nulls_first = false;
                    break;
                  default:
                    nulls_first = !item->is_descending();
                    break;
                }
                if (va.is_null() && vb.is_null()) continue;
                if (va.is_null()) return nulls_first;
                return !nulls_first;
              }
              bool less = ValueLess(va, vb);
              if (item->is_descending()) less = !less;
              return less;
            }
            return false;
          });
      return rows;
    }
    case ::googlesql::RESOLVED_WITH_SCAN: {
      const auto* with_scan = scan->GetAs<::googlesql::ResolvedWithScan>();
      absl::flat_hash_map<std::string, CteTable> tables;
      for (int i = 0; i < with_scan->with_entry_list_size(); ++i) {
        const ::googlesql::ResolvedWithEntry* entry =
            with_scan->with_entry_list(i);
        if (entry == nullptr || entry->with_subquery() == nullptr) {
          return absl::InternalError("semantic: malformed WithEntry");
        }
        const ::googlesql::ResolvedScan* sub = entry->with_subquery();
        auto rows = MaterializeScanImpl(sub, ctx);
        if (!rows.ok()) return rows.status();
        CteTable table;
        table.rows = *std::move(rows);
        table.column_ids.reserve(sub->column_list_size());
        for (int j = 0; j < sub->column_list_size(); ++j) {
          table.column_ids.push_back(sub->column_list(j).column_id());
        }
        tables[std::string(entry->with_query_name())] = std::move(table);
      }
      EvalContext body_ctx = ctx;
      body_ctx.with_tables = &tables;
      return MaterializeScanImpl(with_scan->query(), body_ctx);
    }
    case ::googlesql::RESOLVED_WITH_REF_SCAN:
      return MaterializeWithRefScan(
          *scan->GetAs<::googlesql::ResolvedWithRefScan>(), ctx);
    case ::googlesql::RESOLVED_JOIN_SCAN:
      return MaterializeJoinScan(*scan->GetAs<::googlesql::ResolvedJoinScan>(),
                                 ctx);
    case ::googlesql::RESOLVED_SET_OPERATION_SCAN:
      return MaterializeSetOperationScan(
          *scan->GetAs<::googlesql::ResolvedSetOperationScan>(), ctx);
    case ::googlesql::RESOLVED_AGGREGATE_SCAN:
      return MaterializeAggregateScan(
          *scan->GetAs<::googlesql::ResolvedAggregateScan>(), ctx);
    case ::googlesql::RESOLVED_LIMIT_OFFSET_SCAN:
      return MaterializeLimitOffsetScan(
          *scan->GetAs<::googlesql::ResolvedLimitOffsetScan>(), ctx);
    default:
      return MakeSemanticError(SemanticErrorReason::kNotImplemented,
                               absl::StrCat("semantic: scan kind ",
                                            scan->node_kind_string(),
                                            " is not yet implemented"));
  }
}

}  // namespace

absl::StatusOr<const ::googlesql::ResolvedProjectScan*> FindOutputProjectScan(
    const ::googlesql::ResolvedScan* scan) {
  scan = StripBarrierScans(scan);
  if (scan == nullptr) {
    return nullptr;
  }
  switch (scan->node_kind()) {
    case ::googlesql::RESOLVED_PROJECT_SCAN:
      return scan->GetAs<::googlesql::ResolvedProjectScan>();
    case ::googlesql::RESOLVED_ORDER_BY_SCAN:
      return FindOutputProjectScan(
          scan->GetAs<::googlesql::ResolvedOrderByScan>()->input_scan());
    case ::googlesql::RESOLVED_WITH_SCAN:
      return FindOutputProjectScan(
          scan->GetAs<::googlesql::ResolvedWithScan>()->query());
    case ::googlesql::RESOLVED_FILTER_SCAN:
      return FindOutputProjectScan(
          scan->GetAs<::googlesql::ResolvedFilterScan>()->input_scan());
    case ::googlesql::RESOLVED_LIMIT_OFFSET_SCAN:
      return FindOutputProjectScan(
          scan->GetAs<::googlesql::ResolvedLimitOffsetScan>()->input_scan());
    case ::googlesql::RESOLVED_ARRAY_SCAN: {
      const auto* array = scan->GetAs<::googlesql::ResolvedArrayScan>();
      if (array->input_scan() != nullptr) {
        return FindOutputProjectScan(array->input_scan());
      }
      return nullptr;
    }
    case ::googlesql::RESOLVED_JOIN_SCAN: {
      const auto* join = scan->GetAs<::googlesql::ResolvedJoinScan>();
      auto left = FindOutputProjectScan(join->left_scan());
      if (!left.ok()) return left.status();
      if (*left != nullptr) return left;
      return FindOutputProjectScan(join->right_scan());
    }
    case ::googlesql::RESOLVED_SET_OPERATION_SCAN: {
      const auto* set_op = scan->GetAs<::googlesql::ResolvedSetOperationScan>();
      for (int i = 0; i < set_op->input_item_list_size(); ++i) {
        const ::googlesql::ResolvedSetOperationItem* item =
            set_op->input_item_list(i);
        if (item == nullptr || item->scan() == nullptr) continue;
        auto found = FindOutputProjectScan(item->scan());
        if (!found.ok()) return found.status();
        if (*found != nullptr) return found;
      }
      return nullptr;
    }
    default:
      return nullptr;
  }
}

absl::StatusOr<std::vector<ColumnBindings>> MaterializeScan(
    const ::googlesql::ResolvedScan* scan, EvalContext& ctx) {
  return MaterializeScanImpl(scan, ctx);
}

absl::StatusOr<Value> EvalSubqueryExpr(
    const ::googlesql::ResolvedSubqueryExpr& node, const EvalContext& ctx) {
  if (node.subquery() == nullptr) {
    return absl::InvalidArgumentError(
        "semantic: ResolvedSubqueryExpr has null subquery");
  }
  EvalContext inner_ctx = ctx;
  ColumnBindings outer_bind;
  if (ctx.columns != nullptr) {
    outer_bind = *ctx.columns;
  }
  inner_ctx.columns = &outer_bind;

  auto rows_or = MaterializeScanImpl(node.subquery(), inner_ctx);
  if (!rows_or.ok()) return rows_or.status();
  const std::vector<ColumnBindings>& rows = *rows_or;
  const ::googlesql::ResolvedScan* sub = StripBarrierScans(node.subquery());
  int value_col_id = -1;
  if (sub != nullptr && sub->column_list_size() > 0) {
    value_col_id = sub->column_list(0).column_id();
  }

  auto value_from_row =
      [&](const ColumnBindings& row) -> absl::StatusOr<Value> {
    if (value_col_id >= 0) {
      auto it = row.find(value_col_id);
      if (it != row.end()) return it->second;
    }
    if (!row.empty()) return row.begin()->second;
    return absl::InternalError(
        "semantic: subquery row missing projected column");
  };

  switch (node.subquery_type()) {
    case ::googlesql::ResolvedSubqueryExpr::EXISTS:
      return Value::Bool(!rows.empty());
    case ::googlesql::ResolvedSubqueryExpr::SCALAR: {
      if (rows.empty()) return Value::Null(node.type());
      if (rows.size() > 1) {
        return MakeSemanticError(
            SemanticErrorReason::kInvalidArgument,
            "semantic: scalar subquery returned more than one row");
      }
      return value_from_row(rows[0]);
    }
    case ::googlesql::ResolvedSubqueryExpr::ARRAY: {
      if (node.type() == nullptr || !node.type()->IsArray()) {
        return absl::InternalError(
            "semantic: ARRAY subquery missing ARRAY type");
      }
      std::vector<Value> elements;
      elements.reserve(rows.size());
      for (const ColumnBindings& row : rows) {
        auto v = value_from_row(row);
        if (!v.ok()) return v.status();
        elements.push_back(*std::move(v));
      }
      return Value::Array(node.type()->AsArray(), std::move(elements));
    }
    case ::googlesql::ResolvedSubqueryExpr::IN: {
      if (node.in_expr() == nullptr) {
        return absl::InvalidArgumentError(
            "semantic: IN subquery missing in_expr");
      }
      auto lhs = EvalExpr(*node.in_expr(), ctx);
      if (!lhs.ok()) return lhs.status();
      if (lhs->is_null()) return Value::NullBool();
      for (const ColumnBindings& row : rows) {
        auto rv = value_from_row(row);
        if (!rv.ok()) return rv.status();
        if (lhs->is_null() || rv->is_null()) continue;
        if (lhs->Equals(*rv)) return Value::Bool(true);
      }
      return Value::Bool(false);
    }
    default:
      return MakeSemanticError(SemanticErrorReason::kNotImplemented,
                               "semantic: subquery type not yet implemented");
  }
}

}  // namespace semantic
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
