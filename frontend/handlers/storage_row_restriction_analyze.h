#ifndef BIGQUERY_EMULATOR_FRONTEND_HANDLERS_STORAGE_ROW_RESTRICTION_ANALYZE_H_
#define BIGQUERY_EMULATOR_FRONTEND_HANDLERS_STORAGE_ROW_RESTRICTION_ANALYZE_H_

#include <string>

#include "absl/status/status.h"
#include "absl/strings/string_view.h"
#include "backend/storage/storage.h"

namespace bigquery_emulator {
namespace frontend {

absl::Status TranspileRowRestriction(absl::string_view restriction,
                                     const backend::storage::TableId& table,
                                     backend::storage::Storage* storage,
                                     std::string* where_sql);

}  // namespace frontend
}  // namespace bigquery_emulator

#endif  // BIGQUERY_EMULATOR_FRONTEND_HANDLERS_STORAGE_ROW_RESTRICTION_ANALYZE_H_
