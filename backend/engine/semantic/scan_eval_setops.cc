#include <algorithm>
#include <map>
#include <string>
#include <utility>
#include <vector>

#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "absl/strings/str_cat.h"
#include "backend/engine/semantic/error.h"
#include "backend/engine/semantic/scan_eval_internal.h"
#include "backend/engine/semantic/value.h"
#include "googlesql/public/type.h"
#include "googlesql/resolved_ast/resolved_ast.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace semantic {
namespace scan_eval_internal {

namespace {

std::string SetOperationRowFingerprint(
    const ColumnBindings& row,
    const ::googlesql::ResolvedSetOperationScan& set_op) {
  std::string fp;
  for (int j = 0; j < set_op.column_list_size(); ++j) {
    const int col_id = set_op.column_list(j).column_id();
    auto it = row.find(col_id);
    if (it != row.end()) {
      absl::StrAppend(&fp, it->second.DebugString(), "\x1e");
    } else {
      absl::StrAppend(&fp, "<missing>\x1e");
    }
  }
  return fp;
}

ColumnBindings RemapSetOperationRow(
    const ColumnBindings& part_row,
    const ::googlesql::ResolvedScan* part_scan,
    const ::googlesql::ResolvedSetOperationScan& set_op) {
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
  return remapped;
}

absl::StatusOr<std::vector<ColumnBindings>> MaterializeSetOperationArm(
    const ::googlesql::ResolvedSetOperationItem* item,
    const ::googlesql::ResolvedSetOperationScan& set_op,
    EvalContext& ctx) {
  if (item == nullptr || item->scan() == nullptr) {
    return absl::InternalError("semantic: SetOperationItem has null scan");
  }
  auto part = MaterializeScanImpl(item->scan(), ctx);
  if (!part.ok()) return part.status();
  const ::googlesql::ResolvedScan* part_scan = item->scan();
  std::vector<ColumnBindings> out;
  out.reserve(part->size());
  for (const ColumnBindings& part_row : *part) {
    out.push_back(RemapSetOperationRow(part_row, part_scan, set_op));
  }
  return out;
}

std::vector<ColumnBindings> DedupeSetOperationRowsPreserveOrder(
    const std::vector<ColumnBindings>& rows,
    const ::googlesql::ResolvedSetOperationScan& set_op) {
  std::vector<ColumnBindings> out;
  std::map<std::string, int> seen;
  for (const ColumnBindings& row : rows) {
    const std::string fp = SetOperationRowFingerprint(row, set_op);
    if (seen.emplace(fp, 1).second) {
      out.push_back(row);
    }
  }
  return out;
}

std::map<std::string, int> CountSetOperationRowsByFingerprint(
    const std::vector<ColumnBindings>& rows,
    const ::googlesql::ResolvedSetOperationScan& set_op) {
  std::map<std::string, int> counts;
  for (const ColumnBindings& row : rows) {
    ++counts[SetOperationRowFingerprint(row, set_op)];
  }
  return counts;
}

std::map<std::string, ColumnBindings> FirstSetOperationRowByFingerprint(
    const std::vector<ColumnBindings>& rows,
    const ::googlesql::ResolvedSetOperationScan& set_op) {
  std::map<std::string, ColumnBindings> first;
  for (const ColumnBindings& row : rows) {
    const std::string fp = SetOperationRowFingerprint(row, set_op);
    first.emplace(fp, row);
  }
  return first;
}

std::vector<ColumnBindings> ExpandSetOperationCounts(
    const std::map<std::string, int>& counts,
    const std::map<std::string, ColumnBindings>& representatives) {
  std::vector<ColumnBindings> out;
  for (const auto& [fp, count] : counts) {
    if (count <= 0) continue;
    auto it = representatives.find(fp);
    if (it == representatives.end()) continue;
    for (int i = 0; i < count; ++i) {
      out.push_back(it->second);
    }
  }
  return out;
}

std::vector<ColumnBindings> ApplyExceptAll(
    const std::vector<ColumnBindings>& left,
    const std::vector<ColumnBindings>& right,
    const ::googlesql::ResolvedSetOperationScan& set_op) {
  std::map<std::string, int> left_counts =
      CountSetOperationRowsByFingerprint(left, set_op);
  const std::map<std::string, int> right_counts =
      CountSetOperationRowsByFingerprint(right, set_op);
  for (const auto& [fp, count] : right_counts) {
    auto it = left_counts.find(fp);
    if (it == left_counts.end()) continue;
    it->second = std::max(0, it->second - count);
  }
  const std::map<std::string, ColumnBindings> reps =
      FirstSetOperationRowByFingerprint(left, set_op);
  return ExpandSetOperationCounts(left_counts, reps);
}

std::vector<ColumnBindings> ApplyIntersectAllArms(
    const std::vector<std::vector<ColumnBindings>>& arms,
    const ::googlesql::ResolvedSetOperationScan& set_op) {
  if (arms.empty()) return {};
  std::map<std::string, int> counts =
      CountSetOperationRowsByFingerprint(arms[0], set_op);
  for (size_t i = 1; i < arms.size(); ++i) {
    const std::map<std::string, int> next =
        CountSetOperationRowsByFingerprint(arms[i], set_op);
    for (auto it = counts.begin(); it != counts.end();) {
      auto nit = next.find(it->first);
      const int intersect =
          (nit == next.end()) ? 0 : std::min(it->second, nit->second);
      if (intersect <= 0) {
        it = counts.erase(it);
      } else {
        it->second = intersect;
        ++it;
      }
    }
  }
  const std::map<std::string, ColumnBindings> reps =
      FirstSetOperationRowByFingerprint(arms[0], set_op);
  return ExpandSetOperationCounts(counts, reps);
}

std::vector<ColumnBindings> ApplyIntersectDistinctArms(
    const std::vector<std::vector<ColumnBindings>>& arms,
    const ::googlesql::ResolvedSetOperationScan& set_op) {
  if (arms.empty()) return {};
  std::map<std::string, ColumnBindings> candidates =
      FirstSetOperationRowByFingerprint(arms[0], set_op);
  for (size_t i = 1; i < arms.size(); ++i) {
    const std::map<std::string, ColumnBindings> next =
        FirstSetOperationRowByFingerprint(arms[i], set_op);
    for (auto it = candidates.begin(); it != candidates.end();) {
      if (next.find(it->first) == next.end()) {
        it = candidates.erase(it);
      } else {
        ++it;
      }
    }
  }
  std::vector<ColumnBindings> out;
  out.reserve(candidates.size());
  for (const auto& [fp, row] : candidates) {
    (void)fp;
    out.push_back(row);
  }
  return out;
}

std::vector<ColumnBindings> ApplyExceptDistinctArms(
    const std::vector<std::vector<ColumnBindings>>& arms,
    const ::googlesql::ResolvedSetOperationScan& set_op) {
  if (arms.empty()) return {};
  std::map<std::string, ColumnBindings> left =
      FirstSetOperationRowByFingerprint(arms[0], set_op);
  for (size_t i = 1; i < arms.size(); ++i) {
    const std::map<std::string, ColumnBindings> right =
        FirstSetOperationRowByFingerprint(arms[i], set_op);
    for (const auto& [fp, row] : right) {
      (void)row;
      left.erase(fp);
    }
  }
  std::vector<ColumnBindings> out;
  out.reserve(left.size());
  for (const auto& [fp, row] : left) {
    (void)fp;
    out.push_back(row);
  }
  return out;
}

}  // namespace

absl::StatusOr<std::vector<ColumnBindings>> MaterializeSetOperationScan(
    const ::googlesql::ResolvedSetOperationScan& set_op, EvalContext& ctx) {
  std::vector<std::vector<ColumnBindings>> arms;
  arms.reserve(static_cast<size_t>(set_op.input_item_list_size()));
  for (int i = 0; i < set_op.input_item_list_size(); ++i) {
    auto arm =
        MaterializeSetOperationArm(set_op.input_item_list(i), set_op, ctx);
    if (!arm.ok()) return arm.status();
    arms.push_back(*std::move(arm));
  }

  switch (set_op.op_type()) {
    case ::googlesql::ResolvedSetOperationScan::UNION_ALL: {
      std::vector<ColumnBindings> out;
      for (const std::vector<ColumnBindings>& arm : arms) {
        out.insert(out.end(), arm.begin(), arm.end());
      }
      return out;
    }
    case ::googlesql::ResolvedSetOperationScan::UNION_DISTINCT: {
      std::vector<ColumnBindings> out;
      for (const std::vector<ColumnBindings>& arm : arms) {
        out.insert(out.end(), arm.begin(), arm.end());
      }
      return DedupeSetOperationRowsPreserveOrder(out, set_op);
    }
    case ::googlesql::ResolvedSetOperationScan::INTERSECT_ALL:
      return ApplyIntersectAllArms(arms, set_op);
    case ::googlesql::ResolvedSetOperationScan::INTERSECT_DISTINCT:
      return ApplyIntersectDistinctArms(arms, set_op);
    case ::googlesql::ResolvedSetOperationScan::EXCEPT_ALL: {
      if (arms.empty()) return std::vector<ColumnBindings>{};
      std::vector<ColumnBindings> out = arms[0];
      for (size_t i = 1; i < arms.size(); ++i) {
        out = ApplyExceptAll(out, arms[i], set_op);
      }
      return out;
    }
    case ::googlesql::ResolvedSetOperationScan::EXCEPT_DISTINCT:
      return ApplyExceptDistinctArms(arms, set_op);
    default:
      return MakeSemanticError(
          SemanticErrorReason::kNotImplemented,
          "semantic: SetOperationScan op is not implemented");
  }
}

}  // namespace scan_eval_internal
}  // namespace semantic
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
