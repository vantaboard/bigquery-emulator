#include "backend/engine/duckdb/duckdb_executor_security.h"

#include "absl/types/span.h"
#include "backend/catalog/table_governance.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace duckdb {
namespace internal {

namespace {

catalog::TableGovernance ToCatalogGovernance(
    const storage::TableGovernance& storage_gov) {
  catalog::TableGovernance out;
  out.row_access_policies.reserve(storage_gov.row_access_policies.size());
  for (const storage::RowAccessPolicyRecord& p :
       storage_gov.row_access_policies) {
    catalog::RowAccessPolicyRecord rec;
    rec.policy_id = p.policy_id;
    rec.filter_predicate = p.filter_predicate;
    rec.grantees = p.grantees;
    rec.creation_time_ms = p.creation_time_ms;
    rec.last_modified_time_ms = p.last_modified_time_ms;
    out.row_access_policies.push_back(std::move(rec));
  }
  out.columns.reserve(storage_gov.columns.size());
  for (const auto& entry : storage_gov.columns) {
    catalog::ColumnGovernanceRecord col;
    col.policy_tags = entry.second.policy_tags;
    col.mask_grantees = entry.second.mask_grantees;
    col.default_mask_value = entry.second.default_mask_value;
    switch (entry.second.mask_kind) {
      case storage::DataMaskKind::kNullify:
        col.mask_kind = catalog::DataMaskKind::kNullify;
        break;
      case storage::DataMaskKind::kSha256:
        col.mask_kind = catalog::DataMaskKind::kSha256;
        break;
      case storage::DataMaskKind::kDefaultValue:
        col.mask_kind = catalog::DataMaskKind::kDefaultValue;
        break;
      case storage::DataMaskKind::kDenied:
        col.mask_kind = catalog::DataMaskKind::kDenied;
        break;
      default:
        col.mask_kind = catalog::DataMaskKind::kNone;
        break;
    }
    out.columns.emplace_back(entry.first, std::move(col));
  }
  return out;
}

}  // namespace

std::vector<OutputColumnMask> BuildOutputColumnMasks(
    const schema::TableSchema& output_schema,
    const catalog::TableGovernance& table_governance,
    absl::string_view principal_email) {
  std::vector<OutputColumnMask> masks;
  masks.reserve(output_schema.columns.size());
  for (const schema::ColumnSchema& column : output_schema.columns) {
    OutputColumnMask mask;
    mask.column_type = column.type;
    if (const catalog::ColumnGovernanceRecord* gov =
            catalog::FindColumnGovernance(table_governance, column.name)) {
      mask.governance = *gov;
      mask.mask =
          catalog::EffectiveColumnMask(*gov, column.type, principal_email);
    }
    masks.push_back(std::move(mask));
  }
  return masks;
}

absl::Status ApplyOutputColumnMasks(absl::Span<const OutputColumnMask> masks,
                                    storage::Row* row) {
  if (row == nullptr) {
    return absl::InvalidArgumentError("ApplyOutputColumnMasks: row is null");
  }
  if (masks.size() != row->cells.size()) {
    return absl::InvalidArgumentError(
        "ApplyOutputColumnMasks: mask/cell count mismatch");
  }
  for (size_t i = 0; i < masks.size(); ++i) {
    if (masks[i].mask == catalog::DataMaskKind::kNone) continue;
    absl::Status masked = catalog::ApplyDataMask(masks[i].mask,
                                                 masks[i].governance,
                                                 masks[i].column_type,
                                                 &row->cells[i]);
    if (!masked.ok()) return masked;
  }
  return absl::OkStatus();
}

catalog::TableGovernance StorageGovernanceToCatalog(
    const storage::TableGovernance& storage_gov) {
  return ToCatalogGovernance(storage_gov);
}

}  // namespace internal
}  // namespace duckdb
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
