#include <filesystem>
#include <memory>
#include <string>
#include <system_error>
#include <utility>
#include <vector>

#include "absl/status/statusor.h"
#include "absl/strings/str_cat.h"
#include "absl/synchronization/mutex.h"
#include "backend/schema/schema.h"
#include "backend/storage/duckdb/duckdb_storage.h"
#include "backend/storage/duckdb/duckdb_storage_internal.h"
#include "backend/storage/storage.h"

namespace bigquery_emulator {
namespace backend {
namespace storage {
namespace duckdb {

namespace fs = std::filesystem;

namespace {

struct ReadStreamPlan {
  schema::TableSchema effective_schema;
  std::string select_cols;
};

absl::StatusOr<ReadStreamPlan> PlanReadStreamProjection(
    const schema::TableSchema& physical, const ReadFilter& filter) {
  ReadStreamPlan plan;
  plan.effective_schema = physical;
  if (filter.selected_fields.empty()) {
    plan.select_cols = internal::RenderColumnIdentList(physical);
    return plan;
  }
  auto projected_or = internal::ProjectSchema(physical, filter.selected_fields);
  if (!projected_or.ok()) return projected_or.status();
  plan.effective_schema = std::move(*projected_or);
  plan.select_cols =
      internal::RenderSelectedColumnIdentList(physical, filter.selected_fields);
  return plan;
}

void AppendReadStreamFilters(const ReadFilter& filter, std::string* sql) {
  if (filter.where_sql.has_value() && !filter.where_sql->empty()) {
    absl::StrAppend(sql, internal::RenderWhereSqlClause(*filter.where_sql));
  } else if (filter.equality_predicate.has_value()) {
    absl::StrAppend(
        *sql, internal::RenderPredicateClause(*filter.equality_predicate));
  }
  if (filter.row_start > 0 || filter.row_end >= 0) {
    const bool has_where = sql->find(" WHERE ") != std::string::npos;
    if (has_where) {
      if (filter.row_start > 0) {
        absl::StrAppend(*sql, " AND file_row_number >= ", filter.row_start);
      }
      if (filter.row_end >= 0) {
        absl::StrAppend(*sql, " AND file_row_number < ", filter.row_end);
      }
    } else {
      absl::StrAppend(
          *sql,
          internal::RenderRowPartitionClause(filter.row_start, filter.row_end));
    }
  }
}

void AppendReadStreamPagination(const ReadFilter& filter, std::string* sql) {
  absl::StrAppend(sql, " ORDER BY file_row_number");
  if (filter.row_limit > 0) {
    absl::StrAppend(sql, " LIMIT ", filter.row_limit);
  }
  if (filter.offset > 0) {
    if (filter.row_limit <= 0) {
      absl::StrAppend(sql, " LIMIT ALL");
    }
    absl::StrAppend(sql, " OFFSET ", filter.offset);
  }
}

std::string BuildReadStreamSql(const std::string& parquet_path,
                               absl::string_view select_cols,
                               const ReadFilter& filter) {
  std::string sql =
      absl::StrCat("SELECT ",
                   select_cols,
                   " FROM read_parquet('",
                   internal::EscapeStringLiteralInner(parquet_path),
                   "', file_row_number = true)");
  AppendReadStreamFilters(filter, &sql);
  AppendReadStreamPagination(filter, &sql);
  return sql;
}

}  // namespace

absl::StatusOr<std::unique_ptr<RowIterator>> DuckDBStorage::CreateReadStream(
    const TableId& id, const ReadFilter& filter) const {
  absl::MutexLock lock(&mu_);
  const fs::path ds_dir = DatasetDir(id.project_id, id.dataset_id);
  std::error_code ec;
  if (!fs::exists(ds_dir, ec)) {
    return absl::NotFoundError(
        absl::StrCat("dataset not found: ", id.project_id, ".", id.dataset_id));
  }
  const fs::path meta_path = TableMetaPath(id);
  if (!fs::exists(meta_path, ec)) {
    return absl::NotFoundError(absl::StrCat("table not found: ",
                                            id.project_id,
                                            ".",
                                            id.dataset_id,
                                            ".",
                                            id.table_id));
  }
  auto contents_or = internal::ReadFile(meta_path);
  if (!contents_or.ok()) return contents_or.status();
  auto schema_or = internal::ParseTableMetaJson(*contents_or);
  if (!schema_or.ok()) return schema_or.status();
  const schema::TableSchema& schema = *schema_or;
  const schema::TableSchema physical = internal::ParquetStorageSchema(schema);

  const std::string parquet_path = TableParquetPath(id);
  std::vector<Row> rows;
  if (!fs::exists(parquet_path, ec)) {
    return std::unique_ptr<RowIterator>(
        new internal::VectorRowIterator(std::move(rows)));
  }

  auto plan_or = PlanReadStreamProjection(physical, filter);
  if (!plan_or.ok()) return plan_or.status();

  const std::string sql =
      BuildReadStreamSql(parquet_path, plan_or->select_cols, filter);
  auto status = internal::ExecuteSelect(impl_.get(),
                                        sql,
                                        plan_or->effective_schema,
                                        "CreateReadStream",
                                        id,
                                        parquet_path,
                                        &rows);
  if (!status.ok()) return status;
  return std::unique_ptr<RowIterator>(
      new internal::VectorRowIterator(std::move(rows)));
}

}  // namespace duckdb
}  // namespace storage
}  // namespace backend
}  // namespace bigquery_emulator
