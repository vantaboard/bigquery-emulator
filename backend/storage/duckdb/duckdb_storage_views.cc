#include <string>
#include <utility>
#include <vector>

#include "absl/status/status.h"
#include "absl/status/statusor.h"
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

absl::StatusOr<std::vector<ViewRecord>> QueryViews(DuckDBStorage::Impl* impl,
                                                   absl::string_view where_clause) {
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

}  // namespace

absl::Status DuckDBStorage::UpsertView(const ViewRecord& record) {
  absl::Status id_status = ValidateViewId(record.id);
  if (!id_status.ok()) return id_status;
  if (record.ddl_sql.empty()) {
    return absl::InvalidArgumentError("UpsertView: ddl_sql is required");
  }
  absl::MutexLock lock(&mu_);
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
  absl::Status init = EnsureViewsTable(impl_.get());
  if (!init.ok()) return init;
  const std::string sql = absl::StrCat(
      "DELETE FROM ",
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
  return QueryViews(impl_.get(), "");
}

}  // namespace duckdb
}  // namespace storage
}  // namespace backend
}  // namespace bigquery_emulator
