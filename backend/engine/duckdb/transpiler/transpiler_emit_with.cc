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

std::string Transpiler::EmitSampleScan(
    const ::googlesql::ResolvedSampleScan* node) {
  // BigQuery `TABLESAMPLE <method> (<n> PERCENT)` lowers to a
  // `ResolvedSampleScan` whose `method()` carries the user-spelled
  // method (`SYSTEM` for BQ surface SQL), `size()` is the size
  // expression (literal or parameter), and `unit()` is PERCENT or
  // ROWS. DuckDB spells the equivalent shape as:
  //
  //   SELECT * FROM <input> USING SAMPLE <size> PERCENT (<method>)
  //   SELECT * FROM <input> USING SAMPLE <size> ROWS    (<method>)
  //
  // DuckDB ships three sampling methods (see
  // https://duckdb.org/docs/sql/samples):
  //
  //   * `system`    -- coarse-grained block sampling; PERCENT only.
  //                    Matches BigQuery's `TABLESAMPLE SYSTEM` shape.
  //   * `bernoulli` -- per-row sampling; PERCENT only.
  //                    The independent-Bernoulli semantics match the
  //                    standard SQL definition GoogleSQL surfaces.
  //   * `reservoir` -- fixed-row sampling; ROWS only.
  //                    Matches the "exactly N rows" target.
  //
  // We bail (return "") on anything outside that matrix so the
  // engine surfaces UNIMPLEMENTED rather than emitting SQL with a
  // method/unit combination DuckDB rejects at parse time. The plan
  // also asks us to bail on:
  //
  //   * `weight_column()` -- DuckDB has no `WITH WEIGHT` analog.
  //   * `partition_by_list()` -- BigQuery's STRATIFY BY surface; no
  //     DuckDB analog.
  if (node == nullptr || node->input_scan() == nullptr) return "";
  if (node->weight_column() != nullptr) return "";
  if (node->partition_by_list_size() > 0) return "";

  std::string method_lower = absl::AsciiStrToLower(node->method());
  const bool is_system = (method_lower == "system");
  const bool is_bernoulli = (method_lower == "bernoulli");
  const bool is_reservoir = (method_lower == "reservoir");
  if (!is_system && !is_bernoulli && !is_reservoir) return "";

  std::string input = EmitScan(node->input_scan());
  if (input.empty()) return "";
  if (node->size() == nullptr) return "";
  std::string size = EmitExpr(node->size());
  if (size.empty()) return "";

  std::string sample_amount;
  switch (node->unit()) {
    case ::googlesql::ResolvedSampleScan::PERCENT:
      // SYSTEM and BERNOULLI consume PERCENT. RESERVOIR is rows-only.
      if (is_reservoir) return "";
      sample_amount = absl::StrCat(size, " PERCENT");
      break;
    case ::googlesql::ResolvedSampleScan::ROWS:
      // RESERVOIR is the only DuckDB method that consumes ROWS.
      if (!is_reservoir) return "";
      sample_amount = absl::StrCat(size, " ROWS");
      break;
    default:
      return "";
  }
  std::string sample_suffix = absl::StrCat(" (", method_lower, ")");
  if (node->repeatable_argument() != nullptr) {
    std::string seed = EmitExpr(node->repeatable_argument());
    if (seed.empty()) return "";
    sample_suffix = absl::StrCat(" (", method_lower, ", ", seed, ")");
  }
  return absl::StrCat("SELECT * FROM (",
                      input,
                      ") USING SAMPLE ",
                      sample_amount,
                      sample_suffix);
}

// Synthesize a stable positional column name for the i-th column
// of a `ResolvedWithScan` CTE entry. The two-sided contract:
//
//   * `EmitWithScan` projects each CTE entry's inner SELECT to
//     `<inner_name> AS "_cte_<idx>"`.
//   * `EmitWithRefScan` reads those positional names back out and
//     aliases each to its own `column_list(i).name()`.
//
// Going through a fixed positional alias decouples the analyzer's
// per-CTE column-name choice (which may dedupe to e.g. `id#3`)
// from the per-reference column names (which the analyzer assigns
// fresh on each `ResolvedWithRefScan`). Both sides agree on the
// `_cte_<idx>` names regardless of what the analyzer chose, so a
// CTE referenced multiple times resolves cleanly without a
// name-collision rewrite.
//
// The leading underscore + `cte_` prefix keeps the synthesized
// name out of the BigQuery user-name namespace (BQ column names
// cannot start with `_cte_`-style internal prefixes in user SQL,
// and even if they did, the analyzer's name dedup would not pick
// the same form).
static std::string WithScanColumnAnchor(int idx) {
  return absl::StrCat("_cte_", idx);
}

