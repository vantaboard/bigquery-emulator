#ifndef BIGQUERY_EMULATOR_BACKEND_ENGINE_SEMANTIC_ROW_SOURCE_H_
#define BIGQUERY_EMULATOR_BACKEND_ENGINE_SEMANTIC_ROW_SOURCE_H_

// Compositional `RowSource` adapters used by the semantic executor.
//
// Design constraint #2 from
// `.cursor/plans/semantic-executor-core.plan.md` is that the
// semantic executor REUSES `DuckDbExecutor` for shapes the
// fast-path covers (table scans, joins, aggregations); it never
// opens a new DuckDB connection of its own. The
// `MaterializedRowSource` here is the composition primitive
// downstream plans use to:
//
//   * stream rows produced by a `DuckDbExecutor`-backed `RowSource`
//     through a semantic-executor projection (e.g. a SELECT whose
//     FROM is a fast-path scan but whose projection list contains
//     a SAFE_DIVIDE that must surface BigQuery-exact errors);
//   * inject synthetic Arrow batches into unit / integration tests
//     so the streaming contract can be exercised without spinning
//     up a real DuckDB query.
//
// For the basic scalar-only SELECT path shipped in this plan, the
// executor never reaches for the adapter -- the value table /
// FROM-clause shapes that do are owned by
// `array-struct-semantic-path.plan.md` and
// `cte-subquery-routing.plan.md`. The adapter ships here so those
// plans have a stable interface to consume.

#include <memory>
#include <utility>
#include <vector>

#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "backend/engine/engine.h"
#include "backend/schema/schema.h"
#include "backend/storage/storage.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace semantic {

// `MaterializedRowSource` is the simplest `RowSource` adapter: it
// owns a pre-built vector of `storage::Row` plus a `TableSchema`
// and streams them one-at-a-time on `Next`. Downstream plans use
// it to wrap a drained `DuckDbExecutor` result, but it is also the
// primitive the semantic executor's scalar-only SELECT path uses
// internally (single-row case).
class MaterializedRowSource : public RowSource {
 public:
  MaterializedRowSource(schema::TableSchema schema,
                        std::vector<storage::Row> rows)
      : schema_(std::move(schema)), rows_(std::move(rows)) {}

  const schema::TableSchema& schema() const override {
    return schema_;
  }

  absl::StatusOr<bool> Next(storage::Row* row) override {
    if (row == nullptr) {
      return absl::InvalidArgumentError(
          "MaterializedRowSource::Next called with null row");
    }
    if (cursor_ >= rows_.size()) return false;
    *row = rows_[cursor_++];
    return true;
  }

 private:
  schema::TableSchema schema_;
  std::vector<storage::Row> rows_;
  size_t cursor_ = 0;
};

// `DrainRowSource` reads every row out of `source` (a producer the
// caller owns) into a `MaterializedRowSource`. Used by downstream
// plans to wrap a `DuckDbExecutor`-backed result and stream it
// through the semantic executor.
//
// Drains eagerly so the upstream's lifetime is decoupled from the
// returned adapter's; the upstream is closed when the caller drops
// it. The single materialization pass is acceptable today because
// the fast-path shapes the semantic executor composes with cap out
// at a few thousand rows; streaming-style draining is in scope for
// the row-source plan that follows
// `array-struct-semantic-path.plan.md`.
absl::StatusOr<std::unique_ptr<RowSource>> DrainRowSource(RowSource& source);

}  // namespace semantic
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator

#endif  // BIGQUERY_EMULATOR_BACKEND_ENGINE_SEMANTIC_ROW_SOURCE_H_
