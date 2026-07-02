#include <algorithm>
#include <filesystem>
#include <string>
#include <utility>
#include <vector>

#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "absl/strings/match.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/str_join.h"
#include "absl/strings/string_view.h"
#include "absl/synchronization/mutex.h"
#include "backend/storage/duckdb/duckdb_storage.h"
#include "backend/storage/duckdb/duckdb_storage_governance.h"
#include "backend/storage/duckdb/duckdb_storage_internal.h"
#include "backend/storage/duckdb/duckdb_storage_tombstone_registry_internal.h"
#include "backend/storage/storage.h"
#include "duckdb.h"

namespace bigquery_emulator {
namespace backend {
namespace storage {
namespace duckdb {

namespace fs = std::filesystem;
namespace internal_ns = tombstone_registry_internal;

absl::Status internal_ns::RunDatasetScopedDelete(DuckDBStorage::Impl* impl,
                                                 absl::string_view table,
                                                 const DatasetId& id) {
  const std::string sql =
      absl::StrCat("DELETE FROM ",
                   table,
                   " WHERE project_id = '",
                   internal::EscapeStringLiteralInner(id.project_id),
                   "' AND dataset_id = '",
                   internal::EscapeStringLiteralInner(id.dataset_id),
                   "'");
  return internal::RunSql(impl, sql);
}

absl::StatusOr<std::vector<ViewRecord>> internal_ns::QueryViewsForDataset(
    DuckDBStorage::Impl* impl, const DatasetId& id) {
  const std::string where =
      absl::StrCat("project_id = '",
                   internal::EscapeStringLiteralInner(id.project_id),
                   "' AND dataset_id = '",
                   internal::EscapeStringLiteralInner(id.dataset_id),
                   "'");
  const std::string sql =
      absl::StrCat("SELECT project_id, dataset_id, view_id, ddl_sql FROM ",
                   internal_ns::kViewsTable,
                   " WHERE ",
                   where,
                   " ORDER BY view_id");
  ::duckdb_result result;
  if (::duckdb_query(impl->connection, sql.c_str(), &result) !=
      ::DuckDBSuccess) {
    const char* err = ::duckdb_result_error(&result);
    std::string detail = err == nullptr ? std::string("") : std::string(err);
    ::duckdb_destroy_result(&result);
    return internal::DuckDBError(
        absl::StatusCode::kInternal, "QueryViewsForDataset failed", detail);
  }
  std::vector<ViewRecord> out;
  const idx_t n = ::duckdb_row_count(&result);
  out.reserve(static_cast<size_t>(n));
  for (idx_t row = 0; row < n; ++row) {
    ViewRecord rec;
    rec.id.project_id = std::string(::duckdb_value_varchar(&result, 0, row));
    rec.id.dataset_id = std::string(::duckdb_value_varchar(&result, 1, row));
    rec.id.view_id = std::string(::duckdb_value_varchar(&result, 2, row));
    rec.ddl_sql = std::string(::duckdb_value_varchar(&result, 3, row));
    out.push_back(std::move(rec));
  }
  ::duckdb_destroy_result(&result);
  return out;
}

absl::StatusOr<std::vector<RoutineRecord>> internal_ns::QueryRoutinesForDataset(
    DuckDBStorage::Impl* impl, const DatasetId& id) {
  const std::string where =
      absl::StrCat("project_id = '",
                   internal::EscapeStringLiteralInner(id.project_id),
                   "' AND dataset_id = '",
                   internal::EscapeStringLiteralInner(id.dataset_id),
                   "'");
  const std::string sql = absl::StrCat(
      "SELECT project_id, dataset_id, routine_id, kind, language, ddl_sql, ",
      "is_temp, signature_json FROM ",
      internal_ns::kRoutinesTable,
      " WHERE ",
      where,
      " ORDER BY routine_id");
  ::duckdb_result result;
  if (::duckdb_query(impl->connection, sql.c_str(), &result) !=
      ::DuckDBSuccess) {
    const char* err = ::duckdb_result_error(&result);
    std::string detail = err == nullptr ? std::string("") : std::string(err);
    ::duckdb_destroy_result(&result);
    return internal::DuckDBError(
        absl::StatusCode::kInternal, "QueryRoutinesForDataset failed", detail);
  }
  std::vector<RoutineRecord> out;
  const idx_t n = ::duckdb_row_count(&result);
  out.reserve(static_cast<size_t>(n));
  for (idx_t row = 0; row < n; ++row) {
    RoutineRecord rec;
    rec.id.project_id = std::string(::duckdb_value_varchar(&result, 0, row));
    rec.id.dataset_id = std::string(::duckdb_value_varchar(&result, 1, row));
    rec.id.routine_id = std::string(::duckdb_value_varchar(&result, 2, row));
    auto kind_or = internal_ns::RoutineKindFromString(
        ::duckdb_value_varchar(&result, 3, row));
    if (!kind_or.ok()) {
      ::duckdb_destroy_result(&result);
      return kind_or.status();
    }
    rec.kind = *kind_or;
    rec.language = std::string(::duckdb_value_varchar(&result, 4, row));
    rec.ddl_sql = std::string(::duckdb_value_varchar(&result, 5, row));
    rec.is_temp = ::duckdb_value_boolean(&result, 6, row);
    if (!::duckdb_value_is_null(&result, 7, row)) {
      rec.signature_json = std::string(::duckdb_value_varchar(&result, 7, row));
    }
    out.push_back(std::move(rec));
  }
  ::duckdb_destroy_result(&result);
  return out;
}

std::vector<std::string> internal_ns::ListTableIdsInDatasetDir(
    const fs::path& ds_dir) {
  std::vector<std::string> out;
  std::error_code ec;
  for (const auto& entry : fs::directory_iterator(ds_dir, ec)) {
    if (ec) break;
    const std::string name = entry.path().filename().string();
    if (name == internal::kDatasetMetaFile) continue;
    if (name.size() <= internal::kTableMetaSuffix.size()) continue;
    if (!absl::EndsWith(name, internal::kTableMetaSuffix)) continue;
    out.push_back(
        name.substr(0, name.size() - internal::kTableMetaSuffix.size()));
  }
  std::sort(out.begin(), out.end());
  return out;
}

absl::Status internal_ns::UpsertViewLocked(DuckDBStorage::Impl* impl,
                                           absl::string_view data_dir,
                                           const ViewRecord& record) {
  const TableId table_id{
      record.id.project_id, record.id.dataset_id, record.id.view_id};
  const fs::path ds_dir = fs::path(std::string(data_dir)) /
                          table_id.project_id / table_id.dataset_id;
  absl::string_view view_query =
      record.view_query.empty() ? record.ddl_sql : record.view_query;
  auto meta_json_or = internal::RenderViewTableMetaJson(
      record.schema, view_query, record.ddl_sql);
  if (!meta_json_or.ok()) return meta_json_or.status();
  const fs::path meta_path =
      ds_dir / absl::StrCat(table_id.table_id, internal::kTableMetaSuffix);
  absl::Status written = internal::WriteFileAtomic(meta_path, *meta_json_or);
  if (!written.ok()) return written;
  const std::string sql = absl::StrCat(
      "INSERT INTO ",
      internal_ns::kViewsTable,
      " (project_id, dataset_id, view_id, ddl_sql) VALUES ('",
      internal::EscapeStringLiteralInner(record.id.project_id),
      "', '",
      internal::EscapeStringLiteralInner(record.id.dataset_id),
      "', '",
      internal::EscapeStringLiteralInner(record.id.view_id),
      "', '",
      internal::EscapeStringLiteralInner(record.ddl_sql),
      "') ON CONFLICT (project_id, dataset_id, view_id) DO UPDATE SET ",
      "ddl_sql = excluded.ddl_sql");
  return internal::RunSql(impl, sql);
}

absl::Status internal_ns::UpsertRoutineLocked(DuckDBStorage::Impl* impl,
                                              const RoutineRecord& record) {
  const std::string sql = absl::StrCat(
      "INSERT INTO ",
      internal_ns::kRoutinesTable,
      " (project_id, dataset_id, routine_id, kind, language, ddl_sql, ",
      "is_temp, signature_json) VALUES (",
      "'",
      internal::EscapeStringLiteralInner(record.id.project_id),
      "', '",
      internal::EscapeStringLiteralInner(record.id.dataset_id),
      "', '",
      internal::EscapeStringLiteralInner(record.id.routine_id),
      "', '",
      internal_ns::RoutineKindToString(record.kind),
      "', '",
      internal::EscapeStringLiteralInner(record.language),
      "', '",
      internal::EscapeStringLiteralInner(record.ddl_sql),
      "', ",
      record.is_temp ? "TRUE" : "FALSE",
      ", ",
      record.signature_json.empty()
          ? "NULL"
          : absl::StrCat(
                "'",
                internal::EscapeStringLiteralInner(record.signature_json),
                "'"),
      ") ON CONFLICT (project_id, dataset_id, routine_id) DO UPDATE SET ",
      "kind = excluded.kind, ",
      "language = excluded.language, ",
      "ddl_sql = excluded.ddl_sql, ",
      "is_temp = excluded.is_temp, ",
      "signature_json = excluded.signature_json");
  return internal::RunSql(impl, sql);
}

absl::Status DuckDBStorage::SnapshotDatasetRegistryForTombstoneLocked(
    const DatasetId& id, const fs::path& tombstone_dir) {
  absl::Status init = EnsureGovernanceTables(impl_.get());
  if (!init.ok()) return init;

  auto views_or = internal_ns::QueryViewsForDataset(impl_.get(), id);
  if (!views_or.ok()) return views_or.status();
  std::vector<std::string> view_json;
  view_json.reserve(views_or->size());
  for (const ViewRecord& rec : *views_or) {
    view_json.push_back(internal_ns::RenderViewRecordJson(rec));
  }
  absl::Status write_views = internal::WriteFileAtomic(
      tombstone_dir / "views.json",
      absl::StrCat("[", absl::StrJoin(view_json, ","), "]"));
  if (!write_views.ok()) return write_views;

  auto routines_or = internal_ns::QueryRoutinesForDataset(impl_.get(), id);
  if (!routines_or.ok()) return routines_or.status();
  std::vector<std::string> routine_json;
  routine_json.reserve(routines_or->size());
  for (const RoutineRecord& rec : *routines_or) {
    routine_json.push_back(internal_ns::RenderRoutineRecordJson(rec));
  }
  absl::Status write_routines = internal::WriteFileAtomic(
      tombstone_dir / "routines.json",
      absl::StrCat("[", absl::StrJoin(routine_json, ","), "]"));
  if (!write_routines.ok()) return write_routines;

  const fs::path ds_dir = fs::path(data_dir_) / id.project_id / id.dataset_id;
  std::vector<std::pair<TableId, TableGovernance>> gov_tables;
  for (const std::string& table_id :
       internal_ns::ListTableIdsInDatasetDir(ds_dir)) {
    TableId tid{id.project_id, id.dataset_id, table_id};
    auto gov_or = LoadTableGovernance(impl_.get(), tid);
    if (!gov_or.ok()) return gov_or.status();
    if (gov_or->row_access_policies.empty() && gov_or->columns.empty()) {
      continue;
    }
    gov_tables.emplace_back(tid, *gov_or);
  }
  return internal::WriteFileAtomic(
      tombstone_dir / "governance.json",
      internal_ns::RenderGovernanceSnapshotJson(gov_tables));
}

absl::Status DuckDBStorage::RestoreDatasetRegistryFromTombstoneLocked(
    const DatasetId& id, const fs::path& tombstone_dir) {
  absl::Status init = EnsureGovernanceTables(impl_.get());
  if (!init.ok()) return init;

  const fs::path views_path = tombstone_dir / "views.json";
  std::error_code ec;
  if (fs::exists(views_path, ec)) {
    auto contents_or = internal::ReadFile(views_path);
    if (!contents_or.ok()) return contents_or.status();
    for (absl::string_view entry :
         internal_ns::SplitTopLevelJsonObjects(*contents_or)) {
      auto rec_or = internal_ns::ParseViewRecordJson(entry, id);
      if (!rec_or.ok()) return rec_or.status();
      absl::Status upserted =
          internal_ns::UpsertViewLocked(impl_.get(), data_dir_, *rec_or);
      if (!upserted.ok()) return upserted;
    }
  }

  const fs::path routines_path = tombstone_dir / "routines.json";
  if (fs::exists(routines_path, ec)) {
    auto contents_or = internal::ReadFile(routines_path);
    if (!contents_or.ok()) return contents_or.status();
    for (absl::string_view entry :
         internal_ns::SplitTopLevelJsonObjects(*contents_or)) {
      auto rec_or = internal_ns::ParseRoutineRecordJson(entry, id);
      if (!rec_or.ok()) return rec_or.status();
      absl::Status upserted =
          internal_ns::UpsertRoutineLocked(impl_.get(), *rec_or);
      if (!upserted.ok()) return upserted;
    }
  }

  const fs::path gov_path = tombstone_dir / "governance.json";
  if (fs::exists(gov_path, ec)) {
    auto contents_or = internal::ReadFile(gov_path);
    if (!contents_or.ok()) return contents_or.status();
    return internal_ns::RestoreGovernanceFromSnapshotLocked(
        impl_.get(), id, *contents_or);
  }
  return absl::OkStatus();
}

absl::Status DuckDBStorage::PurgeDatasetRegistryRowsLocked(
    const DatasetId& id) {
  absl::Status views = internal_ns::RunDatasetScopedDelete(
      impl_.get(), internal_ns::kViewsTable, id);
  if (!views.ok()) return views;
  absl::Status routines = internal_ns::RunDatasetScopedDelete(
      impl_.get(), internal_ns::kRoutinesTable, id);
  if (!routines.ok()) return routines;
  absl::Status rap = internal_ns::RunDatasetScopedDelete(
      impl_.get(), internal_ns::kRowAccessPoliciesTable, id);
  if (!rap.ok()) return rap;
  return internal_ns::RunDatasetScopedDelete(
      impl_.get(), internal_ns::kColumnGovernanceTable, id);
}

absl::StatusOr<std::string> DuckDBStorage::GetDatasetRestMetadataJsonLocked(
    const DatasetId& id) const {
  const fs::path meta_path = DatasetMetaPath(id);
  auto contents_or = internal::ReadFile(meta_path);
  if (!contents_or.ok()) return contents_or.status();
  return internal::ExtractRestMetadataFromDatasetMetaJson(*contents_or);
}

absl::StatusOr<std::string> DuckDBStorage::GetDatasetRestMetadataJson(
    const DatasetId& id) const {
  absl::MutexLock lock(&mu_);
  return GetDatasetRestMetadataJsonLocked(id);
}

}  // namespace duckdb
}  // namespace storage
}  // namespace backend
}  // namespace bigquery_emulator
