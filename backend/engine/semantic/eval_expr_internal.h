#ifndef BIGQUERY_EMULATOR_BACKEND_ENGINE_SEMANTIC_EVAL_EXPR_INTERNAL_H_
#define BIGQUERY_EMULATOR_BACKEND_ENGINE_SEMANTIC_EVAL_EXPR_INTERNAL_H_

#include <optional>
#include <string>
#include <vector>

#include "absl/status/statusor.h"
#include "absl/strings/string_view.h"
#include "backend/engine/semantic/eval_context.h"
#include "backend/engine/semantic/value.h"
#include "googlesql/public/function.h"
#include "googlesql/public/type.h"
#include "googlesql/resolved_ast/resolved_ast.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace semantic {
namespace eval_expr_internal {

Value NullOfType(const ::googlesql::Type* type);
absl::StatusOr<double> ToDouble(const Value& v);
std::string LowerFunctionDispatchName(const ::googlesql::Function* fn);

absl::StatusOr<Value> ArithmeticAdd(const Value& a, const Value& b);
absl::StatusOr<Value> ArithmeticSub(const Value& a, const Value& b);
absl::StatusOr<Value> ArithmeticMul(const Value& a, const Value& b);
absl::StatusOr<Value> ArithmeticDiv(const Value& a, const Value& b);
absl::StatusOr<Value> UnaryMinus(const Value& a);

absl::StatusOr<Value> DispatchAdd(const std::vector<Value>& args,
                                  const ::googlesql::Type* return_type);
absl::StatusOr<Value> DispatchSub(const std::vector<Value>& args,
                                  const ::googlesql::Type* return_type);
absl::StatusOr<Value> DispatchMul(const std::vector<Value>& args,
                                  const ::googlesql::Type* return_type);
absl::StatusOr<Value> DispatchDiv(const std::vector<Value>& args,
                                  const ::googlesql::Type* return_type);
absl::StatusOr<Value> DispatchUnaryMinus(const std::vector<Value>& args,
                                         const ::googlesql::Type* return_type);
absl::StatusOr<Value> DispatchAbs(const std::vector<Value>& args,
                                  const ::googlesql::Type* return_type);
absl::StatusOr<Value> DispatchEqual(const std::vector<Value>& args);
absl::StatusOr<Value> DispatchNotEqual(const std::vector<Value>& args);
absl::StatusOr<Value> DispatchLess(const std::vector<Value>& args);
absl::StatusOr<Value> DispatchLessOrEqual(const std::vector<Value>& args);
absl::StatusOr<Value> DispatchGreater(const std::vector<Value>& args);
absl::StatusOr<Value> DispatchGreaterOrEqual(const std::vector<Value>& args);
absl::StatusOr<Value> DispatchAnd(const std::vector<Value>& args);
absl::StatusOr<Value> DispatchOr(const std::vector<Value>& args);
absl::StatusOr<Value> DispatchNot(const std::vector<Value>& args);
absl::StatusOr<Value> DispatchIsNull(const std::vector<Value>& args,
                                     bool negate);
absl::StatusOr<Value> DispatchCoalesce(const std::vector<Value>& args,
                                       const ::googlesql::Type* return_type);
absl::StatusOr<Value> DispatchIfNull(const std::vector<Value>& args);
absl::StatusOr<Value> DispatchNullIf(const std::vector<Value>& args,
                                     const ::googlesql::Type* return_type);
absl::StatusOr<Value> DispatchIf(const std::vector<Value>& args);
absl::StatusOr<Value> DispatchCaseWithValue(
    const std::vector<Value>& args, const ::googlesql::Type* return_type);
absl::StatusOr<Value> DispatchCaseNoValue(const std::vector<Value>& args,
                                          const ::googlesql::Type* return_type);
absl::StatusOr<Value> WrapSafe(absl::StatusOr<Value> result,
                               const ::googlesql::Type* return_type);

absl::StatusOr<Value> EvalResolvedCast(const ::googlesql::ResolvedCast& cast,
                                       Value inner,
                                       const ::googlesql::Type* source);

absl::StatusOr<Value> DispatchFunctionByName(
    absl::string_view name,
    const std::vector<Value>& args,
    const ::googlesql::Type* return_type,
    const EvalContext* ctx = nullptr);

}  // namespace eval_expr_internal
}  // namespace semantic
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator

#endif  // BIGQUERY_EMULATOR_BACKEND_ENGINE_SEMANTIC_EVAL_EXPR_INTERNAL_H_
