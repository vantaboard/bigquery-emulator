#include <algorithm>
#include <cstdint>
#include <numeric>
#include <string>
#include <vector>

#include "absl/container/flat_hash_map.h"
#include "absl/status/statusor.h"
#include "absl/strings/str_cat.h"
#include "backend/engine/semantic/error.h"
#include "backend/engine/semantic/eval_expr.h"
#include "backend/engine/semantic/scan_eval_internal.h"
#include "backend/engine/semantic/value.h"
#include "googlesql/public/type.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace semantic {
namespace scan_eval_internal {

absl::Status ApplyAnalyticNtile(
    const ::googlesql::ResolvedAnalyticFunctionCall& afn,
    const AnalyticGroupLayout& layout,
    const std::vector<ColumnBindings>& input_rows,
    int out_col_id,
    const EvalContext& ctx,
    std::vector<ColumnBindings>& out_rows) {
  if (afn.argument_list_size() != 1 || afn.argument_list(0) == nullptr) {
    return absl::InvalidArgumentError(
        "semantic: analytic NTILE expects one argument");
  }
  EvalContext arg_ctx = ctx;
  if (!input_rows.empty()) {
    arg_ctx.columns = &input_rows[0];
  }
  auto buckets_or = EvalExpr(*afn.argument_list(0), arg_ctx);
  if (!buckets_or.ok()) return buckets_or.status();
  if (buckets_or->is_null() ||
      buckets_or->type_kind() != ::googlesql::TYPE_INT64) {
    return MakeSemanticError(
        SemanticErrorReason::kInvalidArgument,
        "semantic: NTILE buckets argument must be a positive INT64");
  }
  const int64_t n_buckets = buckets_or->int64_value();
  if (n_buckets <= 0) {
    return MakeSemanticError(
        SemanticErrorReason::kInvalidArgument,
        "semantic: NTILE buckets argument must be a positive INT64");
  }

  absl::flat_hash_map<std::string, int64_t> partition_sizes;
  for (const std::string& fp : layout.partition_fps) {
    partition_sizes[fp]++;
  }

  for (size_t r = 0; r < out_rows.size(); ++r) {
    const int64_t S = partition_sizes[layout.partition_fps[r]];
    const int64_t rn = layout.row_numbers[r];
    if (S <= 0 || rn <= 0) {
      out_rows[r][out_col_id] = Value::NullInt64();
      continue;
    }
    // First (S % n) buckets get floor(S/n)+1 rows; the rest get
    // floor(S/n). When n > S, each row gets its own bucket 1..S.
    const int64_t q = S / n_buckets;
    const int64_t rem = S % n_buckets;
    int64_t bucket = 0;
    if (q == 0) {
      bucket = rn;
    } else if (rn <= rem * (q + 1)) {
      bucket = (rn - 1) / (q + 1) + 1;
    } else {
      bucket = rem + (rn - rem * (q + 1) - 1) / q + 1;
    }
    out_rows[r][out_col_id] = Value::Int64(bucket);
  }
  return absl::OkStatus();
}

void ApplyAnalyticRank(const ::googlesql::ResolvedWindowOrdering* order_spec,
                       const AnalyticGroupLayout& layout,
                       const std::vector<ColumnBindings>& input_rows,
                       int out_col_id,
                       bool dense,
                       std::vector<ColumnBindings>& out_rows) {
  std::vector<size_t> order(input_rows.size());
  std::iota(order.begin(), order.end(), 0);
  std::stable_sort(order.begin(), order.end(), [&](size_t a, size_t b) {
    if (layout.partition_fps[a] != layout.partition_fps[b]) {
      return layout.partition_fps[a] < layout.partition_fps[b];
    }
    return layout.row_numbers[a] < layout.row_numbers[b];
  });

  std::string prev_fp;
  size_t prev_idx = 0;
  bool have_prev = false;
  int64_t olympic_rank = 0;
  int64_t dense_rank = 0;
  for (size_t sorted_pos = 0; sorted_pos < order.size(); ++sorted_pos) {
    const size_t idx = order[sorted_pos];
    const std::string& fp = layout.partition_fps[idx];
    const bool new_partition = !have_prev || fp != prev_fp;
    const bool peer_break =
        new_partition ||
        !OrderKeysEqual(order_spec, input_rows[idx], input_rows[prev_idx]);
    if (peer_break) {
      olympic_rank = layout.row_numbers[idx];
      if (new_partition) {
        dense_rank = 1;
      } else {
        dense_rank++;
      }
    }
    out_rows[idx][out_col_id] = Value::Int64(dense ? dense_rank : olympic_rank);
    prev_fp = fp;
    prev_idx = idx;
    have_prev = true;
  }
}

absl::Status ApplyAnalyticLagLead(
    const ::googlesql::ResolvedAnalyticFunctionCall& afn,
    const AnalyticGroupLayout& layout,
    const std::vector<ColumnBindings>& input_rows,
    int out_col_id,
    const EvalContext& ctx,
    int direction,
    std::vector<ColumnBindings>& out_rows) {
  if (afn.argument_list_size() < 1 || afn.argument_list(0) == nullptr) {
    return absl::InvalidArgumentError(
        "semantic: analytic LAG/LEAD expects a value argument");
  }
  int64_t offset = 1;
  if (afn.argument_list_size() >= 2 && afn.argument_list(1) != nullptr) {
    EvalContext arg_ctx = ctx;
    if (!input_rows.empty()) {
      arg_ctx.columns = &input_rows[0];
    }
    auto offset_or = EvalExpr(*afn.argument_list(1), arg_ctx);
    if (!offset_or.ok()) return offset_or.status();
    if (offset_or->is_null() ||
        offset_or->type_kind() != ::googlesql::TYPE_INT64 ||
        offset_or->int64_value() <= 0) {
      return MakeSemanticError(
          SemanticErrorReason::kInvalidArgument,
          "semantic: LAG/LEAD offset must be a positive INT64");
    }
    offset = offset_or->int64_value();
  }

  Value default_value;
  bool has_default = false;
  if (afn.argument_list_size() >= 3 && afn.argument_list(2) != nullptr) {
    EvalContext arg_ctx = ctx;
    if (!input_rows.empty()) {
      arg_ctx.columns = &input_rows[0];
    }
    auto def_or = EvalExpr(*afn.argument_list(2), arg_ctx);
    if (!def_or.ok()) return def_or.status();
    default_value = *std::move(def_or);
    has_default = true;
  }

  absl::flat_hash_map<std::string, size_t> by_partition_rn;
  by_partition_rn.reserve(input_rows.size());
  for (size_t r = 0; r < input_rows.size(); ++r) {
    by_partition_rn[absl::StrCat(
        layout.partition_fps[r], "\x1e", layout.row_numbers[r])] = r;
  }

  for (size_t r = 0; r < out_rows.size(); ++r) {
    const int64_t target_rn =
        layout.row_numbers[r] + static_cast<int64_t>(direction) * offset;
    auto it = by_partition_rn.find(
        absl::StrCat(layout.partition_fps[r], "\x1e", target_rn));
    if (it == by_partition_rn.end()) {
      if (has_default) {
        out_rows[r][out_col_id] = default_value;
      } else if (afn.argument_list(0)->type() != nullptr) {
        out_rows[r][out_col_id] = Value::Null(afn.argument_list(0)->type());
      } else {
        out_rows[r][out_col_id] = Value::NullInt64();
      }
      continue;
    }
    EvalContext target_ctx = ctx;
    target_ctx.columns = &input_rows[it->second];
    auto value_or = EvalExpr(*afn.argument_list(0), target_ctx);
    if (!value_or.ok()) return value_or.status();
    out_rows[r][out_col_id] = *std::move(value_or);
  }
  return absl::OkStatus();
}

}  // namespace scan_eval_internal
}  // namespace semantic
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
