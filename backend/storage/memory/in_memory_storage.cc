#include "backend/storage/memory/in_memory_storage.h"

#include <cstddef>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/string_view.h"
#include "absl/synchronization/mutex.h"
#include "absl/types/span.h"
#include "backend/schema/schema.h"
#include "backend/storage/storage.h"

namespace bigquery_emulator {
namespace backend {
namespace storage {
namespace memory {

namespace {

// Iterator implementation that owns a private copy of the row vector
// captured at `ScanRows` time. The owning storage's lock is released
// before the caller starts iterating, so the snapshot is necessarily
// stable for the iterator's lifetime.
class VectorRowIterator : public RowIterator {
 public:
  explicit VectorRowIterator(std::vector<Row> rows)
      : rows_(std::move(rows)), pos_(0) {}

  absl::StatusOr<bool> Next(Row* row) override {
    if (pos_ >= rows_.size()) return false;
    *row = rows_[pos_++];
    return true;
  }

 private:
  std::vector<Row> rows_;
  size_t pos_;
};

// Total column count for a flat row check. ARRAY / STRUCT cells count
// as one cell from the perspective of `AppendRows`; the inner shape is
// validated by the engine, not the store.
size_t TopLevelColumnCount(const schema::TableSchema& schema) {
  return schema.columns.size();
}

}  // namespace

std::string InMemoryStorage::DatasetKey(absl::string_view project_id,
                                          absl::string_view dataset_id) {
  return absl::StrCat(project_id, "\x1f", dataset_id);
}

std::string InMemoryStorage::DatasetKey(const DatasetId& id) {
  return DatasetKey(id.project_id, id.dataset_id);
}

std::string InMemoryStorage::DatasetKey(const TableId& id) {
  return DatasetKey(id.project_id, id.dataset_id);
}

absl::Status InMemoryStorage::CreateDataset(const DatasetId& id,
                                              absl::string_view location) {
  if (id.project_id.empty() || id.dataset_id.empty()) {
    return absl::InvalidArgumentError(
        "CreateDataset: project_id and dataset_id must be non-empty");
  }
  const std::string key = DatasetKey(id);
  absl::MutexLock lock(&mu_);
  if (datasets_.find(key) != datasets_.end()) {
    return absl::AlreadyExistsError(
        absl::StrCat("dataset already exists: ", id.project_id, ".",
                     id.dataset_id));
  }
  DatasetState state;
  state.location = std::string(location);
  datasets_.emplace(key, std::move(state));
  return absl::OkStatus();
}

absl::Status InMemoryStorage::DropDataset(const DatasetId& id,
                                            bool delete_contents) {
  const std::string key = DatasetKey(id);
  absl::MutexLock lock(&mu_);
  auto it = datasets_.find(key);
  if (it == datasets_.end()) {
    return absl::NotFoundError(absl::StrCat("dataset not found: ",
                                             id.project_id, ".",
                                             id.dataset_id));
  }
  if (!delete_contents && !it->second.tables.empty()) {
    return absl::FailedPreconditionError(
        absl::StrCat("dataset is not empty: ", id.project_id, ".",
                     id.dataset_id,
                     " (use delete_contents=true to drop with tables)"));
  }
  datasets_.erase(it);
  return absl::OkStatus();
}

absl::Status InMemoryStorage::CreateTable(
    const TableId& id, const schema::TableSchema& schema) {
  if (id.table_id.empty()) {
    return absl::InvalidArgumentError(
        "CreateTable: table_id must be non-empty");
  }
  const std::string key = DatasetKey(id);
  absl::MutexLock lock(&mu_);
  auto ds_it = datasets_.find(key);
  if (ds_it == datasets_.end()) {
    return absl::NotFoundError(absl::StrCat("dataset not found: ",
                                             id.project_id, ".",
                                             id.dataset_id));
  }
  auto& tables = ds_it->second.tables;
  if (tables.find(id.table_id) != tables.end()) {
    return absl::AlreadyExistsError(
        absl::StrCat("table already exists: ", id.project_id, ".",
                     id.dataset_id, ".", id.table_id));
  }
  TableState state;
  state.schema = schema;
  tables.emplace(id.table_id, std::move(state));
  return absl::OkStatus();
}

absl::Status InMemoryStorage::DropTable(const TableId& id) {
  const std::string key = DatasetKey(id);
  absl::MutexLock lock(&mu_);
  auto ds_it = datasets_.find(key);
  if (ds_it == datasets_.end()) {
    return absl::NotFoundError(absl::StrCat("dataset not found: ",
                                             id.project_id, ".",
                                             id.dataset_id));
  }
  auto& tables = ds_it->second.tables;
  auto t_it = tables.find(id.table_id);
  if (t_it == tables.end()) {
    return absl::NotFoundError(absl::StrCat("table not found: ",
                                             id.project_id, ".",
                                             id.dataset_id, ".",
                                             id.table_id));
  }
  tables.erase(t_it);
  return absl::OkStatus();
}

absl::StatusOr<schema::TableSchema> InMemoryStorage::GetSchema(
    const TableId& id) const {
  const std::string key = DatasetKey(id);
  absl::MutexLock lock(&mu_);
  auto ds_it = datasets_.find(key);
  if (ds_it == datasets_.end()) {
    return absl::NotFoundError(absl::StrCat("dataset not found: ",
                                             id.project_id, ".",
                                             id.dataset_id));
  }
  const auto& tables = ds_it->second.tables;
  auto t_it = tables.find(id.table_id);
  if (t_it == tables.end()) {
    return absl::NotFoundError(absl::StrCat("table not found: ",
                                             id.project_id, ".",
                                             id.dataset_id, ".",
                                             id.table_id));
  }
  return t_it->second.schema;
}

absl::Status InMemoryStorage::AppendRows(const TableId& id,
                                          absl::Span<const Row> rows) {
  const std::string key = DatasetKey(id);
  absl::MutexLock lock(&mu_);
  auto ds_it = datasets_.find(key);
  if (ds_it == datasets_.end()) {
    return absl::NotFoundError(absl::StrCat("dataset not found: ",
                                             id.project_id, ".",
                                             id.dataset_id));
  }
  auto& tables = ds_it->second.tables;
  auto t_it = tables.find(id.table_id);
  if (t_it == tables.end()) {
    return absl::NotFoundError(absl::StrCat("table not found: ",
                                             id.project_id, ".",
                                             id.dataset_id, ".",
                                             id.table_id));
  }
  TableState& state = t_it->second;
  const size_t expected = TopLevelColumnCount(state.schema);
  // Cheap shape check: every row must have exactly one cell per
  // top-level column. The proto / engine layer is responsible for
  // type-checking each cell; the store just refuses obviously
  // misshapen batches so callers don't silently corrupt the table.
  for (size_t i = 0; i < rows.size(); ++i) {
    if (rows[i].cells.size() != expected) {
      return absl::InvalidArgumentError(
          absl::StrCat("AppendRows: row[", i, "] has ", rows[i].cells.size(),
                       " cell(s) but table ", id.project_id, ".",
                       id.dataset_id, ".", id.table_id, " has ",
                       expected, " column(s)"));
    }
  }
  state.rows.reserve(state.rows.size() + rows.size());
  for (const auto& row : rows) {
    state.rows.push_back(row);
  }
  return absl::OkStatus();
}

absl::StatusOr<std::unique_ptr<RowIterator>> InMemoryStorage::ScanRows(
    const TableId& id) const {
  const std::string key = DatasetKey(id);
  std::vector<Row> snapshot;
  {
    absl::MutexLock lock(&mu_);
    auto ds_it = datasets_.find(key);
    if (ds_it == datasets_.end()) {
      return absl::NotFoundError(absl::StrCat("dataset not found: ",
                                               id.project_id, ".",
                                               id.dataset_id));
    }
    const auto& tables = ds_it->second.tables;
    auto t_it = tables.find(id.table_id);
    if (t_it == tables.end()) {
      return absl::NotFoundError(absl::StrCat("table not found: ",
                                               id.project_id, ".",
                                               id.dataset_id, ".",
                                               id.table_id));
    }
    snapshot = t_it->second.rows;
  }
  return std::unique_ptr<RowIterator>(
      new VectorRowIterator(std::move(snapshot)));
}

}  // namespace memory
}  // namespace storage
}  // namespace backend
}  // namespace bigquery_emulator
