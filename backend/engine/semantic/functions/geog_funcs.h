#ifndef BIGQUERY_EMULATOR_BACKEND_ENGINE_SEMANTIC_FUNCTIONS_GEOG_FUNCS_H_
#define BIGQUERY_EMULATOR_BACKEND_ENGINE_SEMANTIC_FUNCTIONS_GEOG_FUNCS_H_

#include <string>
#include <vector>

#include "absl/status/statusor.h"
#include "backend/engine/semantic/value.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace semantic {
namespace functions {

absl::StatusOr<Value> StGeogPoint(const std::vector<Value>& args);

// Used by FORMAT('%T', ...) for geography values.
absl::StatusOr<std::string> GeographyTypeLiteralForFormat();

absl::StatusOr<Value> EmuFormatTypeLiteral(const std::vector<Value>& args);

}  // namespace functions
}  // namespace semantic
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator

#endif  // BIGQUERY_EMULATOR_BACKEND_ENGINE_SEMANTIC_FUNCTIONS_GEOG_FUNCS_H_
