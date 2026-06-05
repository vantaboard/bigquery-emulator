#include <algorithm>
#include <optional>
#include <string>
#include <vector>

#include "absl/container/flat_hash_map.h"
#include "absl/container/flat_hash_set.h"
#include "absl/log/log.h"
#include "absl/strings/ascii.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/str_join.h"
#include "absl/strings/str_replace.h"
#include "absl/strings/string_view.h"
#include "absl/time/time.h"
#include "backend/engine/disposition.h"
#include "backend/engine/duckdb/transpiler/functions.h"
#include "backend/engine/duckdb/transpiler/transpiler.h"
#include "backend/engine/duckdb/transpiler/transpiler_internal.h"
#include "backend/engine/duckdb/transpiler/types.h"
#include "googlesql/public/catalog.h"
#include "googlesql/public/function.h"
#include "googlesql/public/options.pb.h"
#include "googlesql/public/sql_function.h"
#include "googlesql/public/templated_sql_function.h"
#include "googlesql/public/type.h"
#include "googlesql/public/type.pb.h"
#include "googlesql/public/types/struct_type.h"
#include "googlesql/public/value.h"
#include "googlesql/resolved_ast/resolved_ast.h"
#include "googlesql/resolved_ast/resolved_ast_visitor.h"
#include "googlesql/resolved_ast/resolved_node.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace duckdb {
namespace transpiler {

namespace {

bool OutputIncludesColumnName(const std::vector<std::string>& output_names,
                              absl::string_view col_name) {
  if (output_names.empty()) return true;
  return std::find(output_names.begin(), output_names.end(), col_name) !=
         output_names.end();
}

std::string BuildPercentileDiscRespectNullsSql(
    absl::string_view coalesce_expr,
    absl::string_view p_expr,
    absl::string_view partition_clause,
    absl::string_view frame_clause) {
  (void)coalesce_expr;
  (void)p_expr;
  (void)partition_clause;
  (void)frame_clause;
  return "";
}

std::string BuildPercentileDiscRespectNullsScalarSql(
    absl::string_view p_expr, absl::string_view col_name) {
  return absl::StrCat("NULLIF(list_extract(list(",
                      internal::QuoteIdent(internal::kBqPctCoalesceCol),
                      " ORDER BY ",
                      internal::QuoteIdent(internal::kBqPctCoalesceCol),
                      " ASC), CAST(FLOOR((",
                      p_expr,
                      ") * (COUNT(*) - 1)) + 1 AS BIGINT)), ",
                      internal::kBqPctNullSentinel,
                      ") AS ",
                      internal::QuoteIdent(col_name));
}

bool AnalyticScanIsOnlyPercentileDiscRespectNulls(
    const ::googlesql::ResolvedAnalyticScan* node) {
  if (node == nullptr || node->function_group_list_size() == 0) return false;
  for (int g = 0; g < node->function_group_list_size(); ++g) {
    const ::googlesql::ResolvedAnalyticFunctionGroup* group =
        node->function_group_list(g);
    if (group == nullptr) return false;
    if (group->partition_by() != nullptr &&
        group->partition_by()->partition_by_list_size() > 0) {
      return false;
    }
    for (int f = 0; f < group->analytic_function_list_size(); ++f) {
      const ::googlesql::ResolvedComputedColumnBase* fn_col =
          group->analytic_function_list(f);
      if (fn_col == nullptr || fn_col->expr() == nullptr ||
          fn_col->expr()->node_kind() !=
              ::googlesql::RESOLVED_ANALYTIC_FUNCTION_CALL) {
        return false;
      }
      const auto* afn =
          fn_col->expr()->GetAs<::googlesql::ResolvedAnalyticFunctionCall>();
      if (afn == nullptr || afn->function() == nullptr ||
          internal::ResolveFunctionName(afn->function()) != "percentile_disc" ||
          afn->null_handling_modifier() !=
              ::googlesql::ResolvedNonScalarFunctionCallBase::RESPECT_NULLS) {
        return false;
      }
    }
  }
  return true;
}

}  // namespace

std::string Transpiler::BuildPartitionClause(
    const ::googlesql::ResolvedWindowPartitioning* p) {
  if (p == nullptr) return "";
  // PARTITION BY hints and collations are BQ-specific; bail so the
  // engine surfaces UNIMPLEMENTED.
  if (!p->collation_list().empty() || p->hint_list_size() > 0) {
    return std::string(kAnalyticBail);
  }
  std::vector<std::string> cols;
  cols.reserve(p->partition_by_list_size());
  for (int i = 0; i < p->partition_by_list_size(); ++i) {
    std::string c = EmitColumnRef(p->partition_by_list(i));
    if (c.empty()) return std::string(kAnalyticBail);
    cols.push_back(std::move(c));
  }
  if (cols.empty()) return "";
  return absl::StrCat("PARTITION BY ", absl::StrJoin(cols, ", "));
}

std::string Transpiler::BuildOrderClause(
    const ::googlesql::ResolvedWindowOrdering* o) {
  return BuildOrderClause(o,
                          /*bigquery_null_defaults=*/false,
                          /*append_input_rn=*/false);
}

std::string Transpiler::BuildOrderClause(
    const ::googlesql::ResolvedWindowOrdering* o,
    bool bigquery_null_defaults,
    bool append_input_rn) {
  if (o == nullptr) return "";
  if (o->hint_list_size() > 0) return std::string(kAnalyticBail);
  std::vector<std::string> items;
  items.reserve(o->order_by_item_list_size() + 1);
  for (int i = 0; i < o->order_by_item_list_size(); ++i) {
    const ::googlesql::ResolvedOrderByItem* it = o->order_by_item_list(i);
    if (it == nullptr || it->column_ref() == nullptr)
      return std::string(kAnalyticBail);
    if (it->collation_name() != nullptr) return std::string(kAnalyticBail);
    std::string col = EmitColumnRef(it->column_ref());
    if (col.empty()) return std::string(kAnalyticBail);
    items.push_back(absl::StrCat(
        col, internal::OrderByItemSuffix(it, bigquery_null_defaults)));
  }
  if (append_input_rn) {
    items.push_back(
        absl::StrCat(internal::QuoteIdent(internal::kBqInputRnCol), " ASC"));
  }
  if (items.empty()) return "";
  return absl::StrCat("ORDER BY ", absl::StrJoin(items, ", "));
}

std::string Transpiler::BuildFrameClause(
    const ::googlesql::ResolvedWindowFrame* wf) {
  if (wf == nullptr) return "";
  const char* unit = nullptr;
  switch (wf->frame_unit()) {
    case ::googlesql::ResolvedWindowFrame::ROWS:
      unit = "ROWS";
      break;
    case ::googlesql::ResolvedWindowFrame::RANGE:
      unit = "RANGE";
      break;
    default:
      return std::string(kAnalyticBail);
  }
  std::string start = EmitFrameBound(wf->start_expr());
  if (start.empty()) return std::string(kAnalyticBail);
  std::string end = EmitFrameBound(wf->end_expr());
  if (end.empty()) return std::string(kAnalyticBail);
  return absl::StrCat(unit, " BETWEEN ", start, " AND ", end);
}

std::string Transpiler::BuildAnalyticProjection(
    const ::googlesql::ResolvedComputedColumnBase* col,
    absl::string_view partition_clause,
    absl::string_view order_clause) {
  if (col == nullptr || col->expr() == nullptr) return "";
  if (col->expr()->node_kind() !=
      ::googlesql::RESOLVED_ANALYTIC_FUNCTION_CALL) {
    return "";
  }
  const auto* afn =
      col->expr()->GetAs<::googlesql::ResolvedAnalyticFunctionCall>();
  std::string frame_clause = BuildFrameClause(afn->window_frame());
  if (frame_clause == kAnalyticBail) return "";

  std::string fn_sql;
  if (afn != nullptr && afn->function() != nullptr &&
      internal::ResolveFunctionName(afn->function()) == "percentile_disc" &&
      afn->null_handling_modifier() ==
          ::googlesql::ResolvedNonScalarFunctionCallBase::RESPECT_NULLS &&
      afn->argument_list_size() >= 2) {
    std::string sort_key = EmitExpr(afn->argument_list(0));
    std::string p_expr = EmitExpr(afn->argument_list(1));
    if (sort_key.empty() || p_expr.empty()) return "";
    const std::string coalesce_col =
        internal::QuoteIdent(internal::kBqPctCoalesceCol);
    fn_sql = BuildPercentileDiscRespectNullsSql(
        coalesce_col, p_expr, partition_clause, frame_clause);
  } else {
    fn_sql = EmitAnalyticFunctionCall(afn);
    if (fn_sql.empty()) return "";
  }

  // Frame clause sits *inside* the OVER (...). DuckDB requires an
  // ORDER BY for ROWS / RANGE frames; we leave that contract to the
  // analyzer (which rejects malformed cases at AnalyzeStatement time)
  // and just propagate the bounds verbatim.
  if (fn_sql.find(" OVER (") != std::string::npos) {
    return absl::StrCat(
        fn_sql, " AS ", internal::QuoteIdent(col->column().name()));
  }

  std::vector<absl::string_view> over_parts;
  if (!partition_clause.empty()) over_parts.push_back(partition_clause);
  if (!order_clause.empty()) over_parts.push_back(order_clause);
  if (!frame_clause.empty()) over_parts.push_back(frame_clause);
  std::string over =
      absl::StrCat("OVER (", absl::StrJoin(over_parts, " "), ")");
  return absl::StrCat(
      fn_sql, " ", over, " AS ", internal::QuoteIdent(col->column().name()));
}

void Transpiler::CaptureAnalyticOutputOrder(
    const ::googlesql::ResolvedAnalyticFunctionGroup* group) {
  if (group == nullptr || !output_order_items_.empty()) return;

  const ::googlesql::ResolvedWindowOrdering* order_by = group->order_by();
  const bool over_has_order =
      order_by != nullptr && order_by->order_by_item_list_size() > 0;

  const ::googlesql::ResolvedWindowPartitioning* partition =
      group->partition_by();
  if (partition != nullptr) {
    if (!partition->collation_list().empty() ||
        partition->hint_list_size() > 0) {
      return;
    }
    for (int i = 0; i < partition->partition_by_list_size(); ++i) {
      const ::googlesql::ResolvedExpr* part_expr_node =
          partition->partition_by_list(i);
      if (part_expr_node == nullptr) continue;
      if (part_expr_node->node_kind() == ::googlesql::RESOLVED_COLUMN_REF) {
        const auto* ref =
            part_expr_node->GetAs<::googlesql::ResolvedColumnRef>();
        if (ref == nullptr) continue;
        if (OutputIncludesColumnName(query_output_column_names_,
                                     ref->column().name()) ||
            !over_has_order) {
          std::string col = EmitColumnRef(ref);
          if (col.empty()) return;
          output_order_items_.push_back(absl::StrCat(col, " ASC"));
        }
      } else if (!over_has_order) {
        std::string part_expr = EmitExpr(part_expr_node);
        if (part_expr.empty()) return;
        output_order_items_.push_back(absl::StrCat(part_expr, " ASC"));
      }
    }
  }

  if (order_by != nullptr) {
    if (order_by->hint_list_size() > 0) return;
    for (int i = 0; i < order_by->order_by_item_list_size(); ++i) {
      const ::googlesql::ResolvedOrderByItem* item =
          order_by->order_by_item_list(i);
      if (item == nullptr || item->column_ref() == nullptr) return;
      if (item->collation_name() != nullptr) return;
      std::string col = EmitColumnRef(item->column_ref());
      if (col.empty()) return;
      output_order_items_.push_back(absl::StrCat(
          col,
          internal::OrderByItemSuffix(item, /*bigquery_null_defaults=*/true)));
    }
  }

  if (input_rn_ordering_) {
    output_order_items_.push_back(
        absl::StrCat(internal::QuoteIdent(internal::kBqInputRnCol), " ASC"));
  }
}

std::string Transpiler::EmitAnalyticScan(
    const ::googlesql::ResolvedAnalyticScan* node) {
  // Emit `SELECT *, <fn> OVER (PARTITION BY ... ORDER BY ... [frame])
  // AS "<col>", ... FROM (<input>)` -- one projection per analytic
  // function across every group. The OVER clause lives at the group
  // level (PARTITION / ORDER BY) plus per-function (window_frame), so
  // we walk the function-group list once and delegate the per-clause
  // assembly to `BuildPartitionClause` / `BuildOrderClause` /
  // `BuildAnalyticProjection`.
  //
  // Skiplisted shapes the first emit pass does not cover (each helper
  // returns `kAnalyticBail` for these, which we propagate as the
  // empty-string fallback contract):
  //   * Hint lists on the partition / order spec (PARTITION BY ...
  //     OPTIONS(...) and similar) -- BQ-specific.
  //   * Collation lists on PARTITION BY -- collations land separately.
  //   * The `partition_by` `parameter_list` -- lateral-correlated
  //     analytics; defer to the lateral-rewrite plan.
  if (node == nullptr) return "";
  std::string input = EmitScan(node->input_scan());
  if (input.empty()) return "";
  if (!input_has_rn_column_) {
    input = absl::StrCat("SELECT *, row_number() OVER () AS ",
                         internal::QuoteIdent(internal::kBqInputRnCol),
                         " FROM (",
                         input,
                         ")");
    input_has_rn_column_ = true;
  }
  input_rn_ordering_ = true;

  std::string pct_respect_nulls_sort_key;
  for (int g = 0; g < node->function_group_list_size(); ++g) {
    const ::googlesql::ResolvedAnalyticFunctionGroup* group =
        node->function_group_list(g);
    if (group == nullptr) continue;
    for (int f = 0; f < group->analytic_function_list_size(); ++f) {
      const ::googlesql::ResolvedComputedColumnBase* fn_col =
          group->analytic_function_list(f);
      if (fn_col == nullptr || fn_col->expr() == nullptr ||
          fn_col->expr()->node_kind() !=
              ::googlesql::RESOLVED_ANALYTIC_FUNCTION_CALL) {
        continue;
      }
      const auto* afn =
          fn_col->expr()->GetAs<::googlesql::ResolvedAnalyticFunctionCall>();
      if (afn == nullptr || afn->function() == nullptr ||
          internal::ResolveFunctionName(afn->function()) != "percentile_disc" ||
          afn->null_handling_modifier() !=
              ::googlesql::ResolvedNonScalarFunctionCallBase::RESPECT_NULLS ||
          afn->argument_list_size() == 0) {
        continue;
      }
      pct_respect_nulls_sort_key = EmitExpr(afn->argument_list(0));
      if (pct_respect_nulls_sort_key.empty()) return "";
      break;
    }
    if (!pct_respect_nulls_sort_key.empty()) break;
  }
  if (!pct_respect_nulls_sort_key.empty()) {
    input = absl::StrCat("SELECT *, IF(",
                         pct_respect_nulls_sort_key,
                         " IS NULL, ",
                         internal::kBqPctNullSentinel,
                         ", ",
                         pct_respect_nulls_sort_key,
                         ") AS ",
                         internal::QuoteIdent(internal::kBqPctCoalesceCol),
                         " FROM (",
                         input,
                         ")");
  }

  if (AnalyticScanIsOnlyPercentileDiscRespectNulls(node) &&
      !pct_respect_nulls_sort_key.empty()) {
    std::vector<std::string> pct_projections;
    std::vector<std::string> pct_refs;
    for (int g = 0; g < node->function_group_list_size(); ++g) {
      const ::googlesql::ResolvedAnalyticFunctionGroup* group =
          node->function_group_list(g);
      if (group == nullptr) return "";
      for (int f = 0; f < group->analytic_function_list_size(); ++f) {
        const ::googlesql::ResolvedComputedColumnBase* fn_col =
            group->analytic_function_list(f);
        if (fn_col == nullptr || fn_col->expr() == nullptr ||
            fn_col->expr()->node_kind() !=
                ::googlesql::RESOLVED_ANALYTIC_FUNCTION_CALL) {
          return "";
        }
        const auto* afn =
            fn_col->expr()->GetAs<::googlesql::ResolvedAnalyticFunctionCall>();
        if (afn == nullptr || afn->argument_list_size() < 2) return "";
        std::string p_expr = EmitExpr(afn->argument_list(1));
        if (p_expr.empty()) return "";
        pct_projections.push_back(BuildPercentileDiscRespectNullsScalarSql(
            p_expr, fn_col->column().name()));
        pct_refs.push_back(absl::StrCat(
            "_pct.", internal::QuoteIdent(fn_col->column().name())));
      }
    }
    if (pct_projections.empty()) return "";
    return absl::StrCat("SELECT _base.*, ",
                        absl::StrJoin(pct_refs, ", "),
                        " FROM (",
                        input,
                        ") _base CROSS JOIN (SELECT ",
                        absl::StrJoin(pct_projections, ", "),
                        " FROM (",
                        input,
                        ")) _pct");
  }

  std::vector<std::string> projections;
  for (int g = 0; g < node->function_group_list_size(); ++g) {
    const ::googlesql::ResolvedAnalyticFunctionGroup* group =
        node->function_group_list(g);
    if (group == nullptr) return "";

    std::string partition_clause = BuildPartitionClause(group->partition_by());
    if (partition_clause == kAnalyticBail) return "";
    const bool append_input_rn =
        input_rn_ordering_ && !internal::AnalyticGroupHasRangeFrame(group) &&
        (internal::AnalyticOrderNeedsInputRn(group) ||
         internal::AnalyticGroupNeedsInputRnForEmptyOrder(group));
    std::string order_clause =
        BuildOrderClause(group->order_by(),
                         /*bigquery_null_defaults=*/true,
                         /*append_input_rn=*/append_input_rn);
    if (order_clause == kAnalyticBail) return "";
    CaptureAnalyticOutputOrder(group);

    for (int f = 0; f < group->analytic_function_list_size(); ++f) {
      const ::googlesql::ResolvedComputedColumnBase* fn_col =
          group->analytic_function_list(f);
      std::string effective_order = order_clause;
      if (fn_col != nullptr && fn_col->expr() != nullptr &&
          fn_col->expr()->node_kind() ==
              ::googlesql::RESOLVED_ANALYTIC_FUNCTION_CALL) {
        const auto* afn =
            fn_col->expr()->GetAs<::googlesql::ResolvedAnalyticFunctionCall>();
        if (afn != nullptr && afn->function() != nullptr &&
            internal::ResolveFunctionName(afn->function()) ==
                "percentile_disc" &&
            afn->null_handling_modifier() ==
                ::googlesql::ResolvedNonScalarFunctionCallBase::RESPECT_NULLS) {
          effective_order.clear();
        }
      }
      std::string projection =
          BuildAnalyticProjection(fn_col, partition_clause, effective_order);
      if (projection.empty()) return "";
      projections.push_back(std::move(projection));
    }
  }

  if (projections.empty()) {
    // No analytic functions in any group is a malformed AST; the
    // analyzer would not produce it, but we guard so an unexpected
    // shape falls back rather than emitting illegal SQL.
    return "";
  }
  std::string select_list =
      absl::StrCat("*, ", absl::StrJoin(projections, ", "));
  return absl::StrCat("SELECT ", select_list, " FROM (", input, ")");
}

}  // namespace transpiler
}  // namespace duckdb
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
