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

// ---------------------------------------------------------------------------
// Per-shape Emit hooks.
// ---------------------------------------------------------------------------

// Statements ----------------------------------------------------------------

std::string Transpiler::EmitQueryStmt(
    const ::googlesql::ResolvedQueryStmt* node) {
  // Lower the inner `query()` scan and then apply the
  // `output_column_list()` mapping as the final SELECT list, so the
  // user-visible aliases (`SELECT id AS user_id ...`) land on the
  // outermost projection. The inner scan emit already produces a
  // self-contained SELECT, so we wrap it as a derived table and let
  // each `ResolvedOutputColumn` rewrite the column reference into the
  // user-facing name.
  //
  // Value-table queries (`SELECT AS VALUE ...`) collapse the row to
  // a single anonymous value; DuckDB has no direct analog. The
  // route classifier (`backend/engine/coordinator/route_classifier.cc`)
  // promotes any `ResolvedQueryStmt` whose `is_value_table()` flag
  // is set to `kSemanticExecutor` via its
  // `VisitResolvedQueryStmt` override, so the local coordinator
  // hands the statement off to the semantic executor (stub today;
  // owned by `docs/ENGINE_POLICY.md`) before the
  // transpiler is ever asked to lower it. We touch the accessor
  // below so `ResolvedAST::CheckFieldsAccessed` still sees the
  // field read in case the transpiler is invoked through a path
  // that bypasses the classifier (legacy tests, debugging).
  if (node == nullptr) return "";
  (void)node->is_value_table();
  output_order_items_.clear();
  input_has_rn_column_ = false;
  input_rn_ordering_ = false;
  output_includes_input_rn_ = false;
  query_output_column_names_.clear();
  query_output_column_names_.reserve(node->output_column_list_size());
  for (int i = 0; i < node->output_column_list_size(); ++i) {
    const ::googlesql::ResolvedOutputColumn* out = node->output_column_list(i);
    if (out != nullptr) {
      query_output_column_names_.push_back(out->column().name());
    }
  }
  std::string inner = EmitScan(node->query());
  if (inner.empty()) return "";
  std::vector<std::string> outputs;
  outputs.reserve(node->output_column_list_size());
  for (int i = 0; i < node->output_column_list_size(); ++i) {
    std::string oc = EmitOutputColumn(node->output_column_list(i));
    if (oc.empty()) return "";
    outputs.push_back(std::move(oc));
  }
  if (outputs.empty()) return "";
  std::vector<std::string> outer_refs;
  outer_refs.reserve(node->output_column_list_size());
  for (int i = 0; i < node->output_column_list_size(); ++i) {
    const ::googlesql::ResolvedOutputColumn* out = node->output_column_list(i);
    if (out == nullptr) return "";
    outer_refs.push_back(internal::QuoteIdent(out->name()));
  }
  std::string sql = absl::StrCat(
      "SELECT ", absl::StrJoin(outputs, ", "), " FROM (", inner, ")");
  if (!output_order_items_.empty()) {
    const std::vector<std::string> extra_order_cols =
        internal::ExtraOrderColumnsForWrap(output_order_items_, node);
    const bool order_needs_rn =
        input_rn_ordering_ && !output_includes_input_rn_ &&
        std::any_of(output_order_items_.begin(),
                    output_order_items_.end(),
                    [](const std::string& item) {
                      return item.find(internal::QuoteIdent(
                                 internal::kBqInputRnCol)) != std::string::npos;
                    });
    if (!extra_order_cols.empty() || order_needs_rn) {
      std::vector<std::string> inner_select = outputs;
      inner_select.insert(
          inner_select.end(), extra_order_cols.begin(), extra_order_cols.end());
      if (order_needs_rn) {
        inner_select.push_back(internal::QuoteIdent(internal::kBqInputRnCol));
      }
      // Inner ORDER BY subquery already applied output aliases; the outer
      // SELECT must reference those names, not the pre-alias expressions.
      sql = absl::StrCat("SELECT ",
                         absl::StrJoin(outer_refs, ", "),
                         " FROM (SELECT ",
                         absl::StrJoin(inner_select, ", "),
                         " FROM (",
                         inner,
                         ") ORDER BY ",
                         absl::StrJoin(output_order_items_, ", "),
                         ")");
    } else {
      absl::StrAppend(
          &sql, " ORDER BY ", absl::StrJoin(output_order_items_, ", "));
    }
    output_order_items_.clear();
    input_rn_ordering_ = false;
  } else if (input_rn_ordering_) {
    // DuckDB allows ORDER BY a column from the inner subquery even when
    // the outer SELECT does not project it (UNNEST input-order tests).
    absl::StrAppend(
        &sql, " ORDER BY ", internal::QuoteIdent(internal::kBqInputRnCol));
    input_rn_ordering_ = false;
  }
  query_output_column_names_.clear();
  return sql;
}

