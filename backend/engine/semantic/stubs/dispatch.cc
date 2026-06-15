#include "backend/engine/semantic/stubs/dispatch.h"

#include <optional>
#include <vector>

#include "absl/status/statusor.h"
#include "absl/strings/string_view.h"
#include "backend/engine/semantic/error.h"
#include "backend/engine/semantic/stubs/keys.h"
#include "backend/engine/semantic/value.h"
#include "googlesql/public/type.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace semantic {
namespace stubs {

std::optional<absl::StatusOr<Value>> Dispatch(
    absl::string_view name,
    const std::vector<Value>& args,
    const ::googlesql::Type* return_type) {
  (void)return_type;
  // KEYS.* family. The dotted, lowercased BigQuery name is the
  // dispatch key; the route classifier promotes the surrounding
  // query to `kLocalStub` based on the same YAML key, so a hit
  // here corresponds exactly to a `local_stub` row in
  // `functions.yaml`. All four KEYS.* scalars are registered on the catalog
  // for analysis; execution is handled here on the semantic stub lane.
  if (name == "keys.new_keyset") return KeysNewKeyset(args);
  if (name == "keys.keyset_length") return KeysKeysetLength(args);
  if (name == "keys.encrypt") return KeysEncrypt(args);
  if (name == "keys.decrypt_bytes") return KeysDecryptBytes(args);
  if (name == "session_user") {
    if (!args.empty()) {
      return MakeSemanticError(SemanticErrorReason::kInvalidArgument,
                               "semantic stub: SESSION_USER expects no args");
    }
    return Value::String("bigquery-emulator@local");
  }
  return std::nullopt;
}

}  // namespace stubs
}  // namespace semantic
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
