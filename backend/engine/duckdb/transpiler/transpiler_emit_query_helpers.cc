#include "backend/engine/duckdb/transpiler/transpiler_emit_query_helpers.h"

#include <algorithm>
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

const ::googlesql::ResolvedComputedColumn* FindProjectComputedColumn(
    const ::googlesql::ResolvedProjectScan* node, int column_id) {
  for (int j = 0; j < node->expr_list_size(); ++j) {
    const ::googlesql::ResolvedComputedColumn* cc = node->expr_list(j);
    if (cc != nullptr && cc->column().column_id() == column_id) {
      return cc;
    }
  }
  return nullptr;
}

bool ColumnShadowedByComputed(const ::googlesql::ResolvedProjectScan* node,
                              absl::string_view col_name) {
  for (int j = 0; j < node->expr_list_size(); ++j) {
    const ::googlesql::ResolvedComputedColumn* cc = node->expr_list(j);
    if (cc != nullptr && cc->column().name() == col_name) {
      return true;
    }
  }
  return false;
}

enum class ProjectColumnResult {
  kSkip,
  kFail,
  kOk,
};

ProjectColumnResult EmitProjectScanColumn(
    const ::googlesql::ResolvedProjectScan* node,
    const ::googlesql::ResolvedColumn& col,
    bool input_id_aliases,
    const std::function<
        std::string(const ::googlesql::ResolvedComputedColumn*)>& emit_computed,
    std::string* out) {
  const ::googlesql::ResolvedComputedColumn* match =
      FindProjectComputedColumn(node, col.column_id());
  if (match != nullptr) {
    *out = emit_computed(match);
    return out->empty() ? ProjectColumnResult::kFail : ProjectColumnResult::kOk;
  }
  if (ColumnShadowedByComputed(node, col.name())) {
    return ProjectColumnResult::kSkip;
  }
  if (input_id_aliases) {
    std::string id_alias = internal::JoinColumnIdAlias(col.column_id());
    std::string quoted = internal::QuoteIdent(col.name());
    if (node->expr_list_size() > 0 && id_alias != quoted) {
      *out = absl::StrCat(id_alias, " AS ", quoted);
    } else {
      *out = std::move(id_alias);
    }
  } else {
    *out = internal::QuoteIdent(col.name());
  }
  return ProjectColumnResult::kOk;
}

}  // namespace

void FilterOutputOrderForQueryStmt(const ::googlesql::ResolvedQueryStmt* node,
                                   std::vector<std::string>* output_order_items,
                                   std::vector<int>* output_order_column_ids) {
  if (output_order_items->empty()) return;
  std::vector<std::string> filtered_items;
  std::vector<int> filtered_ids;
  filtered_items.reserve(output_order_items->size());
  filtered_ids.reserve(output_order_items->size());
  for (size_t i = 0; i < output_order_items->size(); ++i) {
    const std::string& item = (*output_order_items)[i];
    if (item.empty() || item[0] != '"') continue;
    const size_t end = item.find('"', 1);
    if (end == std::string::npos) continue;
    const std::string quoted_col = item.substr(0, end + 1);
    if (!internal::OutputListContainsColumn(quoted_col, node)) continue;
    filtered_items.push_back(item);
    filtered_ids.push_back(i < output_order_column_ids->size()
                               ? (*output_order_column_ids)[i]
                               : -1);
  }
  *output_order_items = std::move(filtered_items);
  *output_order_column_ids = std::move(filtered_ids);
}

