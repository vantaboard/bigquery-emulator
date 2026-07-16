#include "backend/sqltools/sql_completion_data_internal.h"

namespace bigquery_emulator {
namespace backend {
namespace sqltools {
namespace completion_data_internal {
const std::vector<FunctionInfoEntry>& TvfFunctionInfoEntries() {
  static const std::vector<FunctionInfoEntry>* kEntries =
      new std::vector<FunctionInfoEntry>{
          {"ML.EVALUATE",
           "MODEL model, TABLE table | (subquery) [, STRUCT(numeric_expr AS "
           "threshold)]",
           "",
           ""},
          {"ML.FEATURE_INFO", "MODEL model", "", ""},
          {"ML.PREDICT", "MODEL model, TABLE table | (subquery)", "", ""},
          {"ML.ROC_CURVE",
           "MODEL model, TABLE table | (subquery) [, thresholds_array]",
           "",
           ""},
          {"ML.TRAINING_INFO", "MODEL model", "", ""},
          {"ML.WEIGHTS", "MODEL model", "", ""},
          {"GAP_FILL",
           "TABLE table | (subquery), time_series_column, bucket_width [, "
           "partitioning_columns, value_columns, origin, ignore_null_values]",
           "Finds and fills gaps in a time series.",
           "https://cloud.google.com/bigquery/docs/reference/standard-sql/"
           "functions-and-operators#gap_fill"},
          {"RANGE_SESSIONIZE",
           "TABLE table | (subquery), STRING range_column, ARRAY<STRING> "
           "partitioning_columns [, sessionize_mode]",
           "Produces a table of sessionized ranges.",
           "https://cloud.google.com/bigquery/docs/reference/standard-sql/"
           "range-functions#range_sessionize"},
      };
  return *kEntries;
}

}  // namespace completion_data_internal
}  // namespace sqltools
}  // namespace backend
}  // namespace bigquery_emulator
