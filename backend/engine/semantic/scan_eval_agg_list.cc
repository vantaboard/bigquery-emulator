#include <algorithm>
#include <string>
#include <utility>
#include <vector>

#include "absl/status/statusor.h"
#include "backend/engine/semantic/error.h"
#include "backend/engine/semantic/scan_eval_internal.h"
#include "backend/engine/semantic/value.h"
#include "googlesql/public/type.h"
#include "googlesql/resolved_ast/resolved_ast.h"
#include "googlesql/resolved_ast/resolved_node_kind.pb.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace semantic {
namespace scan_eval_internal {

using ::bigquery_emulator::backend::engine::semantic::EvalContext;
using ::bigquery_emulator::backend::engine::semantic::EvalExpr;

bool ValueLess(const Value& a, const Value& b) {
  if (a.is_null() && b.is_null()) return false;
  if (a.is_null()) return true;
  if (b.is_null()) return false;
  if (a.type_kind() != b.type_kind()) {
    return static_cast<int>(a.type_kind()) < static_cast<int>(b.type_kind());
  }
  switch (a.type_kind()) {
    case ::googlesql::TYPE_BOOL:
      return !a.bool_value() && b.bool_value();
    case ::googlesql::TYPE_INT64:
      return a.int64_value() < b.int64_value();
    case ::googlesql::TYPE_DOUBLE:
      return a.double_value() < b.double_value();
    case ::googlesql::TYPE_STRING:
      return a.string_value() < b.string_value();
    case ::googlesql::TYPE_BYTES:
      return a.bytes_value() < b.bytes_value();
    default:
      return a.DebugString() < b.DebugString();
  }
}

bool ValueEqual(const Value& a, const Value& b) {
  if (a.is_null() && b.is_null()) return true;
  if (a.is_null() || b.is_null()) return false;
  return a.Equals(b);
}

int CompareArrayAggOrderKey(const ::googlesql::ResolvedOrderByItem& item,
                            const Value& va,
                            const Value& vb) {
  if (ValueEqual(va, vb)) return 0;
  if (va.is_null() || vb.is_null()) {
    bool nulls_first = false;
    switch (item.null_order()) {
      case ::googlesql::ResolvedOrderByItem::NULLS_FIRST:
        nulls_first = true;
        break;
      case ::googlesql::ResolvedOrderByItem::NULLS_LAST:
        nulls_first = false;
        break;
      default:
        nulls_first = !item.is_descending();
        break;
    }
    if (va.is_null() && vb.is_null()) return 0;
    if (va.is_null()) return nulls_first ? -1 : 1;
    return nulls_first ? 1 : -1;
  }
  return (item.is_descending() ? !ValueLess(va, vb) : ValueLess(va, vb)) ? -1
                                                                         : 1;
}

namespace {

struct AggListRow {
  Value val{};
  std::vector<Value> sort_keys{};
};

absl::StatusOr<std::vector<AggListRow>> CollectOrderedAggRows(
    const ::googlesql::ResolvedAggregateFunctionCall& call,
    const std::vector<std::vector<Value>>& input_column_values,
    const std::vector<ColumnBindings>& input_rows,
    bool ignore_nulls,
    const EvalContext& ctx) {
  std::vector<AggListRow> rows;
  rows.reserve(input_rows.size());
  for (size_t r = 0; r < input_rows.size(); ++r) {
    const Value& v = input_column_values[0][r];
    if (ignore_nulls && v.is_null()) continue;
    AggListRow row;
    row.val = v;
    EvalContext row_ctx = ctx;
    row_ctx.columns = &input_rows[r];
    row.sort_keys.reserve(call.order_by_item_list_size());
    for (int i = 0; i < call.order_by_item_list_size(); ++i) {
      const ::googlesql::ResolvedOrderByItem* item = call.order_by_item_list(i);
      if (item == nullptr || item->column_ref() == nullptr) {
        return MakeSemanticError(
            SemanticErrorReason::kInvalidArgument,
            "semantic: aggregate ORDER BY requires column references");
      }
      auto key_or = EvalExpr(*item->column_ref(), row_ctx);
      if (!key_or.ok()) return key_or.status();
      row.sort_keys.push_back(*std::move(key_or));
    }
    rows.push_back(std::move(row));
  }
  return rows;
}

std::vector<AggListRow> DedupAggRows(std::vector<AggListRow> rows) {
  std::vector<AggListRow> deduped;
  for (AggListRow& row : rows) {
    bool seen = false;
    for (const AggListRow& prior : deduped) {
      if (ValueEqual(prior.val, row.val)) {
        seen = true;
        break;
      }
    }
    if (!seen) deduped.push_back(std::move(row));
  }
  return deduped;
}

absl::StatusOr<std::vector<AggListRow>> SortAggRows(
    const ::googlesql::ResolvedAggregateFunctionCall& call,
    std::vector<AggListRow> rows) {
  if (call.order_by_item_list_size() == 0) return rows;
  std::stable_sort(
      rows.begin(), rows.end(), [&](const AggListRow& a, const AggListRow& b) {
        for (int i = 0; i < call.order_by_item_list_size(); ++i) {
          const ::googlesql::ResolvedOrderByItem* item =
              call.order_by_item_list(i);
          switch (
              CompareArrayAggOrderKey(*item, a.sort_keys[i], b.sort_keys[i])) {
            case -1:
              return true;
            case 1:
              return false;
            default:
              break;
          }
        }
        return false;
      });
  return rows;
}

absl::StatusOr<std::vector<AggListRow>> ApplyAggRowLimit(
    const ::googlesql::ResolvedAggregateFunctionCall& call,
    const EvalContext& ctx,
    std::vector<AggListRow> rows) {
  if (call.limit() == nullptr) return rows;
  auto limit_or = EvalExpr(*call.limit(), ctx);
  if (!limit_or.ok()) return limit_or.status();
  if (limit_or->is_null()) {
    return MakeSemanticError(SemanticErrorReason::kInvalidArgument,
                             "semantic: aggregate LIMIT must not be NULL");
  }
  if (limit_or->type_kind() != ::googlesql::TYPE_INT64) {
    return MakeSemanticError(SemanticErrorReason::kInvalidArgument,
                             "semantic: aggregate LIMIT must be INT64");
  }
  const int64_t limit = limit_or->int64_value();
  if (limit < 0) {
    return MakeSemanticError(SemanticErrorReason::kInvalidArgument,
                             "semantic: aggregate LIMIT must be non-negative");
  }
  if (rows.size() > static_cast<size_t>(limit)) {
    rows.resize(static_cast<size_t>(limit));
  }
  return rows;
}

}  // namespace

absl::StatusOr<Value> EvalArrayAgg(
    const ::googlesql::ResolvedAggregateFunctionCall& call,
    const std::vector<std::vector<Value>>& input_column_values,
    const std::vector<ColumnBindings>& input_rows,
    const EvalContext& ctx) {
  if (input_column_values.size() != 1) {
    return MakeSemanticError(SemanticErrorReason::kInvalidArgument,
                             "semantic: ARRAY_AGG expects one argument");
  }
  const ::googlesql::Type* out_type = call.type();
  if (out_type == nullptr || !out_type->IsArray()) {
    return MakeSemanticError(SemanticErrorReason::kInvalidArgument,
                             "semantic: ARRAY_AGG return type must be ARRAY");
  }
  const ::googlesql::ArrayType* arr_type = out_type->AsArray();
  const bool ignore_nulls =
      call.null_handling_modifier() ==
      ::googlesql::ResolvedNonScalarFunctionCallBase::IGNORE_NULLS;
  const bool distinct = call.distinct();

  auto rows_or = CollectOrderedAggRows(
      call, input_column_values, input_rows, ignore_nulls, ctx);
  if (!rows_or.ok()) return rows_or.status();
  std::vector<AggListRow> rows = *std::move(rows_or);
  if (distinct) {
    rows = DedupAggRows(std::move(rows));
  }
  if (auto sorted = SortAggRows(call, std::move(rows)); !sorted.ok()) {
    return sorted.status();
  } else {
    rows = *std::move(sorted);
  }
  if (auto limited = ApplyAggRowLimit(call, ctx, std::move(rows));
      !limited.ok()) {
    return limited.status();
  } else {
    rows = *std::move(limited);
  }
  std::vector<Value> elements;
  elements.reserve(rows.size());
  for (const AggListRow& row : rows) {
    elements.push_back(row.val);
  }
  return Value::Array(arr_type, std::move(elements));
}

std::string StringifyAggValue(const Value& v) {
  if (v.is_null()) return "";
  if (v.type_kind() == ::googlesql::TYPE_STRING) {
    return std::string(v.string_value());
  }
  if (v.type_kind() == ::googlesql::TYPE_BYTES) {
    return std::string(v.bytes_value());
  }
  return v.DebugString();
}

absl::StatusOr<Value> EvalStringAgg(
    const ::googlesql::ResolvedAggregateFunctionCall& call,
    const std::vector<std::vector<Value>>& input_column_values,
    const std::vector<ColumnBindings>& input_rows,
    const EvalContext& ctx) {
  if (input_column_values.empty()) {
    return MakeSemanticError(SemanticErrorReason::kInvalidArgument,
                             "semantic: STRING_AGG expects one argument");
  }
  std::string delimiter = ",";
  if (call.argument_list_size() >= 2) {
    if (input_column_values.size() < 2 || input_column_values[1].empty()) {
      return MakeSemanticError(SemanticErrorReason::kInvalidArgument,
                               "semantic: STRING_AGG delimiter missing");
    }
    const Value& delim = input_column_values[1][0];
    if (delim.is_null()) return Value::NullString();
    if (delim.type_kind() != ::googlesql::TYPE_STRING) {
      return MakeSemanticError(SemanticErrorReason::kInvalidArgument,
                               "semantic: STRING_AGG delimiter must be STRING");
    }
    delimiter = std::string(delim.string_value());
  }
  const bool ignore_nulls =
      call.null_handling_modifier() ==
      ::googlesql::ResolvedNonScalarFunctionCallBase::IGNORE_NULLS;
  const bool distinct = call.distinct();

  auto rows_or = CollectOrderedAggRows(
      call, input_column_values, input_rows, ignore_nulls, ctx);
  if (!rows_or.ok()) return rows_or.status();
  std::vector<AggListRow> rows = *std::move(rows_or);
  if (distinct) {
    rows = DedupAggRows(std::move(rows));
  }
  if (auto sorted = SortAggRows(call, std::move(rows)); !sorted.ok()) {
    return sorted.status();
  } else {
    rows = *std::move(sorted);
  }
  if (auto limited = ApplyAggRowLimit(call, ctx, std::move(rows));
      !limited.ok()) {
    return limited.status();
  } else {
    rows = *std::move(limited);
  }

  if (rows.empty()) {
    if (ignore_nulls) {
      return Value::NullString();
    }
    return Value::String("");
  }
  std::string out;
  bool appended = false;
  for (const AggListRow& row : rows) {
    if (row.val.is_null()) continue;
    if (appended) out.append(delimiter);
    out.append(StringifyAggValue(row.val));
    appended = true;
  }
  if (!appended) {
    return Value::NullString();
  }
  return Value::String(std::move(out));
}

}  // namespace scan_eval_internal
}  // namespace semantic
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
