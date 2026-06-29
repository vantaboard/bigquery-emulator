#include "backend/engine/coordinator/script_row_iterator.h"

#include <string>

#include "absl/status/status.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace coordinator {

MaterializedEvaluatorTableIterator::MaterializedEvaluatorTableIterator(
    std::vector<std::string> column_names,
    std::vector<const ::googlesql::Type*> column_types,
    std::vector<std::vector<::googlesql::Value>> rows)
    : column_names_(std::move(column_names)),
      column_types_(std::move(column_types)),
      rows_(std::move(rows)) {}

int MaterializedEvaluatorTableIterator::NumColumns() const {
  return static_cast<int>(column_names_.size());
}

std::string MaterializedEvaluatorTableIterator::GetColumnName(int i) const {
  return column_names_[static_cast<size_t>(i)];
}

const ::googlesql::Type* MaterializedEvaluatorTableIterator::GetColumnType(
    int i) const {
  return column_types_[static_cast<size_t>(i)];
}

bool MaterializedEvaluatorTableIterator::NextRow() {
  ++cursor_;
  if (cursor_ >= static_cast<int>(rows_.size())) {
    return false;
  }
  return true;
}

const ::googlesql::Value& MaterializedEvaluatorTableIterator::GetValue(
    int i) const {
  return rows_[static_cast<size_t>(cursor_)][static_cast<size_t>(i)];
}

absl::Status MaterializedEvaluatorTableIterator::Status() const {
  return status_;
}

absl::Status MaterializedEvaluatorTableIterator::Cancel() {
  return absl::OkStatus();
}

}  // namespace coordinator
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator
