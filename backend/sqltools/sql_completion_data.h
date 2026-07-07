#ifndef BIGQUERY_EMULATOR_BACKEND_SQLTOOLS_SQL_COMPLETION_DATA_H_
#define BIGQUERY_EMULATOR_BACKEND_SQLTOOLS_SQL_COMPLETION_DATA_H_

#include <string>
#include <vector>

namespace bigquery_emulator {
namespace backend {
namespace sqltools {

struct ClauseWordEntry {
  std::string name;
  std::string type;
};

struct FunctionInfoEntry {
  std::string name;
  std::string args;
  std::string description;
  std::string url;
};

const std::vector<ClauseWordEntry>& ClauseWords();
const std::vector<std::string>& ExpressionKeywords();
const std::vector<FunctionInfoEntry>& FunctionInfo();
const std::vector<FunctionInfoEntry>& TvfFunctionInfo();
const std::vector<std::string>& TypeNames();

}  // namespace sqltools
}  // namespace backend
}  // namespace bigquery_emulator

#endif  // BIGQUERY_EMULATOR_BACKEND_SQLTOOLS_SQL_COMPLETION_DATA_H_
