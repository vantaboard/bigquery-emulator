#include <algorithm>
#include <filesystem>
#include <string>
#include <system_error>
#include <utility>
#include <vector>

#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "absl/strings/match.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/string_view.h"
#include "absl/synchronization/mutex.h"
#include "absl/time/time.h"
#include "backend/schema/schema.h"
#include "backend/storage/duckdb/duckdb_storage.h"
#include "backend/storage/duckdb/duckdb_storage_internal.h"
#include "backend/storage/duckdb/duckdb_storage_version_log.h"
#include "duckdb.h"

namespace bigquery_emulator {
namespace backend {
namespace storage {
namespace duckdb {

namespace fs = std::filesystem;

// ---------------------------------------------------------------------------
// Dataset CRUD
// ---------------------------------------------------------------------------

absl::Status DuckDBStorage::CreateDataset(const DatasetId& id,
                                          absl::string_view location) {
  if (id.project_id.empty() || id.dataset_id.empty()) {
    return absl::InvalidArgumentError(
        "CreateDataset: project_id and dataset_id must be non-empty");
  }
  absl::MutexLock lock(&mu_);
  const fs::path ds_dir = DatasetDir(id);
  std::error_code ec;
  if (fs::exists(ds_dir, ec)) {
    return absl::AlreadyExistsError(absl::StrCat(
        "dataset already exists: ", id.project_id, ".", id.dataset_id));
  }
  fs::create_directories(ds_dir, ec);
  if (ec) {
    return internal::FilesystemStatus(
        absl::StrCat("failed to create dataset dir: ", ds_dir.string()), ec);
  }
  const auto meta_status = internal::WriteFileAtomic(
      DatasetMetaPath(id), internal::RenderDatasetMetaJson(location));
  if (!meta_status.ok()) {
    fs::remove_all(ds_dir, ec);
    return meta_status;
  }
  const std::string schema_name = DuckDBSchemaName(id);
  // CREATE SCHEMA so the catalog file mirrors the on-disk layout.
  // The two are not strictly redundant: the schema lets the DuckDB
  // engine attach tables by qualified name without re-deriving the
  // directory from the data dir on every query.
  const auto sql_status =
      internal::RunSql(impl_.get(),
                       absl::StrCat("CREATE SCHEMA IF NOT EXISTS ",
                                    internal::QuoteIdent(schema_name)));
  if (!sql_status.ok()) {
    fs::remove_all(ds_dir, ec);
    return sql_status;
  }
  return absl::OkStatus();
}

absl::Status DuckDBStorage::DropDataset(const DatasetId& id,
                                        bool delete_contents) {
  absl::MutexLock lock(&mu_);
  const fs::path ds_dir = DatasetDir(id);
  std::error_code ec;
  if (!fs::exists(ds_dir, ec)) {
    return absl::NotFoundError(
        absl::StrCat("dataset not found: ", id.project_id, ".", id.dataset_id));
  }
  // Count non-metadata entries (any `.meta.json` / `.parquet` pair
  // counts as one table; the dataset sidecar is excluded).
  bool has_tables = false;
  for (const auto& entry : fs::directory_iterator(ds_dir, ec)) {
    if (ec) break;
    const std::string name = entry.path().filename().string();
    if (name == internal::kDatasetMetaFile) continue;
    if (name.size() > internal::kTableMetaSuffix.size() &&
        absl::EndsWith(name, internal::kTableMetaSuffix)) {
      has_tables = true;
      break;
    }
    if (entry.path().extension() == ".parquet") {
      has_tables = true;
      break;
    }
  }
  if (ec) {
    return internal::FilesystemStatus(
        absl::StrCat("failed to enumerate dataset dir: ", ds_dir.string()), ec);
  }
  if (has_tables && !delete_contents) {
    return absl::FailedPreconditionError(
        absl::StrCat("dataset is not empty: ",
                     id.project_id,
                     ".",
                     id.dataset_id,
                     " (use delete_contents=true to drop with tables)"));
  }
  fs::remove_all(ds_dir, ec);
  if (ec) {
    return internal::FilesystemStatus(
        absl::StrCat("failed to remove dataset dir: ", ds_dir.string()), ec);
  }
  // Drop the matching DuckDB schema. We use CASCADE here to mirror
  // the file-tree removal above (every table file is gone, so any
  // catalog references in DuckDB must go too).
  const std::string schema_name = DuckDBSchemaName(id);
  return internal::RunSql(impl_.get(),
                          absl::StrCat("DROP SCHEMA IF EXISTS ",
                                       internal::QuoteIdent(schema_name),
                                       " CASCADE"));
}

absl::StatusOr<std::vector<DatasetId>> DuckDBStorage::ListDatasets(
    absl::string_view project_id) const {
  if (project_id.empty()) {
    return absl::InvalidArgumentError(
        "ListDatasets: project_id must be non-empty");
  }
  absl::MutexLock lock(&mu_);
  // The DuckDB catalog mirrors the on-disk layout but the filesystem
  // is the canonical source of truth here (CreateDataset writes the
  // dir + sidecar BEFORE the DuckDB CREATE SCHEMA, and DropDataset
  // does the reverse), so we enumerate by directory walk to avoid any
  // race between the two. Same posture DropDataset uses to decide
  // whether the dataset is empty.
  const fs::path project_dir = fs::path(data_dir_) / std::string(project_id);
  std::vector<DatasetId> out;
  std::error_code ec;
  if (!fs::exists(project_dir, ec)) {
    return out;
  }
  for (const auto& entry : fs::directory_iterator(project_dir, ec)) {
    if (ec) break;
    if (!entry.is_directory(ec)) continue;
    const fs::path meta =
        entry.path() / std::string(internal::kDatasetMetaFile);
    // Require the sidecar so partially-mkdir'd directories left behind
    // by an interrupted CreateDataset do not surface as listable
    // datasets; CreateDataset writes the sidecar before any catalog
    // mutation, so its presence is the canonical "this is a dataset"
    // marker.
    if (!fs::exists(meta, ec)) continue;
    out.push_back(
        DatasetId{std::string(project_id), entry.path().filename().string()});
  }
  if (ec) {
    return internal::FilesystemStatus(
        absl::StrCat("failed to enumerate project dir: ", project_dir.string()),
        ec);
  }
  // Deterministic ordering (lexicographic) for stable pagination /
  // diff-against-prior-listing behaviour the Storage contract
  // documents.
  std::sort(out.begin(), out.end(), [](const DatasetId& a, const DatasetId& b) {
    return a.dataset_id < b.dataset_id;
  });
  return out;
}

// ---------------------------------------------------------------------------
// Table CRUD
// ---------------------------------------------------------------------------

absl::Status DuckDBStorage::CreateTable(const TableId& id,
                                        const schema::TableSchema& schema) {
  if (id.table_id.empty()) {
    return absl::InvalidArgumentError(
        "CreateTable: table_id must be non-empty");
  }
  absl::MutexLock lock(&mu_);
  const fs::path ds_dir = DatasetDir(id.project_id, id.dataset_id);
  std::error_code ec;
  if (!fs::exists(ds_dir, ec)) {
    return absl::NotFoundError(
        absl::StrCat("dataset not found: ", id.project_id, ".", id.dataset_id));
  }
  const fs::path meta_path = TableMetaPath(id);
  if (fs::exists(meta_path, ec)) {
    return absl::AlreadyExistsError(absl::StrCat("table already exists: ",
                                                 id.project_id,
                                                 ".",
                                                 id.dataset_id,
                                                 ".",
                                                 id.table_id));
  }
  auto meta_json_or = internal::RenderTableMetaJson(schema);
  if (!meta_json_or.ok()) return meta_json_or.status();
  const auto write_status = internal::WriteFileAtomic(meta_path, *meta_json_or);
  if (!write_status.ok()) return write_status;

  // Materialize an empty Parquet file alongside the sidecar so the
  // table file exists from day one: subsequent `read_parquet(...)`
  // calls in ScanRows can rely on the path being valid even before
  // any rows are appended, and a hand-curated --data_dir is
  // inspectable with stock parquet tools without going through the
  // emulator. The DDL plan (`duckdb-storage-ddl_p1e2f3a4`) lays the
  // file out via a transient DuckDB table because COPY needs a
  // bound logical type list, and the empty-result `SELECT ... WHERE
  // FALSE` trick loses the column-type information.
  //
  // Schema-less tables (the view / external-table / legacy DDL
  // surface registers metadata through CreateTable with an empty
  // column list) skip the DuckDB scratch entirely: DuckDB rejects
  // `CREATE TEMP TABLE foo ()` with "Parser Error: Table must have
  // at least one column!" and an empty parquet schema would not
  // round-trip through `read_parquet` anyway. ScanRows already
  // tolerates a missing parquet file (returns an empty iterator),
  // so leaving the sidecar in place is the conservative behavior;
  // a subsequent AppendRows / OverwriteRows path on the same table
  // is the caller's bug and will surface its own RenderColumnList
  // error.
  const std::string parquet_path = TableParquetPath(id);
  if (schema.columns.empty()) {
    return absl::OkStatus();
  }
  const std::string tmp_table = "main.__bqemu_mkempty";
  const std::string cols = internal::RenderColumnList(schema);
  const auto create_status = internal::RunSql(
      impl_.get(),
      absl::StrCat("CREATE OR REPLACE TEMP TABLE ", tmp_table, " ", cols));
  if (!create_status.ok()) {
    fs::remove(meta_path, ec);
    return create_status;
  }
  const auto copy_status = internal::RunSql(
      impl_.get(),
      absl::StrCat("COPY ",
                   tmp_table,
                   " TO '",
                   internal::EscapeStringLiteralInner(parquet_path),
                   "' (FORMAT PARQUET)"));
  const auto drop_status =
      internal::RunSql(impl_.get(), absl::StrCat("DROP TABLE ", tmp_table));
  if (!copy_status.ok()) {
    fs::remove(meta_path, ec);
    fs::remove(parquet_path, ec);
    return copy_status;
  }
  if (!drop_status.ok()) return drop_status;
  const std::int64_t created_ts_ms = absl::ToUnixMillis(absl::Now());
  return internal::InitVersionIndex(*this, id, created_ts_ms);
}

absl::StatusOr<std::vector<TableId>> DuckDBStorage::ListTables(
    const DatasetId& dataset_id) const {
  if (dataset_id.project_id.empty() || dataset_id.dataset_id.empty()) {
    return absl::InvalidArgumentError(
        "ListTables: project_id and dataset_id must be non-empty");
  }
  absl::MutexLock lock(&mu_);
  const fs::path ds_dir = DatasetDir(dataset_id);
  std::vector<TableId> out;
  std::error_code ec;
  if (!fs::exists(ds_dir, ec)) {
    return absl::NotFoundError(absl::StrCat("dataset not found: ",
                                            dataset_id.project_id,
                                            ".",
                                            dataset_id.dataset_id));
  }
  // Tables on disk are the `<table_id>.meta.json` sidecars next to the
  // dataset's `_dataset.meta.json`. The parquet file may or may not
  // exist (view / external / empty-schema tables skip the parquet
  // materialization; see CreateTable comments), so the sidecar is the
  // canonical existence marker the same way DropTable uses it.
  for (const auto& entry : fs::directory_iterator(ds_dir, ec)) {
    if (ec) break;
    const std::string name = entry.path().filename().string();
    if (name == internal::kDatasetMetaFile) continue;
    if (name.size() <= internal::kTableMetaSuffix.size()) continue;
    if (!absl::EndsWith(name, internal::kTableMetaSuffix)) continue;
    std::string table_id =
        name.substr(0, name.size() - internal::kTableMetaSuffix.size());
    out.push_back(TableId{
        dataset_id.project_id, dataset_id.dataset_id, std::move(table_id)});
  }
  if (ec) {
    return internal::FilesystemStatus(
        absl::StrCat("failed to enumerate dataset dir: ", ds_dir.string()), ec);
  }
  std::sort(out.begin(), out.end(), [](const TableId& a, const TableId& b) {
    return a.table_id < b.table_id;
  });
  return out;
}

absl::Status DuckDBStorage::DropTable(const TableId& id) {
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
  const std::int64_t deleted_ms = absl::ToUnixMillis(absl::Now());
  return internal::MoveTableToTombstone(*this, id, deleted_ms);
}

absl::Status DuckDBStorage::RestoreTable(const TableId& id,
                                         std::int64_t deleted_ms) {
  absl::MutexLock lock(&mu_);
  return internal::RestoreTableFromTombstone(*this, id, deleted_ms);
}

absl::StatusOr<schema::TableSchema> DuckDBStorage::GetSchema(
    const TableId& id) const {
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
  return internal::ParseTableMetaJson(*contents_or);
}

}  // namespace duckdb
}  // namespace storage
}  // namespace backend
}  // namespace bigquery_emulator
