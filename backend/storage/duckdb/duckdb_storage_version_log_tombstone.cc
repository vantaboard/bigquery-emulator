#include <filesystem>
#include <string>
#include <system_error>

#include "absl/strings/numbers.h"
#include "absl/strings/str_cat.h"
#include "backend/storage/duckdb/duckdb_storage_internal.h"
#include "backend/storage/duckdb/duckdb_storage_version_log.h"

namespace bigquery_emulator {
namespace backend {
namespace storage {
namespace duckdb {
namespace internal {

namespace fs = std::filesystem;

std::string DatasetDirFromStorage(const DuckDBStorage& storage,
                                  const TableId& id);
std::string TableMetaPathFromStorage(const DuckDBStorage& storage,
                                     const TableId& id);
std::string TableParquetPathFromStorage(const DuckDBStorage& storage,
                                        const TableId& id);
std::string TableVersionsIndexPath(const DuckDBStorage& storage,
                                   const TableId& id);
std::string TableVersionsDir(const DuckDBStorage& storage, const TableId& id);

namespace {

absl::StatusOr<std::int64_t> LatestTombstoneMs(const DuckDBStorage& storage,
                                               const TableId& id) {
  const fs::path root =
      fs::path(storage.data_dir()) / ".tombstones" / id.table_id;
  std::error_code ec;
  if (!fs::exists(root, ec)) {
    return absl::NotFoundError(absl::StrCat("no tombstones for table ",
                                            id.project_id,
                                            ".",
                                            id.dataset_id,
                                            ".",
                                            id.table_id));
  }
  std::int64_t best = 0;
  bool found = false;
  for (const auto& entry : fs::directory_iterator(root, ec)) {
    if (ec) break;
    if (!entry.is_directory()) continue;
    std::int64_t deleted_ms = 0;
    if (!absl::SimpleAtoi(entry.path().filename().string(), &deleted_ms)) {
      continue;
    }
    if (!found || deleted_ms > best) {
      best = deleted_ms;
      found = true;
    }
  }
  if (!found) {
    return absl::NotFoundError(absl::StrCat("no tombstones for table ",
                                            id.project_id,
                                            ".",
                                            id.dataset_id,
                                            ".",
                                            id.table_id));
  }
  return best;
}

}  // namespace

absl::Status MoveTableToTombstone(const DuckDBStorage& storage,
                                  const TableId& id,
                                  std::int64_t deleted_ms) {
  const std::string tombstone_dir = TableTombstoneDir(storage, id, deleted_ms);
  std::error_code ec;
  fs::create_directories(tombstone_dir, ec);
  if (ec) {
    return FilesystemStatus(
        absl::StrCat("failed to create tombstone dir: ", tombstone_dir), ec);
  }

  auto move_if_exists = [&](const std::string& src_path) -> absl::Status {
    std::error_code move_ec;
    if (!fs::exists(src_path, move_ec)) return absl::OkStatus();
    const fs::path dest =
        fs::path(tombstone_dir) / fs::path(src_path).filename();
    fs::rename(src_path, dest, move_ec);
    if (move_ec) {
      return FilesystemStatus(
          absl::StrCat("failed to move ", src_path, " -> ", dest.string()),
          move_ec);
    }
    return absl::OkStatus();
  };

  absl::Status status = move_if_exists(TableMetaPathFromStorage(storage, id));
  if (!status.ok()) return status;
  status = move_if_exists(TableParquetPathFromStorage(storage, id));
  if (!status.ok()) return status;
  status = move_if_exists(TableVersionsIndexPath(storage, id));
  if (!status.ok()) return status;

  const std::string versions_dir = TableVersionsDir(storage, id);
  if (fs::exists(versions_dir, ec)) {
    const fs::path dest =
        fs::path(tombstone_dir) / (absl::StrCat(id.table_id, ".versions"));
    fs::rename(versions_dir, dest, ec);
    if (ec) {
      return FilesystemStatus(
          absl::StrCat("failed to move versions dir to ", dest.string()), ec);
    }
  }
  return absl::OkStatus();
}

absl::Status RestoreTableFromTombstone(const DuckDBStorage& storage,
                                       const TableId& id,
                                       std::int64_t deleted_ms) {
  std::int64_t chosen = deleted_ms;
  if (chosen == 0) {
    auto latest_or = LatestTombstoneMs(storage, id);
    if (!latest_or.ok()) return latest_or.status();
    chosen = *latest_or;
  }

  const fs::path ds_dir = fs::path(DatasetDirFromStorage(storage, id));
  std::error_code ec;
  const fs::path meta_path = TableMetaPathFromStorage(storage, id);
  if (fs::exists(meta_path, ec)) {
    return absl::AlreadyExistsError(absl::StrCat("table already exists: ",
                                                 id.project_id,
                                                 ".",
                                                 id.dataset_id,
                                                 ".",
                                                 id.table_id));
  }

  const std::string tombstone_dir = TableTombstoneDir(storage, id, chosen);
  if (!fs::exists(tombstone_dir, ec)) {
    return absl::NotFoundError(absl::StrCat("tombstone not found for table ",
                                            id.project_id,
                                            ".",
                                            id.dataset_id,
                                            ".",
                                            id.table_id,
                                            " at ",
                                            chosen));
  }

  fs::create_directories(ds_dir, ec);
  if (ec) {
    return FilesystemStatus(
        absl::StrCat("failed to create dataset dir: ", ds_dir.string()), ec);
  }

  auto move_entry = [&](const fs::path& name) -> absl::Status {
    const fs::path src = fs::path(tombstone_dir) / name;
    if (!fs::exists(src, ec)) return absl::OkStatus();
    const fs::path dest = ds_dir / name;
    fs::rename(src, dest, ec);
    if (ec) {
      return FilesystemStatus(
          absl::StrCat(
              "failed to restore ", src.string(), " -> ", dest.string()),
          ec);
    }
    return absl::OkStatus();
  };

  absl::Status status = move_entry(
      fs::path(absl::StrCat(id.table_id, kTableMetaSuffix)).filename());
  if (!status.ok()) return status;
  status =
      move_entry(fs::path(absl::StrCat(id.table_id, ".parquet")).filename());
  if (!status.ok()) return status;
  status = move_entry(
      fs::path(absl::StrCat(id.table_id, ".versions.json")).filename());
  if (!status.ok()) return status;

  const fs::path versions_src =
      fs::path(tombstone_dir) / absl::StrCat(id.table_id, ".versions");
  if (fs::exists(versions_src, ec)) {
    const fs::path versions_dest =
        fs::path(DatasetDirFromStorage(storage, id)) /
        absl::StrCat(id.table_id, ".versions");
    fs::rename(versions_src, versions_dest, ec);
    if (ec) {
      return FilesystemStatus(absl::StrCat("failed to restore versions dir to ",
                                           versions_dest.string()),
                              ec);
    }
  }

  fs::remove(tombstone_dir, ec);
  return absl::OkStatus();
}

}  // namespace internal
}  // namespace duckdb
}  // namespace storage
}  // namespace backend
}  // namespace bigquery_emulator
