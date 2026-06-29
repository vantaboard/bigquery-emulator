#ifndef BIGQUERY_EMULATOR_BACKEND_CATALOG_GOOGLESQL_CATALOG_FIND_HELPERS_H_
#define BIGQUERY_EMULATOR_BACKEND_CATALOG_GOOGLESQL_CATALOG_FIND_HELPERS_H_

#include <optional>
#include <string>

#include "absl/status/statusor.h"
#include "absl/strings/string_view.h"
#include "absl/types/span.h"
#include "backend/catalog/googlesql_catalog.h"
#include "backend/catalog/info_schema_table.h"
#include "backend/schema/schema.h"
#include "googlesql/public/type.h"
#include "googlesql/public/types/type_factory.h"

namespace bigquery_emulator {
namespace backend {
namespace catalog {

struct ParsedFindTablePath {
  absl::string_view project_id;
  absl::string_view dataset_id;
  absl::string_view table_id;
  absl::string_view info_schema_view;
};

absl::StatusOr<ParsedFindTablePath> ParseFindTablePath(
    const absl::Span<const std::string>& path,
    absl::string_view project_id,
    absl::string_view default_dataset_id);

std::optional<InfoSchemaViewKind> ParseInfoSchemaView(
    absl::string_view view_name);

absl::StatusOr<const ::googlesql::Type*> ScalarOrStructType(
    const schema::ColumnSchema& column, ::googlesql::TypeFactory* type_factory);

}  // namespace catalog
}  // namespace backend
}  // namespace bigquery_emulator

#endif  // BIGQUERY_EMULATOR_BACKEND_CATALOG_GOOGLESQL_CATALOG_FIND_HELPERS_H_
