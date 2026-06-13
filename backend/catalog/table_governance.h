#ifndef BIGQUERY_EMULATOR_BACKEND_CATALOG_TABLE_GOVERNANCE_H_
#define BIGQUERY_EMULATOR_BACKEND_CATALOG_TABLE_GOVERNANCE_H_

// Row-access policies and column-level security metadata for the
// emulator's single synthetic principal model (see docs/REST_API.md
// and gateway/middleware/auth.go). Policies whose grantee list is
// empty or includes `emulator@bigquery.local` apply to the synthetic
// caller; policies that name only other principals do not.

#include <cstdint>
#include <string>
#include <vector>

#include "absl/status/statusor.h"
#include "absl/strings/string_view.h"
#include "absl/types/span.h"
#include "backend/schema/schema.h"
#include "backend/storage/storage.h"

namespace bigquery_emulator {
namespace backend {
namespace catalog {

// Canonical synthetic principal the gateway attributes to every request.
inline constexpr absl::string_view kEmulatorPrincipalEmail =
    "emulator@bigquery.local";

enum class DataMaskKind {
  kNone = 0,
  kNullify,
  kSha256,
  kDefaultValue,
  kDenied,
};

// One persisted row-access policy on a table.
struct RowAccessPolicyRecord {
  std::string policy_id;
  std::string filter_predicate;
  std::vector<std::string> grantees;
  std::int64_t creation_time_ms = 0;
  std::int64_t last_modified_time_ms = 0;
};

// Column-level policy tag + optional masking rule.
struct ColumnGovernanceRecord {
  std::vector<std::string> policy_tags;
  DataMaskKind mask_kind = DataMaskKind::kNone;
  std::vector<std::string> mask_grantees;
  std::string default_mask_value;
};

struct TableGovernance {
  std::vector<RowAccessPolicyRecord> row_access_policies;
  // Top-level column name -> governance metadata.
  std::vector<std::pair<std::string, ColumnGovernanceRecord>> columns;
};

// Returns true when `grantees` grants the synthetic emulator principal
// access (empty grantee list means all authenticated callers).
bool GranteesIncludePrincipal(absl::Span<const std::string> grantees,
                              absl::string_view principal_email);

// Builds the OR-combined SQL predicate for row-access policies that
// grant `principal_email`. Empty when no policies apply (all rows
// visible). Returns "(FALSE)" when policies exist but none grant the
// principal (no rows visible).
absl::StatusOr<std::string> ComposeRowAccessFilterSql(
    absl::Span<const RowAccessPolicyRecord> policies,
    absl::string_view principal_email);

DataMaskKind ParseDataMaskKind(absl::string_view name);
absl::string_view DataMaskKindName(DataMaskKind kind);

// Effective mask for a column given stored governance and the caller
// principal. Policy tags with no explicit mask default to SHA256 for
// STRING/BYTES and NULLIFY for other scalars.
DataMaskKind EffectiveColumnMask(const ColumnGovernanceRecord& column,
                                 schema::ColumnType column_type,
                                 absl::string_view principal_email);

// Applies `mask` to `value` in-place for query result marshaling.
absl::Status ApplyDataMask(DataMaskKind mask,
                           const ColumnGovernanceRecord& column,
                           schema::ColumnType column_type,
                           storage::Value* value);

const ColumnGovernanceRecord* FindColumnGovernance(
    const TableGovernance& governance, absl::string_view column_name);

}  // namespace catalog
}  // namespace backend
}  // namespace bigquery_emulator

#endif  // BIGQUERY_EMULATOR_BACKEND_CATALOG_TABLE_GOVERNANCE_H_
