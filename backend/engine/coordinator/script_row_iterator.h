#ifndef BIGQUERY_EMULATOR_BACKEND_ENGINE_COORDINATOR_SCRIPT_ROW_ITERATOR_H_
#define BIGQUERY_EMULATOR_BACKEND_ENGINE_COORDINATOR_SCRIPT_ROW_ITERATOR_H_

#include <memory>
#include <string>
#include <vector>

#include "googlesql/public/evaluator_table_iterator.h"
#include "googlesql/public/type.h"
#include "googlesql/public/value.h"
#include "absl/status/status.h"

namespace bigquery_emulator {
namespace backend {
namespace engine {
namespace coordinator {

class MaterializedEvaluatorTableIterator
    : public ::googlesql::EvaluatorTableIterator {
 public:
  MaterializedEvaluatorTableIterator(
      std::vector<std::string> column_names,
      std::vector<const ::googlesql::Type*> column_types,
      std::vector<std::vector<::googlesql::Value>> rows);

  int NumColumns() const override;
  std::string GetColumnName(int i) const override;
  const ::googlesql::Type* GetColumnType(int i) const override;
  bool NextRow() override;
  const ::googlesql::Value& GetValue(int i) const override;
  absl::Status Status() const override;
  absl::Status Cancel() override;

 private:
  std::vector<std::string> column_names_;
  std::vector<const ::googlesql::Type*> column_types_;
  std::vector<std::vector<::googlesql::Value>> rows_;
  int cursor_ = -1;
  absl::Status status_;
};

}  // namespace coordinator
}  // namespace engine
}  // namespace backend
}  // namespace bigquery_emulator

#endif  // BIGQUERY_EMULATOR_BACKEND_ENGINE_COORDINATOR_SCRIPT_ROW_ITERATOR_H_