std::string Transpiler::EmitCtasSelect(
    const ::googlesql::ResolvedCreateTableAsSelectStmt* stmt) {
  if (stmt == nullptr || stmt->query() == nullptr) return "";
  output_order_items_.clear();
  input_has_rn_column_ = false;
  input_rn_ordering_ = false;
  output_includes_input_rn_ = false;
  query_output_column_names_.clear();
  query_output_column_names_.reserve(stmt->column_definition_list_size());
  for (const auto& def : stmt->column_definition_list()) {
    if (def != nullptr) {
      query_output_column_names_.push_back(def->name());
    }
  }
  std::string inner = EmitScan(stmt->query());
  if (inner.empty()) return "";
  std::vector<std::string> outputs;
  outputs.reserve(stmt->column_definition_list_size());
  std::vector<std::string> outer_refs;
  outer_refs.reserve(stmt->column_definition_list_size());
  for (const auto& def : stmt->column_definition_list()) {
    if (def == nullptr) return "";
    auto oc = ::googlesql::MakeResolvedOutputColumn(def->name(), def->column());
    std::string emitted = EmitOutputColumn(oc.get());
    if (emitted.empty()) return "";
    outputs.push_back(std::move(emitted));
    outer_refs.push_back(internal::QuoteIdent(def->name()));
  }
  if (outputs.empty()) return "";
  std::string sql = absl::StrCat(
      "SELECT ", absl::StrJoin(outputs, ", "), " FROM (", inner, ")");
  query_output_column_names_.clear();
  return sql;
}

std::string Transpiler::EmitInsertSelect(
    const ::googlesql::ResolvedInsertStmt* stmt) {
  if (stmt == nullptr || stmt->query() == nullptr) return "";
  output_order_items_.clear();
  input_has_rn_column_ = false;
  input_rn_ordering_ = false;
  output_includes_input_rn_ = false;
  query_output_column_names_.clear();
  query_output_column_names_.reserve(stmt->query_output_column_list_size());
  for (int i = 0; i < stmt->query_output_column_list_size(); ++i) {
    query_output_column_names_.push_back(
        stmt->query_output_column_list(i).name());
  }
  std::string inner = EmitScan(stmt->query());
  if (inner.empty()) return "";
  std::vector<std::string> outputs;
  outputs.reserve(stmt->query_output_column_list_size());
  for (int i = 0; i < stmt->query_output_column_list_size(); ++i) {
    const ::googlesql::ResolvedColumn& col = stmt->query_output_column_list(i);
    auto oc = ::googlesql::MakeResolvedOutputColumn(col.name(), col);
    std::string emitted = EmitOutputColumn(oc.get());
    if (emitted.empty()) return "";
    outputs.push_back(std::move(emitted));
  }
  if (outputs.empty()) return "";
  std::string sql = absl::StrCat(
      "SELECT ", absl::StrJoin(outputs, ", "), " FROM (", inner, ")");
  if (!output_order_items_.empty()) {
    absl::StrAppend(
        &sql, " ORDER BY ", absl::StrJoin(output_order_items_, ", "));
    output_order_items_.clear();
    input_rn_ordering_ = false;
  } else if (input_rn_ordering_) {
    absl::StrAppend(
        &sql, " ORDER BY ", internal::QuoteIdent(internal::kBqInputRnCol));
    input_rn_ordering_ = false;
  }
  query_output_column_names_.clear();
  return sql;
}

// Scans ---------------------------------------------------------------------

