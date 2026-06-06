#include <cstdint>
#include <numeric>
#include <string>
#include <vector>

#include "absl/container/flat_hash_map.h"
#include "absl/status/statusor.h"
#include "absl/strings/ascii.h"
#include "absl/strings/str_cat.h"
#include "backend/engine/semantic/error.h"
#include "backend/engine/semantic/eval_expr.h"
#include "backend/engine/semantic/scan_eval_internal.h"
#include "backend/engine/semantic/value.h"
#include "googlesql/resolved_ast/resolved_ast.h"
#include "googlesql/resolved_ast/resolved_node_kind.pb.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace semantic {
namespace scan_eval_internal {

namespace {

std::string PartitionFingerprint(const std::vector<Value>& keys) {
  std::string fp;
  for (const Value& key : keys) {
    absl::StrAppend(&fp, key.DebugString(), "\x1e");
  }
  return fp;
}

int CompareOrderByItem(const ::googlesql::ResolvedOrderByItem* item,
                       const Value& va, const Value& vb) {
  if (item == nullptr) return 0;
  if (ValueEqual(va, vb)) return 0;
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
    if (va.is_null() && vb.is_null()) return 0;
    if (va.is_null()) return nulls_first ? -1 : 1;
    return nulls_first ? 1 : -1;
  }
  bool less = ValueLess(va, vb);
  if (item->is_descending()) less = !less;
  return less ? -1 : 1;
}

Value LookupColumnValue(const ColumnBindings& row, int col_id) {
  auto it = row.find(col_id);
  if (it == row.end()) return Value();
  return it->second;
}

std::vector<Value> PartitionKeysForRow(
    const ::googlesql::ResolvedAnalyticFunctionGroup& group,
    const ColumnBindings& row) {
  std::vector<Value> keys;
  const ::googlesql::ResolvedWindowPartitioning* partition =
      group.partition_by();
  if (partition == nullptr) return keys;
  keys.reserve(static_cast<size_t>(partition->partition_by_list_size()));
  for (int p = 0; p < partition->partition_by_list_size(); ++p) {
    const ::googlesql::ResolvedColumnRef* ref = partition->partition_by_list(p);
    if (ref == nullptr) continue;
    keys.push_back(LookupColumnValue(row, ref->column().column_id()));
  }
  return keys;
}

}  // namespace

absl::StatusOr<std::vector<ColumnBindings>> MaterializeAnalyticScan(
    const ::googlesql::ResolvedAnalyticScan& analytic, EvalContext& ctx) {
  auto input_or = MaterializeScanImpl(analytic.input_scan(), ctx);
  if (!input_or.ok()) return input_or.status();
  std::vector<ColumnBindings> input_rows = *std::move(input_or);
  std::vector<ColumnBindings> out_rows = input_rows;

  for (int g = 0; g < analytic.function_group_list_size(); ++g) {
    const ::googlesql::ResolvedAnalyticFunctionGroup* group =
        analytic.function_group_list(g);
    if (group == nullptr) continue;

    const ::googlesql::ResolvedWindowOrdering* order_spec = group->order_by();

    std::vector<std::string> partition_fps(input_rows.size());
    for (size_t r = 0; r < input_rows.size(); ++r) {
      partition_fps[r] =
          PartitionFingerprint(PartitionKeysForRow(*group, input_rows[r]));
    }

    std::vector<size_t> order(input_rows.size());
    std::iota(order.begin(), order.end(), 0);
    std::stable_sort(order.begin(), order.end(),
                     [&](size_t a_idx, size_t b_idx) -> bool {
                       if (partition_fps[a_idx] != partition_fps[b_idx]) {
                         return partition_fps[a_idx] < partition_fps[b_idx];
                       }
                       if (order_spec == nullptr) return false;
                       for (int i = 0; i < order_spec->order_by_item_list_size();
                            ++i) {
                         const ::googlesql::ResolvedOrderByItem* item =
                             order_spec->order_by_item_list(i);
                         if (item == nullptr || item->column_ref() == nullptr) {
                           continue;
                         }
                         const int col_id =
                             item->column_ref()->column().column_id();
                         const int cmp = CompareOrderByItem(
                             item, LookupColumnValue(input_rows[a_idx], col_id),
                             LookupColumnValue(input_rows[b_idx], col_id));
                         if (cmp != 0) return cmp < 0;
                       }
                       return false;
                     });

    absl::flat_hash_map<std::string, int64_t> next_in_partition;
    std::vector<int64_t> row_numbers(input_rows.size(), 0);
    for (size_t sorted_idx : order) {
      int64_t& n = next_in_partition[partition_fps[sorted_idx]];
      n++;
      row_numbers[sorted_idx] = n;
    }

    for (int f = 0; f < group->analytic_function_list_size(); ++f) {
      const ::googlesql::ResolvedComputedColumnBase* cc =
          group->analytic_function_list(f);
      if (cc == nullptr || cc->expr() == nullptr ||
          cc->expr()->node_kind() !=
              ::googlesql::RESOLVED_ANALYTIC_FUNCTION_CALL) {
        return MakeSemanticError(
            SemanticErrorReason::kNotImplemented,
            "semantic: analytic column is not a function call");
      }
      const auto* afn =
          cc->expr()->GetAs<::googlesql::ResolvedAnalyticFunctionCall>();
      if (afn == nullptr || afn->function() == nullptr) {
        return absl::InternalError("semantic: analytic function is null");
      }
      std::string fname = absl::AsciiStrToLower(
          afn->function()->FullName(/*include_group=*/false));
      if (fname.empty()) {
        fname = absl::AsciiStrToLower(afn->function()->Name());
      }
      if (fname != "row_number") {
        return MakeSemanticError(
            SemanticErrorReason::kNotImplemented,
            absl::StrCat("semantic: analytic function '", fname,
                         "' is not yet implemented"));
      }
      const int out_col_id = cc->column().column_id();
      for (size_t r = 0; r < out_rows.size(); ++r) {
        out_rows[r][out_col_id] = Value::Int64(row_numbers[r]);
      }
    }
  }

  return out_rows;
}

}  // namespace scan_eval_internal
}  // namespace semantic
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
