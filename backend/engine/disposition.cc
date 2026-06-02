#include "backend/engine/disposition.h"

#include "absl/strings/string_view.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {

absl::string_view DispositionToString(Disposition d) {
  switch (d) {
    case Disposition::kDuckdbNative:
      return "duckdb_native";
    case Disposition::kDuckdbRewrite:
      return "duckdb_rewrite";
    case Disposition::kDuckdbUdf:
      return "duckdb_udf";
    case Disposition::kSemanticExecutor:
      return "semantic_executor";
    case Disposition::kControlOp:
      return "control_op";
    case Disposition::kLocalStub:
      return "local_stub";
    case Disposition::kUnsupported:
      return "unsupported";
  }
  // Unreachable by construction (the switch is exhaustive and the
  // enum is the closed set above); kept so a `-Wreturn-type` warning
  // doesn't fire on toolchains that distrust enum switches.
  return "unknown";
}

}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