std::string Transpiler::EmitProjectScan(
    const ::googlesql::ResolvedProjectScan* node) {
  // `SELECT <projections> FROM (<input>)`. The output `column_list`
  // is the schema the upstream scan sees; each column either lives in
  // `expr_list` (a `ResolvedComputedColumn` with the bound expression)
  // or passes through from `input_scan`. Computed columns lower as
  // `<expr> AS "<column-name>"`; pass-through columns reference the
  // input column by name (the inner scan emits each input column with
  // its `ResolvedColumn::name()` already).
  //
  // No-op elision: when `expr_list` is empty AND `column_list` is a
  // permutation of `input_scan->column_list` by column id, this
  // ProjectScan is doing nothing the wrapping scan / QueryStmt cannot
  // do on its own outermost SELECT (column reordering / aliasing
  // happens by name there). Returning the inner emit directly
  // strips a redundant `SELECT * FROM (SELECT * FROM ...)` layer that
  // otherwise stacks on top of every analyzer-introduced
  // ProjectScan-over-TableScan pair, e.g. for `SELECT id FROM
  // people` the analyzer always inserts a no-op ProjectScan even
  // though `output_column_list` already references the TableScan
  // columns. We keep the wrap when ProjectScan narrows the column
  // list (strict subset) or when any computed expression lives on
  // it, so semantic-bearing projections are unaffected.
  //
  // The empty-string contract: if any sub-emit returns "" (input scan
  // we cannot lower, or a computed expression outside the function /
  // literal whitelist), we propagate "" so the engine takes the
  // reference-impl fallback for the whole query rather than emitting
  // partial SQL.
  if (node == nullptr) return "";

  if (node->expr_list_size() == 0 && node->input_scan() != nullptr &&
      node->input_scan()->column_list_size() == node->column_list_size()) {
    bool same_set = true;
    for (int i = 0; i < node->column_list_size(); ++i) {
      const int wanted_id = node->column_list(i).column_id();
      bool found = false;
      for (int j = 0; j < node->input_scan()->column_list_size(); ++j) {
        if (node->input_scan()->column_list(j).column_id() == wanted_id) {
          found = true;
          break;
        }
      }
      if (!found) {
        same_set = false;
        break;
      }
    }
    if (same_set && !suppress_rn_in_project_) {
      return EmitScan(node->input_scan());
    }
  }

  std::string input = EmitScan(node->input_scan());
  if (input.empty()) return "";
  const bool input_id_aliases = join_output_uses_id_aliases_;
  // Keep join id aliases visible to EmitComputedColumn / EmitExpr while
  // lowering expressions that reference join-renamed columns (e.g.
  // COALESCE(CAST(u.id AS STRING), ...) over a FULL OUTER JOIN).
  if (!input_id_aliases) {
    join_output_uses_id_aliases_ = false;
  }

  std::vector<std::string> projections;
  projections.reserve(node->column_list_size());
  for (int i = 0; i < node->column_list_size(); ++i) {
    const ::googlesql::ResolvedColumn& col = node->column_list(i);
    const ::googlesql::ResolvedComputedColumn* match = nullptr;
    for (int j = 0; j < node->expr_list_size(); ++j) {
      const ::googlesql::ResolvedComputedColumn* cc = node->expr_list(j);
      if (cc != nullptr && cc->column().column_id() == col.column_id()) {
        match = cc;
        break;
      }
    }
    if (match != nullptr) {
      std::string emitted = EmitComputedColumn(match);
      if (emitted.empty()) return "";
      projections.push_back(std::move(emitted));
    } else {
      // The analyzer may list both a passthrough ResolvedColumn and a
      // computed replacement that share the same user-visible name
      // (e.g. FORMAT_TIMESTAMP AS finish_time over the input finish_time
      // column). Emitting both yields duplicate SELECT aliases and can
      // crash DuckDB on window queries.
      bool shadowed_by_computed = false;
      for (int j = 0; j < node->expr_list_size(); ++j) {
        const ::googlesql::ResolvedComputedColumn* cc = node->expr_list(j);
        if (cc != nullptr && cc->column().name() == col.name()) {
          shadowed_by_computed = true;
          break;
        }
      }
      if (shadowed_by_computed) continue;
      if (input_id_aliases) {
        std::string id_alias = internal::JoinColumnIdAlias(col.column_id());
        std::string quoted = internal::QuoteIdent(col.name());
        // Mixed passthrough + computed projections need stable
        // `ResolvedColumn::name()` aliases so set-op / recursive-CTE
        // arms can reference the child output by name (see
        // `EmitRecursiveScan`'s `output_column_list` contract).
        if (node->expr_list_size() > 0 && id_alias != quoted) {
          projections.push_back(absl::StrCat(id_alias, " AS ", quoted));
        } else {
          projections.push_back(std::move(id_alias));
        }
      } else {
        projections.push_back(internal::QuoteIdent(col.name()));
      }
    }
  }

  if (input_id_aliases) {
    // Computed projections replace join column aliases with new output
    // names; downstream OrderByScan / QueryStmt must reference those names.
    join_output_uses_id_aliases_ = node->expr_list_size() == 0;
  }

  for (const std::string& item : output_order_items_) {
    const std::string col = internal::OrderItemLeadingColumn(item);
    if (col.empty() || col == internal::QuoteIdent(internal::kBqInputRnCol))
      continue;
    if (std::find(projections.begin(), projections.end(), col) !=
        projections.end()) {
      continue;
    }
    projections.push_back(col);
  }
  if (input_has_rn_column_ && !suppress_rn_in_project_) {
    const std::string rn = internal::QuoteIdent(internal::kBqInputRnCol);
    if (std::find(projections.begin(), projections.end(), rn) ==
        projections.end()) {
      projections.push_back(rn);
    }
  }
  if (input_rn_ordering_) {
    output_includes_input_rn_ = true;
  }
  std::string select_list =
      projections.empty() ? "*" : absl::StrJoin(projections, ", ");
  return absl::StrCat("SELECT ", select_list, " FROM (", input, ")");
}

