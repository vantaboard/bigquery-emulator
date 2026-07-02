#include <algorithm>
#include <cstdint>
#include <filesystem>
#include <string>
#include <system_error>
#include <utility>
#include <vector>

#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "absl/strings/match.h"
#include "absl/strings/numbers.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/str_join.h"
#include "absl/time/clock.h"
#include "absl/time/time.h"
#include "backend/storage/duckdb/duckdb_storage_internal.h"
#include "backend/storage/duckdb/duckdb_storage_version_log.h"
#include "backend/storage/storage.h"

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

absl::StatusOr<std::int64_t> LatestDatasetTombstoneMs(
    const DuckDBStorage& storage, const DatasetId& id) {
  const fs::path root = fs::path(storage.data_dir()) / ".tombstones" /
                        "__dataset__" / DatasetTombstoneKey(id);
  std::error_code ec;
  if (!fs::exists(root, ec)) {
    return absl::NotFoundError(absl::StrCat(
        "no tombstones for dataset ", id.project_id, ".", id.dataset_id));
  }
  const std::int64_t now_ms = absl::ToUnixMillis(absl::Now());
  const std::int64_t cutoff = now_ms - kTimeTravelWindowMs;
  std::int64_t best = 0;
  bool found = false;
  for (const auto& entry : fs::directory_iterator(root, ec)) {
    if (ec) break;
    if (!entry.is_directory()) continue;
    std::int64_t deleted_ms = 0;
    if (!absl::SimpleAtoi(entry.path().filename().string(), &deleted_ms)) {
      continue;
    }
    if (deleted_ms < cutoff) continue;
    if (!found || deleted_ms > best) {
      best = deleted_ms;
      found = true;
    }
  }
  if (!found) {
    return absl::NotFoundError(absl::StrCat(
        "no tombstones for dataset ", id.project_id, ".", id.dataset_id));
  }
  return best;
}

absl::Status TombstoneRetentionOk(std::int64_t deleted_ms) {
  const std::int64_t now_ms = absl::ToUnixMillis(absl::Now());
  if (deleted_ms + kTimeTravelWindowMs < now_ms) {
    return absl::NotFoundError("tombstone is past the retention window");
  }
  return absl::OkStatus();
}

absl::StatusOr<std::string> ParseJsonStringField(absl::string_view json,
                                                 absl::string_view field) {
  const std::string needle = absl::StrCat("\"", field, "\":\"");
  const size_t pos = json.find(needle);
  if (pos == absl::string_view::npos) {
    return absl::InvalidArgumentError(
        absl::StrCat("json sidecar missing field ", field));
  }
  absl::string_view rest = json.substr(pos + needle.size());
  std::string out;
  for (size_t i = 0; i < rest.size(); ++i) {
    if (rest[i] == '\\' && i + 1 < rest.size()) {
      out.push_back(rest[i + 1]);
      ++i;
      continue;
    }
    if (rest[i] == '"') break;
    out.push_back(rest[i]);
  }
  return out;
}

std::string RenderTablesManifestJson(
    const std::vector<std::string>& table_ids) {
  std::vector<std::string> quoted;
  quoted.reserve(table_ids.size());
  for (const std::string& id : table_ids) {
    quoted.push_back(absl::StrCat("\"", id, "\""));
  }
  return absl::StrCat("[", absl::StrJoin(quoted, ","), "]");
}

absl::StatusOr<std::vector<std::string>> ParseTablesManifestJson(
    absl::string_view json) {
  std::vector<std::string> out;
  size_t pos = 0;
  while ((pos = json.find('"', pos)) != absl::string_view::npos) {
    ++pos;
    std::string id;
    for (; pos < json.size(); ++pos) {
      if (json[pos] == '\\' && pos + 1 < json.size()) {
        id.push_back(json[pos + 1]);
        ++pos;
        continue;
      }
      if (json[pos] == '"') break;
      id.push_back(json[pos]);
    }
    if (!id.empty()) {
      out.push_back(std::move(id));
    }
    ++pos;
  }
  return out;
}

