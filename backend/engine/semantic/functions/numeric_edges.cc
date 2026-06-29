
#include <cstdint>

#include "absl/status/status.h"
#include "absl/strings/str_cat.h"
#include "backend/engine/semantic/error.h"
#include "googlesql/public/type.h"
#include "googlesql/public/type.pb.h"
#include "googlesql/public/value.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace semantic {
namespace functions {

absl::StatusOr<Value> BitCount(const std::vector<Value>& args) {
  if (args.size() != 1) {
    return absl::InvalidArgumentError(
        "semantic: BIT_COUNT expects exactly one argument");
  }
  const Value& v = args[0];
  if (v.is_null()) return Value::NullInt64();
  if (v.type_kind() == ::googlesql::TYPE_BYTES) {
    int64_t count = 0;
    for (unsigned char c : v.bytes_value()) {
      count += static_cast<int64_t>(std::bitset<8>(c).count());
    }
    return Value::Int64(count);
  }
  if (v.type_kind() != ::googlesql::TYPE_INT64) {
    return MakeSemanticError(
        SemanticErrorReason::kInvalidArgument,
        absl::StrCat("semantic: BIT_COUNT requires INT64 or BYTES, got ",
                     v.type()->DebugString()));
  }
  // BigQuery's contract counts bits in the two's-complement
  // representation. Casting INT64 -> UINT64 via the standard
  // memcpy-equivalent path preserves the bit pattern (the C++
  // standard guarantees the cast on uint64_t reinterprets the
  // two's-complement bits as the matching unsigned value). The
  // popcount is then well-defined for both positive and negative
  // inputs without dispatching on sign.
  uint64_t bits = static_cast<uint64_t>(v.int64_value());
  int64_t count = static_cast<int64_t>(std::bitset<64>(bits).count());
  return Value::Int64(count);
}

absl::StatusOr<Value> IeeeDivide(const std::vector<Value>& args) {
  if (args.size() != 2) {
    return absl::InvalidArgumentError(
        "semantic: IEEE_DIVIDE expects exactly two arguments");
  }
  const Value& a = args[0];
  const Value& b = args[1];
  if (a.is_null() || b.is_null()) return Value::NullDouble();
  if (a.type_kind() != ::googlesql::TYPE_DOUBLE ||
      b.type_kind() != ::googlesql::TYPE_DOUBLE) {
    return MakeSemanticError(
        SemanticErrorReason::kInvalidArgument,
        absl::StrCat("semantic: IEEE_DIVIDE requires FLOAT64 operands, got (",
                     a.type()->DebugString(),
                     ", ",
                     b.type()->DebugString(),
                     ")"));
  }
  // IEEE 754 division: never errors. The hardware (and the C++
  // language for `double / double` without `-ffast-math`)
  // produces +Inf for `n / 0`, -Inf for `-n / 0`, NaN for
  // `0 / 0`, and the usual finite result otherwise. We rely on
  // that contract directly -- there is no overflow check.
  return Value::Double(a.double_value() / b.double_value());
}

}  // namespace functions
}  // namespace semantic
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
