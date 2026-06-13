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

// The analyzer canonicalizes `JOIN ... USING (a, b, ...)` into a
// `join_expr` tree of `$equal` / `$and` calls over matching column
// refs. DuckDB shares the native `USING (...)` syntax, so we peel
// the column names back out of that tree and emit `USING` directly
// rather than lowering through `$equal` (which is owned by
// `docs/ENGINE_POLICY.md`).
static bool TryAppendUsingColumnFromEqualCall(
    const ::googlesql::ResolvedFunctionCall* call,
    std::vector<std::string>* cols) {
  if (call == nullptr || call->function() == nullptr || cols == nullptr) {
    return false;
  }
  if (call->argument_list_size() != 2) return false;
  if (absl::AsciiStrToLower(call->function()->Name()) != "$equal") {
    return false;
  }
  const ::googlesql::ResolvedExpr* lhs = call->argument_list(0);
  const ::googlesql::ResolvedExpr* rhs = call->argument_list(1);
  if (lhs == nullptr || rhs == nullptr ||
      lhs->node_kind() != ::googlesql::RESOLVED_COLUMN_REF ||
      rhs->node_kind() != ::googlesql::RESOLVED_COLUMN_REF) {
    return false;
  }
  const std::string& left_name =
      lhs->GetAs<::googlesql::ResolvedColumnRef>()->column().name();
  const std::string& right_name =
      rhs->GetAs<::googlesql::ResolvedColumnRef>()->column().name();
  if (left_name != right_name) return false;
  cols->push_back(left_name);
  return true;
}

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

static std::string EmitJoinQualifiedColumnRef(
    const ::googlesql::ResolvedColumnRef* node,
    const absl::flat_hash_set<int>& left_ids,
    const absl::flat_hash_set<int>& right_ids,
    bool left_id_aliases,
    bool right_id_aliases) {
  if (node == nullptr) return "";
  const int col_id = node->column().column_id();
  const std::string quoted = internal::QuoteIdent(node->column().name());
  if (left_ids.contains(col_id)) {
    if (left_id_aliases) {
      return absl::StrCat("__bq_l.", internal::JoinColumnIdAlias(col_id));
    }
    return absl::StrCat("__bq_l.", quoted);
  }
  if (right_ids.contains(col_id)) {
    if (right_id_aliases) {
      return absl::StrCat("__bq_r.", internal::JoinColumnIdAlias(col_id));
    }
    return absl::StrCat("__bq_r.", quoted);
  }
  return quoted;
}

static void AppendJoinInternalColumns(bool left_sql_has_rn,
                                      bool right_sql_has_rn,
                                      std::vector<std::string>* projections) {
  if (projections == nullptr) return;
  const std::string quoted = internal::QuoteIdent(internal::kBqInputRnCol);
  for (const std::string& projection : *projections) {
    if (projection.find(quoted) != std::string::npos) return;
  }
  const char* side = nullptr;
  if (left_sql_has_rn) {
    side = "__bq_l";
  } else if (right_sql_has_rn) {
    side = "__bq_r";
  } else {
    return;
  }
  projections->push_back(
      absl::StrCat(side,
                   ".",
                   quoted,
                   " AS ",
                   internal::QuoteIdent(internal::kBqInputRnCol)));
}

static std::string EmitJoinOutputProjections(
    const ::googlesql::ResolvedJoinScan* node,
    const absl::flat_hash_set<int>& left_ids,
    const absl::flat_hash_set<int>& right_ids,
    bool left_id_aliases,
    bool right_id_aliases,
    bool left_sql_has_rn,
    bool right_sql_has_rn) {
  if (node == nullptr) return "";
  std::vector<std::string> projections;
  projections.reserve(node->column_list_size() + 1);
  for (int i = 0; i < node->column_list_size(); ++i) {
    const ::googlesql::ResolvedColumn& col = node->column_list(i);
    const int col_id = col.column_id();
    const std::string quoted = internal::QuoteIdent(col.name());
    std::string ref;
    if (left_ids.contains(col_id)) {
      ref = left_id_aliases
                ? absl::StrCat("__bq_l.", internal::JoinColumnIdAlias(col_id))
                : absl::StrCat("__bq_l.", quoted);
    } else if (right_ids.contains(col_id)) {
      ref = right_id_aliases
                ? absl::StrCat("__bq_r.", internal::JoinColumnIdAlias(col_id))
                : absl::StrCat("__bq_r.", quoted);
    } else {
      return "";
    }
    projections.push_back(
        absl::StrCat(ref, " AS ", internal::JoinColumnIdAlias(col_id)));
  }
  AppendJoinInternalColumns(left_sql_has_rn, right_sql_has_rn, &projections);
  if (projections.empty()) return "";
  return absl::StrJoin(projections, ", ");
}

