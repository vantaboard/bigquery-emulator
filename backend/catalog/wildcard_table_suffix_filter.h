#ifndef BIGQUERY_EMULATOR_BACKEND_CATALOG_WILDCARD_TABLE_SUFFIX_FILTER_H_
#define BIGQUERY_EMULATOR_BACKEND_CATALOG_WILDCARD_TABLE_SUFFIX_FILTER_H_

#include <optional>
#include <string>
#include <vector>

#include "absl/strings/string_view.h"
#include "backend/catalog/wildcard_table_util.h"

namespace googlesql {
class ResolvedExpr;
class ResolvedScan;
class ResolvedTableScan;
}  // namespace googlesql

namespace bigquery_emulator {
namespace backend {
namespace catalog {

// When `filter_expr` is a constant predicate on `_TABLE_SUFFIX` (`=`, `IN`,
// `BETWEEN`, or `AND` of those), returns the allowed suffix strings. When the
// predicate cannot be interpreted, returns `std::nullopt` (materialize all
// matched tables). When the predicate is provably unsatisfiable, returns an
// empty vector.
std::optional<std::vector<std::string>> ExtractTableSuffixAllowList(
    const ::googlesql::ResolvedExpr* filter_expr);

// Walks `scan` for a FilterScan wrapping a TableScan on `wildcard_table_id`
// and returns the suffix allow-list when present.
std::optional<std::vector<std::string>> FindTableSuffixAllowListForWildcardScan(
    const ::googlesql::ResolvedScan* scan, absl::string_view wildcard_table_id);

}  // namespace catalog
}  // namespace backend
}  // namespace bigquery_emulator

#endif  // BIGQUERY_EMULATOR_BACKEND_CATALOG_WILDCARD_TABLE_SUFFIX_FILTER_H_