std::string Transpiler::EmitWithScan(
    const ::googlesql::ResolvedWithScan* node) {
  // BigQuery `WITH a AS (<sub_a>), b AS (<sub_b>) <query>` lowers to
  // DuckDB `WITH "a" AS (<sub_a_sql>), "b" AS (<sub_b_sql>) <query_sql>`.
  // Both engines share the standard non-recursive CTE form, so the
  // emit is mostly bookkeeping: lower each `with_entry_list` entry's
  // subquery and the body, splice them into the CTE syntax.
  //
  // Column-name remapping: the analyzer assigns fresh
  // `ResolvedColumn` ids on each `ResolvedWithRefScan` whose
  // `name()`s do NOT necessarily match the CTE entry's own
  // `column_list()` names (the analyzer dedupes names across the
  // tree). To keep the two sides aligned without a per-ref-scan
  // name-collision rewrite, we project each CTE body to a stable
  // positional anchor name (`_cte_<idx>`) and let
  // `EmitWithRefScan` rename the anchor back to its own per-ref
  // names. The anchor name lives in `WithScanColumnAnchor` so the
  // two emit hooks share the convention.
  //
  // Recursive CTEs (`WITH RECURSIVE`) lower through DuckDB's
  // `WITH RECURSIVE` keyword. Each recursive entry's
  // `with_subquery()` is a `ResolvedRecursiveScan`; the lowering
  // routes through `EmitRecursiveScan` which sets up the per-CTE
  // anchor names and emits the anchor + UNION + recursive term
  // body. Non-recursive entries inside a recursive WithScan still
  // emit through the standard CTE rewriter.
  if (node == nullptr || node->query() == nullptr) return "";
  if (node->with_entry_list_size() == 0) {
    return EmitScan(node->query());
  }
  const bool recursive = node->recursive();
  const bool saved_rn_at_with = input_rn_ordering_;
  input_rn_ordering_ = false;
  const bool body_needs_input_rn =
      internal::ScanTreeContainsAnalytic(node->query());
  const std::vector<std::string> saved_output_order = output_order_items_;
  const std::vector<int> saved_output_order_ids = output_order_column_ids_;
  bool any_cte_has_rn = false;
  std::vector<std::string> ctes;
  ctes.reserve(node->with_entry_list_size());
  for (int i = 0; i < node->with_entry_list_size(); ++i) {
    const bool saved_rn_in_cte = input_rn_ordering_;
    input_rn_ordering_ = false;
    output_order_items_.clear();
    output_order_column_ids_.clear();
    const ::googlesql::ResolvedWithEntry* entry = node->with_entry_list(i);
    if (entry == nullptr || entry->with_subquery() == nullptr) return "";
    if (entry->with_query_name().empty()) return "";
    const ::googlesql::ResolvedScan* sub_scan = entry->with_subquery();
    const bool entry_is_recursive =
        sub_scan->node_kind() == ::googlesql::RESOLVED_RECURSIVE_SCAN;
    if (entry_is_recursive) {
      // Stage the per-CTE context (name + anchor column names) so
      // any `ResolvedRecursiveRefScan` reached from the recursive
      // term's scan walks emits with the right CTE name and
      // anchor-to-ref rename.
      const auto* rec_scan =
          sub_scan->GetAs<::googlesql::ResolvedRecursiveScan>();
      std::vector<std::string> anchor_names;
      anchor_names.reserve(rec_scan->column_list_size());
      for (int j = 0; j < rec_scan->column_list_size(); ++j) {
        anchor_names.push_back(WithScanColumnAnchor(j));
      }
      recursive_cte_stack_.push_back({entry->with_query_name(), anchor_names});
      std::string body_sql = EmitRecursiveScan(rec_scan);
      recursive_cte_stack_.pop_back();
      if (body_sql.empty()) return "";
      // DuckDB's WITH RECURSIVE column list lives between the CTE
      // name and the `AS (...)`; emit it explicitly so the DuckDB
      // planner does not have to infer names from the anchor's
      // first SELECT (which could differ from the analyzer's
      // expected names).
      std::vector<std::string> quoted_cols;
      quoted_cols.reserve(anchor_names.size());
      for (const std::string& name : anchor_names) {
        quoted_cols.push_back(internal::QuoteIdent(name));
      }
      std::string cols_clause =
          quoted_cols.empty()
              ? std::string()
              : absl::StrCat("(", absl::StrJoin(quoted_cols, ", "), ")");
      ctes.push_back(
          absl::StrCat(internal::QuoteIdent(entry->with_query_name()),
                       cols_clause,
                       " AS (",
                       body_sql,
                       ")"));
    } else {
      bool cte_has_rn = false;
      std::string sub = EmitScan(sub_scan);
      if (sub.empty()) return "";
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
        projected = absl::StrCat(
            "SELECT ", absl::StrJoin(cols, ", "), " FROM (", sub, ")");
      }
      if (cte_has_rn && !cols.empty()) {
        projected = absl::StrCat(projected.substr(0, projected.find(" FROM ")),
                                 ", ",
                                 internal::QuoteIdent(internal::kBqInputRnCol),
                                 projected.substr(projected.find(" FROM ")));
      }
      ctes.push_back(
          absl::StrCat(internal::QuoteIdent(entry->with_query_name()),
                       " AS (",
                       projected,
                       ")"));
      any_cte_has_rn = any_cte_has_rn || cte_has_rn;
    }
    input_rn_ordering_ = saved_rn_in_cte;
  }
  input_rn_ordering_ = any_cte_has_rn;
  output_order_items_ = saved_output_order;
  output_order_column_ids_ = saved_output_order_ids;
  std::string body = EmitScan(node->query());
  input_rn_ordering_ = saved_rn_at_with || input_rn_ordering_;
  if (body.empty()) return "";
  const char* keyword = recursive ? "WITH RECURSIVE " : "WITH ";
  return absl::StrCat(keyword, absl::StrJoin(ctes, ", "), " ", body);
}

