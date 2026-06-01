#include "backend/engine/semantic/functions/dispatch.h"

#include <optional>
#include <vector>

#include "absl/status/statusor.h"
#include "absl/strings/string_view.h"
#include "backend/engine/semantic/functions/numeric_edges.h"
#include "backend/engine/semantic/functions/string_funcs.h"
#include "backend/engine/semantic/value.h"
#include "googlesql/public/type.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace semantic {
namespace functions {

std::optional<absl::StatusOr<Value>> Dispatch(
    absl::string_view name,
    const std::vector<Value>& args,
    const ::googlesql::Type* return_type) {
  (void)return_type;
  // Numeric edges.
  if (name == "bit_count") return BitCount(args);
  if (name == "ieee_divide") return IeeeDivide(args);
  // String family.
  if (name == "soundex") return Soundex(args);
  if (name == "instr") return Instr(args);
  return std::nullopt;
}

}  // namespace functions
}  // namespace semantic
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
