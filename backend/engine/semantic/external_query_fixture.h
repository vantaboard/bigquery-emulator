#ifndef BIGQUERY_EMULATOR_BACKEND_ENGINE_SEMANTIC_EXTERNAL_QUERY_FIXTURE_H_
#define BIGQUERY_EMULATOR_BACKEND_ENGINE_SEMANTIC_EXTERNAL_QUERY_FIXTURE_H_

#include <string>
#include <vector>

#include "absl/status/statusor.h"
#include "absl/strings/string_view.h"
#include "backend/engine/semantic/eval_context.h"
#include "googlesql/public/type.h"
#include "googlesql/public/types/type_factory.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace semantic {

struct ExternalQueryFixtureColumn {
  std::string name;
  std::string type;
};

struct ExternalQueryFixtureResult {
  std::vector<ExternalQueryFixtureColumn> schema;
  std::vector<ColumnBindings> rows;
  std::vector<int> column_ids;
};

// Resolve fixture rows for EXTERNAL_QUERY(connection, query). Reads
// $BIGQUERY_EMULATOR_DATA_DIR/external/connections/<conn_id>/.
absl::StatusOr<ExternalQueryFixtureResult> LoadExternalQueryFixture(
    absl::string_view connection_arg,
    absl::string_view query_sql,
    ::googlesql::TypeFactory* type_factory);

}  // namespace semantic
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator

#endif  // BIGQUERY_EMULATOR_BACKEND_ENGINE_SEMANTIC_EXTERNAL_QUERY_FIXTURE_H_