std::string Transpiler::EmitRecursiveScan(
    const ::googlesql::ResolvedRecursiveScan* node) {
  // `EmitWithScan` is the only caller; it has already staged the
  // `recursive_cte_stack_` entry. The anchor names live there too
  // -- we re-read them rather than recomputing so the contract that
  // `EmitRecursiveRefScan` and the anchor projection use the same
  // names is single-sourced.
  if (node == nullptr) return "";
  if (node->non_recursive_term() == nullptr ||
      node->recursive_term() == nullptr) {
    return "";
  }
  if (recursive_cte_stack_.empty()) return "";
  const RecursiveCteContext& ctx = recursive_cte_stack_.back();
  if (static_cast<int>(ctx.column_names.size()) != node->column_list_size()) {
    return "";
  }
  const ::googlesql::ResolvedRecursionDepthModifier* depth_mod =
      node->recursion_depth_modifier();
  int depth_col_idx = -1;
  if (depth_mod != nullptr && depth_mod->recursion_depth_column() != nullptr) {
    const int depth_id =
        depth_mod->recursion_depth_column()->column().column_id();
    for (int j = 0; j < node->column_list_size(); ++j) {
      if (node->column_list(j).column_id() == depth_id) {
        depth_col_idx = j;
        break;
      }
    }
    if (depth_col_idx < 0) return "";
  }

  // Build one recursive-CTE arm. Each item's `output_column_list[j]`
  // names the recursive scan's `column_list(j)` CTE column; the
  // child scan may still emit the analyzer's internal `$colN` aliases.
  // Resolve the child emit name (by `column_id`, then position) before
  // projecting onto the CTE schema name, then rename those CTE columns
  // onto the stable anchor names (`_cte_<idx>`) DuckDB's
  // `WITH RECURSIVE` column list expects.
  auto child_emit_column_name = [](const ::googlesql::ResolvedScan* child,
                                   const ::googlesql::ResolvedColumn& out_col,
                                   int index) -> std::string {
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
  };
  auto build_arm = [&](const ::googlesql::ResolvedSetOperationItem* item,
                       bool is_recursive_arm) -> std::string {
    if (item == nullptr || item->scan() == nullptr) return "";
    if (item->output_column_list_size() != node->column_list_size()) {
      return "";
    }
    const ::googlesql::ResolvedScan* child = item->scan();
    std::string inner = EmitScan(child);
    if (inner.empty()) return "";
    std::vector<std::string> normalized;
    normalized.reserve(node->column_list_size());
    for (int j = 0; j < node->column_list_size(); ++j) {
      if (j == depth_col_idx && !is_recursive_arm) {
        continue;
      }
      const ::googlesql::ResolvedColumn& out_col = item->output_column_list(j);
      const ::googlesql::ResolvedColumn& cte_col = node->column_list(j);
      const std::string src_name = child_emit_column_name(child, out_col, j);
      std::string src_q = internal::QuoteIdent(src_name);
      if (src_name == cte_col.name()) {
        normalized.push_back(std::move(src_q));
      } else {
        normalized.push_back(
            absl::StrCat(src_q, " AS ", internal::QuoteIdent(cte_col.name())));
      }
    }
    std::string normalized_sql = absl::StrCat(
        "SELECT ", absl::StrJoin(normalized, ", "), " FROM (", inner, ")");
    std::vector<std::string> anchor_projs;
    anchor_projs.reserve(node->column_list_size());
    for (int j = 0; j < node->column_list_size(); ++j) {
      const ::googlesql::ResolvedColumn& cte_col = node->column_list(j);
      std::string dst_q = internal::QuoteIdent(ctx.column_names[j]);
      if (j == depth_col_idx) {
        std::string depth_expr;
        if (is_recursive_arm) {
          depth_expr =
              absl::StrCat(internal::QuoteIdent(cte_col.name()), " + 1");
        } else if (depth_mod != nullptr &&
                   depth_mod->lower_bound() != nullptr) {
          depth_expr = EmitExpr(depth_mod->lower_bound());
          if (depth_expr.empty()) return "";
        } else {
          depth_expr = "0";
        }
        anchor_projs.push_back(absl::StrCat(depth_expr, " AS ", dst_q));
        continue;
      }
      std::string src_q = internal::QuoteIdent(cte_col.name());
      anchor_projs.push_back(cte_col.name() == ctx.column_names[j]
                                 ? src_q
                                 : absl::StrCat(src_q, " AS ", dst_q));
    }
    return absl::StrCat("SELECT ",
                        absl::StrJoin(anchor_projs, ", "),
                        " FROM (",
                        normalized_sql,
                        ")");
  };

  std::string anchor = build_arm(node->non_recursive_term(),
                                 /*is_recursive_arm=*/false);
  if (anchor.empty()) return "";
  std::string recursive_arm = build_arm(node->recursive_term(),
                                        /*is_recursive_arm=*/true);
  if (recursive_arm.empty()) return "";

  absl::string_view op;
  switch (node->op_type()) {
    case ::googlesql::ResolvedRecursiveScan::UNION_ALL:
      op = " UNION ALL ";
      break;
    case ::googlesql::ResolvedRecursiveScan::UNION_DISTINCT:
      op = " UNION ";
      break;
    default:
      return "";
  }
  return absl::StrCat(anchor, op, recursive_arm);
}

