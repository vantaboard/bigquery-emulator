#include "backend/engine/duckdb/transpiler/transpiler.h"
#include "backend/engine/duckdb/transpiler/transpiler_internal.h"

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

std::string Transpiler::EmitAggregateFunctionCall(
    const ::googlesql::ResolvedAggregateFunctionCall* node) {
  // Aggregate dispatch lives in the same disposition table the scalar
  // emit consults; we just guard the unsupported modifier set first.
  // `$count_star` is the analyzer's representation of `COUNT(*)` -- we
  // emit it directly rather than going through a kDuckdbNative entry
  // because
  // the dispatch passes zero argument expressions (not even a single
  // `*` placeholder), so the standard `<NAME>(<args>)` shape wouldn't
  // apply.
  //
  // ORDER BY / LIMIT inside `array_agg`, `string_agg`, and
  // `array_concat_agg` lower to DuckDB's ordered-aggregate syntax.
  // HAVING MAX/MIN, multi-level GROUP BY, and aggregate filtering
  // still surface UNIMPLEMENTED.
  // `SAFE.<agg>(...)` (`SAFE_ERROR_MODE`) lowers `SAFE.SUM` via TRY().
  if (node == nullptr || node->function() == nullptr) return "";
  const std::string name = internal::ResolveFunctionName(node->function());
  const bool ordered_agg = internal::SupportsOrderedAggregateModifiers(name);
  if (node->having_modifier() != nullptr || node->group_by_list_size() > 0 ||
      node->group_by_aggregate_list_size() > 0 ||
      node->where_expr() != nullptr || node->having_expr() != nullptr) {
    LOG(INFO) << "duckdb transpiler: aggregate '" << node->function()->Name()
              << "' uses a modifier (HAVING / GROUP BY / filtering) that has "
                 "no DuckDB analog yet; surfacing UNIMPLEMENTED";
    return "";
  }
  if (!ordered_agg &&
      (node->order_by_item_list_size() > 0 || node->limit() != nullptr ||
       node->null_handling_modifier() !=
           ::googlesql::ResolvedNonScalarFunctionCallBase::
               DEFAULT_NULL_HANDLING)) {
    LOG(INFO) << "duckdb transpiler: aggregate '" << node->function()->Name()
              << "' uses a modifier (HAVING / ORDER BY / LIMIT / GROUP BY / "
                 "NULL-handling) that has no DuckDB analog yet; surfacing "
                 "UNIMPLEMENTED";
    return "";
  }
  if (ordered_agg &&
      node->null_handling_modifier() ==
          ::googlesql::ResolvedNonScalarFunctionCallBase::RESPECT_NULLS) {
    LOG(INFO) << "duckdb transpiler: aggregate '" << node->function()->Name()
              << "' uses RESPECT NULLS; surfacing UNIMPLEMENTED";
    return "";
  }
  if (node->error_mode() ==
      ::googlesql::ResolvedFunctionCallBase::SAFE_ERROR_MODE) {
    if (name != "sum") {
      LOG(INFO) << "duckdb transpiler: SAFE aggregate surfaces "
                   "UNIMPLEMENTED (function="
                << node->function()->Name() << ")";
      return "";
    }
  }
  if (name == "$count_star") {
    if (node->argument_list_size() != 0) return "";
    return "COUNT(*)";
  }
  std::vector<std::string> args;
  args.reserve(node->argument_list_size());
  for (int i = 0; i < node->argument_list_size(); ++i) {
    std::string a = EmitExpr(node->argument_list(i));
    if (a.empty()) return "";
    args.push_back(std::move(a));
  }
  std::string order_suffix;
  std::string limit_expr;
  if (node->limit() != nullptr) {
    limit_expr = EmitExpr(node->limit());
    if (limit_expr.empty()) return "";
  }
  if (ordered_agg && node->order_by_item_list_size() > 0) {
    std::vector<std::string> order_items;
    order_items.reserve(node->order_by_item_list_size());
    for (int i = 0; i < node->order_by_item_list_size(); ++i) {
      const ::googlesql::ResolvedOrderByItem* item =
          node->order_by_item_list(i);
      if (item == nullptr || item->column_ref() == nullptr) return "";
      if (item->collation_name() != nullptr) return "";
      std::string col = EmitColumnRef(item->column_ref());
      if (col.empty()) return "";
      order_items.push_back(absl::StrCat(
          col, internal::OrderByItemSuffix(item, /*bigquery_null_defaults=*/true)));
    }
    if (!order_items.empty()) {
      order_suffix =
          absl::StrCat(" ORDER BY ", absl::StrJoin(order_items, ", "));
    }
  }
  const bool ignore_nulls =
      node->null_handling_modifier() ==
      ::googlesql::ResolvedNonScalarFunctionCallBase::IGNORE_NULLS;
  const bool has_limit = !limit_expr.empty();
  const bool has_order = !order_suffix.empty();
  if (name == "array_concat_agg") {
    std::string prefix = node->distinct() ? "DISTINCT " : "";
    if (!has_order) {
      order_suffix =
          absl::StrCat(" ORDER BY ", internal::QuoteIdent(internal::kBqInputRnCol), " ASC");
    }
    std::string list_body = absl::StrCat(
        "list(", prefix, absl::StrJoin(args, ", "), order_suffix, ")");
    if (!args.empty()) {
      list_body =
          absl::StrCat(list_body, " FILTER (WHERE ", args[0], " IS NOT NULL)");
      list_body =
          internal::AppendArrayAggNullFilter(list_body, args[0], ignore_nulls);
    }
    if (has_limit) {
      list_body =
          absl::StrCat("list_slice(", list_body, ", 1, ", limit_expr, ")");
    }
    return absl::StrCat("flatten(", list_body, ")");
  }
  std::vector<std::string> call_args = args;
  if (name == "string_agg" && call_args.size() == 1) {
    // BigQuery STRING_AGG(x ORDER BY ...) defaults the delimiter to ','.
    call_args.push_back("','");
  }
  // DuckDB has no BigQuery-style LIMIT inside STRING_AGG; ARRAY_AGG
  // modifiers lower to `list(... ORDER BY ...)` + optional
  // `list_slice(..., 1, n)`. STRING_AGG LIMIT (with or without ORDER
  // BY) uses the same list staging + `array_to_string`.
  if (name == "array_agg" && (has_order || has_limit)) {
    std::string prefix = node->distinct() ? "DISTINCT " : "";
    std::string body =
        absl::StrCat("list(", prefix, args[0], order_suffix, ")");
    body = internal::AppendArrayAggNullFilter(body, args[0], ignore_nulls);
    if (has_limit) {
      body = absl::StrCat("list_slice(", body, ", 1, ", limit_expr, ")");
    }
    if (!ignore_nulls) {
      body = internal::WrapArrayAggRespectNulls(body, args[0]);
    }
    return body;
  }
  if (name == "array_agg" && !has_order && !has_limit) {
    const std::string rn_order =
        input_rn_ordering_
            ? absl::StrCat(" ORDER BY ", internal::QuoteIdent(internal::kBqInputRnCol), " ASC")
            : "";
    std::string body = node->distinct()
                           ? absl::StrCat("array_agg(DISTINCT ", args[0], ")")
                           : absl::StrCat("list(", args[0], rn_order, ")");
    body = internal::AppendArrayAggNullFilter(body, args[0], ignore_nulls);
    if (!ignore_nulls) {
      body = internal::WrapArrayAggRespectNulls(body, args[0]);
    }
    return body;
  }
  if (name == "string_agg" && node->distinct() && !has_order && !has_limit) {
    return "";
  }
  if (name == "string_agg" && has_limit) {
    std::string prefix = node->distinct() ? "DISTINCT " : "";
    std::string list_body =
        absl::StrCat("list(", prefix, args[0], order_suffix, ")");
    list_body =
        absl::StrCat("list_slice(", list_body, ", 1, ", limit_expr, ")");
    return absl::StrCat("array_to_string(", list_body, ", ", call_args[1], ")");
  }
  const auto* entry = LookupFunction(name);
  if (entry == nullptr) {
    LOG(INFO) << "duckdb transpiler: aggregate '" << name
              << "' has no disposition; surfacing UNIMPLEMENTED";
    return "";
  }
  switch (entry->disposition) {
    case Disposition::kDuckdbNative:
    case Disposition::kDuckdbRewrite: {
      std::string prefix = node->distinct() ? "DISTINCT " : "";
      std::string body = absl::StrCat(entry->duckdb_name,
                                      "(",
                                      prefix,
                                      absl::StrJoin(call_args, ", "),
                                      order_suffix,
                                      ")");
      if (name == "countif") {
        body = absl::StrCat("coalesce(", body, ", 0)");
      }
      if (node->error_mode() ==
          ::googlesql::ResolvedFunctionCallBase::SAFE_ERROR_MODE) {
        return absl::StrCat("TRY(", body, ")");
      }
      return body;
    }
    case Disposition::kDuckdbUdf:
      // Same ready/planned dispatch as scalar `EmitFunctionCall`. A
      // ready `duckdb_udf` aggregate row is rare (aggregates are
      // either pure DuckDB or routed to the semantic executor) but
      // is supported here for symmetry; the YAML generator still
      // refuses a ready row without `duckdb_name=`.
      if (!entry->planned && !entry->duckdb_name.empty()) {
        std::string prefix = node->distinct() ? "DISTINCT " : "";
        return absl::StrCat(
            entry->duckdb_name, "(", prefix, absl::StrJoin(args, ", "), ")");
      }
      LOG(INFO) << "duckdb transpiler: aggregate '" << name
                << "' route=duckdb_udf (planned=" << entry->planned
                << "); surfacing UNIMPLEMENTED";
      return "";
    case Disposition::kSemanticExecutor:
    case Disposition::kControlOp:
    case Disposition::kLocalStub:
      // Aggregates today have no `kLocalStub` rows in
      // `functions.yaml`, but the switch must be exhaustive over
      // the disposition enum. Behave like `kSemanticExecutor` /
      // `kControlOp`: surface the empty-string contract so the
      // engine returns UNIMPLEMENTED rather than the transpiler
      // emitting a guess.
      LOG(INFO) << "duckdb transpiler: aggregate '" << name
                << "' route=" << DispositionToString(entry->disposition)
                << " (planned=" << entry->planned
                << "); surfacing UNIMPLEMENTED";
      return "";
    case Disposition::kUnsupported:
      LOG(INFO) << "duckdb transpiler: aggregate '" << name
                << "' unsupported; surfacing UNIMPLEMENTED";
      return "";
  }
  return "";
}

