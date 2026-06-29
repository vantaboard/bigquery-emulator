#include <algorithm>
#include <functional>
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
#include "backend/engine/duckdb/transpiler/transpiler_emit_analytic_helpers.h"
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

constexpr absl::string_view kLocalAnalyticBail =
    "\x01"
    "bail";

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

using EmitExprFn = std::function<std::string(const ::googlesql::ResolvedExpr*)>;
using EmitColumnRefFn =
    std::function<std::string(const ::googlesql::ResolvedColumnRef*)>;

void CapturePartitionOutputOrder(
    const ::googlesql::ResolvedWindowPartitioning* partition,
    bool over_has_order,
    const std::vector<std::string>& query_output_column_names,
    const EmitExprFn& emit_expr,
    const EmitColumnRefFn& emit_col,
    std::vector<std::string>* output_order_items,
    std::vector<int>* output_order_column_ids,
    bool* failed) {
  if (partition == nullptr || *failed) return;
  (void)partition->hint_list_size();
  if (!partition->collation_list().empty()) {
    return;
  }
  for (int i = 0; i < partition->partition_by_list_size(); ++i) {
    const ::googlesql::ResolvedExpr* part_expr_node =
        partition->partition_by_list(i);
    if (part_expr_node == nullptr) continue;
    if (part_expr_node->node_kind() == ::googlesql::RESOLVED_COLUMN_REF) {
      const auto* ref = part_expr_node->GetAs<::googlesql::ResolvedColumnRef>();
      if (ref == nullptr) continue;
      if (OutputIncludesColumnName(query_output_column_names,
                                   ref->column().name()) ||
          !over_has_order) {
        std::string col = emit_col(ref);
        if (col.empty()) {
          *failed = true;
          return;
        }
        output_order_items->push_back(absl::StrCat(col, " ASC"));
        output_order_column_ids->push_back(ref->column().column_id());
      }
    } else if (!over_has_order) {
      std::string part_expr = emit_expr(part_expr_node);
      if (part_expr.empty()) {
        *failed = true;
        return;
      }
      output_order_items->push_back(absl::StrCat(part_expr, " ASC"));
      output_order_column_ids->push_back(-1);
    }
  }
}

void CaptureOrderByOutputOrder(
    const ::googlesql::ResolvedWindowOrdering* order_by,
    const EmitColumnRefFn& emit_col,
    std::vector<std::string>* output_order_items,
    std::vector<int>* output_order_column_ids,
    bool* failed) {
  if (order_by == nullptr || *failed) return;
  (void)order_by->hint_list_size();
  for (int i = 0; i < order_by->order_by_item_list_size(); ++i) {
    const ::googlesql::ResolvedOrderByItem* item =
        order_by->order_by_item_list(i);
    if (item == nullptr || item->column_ref() == nullptr) {
      *failed = true;
      return;
    }
    if (item->collation_name() != nullptr) {
      *failed = true;
      return;
    }
    std::string col = emit_col(item->column_ref());
    if (col.empty()) {
      *failed = true;
      return;
    }
    output_order_items->push_back(absl::StrCat(
        col,
        internal::OrderByItemSuffix(item, /*bigquery_null_defaults=*/true)));
    output_order_column_ids->push_back(
        item->column_ref()->column().column_id());
  }
}

bool PercentileDiscRespectNullsClearsOrder(
    const ::googlesql::ResolvedComputedColumnBase* fn_col) {
  if (fn_col == nullptr || fn_col->expr() == nullptr ||
      fn_col->expr()->node_kind() !=
          ::googlesql::RESOLVED_ANALYTIC_FUNCTION_CALL) {
    return false;
  }
  const auto* afn =
      fn_col->expr()->GetAs<::googlesql::ResolvedAnalyticFunctionCall>();
  return afn != nullptr && afn->function() != nullptr &&
         internal::ResolveFunctionName(afn->function()) == "percentile_disc" &&
         afn->null_handling_modifier() ==
             ::googlesql::ResolvedNonScalarFunctionCallBase::RESPECT_NULLS;
}