std::string Transpiler::EmitRecursiveRefScan(
    const ::googlesql::ResolvedRecursiveRefScan* node) {
  // The recursive ref scan references its enclosing recursive CTE
  // by position (the analyzer does not carry the CTE name on the
  // ref). `EmitWithScan` pushed the CTE name + anchor column names
  // onto `recursive_cte_stack_` before emitting the recursive
  // term, so the back of the stack is the right context.
  if (node == nullptr) return "";
  if (recursive_cte_stack_.empty()) return "";
  const RecursiveCteContext& ctx = recursive_cte_stack_.back();
  if (static_cast<int>(ctx.column_names.size()) != node->column_list_size()) {
    return "";
  }
  std::vector<std::string> projs;
  projs.reserve(node->column_list_size());
  for (int i = 0; i < node->column_list_size(); ++i) {
    std::string src_q = internal::QuoteIdent(ctx.column_names[i]);
    std::string dst_q = internal::QuoteIdent(node->column_list(i).name());
    projs.push_back(src_q == dst_q ? src_q
                                   : absl::StrCat(src_q, " AS ", dst_q));
  }
  return absl::StrCat("SELECT ",
                      absl::StrJoin(projs, ", "),
                      " FROM ",
                      internal::QuoteIdent(ctx.cte_name));
}

std::string Transpiler::EmitWithRefScan(
    const ::googlesql::ResolvedWithRefScan* node) {
  // A `ResolvedWithRefScan` references a CTE bound earlier in the
  // surrounding `ResolvedWithScan`. The analyzer exposes the CTE
  // name through `with_query_name()` and the per-reference column
  // names through the ref scan's own `column_list()`. We rename
  // the CTE-side positional anchors (`_cte_<idx>`, see
  // `WithScanColumnAnchor`) back to the ref's per-column names so
  // any wrapping `ResolvedProjectScan` / `ResolvedFilterScan` /
  // ... resolves its `ResolvedColumnRef`s by the names the
  // analyzer expects.
  //
  // The result is a self-contained SELECT, matching every other
  // scan emit's compose-as-derived-table contract.
  if (node == nullptr) return "";
  if (node->with_query_name().empty()) return "";
  if (node->column_list_size() == 0) {
    return absl::StrCat("SELECT * FROM ",
                        internal::QuoteIdent(node->with_query_name()));
  }
  std::vector<std::string> cols;
  cols.reserve(node->column_list_size());
  for (int i = 0; i < node->column_list_size(); ++i) {
    cols.push_back(
        absl::StrCat(internal::QuoteIdent(WithScanColumnAnchor(i)),
                     " AS ",
                     internal::QuoteIdent(node->column_list(i).name())));
  }
  if (input_rn_ordering_) {
    cols.push_back(internal::QuoteIdent(internal::kBqInputRnCol));
  }
  return absl::StrCat("SELECT ",
                      absl::StrJoin(cols, ", "),
                      " FROM ",
                      internal::QuoteIdent(node->with_query_name()));
}

}  // namespace transpiler
}  // namespace duckdb
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
