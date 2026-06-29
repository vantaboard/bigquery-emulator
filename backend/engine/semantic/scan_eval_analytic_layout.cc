
#include "googlesql/public/functions/date_time_util.h"
#include "googlesql/public/type.h"

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
                       const Value& va,
                       const Value& vb) {
  if (item == nullptr) return 0;
  if (ValueEqual(va, vb)) return 0;
  if (va.is_null() || vb.is_null()) {
    bool nulls_first = false;
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
  return (item->is_descending() ? !ValueLess(va, vb) : ValueLess(va, vb)) ? -1
                                                                          : 1;
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

absl::StatusOr<int64_t> EvalFrameOffsetInt64(
    const ::googlesql::ResolvedExpr* expr, EvalContext& ctx) {
  if (expr == nullptr) {
    return absl::InvalidArgumentError("semantic: frame offset missing");
  }
  auto value_or = EvalExpr(*expr, ctx);
  if (!value_or.ok()) return value_or.status();
  if (value_or->is_null()) {
    return MakeSemanticError(SemanticErrorReason::kInvalidArgument,
                             "semantic: frame offset must not be NULL");
  }
  if (value_or->type_kind() == ::googlesql::TYPE_INT64) {
    return value_or->int64_value();
  }
  if (value_or->type_kind() == ::googlesql::TYPE_DOUBLE) {
    return static_cast<int64_t>(value_or->double_value());
  }
  return MakeSemanticError(SemanticErrorReason::kInvalidArgument,
                           "semantic: frame offset must be INT64");
}

absl::StatusOr<Value> AddDateOffset(Value date, int64_t offset_days) {
  if (date.is_null()) return date;
  int32_t out = 0;
  if (auto s = ::googlesql::functions::AddDate(
          date.date_value(),
          ::googlesql::functions::DateTimestampPart::DAY,
          offset_days,
          &out);
      !s.ok()) {
    return s;
  }
  return Value::Date(out);
}

absl::StatusOr<Value> AddTimestampOffset(Value ts, int64_t offset_micros) {
  if (ts.is_null()) return ts;
  return Value::TimestampFromUnixMicros(ts.ToUnixMicros() + offset_micros);
}

}  // namespace

Value LookupColumnValue(const ColumnBindings& row, int col_id) {
  auto it = row.find(col_id);
  if (it == row.end()) return Value();
  return it->second;
}

absl::StatusOr<Value> AddValues(const Value& a, const Value& b) {
  if (a.is_null()) return b;
  if (b.is_null()) return a;
  if (a.type_kind() == ::googlesql::TYPE_INT64 &&
      b.type_kind() == ::googlesql::TYPE_INT64) {
    return Value::Int64(a.int64_value() + b.int64_value());
  }
  if (a.type_kind() == ::googlesql::TYPE_DOUBLE &&
      b.type_kind() == ::googlesql::TYPE_DOUBLE) {
    return Value::Double(a.double_value() + b.double_value());
  }
  if (a.type_kind() == ::googlesql::TYPE_FLOAT &&
      b.type_kind() == ::googlesql::TYPE_FLOAT) {
    return Value::Float(a.float_value() + b.float_value());
  }
  return MakeSemanticError(SemanticErrorReason::kNotImplemented,
                           "semantic: analytic SUM add unsupported types");
}

absl::StatusOr<Value> FrameBoundValue(
    const ::googlesql::ResolvedWindowFrameExpr* bound,
    const Value& current_order,
    EvalContext& ctx) {
  if (bound == nullptr) {
    return absl::InvalidArgumentError("semantic: frame bound missing");
  }
  switch (bound->boundary_type()) {
    case ::googlesql::ResolvedWindowFrameExpr::UNBOUNDED_PRECEDING:
    case ::googlesql::ResolvedWindowFrameExpr::UNBOUNDED_FOLLOWING:
      return Value::NullInt64();
    case ::googlesql::ResolvedWindowFrameExpr::CURRENT_ROW:
      return current_order;
    case ::googlesql::ResolvedWindowFrameExpr::OFFSET_PRECEDING: {
      auto offset_or = EvalFrameOffsetInt64(bound->expression(), ctx);
      if (!offset_or.ok()) return offset_or.status();
      if (current_order.type_kind() == ::googlesql::TYPE_DATE) {
        return AddDateOffset(current_order, -*offset_or);
      }
      if (current_order.type_kind() == ::googlesql::TYPE_TIMESTAMP) {
        return AddTimestampOffset(current_order, -*offset_or);
      }
      break;
    }
    case ::googlesql::ResolvedWindowFrameExpr::OFFSET_FOLLOWING: {
      auto offset_or = EvalFrameOffsetInt64(bound->expression(), ctx);
      if (!offset_or.ok()) return offset_or.status();
      if (current_order.type_kind() == ::googlesql::TYPE_DATE) {
        return AddDateOffset(current_order, *offset_or);
      }
      if (current_order.type_kind() == ::googlesql::TYPE_TIMESTAMP) {
        return AddTimestampOffset(current_order, *offset_or);
      }
      break;
    }
  }
  return MakeSemanticError(SemanticErrorReason::kNotImplemented,
                           "semantic: unsupported frame bound for order key");
}

bool ValueInClosedRange(const Value& value,
                        const Value& low,
                        bool has_low,
                        const Value& high,
                        bool has_high) {
  if (value.is_null()) return false;
  if (has_low && !value.is_null() && !low.is_null() && ValueLess(value, low)) {
    return false;
  }
  if (has_high && !value.is_null() && !high.is_null() &&
      ValueLess(high, value)) {
    return false;
  }
  return true;
}

AnalyticGroupLayout BuildAnalyticGroupLayout(
    const ::googlesql::ResolvedAnalyticFunctionGroup& group,
    const ::googlesql::ResolvedWindowOrdering* order_spec,
    const std::vector<ColumnBindings>& input_rows) {
  AnalyticGroupLayout layout;
  layout.partition_fps.resize(input_rows.size());
  for (size_t r = 0; r < input_rows.size(); ++r) {
    layout.partition_fps[r] =
        PartitionFingerprint(PartitionKeysForRow(group, input_rows[r]));
  }

  std::vector<size_t> order(input_rows.size());
  std::iota(order.begin(), order.end(), 0);
  std::stable_sort(
      order.begin(), order.end(), [&](size_t a_idx, size_t b_idx) -> bool {
        if (layout.partition_fps[a_idx] != layout.partition_fps[b_idx]) {
          return layout.partition_fps[a_idx] < layout.partition_fps[b_idx];
        }
        if (order_spec == nullptr) return false;
        for (int i = 0; i < order_spec->order_by_item_list_size(); ++i) {
          const ::googlesql::ResolvedOrderByItem* item =
              order_spec->order_by_item_list(i);
          if (item == nullptr || item->column_ref() == nullptr) {
            continue;
          }
          const int col_id = item->column_ref()->column().column_id();
          switch (CompareOrderByItem(
              item,
              LookupColumnValue(input_rows[a_idx], col_id),
              LookupColumnValue(input_rows[b_idx], col_id))) {
            case -1:
              return true;
            case 1:
              return false;
            default:
              break;
          }
        }
        return false;
      });

  absl::flat_hash_map<std::string, int64_t> next_in_partition;
  layout.row_numbers.assign(input_rows.size(), 0);
  for (size_t sorted_idx : order) {
    int64_t& n = next_in_partition[layout.partition_fps[sorted_idx]];
    n++;
    layout.row_numbers[sorted_idx] = n;
  }
  return layout;
}

}  // namespace scan_eval_internal
}  // namespace semantic
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
