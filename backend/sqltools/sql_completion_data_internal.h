#ifndef BIGQUERY_EMULATOR_BACKEND_SQLTOOLS_SQL_COMPLETION_DATA_INTERNAL_H_
#define BIGQUERY_EMULATOR_BACKEND_SQLTOOLS_SQL_COMPLETION_DATA_INTERNAL_H_

#include <string>
#include <vector>

#include "backend/sqltools/sql_completion_data.h"

namespace bigquery_emulator {
namespace backend {
namespace sqltools {
namespace completion_data_internal {

const std::vector<ClauseWordEntry>& ClauseWordEntries();
const std::vector<std::string>& ExpressionKeywordEntries();
const std::vector<FunctionInfoEntry>& FunctionInfoEntriesA();
const std::vector<FunctionInfoEntry>& FunctionInfoEntriesB();
const std::vector<FunctionInfoEntry>& FunctionInfoEntriesC();
const std::vector<FunctionInfoEntry>& TvfFunctionInfoEntries();
const std::vector<std::string>& TypeNameEntries();

}  // namespace completion_data_internal
}  // namespace sqltools
}  // namespace backend
}  // namespace bigquery_emulator

#endif  // BIGQUERY_EMULATOR_BACKEND_SQLTOOLS_SQL_COMPLETION_DATA_INTERNAL_H_
