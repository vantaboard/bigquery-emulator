
#include "googlesql/public/type.h"
#include "googlesql/public/value.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace duckdb {
namespace internal {

absl::StatusOr<std::int64_t> EvaluateForSystemTimeAsOfMs(
    const ::googlesql::ResolvedExpr& expr) {
  semantic::EvalContext ctx;
  absl::StatusOr<semantic::Value> value_or = semantic::EvalExpr(expr, ctx);
  if (!value_or.ok()) return value_or.status();
  const semantic::Value& value = *value_or;
  if (value.is_null()) {
    return absl::InvalidArgumentError(
        "FOR SYSTEM_TIME AS OF expression must not be NULL");
  }
  if (value.type_kind() != ::googlesql::TYPE_TIMESTAMP) {
    return absl::InvalidArgumentError(
        "FOR SYSTEM_TIME AS OF expression must resolve to TIMESTAMP");
  }
  return absl::ToUnixMillis(value.ToTime());
}

}  // namespace internal
}  // namespace duckdb
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
