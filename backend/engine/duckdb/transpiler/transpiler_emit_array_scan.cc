#include <string>
#include <vector>

#include "absl/container/flat_hash_set.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/str_join.h"
#include "absl/strings/str_join.h"
#include "backend/engine/duckdb/transpiler/transpiler.h"
#include "backend/engine/duckdb/transpiler/transpiler_internal.h"
#include "googlesql/resolved_ast/resolved_ast.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace duckdb {
namespace transpiler {
namespace {

static void AppendScanColumnIds(const ::googlesql::ResolvedScan* scan,
                                absl::flat_hash_set<int>* ids) {
  if (scan == nullptr || ids == nullptr) return;
  for (int i = 0; i < scan->column_list_size(); ++i) {
    ids->insert(scan->column_list(i).column_id());
  }
  switch (scan->node_kind()) {
    case ::googlesql::RESOLVED_PROJECT_SCAN:
      AppendScanColumnIds(
          scan->GetAs<::googlesql::ResolvedProjectScan>()->input_scan(), ids);
      break;
    case ::googlesql::RESOLVED_FILTER_SCAN:
      AppendScanColumnIds(
          scan->GetAs<::googlesql::ResolvedFilterScan>()->input_scan(), ids);
      break;
    case ::googlesql::RESOLVED_ORDER_BY_SCAN:
      AppendScanColumnIds(
          scan->GetAs<::googlesql::ResolvedOrderByScan>()->input_scan(), ids);
      break;
    case ::googlesql::RESOLVED_LIMIT_OFFSET_SCAN:
      AppendScanColumnIds(
          scan->GetAs<::googlesql::ResolvedLimitOffsetScan>()->input_scan(),
          ids);
      break;
    case ::googlesql::RESOLVED_JOIN_SCAN: {
      const auto* join = scan->GetAs<::googlesql::ResolvedJoinScan>();
      AppendScanColumnIds(join->left_scan(), ids);
      AppendScanColumnIds(join->right_scan(), ids);
      break;
    }
    case ::googlesql::RESOLVED_WITH_SCAN:
      AppendScanColumnIds(scan->GetAs<::googlesql::ResolvedWithScan>()->query(),
                          ids);
      break;
    default:
      break;
  }
}

static std::string QualifyLateralArrayExpr(
    const ::googlesql::ResolvedExpr* expr) {
  if (expr == nullptr) return "";
  if (expr->node_kind() == ::googlesql::RESOLVED_COLUMN_REF) {
    const auto* ref = expr->GetAs<::googlesql::ResolvedColumnRef>();
    if (ref == nullptr) return "";
    return absl::StrCat("__bq_l.", internal::QuoteIdent(ref->column().name()));
  }
  return "";
}

static std::string EmitCorrelatedArrayScan(
    const ::googlesql::ResolvedArrayScan* node,
    const std::string& input,
    bool* input_has_rn_column,
    bool* join_output_uses_id_aliases) {
  if (node == nullptr || input.empty() || input_has_rn_column == nullptr ||
      join_output_uses_id_aliases == nullptr) {
    return "";
  }
  const std::string arr = QualifyLateralArrayExpr(node->array_expr_list(0));
  if (arr.empty()) return "";
  const std::string quoted_col =
      internal::QuoteIdent(node->element_column_list(0).name());
  const std::string rn_quoted = internal::QuoteIdent(internal::kBqInputRnCol);
  const int elem_col_id = node->element_column_list(0).column_id();

  absl::flat_hash_set<int> left_ids;
  AppendScanColumnIds(node->input_scan(), &left_ids);

  std::vector<std::string> projections;
  projections.reserve(node->column_list_size() + 1);
  for (int i = 0; i < node->column_list_size(); ++i) {
    const ::googlesql::ResolvedColumn& col = node->column_list(i);
    const int col_id = col.column_id();
    const std::string quoted = internal::QuoteIdent(col.name());
    std::string ref;
    if (left_ids.contains(col_id)) {
      ref = absl::StrCat("__bq_l.", quoted);
    } else if (col_id == elem_col_id) {
      ref = absl::StrCat("__bq_unnest__.", quoted_col);
    } else {
      return "";
    }
    projections.push_back(
        absl::StrCat(ref, " AS ", internal::JoinColumnIdAlias(col_id)));
  }
  bool has_rn = false;
  for (const std::string& projection : projections) {
    if (projection.find(rn_quoted) != std::string::npos) {
      has_rn = true;
      break;
    }
  }
  if (!has_rn) {
    projections.push_back(absl::StrCat("ord AS ", rn_quoted));
  }
  if (projections.empty()) return "";
  *input_has_rn_column = true;
  *join_output_uses_id_aliases = true;
  return absl::StrCat("SELECT ",
                      absl::StrJoin(projections, ", "),
                      " FROM (",
                      input,
                      ") AS __bq_l, unnest(",
                      arr,
                      ") WITH ORDINALITY AS __bq_unnest__(",
                      quoted_col,
                      ", ord)");
}

}  // namespace

std::string Transpiler::EmitArrayScan(
    const ::googlesql::ResolvedArrayScan* node) {
  if (node == nullptr) return "";
  if (node->array_expr_list_size() != 1 ||
      node->element_column_list_size() != 1 ||
      node->array_offset_column() != nullptr || node->join_expr() != nullptr ||
      node->is_outer() || node->array_zip_mode() != nullptr) {
    return "";
  }
  if (node->input_scan() != nullptr &&
      node->input_scan()->node_kind() !=
          ::googlesql::RESOLVED_SINGLE_ROW_SCAN) {
    std::string input = EmitScan(node->input_scan());
    if (input.empty()) return "";
    return EmitCorrelatedArrayScan(
        node, input, &input_has_rn_column_, &join_output_uses_id_aliases_);
  }
  std::string arr = EmitExpr(node->array_expr_list(0));
  if (arr.empty()) return "";
  const std::string quoted_col =
      internal::QuoteIdent(node->element_column_list(0).name());
  input_has_rn_column_ = true;
  return absl::StrCat("SELECT ",
                      quoted_col,
                      ", ord AS ",
                      internal::QuoteIdent(internal::kBqInputRnCol),
                      " FROM unnest(",
                      arr,
                      ") WITH ORDINALITY AS __bq_unnest__(",
                      quoted_col,
                      ", ord)");
}

}  // namespace transpiler
}  // namespace duckdb
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
