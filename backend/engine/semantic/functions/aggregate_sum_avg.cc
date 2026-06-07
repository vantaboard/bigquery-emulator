#include <cmath>
#include <cstdint>
#include <optional>
#include <string>
#include <vector>

#include "absl/status/statusor.h"
#include "absl/strings/str_cat.h"
#include "backend/engine/semantic/error.h"
#include "backend/engine/semantic/functions/specialized_funcs.h"
#include "backend/engine/semantic/value.h"
#include "googlesql/public/type.h"
#include "googlesql/resolved_ast/resolved_ast.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace semantic {
namespace functions {

namespace {

absl::StatusOr<Value> NullOfAggregateType(const ::googlesql::Type* type) {
  if (type == nullptr) return Value::NullInt64();
  switch (type->kind()) {
    case ::googlesql::TYPE_INT64:
      return Value::NullInt64();
    case ::googlesql::TYPE_DOUBLE:
      return Value::NullDouble();
    case ::googlesql::TYPE_NUMERIC:
      return Value::NullNumeric();
    default:
      return Value::NullInt64();
  }
}

template <typename CallLike>
absl::StatusOr<Value> SumAggregateImpl(
    const CallLike& call,
    const std::vector<std::vector<Value>>& input_column_values) {
  if (input_column_values.size() != 1) {
    return MakeSemanticError(SemanticErrorReason::kInvalidArgument,
                             "semantic: SUM expects one argument column");
  }
  if (input_column_values[0].empty()) {
    return NullOfAggregateType(call.type());
  }
  const ::googlesql::Type* out_type = call.type();
  const std::vector<Value>& cells = input_column_values[0];
  bool any_non_null = false;
  switch (cells.front().type_kind()) {
    case ::googlesql::TYPE_INT64: {
      int64_t total = 0;
      for (const Value& v : cells) {
        if (v.is_null()) continue;
        if (v.type_kind() != ::googlesql::TYPE_INT64) {
          return MakeSemanticError(SemanticErrorReason::kInvalidArgument,
                                   "semantic: SUM argument type mismatch");
        }
        any_non_null = true;
        total += v.int64_value();
      }
      if (!any_non_null) return NullOfAggregateType(out_type);
      return Value::Int64(total);
    }
    case ::googlesql::TYPE_DOUBLE: {
      double total = 0;
      for (const Value& v : cells) {
        if (v.is_null()) continue;
        if (v.type_kind() != ::googlesql::TYPE_DOUBLE) {
          return MakeSemanticError(SemanticErrorReason::kInvalidArgument,
                                   "semantic: SUM argument type mismatch");
        }
        any_non_null = true;
        total += v.double_value();
      }
      if (!any_non_null) return NullOfAggregateType(out_type);
      return Value::Double(total);
    }
    default:
      return MakeSemanticError(
          SemanticErrorReason::kNotImplemented,
          "semantic: SUM is not implemented for this argument type");
  }
}

template <typename CallLike>
absl::StatusOr<Value> AvgAggregateImpl(
    const CallLike& call,
    const std::vector<std::vector<Value>>& input_column_values) {
  if (input_column_values.size() != 1 || input_column_values[0].empty()) {
    return NullOfAggregateType(call.type());
  }
  const std::vector<Value>& cells = input_column_values[0];
  int64_t count = 0;
  switch (cells.front().type_kind()) {
    case ::googlesql::TYPE_INT64: {
      int64_t total = 0;
      for (const Value& v : cells) {
        if (v.is_null()) continue;
        if (v.type_kind() != ::googlesql::TYPE_INT64) {
          return MakeSemanticError(SemanticErrorReason::kInvalidArgument,
                                   "semantic: AVG argument type mismatch");
        }
        ++count;
        total += v.int64_value();
      }
      if (count == 0) return NullOfAggregateType(call.type());
      return Value::Double(static_cast<double>(total) /
                           static_cast<double>(count));
    }
    case ::googlesql::TYPE_DOUBLE: {
      double total = 0;
      for (const Value& v : cells) {
        if (v.is_null()) continue;
        if (v.type_kind() != ::googlesql::TYPE_DOUBLE) {
          return MakeSemanticError(SemanticErrorReason::kInvalidArgument,
                                   "semantic: AVG argument type mismatch");
        }
        ++count;
        total += v.double_value();
      }
      if (count == 0) return NullOfAggregateType(call.type());
      return Value::Double(total / static_cast<double>(count));
    }
    default:
      return MakeSemanticError(
          SemanticErrorReason::kNotImplemented,
          "semantic: AVG is not implemented for this argument type");
  }
}

struct BuiltinAggregateView {
  const ::googlesql::Type* return_type;
  bool distinct_flag;
  int argument_list_size() const {
    return 1;
  }
  const ::googlesql::Type* type() const {
    return return_type;
  }
  bool distinct() const {
    return distinct_flag;
  }
};

template <typename CallLike>
absl::StatusOr<Value> MinMaxAggregateImpl(
    const CallLike& call,
    const std::vector<std::vector<Value>>& input_column_values,
    bool pick_max) {
  if (input_column_values.size() != 1) {
    return MakeSemanticError(SemanticErrorReason::kInvalidArgument,
                             "semantic: MIN/MAX expects one argument column");
  }
  if (input_column_values[0].empty()) {
    return NullOfAggregateType(call.type());
  }
  const std::vector<Value>& cells = input_column_values[0];
  std::optional<Value> best;
  for (const Value& v : cells) {
    if (v.is_null()) continue;
    if (!best.has_value()) {
      best = v;
      continue;
    }
    const Value& cur = *best;
    if (cur.type_kind() != v.type_kind()) {
      return MakeSemanticError(SemanticErrorReason::kInvalidArgument,
                               "semantic: MIN/MAX argument type mismatch");
    }
    switch (v.type_kind()) {
      case ::googlesql::TYPE_INT64: {
        const bool take = pick_max ? v.int64_value() > cur.int64_value()
                                   : v.int64_value() < cur.int64_value();
        if (take) best = v;
        break;
      }
      case ::googlesql::TYPE_DOUBLE: {
        const bool take = pick_max ? v.double_value() > cur.double_value()
                                   : v.double_value() < cur.double_value();
        if (take) best = v;
        break;
      }
      default:
        return MakeSemanticError(
            SemanticErrorReason::kNotImplemented,
            "semantic: MIN/MAX is not implemented for this argument type");
    }
  }
  if (!best.has_value()) return NullOfAggregateType(call.type());
  return *best;
}

}  // namespace

absl::StatusOr<Value> SumAggregate(
    const ::googlesql::ResolvedAggregateFunctionCall& call,
    const std::vector<std::vector<Value>>& input_column_values) {
  return SumAggregateImpl(call, input_column_values);
}

absl::StatusOr<Value> AvgAggregate(
    const ::googlesql::ResolvedAggregateFunctionCall& call,
    const std::vector<std::vector<Value>>& input_column_values) {
  return AvgAggregateImpl(call, input_column_values);
}

absl::StatusOr<Value> MinAggregate(
    const ::googlesql::ResolvedAggregateFunctionCall& call,
    const std::vector<std::vector<Value>>& input_column_values) {
  return MinMaxAggregateImpl(call, input_column_values, /*pick_max=*/false);
}

absl::StatusOr<Value> MaxAggregate(
    const ::googlesql::ResolvedAggregateFunctionCall& call,
    const std::vector<std::vector<Value>>& input_column_values) {
  return MinMaxAggregateImpl(call, input_column_values, /*pick_max=*/true);
}

absl::StatusOr<Value> EvalAggregateBuiltin(
    absl::string_view name,
    const ::googlesql::Type* return_type,
    bool distinct,
    const std::vector<std::vector<Value>>& input_column_values) {
  BuiltinAggregateView view{return_type, distinct};
  if (name == "sum") {
    return SumAggregateImpl(view, input_column_values);
  }
  if (name == "avg") {
    return AvgAggregateImpl(view, input_column_values);
  }
  if (name == "min") {
    return MinMaxAggregateImpl(view, input_column_values, /*pick_max=*/false);
  }
  if (name == "max") {
    return MinMaxAggregateImpl(view, input_column_values, /*pick_max=*/true);
  }
  return MakeSemanticError(
      SemanticErrorReason::kNotImplemented,
      absl::StrCat("semantic: aggregate '", name, "' is not implemented"));
}

}  // namespace functions
}  // namespace semantic
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
