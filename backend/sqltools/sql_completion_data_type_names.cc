#include "backend/sqltools/sql_completion_data_internal.h"

namespace bigquery_emulator {
namespace backend {
namespace sqltools {
namespace completion_data_internal {
const std::vector<std::string>& TypeNameEntries() {
  static const std::vector<std::string>* kEntries =
      new std::vector<std::string>{
          "INT64",      "FLOAT64", "NUMERIC",   "DECIMAL",  "BIGNUMERIC",
          "BIGDECIMAL", "BOOL",    "STRING",    "BYTES",    "DATE",
          "DATETIME",   "TIME",    "TIMESTAMP", "INTERVAL", "GEOGRAPHY",
          "JSON",       "ARRAY",   "STRUCT",    "RANGE",
      };
  return *kEntries;
}

}  // namespace completion_data_internal
}  // namespace sqltools
}  // namespace backend
}  // namespace bigquery_emulator