static std::string EmitJoinQualifiedExpr(
    const ::googlesql::ResolvedExpr* expr,
    const absl::flat_hash_set<int>& left_ids,
    const absl::flat_hash_set<int>& right_ids,
    bool left_id_aliases,
    bool right_id_aliases) {
  if (expr == nullptr) return "";
  if (expr->node_kind() == ::googlesql::RESOLVED_COLUMN_REF) {
    return EmitJoinQualifiedColumnRef(
        expr->GetAs<::googlesql::ResolvedColumnRef>(),
        left_ids,
        right_ids,
        left_id_aliases,
        right_id_aliases);
  }
  if (expr->node_kind() == ::googlesql::RESOLVED_FUNCTION_CALL) {
    const auto* call = expr->GetAs<::googlesql::ResolvedFunctionCall>();
    if (call == nullptr || call->function() == nullptr) return "";
    const std::string fn = absl::AsciiStrToLower(call->function()->Name());
    std::vector<std::string> args;
    args.reserve(call->argument_list_size());
    for (int i = 0; i < call->argument_list_size(); ++i) {
      std::string arg = EmitJoinQualifiedExpr(call->argument_list(i),
                                              left_ids,
                                              right_ids,
                                              left_id_aliases,
                                              right_id_aliases);
      if (arg.empty()) return "";
      args.push_back(std::move(arg));
    }
    if (fn == "$equal" && args.size() == 2) {
      return absl::StrCat("(", args[0], " = ", args[1], ")");
    }
    if (fn == "$and" && !args.empty()) {
      return absl::StrCat("(", absl::StrJoin(args, " AND "), ")");
    }
    // bigframes join ordering keys use COALESCE(lhs, sentinel) =
    // COALESCE(rhs, sentinel) in LEFT OUTER JOIN ON clauses.
    if (fn == "coalesce" && args.size() >= 2) {
      return absl::StrCat("COALESCE(", absl::StrJoin(args, ", "), ")");
    }
    return "";
  }
  if (expr->node_kind() == ::googlesql::RESOLVED_LITERAL) {
    const auto* lit = expr->GetAs<::googlesql::ResolvedLiteral>();
    if (lit == nullptr) return "";
    return internal::EmitValueLiteral(lit->value());
  }
  return "";
}

static bool ExtractUsingColumnsFromJoinExpr(
    const ::googlesql::ResolvedExpr* expr, std::vector<std::string>* cols) {
  if (expr == nullptr || cols == nullptr) return false;
  if (expr->node_kind() != ::googlesql::RESOLVED_FUNCTION_CALL) return false;
  const auto* call = expr->GetAs<::googlesql::ResolvedFunctionCall>();
  if (call == nullptr || call->function() == nullptr) return false;
  const std::string fn = absl::AsciiStrToLower(call->function()->Name());
  if (fn == "$equal") {
    return TryAppendUsingColumnFromEqualCall(call, cols);
  }
  if (fn == "$and") {
    for (int i = 0; i < call->argument_list_size(); ++i) {
      const ::googlesql::ResolvedExpr* arg = call->argument_list(i);
      if (arg == nullptr ||
          arg->node_kind() != ::googlesql::RESOLVED_FUNCTION_CALL) {
        return false;
      }
      if (!TryAppendUsingColumnFromEqualCall(
              arg->GetAs<::googlesql::ResolvedFunctionCall>(), cols)) {
        return false;
      }
    }
    return !cols->empty();
  }
  return false;
}

