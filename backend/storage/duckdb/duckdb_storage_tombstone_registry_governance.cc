#include <cstdint>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "absl/status/status.h"
#include "absl/strings/numbers.h"
#include "absl/strings/string_view.h"
#include "backend/storage/duckdb/duckdb_storage_governance.h"
#include "backend/storage/duckdb/duckdb_storage_tombstone_registry_internal.h"
#include "backend/storage/storage.h"
#include "backend/storage/table_governance.h"

namespace bigquery_emulator {
namespace backend {
namespace storage {
namespace duckdb {
namespace tombstone_registry_internal {

namespace {

std::optional<absl::string_view> JsonArraySlice(absl::string_view entry,
                                                absl::string_view key) {
  const size_t key_pos = entry.find(key);
  if (key_pos == absl::string_view::npos) {
    return std::nullopt;
  }
  const size_t open = entry.find('[', key_pos);
  const size_t close = entry.find(']', open);
  if (open == absl::string_view::npos || close == absl::string_view::npos) {
    return std::nullopt;
  }
  return entry.substr(open, close - open + 1);
}

std::int64_t ParseJsonInt64Field(absl::string_view json,
                                 absl::string_view needle) {
  const size_t pos = json.find(needle);
  if (pos == absl::string_view::npos) {
    return 0;
  }
  std::int64_t value = 0;
  (void)absl::SimpleAtoi(json.substr(pos + needle.size()), &value);
  return value;
}

RowAccessPolicyRecord ParseRowAccessPolicy(absl::string_view policy_json) {
  RowAccessPolicyRecord policy;
  auto policy_id_or = ParseJsonStringField(policy_json, "policy_id");
  auto filter_or = ParseJsonStringField(policy_json, "filter_predicate");
  if (!policy_id_or.ok() || !filter_or.ok()) {
    return policy;
  }
  policy.policy_id = *policy_id_or;
  policy.filter_predicate = *filter_or;
  if (auto grantees_slice = JsonArraySlice(policy_json, "\"grantees\"")) {
    policy.grantees = ParseQuotedJsonStringArray(*grantees_slice);
  }
  policy.creation_time_ms =
      ParseJsonInt64Field(policy_json, "\"creation_time_ms\":");
  policy.last_modified_time_ms =
      ParseJsonInt64Field(policy_json, "\"last_modified_time_ms\":");
  return policy;
}

void AppendRowAccessPolicies(absl::string_view entry, TableGovernance& gov) {
  auto policies_slice = JsonArraySlice(entry, "\"row_access_policies\"");
  if (!policies_slice.has_value()) {
    return;
  }
  for (absl::string_view policy_json :
       SplitTopLevelJsonObjects(*policies_slice)) {
    RowAccessPolicyRecord policy = ParseRowAccessPolicy(policy_json);
    if (policy.policy_id.empty()) {
      continue;
    }
    gov.row_access_policies.push_back(std::move(policy));
  }
}

std::optional<std::pair<std::string, ColumnGovernanceRecord>> ParseColumnEntry(
    absl::string_view col_json) {
  auto column_or = ParseJsonStringField(col_json, "column");
  if (!column_or.ok()) {
    return std::nullopt;
  }
  ColumnGovernanceRecord col_gov;
  if (auto tags_slice = JsonArraySlice(col_json, "\"policy_tags\"")) {
    col_gov.policy_tags = ParseQuotedJsonStringArray(*tags_slice);
  }
  if (auto mask_or = ParseJsonStringField(col_json, "mask_kind");
      mask_or.ok()) {
    col_gov.mask_kind = MaskKindFromString(*mask_or);
  }
  if (auto grantees_slice = JsonArraySlice(col_json, "\"mask_grantees\"")) {
    col_gov.mask_grantees = ParseQuotedJsonStringArray(*grantees_slice);
  }
  if (auto default_or = ParseJsonStringField(col_json, "default_mask_value");
      default_or.ok()) {
    col_gov.default_mask_value = *default_or;
  }
  return std::pair<std::string, ColumnGovernanceRecord>{*column_or,
                                                        std::move(col_gov)};
}

void AppendColumnGovernance(absl::string_view entry, TableGovernance& gov) {
  auto columns_slice = JsonArraySlice(entry, "\"columns\"");
  if (!columns_slice.has_value()) {
    return;
  }
  for (absl::string_view col_json : SplitTopLevelJsonObjects(*columns_slice)) {
    auto parsed = ParseColumnEntry(col_json);
    if (!parsed.has_value()) {
      continue;
    }
    gov.columns.emplace_back(std::move(parsed->first),
                             std::move(parsed->second));
  }
}

absl::Status PersistGovernance(DuckDBStorage::Impl* impl,
                               const TableId& tid,
                               const TableGovernance& gov) {
  if (!gov.row_access_policies.empty()) {
    absl::Status saved = SaveRowAccessPolicies(impl, tid, gov);
    if (!saved.ok()) {
      return saved;
    }
  }
  for (const auto& [col, col_gov] : gov.columns) {
    absl::Status col_saved = SaveColumnGovernance(impl, tid, col, col_gov);
    if (!col_saved.ok()) {
      return col_saved;
    }
  }
  return absl::OkStatus();
}

}  // namespace

absl::Status RestoreGovernanceFromSnapshotLocked(DuckDBStorage::Impl* impl,
                                                 const DatasetId& ds,
                                                 absl::string_view json) {
  for (absl::string_view entry : SplitTopLevelJsonObjects(json)) {
    auto table_id_or = ParseJsonStringField(entry, "table_id");
    if (!table_id_or.ok()) {
      return table_id_or.status();
    }
    TableId tid{ds.project_id, ds.dataset_id, *table_id_or};
    TableGovernance gov;
    AppendRowAccessPolicies(entry, gov);
    AppendColumnGovernance(entry, gov);
    absl::Status persisted = PersistGovernance(impl, tid, gov);
    if (!persisted.ok()) {
      return persisted;
    }
  }
  return absl::OkStatus();
}

}  // namespace tombstone_registry_internal
}  // namespace duckdb
}  // namespace storage
}  // namespace backend
}  // namespace bigquery_emulator
