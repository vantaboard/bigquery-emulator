#include <cstdint>
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

absl::Status RestoreGovernanceFromSnapshotLocked(DuckDBStorage::Impl* impl,
                                                 const DatasetId& ds,
                                                 absl::string_view json) {
  for (absl::string_view entry : SplitTopLevelJsonObjects(json)) {
    auto table_id_or = ParseJsonStringField(entry, "table_id");
    if (!table_id_or.ok()) return table_id_or.status();
    TableId tid{ds.project_id, ds.dataset_id, *table_id_or};
    TableGovernance gov;
    const size_t rap_pos = entry.find("\"row_access_policies\"");
    if (rap_pos != absl::string_view::npos) {
      const size_t open = entry.find('[', rap_pos);
      const size_t close = entry.find(']', open);
      if (open != absl::string_view::npos && close != absl::string_view::npos) {
        for (absl::string_view policy_json :
             SplitTopLevelJsonObjects(entry.substr(open, close - open + 1))) {
          auto policy_id_or = ParseJsonStringField(policy_json, "policy_id");
          auto filter_or = ParseJsonStringField(policy_json, "filter_predicate");
          if (!policy_id_or.ok() || !filter_or.ok()) continue;
          RowAccessPolicyRecord policy;
          policy.policy_id = *policy_id_or;
          policy.filter_predicate = *filter_or;
          const size_t grantees_pos = policy_json.find("\"grantees\"");
          if (grantees_pos != absl::string_view::npos) {
            const size_t g_open = policy_json.find('[', grantees_pos);
            const size_t g_close = policy_json.find(']', g_open);
            if (g_open != absl::string_view::npos &&
                g_close != absl::string_view::npos) {
              policy.grantees = ParseQuotedJsonStringArray(
                  policy_json.substr(g_open, g_close - g_open + 1));
            }
          }
          std::int64_t created = 0;
          std::int64_t modified = 0;
          const std::string created_needle = "\"creation_time_ms\":";
          const size_t cpos = policy_json.find(created_needle);
          if (cpos != absl::string_view::npos) {
            (void)absl::SimpleAtoi(
                policy_json.substr(cpos + created_needle.size()), &created);
          }
          const std::string modified_needle = "\"last_modified_time_ms\":";
          const size_t mpos = policy_json.find(modified_needle);
          if (mpos != absl::string_view::npos) {
            (void)absl::SimpleAtoi(
                policy_json.substr(mpos + modified_needle.size()), &modified);
          }
          policy.creation_time_ms = created;
          policy.last_modified_time_ms = modified;
          gov.row_access_policies.push_back(std::move(policy));
        }
      }
    }
    const size_t col_pos = entry.find("\"columns\"");
    if (col_pos != absl::string_view::npos) {
      const size_t open = entry.find('[', col_pos);
      const size_t close = entry.find(']', open);
      if (open != absl::string_view::npos && close != absl::string_view::npos) {
        for (absl::string_view col_json :
             SplitTopLevelJsonObjects(entry.substr(open, close - open + 1))) {
          auto column_or = ParseJsonStringField(col_json, "column");
          if (!column_or.ok()) continue;
          ColumnGovernanceRecord col_gov;
          const size_t tags_pos = col_json.find("\"policy_tags\"");
          if (tags_pos != absl::string_view::npos) {
            const size_t t_open = col_json.find('[', tags_pos);
            const size_t t_close = col_json.find(']', t_open);
            if (t_open != absl::string_view::npos &&
                t_close != absl::string_view::npos) {
              col_gov.policy_tags = ParseQuotedJsonStringArray(
                  col_json.substr(t_open, t_close - t_open + 1));
            }
          }
          auto mask_or = ParseJsonStringField(col_json, "mask_kind");
          if (mask_or.ok()) {
            col_gov.mask_kind = MaskKindFromString(*mask_or);
          }
          const size_t mg_pos = col_json.find("\"mask_grantees\"");
          if (mg_pos != absl::string_view::npos) {
            const size_t m_open = col_json.find('[', mg_pos);
            const size_t m_close = col_json.find(']', m_open);
            if (m_open != absl::string_view::npos &&
                m_close != absl::string_view::npos) {
              col_gov.mask_grantees = ParseQuotedJsonStringArray(
                  col_json.substr(m_open, m_close - m_open + 1));
            }
          }
          auto default_or = ParseJsonStringField(col_json, "default_mask_value");
          if (default_or.ok()) {
            col_gov.default_mask_value = *default_or;
          }
          gov.columns.emplace_back(*column_or, std::move(col_gov));
        }
      }
    }
    if (!gov.row_access_policies.empty()) {
      absl::Status saved = SaveRowAccessPolicies(impl, tid, gov);
      if (!saved.ok()) return saved;
    }
    for (const auto& [col, col_gov] : gov.columns) {
      absl::Status col_saved = SaveColumnGovernance(impl, tid, col, col_gov);
      if (!col_saved.ok()) return col_saved;
    }
  }
  return absl::OkStatus();
}

}  // namespace tombstone_registry_internal
}  // namespace duckdb
}  // namespace storage
}  // namespace backend
}  // namespace bigquery_emulator
