#include "backend/engine/duckdb/transpiler/transpiler_emit_with_helpers.h"

#include <string>
#include <vector>

#include "absl/strings/str_cat.h"
#include "absl/strings/str_join.h"
#include "backend/engine/duckdb/transpiler/transpiler_internal.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace duckdb {
namespace transpiler {

namespace {

std::string RecursiveChildEmitColumnName(
    const ::googlesql::ResolvedScan* child,
    const ::googlesql::ResolvedColumn& out_col,
    int index) {
  if (child != nullptr) {
    for (int k = 0; k < child->column_list_size(); ++k) {
      if (child->column_list(k).column_id() == out_col.column_id()) {
        return std::string(child->column_list(k).name());
      }
    }
    if (index >= 0 && index < child->column_list_size()) {
      return std::string(child->column_list(index).name());
    }
  }
  return std::string(out_col.name());
}

std::string BuildRecursiveNormalizedSelect(
    const ::googlesql::ResolvedRecursiveScan* node,
    const std::vector<std::string>& anchor_names,
    int depth_col_idx,
    const ::googlesql::ResolvedSetOperationItem* item,
    bool is_recursive_arm,
    std::string inner) {
  (void)anchor_names;
  std::vector<std::string> normalized;
  normalized.reserve(node->column_list_size());
  const ::googlesql::ResolvedScan* child = item->scan();
  for (int j = 0; j < node->column_list_size(); ++j) {
    if (j == depth_col_idx && !is_recursive_arm) {
      continue;
    }
    const ::googlesql::ResolvedColumn& out_col = item->output_column_list(j);
    const ::googlesql::ResolvedColumn& cte_col = node->column_list(j);
    const std::string src_name =
        RecursiveChildEmitColumnName(child, out_col, j);
    std::string src_q = internal::QuoteIdent(src_name);
    if (src_name == cte_col.name()) {
      normalized.push_back(std::move(src_q));
    } else {
      normalized.push_back(
          absl::StrCat(src_q, " AS ", internal::QuoteIdent(cte_col.name())));
    }
  }
  return absl::StrCat(
      "SELECT ", absl::StrJoin(normalized, ", "), " FROM (", inner, ")");
}

std::string BuildRecursiveDepthExpr(
    const ::googlesql::ResolvedRecursionDepthModifier* depth_mod,
    const EmitExprFn& emit_expr,
    const ::googlesql::ResolvedColumn& cte_col,
    bool is_recursive_arm) {
  (void)cte_col;
  if (is_recursive_arm) {
    return absl::StrCat(internal::QuoteIdent(cte_col.name()), " + 1");
  }
  if (depth_mod != nullptr && depth_mod->lower_bound() != nullptr) {
    return emit_expr(depth_mod->lower_bound());
  }
  return "0";
}

std::string BuildRecursiveAnchorSelect(
    const ::googlesql::ResolvedRecursiveScan* node,
    const std::vector<std::string>& anchor_names,
    int depth_col_idx,
    const ::googlesql::ResolvedRecursionDepthModifier* depth_mod,
    const EmitExprFn& emit_expr,
    bool is_recursive_arm,
    absl::string_view normalized_sql) {
  std::vector<std::string> anchor_projs;
  anchor_projs.reserve(node->column_list_size());
  for (int j = 0; j < node->column_list_size(); ++j) {
    const ::googlesql::ResolvedColumn& cte_col = node->column_list(j);
    std::string dst_q = internal::QuoteIdent(anchor_names[j]);
    if (j == depth_col_idx) {
      std::string depth_expr = BuildRecursiveDepthExpr(
          depth_mod, emit_expr, cte_col, is_recursive_arm);
      if (depth_expr.empty()) return "";
      anchor_projs.push_back(absl::StrCat(depth_expr, " AS ", dst_q));
      continue;
    }
    std::string src_q = internal::QuoteIdent(cte_col.name());
    anchor_projs.push_back(cte_col.name() == anchor_names[j]
                               ? src_q
                               : absl::StrCat(src_q, " AS ", dst_q));
  }
  return absl::StrCat("SELECT ",
                      absl::StrJoin(anchor_projs, ", "),
                      " FROM (",
                      normalized_sql,
                      ")");
}

}  // namespace

std::string WithScanColumnAnchor(int idx) {
  return absl::StrCat("_cte_", idx);
}

WithEntryEmitResult EmitNonRecursiveWithEntry(
    const ::googlesql::ResolvedWithEntry* entry,
    const ::googlesql::ResolvedScan* sub_scan,
    bool body_needs_input_rn,
    const EmitScanFn& emit_scan) {
  WithEntryEmitResult result;
  bool cte_has_rn = false;
  std::string sub = emit_scan(sub_scan);
  if (sub.empty()) return result;
  if (body_needs_input_rn &&
      sub_scan->node_kind() == ::googlesql::RESOLVED_SET_OPERATION_SCAN &&
      sub_scan->GetAs<::googlesql::ResolvedSetOperationScan>()->op_type() ==
          ::googlesql::ResolvedSetOperationScan::UNION_ALL) {
    sub = absl::StrCat("SELECT *, row_number() OVER () AS ",
                       internal::QuoteIdent(internal::kBqInputRnCol),
                       " FROM (",
                       sub,
                       ")");
    cte_has_rn = true;
  }
  std::vector<std::string> cols;
  cols.reserve(sub_scan->column_list_size());
  for (int j = 0; j < sub_scan->column_list_size(); ++j) {
    cols.push_back(
        absl::StrCat(internal::QuoteIdent(sub_scan->column_list(j).name()),
                     " AS ",
                     internal::QuoteIdent(WithScanColumnAnchor(j))));
  }
  std::string projected;
  if (cols.empty()) {
    projected = absl::StrCat("SELECT * FROM (", sub, ")");
  } else {
    projected =
        absl::StrCat("SELECT ", absl::StrJoin(cols, ", "), " FROM (", sub, ")");
  }
  if (cte_has_rn && !cols.empty()) {
    projected = absl::StrCat(projected.substr(0, projected.find(" FROM ")),
                             ", ",
                             internal::QuoteIdent(internal::kBqInputRnCol),
                             projected.substr(projected.find(" FROM ")));
  }
  result.cte_sql = absl::StrCat(
      internal::QuoteIdent(entry->with_query_name()), " AS (", projected, ")");
  result.has_rn = cte_has_rn;
  return result;
}

std::string FormatRecursiveWithEntry(
    absl::string_view query_name,
    const std::vector<std::string>& anchor_names,
    absl::string_view body_sql) {
  std::vector<std::string> quoted_cols;
  quoted_cols.reserve(anchor_names.size());
  for (const std::string& name : anchor_names) {
    quoted_cols.push_back(internal::QuoteIdent(name));
  }
  std::string cols_clause =
      quoted_cols.empty()
          ? std::string()
          : absl::StrCat("(", absl::StrJoin(quoted_cols, ", "), ")");
  return absl::StrCat(
      internal::QuoteIdent(query_name), cols_clause, " AS (", body_sql, ")");
}

int FindRecursionDepthColumnIndex(
    const ::googlesql::ResolvedRecursiveScan* node,
    const ::googlesql::ResolvedRecursionDepthModifier* depth_mod) {
  if (depth_mod == nullptr || depth_mod->recursion_depth_column() == nullptr) {
    return -1;
  }
  const int depth_id =
      depth_mod->recursion_depth_column()->column().column_id();
  for (int j = 0; j < node->column_list_size(); ++j) {
    if (node->column_list(j).column_id() == depth_id) {
      return j;
    }
  }
  return -2;
}

std::string BuildRecursiveScanArm(
    const ::googlesql::ResolvedRecursiveScan* node,
    const std::vector<std::string>& anchor_names,
    int depth_col_idx,
    const ::googlesql::ResolvedRecursionDepthModifier* depth_mod,
    const EmitExprFn& emit_expr,
    const EmitScanFn& emit_scan,
    const ::googlesql::ResolvedSetOperationItem* item,
    bool is_recursive_arm) {
  if (item == nullptr || item->scan() == nullptr) return "";
  if (item->output_column_list_size() != node->column_list_size()) {
    return "";
  }
  std::string inner = emit_scan(item->scan());
  if (inner.empty()) return "";
  std::string normalized_sql = BuildRecursiveNormalizedSelect(
      node, anchor_names, depth_col_idx, item, is_recursive_arm, inner);
  return BuildRecursiveAnchorSelect(node,
                                    anchor_names,
                                    depth_col_idx,
                                    depth_mod,
                                    emit_expr,
                                    is_recursive_arm,
                                    normalized_sql);
}

}  // namespace transpiler
}  // namespace duckdb
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