std::string Transpiler::EmitJoinScan(
    const ::googlesql::ResolvedJoinScan* node) {
  // INNER / LEFT / RIGHT / FULL all map directly onto DuckDB join
  // syntax. We compose the two input scans as derived tables for the
  // same reason `EmitFilterScan` does: each scan emits a self-
  // contained SELECT so the join sees the column aliases the child
  // emitted. CROSS JOIN is the natural fallback when the analyzer
  // hands us an INNER join with no `join_expr`.
  //
  // Lateral joins (`is_lateral`) are caught upstream by the route
  // classifier's `VisitResolvedJoinScan` override, which promotes
  // the route to `kSemanticExecutor`. We touch the accessor below
  // so `ResolvedAST::CheckFieldsAccessed` still observes the read
  // in case the transpiler is invoked through a path that bypasses
  // the classifier (legacy tests, debugging). Lateral correlated
  // `parameter_list` slots stay on the empty-string gate today
  // (`docs/ENGINE_POLICY.md`). `JOIN ... USING(...)`
  // lowers to DuckDB's native `USING (...)` by peeling column names
  // out of the analyzer's `$equal` / `$and` `join_expr` tree
  // (`docs/ENGINE_POLICY.md`).
  if (node == nullptr) return "";
  (void)node->is_lateral();
  if (node->parameter_list_size() > 0) {
    return "";
  }
  std::string left = EmitScan(node->left_scan());
  if (left.empty()) return "";
  const bool left_id_aliases = join_output_uses_id_aliases_;
  join_output_uses_id_aliases_ = false;
  std::string right = EmitScan(node->right_scan());
  if (right.empty()) return "";
  const bool right_id_aliases = join_output_uses_id_aliases_;
  join_output_uses_id_aliases_ = false;
  const std::string rn_quoted = internal::QuoteIdent(internal::kBqInputRnCol);
  const bool left_sql_has_rn = left.find(rn_quoted) != std::string::npos;
  const bool right_sql_has_rn = right.find(rn_quoted) != std::string::npos;

  const char* join_kw = nullptr;
  switch (node->join_type()) {
    case ::googlesql::ResolvedJoinScan::INNER:
      join_kw = "INNER JOIN";
      break;
    case ::googlesql::ResolvedJoinScan::LEFT:
      join_kw = "LEFT JOIN";
      break;
    case ::googlesql::ResolvedJoinScan::RIGHT:
      join_kw = "RIGHT JOIN";
      break;
    case ::googlesql::ResolvedJoinScan::FULL:
      join_kw = "FULL JOIN";
      break;
    default:
      return "";
  }

  if (node->has_using()) {
    std::vector<std::string> using_cols;
    if (!ExtractUsingColumnsFromJoinExpr(node->join_expr(), &using_cols)) {
      return "";
    }
    std::vector<std::string> quoted_using;
    quoted_using.reserve(using_cols.size());
    for (const std::string& col : using_cols) {
      quoted_using.push_back(internal::QuoteIdent(col));
    }
    return absl::StrCat("SELECT * FROM (",
                        left,
                        ") ",
                        join_kw,
                        " (",
                        right,
                        ") USING (",
                        absl::StrJoin(quoted_using, ", "),
                        ")");
  }

  if (node->join_expr() == nullptr) {
    // INNER + no join_expr is the analyzer's representation of
    // CROSS JOIN. LEFT / RIGHT / FULL without a condition is a
    // grammar error the analyzer rejects upstream; we double-check
    // here so a malformed AST falls back instead of emitting
    // illegal SQL.
    if (node->join_type() != ::googlesql::ResolvedJoinScan::INNER) {
      return "";
    }
    absl::flat_hash_set<int> left_ids;
    absl::flat_hash_set<int> right_ids;
    AppendScanColumnIds(node->left_scan(), &left_ids);
    AppendScanColumnIds(node->right_scan(), &right_ids);
    std::string projections = EmitJoinOutputProjections(node,
                                                        left_ids,
                                                        right_ids,
                                                        left_id_aliases,
                                                        right_id_aliases,
                                                        left_sql_has_rn,
                                                        right_sql_has_rn);
    if (projections.empty()) return "";
    join_output_uses_id_aliases_ = true;
    return absl::StrCat("SELECT ",
                        projections,
                        " FROM (",
                        left,
                        ") AS __bq_l CROSS JOIN (",
                        right,
                        ") AS __bq_r");
  }
  absl::flat_hash_set<int> left_ids;
  absl::flat_hash_set<int> right_ids;
  AppendScanColumnIds(node->left_scan(), &left_ids);
  AppendScanColumnIds(node->right_scan(), &right_ids);
  std::string projections = EmitJoinOutputProjections(node,
                                                      left_ids,
                                                      right_ids,
                                                      left_id_aliases,
                                                      right_id_aliases,
                                                      left_sql_has_rn,
                                                      right_sql_has_rn);
  if (projections.empty()) return "";
  std::string on = EmitJoinQualifiedExpr(node->join_expr(),
                                         left_ids,
                                         right_ids,
                                         left_id_aliases,
                                         right_id_aliases);
  if (on.empty()) return "";
  join_output_uses_id_aliases_ = true;
  return absl::StrCat("SELECT ",
                      projections,
                      " FROM (",
                      left,
                      ") AS __bq_l ",
                      join_kw,
                      " (",
                      right,
                      ") AS __bq_r ON ",
                      on);
}

}  // namespace transpiler
}  // namespace duckdb
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
