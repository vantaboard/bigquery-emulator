#include "backend/storage/duckdb/duckdb_storage_version_log.h"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <string>
#include <system_error>
#include <utility>
#include <vector>

#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "absl/strings/numbers.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/str_format.h"
#include "absl/strings/str_join.h"
#include "absl/strings/string_view.h"
#include "absl/time/clock.h"
#include "absl/time/time.h"
#include "backend/storage/duckdb/duckdb_storage.h"
#include "backend/storage/duckdb/duckdb_storage_internal.h"
#include "backend/storage/storage.h"

namespace bigquery_emulator {
namespace backend {
namespace storage {
namespace duckdb {
namespace internal {

namespace fs = std::filesystem;

std::string DatasetDirFromStorage(const DuckDBStorage& storage,
                                  const TableId& id) {
  return (fs::path(storage.data_dir()) / id.project_id / id.dataset_id)
      .string();
}

std::string TableMetaPathFromStorage(const DuckDBStorage& storage,
                                     const TableId& id) {
  return (fs::path(DatasetDirFromStorage(storage, id)) /
          absl::StrCat(id.table_id, kTableMetaSuffix))
      .string();
}

std::string TableParquetPathFromStorage(const DuckDBStorage& storage,
                                        const TableId& id) {
  return (fs::path(DatasetDirFromStorage(storage, id)) /
          absl::StrCat(id.table_id, ".parquet"))
      .string();
}

namespace {

std::string EscapeJsonString(absl::string_view s) {
  std::string out;
  out.reserve(s.size() + 8);
  for (char c : s) {
    switch (c) {
      case '\\':
        out += "\\\\";
        break;
      case '"':
        out += "\\\"";
        break;
      default:
        out.push_back(c);
        break;
    }
  }
  return out;
}

absl::StatusOr<std::int64_t> ParseJsonInt64Field(absl::string_view json,
                                                 absl::string_view field) {
  const std::string needle = absl::StrCat("\"", field, "\":");
  const size_t pos = json.find(needle);
  if (pos == absl::string_view::npos) {
    return absl::InvalidArgumentError(
        absl::StrCat("version index missing field ", field));
  }
  absl::string_view rest = json.substr(pos + needle.size());
  while (!rest.empty() && (rest[0] == ' ' || rest[0] == '\t')) {
    rest.remove_prefix(1);
  }
  size_t end = 0;
  while (end < rest.size() &&
         (rest[end] == '-' || (rest[end] >= '0' && rest[end] <= '9'))) {
    ++end;
  }
  if (end == 0) {
    return absl::InvalidArgumentError(
        absl::StrCat("version index field ", field, " is not an integer"));
  }
  std::int64_t value = 0;
  if (!absl::SimpleAtoi(rest.substr(0, end), &value)) {
    return absl::InvalidArgumentError(
        absl::StrCat("version index field ", field, " overflow"));
  }
  return value;
}

absl::StatusOr<std::string> ParseJsonStringField(absl::string_view json,
                                                 absl::string_view field) {
  const std::string needle = absl::StrCat("\"", field, "\":\"");
  const size_t pos = json.find(needle);
  if (pos == absl::string_view::npos) {
    return absl::InvalidArgumentError(
        absl::StrCat("version entry missing field ", field));
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

bool ParseVersionEntryFile(absl::string_view json,
                           size_t pos,
                           VersionEntry* entry) {
  const size_t file_key = json.find("\"file\":\"", pos);
  if (file_key == absl::string_view::npos) {
    return false;
  }
  absl::string_view file_rest = json.substr(file_key + 8);
  for (size_t i = 0; i < file_rest.size(); ++i) {
    if (file_rest[i] == '\\' && i + 1 < file_rest.size()) {
      entry->file.push_back(file_rest[i + 1]);
      ++i;
      continue;
    }
    if (file_rest[i] == '"') break;
    entry->file.push_back(file_rest[i]);
  }
  return !entry->file.empty();
}

bool ParseVersionEntryAt(absl::string_view json,
                         size_t pos,
                         VersionEntry* entry) {
  const std::string key = "\"valid_from_ms\":";
  absl::string_view rest = json.substr(pos + key.size());
  size_t end = 0;
  while (end < rest.size() &&
         (rest[end] == '-' || (rest[end] >= '0' && rest[end] <= '9'))) {
    ++end;
  }
  if (end > 0) {
    if (!absl::SimpleAtoi(rest.substr(0, end), &entry->valid_from_ms)) {
      entry->valid_from_ms = 0;
    }
  }
  return ParseVersionEntryFile(json, pos, entry);
}

std::vector<VersionEntry> ParseVersionEntries(absl::string_view json) {
  std::vector<VersionEntry> out;
  const std::string key = "\"valid_from_ms\":";
  size_t pos = 0;
  while ((pos = json.find(key, pos)) != absl::string_view::npos) {
    VersionEntry entry;
    if (ParseVersionEntryAt(json, pos, &entry)) {
      out.push_back(std::move(entry));
    }
    pos += key.size();
  }
  std::sort(
      out.begin(), out.end(), [](const VersionEntry& a, const VersionEntry& b) {
        return a.valid_from_ms < b.valid_from_ms;
      });
  return out;
}

std::string RenderVersionIndexJson(const VersionIndex& index) {
  std::vector<std::string> version_objs;
  version_objs.reserve(index.versions.size());
  for (const VersionEntry& v : index.versions) {
    version_objs.push_back(
        absl::StrFormat(R"({"valid_from_ms":%d,"file":"%s"})",
                        v.valid_from_ms,
                        EscapeJsonString(v.file)));
  }
  return absl::StrFormat(
      R"({"created_ts_ms":%d,"current_ts_ms":%d,"versions":[%s]})",
      index.created_ts_ms,
      index.current_ts_ms,
      absl::StrJoin(version_objs, ","));
}

absl::StatusOr<VersionIndex> LoadVersionIndexOrInit(
    const DuckDBStorage& storage, const TableId& id) {
  const std::string index_path = TableVersionsIndexPath(storage, id);
  std::error_code ec;
  if (!fs::exists(index_path, ec)) {
    const std::int64_t now_ms = absl::ToUnixMillis(absl::Now());
    VersionIndex index{.created_ts_ms = now_ms, .current_ts_ms = now_ms};
    return index;
  }
  auto contents_or = ReadFile(index_path);
  if (!contents_or.ok()) return contents_or.status();
  VersionIndex index;
  auto created_or = ParseJsonInt64Field(*contents_or, "created_ts_ms");
  if (!created_or.ok()) return created_or.status();
  auto current_or = ParseJsonInt64Field(*contents_or, "current_ts_ms");
  if (!current_or.ok()) return current_or.status();
  index.created_ts_ms = *created_or;
  index.current_ts_ms = *current_or;
  index.versions = ParseVersionEntries(*contents_or);
  return index;
}

std::int64_t EarliestReadableMs(const VersionIndex& index) {
  std::int64_t earliest = index.created_ts_ms;
  if (!index.versions.empty()) {
    earliest = std::min(earliest, index.versions.front().valid_from_ms);
  }
  return earliest;
}

void PruneOldVersions(VersionIndex* index,
                      const DuckDBStorage& storage,
                      const TableId& id,
                      std::int64_t now_ms) {
  const std::int64_t cutoff = now_ms - kTimeTravelWindowMs;
  const std::string versions_dir = TableVersionsDir(storage, id);
  std::vector<VersionEntry> kept;
  kept.reserve(index->versions.size());
  for (const VersionEntry& v : index->versions) {
    if (v.valid_from_ms < cutoff) {
      std::error_code ec;
      fs::remove((fs::path(versions_dir) / v.file).string(), ec);
      continue;
    }
    kept.push_back(v);
  }
  index->versions = std::move(kept);
  if (index->created_ts_ms < cutoff) {
    index->created_ts_ms = cutoff;
  }
}

absl::Status InvalidSnapshotTimeError(const TableId& id,
                                      std::int64_t as_of_ms,
                                      std::int64_t earliest_ms) {
  return absl::InvalidArgumentError(absl::StrFormat(
      "Invalid snapshot time %d for table %s:%s.%s@%d. Cannot read before "
      "%d.",
      as_of_ms,
      id.project_id,
      id.dataset_id,
      id.table_id,
      as_of_ms,
      earliest_ms));
}

}  // namespace

std::string TableVersionsDir(const DuckDBStorage& storage, const TableId& id) {
  return (fs::path(DatasetDirFromStorage(storage, id)) /
          absl::StrCat(id.table_id, ".versions"))
      .string();
}

std::string TableVersionsIndexPath(const DuckDBStorage& storage,
                                   const TableId& id) {
  return (fs::path(DatasetDirFromStorage(storage, id)) /
          absl::StrCat(id.table_id, ".versions.json"))
      .string();
}

std::string TableTombstoneDir(const DuckDBStorage& storage,
                              const TableId& id,
                              std::int64_t deleted_ms) {
  return (fs::path(storage.data_dir()) / ".tombstones" /
          DatasetTombstoneKey(DatasetId{id.project_id, id.dataset_id}) /
          id.table_id / absl::StrCat(deleted_ms))
      .string();
}

std::string DatasetTombstoneKey(const DatasetId& id) {
  return absl::StrCat(id.project_id, ".", id.dataset_id);
}

std::string DatasetTombstoneDir(const DuckDBStorage& storage,
                                const DatasetId& id,
                                std::int64_t deleted_ms) {
  return (fs::path(storage.data_dir()) / ".tombstones" / "__dataset__" /
          DatasetTombstoneKey(id) / absl::StrCat(deleted_ms))
      .string();
}

absl::StatusOr<VersionIndex> ReadVersionIndex(absl::string_view index_path,
                                              bool allow_missing) {
  std::error_code ec;
  if (!fs::exists(index_path, ec)) {
    if (allow_missing) {
      return VersionIndex{};
    }
    return absl::NotFoundError(
        absl::StrCat("version index not found: ", index_path));
  }
  auto contents_or = ReadFile(fs::path(std::string(index_path)));
  if (!contents_or.ok()) return contents_or.status();
  VersionIndex index;
  auto created_or = ParseJsonInt64Field(*contents_or, "created_ts_ms");
  if (!created_or.ok()) return created_or.status();
  auto current_or = ParseJsonInt64Field(*contents_or, "current_ts_ms");
  if (!current_or.ok()) return current_or.status();
  index.created_ts_ms = *created_or;
  index.current_ts_ms = *current_or;
  index.versions = ParseVersionEntries(*contents_or);
  return index;
}

absl::Status WriteVersionIndex(absl::string_view index_path,
                               const VersionIndex& index) {
  return WriteFileAtomic(fs::path(std::string(index_path)),
                         RenderVersionIndexJson(index));
}

absl::Status InitVersionIndex(const DuckDBStorage& storage,
                              const TableId& id,
                              std::int64_t created_ts_ms) {
  VersionIndex index{.created_ts_ms = created_ts_ms,
                     .current_ts_ms = created_ts_ms};
  return WriteVersionIndex(TableVersionsIndexPath(storage, id), index);
}

absl::Status ArchiveParquetBeforeReplace(const DuckDBStorage& storage,
                                         const TableId& id,
                                         absl::string_view live_parquet_path,
                                         std::int64_t mutation_ts_ms) {
  std::error_code ec;
  if (!fs::exists(live_parquet_path, ec)) {
    auto index_or = LoadVersionIndexOrInit(storage, id);
    if (!index_or.ok()) return index_or.status();
    VersionIndex index = *std::move(index_or);
    index.current_ts_ms = mutation_ts_ms;
    PruneOldVersions(&index, storage, id, mutation_ts_ms);
    return WriteVersionIndex(TableVersionsIndexPath(storage, id), index);
  }

  auto index_or = LoadVersionIndexOrInit(storage, id);
  if (!index_or.ok()) return index_or.status();
  VersionIndex index = *std::move(index_or);

  const std::string versions_dir = TableVersionsDir(storage, id);
  fs::create_directories(versions_dir, ec);
  if (ec) {
    return FilesystemStatus(
        absl::StrCat("failed to create versions dir: ", versions_dir), ec);
  }

  const std::string archive_name =
      absl::StrCat(index.current_ts_ms, ".parquet");
  const fs::path archive_path = fs::path(versions_dir) / archive_name;
  fs::copy_file(live_parquet_path,
                archive_path,
                fs::copy_options::overwrite_existing,
                ec);
  if (ec) {
    return FilesystemStatus(
        absl::StrCat("failed to archive parquet to ", archive_path.string()),
        ec);
  }

  index.versions.push_back(
      VersionEntry{.valid_from_ms = index.current_ts_ms, .file = archive_name});
  std::sort(index.versions.begin(),
            index.versions.end(),
            [](const VersionEntry& a, const VersionEntry& b) {
              return a.valid_from_ms < b.valid_from_ms;
            });
  index.current_ts_ms = mutation_ts_ms;
  PruneOldVersions(&index, storage, id, mutation_ts_ms);
  return WriteVersionIndex(TableVersionsIndexPath(storage, id), index);
}

absl::StatusOr<std::string> ResolveParquetSnapshotAt(
    const DuckDBStorage& storage,
    const TableId& id,
    absl::string_view live_parquet_path,
    std::int64_t as_of_ms,
    std::int64_t now_ms) {
  if (as_of_ms > now_ms) {
    auto index_or = LoadVersionIndexOrInit(storage, id);
    if (!index_or.ok()) return index_or.status();
    return InvalidSnapshotTimeError(id, as_of_ms, index_or->created_ts_ms);
  }
  if (as_of_ms < now_ms - kTimeTravelWindowMs) {
    auto index_or = LoadVersionIndexOrInit(storage, id);
    if (!index_or.ok()) return index_or.status();
    const std::int64_t earliest =
        std::max(EarliestReadableMs(*index_or), now_ms - kTimeTravelWindowMs);
    return InvalidSnapshotTimeError(id, as_of_ms, earliest);
  }

  auto index_or = LoadVersionIndexOrInit(storage, id);
  if (!index_or.ok()) return index_or.status();
  const VersionIndex& index = *index_or;

  const std::string index_path = TableVersionsIndexPath(storage, id);
  std::error_code index_ec;
  const bool index_on_disk = fs::exists(index_path, index_ec);

  // No version history yet: client decorators from the same session may
  // predate the first persisted index timestamp; serve live data.
  if (!index_on_disk && index.versions.empty() && as_of_ms <= now_ms) {
    std::error_code live_ec;
    if (fs::exists(live_parquet_path, live_ec)) {
      return std::string(live_parquet_path);
    }
  }

  if (as_of_ms < index.created_ts_ms) {
    return InvalidSnapshotTimeError(id, as_of_ms, index.created_ts_ms);
  }

  if (as_of_ms >= index.current_ts_ms) {
    std::error_code ec;
    if (!fs::exists(live_parquet_path, ec)) {
      return absl::NotFoundError(absl::StrCat("table parquet missing: ",
                                              id.project_id,
                                              ".",
                                              id.dataset_id,
                                              ".",
                                              id.table_id));
    }
    return std::string(live_parquet_path);
  }

  const VersionEntry* best = nullptr;
  for (const VersionEntry& v : index.versions) {
    if (v.valid_from_ms <= as_of_ms) {
      best = &v;
    }
  }
  if (best == nullptr) {
    return InvalidSnapshotTimeError(id, as_of_ms, index.created_ts_ms);
  }
  const std::string path =
      (fs::path(TableVersionsDir(storage, id)) / best->file).string();
  std::error_code ec;
  if (!fs::exists(path, ec)) {
    return InvalidSnapshotTimeError(id, as_of_ms, index.created_ts_ms);
  }
  return path;
}

}  // namespace internal
}  // namespace duckdb
}  // namespace storage
}  // namespace backend
}  // namespace bigquery_emulator
