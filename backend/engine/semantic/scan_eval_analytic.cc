#include <algorithm>
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

double PercentileContFromSorted(const std::vector<double>& sorted, double p) {
  if (sorted.empty()) return 0.0;
  if (sorted.size() == 1) return sorted.front();
  const double rank = p * static_cast<double>(sorted.size() - 1);
  const size_t lo = static_cast<size_t>(rank);
  const size_t hi = std::min(lo + 1, sorted.size() - 1);
  const double frac = rank - static_cast<double>(lo);
  return sorted[lo] + frac * (sorted[hi] - sorted[lo]);
}

absl::StatusOr<Value> PercentileContValue(const Value& percentile,
                                          const std::vector<Value>& values) {
  if (percentile.is_null()) return Value::NullDouble();
  const double p = percentile.double_value();
  std::vector<double> nums;
  nums.reserve(values.size());
  for (const Value& v : values) {
    if (v.is_null()) continue;
    if (v.type_kind() == ::googlesql::TYPE_INT64) {
      nums.push_back(static_cast<double>(v.int64_value()));
    } else if (v.type_kind() == ::googlesql::TYPE_DOUBLE) {
      nums.push_back(v.double_value());
    } else if (v.type_kind() == ::googlesql::TYPE_FLOAT) {
      nums.push_back(static_cast<double>(v.float_value()));
    } else if (v.type_kind() == ::googlesql::TYPE_NUMERIC) {
      nums.push_back(v.numeric_value().ToDouble());
    } else {
      return MakeSemanticError(SemanticErrorReason::kNotImplemented,
                               "semantic: PERCENTILE_CONT unsupported type");
    }
  }
  if (nums.empty()) return Value::NullDouble();
  std::sort(nums.begin(), nums.end());
  return Value::Double(PercentileContFromSorted(nums, p));
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
    std::stable_sort(
        order.begin(), order.end(), [&](size_t a_idx, size_t b_idx) -> bool {
          if (partition_fps[a_idx] != partition_fps[b_idx]) {
            return partition_fps[a_idx] < partition_fps[b_idx];
          }
          if (order_spec == nullptr) return false;
          for (int i = 0; i < order_spec->order_by_item_list_size(); ++i) {
            const ::googlesql::ResolvedOrderByItem* item =
                order_spec->order_by_item_list(i);
            if (item == nullptr || item->column_ref() == nullptr) {
              continue;
            }
            const int col_id = item->column_ref()->column().column_id();
            const int cmp = CompareOrderByItem(
                item,
                LookupColumnValue(input_rows[a_idx], col_id),
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
      const int out_col_id = cc->column().column_id();
      if (fname == "row_number") {
        for (size_t r = 0; r < out_rows.size(); ++r) {
          out_rows[r][out_col_id] = Value::Int64(row_numbers[r]);
        }
        continue;
      }
      if (fname == "count") {
        const ::googlesql::ResolvedWindowFrame* wf = afn->window_frame();
        if (wf != nullptr &&
            wf->frame_unit() == ::googlesql::ResolvedWindowFrame::RANGE &&
            order_spec != nullptr &&
            order_spec->order_by_item_list_size() > 0) {
          const ::googlesql::ResolvedOrderByItem* order_item =
              order_spec->order_by_item_list(0);
          if (order_item == nullptr || order_item->column_ref() == nullptr) {
            return MakeSemanticError(SemanticErrorReason::kNotImplemented,
                                     "semantic: analytic COUNT RANGE missing "
                                     "order key");
          }
          const int order_col_id =
              order_item->column_ref()->column().column_id();
          const ::googlesql::Type* order_type =
              order_item->column_ref()->type();
          if (order_type == nullptr ||
              (order_type->kind() != ::googlesql::TYPE_DATE &&
               order_type->kind() != ::googlesql::TYPE_TIMESTAMP)) {
            return MakeSemanticError(
                SemanticErrorReason::kNotImplemented,
                "semantic: analytic COUNT RANGE requires DATE/TIMESTAMP order");
          }
          for (size_t r = 0; r < out_rows.size(); ++r) {
            const Value current_order =
                LookupColumnValue(input_rows[r], order_col_id);
            EvalContext row_ctx = ctx;
            row_ctx.columns = &input_rows[r];
            auto low_or =
                FrameBoundValue(wf->start_expr(), current_order, row_ctx);
            if (!low_or.ok()) return low_or.status();
            auto high_or =
                FrameBoundValue(wf->end_expr(), current_order, row_ctx);
            if (!high_or.ok()) return high_or.status();
            const bool has_low = !(*low_or).is_null();
            const bool has_high = !(*high_or).is_null();
            int64_t count = 0;
            for (size_t other = 0; other < input_rows.size(); ++other) {
              if (partition_fps[other] != partition_fps[r]) continue;
              const Value other_order =
                  LookupColumnValue(input_rows[other], order_col_id);
              if (!ValueInClosedRange(
                      other_order, *low_or, has_low, *high_or, has_high)) {
                continue;
              }
              if (afn->argument_list_size() == 0) {
                count++;
                continue;
              }
              if (afn->argument_list(0) == nullptr) {
                count++;
                continue;
              }
              EvalContext other_ctx = ctx;
              other_ctx.columns = &input_rows[other];
              auto arg = EvalExpr(*afn->argument_list(0), other_ctx);
              if (!arg.ok()) return arg.status();
              if (!arg->is_null()) count++;
            }
            out_rows[r][out_col_id] = Value::Int64(count);
          }
          continue;
        }
      }
      if (fname == "sum") {
        if (afn->argument_list_size() != 1 ||
            afn->argument_list(0) == nullptr) {
          return absl::InvalidArgumentError(
              "semantic: analytic SUM expects one argument");
        }
        absl::flat_hash_map<std::string, Value> partition_sums;
        for (size_t r = 0; r < input_rows.size(); ++r) {
          EvalContext row_ctx = ctx;
          row_ctx.columns = &input_rows[r];
          auto piece = EvalExpr(*afn->argument_list(0), row_ctx);
          if (!piece.ok()) return piece.status();
          auto it = partition_sums.find(partition_fps[r]);
          if (it == partition_sums.end()) {
            partition_sums.emplace(partition_fps[r], *piece);
          } else {
            auto summed = AddValues(it->second, *piece);
            if (!summed.ok()) return summed.status();
            it->second = *std::move(summed);
          }
        }
        for (size_t r = 0; r < out_rows.size(); ++r) {
          auto it = partition_sums.find(partition_fps[r]);
          if (it == partition_sums.end()) {
            out_rows[r][out_col_id] = Value::NullInt64();
          } else {
            out_rows[r][out_col_id] = it->second;
          }
        }
        continue;
      }
      if (fname == "percentile_cont") {
        if (afn->argument_list_size() != 2 ||
            afn->argument_list(0) == nullptr ||
            afn->argument_list(1) == nullptr) {
          return absl::InvalidArgumentError(
              "semantic: analytic PERCENTILE_CONT expects two arguments");
        }
        absl::flat_hash_map<std::string, std::vector<Value>> partition_values;
        absl::flat_hash_map<std::string, Value> partition_percentile;
        for (size_t r = 0; r < input_rows.size(); ++r) {
          EvalContext row_ctx = ctx;
          row_ctx.columns = &input_rows[r];
          auto piece = EvalExpr(*afn->argument_list(0), row_ctx);
          if (!piece.ok()) return piece.status();
          auto pct = EvalExpr(*afn->argument_list(1), row_ctx);
          if (!pct.ok()) return pct.status();
          partition_values[partition_fps[r]].push_back(*piece);
          partition_percentile.emplace(partition_fps[r], *pct);
        }
        absl::flat_hash_map<std::string, Value> partition_result;
        for (const auto& [fp, vals] : partition_values) {
          auto it = partition_percentile.find(fp);
          if (it == partition_percentile.end()) continue;
          auto out = PercentileContValue(it->second, vals);
          if (!out.ok()) return out.status();
          partition_result.emplace(fp, *out);
        }
        for (size_t r = 0; r < out_rows.size(); ++r) {
          auto it = partition_result.find(partition_fps[r]);
          if (it == partition_result.end()) {
            out_rows[r][out_col_id] = Value::NullDouble();
          } else {
            out_rows[r][out_col_id] = it->second;
          }
        }
        continue;
      }
      return MakeSemanticError(SemanticErrorReason::kNotImplemented,
                               absl::StrCat("semantic: analytic function '",
                                            fname,
                                            "' is not yet implemented"));
    }
  }

  return out_rows;
}

}  // namespace scan_eval_internal
}  // namespace semantic
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
