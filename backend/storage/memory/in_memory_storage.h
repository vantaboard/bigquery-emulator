#ifndef BIGQUERY_EMULATOR_BACKEND_STORAGE_MEMORY_IN_MEMORY_STORAGE_H_
#define BIGQUERY_EMULATOR_BACKEND_STORAGE_MEMORY_IN_MEMORY_STORAGE_H_

// InMemoryStorage is the volatile, CI-friendly `Storage` implementation:
// a nested dataset → table → rows map guarded by a single absl::Mutex.
//
// It is the default profile (`--storage=memory`, see ROADMAP "Pluggable
// engine and storage"). The DuckDB-backed persistent store lands later
// alongside the DuckDB engine.
//
// Concurrency model: every public method acquires the single mutex for
// its duration. Reads do not block other reads in the abstract sense
// of the `Storage` contract (which only requires thread-safety, not
// reader-writer parallelism), but this impl is intentionally simple
// and serializes everything. `ScanRows` materializes the row snapshot
// under the lock and hands the caller an iterator that owns a private
// copy, so the lock is released before the caller starts reading.

#include <cstddef>
#include <map>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "absl/base/thread_annotations.h"
#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "absl/strings/string_view.h"
#include "absl/synchronization/mutex.h"
#include "absl/types/span.h"
#include "backend/schema/schema.h"
#include "backend/storage/storage.h"

namespace bigquery_emulator {
namespace backend {
namespace storage {
namespace memory {

class InMemoryStorage : public Storage {
 public:
  InMemoryStorage() = default;
  ~InMemoryStorage() override = default;

  InMemoryStorage(const InMemoryStorage&) = delete;
  InMemoryStorage& operator=(const InMemoryStorage&) = delete;

  absl::Status CreateDataset(const DatasetId& id,
                              absl::string_view location) override;
  absl::Status DropDataset(const DatasetId& id,
                            bool delete_contents) override;

  absl::Status CreateTable(const TableId& id,
                            const schema::TableSchema& schema) override;
  absl::Status DropTable(const TableId& id) override;

  absl::StatusOr<schema::TableSchema> GetSchema(
      const TableId& id) const override;

  absl::Status AppendRows(const TableId& id,
                           absl::Span<const Row> rows) override;

  absl::Status OverwriteRows(const TableId& id,
                              absl::Span<const Row> rows) override;

  absl::StatusOr<std::unique_ptr<RowIterator>> ScanRows(
      const TableId& id) const override;

 private:
  struct TableState {
    schema::TableSchema schema;
    std::vector<Row> rows;
  };

  struct DatasetState {
    std::string location;
    // Keyed by `table_id`; the project+dataset are implicit in the
    // owning DatasetState.
    std::map<std::string, TableState> tables;
  };

  // Composite key for the dataset map: "<project_id>\x1f<dataset_id>".
  // We use the ASCII Unit Separator so the key is unambiguous even
  // when project / dataset ids contain other punctuation. BigQuery
  // disallows control characters in either id (see
  // docs/bigquery/docs/datasets-intro.md) so collisions are impossible.
  static std::string DatasetKey(absl::string_view project_id,
                                 absl::string_view dataset_id);
  static std::string DatasetKey(const DatasetId& id);
  static std::string DatasetKey(const TableId& id);

  mutable absl::Mutex mu_;
  std::map<std::string, DatasetState> datasets_ ABSL_GUARDED_BY(mu_);
};

}  // namespace memory
}  // namespace storage
}  // namespace backend
}  // namespace bigquery_emulator

#endif  // BIGQUERY_EMULATOR_BACKEND_STORAGE_MEMORY_IN_MEMORY_STORAGE_H_
