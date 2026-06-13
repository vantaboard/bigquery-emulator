#ifndef BIGQUERY_EMULATOR_BACKEND_STORAGE_TABLE_GOVERNANCE_H_
#define BIGQUERY_EMULATOR_BACKEND_STORAGE_TABLE_GOVERNANCE_H_

// Engine-agnostic row-access / column-governance records persisted
// alongside table data. See backend/catalog/table_governance.h for
// the catalog-facing helpers that interpret these records at query
// time.

#include <cstdint>
#include <string>
#include <utility>
#include <vector>

#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "absl/strings/string_view.h"

namespace bigquery_emulator {
namespace backend {
namespace storage {

struct RowAccessPolicyRecord {
  std::string policy_id;
  std::string filter_predicate;
  std::vector<std::string> grantees;
  std::int64_t creation_time_ms = 0;
  std::int64_t last_modified_time_ms = 0;
};

enum class DataMaskKind {
  kNone = 0,
  kNullify,
  kSha256,
  kDefaultValue,
  kDenied,
};

struct ColumnGovernanceRecord {
  std::vector<std::string> policy_tags;
  DataMaskKind mask_kind = DataMaskKind::kNone;
  std::vector<std::string> mask_grantees;
  std::string default_mask_value;
};

struct TableGovernance {
  std::vector<RowAccessPolicyRecord> row_access_policies;
  std::vector<std::pair<std::string, ColumnGovernanceRecord>> columns;
};

}  // namespace storage
}  // namespace backend
}  // namespace bigquery_emulator

#endif  // BIGQUERY_EMULATOR_BACKEND_STORAGE_TABLE_GOVERNANCE_H_