std::string Transpiler::EmitTableScan(
    const ::googlesql::ResolvedTableScan* node) {
  // Emit a self-contained SELECT so the result composes as a derived
  // table for any outer scan (FilterScan, ProjectScan, JoinScan, ...)
  // that wraps it. Each `column_list` entry pulls one column from the
  // underlying `Table` via the position recorded in `column_index_list`. The
  // DuckDB-side table name is whatever the catalog's `Table::Name()` returned
  // -- the engine is responsible for ATTACHing storage so the bare name
  // resolves at execution time.
  if (node == nullptr || node->table() == nullptr) return "";
  const ::googlesql::Table* table = node->table();
  std::vector<std::string> projections;
  projections.reserve(node->column_list_size());
  for (int i = 0; i < node->column_list_size(); ++i) {
    const ::googlesql::ResolvedColumn& out = node->column_list(i);
    // `column_index_list` is the canonical mapping from
    // `column_list[i]` to a position in `table->GetColumn(idx)`. The
    // header on `ResolvedTableScan` requires it to be set 1:1 with
    // `column_list` for any modern client -- the older
    // name-matching path is documented as a violation of the
    // ResolvedColumn contract, so we don't fall back to it.
    if (i >= node->column_index_list_size()) return "";
    int src_idx = node->column_index_list(i);
    const ::googlesql::Column* src = table->GetColumn(src_idx);
    if (src == nullptr) return "";
    std::string src_name = src->Name();
    std::string out_name = out.name();
    if (src_name == out_name) {
      // Skip the AS alias when both names already match -- keeps the
      // emitted SQL readable for the common case where the analyzer
      // didn't have to disambiguate (a single-table SELECT *).
      projections.push_back(internal::QuoteIdent(src_name));
    } else {
      projections.push_back(absl::StrCat(internal::QuoteIdent(src_name),
                                         " AS ",
                                         internal::QuoteIdent(out_name)));
    }
  }
  std::string select_list =
      projections.empty() ? "*" : absl::StrJoin(projections, ", ");
  return absl::StrCat(
      "SELECT ", select_list, " FROM ", internal::QuoteIdent(table->Name()));
}

std::string Transpiler::EmitSingleRowScan(
    const ::googlesql::ResolvedSingleRowScan* node) {
  // The analyzer represents "no FROM clause" (`SELECT 1`,
  // `SELECT 'hi'`, ...) as a `ResolvedSingleRowScan`: a relation with
  // exactly one row and no columns. DuckDB has no first-class
  // single-row table, but a self-contained `SELECT 1` produces the
  // same shape -- one row, with a synthetic column the surrounding
  // `EmitProjectScan` wrap discards. Emitting it as a derived table
  // keeps the composition contract every other scan emit follows
  // (each scan returns a self-contained `SELECT` so a wrapping scan
  // can splice it into `FROM (<inner>)` without re-emitting).
  if (node == nullptr) return "";
  return "SELECT 1";
}

std::string Transpiler::EmitFilterScan(
    const ::googlesql::ResolvedFilterScan* node) {
  // Wrap the input scan as a derived table so the WHERE clause sees
  // exactly the column names the input emitted. DuckDB supports
  // unparenthesized table references in `FROM`, but the derived-table
  // shape is robust across the scan shapes we already know about (a
  // table scan emits its own `SELECT`, so we can't strip it back to
  // a bare relation here without re-emitting). If either child emits
  // "" (still on the disposition fallback) we propagate the empty
  // string up so the engine surfaces UNIMPLEMENTED.
  if (node == nullptr) return "";
  std::string input = EmitScan(node->input_scan());
  if (input.empty()) return "";
  std::string filter = EmitExpr(node->filter_expr());
  if (filter.empty()) return "";
  return absl::StrCat("SELECT * FROM (", input, ") WHERE ", filter);
}

}  // namespace transpiler
}  // namespace duckdb
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
