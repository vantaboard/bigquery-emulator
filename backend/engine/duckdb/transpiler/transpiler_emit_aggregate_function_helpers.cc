#include "backend/engine/duckdb/transpiler/transpiler_emit_aggregate_function_helpers.h"

#include <string>
#include <vector>

#include "absl/log/log.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/str_join.h"
#include "backend/engine/disposition.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace duckdb {
namespace transpiler {

std::string EmitDispositionAggregateCall(
    absl::string_view name,
    const ::googlesql::ResolvedAggregateFunctionCall* node,
    const std::vector<std::string>& args,
    absl::string_view order_suffix,
    const FnEntry* entry,
    const std::string& filter_suffix) {
  if (entry == nullptr) {
    LOG(INFO) << "duckdb transpiler: aggregate '" << name
              << "' has no disposition; surfacing UNIMPLEMENTED";
    return "";
  }
  switch (entry->disposition) {
    case Disposition::kDuckdbNative:
    case Disposition::kDuckdbRewrite: {
      std::string prefix = node->distinct() ? "DISTINCT " : "";
      std::string body = absl::StrCat(entry->duckdb_name,
                                      "(",
                                      prefix,
                                      absl::StrJoin(args, ", "),
                                      order_suffix,
                                      ")");
      if (name == "countif") {
        body = absl::StrCat("coalesce(", body, ", 0)");
      }
      if (!filter_suffix.empty()) {
        body = absl::StrCat(body, filter_suffix);
      }
      return body;
    }
    case Disposition::kDuckdbUdf:
      if (!entry->planned && !entry->duckdb_name.empty()) {
        std::string prefix = node->distinct() ? "DISTINCT " : "";
        std::string body = absl::StrCat(
            entry->duckdb_name, "(", prefix, absl::StrJoin(args, ", "), ")");
        if (!filter_suffix.empty()) {
          body = absl::StrCat(body, filter_suffix);
        }
        return body;
      }
      LOG(INFO) << "duckdb transpiler: aggregate '" << name
                << "' route=duckdb_udf (planned=" << entry->planned
                << "); surfacing UNIMPLEMENTED";
      return "";
    case Disposition::kSemanticExecutor:
    case Disposition::kControlOp:
    case Disposition::kLocalStub:
      LOG(INFO) << "duckdb transpiler: aggregate '" << name
                << "' route=" << DispositionToString(entry->disposition)
                << " (planned=" << entry->planned
                << "); surfacing UNIMPLEMENTED";
      return "";
    case Disposition::kUnsupported:
      LOG(INFO) << "duckdb transpiler: aggregate '" << name
                << "' unsupported; surfacing UNIMPLEMENTED";
      return "";
  }
  return "";
}

}  // namespace transpiler
}  // namespace duckdb
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
