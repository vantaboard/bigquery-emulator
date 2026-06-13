#ifndef BIGQUERY_EMULATOR_BACKEND_ENGINE_COORDINATOR_SQL_PREPROCESS_INTERNAL_H_
#define BIGQUERY_EMULATOR_BACKEND_ENGINE_COORDINATOR_SQL_PREPROCESS_INTERNAL_H_

#include <string>

#include "absl/strings/string_view.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace coordinator {
namespace sql_preprocess_internal {

std::string PreprocessFunctionBodyBase(absl::string_view sql);
std::string NormalizeCreateFunctionAsParens(absl::string_view sql);
bool HasDecoratorAndSystemTimeConflict(absl::string_view sql);
std::string LowerTableDecorators(absl::string_view sql);

}  // namespace sql_preprocess_internal
}  // namespace coordinator
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator

#endif  // BIGQUERY_EMULATOR_BACKEND_ENGINE_COORDINATOR_SQL_PREPROCESS_INTERNAL_H_
