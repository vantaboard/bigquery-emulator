#include <filesystem>
#include <string>
#include <utility>
#include <vector>

#include "absl/container/flat_hash_set.h"
#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "absl/strings/match.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/string_view.h"
#include "absl/synchronization/mutex.h"
#include "backend/storage/duckdb/duckdb_storage.h"
#include "backend/storage/duckdb/duckdb_storage_internal.h"
#include "duckdb.h"

namespace bigquery_emulator {
namespace backend {
namespace storage {
namespace duckdb {

namespace fs = std::filesystem;

namespace {

constexpr absl::string_view kViewsTable = "main.__bqemu_views";

absl::Status EnsureViewsTable(DuckDBStorage::Impl* impl) {
  return internal::RunSql(
      impl,
      absl::StrCat("CREATE TABLE IF NOT EXISTS ",
                   kViewsTable,
                   " (",
                   "project_id VARCHAR NOT NULL, ",
                   "dataset_id VARCHAR NOT NULL, ",
                   "view_id VARCHAR NOT NULL, ",
                   "ddl_sql VARCHAR NOT NULL, ",
                   "PRIMARY KEY (project_id, dataset_id, view_id)",
                   ")"));
}

absl::Status ValidateViewId(const ViewId& id) {
  if (id.project_id.empty() || id.dataset_id.empty() || id.view_id.empty()) {
    return absl::InvalidArgumentError(
        "view id requires non-empty project_id, dataset_id, view_id");
  }
  return absl::OkStatus();
}

absl::StatusOr<ViewRecord> ReadViewRow(::duckdb_result* result, idx_t row) {
  ViewRecord out;
  out.id.project_id = std::string(::duckdb_value_varchar(result, 0, row));
  out.id.dataset_id = std::string(::duckdb_value_varchar(result, 1, row));
  out.id.view_id = std::string(::duckdb_value_varchar(result, 2, row));
  out.ddl_sql = std::string(::duckdb_value_varchar(result, 3, row));
  return out;
}

absl::StatusOr<std::vector<ViewRecord>> QueryViews(
    DuckDBStorage::Impl* impl, absl::string_view where_clause) {
  std::string sql = absl::StrCat(
      "SELECT project_id, dataset_id, view_id, ddl_sql FROM ", kViewsTable);
  if (!where_clause.empty()) {
    absl::StrAppend(&sql, " WHERE ", where_clause);
  }
  absl::StrAppend(&sql, " ORDER BY project_id, dataset_id, view_id");

  ::duckdb_result result;
  const auto state = ::duckdb_query(impl->connection, sql.c_str(), &result);
  if (state != ::DuckDBSuccess) {
    const auto* err = ::duckdb_result_error(&result);
    std::string detail = err == nullptr ? std::string("") : std::string(err);
    ::duckdb_destroy_result(&result);
    return internal::DuckDBError(
        absl::StatusCode::kInternal, "ListViews query failed", detail);
  }
  std::vector<ViewRecord> out;
  const idx_t n = ::duckdb_row_count(&result);
  out.reserve(static_cast<size_t>(n));
  for (idx_t row = 0; row < n; ++row) {
    auto rec_or = ReadViewRow(&result, row);
    if (!rec_or.ok()) {
      ::duckdb_destroy_result(&result);
      return rec_or.status();
    }
    out.push_back(std::move(*rec_or));
  }
  ::duckdb_destroy_result(&result);
  return out;
}

absl::Status WriteViewSidecar(absl::string_view data_dir,
                              const ViewRecord& record) {
  const TableId table_id{
      record.id.project_id, record.id.dataset_id, record.id.view_id};
  const fs::path ds_dir = fs::path(std::string(data_dir)) /
                          table_id.project_id / table_id.dataset_id;
  std::error_code ec;
  if (!fs::exists(ds_dir, ec)) {
    return absl::NotFoundError(absl::StrCat(
        "dataset not found: ", table_id.project_id, ".", table_id.dataset_id));
  }
  absl::string_view view_query = record.view_query;
  if (view_query.empty()) {
    view_query = record.ddl_sql;
  }
  auto meta_json_or = internal::RenderViewTableMetaJson(
      record.schema, view_query, record.ddl_sql);
  if (!meta_json_or.ok()) return meta_json_or.status();
  const fs::path meta_path =
      ds_dir / absl::StrCat(table_id.table_id, internal::kTableMetaSuffix);
  return internal::WriteFileAtomic(meta_path, *meta_json_or);
}

absl::Status RemoveViewSidecar(absl::string_view data_dir, const ViewId& id) {
  const fs::path meta_path =
      fs::path(std::string(data_dir)) / id.project_id / id.dataset_id /
      absl::StrCat(id.view_id, internal::kTableMetaSuffix);
  std::error_code ec;
  if (!fs::exists(meta_path, ec)) {
    return absl::OkStatus();
  }
  if (!fs::remove(meta_path, ec)) {
    return internal::FilesystemStatus(
        absl::StrCat("failed to remove view sidecar: ", meta_path.string()),
        ec);
  }
  return absl::OkStatus();
}

absl::StatusOr<ViewRecord> ViewRecordFromSidecar(absl::string_view data_dir,
                                                 const TableId& id) {
  const fs::path meta_path =
      fs::path(std::string(data_dir)) / id.project_id / id.dataset_id /
      absl::StrCat(id.table_id, internal::kTableMetaSuffix);
  auto contents_or = internal::ReadFile(meta_path);
  if (!contents_or.ok()) return contents_or.status();
  auto info_or = internal::ParseTableResourceInfo(*contents_or);
  if (!info_or.ok()) return info_or.status();
  if (!absl::EqualsIgnoreCase(info_or->table_type, "VIEW")) {
    return absl::NotFoundError(absl::StrCat("table is not a logical view: ",
                                            id.project_id,
                                            ".",
                                            id.dataset_id,
                                            ".",
                                            id.table_id));
  }
  ViewRecord rec;
  rec.id.project_id = id.project_id;
  rec.id.dataset_id = id.dataset_id;
  rec.id.view_id = id.table_id;
  rec.ddl_sql = info_or->ddl_sql;
  rec.view_query = info_or->view_query;
  auto schema_or = internal::ParseTableMetaJson(*contents_or);
  if (schema_or.ok()) {
    rec.schema = std::move(*schema_or);
  }
  return rec;
}

std::string ViewRecordKey(const ViewId& id) {
  return absl::StrCat(id.project_id, "\x1f", id.dataset_id, "\x1f", id.view_id);
}

void MergeViewSidecars(absl::string_view data_dir,
                       std::vector<ViewRecord>* views,
                       absl::flat_hash_set<std::string>* seen) {
  for (ViewRecord& rec : *views) {
    seen->insert(ViewRecordKey(rec.id));
    TableId tid{rec.id.project_id, rec.id.dataset_id, rec.id.view_id};
    if (auto sidecar_or = ViewRecordFromSidecar(data_dir, tid);
        sidecar_or.ok()) {
      if (rec.view_query.empty()) {
        rec.view_query = sidecar_or->view_query;
      }
      if (rec.schema.columns.empty()) {
        rec.schema = std::move(sidecar_or->schema);
      }
    }
  }
}

void CollectOrphanViewsInDataset(absl::string_view data_dir,
                                 absl::string_view project_id,
                                 absl::string_view dataset_id,
                                 absl::flat_hash_set<std::string>* seen,
                                 std::vector<ViewRecord>* out) {
  const fs::path dataset_dir = fs::path(data_dir) / project_id / dataset_id;
  std::error_code ec;
  for (const auto& table_entry : fs::directory_iterator(dataset_dir, ec)) {
    if (ec) break;
    const std::string name = table_entry.path().filename().string();
    if (name.size() <= internal::kTableMetaSuffix.size()) continue;
    if (!absl::EndsWith(name, internal::kTableMetaSuffix)) continue;
    const std::string table_id =
        name.substr(0, name.size() - internal::kTableMetaSuffix.size());
    ViewId vid{std::string(project_id), std::string(dataset_id), table_id};
    if (!seen->insert(ViewRecordKey(vid)).second) continue;
    TableId tid{std::string(project_id), std::string(dataset_id), table_id};
    auto rec_or = ViewRecordFromSidecar(data_dir, tid);
    if (!rec_or.ok()) continue;
    out->push_back(std::move(*rec_or));
  }
}

void CollectOrphanViewsFromFilesystem(absl::string_view data_dir,
                                      absl::flat_hash_set<std::string>* seen,
                                      std::vector<ViewRecord>* out) {
  const fs::path root(data_dir);
  std::error_code ec;
  if (!fs::exists(root, ec)) {
    return;
  }
  for (const auto& project_entry : fs::directory_iterator(root, ec)) {
    if (ec || !project_entry.is_directory(ec)) continue;
    const std::string project_id = project_entry.path().filename().string();
    for (const auto& dataset_entry :
         fs::directory_iterator(project_entry.path(), ec)) {
      if (ec || !dataset_entry.is_directory(ec)) continue;
      const std::string dataset_id = dataset_entry.path().filename().string();
      CollectOrphanViewsInDataset(data_dir, project_id, dataset_id, seen, out);
    }
  }
}

}  // namespace

absl::Status DuckDBStorage::UpsertView(const ViewRecord& record) {
  absl::Status id_status = ValidateViewId(record.id);
  if (!id_status.ok()) return id_status;
  if (record.ddl_sql.empty()) {
    return absl::InvalidArgumentError("UpsertView: ddl_sql is required");
  }
  absl::MutexLock lock(&mu_);
  absl::Status sidecar = WriteViewSidecar(data_dir_, record);
  if (!sidecar.ok()) return sidecar;

  absl::Status init = EnsureViewsTable(impl_.get());
  if (!init.ok()) return init;

  const std::string sql = absl::StrCat(
      "INSERT INTO ",
      kViewsTable,
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
  return internal::RunSql(impl_.get(), sql);
}

absl::Status DuckDBStorage::DeleteView(const ViewId& id) {
  absl::Status id_status = ValidateViewId(id);
  if (!id_status.ok()) return id_status;
  absl::MutexLock lock(&mu_);
  absl::Status removed = RemoveViewSidecar(data_dir_, id);
  if (!removed.ok()) return removed;

  absl::Status init = EnsureViewsTable(impl_.get());
  if (!init.ok()) return init;
  const std::string sql =
      absl::StrCat("DELETE FROM ",
                   kViewsTable,
                   " WHERE project_id = '",
                   internal::EscapeStringLiteralInner(id.project_id),
                   "' AND dataset_id = '",
                   internal::EscapeStringLiteralInner(id.dataset_id),
                   "' AND view_id = '",
                   internal::EscapeStringLiteralInner(id.view_id),
                   "'");
  return internal::RunSql(impl_.get(), sql);
}

absl::StatusOr<std::vector<ViewRecord>> DuckDBStorage::ListAllViews() const {
  absl::MutexLock lock(&mu_);
  if (impl_ == nullptr) {
    return absl::InternalError("DuckDBStorage::ListAllViews: not open");
  }
  absl::flat_hash_set<std::string> seen;
  absl::StatusOr<std::vector<ViewRecord>> from_db = QueryViews(impl_.get(), "");
  if (!from_db.ok()) return from_db.status();
  std::vector<ViewRecord> out = std::move(*from_db);
  MergeViewSidecars(data_dir_, &out, &seen);
  CollectOrphanViewsFromFilesystem(data_dir_, &seen, &out);
  std::sort(
      out.begin(), out.end(), [](const ViewRecord& a, const ViewRecord& b) {
        if (a.id.project_id != b.id.project_id) {
          return a.id.project_id < b.id.project_id;
        }
        if (a.id.dataset_id != b.id.dataset_id) {
          return a.id.dataset_id < b.id.dataset_id;
        }
        return a.id.view_id < b.id.view_id;
      });
  return out;
}

absl::StatusOr<TableResourceInfo> DuckDBStorage::GetTableResourceInfo(
    const TableId& id) const {
  absl::MutexLock lock(&mu_);
  const fs::path meta_path =
      fs::path(data_dir_) / id.project_id / id.dataset_id /
      absl::StrCat(id.table_id, internal::kTableMetaSuffix);
  auto contents_or = internal::ReadFile(meta_path);
  if (!contents_or.ok()) return contents_or.status();
  return internal::ParseTableResourceInfo(*contents_or);
}

}  // namespace duckdb
}  // namespace storage
}  // namespace backend
}  // namespace bigquery_emulator
