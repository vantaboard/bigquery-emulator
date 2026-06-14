#include "backend/storage/duckdb/duckdb_storage_governance.h"

#include <filesystem>
#include <string>
#include <utility>
#include <vector>

#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "absl/strings/ascii.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/str_join.h"
#include "absl/strings/str_replace.h"
#include "absl/strings/str_split.h"
#include "absl/strings/string_view.h"
#include "absl/synchronization/mutex.h"
#include "absl/time/clock.h"
#include "absl/time/time.h"
#include "absl/types/span.h"
#include "backend/storage/duckdb/duckdb_storage.h"
#include "backend/storage/duckdb/duckdb_storage_internal.h"
#include "duckdb.h"

namespace bigquery_emulator {
namespace backend {
namespace storage {
namespace duckdb {

namespace {

constexpr absl::string_view kRowAccessPoliciesTable =
    "main.__bqemu_row_access_policies";
constexpr absl::string_view kColumnGovernanceTable =
    "main.__bqemu_column_governance";

std::string SqlQuote(absl::string_view s) {
  return absl::StrReplaceAll(s, {{"'", "''"}});
}

DataMaskKind MaskKindFromString(absl::string_view s) {
  const std::string upper = absl::AsciiStrToUpper(s);
  if (upper == "NULLIFY") return DataMaskKind::kNullify;
  if (upper == "SHA256") return DataMaskKind::kSha256;
  if (upper == "DEFAULT_VALUE") return DataMaskKind::kDefaultValue;
  if (upper == "DENIED") return DataMaskKind::kDenied;
  return DataMaskKind::kNone;
}

std::string MaskKindToString(DataMaskKind kind) {
  switch (kind) {
    case DataMaskKind::kNullify:
      return "NULLIFY";
    case DataMaskKind::kSha256:
      return "SHA256";
    case DataMaskKind::kDefaultValue:
      return "DEFAULT_VALUE";
    case DataMaskKind::kDenied:
      return "DENIED";
    default:
      return "NONE";
  }
}

std::vector<std::string> SplitCsv(absl::string_view csv) {
  if (csv.empty()) return {};
  return absl::StrSplit(csv, ',');
}

std::string JoinCsv(absl::Span<const std::string> values) {
  return absl::StrJoin(values, ",");
}

absl::Status EnsureTableExistsUnlocked(const DuckDBStorage& storage,
                                       const TableId& id) {
  namespace fs = std::filesystem;
  const fs::path meta_path =
      fs::path(storage.data_dir()) / id.project_id / id.dataset_id /
      absl::StrCat(id.table_id, internal::kTableMetaSuffix);
  std::error_code ec;
  if (!fs::exists(meta_path, ec)) {
    return absl::NotFoundError(absl::StrCat("table not found: ",
                                            id.project_id,
                                            ".",
                                            id.dataset_id,
                                            ".",
                                            id.table_id));
  }
  return absl::OkStatus();
}

}  // namespace

absl::Status EnsureGovernanceTables(DuckDBStorage::Impl* impl) {
  absl::Status rap = internal::RunSql(
      impl,
      absl::StrCat("CREATE TABLE IF NOT EXISTS ",
                   kRowAccessPoliciesTable,
                   " (",
                   "project_id VARCHAR NOT NULL, ",
                   "dataset_id VARCHAR NOT NULL, ",
                   "table_id VARCHAR NOT NULL, ",
                   "policy_id VARCHAR NOT NULL, ",
                   "filter_predicate VARCHAR NOT NULL, ",
                   "grantees_csv VARCHAR, ",
                   "creation_time_ms BIGINT NOT NULL, ",
                   "last_modified_time_ms BIGINT NOT NULL, ",
                   "PRIMARY KEY (project_id, dataset_id, table_id, policy_id)",
                   ")"));
  if (!rap.ok()) return rap;
  return internal::RunSql(
      impl,
      absl::StrCat(
          "CREATE TABLE IF NOT EXISTS ",
          kColumnGovernanceTable,
          " (",
          "project_id VARCHAR NOT NULL, ",
          "dataset_id VARCHAR NOT NULL, ",
          "table_id VARCHAR NOT NULL, ",
          "column_name VARCHAR NOT NULL, ",
          "policy_tags_csv VARCHAR, ",
          "mask_kind VARCHAR NOT NULL, ",
          "mask_grantees_csv VARCHAR, ",
          "default_mask_value VARCHAR, ",
          "PRIMARY KEY (project_id, dataset_id, table_id, column_name)",
          ")"));
}

absl::StatusOr<TableGovernance> LoadTableGovernance(DuckDBStorage::Impl* impl,
                                                    const TableId& id) {
  TableGovernance gov;
  const std::string where = absl::StrCat("project_id='",
                                         SqlQuote(id.project_id),
                                         "' AND dataset_id='",
                                         SqlQuote(id.dataset_id),
                                         "' AND table_id='",
                                         SqlQuote(id.table_id),
                                         "'");

  {
    const std::string sql = absl::StrCat(
        "SELECT policy_id, filter_predicate, grantees_csv, creation_time_ms, "
        "last_modified_time_ms FROM ",
        kRowAccessPoliciesTable,
        " WHERE ",
        where,
        " ORDER BY policy_id");
    ::duckdb_result result;
    if (::duckdb_query(impl->connection, sql.c_str(), &result) !=
        ::DuckDBSuccess) {
      const char* err = ::duckdb_result_error(&result);
      std::string detail = err == nullptr ? std::string("") : std::string(err);
      ::duckdb_destroy_result(&result);
      return internal::DuckDBError(absl::StatusCode::kInternal,
                                   "LoadTableGovernance row policies failed",
                                   detail);
    }
    const idx_t n = ::duckdb_row_count(&result);
    gov.row_access_policies.reserve(static_cast<size_t>(n));
    for (idx_t row = 0; row < n; ++row) {
      RowAccessPolicyRecord rec;
      rec.policy_id = std::string(::duckdb_value_varchar(&result, 0, row));
      rec.filter_predicate =
          std::string(::duckdb_value_varchar(&result, 1, row));
      if (!::duckdb_value_is_null(&result, 2, row)) {
        rec.grantees = SplitCsv(::duckdb_value_varchar(&result, 2, row));
      }
      rec.creation_time_ms = ::duckdb_value_int64(&result, 3, row);
      rec.last_modified_time_ms = ::duckdb_value_int64(&result, 4, row);
      gov.row_access_policies.push_back(std::move(rec));
    }
    ::duckdb_destroy_result(&result);
  }

  {
    const std::string sql = absl::StrCat(
        "SELECT column_name, policy_tags_csv, mask_kind, mask_grantees_csv, "
        "default_mask_value FROM ",
        kColumnGovernanceTable,
        " WHERE ",
        where,
        " ORDER BY column_name");
    ::duckdb_result result;
    if (::duckdb_query(impl->connection, sql.c_str(), &result) !=
        ::DuckDBSuccess) {
      const char* err = ::duckdb_result_error(&result);
      std::string detail = err == nullptr ? std::string("") : std::string(err);
      ::duckdb_destroy_result(&result);
      return internal::DuckDBError(absl::StatusCode::kInternal,
                                   "LoadTableGovernance columns failed",
                                   detail);
    }
    const idx_t n = ::duckdb_row_count(&result);
    gov.columns.reserve(static_cast<size_t>(n));
    for (idx_t row = 0; row < n; ++row) {
      ColumnGovernanceRecord col;
      const std::string column_name =
          std::string(::duckdb_value_varchar(&result, 0, row));
      if (!::duckdb_value_is_null(&result, 1, row)) {
        col.policy_tags = SplitCsv(::duckdb_value_varchar(&result, 1, row));
      }
      col.mask_kind =
          MaskKindFromString(::duckdb_value_varchar(&result, 2, row));
      if (!::duckdb_value_is_null(&result, 3, row)) {
        col.mask_grantees = SplitCsv(::duckdb_value_varchar(&result, 3, row));
      }
      if (!::duckdb_value_is_null(&result, 4, row)) {
        col.default_mask_value =
            std::string(::duckdb_value_varchar(&result, 4, row));
      }
      gov.columns.emplace_back(column_name, std::move(col));
    }
    ::duckdb_destroy_result(&result);
  }

  return gov;
}

absl::Status SaveRowAccessPolicies(DuckDBStorage::Impl* impl,
                                   const TableId& id,
                                   const TableGovernance& gov) {
  const std::string where = absl::StrCat("project_id='",
                                         SqlQuote(id.project_id),
                                         "' AND dataset_id='",
                                         SqlQuote(id.dataset_id),
                                         "' AND table_id='",
                                         SqlQuote(id.table_id),
                                         "'");
  absl::Status del = internal::RunSql(
      impl,
      absl::StrCat("DELETE FROM ", kRowAccessPoliciesTable, " WHERE ", where));
  if (!del.ok()) return del;
  for (const RowAccessPolicyRecord& policy : gov.row_access_policies) {
    const std::string sql = absl::StrCat("INSERT INTO ",
                                         kRowAccessPoliciesTable,
                                         " VALUES ('",
                                         SqlQuote(id.project_id),
                                         "', '",
                                         SqlQuote(id.dataset_id),
                                         "', '",
                                         SqlQuote(id.table_id),
                                         "', '",
                                         SqlQuote(policy.policy_id),
                                         "', '",
                                         SqlQuote(policy.filter_predicate),
                                         "', '",
                                         SqlQuote(JoinCsv(policy.grantees)),
                                         "', ",
                                         policy.creation_time_ms,
                                         ", ",
                                         policy.last_modified_time_ms,
                                         ")");
    absl::Status ins = internal::RunSql(impl, sql);
    if (!ins.ok()) return ins;
  }
  return absl::OkStatus();
}

absl::Status SaveColumnGovernance(DuckDBStorage::Impl* impl,
                                  const TableId& id,
                                  absl::string_view column_name,
                                  const ColumnGovernanceRecord& column) {
  const std::string sql = absl::StrCat("INSERT OR REPLACE INTO ",
                                       kColumnGovernanceTable,
                                       " VALUES ('",
                                       SqlQuote(id.project_id),
                                       "', '",
                                       SqlQuote(id.dataset_id),
                                       "', '",
                                       SqlQuote(id.table_id),
                                       "', '",
                                       SqlQuote(column_name),
                                       "', '",
                                       SqlQuote(JoinCsv(column.policy_tags)),
                                       "', '",
                                       MaskKindToString(column.mask_kind),
                                       "', '",
                                       SqlQuote(JoinCsv(column.mask_grantees)),
                                       "', '",
                                       SqlQuote(column.default_mask_value),
                                       "')");
  return internal::RunSql(impl, sql);
}

std::string DuckDBStorage::TableGovernancePath(const TableId& id) const {
  (void)id;
  return std::string();
}

absl::StatusOr<TableGovernance> DuckDBStorage::GetTableGovernance(
    const TableId& id) const {
  absl::MutexLock lock(&mu_);
  absl::Status exists = EnsureTableExistsUnlocked(*this, id);
  if (!exists.ok()) return exists;
  absl::Status init = EnsureGovernanceTables(impl_.get());
  if (!init.ok()) return init;
  return LoadTableGovernance(impl_.get(), id);
}

absl::Status DuckDBStorage::PutTableGovernance(const TableId& id,
                                               const TableGovernance& gov) {
  absl::MutexLock lock(&mu_);
  absl::Status exists = EnsureTableExistsUnlocked(*this, id);
  if (!exists.ok()) return exists;
  absl::Status init = EnsureGovernanceTables(impl_.get());
  if (!init.ok()) return init;
  return SaveRowAccessPolicies(impl_.get(), id, gov);
}

absl::Status DuckDBStorage::UpsertRowAccessPolicy(
    const TableId& id, const RowAccessPolicyRecord& policy) {
  absl::MutexLock lock(&mu_);
  absl::Status exists = EnsureTableExistsUnlocked(*this, id);
  if (!exists.ok()) return exists;
  absl::Status init = EnsureGovernanceTables(impl_.get());
  if (!init.ok()) return init;
  absl::StatusOr<TableGovernance> gov_or = LoadTableGovernance(impl_.get(), id);
  if (!gov_or.ok()) return gov_or.status();
  TableGovernance gov = std::move(*gov_or);
  const std::int64_t now_ms = absl::ToUnixMillis(absl::Now());
  bool replaced = false;
  for (RowAccessPolicyRecord& existing : gov.row_access_policies) {
    if (existing.policy_id == policy.policy_id) {
      existing = policy;
      if (existing.creation_time_ms == 0) existing.creation_time_ms = now_ms;
      existing.last_modified_time_ms = now_ms;
      replaced = true;
      break;
    }
  }
  if (!replaced) {
    RowAccessPolicyRecord inserted = policy;
    if (inserted.creation_time_ms == 0) inserted.creation_time_ms = now_ms;
    inserted.last_modified_time_ms = now_ms;
    gov.row_access_policies.push_back(std::move(inserted));
  }
  return SaveRowAccessPolicies(impl_.get(), id, gov);
}

absl::Status DuckDBStorage::DeleteRowAccessPolicy(const TableId& id,
                                                  absl::string_view policy_id) {
  absl::MutexLock lock(&mu_);
  absl::Status exists = EnsureTableExistsUnlocked(*this, id);
  if (!exists.ok()) return exists;
  absl::Status init = EnsureGovernanceTables(impl_.get());
  if (!init.ok()) return init;
  absl::StatusOr<TableGovernance> gov_or = LoadTableGovernance(impl_.get(), id);
  if (!gov_or.ok()) return gov_or.status();
  TableGovernance gov = std::move(*gov_or);
  std::vector<RowAccessPolicyRecord> kept;
  kept.reserve(gov.row_access_policies.size());
  bool found = false;
  for (RowAccessPolicyRecord& p : gov.row_access_policies) {
    if (p.policy_id == policy_id) {
      found = true;
      continue;
    }
    kept.push_back(std::move(p));
  }
  if (!found) {
    return absl::NotFoundError(
        absl::StrCat("row access policy not found: ", policy_id));
  }
  gov.row_access_policies = std::move(kept);
  return SaveRowAccessPolicies(impl_.get(), id, gov);
}

absl::Status DuckDBStorage::SetColumnGovernance(
    const TableId& id,
    absl::string_view column_name,
    const ColumnGovernanceRecord& column) {
  absl::MutexLock lock(&mu_);
  absl::Status exists = EnsureTableExistsUnlocked(*this, id);
  if (!exists.ok()) return exists;
  absl::Status init = EnsureGovernanceTables(impl_.get());
  if (!init.ok()) return init;
  return SaveColumnGovernance(impl_.get(), id, column_name, column);
}

}  // namespace duckdb
}  // namespace storage
}  // namespace backend
}  // namespace bigquery_emulator
