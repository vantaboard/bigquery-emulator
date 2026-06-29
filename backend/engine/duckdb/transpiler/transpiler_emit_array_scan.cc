

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
    const ::googlesql::ResolvedExpr* expr, bool input_id_aliases) {
  if (expr == nullptr) return "";
  if (expr->node_kind() == ::googlesql::RESOLVED_COLUMN_REF) {
    const auto* ref = expr->GetAs<::googlesql::ResolvedColumnRef>();
    if (ref == nullptr) return "";
    if (input_id_aliases) {
      return absl::StrCat(
          "__bq_l.", internal::JoinColumnIdAlias(ref->column().column_id()));
    }
    return absl::StrCat("__bq_l.", internal::QuoteIdent(ref->column().name()));
  }
  return "";
}

struct UnnestSideColumn {
  int column_id;
  std::string name;
};

static std::string EmitStandaloneUnnestSideFromExpr(
    absl::string_view arr, absl::string_view col_name) {
  if (arr.empty() || col_name.empty()) return "";
  const std::string quoted_col = internal::QuoteIdent(col_name);
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

static void AppendCrossJoinRnProjection(bool left_has_rn,
                                        bool right_has_rn,
                                        std::vector<std::string>* projections) {
  if (projections == nullptr) return;
  const std::string rn_quoted = internal::QuoteIdent(internal::kBqInputRnCol);
  for (const std::string& projection : *projections) {
    if (projection.find(rn_quoted) != std::string::npos) return;
  }
  const char* side =
      left_has_rn ? "__bq_l" : (right_has_rn ? "__bq_r" : nullptr);
  if (side == nullptr) return;
  projections->push_back(absl::StrCat(side, ".", rn_quoted, " AS ", rn_quoted));
}

static std::string EmitBinaryCrossJoinMerge(
    const std::string& left_sql,
    const std::vector<UnnestSideColumn>& left_cols,
    bool left_id_aliases,
    bool left_has_rn,
    const std::string& right_sql,
    const std::vector<UnnestSideColumn>& right_cols,
    bool right_id_aliases,
    bool right_has_rn) {
  if (left_sql.empty() || right_sql.empty()) return "";
  std::vector<std::string> projections;
  projections.reserve(left_cols.size() + right_cols.size() + 1);
  auto append_side = [&](const std::vector<UnnestSideColumn>& cols,
                         const char* side_alias,
                         bool id_aliases) {
    for (const UnnestSideColumn& col : cols) {
      const std::string quoted = internal::QuoteIdent(col.name);
      const std::string ref =
          id_aliases
              ? absl::StrCat(
                    side_alias, ".", internal::JoinColumnIdAlias(col.column_id))
              : absl::StrCat(side_alias, ".", quoted);
      projections.push_back(absl::StrCat(
          ref, " AS ", internal::JoinColumnIdAlias(col.column_id)));
    }
  };
  append_side(left_cols, "__bq_l", left_id_aliases);
  append_side(right_cols, "__bq_r", right_id_aliases);
  AppendCrossJoinRnProjection(left_has_rn, right_has_rn, &projections);
  if (projections.empty()) return "";
  return absl::StrCat("SELECT ",
                      absl::StrJoin(projections, ", "),
                      " FROM (",
                      left_sql,
                      ") AS __bq_l CROSS JOIN (",
                      right_sql,
                      ") AS __bq_r");
}

static bool IsStandaloneUnnestShape(
    const ::googlesql::ResolvedArrayScan* node) {
  return node != nullptr && node->array_expr_list_size() == 1 &&
         node->element_column_list_size() == 1 &&
         node->array_offset_column() == nullptr &&
         node->join_expr() == nullptr && !node->is_outer() &&
         node->array_zip_mode() == nullptr;
}

static bool CollectNestedUnnestChain(
    const ::googlesql::ResolvedArrayScan* outer,
    std::vector<const ::googlesql::ResolvedArrayScan*>* chain) {
  if (outer == nullptr || chain == nullptr) return false;
  chain->clear();
  const ::googlesql::ResolvedArrayScan* cur = outer;
  while (true) {
    if (!IsStandaloneUnnestShape(cur)) return false;
    chain->push_back(cur);
    const ::googlesql::ResolvedScan* input = cur->input_scan();
    if (input == nullptr) {
      return chain->size() >= 2;
    }
    if (input->node_kind() != ::googlesql::RESOLVED_ARRAY_SCAN) {
      return false;
    }
    cur = input->GetAs<::googlesql::ResolvedArrayScan>();
  }
}

static bool IsNestedStandaloneUnnestCrossProduct(
    const ::googlesql::ResolvedArrayScan* node) {
  std::vector<const ::googlesql::ResolvedArrayScan*> chain;
  return CollectNestedUnnestChain(node, &chain);
}

static bool IsCrossProductMultiArrayScan(
    const ::googlesql::ResolvedArrayScan* node) {
  if (node == nullptr) return false;
  if (node->array_expr_list_size() < 2) return false;
  if (node->array_zip_mode() != nullptr ||
      node->array_offset_column() != nullptr || node->join_expr() != nullptr ||
      node->is_outer()) {
    return false;
  }
  if (node->element_column_list_size() != node->array_expr_list_size()) {
    return false;
  }
  if (node->input_scan() != nullptr &&
      node->input_scan()->node_kind() !=
          ::googlesql::RESOLVED_SINGLE_ROW_SCAN) {
    return false;
  }
  return true;
}

static std::string EmitUnnestCrossProductChain(
    const ::googlesql::ResolvedArrayScan* output_node,
    const std::vector<std::pair<std::string, UnnestSideColumn>>& sides) {
  if (output_node == nullptr || sides.size() < 2) return "";
  std::string combined =
      EmitStandaloneUnnestSideFromExpr(sides[0].first, sides[0].second.name);
  if (combined.empty()) return "";
  std::vector<UnnestSideColumn> accumulated;
  accumulated.push_back(sides[0].second);
  bool combined_id_aliases = false;
  const bool side_has_rn = true;
  for (size_t i = 1; i < sides.size(); ++i) {
    std::string right =
        EmitStandaloneUnnestSideFromExpr(sides[i].first, sides[i].second.name);
    if (right.empty()) return "";
    std::vector<UnnestSideColumn> right_cols = {sides[i].second};
    combined = EmitBinaryCrossJoinMerge(combined,
                                        accumulated,
                                        combined_id_aliases,
                                        side_has_rn,
                                        right,
                                        right_cols,
                                        /*right_id_aliases=*/false,
                                        side_has_rn);
    if (combined.empty()) return "";
    accumulated.insert(accumulated.end(), right_cols.begin(), right_cols.end());
    combined_id_aliases = true;
  }

  absl::flat_hash_set<int> emitted_ids;
  std::vector<std::string> projections;
  projections.reserve(output_node->column_list_size() + 1);
  for (int i = 0; i < output_node->column_list_size(); ++i) {
    const ::googlesql::ResolvedColumn& col = output_node->column_list(i);
    const int col_id = col.column_id();
    if (!emitted_ids.insert(col_id).second) continue;
    const std::string id_alias = internal::JoinColumnIdAlias(col_id);
    projections.push_back(absl::StrCat("__bq_l.", id_alias, " AS ", id_alias));
  }
  AppendCrossJoinRnProjection(/*left_has_rn=*/true,
                              /*right_has_rn=*/false,
                              &projections);
  if (projections.empty()) return "";
  return absl::StrCat("SELECT ",
                      absl::StrJoin(projections, ", "),
                      " FROM (",
                      combined,
                      ") AS __bq_l");
}

static std::string EmitCrossProductMultiArrayScanImpl(
    const ::googlesql::ResolvedArrayScan* node,
    const std::function<std::string(int index)>& emit_array_expr) {
  if (node == nullptr || !emit_array_expr) return "";
  std::vector<std::pair<std::string, UnnestSideColumn>> sides;
  sides.reserve(node->array_expr_list_size());
  for (int i = 0; i < node->array_expr_list_size(); ++i) {
    std::string arr = emit_array_expr(i);
    if (arr.empty()) return "";
    sides.push_back({std::move(arr),
                     {node->element_column_list(i).column_id(),
                      std::string(node->element_column_list(i).name())}});
  }
  return EmitUnnestCrossProductChain(node, sides);
}

static std::string EmitCorrelatedArrayScan(
    const ::googlesql::ResolvedArrayScan* node,
    const std::string& input,
    bool input_id_aliases,
    bool* input_has_rn_column,
    bool* join_output_uses_id_aliases) {
  if (node == nullptr || input.empty() || input_has_rn_column == nullptr ||
      join_output_uses_id_aliases == nullptr) {
    return "";
  }
  const std::string arr =
      QualifyLateralArrayExpr(node->array_expr_list(0), input_id_aliases);
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
      ref = input_id_aliases
                ? absl::StrCat("__bq_l.", internal::JoinColumnIdAlias(col_id))
                : absl::StrCat("__bq_l.", quoted);
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
  if (IsCrossProductMultiArrayScan(node) ||
      IsNestedStandaloneUnnestCrossProduct(node)) {
    return EmitUnnestCrossProductScan(node);
  }
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
    const bool input_id_aliases = join_output_uses_id_aliases_;
    std::string correlated =
        EmitCorrelatedArrayScan(node,
                                input,
                                input_id_aliases,
                                &input_has_rn_column_,
                                &join_output_uses_id_aliases_);
    if (!correlated.empty()) {
      join_id_aliases_in_query_ = true;
      join_output_columns_use_id_aliases_ = true;
    }
    return correlated;
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

std::string Transpiler::EmitUnnestCrossProductScan(
    const ::googlesql::ResolvedArrayScan* node) {
  if (node == nullptr) return "";
  std::string sql;
  if (IsCrossProductMultiArrayScan(node)) {
    sql = EmitCrossProductMultiArrayScanImpl(
        node, [this, node](int index) -> std::string {
          return EmitExpr(node->array_expr_list(index));
        });
  } else {
    std::vector<const ::googlesql::ResolvedArrayScan*> chain;
    if (!CollectNestedUnnestChain(node, &chain)) return "";
    std::vector<std::pair<std::string, UnnestSideColumn>> sides;
    sides.reserve(chain.size());
    for (auto it = chain.rbegin(); it != chain.rend(); ++it) {
      const ::googlesql::ResolvedArrayScan* layer = *it;
      std::string arr = EmitExpr(layer->array_expr_list(0));
      if (arr.empty()) return "";
      sides.push_back({std::move(arr),
                       {layer->element_column_list(0).column_id(),
                        std::string(layer->element_column_list(0).name())}});
    }
    sql = EmitUnnestCrossProductChain(node, sides);
  }
  if (sql.empty()) return "";
  join_output_uses_id_aliases_ = true;
  join_id_aliases_in_query_ = true;
  join_output_columns_use_id_aliases_ = true;
  input_has_rn_column_ = true;
  return sql;
}

}  // namespace transpiler
}  // namespace duckdb
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