std::string Transpiler::EmitAnalyticFunctionCall(
    const ::googlesql::ResolvedAnalyticFunctionCall* node) {
  // Window functions: route the name through the disposition table
  // and emit `<NAME>(<args>)` for the analytic call body. The OVER
  // clause (PARTITION BY, ORDER BY, frame) is stitched on by
  // `EmitAnalyticScan` since those live on the surrounding group.
  //
  // The function table flags ROW_NUMBER / RANK / DENSE_RANK / CUME_DIST
  // / PERCENT_RANK / NTILE / LAG / LEAD / FIRST_VALUE / LAST_VALUE /
  // NTH_VALUE as `kDuckdbNative` so they fall through here;
  // aggregate-over-window calls (SUM / COUNT / AVG / MIN / MAX OVER
  // (...)) flow through the same map entries the scalar aggregate
  // emit uses.
  //
  // We share the SAFE-mode / modifier-rejection contract with
  // `EmitAggregateFunctionCall` because GoogleSQL hands us the same
  // base class for both.
  if (node == nullptr || node->function() == nullptr) return "";
  if (node->error_mode() ==
      ::googlesql::ResolvedFunctionCallBase::SAFE_ERROR_MODE) {
    LOG(INFO) << "duckdb transpiler: SAFE analytic function surfaces "
                 "UNIMPLEMENTED (function="
              << node->function()->Name() << ")";
    return "";
  }
  if (node->null_handling_modifier() !=
      ::googlesql::ResolvedNonScalarFunctionCallBase::DEFAULT_NULL_HANDLING) {
    const std::string name = internal::ResolveFunctionName(node->function());
    if (name != "percentile_disc") {
      // IGNORE / RESPECT NULLS modifies LAG / LEAD / FIRST_VALUE /
      // LAST_VALUE semantics; DuckDB has the same keywords but the
      // rewrite needs more care than this emit pass covers.
      LOG(INFO) << "duckdb transpiler: analytic '" << node->function()->Name()
                << "' uses IGNORE/RESPECT NULLS; surfacing UNIMPLEMENTED";
      return "";
    }
  }
  std::vector<std::string> args;
  args.reserve(node->argument_list_size());
  for (int i = 0; i < node->argument_list_size(); ++i) {
    std::string a = EmitExpr(node->argument_list(i));
    if (a.empty()) return "";
    args.push_back(std::move(a));
  }
  const std::string name = internal::ResolveFunctionName(node->function());
  if (name == "$count_star") {
    if (!args.empty()) return "";
    return "COUNT(*)";
  }
  const auto* entry = LookupFunction(name);
  if (entry == nullptr) {
    LOG(INFO) << "duckdb transpiler: analytic function '" << name
              << "' has no disposition; surfacing UNIMPLEMENTED";
    return "";
  }
  switch (entry->disposition) {
    case Disposition::kDuckdbNative:
    case Disposition::kDuckdbRewrite: {
      std::string prefix = node->distinct() ? "DISTINCT " : "";
      std::string body = absl::StrCat(
          entry->duckdb_name, "(", prefix, absl::StrJoin(args, ", "), ")");
      return body;
    }
    case Disposition::kDuckdbUdf:
      // Symmetric to `EmitFunctionCall` /
      // `EmitAggregateFunctionCall`. A ready row's `duckdb_name`
      // points at the registered UDF / macro; a planned row
      // surfaces UNIMPLEMENTED until the wrapper lands.
      if (!entry->planned && !entry->duckdb_name.empty()) {
        std::string prefix = node->distinct() ? "DISTINCT " : "";
        return absl::StrCat(
            entry->duckdb_name, "(", prefix, absl::StrJoin(args, ", "), ")");
      }
      LOG(INFO) << "duckdb transpiler: analytic function '" << name
                << "' route=duckdb_udf (planned=" << entry->planned
                << "); surfacing UNIMPLEMENTED";
      return "";
    case Disposition::kSemanticExecutor:
    case Disposition::kControlOp:
    case Disposition::kLocalStub:
      // Analytic functions have no `kLocalStub` rows in
      // `functions.yaml`, but the switch must be exhaustive over
      // the disposition enum. Surface UNIMPLEMENTED so the
      // engine never lowers a stub family through the fast path.
      LOG(INFO) << "duckdb transpiler: analytic function '" << name
                << "' route=" << DispositionToString(entry->disposition)
                << " (planned=" << entry->planned
                << "); surfacing UNIMPLEMENTED";
      return "";
    case Disposition::kUnsupported:
      LOG(INFO) << "duckdb transpiler: analytic function '" << name
                << "' unsupported; surfacing UNIMPLEMENTED";
      return "";
  }
  return "";
}

