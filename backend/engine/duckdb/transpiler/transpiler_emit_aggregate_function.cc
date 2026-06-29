
#include "absl/log/log.h"
#include "googlesql/public/catalog.h"
#include "googlesql/public/function.h"
#include "googlesql/public/sql_function.h"
#include "googlesql/public/templated_sql_function.h"
#include "googlesql/public/type.h"
#include "googlesql/public/types/struct_type.h"
#include "googlesql/public/value.h"
#include "googlesql/resolved_ast/resolved_node.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace duckdb {
namespace transpiler {

namespace {

using EmitExprFn = std::function<std::string(const ::googlesql::ResolvedExpr*)>;
using EmitColumnRefFn =
    std::function<std::string(const ::googlesql::ResolvedColumnRef*)>;

bool RejectUnsupportedAggregateModifiers(
    const ::googlesql::ResolvedAggregateFunctionCall* node,
    absl::string_view name,
    bool ordered_agg) {
  if (node->having_modifier() != nullptr || node->group_by_list_size() > 0 ||
      node->group_by_aggregate_list_size() > 0 ||
      node->having_expr() != nullptr) {
    LOG(INFO) << "duckdb transpiler: aggregate '" << node->function()->Name()
              << "' uses a modifier (HAVING / GROUP BY / filtering) that has "
                 "no DuckDB analog yet; surfacing UNIMPLEMENTED";
    return true;
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
    return true;
  }
  if (ordered_agg &&
      node->null_handling_modifier() ==
          ::googlesql::ResolvedNonScalarFunctionCallBase::RESPECT_NULLS) {
    LOG(INFO) << "duckdb transpiler: aggregate '" << node->function()->Name()
              << "' uses RESPECT NULLS; surfacing UNIMPLEMENTED";
    return true;
  }
  if (node->error_mode() ==
      ::googlesql::ResolvedFunctionCallBase::SAFE_ERROR_MODE) {
    LOG(INFO) << "duckdb transpiler: SAFE aggregate surfaces UNIMPLEMENTED "
                 "(function="
              << node->function()->Name() << ")";
    return true;
  }
  return false;
}

std::string BuildAggregateFilterSuffix(
    const ::googlesql::ResolvedAggregateFunctionCall* node,
    const EmitExprFn& emit_expr) {
  if (node->where_expr() == nullptr) return "";
  std::string filter_expr = emit_expr(node->where_expr());
  if (filter_expr.empty()) return std::string();
  return absl::StrCat(" FILTER (WHERE ", filter_expr, ")");
}

struct OrderedAggParts {
  std::string order_suffix;
  std::string limit_expr;
};

OrderedAggParts BuildOrderedAggParts(
    const ::googlesql::ResolvedAggregateFunctionCall* node,
    bool ordered_agg,
    const EmitExprFn& emit_expr,
    const EmitColumnRefFn& emit_col,
    bool* ok) {
  OrderedAggParts parts;
  *ok = true;
  if (node->limit() != nullptr) {
    parts.limit_expr = emit_expr(node->limit());
    if (parts.limit_expr.empty()) {
      *ok = false;
      return parts;
    }
  }
  if (!ordered_agg || node->order_by_item_list_size() == 0) {
    return parts;
  }
  std::vector<std::string> order_items;
  order_items.reserve(node->order_by_item_list_size());
  for (int i = 0; i < node->order_by_item_list_size(); ++i) {
    const ::googlesql::ResolvedOrderByItem* item = node->order_by_item_list(i);
    if (item == nullptr || item->column_ref() == nullptr) {
      *ok = false;
      return parts;
    }
    if (item->collation_name() != nullptr) {
      *ok = false;
      return parts;
    }
    std::string col = emit_col(item->column_ref());
    if (col.empty()) {
      *ok = false;
      return parts;
    }
    order_items.push_back(absl::StrCat(
        col,
        internal::OrderByItemSuffix(item, /*bigquery_null_defaults=*/true)));
  }
  if (!order_items.empty()) {
    parts.order_suffix =
        absl::StrCat(" ORDER BY ", absl::StrJoin(order_items, ", "));
  }
  return parts;
}

std::string EmitArrayConcatAggCall(
    const ::googlesql::ResolvedAggregateFunctionCall* node,
    const std::vector<std::string>& args,
    const OrderedAggParts& ordered,
    bool ignore_nulls,
    const std::string& filter_suffix) {
  const bool has_limit = !ordered.limit_expr.empty();
  const bool has_order = !ordered.order_suffix.empty();
  std::string prefix = node->distinct() ? "DISTINCT " : "";
  std::string order_suffix = ordered.order_suffix;
  if (!has_order) {
    order_suffix = absl::StrCat(
        " ORDER BY ", internal::QuoteIdent(internal::kBqInputRnCol), " ASC");
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
    list_body = absl::StrCat(
        "list_slice(", list_body, ", 1, ", ordered.limit_expr, ")");
  }
  std::string body = absl::StrCat("flatten(", list_body, ")");
  if (!filter_suffix.empty()) {
    body = absl::StrCat(body, filter_suffix);
  }
  return body;
}

std::string EmitArrayAggCall(
    const ::googlesql::ResolvedAggregateFunctionCall* node,
    const std::vector<std::string>& args,
    const OrderedAggParts& ordered,
    bool ignore_nulls,
    bool input_rn_ordering,
    const std::string& filter_suffix) {
  const bool has_limit = !ordered.limit_expr.empty();
  const bool has_order = !ordered.order_suffix.empty();
  if (has_order || has_limit) {
    std::string prefix = node->distinct() ? "DISTINCT " : "";
    std::string body =
        absl::StrCat("list(", prefix, args[0], ordered.order_suffix, ")");
    body = internal::AppendArrayAggNullFilter(body, args[0], ignore_nulls);
    if (has_limit) {
      body =
          absl::StrCat("list_slice(", body, ", 1, ", ordered.limit_expr, ")");
    }
    if (!ignore_nulls) {
      body = internal::WrapArrayAggRespectNulls(body, args[0]);
    }
    if (!filter_suffix.empty()) {
      body = absl::StrCat(body, filter_suffix);
    }
    return body;
  }
  const std::string rn_order =
      input_rn_ordering
          ? absl::StrCat(" ORDER BY ",
                         internal::QuoteIdent(internal::kBqInputRnCol),
                         " ASC")
          : "";
  std::string body = node->distinct()
                         ? absl::StrCat("array_agg(DISTINCT ", args[0], ")")
                         : absl::StrCat("list(", args[0], rn_order, ")");
  body = internal::AppendArrayAggNullFilter(body, args[0], ignore_nulls);
  if (!ignore_nulls) {
    body = internal::WrapArrayAggRespectNulls(body, args[0]);
  }
  if (!filter_suffix.empty()) {
    body = absl::StrCat(body, filter_suffix);
  }
  return body;
}

std::string EmitStringAggCall(
    const ::googlesql::ResolvedAggregateFunctionCall* node,
    const std::vector<std::string>& args,
    const OrderedAggParts& ordered,
    const std::string& filter_suffix) {
  const bool has_limit = !ordered.limit_expr.empty();
  const bool has_order = !ordered.order_suffix.empty();
  std::vector<std::string> call_args = args;
  if (call_args.size() == 1) {
    call_args.push_back("','");
  }
  if (node->distinct() && !has_order && !has_limit) {
    return "";
  }
  if (!has_limit) {
    return "";
  }
  std::string prefix = node->distinct() ? "DISTINCT " : "";
  std::string list_body =
      absl::StrCat("list(", prefix, args[0], ordered.order_suffix, ")");
  list_body =
      absl::StrCat("list_slice(", list_body, ", 1, ", ordered.limit_expr, ")");
  std::string body =
      absl::StrCat("array_to_string(", list_body, ", ", call_args[1], ")");
  if (!filter_suffix.empty()) {
    body = absl::StrCat(body, filter_suffix);
  }
  return body;
}

std::optional<std::string> TryEmitNamedAggregateCall(
    absl::string_view name,
    const ::googlesql::ResolvedAggregateFunctionCall* node,
    const std::vector<std::string>& args,
    const OrderedAggParts& ordered,
    bool ignore_nulls,
    bool input_rn_ordering,
    const std::string& filter_suffix) {
  if (name == "array_concat_agg") {
    return EmitArrayConcatAggCall(
        node, args, ordered, ignore_nulls, filter_suffix);
  }
  if (name == "array_agg") {
    return EmitArrayAggCall(
        node, args, ordered, ignore_nulls, input_rn_ordering, filter_suffix);
  }
  if (name == "string_agg") {
    if (std::string body =
            EmitStringAggCall(node, args, ordered, filter_suffix);
        !body.empty()) {
      return body;
    }
    if (node->distinct() && ordered.order_suffix.empty() &&
        ordered.limit_expr.empty()) {
      return std::string();
    }
  }
  return std::nullopt;
}

std::optional<std::string> TryEmitAggregateEarlyExit(
    const ::googlesql::ResolvedAggregateFunctionCall* node,
    absl::string_view name,
    const EmitExprFn& emit_expr,
    std::string* filter_suffix) {
  if (const auto* sql_fn =
          dynamic_cast<const ::googlesql::SQLFunction*>(node->function());
      sql_fn != nullptr && sql_fn->IsAggregate()) {
    LOG(INFO) << "duckdb transpiler: SQL UDAF '" << node->function()->Name()
              << "' routes to semantic executor; surfacing UNIMPLEMENTED";
    return std::string();
  }
  const bool ordered_agg = internal::SupportsOrderedAggregateModifiers(name);
  if (RejectUnsupportedAggregateModifiers(node, name, ordered_agg)) {
    return std::string();
  }
  *filter_suffix = BuildAggregateFilterSuffix(node, emit_expr);
  if (node->where_expr() != nullptr && filter_suffix->empty()) {
    return std::string();
  }
  if (name == "$count_star") {
    if (node->argument_list_size() != 0) return std::string();
    return filter_suffix->empty() ? std::optional<std::string>("COUNT(*)")
                                  : std::optional<std::string>(absl::StrCat(
                                        "COUNT(*)", *filter_suffix));
  }
  return std::nullopt;
}

}  // namespace

std::string Transpiler::EmitAggregateFunctionCall(
    const ::googlesql::ResolvedAggregateFunctionCall* node) {
  if (node == nullptr || node->function() == nullptr) return "";
  const std::string name = internal::ResolveFunctionName(node->function());
  const EmitExprFn emit_expr = [this](const ::googlesql::ResolvedExpr* expr) {
    return EmitExpr(expr);
  };
  std::string filter_suffix;
  if (auto early =
          TryEmitAggregateEarlyExit(node, name, emit_expr, &filter_suffix);
      early.has_value()) {
    return *early;
  }
  const bool ordered_agg = internal::SupportsOrderedAggregateModifiers(name);

  const EmitColumnRefFn emit_col =
      [this](const ::googlesql::ResolvedColumnRef* ref) {
        return EmitColumnRef(ref);
      };

  std::vector<std::string> args;
  args.reserve(node->argument_list_size());
  for (int i = 0; i < node->argument_list_size(); ++i) {
    std::string a = emit_expr(node->argument_list(i));
    if (a.empty()) return "";
    args.push_back(std::move(a));
  }

  bool ordered_ok = true;
  OrderedAggParts ordered =
      BuildOrderedAggParts(node, ordered_agg, emit_expr, emit_col, &ordered_ok);
  if (!ordered_ok) return "";

  const bool ignore_nulls =
      node->null_handling_modifier() ==
      ::googlesql::ResolvedNonScalarFunctionCallBase::IGNORE_NULLS;

  if (auto named = TryEmitNamedAggregateCall(name,
                                             node,
                                             args,
                                             ordered,
                                             ignore_nulls,
                                             input_rn_ordering_,
                                             filter_suffix);
      named.has_value()) {
    return *named;
  }

  std::vector<std::string> call_args = args;
  if (name == "string_agg" && call_args.size() == 1) {
    call_args.push_back("','");
  }
  return EmitDispositionAggregateCall(name,
                                      node,
                                      call_args,
                                      ordered.order_suffix,
                                      LookupFunction(name),
                                      filter_suffix);
}

std::string Transpiler::EmitAnalyticFunctionCall(
    const ::googlesql::ResolvedAnalyticFunctionCall* node) {
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
    if (name != "percentile_disc" &&
        !internal::SupportsAnalyticNullHandling(name)) {
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
          entry->duckdb_name, "(", prefix, absl::StrJoin(args, ", "));
      if (internal::SupportsAnalyticNullHandling(name)) {
        absl::StrAppend(&body, internal::AnalyticNullHandlingSuffix(node));
      }
      absl::StrAppend(&body, ")");
      return body;
    }
    case Disposition::kDuckdbUdf:
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
