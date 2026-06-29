#ifndef BIGQUERY_EMULATOR_BACKEND_ENGINE_COORDINATOR_SQL_PREPROCESS_REWRITES_HELPERS_H_
#define BIGQUERY_EMULATOR_BACKEND_ENGINE_COORDINATOR_SQL_PREPROCESS_REWRITES_HELPERS_H_

#include <string>

#include "absl/strings/string_view.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace coordinator {
namespace sql_preprocess_internal {

bool IsIdentChar(char c);
bool MatchCaseInsensitiveAt(absl::string_view sql,
                            size_t pos,
                            absl::string_view literal);
bool IsWordBoundaryBefore(absl::string_view sql, size_t pos);
bool IsWordBoundaryAfter(absl::string_view sql, size_t pos, size_t lit_size);

bool TryAppendAnonymousStructOffsetRewrite(absl::string_view sql,
                                           size_t i,
                                           std::string* out,
                                           size_t* next_i);
bool TryAppendFormatPercentTRewrite(absl::string_view sql,
                                    size_t i,
                                    std::string* out,
                                    size_t* next_i);
bool TryAppendIntegerAliasRewrite(absl::string_view sql,
                                  size_t i,
                                  std::string* out,
                                  size_t* next_i);

bool AdvanceSingleQuoted(absl::string_view sql,
                         size_t i,
                         std::string* out,
                         bool* in_single_quoted,
                         size_t* next_i);
bool AdvanceDoubleQuoted(absl::string_view sql,
                         size_t i,
                         std::string* out,
                         bool* in_double_quoted,
                         size_t* next_i);

}  // namespace sql_preprocess_internal
}  // namespace coordinator
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator

#endif  // BIGQUERY_EMULATOR_BACKEND_ENGINE_COORDINATOR_SQL_PREPROCESS_REWRITES_HELPERS_H_
