#ifndef BIGQUERY_EMULATOR_BACKEND_ENGINE_SEMANTIC_GEOGRAPHY_VALUE_H_
#define BIGQUERY_EMULATOR_BACKEND_ENGINE_SEMANTIC_GEOGRAPHY_VALUE_H_

#include <string>

#include "absl/status/statusor.h"
#include "googlesql/public/value.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace semantic {

// Build a non-null GEOGRAPHY Value whose WKT payload lives in the
// emulator-side registry (googlesql prebuilt GeographyRef is a stub).
::googlesql::Value GeographyFromWkt(std::string wkt);

// Returns WKT for emulator-backed geography values; empty for NULL or
// values without a registered payload.
std::string GeographyWkt(const ::googlesql::Value& value);

// SQL literal for FORMAT('%T', ...) / TO_JSON_STRING on geography.
absl::StatusOr<std::string> GeographySqlLiteral(const ::googlesql::Value& value);

}  // namespace semantic
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator

#endif  // BIGQUERY_EMULATOR_BACKEND_ENGINE_SEMANTIC_GEOGRAPHY_VALUE_H_
