
#include "googlesql/public/simple_catalog.h"
#include "googlesql/public/type.h"
#include "googlesql/public/types/struct_type.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace semantic {
namespace scan_eval_internal {

using ::bigquery_emulator::backend::engine::semantic::EvalContext;

absl::StatusOr<bool> EvalBoolExpr(const ::googlesql::ResolvedExpr* expr,
                                  EvalContext& ctx) {
  if (expr == nullptr) {
    return absl::InvalidArgumentError("semantic: null filter expression");
  }
  auto v = EvalExpr(*expr, ctx);
  if (!v.ok()) return v.status();
  if (v->is_null()) return false;
  if (v->type_kind() != ::googlesql::TYPE_BOOL) {
    return MakeSemanticError(SemanticErrorReason::kInvalidArgument,
                             "semantic: expected BOOL in predicate");
  }
  return v->bool_value();
}

absl::StatusOr<int64_t> EvalLimitOffsetInt64(
    const ::googlesql::ResolvedExpr* expr,
    EvalContext& ctx,
    absl::string_view role) {
  if (expr == nullptr) {
    return absl::InvalidArgumentError(
        absl::StrCat("semantic: LIMIT/OFFSET missing ", role, " expression"));
  }
  auto value_or = EvalExpr(*expr, ctx);
  if (!value_or.ok()) return value_or.status();
  const Value& value = *value_or;
  if (value.is_null()) {
    return MakeSemanticError(
        SemanticErrorReason::kInvalidArgument,
        absl::StrCat("semantic: LIMIT/OFFSET ", role, " must not be NULL"));
  }
  if (value.type_kind() != ::googlesql::TYPE_INT64) {
    return MakeSemanticError(
        SemanticErrorReason::kInvalidArgument,
        absl::StrCat("semantic: LIMIT/OFFSET ", role, " must be INT64"));
  }
  return value.int64_value();
}

absl::StatusOr<std::vector<ColumnBindings>> MaterializeLimitOffsetScan(
    const ::googlesql::ResolvedLimitOffsetScan& scan, EvalContext& ctx) {
  auto input_or = MaterializeScanImpl(scan.input_scan(), ctx);
  if (!input_or.ok()) return input_or.status();
  std::vector<ColumnBindings> rows = *std::move(input_or);

  int64_t offset = 0;
  if (scan.offset() != nullptr) {
    auto offset_or = EvalLimitOffsetInt64(scan.offset(), ctx, "offset");
    if (!offset_or.ok()) return offset_or.status();
    offset = *offset_or;
    if (offset < 0) {
      return MakeSemanticError(SemanticErrorReason::kInvalidArgument,
                               "semantic: OFFSET must be non-negative");
    }
  }

  size_t start = static_cast<size_t>(offset);
  if (start >= rows.size()) {
    return std::vector<ColumnBindings>{};
  }

  size_t end = rows.size();
  if (scan.limit() != nullptr) {
    auto limit_or = EvalLimitOffsetInt64(scan.limit(), ctx, "limit");
    if (!limit_or.ok()) return limit_or.status();
    const int64_t limit = *limit_or;
    if (limit < 0) {
      return MakeSemanticError(SemanticErrorReason::kInvalidArgument,
                               "semantic: LIMIT must be non-negative");
    }
    end = std::min(end, start + static_cast<size_t>(limit));
  }

  std::vector<ColumnBindings> window;
  window.reserve(end - start);
  for (size_t i = start; i < end; ++i) {
    window.push_back(std::move(rows[i]));
  }
  return window;
}

namespace {

bool CompareOrderByItemValues(const ::googlesql::ResolvedOrderByItem* item,
                              const Value& va,
                              const Value& vb) {
  if (item == nullptr || item->column_ref() == nullptr) return false;
  if (ValueEqual(va, vb)) return false;
  if (va.is_null() || vb.is_null()) {
    bool nulls_first = false;
    switch (item->null_order()) {
      case ::googlesql::ResolvedOrderByItem::NULLS_FIRST:
        nulls_first = true;
        break;
      case ::googlesql::ResolvedOrderByItem::NULLS_LAST:
        nulls_first = false;
        break;
      default:
        nulls_first = !item->is_descending();
        break;
    }
    if (va.is_null() && vb.is_null()) return false;
    if (va.is_null()) return nulls_first;
    return !nulls_first;
  }
  if (item->collation_name() != nullptr &&
      item->collation_name()->node_kind() == ::googlesql::RESOLVED_LITERAL &&
      item->collation_name()->type()->kind() == ::googlesql::TYPE_STRING &&
      va.type_kind() == ::googlesql::TYPE_STRING &&
      vb.type_kind() == ::googlesql::TYPE_STRING) {
    const std::string collation = item->collation_name()
                                      ->GetAs<::googlesql::ResolvedLiteral>()
                                      ->value()
                                      .string_value();
    if (collation == "und:ci") {
      return item->is_descending()
                 ? !(absl::AsciiStrToLower(va.string_value()) <
                     absl::AsciiStrToLower(vb.string_value()))
                 : absl::AsciiStrToLower(va.string_value()) <
                       absl::AsciiStrToLower(vb.string_value());
    }
  }
  return item->is_descending() ? !ValueLess(va, vb) : ValueLess(va, vb);
}

bool OrderByScanRowLess(const ::googlesql::ResolvedOrderByScan& order,
                        const ColumnBindings& a,
                        const ColumnBindings& b) {
  for (int i = 0; i < order.order_by_item_list_size(); ++i) {
    const ::googlesql::ResolvedOrderByItem* item = order.order_by_item_list(i);
    if (item == nullptr || item->column_ref() == nullptr) continue;
    const int col_id = item->column_ref()->column().column_id();
    auto av = a.find(col_id);
    auto bv = b.find(col_id);
    const Value va = av == a.end() ? Value() : av->second;
    const Value vb = bv == b.end() ? Value() : bv->second;
    if (ValueEqual(va, vb)) continue;
    return CompareOrderByItemValues(item, va, vb);
  }
  return false;
}

}  // namespace

absl::StatusOr<std::vector<ColumnBindings>> MaterializeFilterScan(
    const ::googlesql::ResolvedFilterScan& filter, EvalContext& ctx) {
  auto input = MaterializeScanImpl(filter.input_scan(), ctx);
  if (!input.ok()) return input.status();
  std::vector<ColumnBindings> out;
  absl::flat_hash_map<std::string, ::googlesql::Value> by_name;
  for (const ColumnBindings& row : *input) {
    ColumnBindings merged;
    if (ctx.columns != nullptr) {
      merged = *ctx.columns;
    }
    for (const auto& [col_id, val] : row) {
      merged[col_id] = val;
    }
    EvalContext row_ctx = ctx;
    row_ctx.columns = &merged;
    PopulateColumnNameBindings(filter.input_scan(), merged, by_name);
    if (ctx.columns_by_name != nullptr) {
      for (const auto& [name, val] : *ctx.columns_by_name) {
        by_name[name] = val;
      }
    }
    row_ctx.columns_by_name = &by_name;
    auto ok = EvalBoolExpr(filter.filter_expr(), row_ctx);
    if (!ok.ok()) return ok.status();
    if (*ok) out.push_back(row);
  }
  return out;
}

absl::StatusOr<std::vector<ColumnBindings>> MaterializeOrderByScanRows(
    const ::googlesql::ResolvedOrderByScan& order, EvalContext& ctx) {
  auto input = MaterializeScanImpl(order.input_scan(), ctx);
  if (!input.ok()) return input.status();
  std::vector<ColumnBindings> rows = *std::move(input);
  std::stable_sort(rows.begin(),
                   rows.end(),
                   [&](const ColumnBindings& a, const ColumnBindings& b) {
                     return OrderByScanRowLess(order, a, b);
                   });
  return rows;
}

absl::StatusOr<std::vector<ColumnBindings>> MaterializeWithScanBody(
    const ::googlesql::ResolvedWithScan& with_scan, const EvalContext& ctx) {
  absl::flat_hash_map<std::string, CteTable> tables;
  for (int i = 0; i < with_scan.with_entry_list_size(); ++i) {
    const ::googlesql::ResolvedWithEntry* entry = with_scan.with_entry_list(i);
    if (entry == nullptr || entry->with_subquery() == nullptr) {
      return absl::InternalError("semantic: malformed WithEntry");
    }
    const ::googlesql::ResolvedScan* sub = entry->with_subquery();
    EvalContext entry_ctx = ctx;
    entry_ctx.with_tables = &tables;
    auto rows = MaterializeScanImpl(sub, entry_ctx);
    if (!rows.ok()) return rows.status();
    CteTable table;
    table.rows = *std::move(rows);
    table.column_ids.reserve(sub->column_list_size());
    for (int j = 0; j < sub->column_list_size(); ++j) {
      table.column_ids.push_back(sub->column_list(j).column_id());
    }
    tables[std::string(entry->with_query_name())] = std::move(table);
  }
  EvalContext body_ctx = ctx;
  body_ctx.with_tables = &tables;
  return MaterializeScanImpl(with_scan.query(), body_ctx);
}

absl::StatusOr<std::vector<ColumnBindings>> MaterializeScanImpl(
    const ::googlesql::ResolvedScan* scan, EvalContext& ctx) {
  scan = StripBarrierScans(scan);
  if (scan == nullptr) {
    return absl::InvalidArgumentError("semantic: null scan");
  }
  switch (scan->node_kind()) {
    case ::googlesql::RESOLVED_TABLE_SCAN:
      return MaterializeTableScan(
          *scan->GetAs<::googlesql::ResolvedTableScan>());
    case ::googlesql::RESOLVED_SINGLE_ROW_SCAN:
      return MaterializeSingleRowScan(
          *scan->GetAs<::googlesql::ResolvedSingleRowScan>());
    case ::googlesql::RESOLVED_ARRAY_SCAN:
      return MaterializeArrayScan(
          *scan->GetAs<::googlesql::ResolvedArrayScan>(), ctx);
    case ::googlesql::RESOLVED_PROJECT_SCAN: {
      const auto* project = scan->GetAs<::googlesql::ResolvedProjectScan>();
      auto input = MaterializeScanImpl(project->input_scan(), ctx);
      if (!input.ok()) return input.status();
      return ProjectRows(*project, *input, ctx);
    }
    case ::googlesql::RESOLVED_FILTER_SCAN:
      return MaterializeFilterScan(
          *scan->GetAs<::googlesql::ResolvedFilterScan>(), ctx);
    case ::googlesql::RESOLVED_ORDER_BY_SCAN:
      return MaterializeOrderByScanRows(
          *scan->GetAs<::googlesql::ResolvedOrderByScan>(), ctx);
    case ::googlesql::RESOLVED_WITH_SCAN:
      return MaterializeWithScanBody(
          *scan->GetAs<::googlesql::ResolvedWithScan>(), ctx);
    case ::googlesql::RESOLVED_WITH_REF_SCAN:
      return MaterializeWithRefScan(
          *scan->GetAs<::googlesql::ResolvedWithRefScan>(), ctx);
    case ::googlesql::RESOLVED_RELATION_ARGUMENT_SCAN:
      return MaterializeRelationArgumentScan(
          *scan->GetAs<::googlesql::ResolvedRelationArgumentScan>(), ctx);
    case ::googlesql::RESOLVED_JOIN_SCAN:
      return MaterializeJoinScan(*scan->GetAs<::googlesql::ResolvedJoinScan>(),
                                 ctx);
    case ::googlesql::RESOLVED_SET_OPERATION_SCAN:
      return MaterializeSetOperationScan(
          *scan->GetAs<::googlesql::ResolvedSetOperationScan>(), ctx);
    case ::googlesql::RESOLVED_AGGREGATE_SCAN:
    case ::googlesql::RESOLVED_ANONYMIZED_AGGREGATE_SCAN:
    case ::googlesql::RESOLVED_DIFFERENTIAL_PRIVACY_AGGREGATE_SCAN:
    case ::googlesql::RESOLVED_AGGREGATION_THRESHOLD_AGGREGATE_SCAN: {
      const auto* aggregate =
          dynamic_cast<const ::googlesql::ResolvedAggregateScanBase*>(scan);
      if (aggregate == nullptr) {
        return absl::InvalidArgumentError(
            "semantic: aggregate scan has unexpected node kind");
      }
      return MaterializeAggregateScan(*aggregate, ctx);
    }
    case ::googlesql::RESOLVED_ANALYTIC_SCAN:
      return MaterializeAnalyticScan(
          *scan->GetAs<::googlesql::ResolvedAnalyticScan>(), ctx);
    case ::googlesql::RESOLVED_SAMPLE_SCAN:
      return MaterializeSampleScan(
          *scan->GetAs<::googlesql::ResolvedSampleScan>(), ctx);
    case ::googlesql::RESOLVED_LIMIT_OFFSET_SCAN:
      return MaterializeLimitOffsetScan(
          *scan->GetAs<::googlesql::ResolvedLimitOffsetScan>(), ctx);
    case ::googlesql::RESOLVED_TVFSCAN:
      return MaterializeTvfScan(*scan->GetAs<::googlesql::ResolvedTVFScan>(),
                                ctx);
    case ::googlesql::RESOLVED_MATCH_RECOGNIZE_SCAN:
      return MaterializeMatchRecognizeScan(
          *scan->GetAs<::googlesql::ResolvedMatchRecognizeScan>(), ctx);
    default:
      return MakeSemanticError(SemanticErrorReason::kNotImplemented,
                               absl::StrCat("semantic: scan kind ",
                                            scan->node_kind_string(),
                                            " is not yet implemented"));
  }
}

}  // namespace scan_eval_internal
}  // namespace semantic
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