std::vector<std::string> ListTableIdsInDatasetDir(const fs::path& ds_dir) {
  std::vector<std::string> out;
  std::error_code ec;
  for (const auto& entry : fs::directory_iterator(ds_dir, ec)) {
    if (ec) break;
    const std::string name = entry.path().filename().string();
    if (name == kDatasetMetaFile) continue;
    if (name.size() <= kTableMetaSuffix.size()) continue;
    if (!absl::EndsWith(name, kTableMetaSuffix)) continue;
    out.push_back(name.substr(0, name.size() - kTableMetaSuffix.size()));
  }
  std::sort(out.begin(), out.end());
  return out;
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

absl::Status MoveDatasetToTombstone(const DuckDBStorage& storage,
                                    const DatasetId& id,
                                    std::int64_t deleted_ms) {
  const fs::path ds_dir =
      fs::path(storage.data_dir()) / id.project_id / id.dataset_id;
  std::error_code ec;
  if (!fs::exists(ds_dir, ec)) {
    return absl::NotFoundError(
        absl::StrCat("dataset not found: ", id.project_id, ".", id.dataset_id));
  }

  const fs::path dataset_meta = ds_dir / std::string(kDatasetMetaFile);
  if (!fs::exists(dataset_meta, ec)) {
    return absl::NotFoundError(
        absl::StrCat("dataset not found: ", id.project_id, ".", id.dataset_id));
  }

  const std::string tombstone_dir =
      DatasetTombstoneDir(storage, id, deleted_ms);
  fs::create_directories(tombstone_dir, ec);
  if (ec) {
    return FilesystemStatus(
        absl::StrCat("failed to create dataset tombstone dir: ", tombstone_dir),
        ec);
  }

  auto meta_or = ReadFile(dataset_meta);
  if (!meta_or.ok()) return meta_or.status();
  const auto write_meta = WriteFileAtomic(
      fs::path(tombstone_dir) / std::string(kDatasetMetaFile), *meta_or);
  if (!write_meta.ok()) return write_meta;

  const std::vector<std::string> table_ids = ListTableIdsInDatasetDir(ds_dir);
  const auto write_manifest =
      WriteFileAtomic(fs::path(tombstone_dir) / "tables.json",
                      RenderTablesManifestJson(table_ids));
  if (!write_manifest.ok()) return write_manifest;

  for (const std::string& table_id : table_ids) {
    const TableId tid{id.project_id, id.dataset_id, table_id};
    absl::Status moved = MoveTableToTombstone(storage, tid, deleted_ms);
    if (!moved.ok()) return moved;
  }

  fs::remove_all(ds_dir, ec);
  if (ec) {
    return FilesystemStatus(
        absl::StrCat("failed to remove dataset dir: ", ds_dir.string()), ec);
  }
  return absl::OkStatus();
}

absl::Status RestoreDatasetFromTombstone(const DuckDBStorage& storage,
                                         const DatasetId& id,
                                         std::int64_t deleted_ms) {
  const fs::path live_dir =
      fs::path(storage.data_dir()) / id.project_id / id.dataset_id;
  std::error_code ec;
  const fs::path live_meta = live_dir / std::string(kDatasetMetaFile);
  if (fs::exists(live_meta, ec)) {
    return absl::AlreadyExistsError(absl::StrCat(
        "dataset already exists: ", id.project_id, ".", id.dataset_id));
  }

  std::int64_t chosen = deleted_ms;
  if (chosen == 0) {
    auto latest_or = LatestDatasetTombstoneMs(storage, id);
    if (!latest_or.ok()) return latest_or.status();
    chosen = *latest_or;
  }

  absl::Status retention = TombstoneRetentionOk(chosen);
  if (!retention.ok()) return retention;

  const std::string tombstone_dir = DatasetTombstoneDir(storage, id, chosen);
  if (!fs::exists(tombstone_dir, ec)) {
    return absl::NotFoundError(absl::StrCat("tombstone not found for dataset ",
                                            id.project_id,
                                            ".",
                                            id.dataset_id,
                                            " at ",
                                            chosen));
  }

  const fs::path tomb_meta =
      fs::path(tombstone_dir) / std::string(kDatasetMetaFile);
  auto meta_or = ReadFile(tomb_meta);
  if (!meta_or.ok()) return meta_or.status();

  auto manifest_or = ReadFile(fs::path(tombstone_dir) / "tables.json");
  if (!manifest_or.ok()) return manifest_or.status();
  auto table_ids_or = ParseTablesManifestJson(*manifest_or);
  if (!table_ids_or.ok()) return table_ids_or.status();

  fs::create_directories(live_dir, ec);
  if (ec) {
    return FilesystemStatus(
        absl::StrCat("failed to create dataset dir: ", live_dir.string()), ec);
  }
  absl::Status write_meta = WriteFileAtomic(live_meta, *meta_or);
  if (!write_meta.ok()) return write_meta;

  for (const std::string& table_id : *table_ids_or) {
    const TableId tid{id.project_id, id.dataset_id, table_id};
    absl::Status restored = RestoreTableFromTombstone(storage, tid, chosen);
    if (!restored.ok()) return restored;
  }

  fs::remove_all(tombstone_dir, ec);
  if (ec) {
    return FilesystemStatus(
        absl::StrCat("failed to remove dataset tombstone dir: ", tombstone_dir),
        ec);
  }
  return absl::OkStatus();
}

}  // namespace internal
}  // namespace duckdb
}  // namespace storage
}  // namespace backend
}  // namespace bigquery_emulator
