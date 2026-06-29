#include <cstddef>
#include <string>
#include <utility>
#include <vector>

#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "absl/strings/ascii.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/string_view.h"
#include "absl/synchronization/mutex.h"
#include "backend/storage/duckdb/duckdb_storage.h"
#include "backend/storage/duckdb/duckdb_storage_governance.h"
#include "backend/storage/duckdb/duckdb_storage_internal.h"
#include "backend/storage/storage.h"
#include "duckdb.h"

namespace bigquery_emulator {
namespace backend {
namespace storage {
namespace duckdb {

namespace {

constexpr absl::string_view kRoutinesTable = "main.__bqemu_routines";

absl::Status EnsureRoutinesTable(DuckDBStorage::Impl* impl) {
  return internal::RunSql(
      impl,
      absl::StrCat("CREATE TABLE IF NOT EXISTS ",
                   kRoutinesTable,
                   " (",
                   "project_id VARCHAR NOT NULL, ",
                   "dataset_id VARCHAR NOT NULL, ",
                   "routine_id VARCHAR NOT NULL, ",
                   "kind VARCHAR NOT NULL, ",
                   "language VARCHAR NOT NULL, ",
                   "ddl_sql VARCHAR NOT NULL, ",
                   "is_temp BOOLEAN NOT NULL DEFAULT FALSE, ",
                   "signature_json VARCHAR, ",
                   "PRIMARY KEY (project_id, dataset_id, routine_id)",
                   ")"));
}

std::string RoutineKindToString(RoutineKind kind) {
  switch (kind) {
    case RoutineKind::kScalarFunction:
      return "scalar_function";
    case RoutineKind::kAggregateFunction:
      return "aggregate_function";
    case RoutineKind::kTableValuedFunction:
      return "table_valued_function";
    case RoutineKind::kProcedure:
      return "procedure";
  }
  return "scalar_function";
}

absl::StatusOr<RoutineKind> RoutineKindFromString(absl::string_view s) {
  const std::string lower = absl::AsciiStrToLower(s);
  if (lower == "scalar_function") return RoutineKind::kScalarFunction;
  if (lower == "aggregate_function") return RoutineKind::kAggregateFunction;
  if (lower == "table_valued_function") {
    return RoutineKind::kTableValuedFunction;
  }
  if (lower == "procedure") return RoutineKind::kProcedure;
  return absl::InvalidArgumentError(absl::StrCat("unknown routine kind: ", s));
}

absl::Status ValidateRoutineId(const RoutineId& id) {
  if (id.project_id.empty() || id.dataset_id.empty() || id.routine_id.empty()) {
    return absl::InvalidArgumentError(
        "routine id requires non-empty project_id, dataset_id, routine_id");
  }
  return absl::OkStatus();
}

absl::StatusOr<RoutineRecord> ReadRoutineRow(::duckdb_result* result,
                                             idx_t row) {
  RoutineRecord out;
  out.id.project_id = std::string(::duckdb_value_varchar(result, 0, row));
  out.id.dataset_id = std::string(::duckdb_value_varchar(result, 1, row));
  out.id.routine_id = std::string(::duckdb_value_varchar(result, 2, row));
  auto kind_or = RoutineKindFromString(::duckdb_value_varchar(result, 3, row));
  if (!kind_or.ok()) return kind_or.status();
  out.kind = *kind_or;
  out.language = std::string(::duckdb_value_varchar(result, 4, row));
  out.ddl_sql = std::string(::duckdb_value_varchar(result, 5, row));
  out.is_temp = ::duckdb_value_boolean(result, 6, row);
  if (!::duckdb_value_is_null(result, 7, row)) {
    out.signature_json = std::string(::duckdb_value_varchar(result, 7, row));
  }
  return out;
}

absl::StatusOr<std::vector<RoutineRecord>> QueryRoutines(
    DuckDBStorage::Impl* impl, absl::string_view where_clause) {
  std::string sql = absl::StrCat(
      "SELECT project_id, dataset_id, routine_id, kind, language, ",
      "ddl_sql, is_temp, signature_json FROM ",
      kRoutinesTable);
  if (!where_clause.empty()) {
    absl::StrAppend(&sql, " WHERE ", where_clause);
  }
  absl::StrAppend(&sql, " ORDER BY project_id, dataset_id, routine_id");

  ::duckdb_result result;
  const auto state = ::duckdb_query(impl->connection, sql.c_str(), &result);
  if (state != ::DuckDBSuccess) {
    const auto* err = ::duckdb_result_error(&result);
    std::string detail = err == nullptr ? std::string("") : std::string(err);
    ::duckdb_destroy_result(&result);
    return internal::DuckDBError(
        absl::StatusCode::kInternal, "ListRoutines query failed", detail);
  }
  std::vector<RoutineRecord> out;
  const idx_t n = ::duckdb_row_count(&result);
  out.reserve(static_cast<size_t>(n));
  for (idx_t row = 0; row < n; ++row) {
    auto rec_or = ReadRoutineRow(&result, row);
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

absl::Status DuckDBStorage::InitCatalogTables() {
  absl::MutexLock lock(&mu_);
  absl::Status routines = EnsureRoutinesTable(impl_.get());
  if (!routines.ok()) return routines;
  absl::Status views =
      internal::RunSql(impl_.get(),
                       "CREATE TABLE IF NOT EXISTS main.__bqemu_views ("
                       "project_id VARCHAR NOT NULL, "
                       "dataset_id VARCHAR NOT NULL, "
                       "view_id VARCHAR NOT NULL, "
                       "ddl_sql VARCHAR NOT NULL, "
                       "PRIMARY KEY (project_id, dataset_id, view_id))");
  if (!views.ok()) return views;
  return EnsureGovernanceTables(impl_.get());
}

absl::Status DuckDBStorage::UpsertRoutine(const RoutineRecord& record) {
  absl::Status id_status = ValidateRoutineId(record.id);
  if (!id_status.ok()) return id_status;
  if (record.ddl_sql.empty()) {
    return absl::InvalidArgumentError("UpsertRoutine: ddl_sql is required");
  }
  absl::MutexLock lock(&mu_);
  absl::Status init = EnsureRoutinesTable(impl_.get());
  if (!init.ok()) return init;

  const std::string sql = absl::StrCat(
      "INSERT INTO ",
      kRoutinesTable,
      " (project_id, dataset_id, routine_id, kind, language, ddl_sql, ",
      "is_temp, signature_json) VALUES (",
      "'",
      internal::EscapeStringLiteralInner(record.id.project_id),
      "', '",
      internal::EscapeStringLiteralInner(record.id.dataset_id),
      "', '",
      internal::EscapeStringLiteralInner(record.id.routine_id),
      "', '",
      RoutineKindToString(record.kind),
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
  return internal::RunSql(impl_.get(), sql);
}

absl::Status DuckDBStorage::DeleteRoutine(const RoutineId& id) {
  absl::Status id_status = ValidateRoutineId(id);
  if (!id_status.ok()) return id_status;
  absl::MutexLock lock(&mu_);
  const std::string sql =
      absl::StrCat("DELETE FROM ",
                   kRoutinesTable,
                   " WHERE project_id = '",
                   internal::EscapeStringLiteralInner(id.project_id),
                   "' AND dataset_id = '",
                   internal::EscapeStringLiteralInner(id.dataset_id),
                   "' AND routine_id = '",
                   internal::EscapeStringLiteralInner(id.routine_id),
                   "'");
  absl::Status deleted = internal::RunSql(impl_.get(), sql);
  if (!deleted.ok()) return deleted;
  return absl::OkStatus();
}

absl::StatusOr<RoutineRecord> DuckDBStorage::GetRoutine(
    const RoutineId& id) const {
  absl::Status id_status = ValidateRoutineId(id);
  if (!id_status.ok()) return id_status;
  absl::MutexLock lock(&mu_);
  const std::string where =
      absl::StrCat("project_id = '",
                   internal::EscapeStringLiteralInner(id.project_id),
                   "' AND dataset_id = '",
                   internal::EscapeStringLiteralInner(id.dataset_id),
                   "' AND routine_id = '",
                   internal::EscapeStringLiteralInner(id.routine_id),
                   "'");
  auto rows_or = QueryRoutines(impl_.get(), where);
  if (!rows_or.ok()) return rows_or.status();
  if (rows_or->empty()) {
    return absl::NotFoundError(absl::StrCat("routine not found: ",
                                            id.project_id,
                                            ".",
                                            id.dataset_id,
                                            ".",
                                            id.routine_id));
  }
  return (*rows_or)[0];
}

absl::StatusOr<std::vector<RoutineRecord>> DuckDBStorage::ListRoutines(
    const DatasetId& dataset_id) const {
  if (dataset_id.project_id.empty() || dataset_id.dataset_id.empty()) {
    return absl::InvalidArgumentError(
        "ListRoutines: project_id and dataset_id must be non-empty");
  }
  absl::MutexLock lock(&mu_);
  const std::string where =
      absl::StrCat("project_id = '",
                   internal::EscapeStringLiteralInner(dataset_id.project_id),
                   "' AND dataset_id = '",
                   internal::EscapeStringLiteralInner(dataset_id.dataset_id),
                   "'");
  return QueryRoutines(impl_.get(), where);
}

absl::StatusOr<std::vector<RoutineRecord>> DuckDBStorage::ListAllRoutines()
    const {
  absl::MutexLock lock(&mu_);
  return QueryRoutines(impl_.get(), "");
}

}  // namespace duckdb
}  // namespace storage
}  // namespace backend
}  // namespace bigquery_emulator
