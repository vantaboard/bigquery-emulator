#include "backend/engine/semantic/row_source.h"

#include <memory>
#include <utility>
#include <vector>

#include "absl/status/statusor.h"
#include "backend/engine/engine.h"
#include "backend/schema/schema.h"
#include "backend/storage/storage.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace semantic {

absl::StatusOr<std::unique_ptr<RowSource>> DrainRowSource(RowSource& source) {
  std::vector<storage::Row> rows;
  storage::Row row;
  while (true) {
    auto has = source.Next(&row);
    if (!has.ok()) return has.status();
    if (!*has) break;
    rows.push_back(row);
  }
  return std::unique_ptr<RowSource>(
      new MaterializedRowSource(source.schema(), std::move(rows)));
}

}  // namespace semantic
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
