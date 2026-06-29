#ifndef BIGQUERY_EMULATOR_BACKEND_SQLTOOLS_SQL_TOOLS_COMPLETE_HELPERS_H_
#define BIGQUERY_EMULATOR_BACKEND_SQLTOOLS_SQL_TOOLS_COMPLETE_HELPERS_H_

#include <string>
#include <vector>

#include "absl/strings/string_view.h"
#include "googlesql/public/parse_tokens.h"

namespace bigquery_emulator {
namespace backend {
namespace sqltools {
namespace sql_tools_complete_internal {

enum class CompletionContextKind {
  kGeneral,
  kTable,
  kColumn,
  kMember,
  kRoutine,
};

struct CompletionContext {
  CompletionContextKind kind = CompletionContextKind::kGeneral;
  std::string qualifier;
};

CompletionContext InferCompletionContext(
    const std::vector<::googlesql::ParseToken>& tokens);

void FindReplacementSpan(absl::string_view sql,
                         size_t cursor,
                         CompletionContextKind context_kind,
                         int* replacement_start,
                         int* replacement_end,
                         std::string* prefix);

}  // namespace sql_tools_complete_internal
}  // namespace sqltools
}  // namespace backend
}  // namespace bigquery_emulator

#endif  // BIGQUERY_EMULATOR_BACKEND_SQLTOOLS_SQL_TOOLS_COMPLETE_HELPERS_H_