std::string Transpiler::EmitFrameBound(
    const ::googlesql::ResolvedWindowFrameExpr* expr) {
  if (expr == nullptr) return "";
  switch (expr->boundary_type()) {
    case ::googlesql::ResolvedWindowFrameExpr::UNBOUNDED_PRECEDING:
      return "UNBOUNDED PRECEDING";
    case ::googlesql::ResolvedWindowFrameExpr::CURRENT_ROW:
      return "CURRENT ROW";
    case ::googlesql::ResolvedWindowFrameExpr::UNBOUNDED_FOLLOWING:
      return "UNBOUNDED FOLLOWING";
    case ::googlesql::ResolvedWindowFrameExpr::OFFSET_PRECEDING: {
      if (expr->expression() == nullptr) return "";
      std::string e = EmitExpr(expr->expression());
      if (e.empty()) return "";
      return absl::StrCat(e, " PRECEDING");
    }
    case ::googlesql::ResolvedWindowFrameExpr::OFFSET_FOLLOWING: {
      if (expr->expression() == nullptr) return "";
      std::string e = EmitExpr(expr->expression());
      if (e.empty()) return "";
      return absl::StrCat(e, " FOLLOWING");
    }
  }
  return "";
}

}  // namespace transpiler
}  // namespace duckdb
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
