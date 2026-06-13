#include "backend/catalog/table_governance.h"

#include <cstdint>
#include <string>

#include "absl/status/status.h"
#include "absl/strings/ascii.h"
#include "absl/strings/match.h"
#include "absl/strings/numbers.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/str_join.h"
#include "absl/strings/string_view.h"
#include <openssl/sha.h>

namespace bigquery_emulator {
namespace backend {
namespace catalog {

namespace {

bool GranteeMatchesPrincipal(absl::string_view grantee,
                             absl::string_view principal_email) {
  if (grantee.empty()) return false;
  if (absl::StartsWith(grantee, "user:")) {
    return grantee.substr(5) == principal_email;
  }
  if (grantee == "allAuthenticatedUsers" || grantee == "allUsers") {
    return true;
  }
  return false;
}

std::string Sha256Hex(absl::string_view input) {
  unsigned char digest[SHA256_DIGEST_LENGTH];
  SHA256(reinterpret_cast<const unsigned char*>(input.data()), input.size(),
         digest);
  static const char kHex[] = "0123456789abcdef";
  std::string out;
  out.reserve(SHA256_DIGEST_LENGTH * 2);
  for (unsigned char b : digest) {
    out.push_back(kHex[b >> 4]);
    out.push_back(kHex[b & 0x0f]);
  }
  return out;
}

}  // namespace

bool GranteesIncludePrincipal(absl::Span<const std::string> grantees,
                              absl::string_view principal_email) {
  if (grantees.empty()) return true;
  for (const std::string& g : grantees) {
    if (GranteeMatchesPrincipal(g, principal_email)) return true;
  }
  return false;
}

absl::StatusOr<std::string> ComposeRowAccessFilterSql(
    absl::Span<const RowAccessPolicyRecord> policies,
    absl::string_view principal_email) {
  if (policies.empty()) return std::string();
  std::vector<std::string> parts;
  parts.reserve(policies.size());
  for (const RowAccessPolicyRecord& policy : policies) {
    if (!GranteesIncludePrincipal(policy.grantees, principal_email)) {
      continue;
    }
    if (policy.filter_predicate.empty()) {
      return absl::InvalidArgumentError(
          "row access policy filter predicate must be non-empty");
    }
    parts.push_back(absl::StrCat("(", policy.filter_predicate, ")"));
  }
  if (parts.empty()) return std::string("(FALSE)");
  if (parts.size() == 1) return parts[0];
  return absl::StrCat("(", absl::StrJoin(parts, " OR "), ")");
}

DataMaskKind ParseDataMaskKind(absl::string_view name) {
  const std::string upper = absl::AsciiStrToUpper(name);
  if (upper == "NULLIFY" || upper == "NULL") return DataMaskKind::kNullify;
  if (upper == "SHA256" || upper == "HASH") return DataMaskKind::kSha256;
  if (upper == "DEFAULT" || upper == "DEFAULT_VALUE") {
    return DataMaskKind::kDefaultValue;
  }
  if (upper == "DENIED" || upper == "DENY") return DataMaskKind::kDenied;
  return DataMaskKind::kNone;
}

absl::string_view DataMaskKindName(DataMaskKind kind) {
  switch (kind) {
    case DataMaskKind::kNullify:
      return "NULLIFY";
    case DataMaskKind::kSha256:
      return "SHA256";
    case DataMaskKind::kDefaultValue:
      return "DEFAULT_VALUE";
    case DataMaskKind::kDenied:
      return "DENIED";
    case DataMaskKind::kNone:
      return "NONE";
  }
  return "NONE";
}

DataMaskKind EffectiveColumnMask(const ColumnGovernanceRecord& column,
                                 schema::ColumnType column_type,
                                 absl::string_view principal_email) {
  if (column.policy_tags.empty() && column.mask_kind == DataMaskKind::kNone) {
    return DataMaskKind::kNone;
  }
  if (GranteesIncludePrincipal(column.mask_grantees, principal_email)) {
    return DataMaskKind::kNone;
  }
  if (column.mask_kind != DataMaskKind::kNone) {
    return column.mask_kind;
  }
  if (!column.policy_tags.empty()) {
    switch (column_type) {
      case schema::ColumnType::kString:
      case schema::ColumnType::kBytes:
        return DataMaskKind::kSha256;
      default:
        return DataMaskKind::kNullify;
    }
  }
  return DataMaskKind::kNone;
}

absl::Status ApplyDataMask(DataMaskKind mask,
                           const ColumnGovernanceRecord& column,
                           schema::ColumnType column_type,
                           storage::Value* value) {
  if (value == nullptr) {
    return absl::InvalidArgumentError("ApplyDataMask: value is null");
  }
  switch (mask) {
    case DataMaskKind::kNone:
      return absl::OkStatus();
    case DataMaskKind::kDenied:
      return absl::PermissionDeniedError(
          "Access denied: caller lacks Fine-Grained Reader access to column");
    case DataMaskKind::kNullify:
      *value = storage::Value::Null();
      return absl::OkStatus();
    case DataMaskKind::kDefaultValue:
      if (column_type == schema::ColumnType::kString ||
          column_type == schema::ColumnType::kBytes) {
        *value = storage::Value::String(column.default_mask_value);
      } else if (column_type == schema::ColumnType::kInt64) {
        int64_t v = 0;
        if (!column.default_mask_value.empty()) {
          if (!absl::SimpleAtoi(column.default_mask_value, &v)) {
            return absl::InvalidArgumentError(
                "default mask value is not a valid INT64");
          }
        }
        *value = storage::Value::Int64(v);
      } else {
        *value = storage::Value::Null();
      }
      return absl::OkStatus();
    case DataMaskKind::kSha256:
      if (value->is_null()) return absl::OkStatus();
      if (column_type == schema::ColumnType::kString ||
          column_type == schema::ColumnType::kBytes) {
        *value = storage::Value::String(Sha256Hex(value->string_value()));
        return absl::OkStatus();
      }
      *value = storage::Value::Null();
      return absl::OkStatus();
  }
  return absl::OkStatus();
}

const ColumnGovernanceRecord* FindColumnGovernance(
    const TableGovernance& governance, absl::string_view column_name) {
  for (const auto& entry : governance.columns) {
    if (entry.first == column_name) return &entry.second;
  }
  return nullptr;
}

}  // namespace catalog
}  // namespace backend
}  // namespace bigquery_emulator