bool AppendAnalyticGroupProjections(
    const ::googlesql::ResolvedAnalyticFunctionGroup* group,
    bool input_rn_ordering,
    const std::function<std::string(
        const ::googlesql::ResolvedWindowPartitioning*)>& build_partition,
    const std::function<std::string(
        const ::googlesql::ResolvedWindowOrdering*, bool, bool)>& build_order,
    const std::function<
        void(const ::googlesql::ResolvedAnalyticFunctionGroup*)>& capture_order,
    const std::function<
        std::string(const ::googlesql::ResolvedComputedColumnBase*,
                    absl::string_view,
                    absl::string_view)>& build_projection,
    std::vector<std::string>* projections) {
  if (group == nullptr) return false;
  std::string partition_clause = build_partition(group->partition_by());
  if (partition_clause == kLocalAnalyticBail) return false;
  const bool append_input_rn =
      input_rn_ordering && !internal::AnalyticGroupHasRangeFrame(group) &&
      (internal::AnalyticOrderNeedsInputRn(group) ||
       internal::AnalyticGroupNeedsInputRnForEmptyOrder(group));
  std::string order_clause = build_order(
      group->order_by(), /*bigquery_null_defaults=*/true, append_input_rn);
  if (order_clause == kLocalAnalyticBail) return false;
  capture_order(group);

  for (int f = 0; f < group->analytic_function_list_size(); ++f) {
    const ::googlesql::ResolvedComputedColumnBase* fn_col =
        group->analytic_function_list(f);
    std::string effective_order = order_clause;
    if (PercentileDiscRespectNullsClearsOrder(fn_col)) {
      effective_order.clear();
    }
    std::string projection =
        build_projection(fn_col, partition_clause, effective_order);
    if (projection.empty()) return false;
    projections->push_back(std::move(projection));
  }
  return true;
}

}  // namespace

std::string Transpiler::BuildPartitionClause(
    const ::googlesql::ResolvedWindowPartitioning* p) {
  if (p == nullptr) return "";
  (void)p->hint_list_size();
  if (!p->collation_list().empty()) {
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
  (void)o->hint_list_size();
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
  if (afn == nullptr) return "";
  std::string frame_clause = BuildFrameClause(afn->window_frame());
  if (frame_clause == kAnalyticBail) return "";

  std::string fn_sql;
  if (afn->function() != nullptr &&
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

  const EmitExprFn emit_expr = [this](const ::googlesql::ResolvedExpr* expr) {
    return EmitExpr(expr);
  };
  const EmitColumnRefFn emit_col =
      [this](const ::googlesql::ResolvedColumnRef* ref) {
        return EmitColumnRef(ref);
      };

  bool failed = false;
  CapturePartitionOutputOrder(group->partition_by(),
                              over_has_order,
                              query_output_column_names_,
                              emit_expr,
                              emit_col,
                              &output_order_items_,
                              &output_order_column_ids_,
                              &failed);
  if (failed) return;
  CaptureOrderByOutputOrder(order_by,
                            emit_col,
                            &output_order_items_,
                            &output_order_column_ids_,
                            &failed);
  if (failed) return;

  if (input_rn_ordering_) {
    output_order_items_.push_back(
        absl::StrCat(internal::QuoteIdent(internal::kBqInputRnCol), " ASC"));
    output_order_column_ids_.push_back(-1);
  }
}

std::string Transpiler::EmitAnalyticScan(
    const ::googlesql::ResolvedAnalyticScan* node) {
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

  const EmitExprFn emit_expr = [this](const ::googlesql::ResolvedExpr* expr) {
    return EmitExpr(expr);
  };

  std::optional<std::string> pct_sort_key =
      FindPercentileDiscSortKey(node, emit_expr);
  if (pct_sort_key.has_value()) {
    if (pct_sort_key->empty()) return "";
    input = WrapInputWithPctCoalesce(input, *pct_sort_key);
    if (std::string special =
            TryEmitPercentileDiscOnlyScan(node, input, emit_expr);
        !special.empty()) {
      return special;
    }
  }

  std::vector<std::string> projections;
  const auto build_partition =
      [this](const ::googlesql::ResolvedWindowPartitioning* p) {
        return BuildPartitionClause(p);
      };
  const auto build_order = [this](const ::googlesql::ResolvedWindowOrdering* o,
                                  bool bq_nulls,
                                  bool append_rn) {
    return BuildOrderClause(o, bq_nulls, append_rn);
  };
  const auto capture_order =
      [this](const ::googlesql::ResolvedAnalyticFunctionGroup* group) {
        CaptureAnalyticOutputOrder(group);
      };
  const auto build_projection =
      [this](const ::googlesql::ResolvedComputedColumnBase* col,
             absl::string_view partition,
             absl::string_view order) {
        return BuildAnalyticProjection(col, partition, order);
      };
  for (int g = 0; g < node->function_group_list_size(); ++g) {
    if (!AppendAnalyticGroupProjections(node->function_group_list(g),
                                        input_rn_ordering_,
                                        build_partition,
                                        build_order,
                                        capture_order,
                                        build_projection,
                                        &projections)) {
      return "";
    }
  }

  if (projections.empty()) return "";
  std::string select_list =
      absl::StrCat("*, ", absl::StrJoin(projections, ", "));
  return absl::StrCat("SELECT ", select_list, " FROM (", input, ")");
}

}  // namespace transpiler
}  // namespace duckdb
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
