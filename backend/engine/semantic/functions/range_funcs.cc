

#include "absl/status/status.h"
#include "backend/engine/semantic/error.h"
#include "googlesql/public/type.h"
#include "googlesql/public/type.pb.h"
#include "googlesql/public/types/range_type.h"
#include "googlesql/public/value.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace semantic {
namespace functions {

namespace {

using ::googlesql::Value;

Value NullForRangeElement(const Value& range) {
  const auto* rt = range.type()->AsRange();
  return Value::Null(rt->element_type());
}

bool BoundedLessOrEqual(const Value& bound, const Value& other) {
  return bound.LessThan(other) || bound.Equals(other);
}

bool RangesOverlap(const Value& a, const Value& b) {
  const Value& a_start = a.start();
  const Value& a_end = a.end();
  const Value& b_start = b.start();
  const Value& b_end = b.end();

  if (!a_end.is_null() && !b_start.is_null()) {
    if (BoundedLessOrEqual(a_end, b_start)) return false;
  }
  if (!b_end.is_null() && !a_start.is_null()) {
    if (BoundedLessOrEqual(b_end, a_start)) return false;
  }
  return true;
}

}  // namespace

absl::StatusOr<Value> RangeCtor(const std::vector<Value>& args,
                                const ::googlesql::Type* return_type) {
  (void)return_type;
  if (args.size() != 2) {
    return absl::InvalidArgumentError(
        "semantic: RANGE expects exactly two arguments");
  }
  auto range = Value::MakeRange(args[0], args[1]);
  if (!range.ok()) {
    return MakeSemanticError(SemanticErrorReason::kInvalidArgument,
                             range.status().message());
  }
  return *std::move(range);
}

absl::StatusOr<Value> RangeStart(const std::vector<Value>& args) {
  if (args.size() != 1) {
    return absl::InvalidArgumentError(
        "semantic: RANGE_START expects exactly one argument");
  }
  if (args[0].is_null()) {
    return Value::NullInt64();
  }
  if (args[0].type_kind() != ::googlesql::TYPE_RANGE) {
    return MakeSemanticError(SemanticErrorReason::kInvalidArgument,
                             "semantic: RANGE_START expects RANGE argument");
  }
  if (args[0].start().is_null()) {
    return NullForRangeElement(args[0]);
  }
  return args[0].start();
}

absl::StatusOr<Value> RangeEnd(const std::vector<Value>& args) {
  if (args.size() != 1) {
    return absl::InvalidArgumentError(
        "semantic: RANGE_END expects exactly one argument");
  }
  if (args[0].is_null()) {
    return Value::NullInt64();
  }
  if (args[0].type_kind() != ::googlesql::TYPE_RANGE) {
    return MakeSemanticError(SemanticErrorReason::kInvalidArgument,
                             "semantic: RANGE_END expects RANGE argument");
  }
  if (args[0].end().is_null()) {
    return NullForRangeElement(args[0]);
  }
  return args[0].end();
}

absl::StatusOr<Value> RangeOverlaps(const std::vector<Value>& args) {
  if (args.size() != 2) {
    return absl::InvalidArgumentError(
        "semantic: RANGE_OVERLAPS expects exactly two arguments");
  }
  if (args[0].is_null() || args[1].is_null()) {
    return Value::NullBool();
  }
  if (args[0].type_kind() != ::googlesql::TYPE_RANGE ||
      args[1].type_kind() != ::googlesql::TYPE_RANGE) {
    return MakeSemanticError(
        SemanticErrorReason::kInvalidArgument,
        "semantic: RANGE_OVERLAPS expects RANGE arguments");
  }
  return Value::Bool(RangesOverlap(args[0], args[1]));
}

}  // namespace functions
}  // namespace semantic
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
