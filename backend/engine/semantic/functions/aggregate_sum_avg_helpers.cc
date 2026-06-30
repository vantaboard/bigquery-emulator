#include <cmath>
#include <cstdint>
#include <optional>
#include <string>
#include <vector>

#include "absl/status/statusor.h"
#include "absl/strings/str_cat.h"
#include "backend/engine/semantic/error.h"
#include "backend/engine/semantic/value.h"
#include "googlesql/public/numeric_value.h"
#include "googlesql/public/type.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace semantic {
namespace functions {
namespace aggregate_sum_avg_internal {

absl::StatusOr<Value> NullOfAggregateType(const ::googlesql::Type* type) {
  if (type == nullptr) return Value::NullInt64();
  switch (type->kind()) {
    case ::googlesql::TYPE_INT64:
      return Value::NullInt64();
    case ::googlesql::TYPE_DOUBLE:
      return Value::NullDouble();
    case ::googlesql::TYPE_NUMERIC:
      return Value::NullNumeric();
    case ::googlesql::TYPE_BIGNUMERIC:
      return Value::NullBigNumeric();
    default:
      return Value::NullInt64();
  }
}

absl::StatusOr<Value> CoerceToNumeric(const Value& v) {
  if (v.type_kind() == ::googlesql::TYPE_NUMERIC) {
    return Value::Numeric(v.numeric_value());
  }
  if (v.type_kind() == ::googlesql::TYPE_INT64) {
    auto n =
        ::googlesql::NumericValue::FromString(absl::StrCat(v.int64_value()));
    if (!n.ok()) {
      return MakeSemanticError(SemanticErrorReason::kInvalidArgument,
                               n.status().message());
    }
    return Value::Numeric(*n);
  }
  if (v.type_kind() == ::googlesql::TYPE_DOUBLE) {
    auto n = ::googlesql::NumericValue::FromDouble(v.double_value());
    if (!n.ok()) {
      if (n.status().code() == absl::StatusCode::kOutOfRange) {
        return MakeSemanticError(SemanticErrorReason::kOverflow,
                                 n.status().message());
      }
      return MakeSemanticError(SemanticErrorReason::kInvalidArgument,
                               n.status().message());
    }
    return Value::Numeric(*n);
  }
  if (v.type_kind() == ::googlesql::TYPE_FLOAT) {
    auto n = ::googlesql::NumericValue::FromDouble(
        static_cast<double>(v.float_value()));
    if (!n.ok()) {
      if (n.status().code() == absl::StatusCode::kOutOfRange) {
        return MakeSemanticError(SemanticErrorReason::kOverflow,
                                 n.status().message());
      }
      return MakeSemanticError(SemanticErrorReason::kInvalidArgument,
                               n.status().message());
    }
    return Value::Numeric(*n);
  }
  return MakeSemanticError(SemanticErrorReason::kInvalidArgument,
                           absl::StrCat("semantic: cannot coerce ",
                                        v.type()->DebugString(),
                                        " to NUMERIC for SUM"));
}

absl::StatusOr<Value> SumNumericAggregateCells(
    const ::googlesql::Type* return_type,
    const std::vector<Value>& cells) {
  std::optional<::googlesql::NumericValue> total;
  bool any_non_null = false;
  for (const Value& v : cells) {
    if (v.is_null()) continue;
    any_non_null = true;
    auto coerced = CoerceToNumeric(v);
    if (!coerced.ok()) return coerced.status();
    if (!total.has_value()) {
      total = coerced->numeric_value();
    } else {
      auto sum = total->Add(coerced->numeric_value());
      if (!sum.ok()) {
        if (sum.status().code() == absl::StatusCode::kOutOfRange) {
          return MakeSemanticError(SemanticErrorReason::kOverflow,
                                   sum.status().message());
        }
        return MakeSemanticError(SemanticErrorReason::kInvalidArgument,
                                 sum.status().message());
      }
      total = *sum;
    }
  }
  if (!any_non_null) return NullOfAggregateType(return_type);
  return Value::Numeric(*total);
}

absl::StatusOr<Value> SumInt64Cells(const ::googlesql::Type* return_type,
                                    const std::vector<Value>& cells) {
  int64_t total = 0;
  bool any_non_null = false;
  for (const Value& v : cells) {
    if (v.is_null()) continue;
    if (v.type_kind() != ::googlesql::TYPE_INT64) {
      return MakeSemanticError(SemanticErrorReason::kInvalidArgument,
                               "semantic: SUM argument type mismatch");
    }
    any_non_null = true;
    int64_t next = 0;
    if (__builtin_add_overflow(total, v.int64_value(), &next)) {
      return MakeSemanticError(SemanticErrorReason::kOverflow,
                               "semantic: integer overflow");
    }
    total = next;
  }
  if (!any_non_null) return NullOfAggregateType(return_type);
  return Value::Int64(total);
}

absl::StatusOr<Value> SumDoubleCells(const ::googlesql::Type* return_type,
                                     const std::vector<Value>& cells) {
  double total = 0;
  bool any_non_null = false;
  for (const Value& v : cells) {
    if (v.is_null()) continue;
    if (v.type_kind() != ::googlesql::TYPE_DOUBLE) {
      return MakeSemanticError(SemanticErrorReason::kInvalidArgument,
                               "semantic: SUM argument type mismatch");
    }
    any_non_null = true;
    total += v.double_value();
  }
  if (!any_non_null) return NullOfAggregateType(return_type);
  return Value::Double(total);
}

absl::StatusOr<Value> SumNumericCells(const ::googlesql::Type* return_type,
                                      const std::vector<Value>& cells) {
  std::optional<::googlesql::NumericValue> total;
  bool any_non_null = false;
  for (const Value& v : cells) {
    if (v.is_null()) continue;
    if (v.type_kind() != ::googlesql::TYPE_NUMERIC) {
      return MakeSemanticError(SemanticErrorReason::kInvalidArgument,
                               "semantic: SUM argument type mismatch");
    }
    any_non_null = true;
    if (!total.has_value()) {
      total = v.numeric_value();
    } else {
      auto sum = total->Add(v.numeric_value());
      if (!sum.ok()) {
        if (sum.status().code() == absl::StatusCode::kOutOfRange) {
          return MakeSemanticError(SemanticErrorReason::kOverflow,
                                   sum.status().message());
        }
        return MakeSemanticError(SemanticErrorReason::kInvalidArgument,
                                 sum.status().message());
      }
      total = *sum;
    }
  }
  if (!any_non_null) return NullOfAggregateType(return_type);
  return Value::Numeric(*total);
}

absl::StatusOr<Value> AvgInt64Cells(const ::googlesql::Type* return_type,
                                    const std::vector<Value>& cells) {
  int64_t count = 0;
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
  if (count == 0) return NullOfAggregateType(return_type);
  return Value::Double(static_cast<double>(total) / static_cast<double>(count));
}

absl::StatusOr<Value> AvgDoubleCells(const ::googlesql::Type* return_type,
                                     const std::vector<Value>& cells) {
  int64_t count = 0;
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
  if (count == 0) return NullOfAggregateType(return_type);
  return Value::Double(total / static_cast<double>(count));
}

absl::StatusOr<Value> AvgNumericCells(const ::googlesql::Type* return_type,
                                      const std::vector<Value>& cells) {
  int64_t count = 0;
  std::optional<::googlesql::NumericValue> total;
  for (const Value& v : cells) {
    if (v.is_null()) continue;
    if (v.type_kind() != ::googlesql::TYPE_NUMERIC) {
      return MakeSemanticError(SemanticErrorReason::kInvalidArgument,
                               "semantic: AVG argument type mismatch");
    }
    ++count;
    if (!total.has_value()) {
      total = v.numeric_value();
    } else {
      auto sum = total->Add(v.numeric_value());
      if (!sum.ok()) {
        if (sum.status().code() == absl::StatusCode::kOutOfRange) {
          return MakeSemanticError(SemanticErrorReason::kOverflow,
                                   sum.status().message());
        }
        return MakeSemanticError(SemanticErrorReason::kInvalidArgument,
                                 sum.status().message());
      }
      total = *sum;
    }
  }
  if (count == 0) return NullOfAggregateType(return_type);
  auto avg = total->Divide(::googlesql::NumericValue(count));
  if (!avg.ok()) {
    if (avg.status().code() == absl::StatusCode::kOutOfRange) {
      return MakeSemanticError(SemanticErrorReason::kOverflow,
                               avg.status().message());
    }
    return MakeSemanticError(SemanticErrorReason::kInvalidArgument,
                             avg.status().message());
  }
  return Value::Numeric(*avg);
}

absl::StatusOr<Value> AvgBigNumericCells(const ::googlesql::Type* return_type,
                                         const std::vector<Value>& cells) {
  int64_t count = 0;
  std::optional<::googlesql::BigNumericValue> total;
  for (const Value& v : cells) {
    if (v.is_null()) continue;
    if (v.type_kind() != ::googlesql::TYPE_BIGNUMERIC) {
      return MakeSemanticError(SemanticErrorReason::kInvalidArgument,
                               "semantic: AVG argument type mismatch");
    }
    ++count;
    if (!total.has_value()) {
      total = v.bignumeric_value();
    } else {
      auto sum = total->Add(v.bignumeric_value());
      if (!sum.ok()) {
        if (sum.status().code() == absl::StatusCode::kOutOfRange) {
          return MakeSemanticError(SemanticErrorReason::kOverflow,
                                   sum.status().message());
        }
        return MakeSemanticError(SemanticErrorReason::kInvalidArgument,
                                 sum.status().message());
      }
      total = *sum;
    }
  }
  if (count == 0) return NullOfAggregateType(return_type);
  auto avg = total->Divide(::googlesql::BigNumericValue(count));
  if (!avg.ok()) {
    if (avg.status().code() == absl::StatusCode::kOutOfRange) {
      return MakeSemanticError(SemanticErrorReason::kOverflow,
                               avg.status().message());
    }
    return MakeSemanticError(SemanticErrorReason::kInvalidArgument,
                             avg.status().message());
  }
  return Value::BigNumeric(*avg);
}

bool ShouldReplaceMinMax(const Value& cur, const Value& v, bool pick_max) {
  switch (v.type_kind()) {
    case ::googlesql::TYPE_INT64:
      return pick_max ? v.int64_value() > cur.int64_value()
                      : v.int64_value() < cur.int64_value();
    case ::googlesql::TYPE_DOUBLE:
      return pick_max ? v.double_value() > cur.double_value()
                      : v.double_value() < cur.double_value();
    case ::googlesql::TYPE_NUMERIC:
      return pick_max ? v.numeric_value() > cur.numeric_value()
                      : v.numeric_value() < cur.numeric_value();
    case ::googlesql::TYPE_BIGNUMERIC:
      return pick_max ? v.bignumeric_value() > cur.bignumeric_value()
                      : v.bignumeric_value() < cur.bignumeric_value();
    case ::googlesql::TYPE_TIMESTAMP:
      return pick_max ? v.ToUnixMicros() > cur.ToUnixMicros()
                      : v.ToUnixMicros() < cur.ToUnixMicros();
    default:
      return false;
  }
}

bool IsSupportedMinMaxType(::googlesql::TypeKind kind) {
  switch (kind) {
    case ::googlesql::TYPE_INT64:
    case ::googlesql::TYPE_DOUBLE:
    case ::googlesql::TYPE_NUMERIC:
    case ::googlesql::TYPE_BIGNUMERIC:
    case ::googlesql::TYPE_TIMESTAMP:
      return true;
    default:
      return false;
  }
}

}  // namespace aggregate_sum_avg_internal
}  // namespace functions
}  // namespace semantic
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
