#include "backend/engine/semantic/scan_eval_internal.h"

#include <algorithm>
#include <map>
#include <string>
#include <utility>
#include <vector>

#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "absl/strings/ascii.h"
#include "absl/strings/str_cat.h"
#include "backend/engine/semantic/error.h"
#include "backend/engine/semantic/functions/specialized_funcs.h"
#include "backend/engine/semantic/value.h"
#include "googlesql/public/type.h"
#include "googlesql/resolved_ast/resolved_ast.h"
#include "googlesql/resolved_ast/resolved_node_kind.pb.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace semantic {
namespace scan_eval_internal {

using ::bigquery_emulator::backend::engine::semantic::EvalContext;
using ::bigquery_emulator::backend::engine::semantic::EvalExpr;

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

}  // namespace scan_eval_internal
}  // namespace semantic
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