void AppendQueryOrderByClause(const ::googlesql::ResolvedQueryStmt* node,
                              absl::string_view inner,
                              const std::vector<std::string>& outputs,
                              const std::vector<std::string>& outer_refs,
                              bool join_id_aliases_in_query,
                              bool input_rn_ordering,
                              bool output_includes_input_rn,
                              std::vector<std::string>* output_order_items,
                              std::vector<int>* output_order_column_ids,
                              bool* input_rn_ordering_out,
                              std::string* sql) {
  if (output_order_items->empty()) {
    if (input_rn_ordering) {
      absl::StrAppend(
          sql, " ORDER BY ", internal::QuoteIdent(internal::kBqInputRnCol));
      *input_rn_ordering_out = false;
    }
    return;
  }
  std::vector<std::string> wrap_order_items;
  wrap_order_items.reserve(output_order_items->size());
  for (size_t i = 0; i < output_order_items->size(); ++i) {
    const int col_id = i < output_order_column_ids->size()
                           ? (*output_order_column_ids)[i]
                           : -1;
    wrap_order_items.push_back(internal::RemapOrderItemForJoinAliases(
        (*output_order_items)[i], col_id, join_id_aliases_in_query));
  }
  const std::vector<std::string> extra_order_cols =
      internal::ExtraOrderColumnsForWrap(wrap_order_items,
                                         node,
                                         output_order_column_ids,
                                         join_id_aliases_in_query);
  const bool order_needs_rn =
      input_rn_ordering && !output_includes_input_rn &&
      std::any_of(wrap_order_items.begin(),
                  wrap_order_items.end(),
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
    *sql = absl::StrCat("SELECT ",
                        absl::StrJoin(outer_refs, ", "),
                        " FROM (SELECT ",
                        absl::StrJoin(inner_select, ", "),
                        " FROM (",
                        inner,
                        ") ORDER BY ",
                        absl::StrJoin(wrap_order_items, ", "),
                        ")");
  } else {
    absl::StrAppend(sql, " ORDER BY ", absl::StrJoin(wrap_order_items, ", "));
  }
  output_order_items->clear();
  output_order_column_ids->clear();
  *input_rn_ordering_out = false;
}

bool IsNoOpProjectScan(const ::googlesql::ResolvedProjectScan* node) {
  if (node->expr_list_size() != 0 || node->input_scan() == nullptr ||
      node->input_scan()->column_list_size() != node->column_list_size()) {
    return false;
  }
  for (int i = 0; i < node->column_list_size(); ++i) {
    const int wanted_id = node->column_list(i).column_id();
    bool found = false;
    for (int j = 0; j < node->input_scan()->column_list_size(); ++j) {
      if (node->input_scan()->column_list(j).column_id() == wanted_id) {
        found = true;
        break;
      }
    }
    if (!found) return false;
  }
  return true;
}

bool BuildProjectScanProjections(
    const ::googlesql::ResolvedProjectScan* node,
    bool input_id_aliases,
    const std::function<
        std::string(const ::googlesql::ResolvedComputedColumn*)>& emit_computed,
    std::vector<std::string>* projections) {
  projections->reserve(node->column_list_size());
  for (int i = 0; i < node->column_list_size(); ++i) {
    std::string emitted;
    switch (EmitProjectScanColumn(node,
                                  node->column_list(i),
                                  input_id_aliases,
                                  emit_computed,
                                  &emitted)) {
      case ProjectColumnResult::kSkip:
        continue;
      case ProjectColumnResult::kFail:
        return false;
      case ProjectColumnResult::kOk:
        projections->push_back(std::move(emitted));
        break;
    }
  }
  return true;
}

void AppendProjectScanOrderColumns(
    const std::vector<std::string>& output_order_items,
    const std::vector<int>& output_order_column_ids,
    bool join_id_aliases,
    bool input_id_aliases,
    bool input_has_rn_column,
    bool suppress_rn,
    std::vector<std::string>* projections) {
  for (size_t i = 0; i < output_order_items.size(); ++i) {
    const std::string& item = output_order_items[i];
    const std::string col = internal::OrderItemLeadingColumn(item);
    if (col.empty() || col == internal::QuoteIdent(internal::kBqInputRnCol)) {
      continue;
    }
    const int column_id =
        i < output_order_column_ids.size() ? output_order_column_ids[i] : -1;
    const std::string proj = internal::OrderColumnExprForWrap(
        col, column_id, join_id_aliases || input_id_aliases);
    if (std::find(projections->begin(), projections->end(), proj) !=
        projections->end()) {
      continue;
    }
    projections->push_back(proj);
  }
  if (input_has_rn_column && !suppress_rn) {
    const std::string rn = internal::QuoteIdent(internal::kBqInputRnCol);
    if (std::find(projections->begin(), projections->end(), rn) ==
        projections->end()) {
      projections->push_back(rn);
    }
  }
}

}  // namespace transpiler
}  // namespace duckdb
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
