#include <cstddef>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "absl/container/flat_hash_map.h"
#include "absl/container/flat_hash_set.h"
#include "absl/status/statusor.h"
#include "absl/strings/str_cat.h"
#include "backend/engine/semantic/array_struct/array_scan.h"
#include "backend/engine/semantic/eval_expr_internal.h"
#include "backend/engine/semantic/outer_row_eval.h"
#include "backend/engine/semantic/scan_eval_internal.h"
#include "backend/engine/semantic/scan_eval_materialize_internal.h"
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
using ::bigquery_emulator::backend::engine::semantic::eval_expr_internal::
    LowerFunctionDispatchName;

namespace materialize_internal {

absl::StatusOr<ColumnBindings> ProjectOneInputRow(
    const ::googlesql::ResolvedProjectScan& project,
    const ColumnBindings& input,
    const EvalContext& ctx,
    const absl::flat_hash_map<int, const ::googlesql::ResolvedExpr*>&
        expr_by_column_id) {
  ColumnBindings merged;
  if (ctx.columns != nullptr) {
    merged = *ctx.columns;
  }
  for (const auto& [col_id, val] : input) {
    merged[col_id] = val;
  }
  ColumnBindings row = merged;
  row.reserve(row.size() + project.column_list_size());
  absl::flat_hash_map<std::string, Value> by_name;
  PopulateColumnNameBindings(project.input_scan(), merged, by_name);
  if (ctx.columns_by_name != nullptr) {
    for (const auto& [name, val] : *ctx.columns_by_name) {
      by_name[name] = val;
    }
  }
  for (int i = 0; i < project.column_list_size(); ++i) {
    const ::googlesql::ResolvedColumn& col = project.column_list(i);
    const int col_id = col.column_id();
    auto eit = expr_by_column_id.find(col_id);
    EvalContext row_ctx = ctx;
    row_ctx.columns = &merged;
    row_ctx.columns_by_name = &by_name;
    Value v;
    if (eit != expr_by_column_id.end()) {
      auto eval_v = EvalExpr(*eit->second, row_ctx);
      if (!eval_v.ok()) return eval_v.status();
      v = *std::move(eval_v);
    } else {
      auto cit = merged.find(col_id);
      if (cit == merged.end()) {
        return absl::InternalError(
            absl::StrCat("semantic: ProjectScan missing binding for column '",
                         col.name(),
                         "'"));
      }
      v = cit->second;
    }
    row.emplace(col_id, std::move(v));
  }
  return row;
}

void AppendNullRightColumns(const ::googlesql::ResolvedScan* rscan,
                            ColumnBindings* merged) {
  if (rscan == nullptr) return;
  for (int i = 0; i < rscan->column_list_size(); ++i) {
    const ::googlesql::ResolvedColumn& col = rscan->column_list(i);
    merged->emplace(col.column_id(), Value::Null(col.type()));
  }
}

absl::StatusOr<bool> ShouldIncludeJoinRow(
    const ::googlesql::ResolvedJoinScan& join,
    bool is_cross,
    const ColumnBindings& merged,
    const EvalContext& ctx) {
  if (is_cross || join.join_expr() == nullptr) return true;
  EvalContext merged_ctx = ctx;
  merged_ctx.columns = &merged;
  return EvalBoolExpr(join.join_expr(), merged_ctx);
}

namespace {

// Key types where `GroupKeyFingerprint` equality coincides with SQL
// equality for non-NULL values. DOUBLE/FLOAT are excluded (NaN and
// signed-zero semantics), as are INTERVAL (distinct representations
// compare equal) and composite/exotic types (nested-NULL equality).
bool HashableJoinKeyType(const ::googlesql::Type* type) {
  if (type == nullptr) return false;
  switch (type->kind()) {
    case ::googlesql::TYPE_INT32:
    case ::googlesql::TYPE_INT64:
    case ::googlesql::TYPE_UINT32:
    case ::googlesql::TYPE_UINT64:
    case ::googlesql::TYPE_BOOL:
    case ::googlesql::TYPE_STRING:
    case ::googlesql::TYPE_BYTES:
    case ::googlesql::TYPE_DATE:
    case ::googlesql::TYPE_TIMESTAMP:
    case ::googlesql::TYPE_TIME:
    case ::googlesql::TYPE_DATETIME:
    case ::googlesql::TYPE_NUMERIC:
    case ::googlesql::TYPE_BIGNUMERIC:
      return true;
    default:
      return false;
  }
}

void FlattenAndConjuncts(const ::googlesql::ResolvedExpr* expr,
                         std::vector<const ::googlesql::ResolvedExpr*>& out) {
  if (expr == nullptr) return;
  if (expr->node_kind() == ::googlesql::RESOLVED_FUNCTION_CALL) {
    const auto* call = expr->GetAs<::googlesql::ResolvedFunctionCall>();
    if (call->function() != nullptr &&
        LowerFunctionDispatchName(call->function()) == "$and") {
      for (int i = 0; i < call->argument_list_size(); ++i) {
        FlattenAndConjuncts(call->argument_list(i), out);
      }
      return;
    }
  }
  out.push_back(expr);
}

// Returns the column id when `expr` is an uncorrelated bare column
// reference of a hash-safe type; -1 otherwise.
int HashableColumnRefId(const ::googlesql::ResolvedExpr* expr) {
  if (expr == nullptr ||
      expr->node_kind() != ::googlesql::RESOLVED_COLUMN_REF) {
    return -1;
  }
  const auto* ref = expr->GetAs<::googlesql::ResolvedColumnRef>();
  if (ref->is_correlated() || !HashableJoinKeyType(ref->type())) return -1;
  return ref->column().column_id();
}

absl::StatusOr<std::string> JoinKeyFingerprint(const ColumnBindings& row,
                                               const std::vector<int>& key_ids,
                                               bool* has_null_key) {
  std::vector<Value> keys;
  keys.reserve(key_ids.size());
  *has_null_key = false;
  for (int id : key_ids) {
    auto it = row.find(id);
    if (it == row.end()) {
      return absl::InternalError(
          absl::StrCat("semantic: join key column_id=",
                       id,
                       " missing from materialized row"));
    }
    if (it->second.is_null()) {
      *has_null_key = true;
      return std::string();
    }
    keys.push_back(it->second);
  }
  return GroupKeyFingerprint(keys);
}

}  // namespace

std::optional<EquiJoinPlan> PlanEquiJoin(
    const ::googlesql::ResolvedJoinScan& join) {
  if (join.is_lateral() || join.join_expr() == nullptr) return std::nullopt;
  if (join.join_type() != ::googlesql::ResolvedJoinScan::INNER &&
      join.join_type() != ::googlesql::ResolvedJoinScan::LEFT) {
    return std::nullopt;
  }
  if (join.left_scan() == nullptr || join.right_scan() == nullptr) {
    return std::nullopt;
  }
  absl::flat_hash_set<int> left_ids;
  for (int i = 0; i < join.left_scan()->column_list_size(); ++i) {
    left_ids.insert(join.left_scan()->column_list(i).column_id());
  }
  absl::flat_hash_set<int> right_ids;
  for (int i = 0; i < join.right_scan()->column_list_size(); ++i) {
    right_ids.insert(join.right_scan()->column_list(i).column_id());
  }

  std::vector<const ::googlesql::ResolvedExpr*> conjuncts;
  FlattenAndConjuncts(join.join_expr(), conjuncts);

  EquiJoinPlan plan;
  for (const ::googlesql::ResolvedExpr* conjunct : conjuncts) {
    int lhs = -1;
    int rhs = -1;
    if (conjunct != nullptr &&
        conjunct->node_kind() == ::googlesql::RESOLVED_FUNCTION_CALL) {
      const auto* call = conjunct->GetAs<::googlesql::ResolvedFunctionCall>();
      if (call->function() != nullptr && call->argument_list_size() == 2 &&
          LowerFunctionDispatchName(call->function()) == "$equal") {
        lhs = HashableColumnRefId(call->argument_list(0));
        rhs = HashableColumnRefId(call->argument_list(1));
      }
    }
    if (lhs >= 0 && rhs >= 0 && left_ids.contains(lhs) &&
        right_ids.contains(rhs)) {
      plan.left_key_ids.push_back(lhs);
      plan.right_key_ids.push_back(rhs);
    } else if (lhs >= 0 && rhs >= 0 && left_ids.contains(rhs) &&
               right_ids.contains(lhs)) {
      plan.left_key_ids.push_back(rhs);
      plan.right_key_ids.push_back(lhs);
    } else {
      plan.residual.push_back(conjunct);
    }
  }
  if (plan.left_key_ids.empty()) return std::nullopt;
  return plan;
}

absl::StatusOr<std::vector<ColumnBindings>> MaterializeHashEquiJoinRows(
    const ::googlesql::ResolvedJoinScan& join,
    const EquiJoinPlan& plan,
    const std::vector<ColumnBindings>& left_rows,
    const std::vector<ColumnBindings>& right_rows,
    const EvalContext& ctx) {
  const bool is_left_outer =
      join.join_type() == ::googlesql::ResolvedJoinScan::LEFT;
  const ::googlesql::ResolvedScan* rscan = StripBarrierScans(join.right_scan());

  absl::flat_hash_map<std::string, std::vector<size_t>> right_index;
  right_index.reserve(right_rows.size());
  for (size_t r = 0; r < right_rows.size(); ++r) {
    bool has_null_key = false;
    auto fp =
        JoinKeyFingerprint(right_rows[r], plan.right_key_ids, &has_null_key);
    if (!fp.ok()) return fp.status();
    if (has_null_key) continue;  // NULL keys never match.
    right_index[*fp].push_back(r);
  }

  const std::vector<size_t> no_matches;
  std::vector<ColumnBindings> out;
  for (const ColumnBindings& lrow : left_rows) {
    bool has_null_key = false;
    auto fp = JoinKeyFingerprint(lrow, plan.left_key_ids, &has_null_key);
    if (!fp.ok()) return fp.status();
    const std::vector<size_t>* matches = &no_matches;
    if (!has_null_key) {
      auto it = right_index.find(*fp);
      if (it != right_index.end()) matches = &it->second;
    }
    bool any_match = false;
    for (size_t r : *matches) {
      ColumnBindings merged = lrow;
      merged.insert(right_rows[r].begin(), right_rows[r].end());
      bool include = true;
      for (const ::googlesql::ResolvedExpr* residual : plan.residual) {
        EvalContext merged_ctx = ctx;
        merged_ctx.columns = &merged;
        auto ok = EvalBoolExpr(residual, merged_ctx);
        if (!ok.ok()) return ok.status();
        if (!*ok) {
          include = false;
          break;
        }
      }
      if (include) {
        any_match = true;
        out.push_back(std::move(merged));
      }
    }
    if (!any_match && is_left_outer) {
      ColumnBindings merged = lrow;
      AppendNullRightColumns(rscan, &merged);
      out.push_back(std::move(merged));
    }
  }
  return out;
}

absl::StatusOr<std::vector<ColumnBindings>> MaterializeNestedLoopJoinRows(
    const ::googlesql::ResolvedJoinScan& join,
    const std::vector<ColumnBindings>& left_rows,
    const std::vector<ColumnBindings>& right_rows,
    const EvalContext& ctx) {
  const bool is_left_outer =
      join.join_type() == ::googlesql::ResolvedJoinScan::LEFT;
  const bool is_cross =
      join.join_expr() == nullptr &&
      join.join_type() == ::googlesql::ResolvedJoinScan::INNER;
  const ::googlesql::ResolvedScan* rscan = StripBarrierScans(join.right_scan());

  std::vector<ColumnBindings> out;
  for (const ColumnBindings& lrow : left_rows) {
    bool any_match = false;
    for (const ColumnBindings& rrow : right_rows) {
      ColumnBindings merged = lrow;
      merged.insert(rrow.begin(), rrow.end());
      auto include_or = ShouldIncludeJoinRow(join, is_cross, merged, ctx);
      if (!include_or.ok()) return include_or.status();
      if (*include_or) {
        any_match = true;
        out.push_back(std::move(merged));
      }
    }
    if (!any_match && is_left_outer) {
      ColumnBindings merged = lrow;
      AppendNullRightColumns(rscan, &merged);
      out.push_back(std::move(merged));
    }
  }
  return out;
}

absl::StatusOr<std::vector<ColumnBindings>> MaterializeLateralJoinRows(
    const ::googlesql::ResolvedJoinScan& join,
    const std::vector<ColumnBindings>& left_rows,
    EvalContext& ctx) {
  const bool is_left_outer =
      join.join_type() == ::googlesql::ResolvedJoinScan::LEFT;
  const bool is_cross =
      join.join_expr() == nullptr &&
      join.join_type() == ::googlesql::ResolvedJoinScan::INNER;
  const ::googlesql::ResolvedScan* rscan = StripBarrierScans(join.right_scan());

  std::vector<ColumnBindings> out;
  for (const ColumnBindings& lrow : left_rows) {
    OuterRowFrame frame = MakeOuterRowFrame(ctx, lrow, join.left_scan());
    BindCorrelatedColumnRefs(join.right_scan(), frame);
    auto right_or = MaterializeScanImpl(join.right_scan(), frame.row_ctx);
    if (!right_or.ok()) return right_or.status();

    bool any_match = false;
    for (const ColumnBindings& rrow : *right_or) {
      ColumnBindings merged = lrow;
      merged.insert(rrow.begin(), rrow.end());
      auto include_or = ShouldIncludeJoinRow(join, is_cross, merged, ctx);
      if (!include_or.ok()) return include_or.status();
      if (*include_or) {
        any_match = true;
        out.push_back(std::move(merged));
      }
    }
    if (!any_match && is_left_outer) {
      ColumnBindings merged = lrow;
      AppendNullRightColumns(rscan, &merged);
      out.push_back(std::move(merged));
    }
  }
  return out;
}

absl::StatusOr<std::vector<ColumnBindings>> MaterializeArrayScanWithJoinExpr(
    const ::googlesql::ResolvedArrayScan& scan, EvalContext& ctx) {
  auto left_or = MaterializeScanImpl(scan.input_scan(), ctx);
  if (!left_or.ok()) return left_or.status();
  std::vector<ColumnBindings> out;
  for (const ColumnBindings& lrow : *left_or) {
    OuterRowFrame frame = MakeOuterRowFrame(ctx, lrow, scan.input_scan());
    auto array_rows = array_struct::EvaluateArrayScan(scan, frame.row_ctx);
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

absl::StatusOr<std::vector<ColumnBindings>> MaterializeArrayScanFromLeftInput(
    const ::googlesql::ResolvedArrayScan& scan, EvalContext& ctx) {
  auto left_or = MaterializeScanImpl(scan.input_scan(), ctx);
  if (!left_or.ok()) return left_or.status();
  std::vector<ColumnBindings> out;
  for (const ColumnBindings& lrow : *left_or) {
    OuterRowFrame frame = MakeOuterRowFrame(ctx, lrow, scan.input_scan());
    auto array_rows = array_struct::EvaluateArrayScan(scan, frame.row_ctx);
    if (!array_rows.ok()) return array_rows.status();
    for (const ColumnBindings& arow : *array_rows) {
      ColumnBindings merged = lrow;
      merged.insert(arow.begin(), arow.end());
      out.push_back(std::move(merged));
    }
  }
  return out;
}

}  // namespace materialize_internal
}  // namespace scan_eval_internal
}  // namespace semantic
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
