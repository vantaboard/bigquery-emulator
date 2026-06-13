#ifndef BIGQUERY_EMULATOR_BACKEND_CATALOG_WILDCARD_TABLE_UTIL_H_
#define BIGQUERY_EMULATOR_BACKEND_CATALOG_WILDCARD_TABLE_UTIL_H_

#include <string>
#include <vector>

#include "absl/status/statusor.h"
#include "absl/strings/string_view.h"
#include "backend/schema/schema.h"
#include "backend/storage/storage.h"

namespace bigquery_emulator {
namespace backend {
namespace catalog {

inline constexpr char kTableSuffixColumnName[] = "_TABLE_SUFFIX";

// Prefix matched by a wildcard table id such as `events_*` -> `events_`.
bool IsWildcardTableId(absl::string_view table_id);

std::string WildcardTablePrefix(absl::string_view wildcard_table_id);

bool TableMatchesWildcard(absl::string_view table_id,
                          absl::string_view wildcard_table_id);

// Suffix portion of `table_id` relative to `prefix` (the wildcard match).
std::string TableSuffixFor(absl::string_view table_id,
                           absl::string_view prefix);

// Merge schemas across `matched` tables. Column order follows the
// lexicographically last table (proxy for BigQuery's "most recently
// created" table), with extra columns from earlier tables appended.
// Appends the synthetic `_TABLE_SUFFIX` STRING column.
absl::StatusOr<schema::TableSchema> UnifyWildcardTableSchemas(
    const storage::Storage* storage,
    const std::vector<storage::TableId>& matched);

// Per matched table: map each union-schema column index to the
// physical row cell index, or -1 when the table lacks the column.
struct WildcardColumnMap {
  storage::TableId table_id;
  schema::TableSchema schema;
  std::vector<int> union_to_physical;
};

absl::StatusOr<std::vector<WildcardColumnMap>> BuildWildcardColumnMaps(
    const storage::Storage* storage,
    const std::vector<storage::TableId>& matched,
    const schema::TableSchema& union_schema);

// Returns true when `suffix` satisfies a constant `_TABLE_SUFFIX` predicate.
// For `=`, `IN`, or a single-element list. For `BETWEEN`, `allowlist` must
// contain exactly two bounds (inclusive, lexicographic).
bool SuffixMatchesAllowList(absl::string_view suffix,
                            const std::vector<std::string>& allowlist);

}  // namespace catalog
}  // namespace backend
}  // namespace bigquery_emulator

#endif  // BIGQUERY_EMULATOR_BACKEND_CATALOG_WILDCARD_TABLE_UTIL_H_
